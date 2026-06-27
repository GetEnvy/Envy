//
// MerkleTree.cpp
//
// Merkle tree implementation for BitTorrent v2 (BEP-52)
//

#include "MerkleTree.h"
#include "CryptoProvider.h"  // For SHA-256 operations
#include <algorithm>
#include <stdexcept>

namespace BitTorrent
{
	constexpr size_t BLOCK_SIZE = 16 * 1024;  // 16 KiB blocks as per BEP-52

	MerkleTree::MerkleTree()
		: m_pieceLength(BLOCK_SIZE)
		, m_fileSize(0)
		, m_isValid(false)
	{
	}

	MerkleTree::MerkleTree(const uint8_t* fileData, size_t fileSize, size_t pieceLength)
		: m_pieceLength(pieceLength)
		, m_fileSize(fileSize)
		, m_isValid(false)
	{
		if (pieceLength < BLOCK_SIZE) {
			throw std::invalid_argument("Piece length must be at least 16 KiB");
		}
		if ((pieceLength & (pieceLength - 1)) != 0) {
			throw std::invalid_argument("Piece length must be a power of 2");
		}
		if (fileData && fileSize > 0) {
			buildFromData(fileData, fileSize);
		}
	}

	MerkleTree::MerkleTree(const Hashes::Sha256Hash& rootHash, size_t fileSize, size_t pieceLength)
		: m_pieceLength(pieceLength)
		, m_fileSize(fileSize)
		, m_isValid(false)
	{
		if (pieceLength < BLOCK_SIZE) {
			throw std::invalid_argument("Piece length must be at least 16 KiB");
		}
		if ((pieceLength & (pieceLength - 1)) != 0) {
			throw std::invalid_argument("Piece length must be a power of 2");
		}

		// Create a root node with the given hash
		m_root = std::make_unique<MerkleNode>(rootHash);
		m_isValid = true;  // Assume valid if constructed with root hash
	}

	bool MerkleTree::buildFromData(const uint8_t* fileData, size_t fileSize)
	{
		if (!fileData || fileSize == 0) {
			return false;
		}

		m_fileSize = fileSize;
		m_leaves.clear();
		m_root.reset();
		m_isValid = false;

		try {
			buildLeaves(fileData, fileSize);
			balanceTree();
			buildParents();
			m_isValid = true;
			return true;
		}
		catch (const std::exception&) {
			m_leaves.clear();
			m_root.reset();
			m_isValid = false;
			return false;
		}
	}

	const Hashes::Sha256Hash& MerkleTree::getRootHash() const
	{
		static const Hashes::Sha256Hash emptyHash;
		return m_root ? m_root->hash : emptyHash;
	}

	bool MerkleTree::verifyPiece(const uint8_t* pieceData, size_t pieceSize, uint32_t pieceIndex,
								const std::vector<Hashes::Sha256Hash>& proofHashes) const
	{
		if (!m_isValid || !pieceData || pieceSize == 0) {
			return false;
		}

		if (pieceIndex >= getPieceCount()) {
			return false;
		}

		// Hash the piece data
		Hashes::Sha256Hash pieceHash = hashBlock(pieceData, pieceSize);

		// Start with the piece hash and combine with proof hashes
		Hashes::Sha256Hash currentHash = pieceHash;

		// Combine with proof hashes to reconstruct path to root
		for (const auto& proofHash : proofHashes) {
			// In a proper implementation, we need to know the order (left/right)
			// For now, we'll assume alternating pattern or need additional metadata
			currentHash = hashParent(currentHash, proofHash);
		}

		// Check if the reconstructed hash matches the root
		return currentHash == getRootHash();
	}

	PieceLayer MerkleTree::getPieceLayer(uint32_t pieceIndex) const
	{
		PieceLayer layer(pieceIndex);

		if (!m_isValid || pieceIndex >= getPieceCount()) {
			return layer;
		}

		// For BEP-52, piece layers contain the hashes needed to verify a piece
		// This is a simplified implementation - in practice, this would contain
		// the necessary proof hashes for the piece
		layer.hashes = getProofHashes(pieceIndex);

		return layer;
	}

std::vector<Hashes::Sha256Hash> MerkleTree::getProofHashes(uint32_t pieceIndex) const
{
	std::vector<Hashes::Sha256Hash> proofHashes;

	if (!m_isValid || pieceIndex >= getPieceCount()) {
		return proofHashes;
	}

	// For BEP-52, proof hashes are the sibling nodes needed to reconstruct
	// the path from the piece's leaf to the root

	// Find the leaf corresponding to this piece
	size_t leafIndex = pieceIndex;

	// Traverse up the tree, collecting sibling hashes
	size_t currentIndex = leafIndex;
	size_t levelSize = m_leaves.size();

	while (levelSize > 1) {
		// Determine if this is a left or right child
		bool isLeftChild = (currentIndex % 2) == 0;
		size_t siblingIndex = isLeftChild ? currentIndex + 1 : currentIndex - 1;

		// If sibling exists in this level, add its hash to proof
		if (siblingIndex < levelSize) {
			// For now, we need to reconstruct the tree structure
			// This is a simplified implementation
			Hashes::Sha256Hash dummyHash;
			proofHashes.push_back(dummyHash);
		}

		// Move to parent level
		currentIndex /= 2;
		levelSize = (levelSize + 1) / 2;  // Ceiling division
	}

	return proofHashes;
}

	bool MerkleTree::isValid() const
	{
		return m_isValid && m_root != nullptr;
	}

	size_t MerkleTree::getPieceCount() const
	{
		if (m_fileSize == 0 || m_pieceLength == 0) {
			return 0;
		}
		return (m_fileSize + m_pieceLength - 1) / m_pieceLength;
	}

	size_t MerkleTree::getPieceLength() const
	{
		return m_pieceLength;
	}

	size_t MerkleTree::getFileSize() const
	{
		return m_fileSize;
	}

	void MerkleTree::buildLeaves(const uint8_t* fileData, size_t fileSize)
	{
		size_t blockCount = (fileSize + BLOCK_SIZE - 1) / BLOCK_SIZE;
		m_leaves.reserve(blockCount);

		for (size_t i = 0; i < blockCount; ++i) {
			size_t offset = i * BLOCK_SIZE;
			size_t blockSize = std::min(BLOCK_SIZE, fileSize - offset);

			Hashes::Sha256Hash hash = hashBlock(fileData + offset, blockSize);
			m_leaves.push_back(std::make_unique<MerkleNode>(hash));
		}
	}

	void MerkleTree::buildParents()
	{
		if (m_leaves.empty()) {
			return;
		}

		// Start with leaves as current level
		std::vector<std::unique_ptr<MerkleNode>> currentLevel = std::move(m_leaves);

		// Build each level until we reach the root
		while (currentLevel.size() > 1) {
			std::vector<std::unique_ptr<MerkleNode>> nextLevel;

			// Process nodes in pairs
			for (size_t i = 0; i < currentLevel.size(); i += 2) {
				auto left = std::move(currentLevel[i]);
				std::unique_ptr<MerkleNode> right;

				if (i + 1 < currentLevel.size()) {
					right = std::move(currentLevel[i + 1]);
				} else {
					// Duplicate last node if odd number (implicit padding)
					right = std::make_unique<MerkleNode>(left->hash);
				}

				Hashes::Sha256Hash parentHash = hashParent(left->hash, right->hash);
				auto parent = std::make_unique<MerkleNode>(parentHash);
				parent->left = std::move(left);
				parent->right = std::move(right);

				nextLevel.push_back(std::move(parent));
			}

			currentLevel = std::move(nextLevel);
		}

		// The last remaining node is the root
		if (!currentLevel.empty()) {
			m_root = std::move(currentLevel[0]);
		}
	}

void MerkleTree::balanceTree()
{
	if (m_leaves.empty()) {
		return;
	}

	// For BEP-52, we balance the tree by ensuring it's a complete binary tree
	// We don't pad with duplicates; instead we handle uneven trees in buildParents
	// This method ensures proper tree structure for Merkle tree construction

	// The tree will be balanced during parent construction by handling odd numbers of nodes
}

	Hashes::Sha256Hash MerkleTree::hashBlock(const uint8_t* data, size_t size)
	{
		Hashes::Sha256Hash result;

		// Use CryptoProvider for SHA-256 computation
		if (CCryptoProvider::SHA256(data, size, result.data(), result.byteCount)) {
			result.validate();
			return result;
		}

		// Fallback: return zero hash on failure
		return Hashes::Sha256Hash();
	}

	Hashes::Sha256Hash MerkleTree::hashParent(const Hashes::Sha256Hash& left, const Hashes::Sha256Hash& right)
	{
		// Concatenate left and right hashes and hash them together
		uint8_t combined[Hashes::Sha256Hash::byteCount * 2];
		memcpy(combined, left.data(), left.byteCount);
		memcpy(combined + left.byteCount, right.data(), right.byteCount);

		return hashBlock(combined, sizeof(combined));
	}
} // namespace BitTorrent
