//
// test_main.cpp
//
// Test runner for Envy unit tests
// Structured output for CI integration
//
// This file is part of Envy (getenvy.com) (C) 2016-2026
// License: GNU Affero General Public License v3.0 (AGPLv3)
//

#include "test_framework.h"

// Test modules register their tests via these functions
void register_hashlib_tests(TestSuite& suite);
void register_protocol_parser_smoke_tests(TestSuite& suite);
void register_secureident_policy_smoke_tests(TestSuite& suite);
void register_ed2k_hello_capabilities_smoke_tests(TestSuite& suite);
void register_ed2k_hello_golden_tests(TestSuite& suite);
void register_ed2k_sourceex2_golden_tests(TestSuite& suite);
void register_ed2k_kad_settings_smoke_tests(TestSuite& suite);
void register_ed2k_compressed_upload_smoke_tests(TestSuite& suite);
void register_webhook_registration_smoke_tests(TestSuite& suite);
void register_skin_engine_p0_smoke_tests(TestSuite& suite);
void register_network_interface_selector_smoke_tests(TestSuite& suite);
void register_remote_base64_smoke_tests(TestSuite& suite);
void register_secure_random_smoke_tests(TestSuite& suite);
void register_dc_packet_length_smoke_tests(TestSuite& suite);
void register_dc_hublist_sources_smoke_tests(TestSuite& suite);
void register_dc_user_file_browse_smoke_tests(TestSuite& suite);
void register_dc_nmdc_text_smoke_tests(TestSuite& suite);
void register_dc_maxedout_queue_smoke_tests(TestSuite& suite);
void register_ed2k_lock_order_smoke_tests(TestSuite& suite);
void register_envy_thread_policy_smoke_tests(TestSuite& suite);
void register_remote_password_policy_smoke_tests(TestSuite& suite);
void register_remote_html_escape_smoke_tests(TestSuite& suite);
void register_remote_access_smoke_tests(TestSuite& suite);
void register_xml_peer_parse_smoke_tests(TestSuite& suite);
void register_firewall_wfas_policy_smoke_tests(TestSuite& suite);
void register_network_job_queue_smoke_tests(TestSuite& suite);
void register_chat_session_queue_smoke_tests(TestSuite& suite);
void register_textctrl_viewport_smoke_tests(TestSuite& suite);
void register_adaptive_list_columns_smoke_tests(TestSuite& suite);
void register_bootstrap_catalog_smoke_tests(TestSuite& suite);
void register_kad_nodes_dat_tests(TestSuite& suite);
void register_transfer_settings_limits_smoke_tests(TestSuite& suite);
void register_transfer_state_smoke_tests(TestSuite& suite);
void register_kad_search_res_delivery_smoke_tests(TestSuite& suite);
void register_kad_search_source_request_smoke_tests(TestSuite& suite);
void register_kad_firewall_check_smoke_tests(TestSuite& suite);
void register_kad_bootstrap_cold_start_smoke_tests(TestSuite& suite);
void register_kad_routing_table_tests(TestSuite& suite);
void register_crash_report_policy_smoke_tests(TestSuite& suite);
void register_ed2k_lowid_callback_smoke_tests(TestSuite& suite);
void register_albumfolder_mountcollection_smoke_tests(TestSuite& suite);
void register_btinfo_piece_hash_smoke_tests(TestSuite& suite);

int main(int argc, char** argv)
{
	(void)argc;
	(void)argv;
	TestSuite suite;

	register_hashlib_tests(suite);
	register_protocol_parser_smoke_tests(suite);
	register_secureident_policy_smoke_tests(suite);
	register_ed2k_hello_capabilities_smoke_tests(suite);
	register_ed2k_hello_golden_tests(suite);
	register_ed2k_sourceex2_golden_tests(suite);
	register_ed2k_kad_settings_smoke_tests(suite);
	register_ed2k_compressed_upload_smoke_tests(suite);
	register_webhook_registration_smoke_tests(suite);
	register_skin_engine_p0_smoke_tests(suite);
	register_network_interface_selector_smoke_tests(suite);
	register_remote_base64_smoke_tests(suite);
	register_secure_random_smoke_tests(suite);
	register_dc_packet_length_smoke_tests(suite);
	register_dc_hublist_sources_smoke_tests(suite);
	register_dc_user_file_browse_smoke_tests(suite);
	register_dc_nmdc_text_smoke_tests(suite);
	register_dc_maxedout_queue_smoke_tests(suite);
	register_ed2k_lock_order_smoke_tests(suite);
	register_envy_thread_policy_smoke_tests(suite);
	register_remote_password_policy_smoke_tests(suite);
	register_remote_html_escape_smoke_tests(suite);
	register_remote_access_smoke_tests(suite);
	register_xml_peer_parse_smoke_tests(suite);
	register_firewall_wfas_policy_smoke_tests(suite);
	register_network_job_queue_smoke_tests(suite);
	register_chat_session_queue_smoke_tests(suite);
	register_textctrl_viewport_smoke_tests(suite);
	register_adaptive_list_columns_smoke_tests(suite);
	register_bootstrap_catalog_smoke_tests(suite);
	register_kad_nodes_dat_tests(suite);
	register_transfer_settings_limits_smoke_tests(suite);
	register_transfer_state_smoke_tests(suite);
	register_kad_search_res_delivery_smoke_tests(suite);
	register_kad_search_source_request_smoke_tests(suite);
	register_kad_firewall_check_smoke_tests(suite);
	register_kad_bootstrap_cold_start_smoke_tests(suite);
	register_kad_routing_table_tests(suite);
	register_crash_report_policy_smoke_tests(suite);
	register_ed2k_lowid_callback_smoke_tests(suite);
	register_albumfolder_mountcollection_smoke_tests(suite);
	register_btinfo_piece_hash_smoke_tests(suite);

	int failures = suite.run_all_tests();

	return (failures == 0) ? 0 : 1;
}
