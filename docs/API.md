# API Documentation

## Scope
This repository contains remote-control API documentation in `Remote/api-specification.md`. That file appears to describe intended/target JSON API behavior for the remote UI and should be treated as a design-level spec unless verified against server-side handlers.

## Authentication Model (documented)
Expected headers include:
- `Authorization: Bearer <token>`
- `X-Session-ID: <session_id>`
- `X-CSRF-Token: <csrf_token>`

## Documented Endpoint Families
- `/api/downloads`
- `/api/uploads`
- `/api/searches`
- `/api/system` (in spec document)

## Response Contract (documented)
Spec uses a common envelope:
```json
{
  "success": true,
  "data": {},
  "message": "optional",
  "error": null,
  "timestamp": 0
}
```

## Verification Status
- This repository pass did **not** execute end-to-end API integration tests against a running Envy instance.
- Before relying on this API in external tooling, validate each endpoint against live behavior and update this document with confirmed contracts.

## Source of Truth
- `Remote/api-specification.md` (current design/spec reference)
