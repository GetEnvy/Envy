# Envy Remote API Specification

## Overview

The Envy Remote API provides RESTful endpoints for managing P2P downloads, uploads, searches, and system monitoring. This API is designed to support the modern web interface with JSON responses and proper HTTP status codes.

## Authentication

All API requests require authentication via session cookies or API tokens.

### Headers
```
Authorization: Bearer <token>
X-Session-ID: <session_id>
X-CSRF-Token: <csrf_token>
Content-Type: application/json
Accept: application/json
```

## Response Format

All responses follow this structure:

```json
{
    "success": true|false,
    "data": { ... } | null,
    "message": "Optional message",
    "error": "Error details if success=false",
    "timestamp": 1640995200000
}
```

## Endpoints

### Downloads

#### GET /api/downloads
List all downloads with optional filtering.

**Query Parameters:**
- `status` (string): Filter by status (active, paused, completed, error)
- `search` (string): Filter by filename
- `limit` (number): Maximum results (default: 50)
- `offset` (number): Pagination offset (default: 0)
- `sort` (string): Sort field (filename, size, progress, speed, status)
- `order` (string): Sort order (asc, desc)

**Response:**
```json
{
    "success": true,
    "data": {
        "downloads": [
            {
                "id": "download_123",
                "filename": "example.torrent",
                "size": 1073741824,
                "progress": 45.5,
                "speed": 2048000,
                "sources": 12,
                "totalSources": 15,
                "status": "active",
                "priority": "normal",
                "eta": 3600,
                "hash": "abc123...",
                "addedTime": 1640995200000,
                "completedTime": null
            }
        ],
        "total": 25,
        "filtered": 25,
        "stats": {
            "total_downloads": 25,
            "active_downloads": 3,
            "completed_downloads": 20,
            "paused_downloads": 2
        }
    }
}
```

#### POST /api/downloads
Start a new download.

**Request Body:**
```json
{
    "url": "magnet:?xt=urn:btih:...",
    "filename": "optional_custom_name.torrent",
    "priority": "normal"
}
```

**Response:**
```json
{
    "success": true,
    "data": {
        "id": "download_124",
        "status": "queued"
    },
    "message": "Download started successfully"
}
```

#### GET /api/downloads/{id}
Get details for a specific download.

**Response:**
```json
{
    "success": true,
    "data": {
        "id": "download_123",
        "filename": "example.torrent",
        "size": 1073741824,
        "progress": 45.5,
        "speed": 2048000,
        "sources": [
            {
                "ip": "192.168.1.100",
                "port": 6881,
                "speed": 512000,
                "status": "active"
            }
        ],
        "files": [
            {
                "name": "file1.mp4",
                "size": 536870912,
                "progress": 100
            }
        ],
        "status": "active",
        "priority": "normal",
        "hash": "abc123...",
        "addedTime": 1640995200000,
        "completedTime": null,
        "eta": 3600
    }
}
```

#### POST /api/downloads/{id}/start
Start a paused download.

**Response:**
```json
{
    "success": true,
    "message": "Download started"
}
```

#### POST /api/downloads/{id}/pause
Pause an active download.

**Response:**
```json
{
    "success": true,
    "message": "Download paused"
}
```

#### POST /api/downloads/{id}/cancel
Cancel and remove a download.

**Response:**
```json
{
    "success": true,
    "message": "Download cancelled"
}
```

#### PUT /api/downloads/{id}/priority
Change download priority.

**Request Body:**
```json
{
    "priority": "high"
}
```

**Response:**
```json
{
    "success": true,
    "message": "Priority updated"
}
```

#### DELETE /api/downloads/{id}
Remove a completed download from list.

**Response:**
```json
{
    "success": true,
    "message": "Download removed from list"
}
```

### Uploads

#### GET /api/uploads
List all uploads.

**Query Parameters:**
- `status` (string): Filter by status
- `limit` (number): Maximum results
- `offset` (number): Pagination offset

**Response:**
```json
{
    "success": true,
    "data": {
        "uploads": [
            {
                "id": "upload_456",
                "filename": "shared_file.mp4",
                "size": 536870912,
                "speed": 1024000,
                "peers": 5,
                "status": "active",
                "hash": "def456...",
                "uploaded": 268435456,
                "ratio": 0.5
            }
        ],
        "total": 10,
        "stats": {
            "total_uploads": 10,
            "active_uploads": 3
        }
    }
}
```

### Searches

#### GET /api/searches
List active searches.

**Response:**
```json
{
    "success": true,
    "data": {
        "searches": [
            {
                "id": "search_789",
                "query": "ubuntu iso",
                "status": "active",
                "results": 150,
                "startTime": 1640995200000,
                "endTime": null
            }
        ]
    }
}
```

#### POST /api/searches
Start a new search.

**Request Body:**
```json
{
    "query": "ubuntu iso",
    "fileType": "iso",
    "minSize": 1000000000,
    "maxSize": 5000000000
}
```

**Response:**
```json
{
    "success": true,
    "data": {
        "id": "search_790",
        "status": "started"
    }
}
```

#### GET /api/searches/{id}/results
Get search results.

**Query Parameters:**
- `limit` (number): Maximum results
- `offset` (number): Pagination offset
- `sort` (string): Sort field
- `order` (string): Sort order

**Response:**
```json
{
    "success": true,
    "data": {
        "results": [
            {
                "id": "result_001",
                "filename": "ubuntu-22.04-desktop-amd64.iso",
                "size": 3072000000,
                "type": "iso",
                "sources": 25,
                "speed": 2048000,
                "hash": "ghi789...",
                "rating": 4.5,
                "comments": 12
            }
        ],
        "total": 150
    }
}
```

#### POST /api/searches/{id}/download
Download from search result.

**Request Body:**
```json
{
    "resultId": "result_001"
}
```

### Networks

#### GET /api/networks
Get network status and connections.

**Response:**
```json
{
    "success": true,
    "data": {
        "networks": [
            {
                "name": "BitTorrent",
                "enabled": true,
                "connected": true,
                "peers": 45,
                "incoming": 1024000,
                "outgoing": 2048000,
                "connections": 12
            },
            {
                "name": "G2",
                "enabled": true,
                "connected": true,
                "peers": 23,
                "incoming": 512000,
                "outgoing": 1024000,
                "connections": 8
            }
        ],
        "stats": {
            "total_peers": 68,
            "total_incoming": 1536000,
            "total_outgoing": 3072000
        }
    }
}
```

#### POST /api/networks/{network}/connect
Connect to a network.

#### POST /api/networks/{network}/disconnect
Disconnect from a network.

### Statistics

#### GET /api/stats
Get system statistics.

**Response:**
```json
{
    "success": true,
    "data": {
        "downloads": {
            "active": 3,
            "completed": 125,
            "total": 128
        },
        "uploads": {
            "active": 5,
            "completed": 89,
            "total": 94
        },
        "network": {
            "connected_peers": 68,
            "total_incoming": 1536000,
            "total_outgoing": 3072000
        },
        "system": {
            "uptime": 86400000,
            "memory_usage": 256000000,
            "disk_usage": 1073741824
        }
    }
}
```

#### GET /api/recent-activity
Get recent system activity.

**Query Parameters:**
- `limit` (number): Maximum activities (default: 20)

**Response:**
```json
{
    "success": true,
    "data": {
        "activities": [
            {
                "id": "activity_001",
                "type": "download_started",
                "description": "Started downloading 'ubuntu.iso'",
                "timestamp": 1640995200000,
                "details": {
                    "filename": "ubuntu.iso",
                    "size": 3072000000
                }
            }
        ]
    }
}
```

### Session Management

#### POST /api/session/heartbeat
Keep session alive.

**Response:**
```json
{
    "success": true,
    "data": {
        "session_extended": true,
        "expires_in": 1800000
    }
}
```

#### GET /api/session/info
Get current session information.

**Response:**
```json
{
    "success": true,
    "data": {
        "session_id": "session_123",
        "user": "admin",
        "login_time": 1640991600000,
        "last_activity": 1640995200000,
        "expires_in": 1800000,
        "ip_address": "192.168.1.100"
    }
}
```

#### POST /api/session/logout
Logout current session.

**Response:**
```json
{
    "success": true,
    "message": "Logged out successfully"
}
```

### Progress Updates

#### GET /api/progress/{download_id}
Get progress for specific download.

**Response:**
```json
{
    "success": true,
    "data": {
        "id": "download_123",
        "progress": 67.8,
        "speed": 1536000,
        "eta": 2400,
        "status": "active"
    }
}
```

## Error Responses

### Authentication Error
```json
{
    "success": false,
    "error": "Authentication required",
    "code": "AUTH_REQUIRED",
    "timestamp": 1640995200000
}
```

### Validation Error
```json
{
    "success": false,
    "error": "Invalid input",
    "code": "VALIDATION_ERROR",
    "details": {
        "url": "Invalid URL format"
    },
    "timestamp": 1640995200000
}
```

### Rate Limit Exceeded
```json
{
    "success": false,
    "error": "Rate limit exceeded",
    "code": "RATE_LIMIT",
    "retry_after": 60,
    "timestamp": 1640995200000
}
```

### Server Error
```json
{
    "success": false,
    "error": "Internal server error",
    "code": "SERVER_ERROR",
    "timestamp": 1640995200000
}
```

## HTTP Status Codes

- `200 OK` - Successful request
- `201 Created` - Resource created
- `400 Bad Request` - Invalid request data
- `401 Unauthorized` - Authentication required
- `403 Forbidden` - Access denied
- `404 Not Found` - Resource not found
- `409 Conflict` - Resource conflict
- `422 Unprocessable Entity` - Validation failed
- `429 Too Many Requests` - Rate limit exceeded
- `500 Internal Server Error` - Server error

## Rate Limiting

- General API: 100 requests per 15 minutes
- Downloads API: 50 requests per 15 minutes
- Searches API: 30 requests per 15 minutes

Rate limit headers are included in responses:
```
X-RateLimit-Limit: 100
X-RateLimit-Remaining: 95
X-RateLimit-Reset: 1640996100000
```

## Pagination

For endpoints that return lists, use these parameters:
- `limit`: Maximum items per page (default: 50, max: 200)
- `offset`: Number of items to skip (default: 0)

Response includes pagination metadata:
```json
{
    "data": [...],
    "pagination": {
        "total": 150,
        "limit": 50,
        "offset": 0,
        "has_more": true
    }
}
```

## Versioning

API versioning is handled via URL path:
- Current version: `/api/v1/` (or just `/api/` for latest)
- Future versions: `/api/v2/`, `/api/v3/`, etc.

## Content Types

- Request: `application/json`
- Response: `application/json`
- File uploads: `multipart/form-data`

## CORS

Cross-Origin Resource Sharing is configured for web interface origins only.

## WebSocket Support (Future)

Real-time updates via WebSocket:
```
ws://localhost:8080/api/ws
wss://envy.example.com/api/ws
```

Events:
- `download_progress`
- `network_status`
- `search_results`
- `system_notifications`

## Implementation Notes

### Backend Requirements
1. JSON response formatting
2. Proper HTTP status codes
3. Input validation and sanitization
4. CSRF protection
5. Session management
6. Rate limiting
7. Error handling

### Client Integration
```javascript
// Example API call
EnvyRemote.ajaxRequest('/api/downloads', {
    method: 'GET',
    headers: {
        'Accept': 'application/json'
    }
})
.then(response => {
    if (response.success) {
        // Handle success
        console.log(response.data);
    } else {
        // Handle error
        console.error(response.error);
    }
})
.catch(error => {
    console.error('Request failed:', error);
});
```

### Error Handling
Always check the `success` field first, then handle accordingly:

```javascript
function handleApiResponse(response) {
    if (response.success) {
        // Process data
        processData(response.data);
    } else {
        // Handle error
        showError(response.error, response.code);
    }
}
```

This API specification provides a complete RESTful interface for the modern Envy remote control system.
