//
// GeoIP.csv2dat.cpp
//

/* geoip-csv-to-dat - convert a country database from CSV to GeoIP binary format
 *
 * Copyright (c) 2009 Kalle Olavi Niemitalo.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#define _GNU_SOURCE 1
#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
//#include <error.h>
//#include <errno.h>
#include <fstream>
#include <iostream>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <stdint.h>
#include <sys/types.h>
#include <Ws2tcpip.h>
#include <winsock2.h>
#include <wininet.h>
#include <vector>
#include "GetOpt.h"
#include "GetOpt.cpp"
#include "..\GeoIP.h"

// #include <sysexits.h> :
#define EX_OK 0	/* EXIT_SUCCESS */
#define EX_USAGE 64
#define EX_DATAERR 65
#define EX_NOINPUT 66
#define EX_CANTCREAT 73
#define EX_IOERR 74


// Format of GeoIP Country database files
// ======================================
//
// 1. Binary trie mapping IP addresses to countries.
// 2. Optional unused data.
// 3. Optional database-info block.
// 4. Optional structure-info block.
//
// Binary trie
// -----------
//
// The trie treats IP addresses as bit sequences and maps them to numbers.
// In the country database, each such number is 0xFFFF00 +
// the the country ID that GeoIP_id_by_ipnum() would return.
// The meanings of country IDs are hardcoded in libGeoIP and
// cannot be overridden by the database.
//
// The root node of the trie is at the beginning of the file, and the
// other nodes then follow it. Each node has the same size and
// consists of two little-endian pointers that correspond to the two
// possible values of a bit.  In the country database, the pointers are
// 24-bit, and each node is thus 6 bytes long.
//
// Each pointer is one of:
// - The number that the whole lookup should return, i.e. 0xFFFF00 + id
//   in the country database.
// - The number of the node that should be examined next, counting from
//   0 at the beginning of the file.  Pointing back to nodes with
//   smaller numbers is allowed, but loops are not allowed.
//
// Optional unused data
// --------------------
//
// The file format seems to permit extra data between the binary trie
// and the optional blocks.
//
// Optional database-info block
// ----------------------------
//
// Near the end of the file, there may be a three-byte tag (0x00 0x00 0x00)
// followed by at most DATABASE_INFO_MAX_SIZE - 1 = 99 bytes of
// text that describes the database.  GeoIP_database_info() returns
// this text and appends a terminating '\0'.
//
// The GeoLite Country IPv4 database downloadable from MaxMind
// includes this database-info block.
//
// Optional structure-info block
// -----------------------------
//
// At the very end of the file, there may be a three-byte tag
// (0xFF 0xFF 0xFF) followed by at most STRUCTURE_INFO_MAX_SIZE - 1 =
// 19 bytes.  The first byte is the database type,
// e.g. GEOIP_COUNTRY_EDITION = 1 or GEOIP_COUNTRY_EDITION_V6 = 12,
// possibly with 105 added to it.  Type-specific information then follows.
// There is no type-specific information for the country editions.
//
// The GeoLite Country IPv4 database downloadable from MaxMind does
// not include this structure-info block.

namespace {
      class binary_trie
      {
      public:
            typedef uint_fast32_t edge_type;

            explicit binary_trie(edge_type leaf);
            void set_range(
                  const uint8_t range_min[],
                  const uint8_t range_max[],
                  std::size_t bit_count,
                  edge_type leaf);
            void reorder_depth_first();
            void reorder_in_blocks(std::size_t bytes_per_block);
            void write_binary(std::ostream &dat_stream) const;

      private:
            struct node
            {
                  edge_type edges[2];
            };
            std::vector<node> nodes;

            // This could be std::vector<bool> but that seems slower.
            typedef std::vector<uint8_t> bits_vector;

            void set_range_in_node(
                  const bits_vector *min_bits,
                  const bits_vector *max_bits,
                  std::size_t bit_pos,
                  edge_type edit_node,
                  edge_type leaf);
            void set_range_in_edge(
                  const bits_vector *min_bits,
                  const bits_vector *max_bits,
                  std::size_t bit_pos,
                  edge_type edit_node,
                  bool bit,
                  edge_type leaf);
            void reorder(
                  const std::vector<edge_type> &old_to_new,
                  const std::vector<edge_type> &new_to_old);
      };
}

/** Construct a binary trie and its root node.
 *
 * \param leaf
 * Both edges of the root node will initially point to this leaf.
 * The caller should provide a value that means nothing was found.  */
binary_trie::binary_trie(edge_type leaf)
{
      const node node = {{ leaf, leaf }};
      nodes.push_back(node);
}

/** Edit the trie so it maps a range of bit sequences to the same leaf.
 *
 * \param range_min
 * The first bit sequence in the range.  Eight bits are packed in each byte.
 * The most significant bit of the whole sequence is in the
 * most significant bit of the first byte.
 *
 * \param range_max
 * The last bit sequence in the range.
 *
 * \param bit_count
 * The number of bits in both sequences.
 *
 * \param leaf
 * The leaf to which all the bit sequences in the range should be mapped.  */
void
binary_trie::set_range(
      const uint8_t range_min[],
      const uint8_t range_max[],
      std::size_t bit_count,
      edge_type leaf)
{
      bits_vector min_bits(bit_count);
      bits_vector max_bits(bit_count);
      for (std::size_t i = 0; i < bit_count; ++i) {
            std::size_t byte_pos = i / 8;
            uint8_t mask = 1 << ((~i) % 8);
            min_bits[i] = ((range_min[byte_pos] & mask) != 0);
            max_bits[i] = ((range_max[byte_pos] & mask) != 0);
      }
      set_range_in_node(&min_bits, &max_bits, 0, 0, leaf);
}

/** Edit a node in the trie so it maps a range of bit sequences to the same leaf.
 *
 * \param min_bits
 * The first bit sequence in the range, or NULL if unbounded.
 *
 * \param max_bits
 * The last bit sequence in the range, or NULL if unbounded.
 *
 * \param bit_pos
 * Which bit in the sequences corresponds to \a edit_node.
 *
 * \param edit_node
 * The node to be modified.
 *
 * \param leaf
 * The leaf to which all the bit sequences in the range should be mapped.  */
void
binary_trie::set_range_in_node(
      const bits_vector *min_bits,
      const bits_vector *max_bits,
      std::size_t bit_pos,
      edge_type edit_node,
      edge_type leaf)
{
      if (!min_bits || (*min_bits)[bit_pos] == false) {
            set_range_in_edge(min_bits,
                          (max_bits && (*max_bits)[bit_pos] == false)
                          ? max_bits : NULL,
                          bit_pos + 1, edit_node, false, leaf);
      }
      if (!max_bits || (*max_bits)[bit_pos] == true) {
            set_range_in_edge((min_bits && (*min_bits)[bit_pos] == true)
                          ? min_bits : NULL,
                          max_bits,
                          bit_pos + 1, edit_node, true, leaf);
      }
}

/** Edit an edge in the trie so it maps a range of bit sequences to the same leaf.
 *
 * \param min_bits
 * The first bit sequence in the range, or NULL if unbounded.
 *
 * \param max_bits
 * The last bit sequence in the range, or NULL if unbounded.
 *
 * \param bit_pos
 * Which bit in the sequences corresponds to \a bit.
 *
 * \param edit_node
 * The node in which the edge to be modified is located.
 *
 * \param bit
 * Which edge of \a edit_node should be modified.
 *
 * \param leaf
 * The leaf to which all the bit sequences in the range should be mapped.  */
void
binary_trie::set_range_in_edge(
      const bits_vector *min_bits,
      const bits_vector *max_bits,
      std::size_t bit_pos,
      edge_type edit_node,
      bool bit,
      edge_type leaf)
{
      // Check if the range fills this edge entirely.
      bool entire = true;
      if (min_bits
          && std::find(min_bits->begin() + bit_pos, min_bits->end(),
                   true) != min_bits->end())
            entire = false;
      if (max_bits
          && std::find(max_bits->begin() + bit_pos, max_bits->end(),
                   false) != max_bits->end())
            entire = false;

      if (entire) {
            nodes[edit_node].edges[bit] = leaf;
      } else {
            edge_type next = nodes[edit_node].edges[bit];
            if (next >= nodes.size()) {
                  const node new_node = {{ next, next }};
                  next = nodes.size();
                  nodes.push_back(new_node);
                  nodes[edit_node].edges[bit] = next;
            }

            set_range_in_node(min_bits, max_bits, bit_pos, next, leaf);
      }
}

/** Renumber the nodes in depth-first order.  */
void
binary_trie::reorder_depth_first()
{
      std::vector<edge_type> old_to_new, new_to_old;
      std::stack<edge_type> depth_first;
      old_to_new.resize(nodes.size(), -1);
      new_to_old.reserve(nodes.size());
      depth_first.push(0);
      while (!depth_first.empty()) {
            const edge_type edge = depth_first.top();
            depth_first.pop();
            if (edge < nodes.size()) {
                  old_to_new[edge] = new_to_old.size();
                  new_to_old.push_back(edge);
                  depth_first.push(nodes[edge].edges[1]);
                  depth_first.push(nodes[edge].edges[0]);
            }
      }
      reorder(old_to_new, new_to_old);
}

/** Renumber the nodes to make lookups use CPU and disk caches more effectively.
 *
 * First group the nodes into blocks so that each block contains the
 * root of a subtrie and as many levels of its descendants as will fit.
 * This way, after the root is paged in, the next few lookup
 * steps need not page in anything else.  Then, sort the nodes of each
 * block in depth-first order.  That should give each lookup almost
 * 1/2 chance to find the next node immediately adjacent.
 *
 * With a block size of 1024 bytes, this renumbering reduces the time
 * required for random lookups by about 1.1%, compared to a plain
 * depth-first order.  However, it's still 2.3% slower than the
 * database optimized by MaxMind.  */
void
binary_trie::reorder_in_blocks(
      std::size_t bytes_per_block)
{
      const edge_type none = -1;
      std::vector<edge_type> old_to_new, new_to_old;
      size_t bytes_left = bytes_per_block;
      old_to_new.resize(nodes.size(), none);
      new_to_old.reserve(nodes.size());
      for (edge_type subtrie = 0; subtrie < nodes.size(); ++subtrie) {
            // If subtrie has already been added to the output, ignore it.
            if (old_to_new[subtrie] != none)
                  continue;

            // Walk breadth-first from subtrie until we have a
            // block full of nodes or the subtrie runs out.
            // Don't add these nodes immediately to the output, however.
            // Instead just list them in nodes_in_block.
            std::set<edge_type> nodes_in_block;
            std::queue<edge_type> breadth_first;
            breadth_first.push(subtrie);
            if (bytes_left <= 0)
                  bytes_left += bytes_per_block;
            while (bytes_left > 0 && !breadth_first.empty()) {
                  edge_type edge = breadth_first.front();
                  breadth_first.pop();
                  if (edge >= nodes.size())
                        continue;

                  // Let the last node of the block straddle the
                  // block boundary.  That's better than making
                  // the hotter first node do so.
                  bytes_left -= 6;
                  nodes_in_block.insert(edge);

                  breadth_first.push(nodes[edge].edges[0]);
                  breadth_first.push(nodes[edge].edges[1]);
            }

            // Add the nodes from nodes_in_block to the output in depth-first order.
            // This assumes they are all reachable from subtrie.
            std::stack<edge_type> depth_first;
            depth_first.push(subtrie);
            while (!depth_first.empty()) {
                  edge_type edge = depth_first.top();
                  depth_first.pop();
                  if (nodes_in_block.find(edge)
                      == nodes_in_block.end())
                        continue;

                  old_to_new[edge] = new_to_old.size();
                  new_to_old.push_back(edge);

                  depth_first.push(nodes[edge].edges[1]);
                  depth_first.push(nodes[edge].edges[0]);
            }
      }
      reorder(old_to_new, new_to_old);
}

void
binary_trie::reorder(
      const std::vector<edge_type> &old_to_new,
      const std::vector<edge_type> &new_to_old)
{
      std::vector<node> new_nodes;
      new_nodes.reserve(new_to_old.size());
      for (std::vector<edge_type>::const_iterator
                 it = new_to_old.begin();
           it != new_to_old.end(); ++it) {
            node new_node;
            for (int bit = 0; bit <= 1; ++bit) {
                  edge_type old_edge = nodes[*it].edges[bit];
                  if (old_edge < nodes.size())
                        new_node.edges[bit] = old_to_new[old_edge];
                  else
                        new_node.edges[bit] = old_edge;
            }
            new_nodes.push_back(new_node);
      }
      swap(new_nodes, nodes);
}

/** Write the trie to a stream in GeoIP binary format. */
void
binary_trie::write_binary(std::ostream &dat_stream) const
{
      for (std::vector<node>::const_iterator it = nodes.begin();
           it != nodes.end(); ++it) {
            union {
                  uint8_t bytes[6];
                  char chars[6];
            } binary = {{
                  (it->edges[0]        ) & 0xFF,
                  (it->edges[0] >>  8) & 0xFF,
                  (it->edges[0] >> 16) & 0xFF,
                  (it->edges[1]        ) & 0xFF,
                  (it->edges[1] >>  8) & 0xFF,
                  (it->edges[1] >> 16) & 0xFF
            }};
            dat_stream.write(binary.chars, 6);
            if (dat_stream.bad())
                  return;
      }
}

namespace {
      void
      csv_line_to_vector(
            const std::string &line,
            std::vector<std::string> &fields)
      {
            fields.clear();
            std::vector<char> field;
            bool quoted = false;
            bool spaces_after_comma = false;
            for (std::string::const_iterator it = line.begin();
                 it != line.end(); ++it) {
                  if (*it == '"') {
                        quoted = !quoted;
                        spaces_after_comma = false;
                  } else if (*it == ',' && !quoted) {
                        fields.push_back(std::string(field.begin(), field.end()));
                        field.clear();
                        spaces_after_comma = true;
                  } else if (*it == ' ' && spaces_after_comma) {
                  } else {
                        field.push_back(*it);
                        spaces_after_comma = false;
                  }
            }
            fields.push_back(std::string(field.begin(), field.end()));
      }

      /** Load ranges of IP addresses from a CSV-formatted stream to a trie.
       *
       * \param trie
       * Load the ranges to this trie, overwriting original values.
       *
       * \param csv_file_name
       * The name of the file that \a csv_stream is reading.
       * This string is used only for error messages.
       *
       * \param csv_stream
       * Load the ranges from this stream.
       *
       * \param address_family
       * The type of IP addresses in the CSV data: either AF_INET
       * for IPv4 or AF_INET6 for IPv6.  */
      void
      csv_stream_to_trie(
            binary_trie &trie,
            const char *csv_file_name,
            std::istream &csv_stream,
            int address_family)
      {
            enum {
                  CSV_FIELD_MIN_TEXT,
                  CSV_FIELD_MAX_TEXT,
                  CSV_FIELD_MIN_DECIMAL,
                  CSV_FIELD_MAX_DECIMAL,
                  CSV_FIELD_COUNTRY_CODE,
                  CSV_FIELD_COUNTRY_NAME,
                  CSV_FIELDS
            };

            std::string csv_line;
            std::vector<std::string> csv_fields;
            int csv_line_number = 0;
            while (getline(csv_stream, csv_line)) {
                  ++csv_line_number;
                  csv_line_to_vector(csv_line, csv_fields);
                  if (csv_fields.size() != CSV_FIELDS) {
                  //    error_at_line(EX_DATAERR, 0, csv_file_name, csv_line_number,
                  //                "Wrong number of fields");
                  }

                  const int countryid = GeoIP_id_by_code(csv_fields[CSV_FIELD_COUNTRY_CODE].c_str());
                  if (countryid == 0) {
                  //    error_at_line(EX_DATAERR, 0, csv_file_name, csv_line_number,
                  //                "Unrecognized country code: %s",
                  //                csv_fields[CSV_FIELD_COUNTRY_CODE].c_str());
                  }
                  const binary_trie::edge_type leaf = 0xFFFF00 + countryid;

                  union {
                        struct in_addr inet;
                        uint8_t inetbytes[4];
                        struct in6_addr inet6;
                  } minaddr, maxaddr;
                  if (inet_pton(address_family, csv_fields[CSV_FIELD_MIN_TEXT].c_str(), &minaddr) <= 0) {
                  //    error_at_line(EX_DATAERR, 0, csv_file_name, csv_line_number,
                  //                "Cannot parse minimum address: %s",
                  //                csv_fields[CSV_FIELD_MIN_TEXT].c_str());
                  }
                  if (inet_pton(address_family, csv_fields[CSV_FIELD_MAX_TEXT].c_str(), &maxaddr) <= 0) {
                  //    error_at_line(EX_DATAERR, 0, csv_file_name, csv_line_number,
                  //                "Cannot parse maximum address: %s",
                  //                csv_fields[CSV_FIELD_MAX_TEXT].c_str());
                  }
                  switch (address_family) {
                  case AF_INET:
                        trie.set_range(minaddr.inetbytes, maxaddr.inetbytes,
                                     32, leaf);
                        break;
                  case AF_INET6:
                        trie.set_range(minaddr.inet6.s6_addr, maxaddr.inet6.s6_addr,
                                     128, leaf);
                        break;
                  default:
                        abort();
                  }
            }
            if (csv_stream.bad()) {
            //    error(EX_IOERR, errno, "%s", csv_file_name);
            }
      }

      /** Load ranges of IP addresses from a CSV-formatted file or
       * standard input to a trie.
       *
       * \param trie
       * Load the ranges to this trie, overwriting original values.
       *
       * \param csv_file_name
       * The name of the CSV file that should be read, or "-" for
       * standard input.
       *
       * \param address_family
       * The type of IP addresses in the CSV data: either AF_INET
       * for IPv4 or AF_INET6 for IPv6.  */
      void
      csv_file_to_trie(
            binary_trie &trie,
            const char *csv_file_name,
            int address_family)
      {
            if (std::strcmp(csv_file_name, "-") == 0) {
                  csv_stream_to_trie(trie, csv_file_name, std::cin, address_family);
            } else {
                  std::ifstream csv_stream(csv_file_name, std::ios::in);
                  if (!csv_stream) {
                  //    error(EX_NOINPUT, errno, "%s", csv_file_name);
                  }
                  csv_stream_to_trie(trie, csv_file_name, csv_stream, address_family);
            }
      }

      /** Write a GeoIP binary database to a stream.
       *
       * \param trie
       * Mapping from IP addresses to country codes or other values.
       *
       * \param dat_file_name
       * The name of the file that \a dat_stream is writing.
       * This string is used only for error messages.
       *
       * \param dat_stream
       * Write the database to this stream.
       *
       * \param database_info
       * Copyright or other information about the database, or NULL.
       * GeoIP_database_info() will return this string.
       *
       * \param address_family
       * The type of IP addresses in the database: either AF_INET
       * for IPv4 or AF_INET6 for IPv6.  */
      void
      write_dat_stream(
            const binary_trie &trie,
            const char *dat_file_name,
            std::ostream &dat_stream,
            const char *database_info,
            int address_family)
      {
            trie.write_binary(dat_stream);
            if (dat_stream.bad()) {
            //    error(EX_IOERR, errno, "%s", dat_file_name);
            }

            if (database_info) {
                  const char tag[3] = { 0, 0, 0 };
                  dat_stream.write(tag, 3);
                  dat_stream.write(database_info, std::strlen(database_info));
                  if (dat_stream.bad()) {
                  //    error(EX_IOERR, errno, "%s", dat_file_name);
                  }
            }

            switch (address_family) {
            case AF_INET: {
                  const char structure_info[4] = { 0xFF, 0xFF, 0xFF, 1 };
                  dat_stream.write(structure_info, 4);
                  break;
            }
            case AF_INET6: {
                  const char structure_info[4] = { 0xFF, 0xFF, 0xFF, 12 };
                  dat_stream.write(structure_info, 4);
                  break;
            }
            default:
                  abort();
            }
            if (dat_stream.bad()) {
            //    error(EX_IOERR, errno, "%s", dat_file_name);
            }
      }

      /** Write a GeoIP binary database to a file or standard output.
       *
       * \param trie
       * Mapping from IP addresses to country codes or other values.
       *
       * \param csv_file_name
       * The name of the file that should be written, or "-" for
       * standard output.
       *
       * \param database_info
       * Copyright or other information about the database, or NULL.
       * GeoIP_database_info() will return this string.
       *
       * \param address_family
       * The type of IP addresses in the database: either AF_INET
       * for IPv4 or AF_INET6 for IPv6.  */
      void
      write_dat_file(
            const binary_trie &trie,
            const char *dat_file_name,
            const char *database_info,
            int address_family)
      {
            if (std::strcmp(dat_file_name, "-") == 0) {
                  write_dat_stream(trie, dat_file_name, std::cout,
                               database_info, address_family);
            } else {
                  std::ofstream dat_stream(
                        dat_file_name,
                        std::ios::out | std::ios::binary);
                  if (!dat_stream) {
                  //    error(EX_CANTCREAT, errno, "%s", dat_file_name);
                  }
                  write_dat_stream(trie, dat_file_name, dat_stream,
                               database_info, address_family);
            }
      }

      struct cmdline {
            const char *csv_file_name;
            const char *dat_file_name;
            int address_family;
            const char *database_info;
            bool verbose;

            cmdline(int argc, char **argv);
      };
}

cmdline::cmdline(int argc, char **argv):
      csv_file_name("GeoIP.csv"),
      dat_file_name("GeoIP.dat"),
      address_family(AF_INET),
      database_info(NULL),
      verbose(false)
{
      enum {
            OPT_HELP = -2
      };

//    static const struct option long_options[] = {
//          { "inet", no_argument, NULL, '4' },
//          { "inet6", no_argument, NULL, '6' },
//          { "info", required_argument, NULL, 'i' },
//          { "output", required_argument, NULL, 'o' },
//          { "verbose", no_argument, NULL, 'v' },
//          { "help", no_argument, NULL, OPT_HELP },
//          { NULL, 0, NULL, 0 }
//    };
      static const char *const usage = "\
Usage: %s [OPTION] [CSV-FILE]...\n\
Convert a country database from CSV to GeoIP binary format.\n\
\n\
  -4, --inet            addresses are IPv4 (default)\n\
  -6, --inet6           addresses are IPv6\n\
  -i, --info=TEXT       add copyright or other info TEXT to output\n\
  -o, --output=FILE     write the binary data to FILE, not stdout\n\
  -v, --verbose         show what is going on\n\
      --help            display this help and exit\n";

      for (;;) {
      //    int optret = getopt_long(argc, argv, "46i:o:v", long_options, NULL);
            int optret = getopt(argc, argv, "46i:o:v");

            if (optret == -1)
                  break;
            switch (optret) {
            case '4':
                  address_family = AF_INET;
                  break;
            case '6':
                  address_family = AF_INET6;
                  break;
            case 'i':
                  database_info = optarg;
                  if (std::strlen(database_info) > 99) {
                  //    error(EX_USAGE, 0,
                  //          "Database info must not be longer than 99 bytes");
                  }
                  break;
            case 'o':
                  dat_file_name = optarg;
                  break;
            case 'v':
                  verbose = true;
                  break;
            case OPT_HELP:
            //    std::printf(usage, program_invocation_name);
                  std::exit(EX_OK);
            case '?':
            //    std::fprintf(stderr,
            //               "Try `%s --help' for more information.\n",
            //               program_invocation_name);
                  std::exit(EX_USAGE);
            default:
                  std::abort();
            }
      }

      if (optind < argc)
            csv_file_name = argv[optind++];

      if (optind < argc) {
      //    error(EX_USAGE, 0,
      //          "Only one non-option argument is allowed");
      }
}

int
main(int argc, char **argv)
{
      cmdline cmdline(argc, argv);

      std::ostream *verbose_stream;
      if (!cmdline.verbose)
            verbose_stream = NULL;
      else if (strcmp(cmdline.dat_file_name, "-") == 0)
            verbose_stream = &std::cerr;
      else
            verbose_stream = &std::cout;

      //if (verbose_stream) {
      //      *verbose_stream << program_invocation_name
      //                  << ": Reading CSV and building the trie"
      //                  << std::endl;
      //}
      binary_trie trie(0xFFFF00);
      csv_file_to_trie(trie, cmdline.csv_file_name, cmdline.address_family);

      //if (verbose_stream) {
      //      *verbose_stream << program_invocation_name
      //                  << ": Optimizing" << std::endl;
      //}
      trie.reorder_depth_first();
      trie.reorder_in_blocks(1024);

      //if (verbose_stream) {
      //      *verbose_stream << program_invocation_name
      //                  << ": Writing output" << std::endl;
      //}
      write_dat_file(trie, cmdline.dat_file_name, cmdline.database_info, cmdline.address_family);

      //if (verbose_stream) {
      //      *verbose_stream << program_invocation_name
      //                  << ": All done" << std::endl;
      //}
}
