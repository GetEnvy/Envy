# Remote Security Fix Checklist (2026-04-22)

- `Remote/envy-modern.js:134,172` → Validate redirect targets against internal allowlist before navigation/fallback.
- `Remote/envy-modern.js:238,282,323,560` → Remove unsafe DOM sinks (`innerHTML`) or sanitize via DOMPurify allowlist.
- `Remote/envy-modern.js:652` → Replace `Math.random()` CSRF generation with `crypto.getRandomValues()` base64url tokens and rotate safely.
- `Remote/envy-modern.js:686-696` → Validate dynamic API params and response shape with typed validation errors.
- `Remote/security-config.js:38-39` → Remove `'unsafe-inline'` from CSP directives.
- `Remote/security-config.js:466-470` → Fix rate limiter config path references to `this.config.windowMs` / `this.config.maxRequests`.
- `Remote/head-modern.html`, `Remote/tail-modern.html`, `Remote/login-modern.html`, `Remote/home-modern.html` → Remove inline handlers/scripts/styles and wire external JS/CSS compatible with strict CSP.
- `Remote/tests/*` → Add Jest regression tests for CSRF, sanitization, redirects, rate limiter, input validation.
