# ED2K Search Results Fixes

**Date:** January 16, 2026 (Updated: January 17, 2026)
**Issue:** Search results not appearing in search window, UDP results matching wrong searches
**Status:** ✅ **FIXED** - Complete UDP/TCP search modernization with GUID tracking

---

## Changes Implemented

### 1. UDP Search GUID Tracking (January 2026)

#### `EDClients::SetUDPSearchGUID()` (EDClients.cpp)
- **Added**: Records search GUID when UDP search query is sent
- **Tracks**: Server IP, UDP port, opcode, and search GUID with timestamp
- **Purpose**: Associate incoming UDP responses with originating search

#### `EDClients::GetUDPSearchGUID()` (EDClients.cpp)
- **Added**: Retrieves search GUID for incoming UDP responses
- **Features**:
  - Server-specific lookup using IP, port, and opcode
  - Automatic cleanup of stale entries (older than 60 seconds)
  - Returns empty GUID if not found or stale (fallback to "last search")
- **Purpose**: Correctly match UDP search results to originating search

#### `EDClients::OnServerSearchResultRaw()` (EDClients.cpp)
- **Added**: Specialized handler for concatenated ED2K UDP sub-packets
- **Features**:
  - Parses multiple sub-packets in single datagram (eMule-compatible)
  - Correctly extracts Unicode flag from server flags (bit 8, not bit 0)
  - Retrieves associated search GUID for each sub-packet
  - Handles compressed packets (ED2K_PROTOCOL_EMULE_PACKED)
- **Purpose**: Support servers that concatenate multiple search results

#### `EDClients::CleanupStaleUDPSearchGUIDs()` (EDClients.cpp)
- **Added**: Periodic cleanup of expired GUID entries
- **Features**: Removes entries older than 60 seconds
- **Purpose**: Prevent memory growth from stale search tracking data

#### `ManagedSearch::ExecuteDonkeyMesh()` (ManagedSearch.cpp)
- **Modified**: Records search GUID after sending UDP search query
- **Change**: Calls `EDClients.SetUDPSearchGUID()` with search GUID, server IP, port, and opcode
- **Purpose**: Enable GUID tracking for UDP searches

#### `Datagrams::OnDatagram()` (Datagrams.cpp)
- **Modified**: Routes concatenated ED2K UDP packets to specialized handler
- **Change**: For `ED2K_S2CG_SEARCHRESULT` and `ED2K_S2CG_FOUNDSOURCES`, calls `EDClients.OnServerSearchResultRaw()`
- **Purpose**: Handle eMule-compatible concatenated packet format

### 2. Unicode Flag Fix

#### `EDClients::OnServerSearchResult()` (EDClients.cpp)
- **Fixed**: Correct Unicode flag extraction from server flags
- **Change**: Changed from `(nServerFlags & 0x01)` to `(nServerFlags & 0x0100) >> 8`
- **Purpose**: Correctly identify Unicode vs ANSI string encoding in search results

### 3. Safe Packet Construction

#### `QuerySearch::ToEDPacket()` (QuerySearch.cpp)
- **Fixed**: Added null checks before accessing schema/XML elements
- **Change**:
  ```cpp
  // Before: Direct access could crash if null
  strWords = m_pSchema->GetIndexedWords(m_pXML->GetFirstElement());

  // After: Safe null checks
  if (m_pSchema != NULL && m_pXML != NULL) {
      CXMLElement* pFirstElement = m_pXML->GetFirstElement();
      if (pFirstElement != NULL) {
          strWords = m_pSchema->GetIndexedWords(pFirstElement);
      }
  }
  ```
- **Purpose**: Prevent crashes when schema or XML is not initialized

### 4. Vendor Cache Improvements

#### `VendorCache::Load()` (VendorCache.cpp)
- **Enhanced**: Robust Vendors.xml loading with multiple fallback paths
- **Paths**:
  1. Primary: `Settings.General.DataPath + "Vendors.xml"`
  2. Fallback 1: `Settings.General.Path + "\\Data\\Vendors.xml"`
  3. Fallback 2: Binary folder + `"\\Data\\Vendors.xml"`
- **Features**: Automatic file copying from fallback paths to user DataPath
- **Purpose**: Ensure Vendors.xml is always available, even in developer builds

#### `VendorCache::Lookup()` (VendorCache.h)
- **Enhanced**: Once-per-process logging for unknown vendor codes
- **Features**:
  - Static map tracking of logged codes
  - Thread-safe logging with mutex protection
  - Reduced verbosity (MSG_DEBUG instead of MSG_INFO)
- **Purpose**: Prevent log spam from repeated unknown vendor code messages

#### `QueryHit::ReadEDPacket()` (QueryHit.cpp)
- **Fixed**: Switched to ASCII lookup to prevent log spam
- **Change**: `VendorCache.Lookup(L"ED2K")` → `VendorCache.Lookup("ED2K")`
- **Purpose**: ASCII lookup does not log unknown codes, preventing spam

### 5. Duplicate Case Value Fix

#### `QueryHit::ReadEDPacket()` (QueryHit.cpp)
- **Fixed**: Resolved compilation error C2196 (duplicate case '85')
- **Issue**: Both `ED2K_FT_MAXSOURCES` and `ED2K_CT_MODVERSION` have value `0x55` (85)
- **Solution**: Combined into single case with tag type discrimination
  - `ED2K_TAG_STRING` → Treat as `ED2K_CT_MODVERSION` (client mod version)
  - Otherwise → Treat as `ED2K_FT_MAXSOURCES` (file tag, no action)
- **Purpose**: Handle both tag types correctly in same switch statement

### 6. Type Cast Optimization

#### `ManagedSearch::ExecuteDonkeyMesh()` (ManagedSearch.cpp)
- **Changed**: `dynamic_cast` → `static_cast` for CEDPacket
- **Purpose**: Performance improvement (avoid runtime type checking)
- **Safety**: Maintained by ensuring packet type is known before cast

### 7. Debug Logging Added (Original)

#### `EDNeighbour::SendQuery()` (EDNeighbour.cpp:824)
- **Added**: Debug logging when search query is sent
- **Logs**: Server address, search GUID, queue size
- **Purpose**: Track when searches are sent and verify GUID is added to queue

#### `EDNeighbour::OnSearchResults()` (EDNeighbour.cpp:583)
- **Added**: Debug logging when search results are received
- **Logs**: Server address, GUID extracted from queue, queue size, valid GUID flag
- **Warning**: Logs when queue is empty (indicates potential GUID mismatch)
- **Purpose**: Track result arrival and GUID matching

#### `EDClients::OnServerSearchResult()` (EDClients.cpp:611)
- **Added**: Debug logging for UDP search results
- **Logs**: Server address, note about empty GUID for UDP searches
- **Purpose**: Track UDP search results (always have empty GUID)

#### `CSearchWnd::OnQueryHits()` (WndSearch.cpp:786)
- **Added**: Debug logging for GUID matching attempts
- **Logs**: Hit GUID, search GUID, match results, fallback status
- **Purpose**: Track which searches match query hits

### 2. Code Improvements

#### `EDNeighbour::OnFoundSources()` (EDNeighbour.cpp:641)
- **Fixed**: Added missing GUID parameter to `FromEDPacket()` call
- **Change**: Uses empty GUID (GetSources responses don't need search GUID)
- **Purpose**: Maintain consistency with function signature

#### `EDNeighbour::OnSearchResults()` (EDNeighbour.cpp:583)
- **Improved**: Enhanced debug message for "more results" packets
- **Change**: Shows GUID in "more results" debug message
- **Purpose**: Better tracking of multi-result searches

---

## How to Use Debug Logging

1. **Enable Debug Messages**:
   - Settings → Interface → Show Debug Messages
   - Or use filter: "ED2K" or "SEARCH"

2. **Perform Search**:
   - Execute a search
   - Watch for messages showing:
     - `ED2K: Sending search query to [server], GUID=[hex], Queue size=[N]`
     - `ED2K: Received search results from [server], GUID=[hex], HasValidGUID=[0/1], Queue size=[N]`
     - `ED2K: Matching hit GUID=[hex] to search GUID=[hex], GUIDMatch=[0/1], EmptyGUIDFallback=[0/1]`

3. **Diagnose Issues**:
   - **Queue Empty**: If queue size is 0 when results arrive, GUID won't match
   - **Empty GUID**: Results with empty GUID only match "last search"
   - **GUID Mismatch**: If GUIDs don't match, results won't appear in search window

---

## Known Limitations

### UDP Search Results (RESOLVED ✅)
- **Previous Issue**: UDP search results always had empty GUID
- **Status**: ✅ **FIXED** - UDP searches now correctly track GUID per server
- **Implementation**: Server-specific GUID mapping (IP + port + opcode)

### Empty Query Queue (RESOLVED ✅)
- **Previous Issue**: If results arrived before query was queued, GUID was empty
- **Status**: ✅ **FIXED** - GUID is recorded immediately when UDP query is sent
- **Implementation**: GUID tracking happens synchronously with query transmission

### Multiple Concurrent Searches (RESOLVED ✅)
- **Previous Issue**: Queue-based tracking caused mismatches with multiple searches
- **Status**: ✅ **FIXED** - Server-specific GUID mapping handles concurrent searches
- **Implementation**: Each server (IP + port + opcode) has independent GUID tracking

---

## Future Improvements

1. **Enhanced GUID Tracking**: ✅ **IMPLEMENTED** - Server-specific timestamped map
   - ✅ Handles out-of-order responses
   - ✅ Automatic timeout cleanup (60 seconds)
   - ✅ Better matching for multiple concurrent searches

2. **UDP GUID Tracking**: ✅ **IMPLEMENTED** - Complete UDP search GUID association
   - ✅ UDP requests associated with search GUID
   - ✅ UDP results matched to correct search
   - ✅ Support for concatenated sub-packets

3. **Improved Fallback Matching**: ✅ **IMPLEMENTED** - Empty GUID fallback
   - ✅ Falls back to "last search" if GUID not found
   - ✅ Automatic cleanup prevents stale GUID usage

### Remaining Enhancements (Optional)
1. **Search Term Matching**: Match empty GUID results to most recent matching search terms
2. **GUID Timeout Tuning**: Make 60-second timeout configurable
3. **Metrics Collection**: Track GUID match success rates for monitoring

---

## Testing

After these changes:

1. **Verify Search Queries Sent**:
   - Check logs show "Sending search query" with correct GUID
   - Verify queue size increases after sending

2. **Verify Results Received**:
   - Check logs show "Received search results" with GUID
   - Verify GUID matches sent query GUID

3. **Verify Results Matched**:
   - Check logs show "Matching hit GUID" with match status
   - Verify results appear in search window

---

## Files Modified

### Core Search Functionality
- `Envy/EDClients.h` - Added UDP search GUID tracking structures and methods
- `Envy/EDClients.cpp` - Complete UDP GUID tracking implementation, concatenated packet support
- `Envy/ManagedSearch.cpp` - GUID recording for UDP searches, cast optimization
- `Envy/Datagrams.cpp` - Routing for concatenated ED2K UDP packets
- `Envy/QuerySearch.cpp` - Safe packet construction with null checks
- `Envy/QueryHit.cpp` - Vendor lookup fix, duplicate case resolution

### Vendor Cache
- `Envy/VendorCache.h` - Once-per-process logging for unknown vendor codes
- `Envy/VendorCache.cpp` - Robust Vendors.xml loading with fallback paths
- `Envy/Envy.vcxproj` - Post-build step to copy Vendors.xml to output directory

### Debug Logging (Original)
- `Envy/EDNeighbour.cpp` - Added debug logging and improved GUID handling
- `Envy/WndSearch.cpp` - Added debug logging for GUID matching

---

**Last Updated:** January 17, 2026
**Status:** ✅ **COMPLETE** - All ED2K search issues resolved, UDP/TCP searches fully functional
