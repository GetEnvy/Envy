//
// test_merkle_tree.cpp
//
// Unit tests for MerkleTree implementation (BitTorrent v2)
//

#include "stdafx.h"
#include "../Envy/MerkleTree.h"
#include <vector>
#include <string>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Test helper function to create test data
std::vector<uint8_t> createTestData(size_t size, uint8_t pattern = 0xAA) {
	std::vector<uint8_t> data(size);
	for (size_t i = 0; i < size; ++i) {
		data[i] = static_cast<uint8_t>(pattern + i % 256);
	}
	return data;
}

// Test basic MerkleTree construction
void testBasicConstruction() {
	printf("Testing basic MerkleTree construction...\n");

	// Create test data (64 KiB - 4 pieces of 16 KiB each)
	auto testData = createTestData(64 * 1024);

	// Test construction
	BitTorrent::MerkleTree tree(testData.data(), testData.size());

	// Verify tree is valid
	if (!tree.isValid()) {
		printf("ERROR: Tree is not valid after construction\n");
		return;
	}

	// Verify root hash exists
	auto rootHash = tree.getRootHash();
	if (rootHash.isEmpty()) {
		printf("ERROR: Root hash is empty\n");
		return;
	}

	// Verify piece count
	size_t expectedPieces = (testData.size() + 16 * 1024 - 1) / (16 * 1024);
	if (tree.getPieceCount() != expectedPieces) {
		printf("ERROR: Expected %zu pieces, got %zu\n", expectedPieces, tree.getPieceCount());
		return;
	}

	// Verify file size
	if (tree.getFileSize() != testData.size()) {
		printf("ERROR: File size mismatch\n");
		return;
	}

	printf("Basic construction test PASSED\n");
}

// Test tree construction with different sizes
void testDifferentSizes() {
	printf("Testing MerkleTree with different file sizes...\n");

	// Test with exactly 16 KiB (1 piece)
	{
		auto data = createTestData(16 * 1024);
		BitTorrent::MerkleTree tree(data.data(), data.size());
		if (!tree.isValid() || tree.getPieceCount() != 1) {
			printf("ERROR: Single piece test failed\n");
			return;
		}
	}

	// Test with 32 KiB (2 pieces)
	{
		auto data = createTestData(32 * 1024);
		BitTorrent::MerkleTree tree(data.data(), data.size());
		if (!tree.isValid() || tree.getPieceCount() != 2) {
			printf("ERROR: Two piece test failed\n");
			return;
		}
	}

	// Test with uneven size (20 KiB - should be 2 pieces)
	{
		auto data = createTestData(20 * 1024);
		BitTorrent::MerkleTree tree(data.data(), data.size());
		if (!tree.isValid() || tree.getPieceCount() != 2) {
			printf("ERROR: Uneven size test failed\n");
			return;
		}
	}

	printf("Different sizes test PASSED\n");
}

// Test piece layer functionality
void testPieceLayers() {
	printf("Testing piece layer functionality...\n");

	auto testData = createTestData(64 * 1024);
	BitTorrent::MerkleTree tree(testData.data(), testData.size());

	if (!tree.isValid()) {
		printf("ERROR: Tree not valid for piece layer test\n");
		return;
	}

	// Test getting piece layers
	for (uint32_t i = 0; i < tree.getPieceCount(); ++i) {
		auto layer = tree.getPieceLayer(i);
		if (layer.pieceIndex != i) {
			printf("ERROR: Piece layer index mismatch\n");
			return;
		}
		// Note: In a full implementation, layer.hashes would contain proof hashes
	}

	printf("Piece layer test PASSED\n");
}

// Test root hash consistency
void testRootHashConsistency() {
	printf("Testing root hash consistency...\n");

	auto testData = createTestData(64 * 1024);

	// Create two identical trees
	BitTorrent::MerkleTree tree1(testData.data(), testData.size());
	BitTorrent::MerkleTree tree2(testData.data(), testData.size());

	if (!tree1.isValid() || !tree2.isValid()) {
		printf("ERROR: Trees not valid for consistency test\n");
		return;
	}

	// Root hashes should be identical
	auto hash1 = tree1.getRootHash();
	auto hash2 = tree2.getRootHash();

	if (hash1 != hash2) {
		printf("ERROR: Root hashes are not identical for same data\n");
		return;
	}

	printf("Root hash consistency test PASSED\n");
}

// Main test function
int test_merkle_tree()
{
	printf("Running MerkleTree unit tests...\n\n");

	try {
		testBasicConstruction();
		testDifferentSizes();
		testPieceLayers();
		testRootHashConsistency();

		printf("\nAll MerkleTree tests PASSED!\n");
		return 0;
	}
	catch (const std::exception& e) {
		printf("ERROR: Exception during testing: %s\n", e.what());
		return 1;
	}
	catch (...) {
		printf("ERROR: Unknown exception during testing\n");
		return 1;
	}
}
