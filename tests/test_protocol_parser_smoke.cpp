//
// test_protocol_parser_smoke.cpp
//
// In-memory smoke tests for ED2K Source Exchange packet boundary checks.
// No live network; exercises EDSourcePacketValidate only.
//
// This file is part of Envy (getenvy.com) (C) 2016-2026
//

#include "test_framework.h"
#include "../Envy/EDSourcePacketValidate.h"

static bool test_source_body_valid_exact()
{
	const DWORD nSourceSize = Ed2kSourceEx2SourceRecordBytes();
	const DWORD nCount = 2;
	const DWORD nRemaining = nCount * nSourceSize;
	return Ed2kValidateSourcePacketBody( nRemaining, nCount, nSourceSize ) == TRUE;
}

static bool test_source_body_valid_empty()
{
	const DWORD nSourceSize = Ed2kSourceEx2SourceRecordBytes();
	return Ed2kValidateSourcePacketBody( 0, 0, nSourceSize ) == TRUE;
}

static bool test_source_body_truncated()
{
	const DWORD nSourceSize = Ed2kSourceEx2SourceRecordBytes();
	return Ed2kValidateSourcePacketBody( nSourceSize, 2, nSourceSize ) == FALSE;
}

static bool test_source_body_count_overflow()
{
	const DWORD nSourceSize = 12u;
	return Ed2kValidateSourcePacketBody( 100, 20, nSourceSize ) == FALSE;
}

static bool test_source_body_zero_record_size()
{
	return Ed2kValidateSourcePacketBody( 100, 1, 0 ) == FALSE;
}

static bool test_source_ex2_request_min_header()
{
	return Ed2kSourceEx2RequestMinBytes() == 22u;
}

static bool test_source_ex2_answer_min_header()
{
	return Ed2kSourceEx2AnswerMinBytes() == 18u;
}

static bool test_source_ex2_legacy_high32_consumed()
{
	const DWORD nRemainingAfterLow = 6;
	return Ed2kSourceEx2LegacyHigh32Bytes( 0, nRemainingAfterLow ) == 4u;
}

static bool test_source_ex2_legacy_high32_skipped_short()
{
	return Ed2kSourceEx2LegacyHigh32Bytes( 0, 5 ) == 0u;
}

static bool test_source_ex2_legacy_high32_skipped_nonzero_low()
{
	return Ed2kSourceEx2LegacyHigh32Bytes( 1024, 10 ) == 0u;
}

static bool test_source_ex2_options_tail_present()
{
	const DWORD nRemaining = 4 + 2;
	const DWORD nLegacy = Ed2kSourceEx2LegacyHigh32Bytes( 0, 6 );
	return Ed2kSourceEx2HasOptionsBytes( nRemaining - nLegacy ) == TRUE;
}

static bool test_source_ex2_options_tail_missing()
{
	return Ed2kSourceEx2HasOptionsBytes( 1 ) == FALSE;
}

static bool test_source_ex2_answer_malformed_count()
{
	const DWORD nSourceSize = Ed2kSourceEx2SourceRecordBytes();
	return Ed2kValidateSourcePacketBody( nSourceSize - 1, 1, nSourceSize ) == FALSE;
}

void register_protocol_parser_smoke_tests(TestSuite& suite)
{
	suite.add_test( "ed2k_source_body_exact_fit", test_source_body_valid_exact );
	suite.add_test( "ed2k_source_body_empty", test_source_body_valid_empty );
	suite.add_test( "ed2k_source_body_truncated", test_source_body_truncated );
	suite.add_test( "ed2k_source_body_count_overflow", test_source_body_count_overflow );
	suite.add_test( "ed2k_source_body_zero_record", test_source_body_zero_record_size );
	suite.add_test( "ed2k_source_ex2_request_min_header", test_source_ex2_request_min_header );
	suite.add_test( "ed2k_source_ex2_answer_min_header", test_source_ex2_answer_min_header );
	suite.add_test( "ed2k_source_ex2_legacy_high32", test_source_ex2_legacy_high32_consumed );
	suite.add_test( "ed2k_source_ex2_legacy_skip_short", test_source_ex2_legacy_high32_skipped_short );
	suite.add_test( "ed2k_source_ex2_legacy_skip_nonzero", test_source_ex2_legacy_high32_skipped_nonzero_low );
	suite.add_test( "ed2k_source_ex2_options_present", test_source_ex2_options_tail_present );
	suite.add_test( "ed2k_source_ex2_options_missing", test_source_ex2_options_tail_missing );
	suite.add_test( "ed2k_source_ex2_answer_truncated", test_source_ex2_answer_malformed_count );
}
