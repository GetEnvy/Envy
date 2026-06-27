//
// MerkleTree.h
//
// Merkle tree implementation for BitTorrent v2 (BEP-52)
// Provides efficient file verification using SHA-256 hashing
//

#pragma once

#include "Hashes.hpp"
#include <vector>
#include <memory>
#include <cstdint>

namespace BitTorrent
{
	//! \brief Merkle tree node for BitTorrent v2 file verification
	struct MerkleNode
	{
		Hashes::Sha256Hash hash;  //!< SHA-256 hash of this node
		std::unique_ptr<MerkleNode> left;   //!< Left child node
		std::unique_ptr<MerkleNode> right;  //!< Right child node

		MerkleNode() = default;
		explicit MerkleNode(const Hashes::Sha256Hash& h) : hash(h) {}
		MerkleNode(const MerkleNode&) = delete;
		MerkleNode& operator=(const MerkleNode&) = delete;
		MerkleNode(MerkleNode&&) = default;
		MerkleNode& operator=(MerkleNode&&) = default;
	};

	//! \brief Piece layer entry for efficient metadata distribution
	struct PieceLayer
	{
		uint32_t pieceIndex;        //!< Index of the piece this layer belongs to
		std::vector<Hashes::Sha256Hash> hashes;  //!< Hashes for this piece layer

		PieceLayer(uint32_t index = 0) : pieceIndex(index) {}
	};

	//! \brief Merkle tree implementation for BitTorrent v2
	class MerkleTree
	{
	public:
		//! \brief Default constructor
		MerkleTree();

		//! \brief Construct Merkle tree from file data
		//! \param fileData Pointer to file data
		//! \param fileSize Size of the file in bytes
		//! \param pieceLength Length of each piece in bytes (must be power of 2, ≥ 16 KiB)
		explicit MerkleTree(const uint8_t* fileData, size_t fileSize, size_t pieceLength = 16 * 1024);

		//! \brief Construct Merkle tree from existing root hash
		//! \param rootHash The root hash of the tree
		//! \param fileSize Size of the file in bytes
		//! \param pieceLength Length of each piece in bytes
		MerkleTree(const Hashes::Sha256Hash& rootHash, size_t fileSize, size_t pieceLength = 16 * 1024);

		MerkleTree(const MerkleTree&) = delete;
		MerkleTree& operator=(const MerkleTree&) = delete;
		MerkleTree(MerkleTree&&) = default;
		MerkleTree& operator=(MerkleTree&&) = default;

		//! \brief Build the Merkle tree from file data
		//! \param fileData Pointer to file data
		//! \param fileSize Size of the file in bytes
		//! \return true if successful, false otherwise
		bool buildFromData(const uint8_t* fileData, size_t fileSize);

		//! \brief Get the root hash of the tree
		//! \return Root hash, or empty hash if tree not built
		const Hashes::Sha256Hash& getRootHash() const;

		//! \brief Verify a piece against the Merkle tree
		//! \param pieceData The piece data to verify
		//! \param pieceIndex Index of the piece
		//! \param proofHashes The proof hashes for verification
		//! \return true if piece is valid, false otherwise
		bool verifyPiece(const uint8_t* pieceData, size_t pieceSize, uint32_t pieceIndex,
						const std::vector<Hashes::Sha256Hash>& proofHashes) const;

		//! \brief Get piece layer for a specific piece
		//! \param pieceIndex Index of the piece
		//! \return Piece layer containing the necessary hashes
		PieceLayer getPieceLayer(uint32_t pieceIndex) const;

		//! \brief Get proof hashes for piece verification
		//! \param pieceIndex Index of the piece to verify
		//! \return Vector of proof hashes needed for verification
		std::vector<Hashes::Sha256Hash> getProofHashes(uint32_t pieceIndex) const;

		//! \brief Check if the tree is valid and complete
		//! \return true if tree is valid, false otherwise
		bool isValid() const;

		//! \brief Get the number of pieces in the file
		//! \return Number of pieces
		size_t getPieceCount() const;

		//! \brief Get the piece length used by this tree
		//! \return Piece length in bytes
		size_t getPieceLength() const;

		//! \brief Get the total file size
		//! \return File size in bytes
		size_t getFileSize() const;

	private:
		//! \brief Build the leaf nodes from file data
		//! \param fileData Pointer to file data
		//! \param fileSize Size of the file in bytes
		void buildLeaves(const uint8_t* fileData, size_t fileSize);

		//! \brief Build parent nodes from leaf nodes
		void buildParents();

		//! \brief Balance the tree by padding with zeros if necessary
		void balanceTree();

		//! \brief Compute hash for a data block
		//! \param data Pointer to data
		//! \param size Size of data
		//! \return SHA-256 hash of the data
		static Hashes::Sha256Hash hashBlock(const uint8_t* data, size_t size);

		//! \brief Compute parent hash from two child hashes
		//! \param left Left child hash
		//! \param right Right child hash
		//! \return Parent hash
		static Hashes::Sha256Hash hashParent(const Hashes::Sha256Hash& left, const Hashes::Sha256Hash& right);

		std::unique_ptr<MerkleNode> m_root;           //!< Root node of the tree
		std::vector<std::unique_ptr<MerkleNode>> m_leaves;  //!< Leaf nodes (one per 16 KiB block)
		size_t m_pieceLength;                      //!< Length of each piece in bytes
		size_t m_fileSize;                         //!< Total file size in bytes
		bool m_isValid;                            //!< Whether the tree is valid
	};
} // namespace BitTorrent
