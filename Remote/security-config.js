/**
 * Envy Remote Security Configuration
 * Version 2026 - Enhanced Security Features
 *
 * This file contains security settings and utilities for the Envy remote interface.
 * It implements modern security practices including CSRF protection, secure headers,
 * session management, and input validation.
 */

(function() {
    'use strict';

    // Security Configuration
    const SECURITY_CONFIG = {
        // CSRF Protection
        csrf: {
            enabled: true,
            tokenLength: 32,
            tokenLifetime: 3600000, // 1 hour
            headerName: 'X-CSRF-Token',
            cookieName: 'envy_csrf_token'
        },

        // Session Management
        session: {
            timeout: 1800000, // 30 minutes
            maxConcurrentSessions: 3,
            rememberMeMaxAge: 604800000, // 7 days
            secureCookies: true,
            httpOnlyCookies: true,
            sameSiteCookies: 'strict'
        },

        // Content Security Policy
        csp: {
            enabled: true,
            defaultSrc: ["'self'"],
            scriptSrc: ["'self'"],
            styleSrc: ["'self'"],
            imgSrc: ["'self'", "data:", "https:"],
            fontSrc: ["'self'", "https:"],
            connectSrc: ["'self'"],
            frameSrc: ["'none'"],
            objectSrc: ["'none'"]
        },

        // HTTPS Enforcement
        https: {
            enabled: true,
            hstsMaxAge: 31536000, // 1 year
            hstsIncludeSubdomains: true,
            hstsPreload: false
        },

        // Rate Limiting
        rateLimit: {
            enabled: true,
            windowMs: 900000, // 15 minutes
            maxRequests: 100, // per window
            skipSuccessfulRequests: false,
            skipFailedRequests: false
        },

        // Input Validation
        validation: {
            maxFileNameLength: 255,
            maxPathLength: 4096,
            allowedFileTypes: ['torrent', 'magnet', 'http', 'https', 'ftp'],
            sanitizeHtml: true
        },

        // Security Headers
        headers: {
            'X-Frame-Options': 'DENY',
            'X-Content-Type-Options': 'nosniff',
            'X-XSS-Protection': '1; mode=block',
            'Referrer-Policy': 'strict-origin-when-cross-origin',
            'Permissions-Policy': 'geolocation=(), microphone=(), camera=()'
        }
    };

    // CSRF Protection Class
    class CSRFProtection {
        constructor(config) {
            this.config = config.csrf;
            this.token = null;
            this.tokenExpiry = null;
        }

        /**
         * Generate a new CSRF token
         */
        generateToken() {
            this.token = window.EnvySecurityUtils
                ? window.EnvySecurityUtils.generateSecureToken(this.config.tokenLength)
                : this.generateFallbackToken();
            this.tokenExpiry = Date.now() + this.config.tokenLifetime;
            this.storeToken();
            return this.token;
        }

        generateFallbackToken() {
            const array = new Uint8Array(this.config.tokenLength);
            crypto.getRandomValues(array);
            return Array.from(array, byte => byte.toString(16).padStart(2, '0')).join('');
        }

        /**
         * Validate a CSRF token
         */
        validateToken(token) {
            if (!this.token || !token) return false;

            // Check if token matches
            if (token !== this.token) return false;

            // Check if token has expired
            if (Date.now() > this.tokenExpiry) {
                this.clearToken();
                return false;
            }

            return true;
        }

        /**
         * Get current token or generate new one
         */
        getToken() {
            if (!this.token || Date.now() > this.tokenExpiry) {
                return this.generateToken();
            }
            return this.token;
        }

        /**
         * Store token in cookie
         */
        storeToken() {
            if (typeof document !== 'undefined') {
                const cookieOptions = `path=/; max-age=${this.config.tokenLifetime / 1000}; SameSite=${SECURITY_CONFIG.session.sameSiteCookies}`;
                document.cookie = `${this.config.cookieName}=${this.token}; ${cookieOptions}`;
            }
        }

        /**
         * Clear stored token
         */
        clearToken() {
            this.token = null;
            this.tokenExpiry = null;
            if (typeof document !== 'undefined') {
                document.cookie = `${this.config.cookieName}=; path=/; max-age=0`;
            }
        }

        /**
         * Add CSRF token to form
         */
        addToForm(form) {
            const token = this.getToken();
            let csrfInput = form.querySelector('input[name="csrf_token"]');

            if (!csrfInput) {
                csrfInput = document.createElement('input');
                csrfInput.type = 'hidden';
                csrfInput.name = 'csrf_token';
                form.appendChild(csrfInput);
            }

            csrfInput.value = token;
        }
    }

    // Session Management Class
    class SessionManager {
        constructor(config) {
            this.config = config.session;
            this.sessionId = null;
            this.lastActivity = Date.now();
            this.heartbeatInterval = null;
        }

        /**
         * Initialize session
         */
        initialize() {
            this.sessionId = this.getStoredSessionId() || this.generateSessionId();
            this.storeSessionId();
            this.startHeartbeat();
            this.setupActivityTracking();
        }

        /**
         * Generate new session ID
         */
        generateSessionId() {
            const array = new Uint8Array(16);
            crypto.getRandomValues(array);
            return Array.from(array, byte => byte.toString(16).padStart(2, '0')).join('');
        }

        /**
         * Get stored session ID
         */
        getStoredSessionId() {
            const cookieName = 'envy_session_id';
            const cookies = document.cookie.split(';');
            for (let cookie of cookies) {
                const [name, value] = cookie.trim().split('=');
                if (name === cookieName) {
                    return value;
                }
            }
            return null;
        }

        /**
         * Store session ID
         */
        storeSessionId() {
            const cookieOptions = `path=/; max-age=${this.config.timeout / 1000}; SameSite=${this.config.sameSiteCookies}`;
            document.cookie = `envy_session_id=${this.sessionId}; ${cookieOptions}`;
        }

        /**
         * Start heartbeat to keep session alive
         */
        startHeartbeat() {
            this.heartbeatInterval = setInterval(() => {
                this.sendHeartbeat();
            }, 60000); // Every minute
        }

        /**
         * Send heartbeat to server
         */
        sendHeartbeat() {
            if (window.EnvyRemote) {
                EnvyRemote.ajaxRequest('/api/session/heartbeat', {
                    method: 'POST',
                    headers: {
                        'X-Session-ID': this.sessionId
                    }
                }).catch(error => {
                    console.warn('Heartbeat failed:', error);
                });
            }
        }

        /**
         * Setup activity tracking
         */
        setupActivityTracking() {
            const events = ['mousedown', 'mousemove', 'keypress', 'scroll', 'touchstart'];
            const updateActivity = () => {
                this.lastActivity = Date.now();
            };

            events.forEach(event => {
                document.addEventListener(event, updateActivity, { passive: true });
            });

            // Check for inactivity every minute
            setInterval(() => {
                const inactiveTime = Date.now() - this.lastActivity;
                if (inactiveTime > this.config.timeout) {
                    this.handleSessionTimeout();
                }
            }, 60000);
        }

        /**
         * Handle session timeout
         */
        handleSessionTimeout() {
            if (window.EnvyRemote) {
                EnvyRemote.showNotification('Session expired. Please login again.', 'error');
                setTimeout(() => {
                    window.location.href = '/remote/login';
                }, 3000);
            }
        }

        /**
         * Logout and clear session
         */
        logout() {
            if (this.heartbeatInterval) {
                clearInterval(this.heartbeatInterval);
            }

            // Clear session cookie
            document.cookie = 'envy_session_id=; path=/; max-age=0';

            // Clear CSRF token
            if (window.csrfProtection) {
                window.csrfProtection.clearToken();
            }

            // Redirect to login
            window.location.href = '/remote/logout';
        }

        /**
         * Get session info
         */
        getSessionInfo() {
            return {
                sessionId: this.sessionId,
                lastActivity: this.lastActivity,
                timeRemaining: this.config.timeout - (Date.now() - this.lastActivity)
            };
        }
    }

    // Input Validation Class
    class InputValidator {
        constructor(config) {
            this.config = config.validation;
        }

        /**
         * Validate filename
         */
        validateFileName(filename) {
            if (!filename || typeof filename !== 'string') return false;
            if (filename.length > this.config.maxFileNameLength) return false;
            if (filename.includes('..') || filename.includes('/') || filename.includes('\\')) return false;

            // Check for dangerous characters
            const dangerousChars = /[<>\"\'\|\?\*\x00-\x1f]/;
            return !dangerousChars.test(filename);
        }

        /**
         * Validate URL
         */
        validateUrl(url) {
            if (!url || typeof url !== 'string') return false;

            try {
                const parsedUrl = new URL(url);

                // Check protocol
                if (!this.config.allowedFileTypes.includes(parsedUrl.protocol.replace(':', ''))) {
                    return false;
                }

                // Check for localhost/private IPs (basic protection)
                const hostname = parsedUrl.hostname;
                if (hostname === 'localhost' || hostname === '127.0.0.1' ||
                    hostname.startsWith('192.168.') || hostname.startsWith('10.') ||
                    hostname.startsWith('172.')) {
                    return false;
                }

                return true;
            } catch (e) {
                return false;
            }
        }

        /**
         * Sanitize HTML input
         */
        sanitizeHtml(input) {
            if (!this.config.sanitizeHtml || !input) return input;

            // Basic HTML sanitization
            return input
                .replace(/</g, '&lt;')
                .replace(/>/g, '&gt;')
                .replace(/"/g, '&quot;')
                .replace(/'/g, '&#x27;')
                .replace(/\//g, '&#x2F;');
        }

        /**
         * Validate search query
         */
        validateSearchQuery(query) {
            if (!query || typeof query !== 'string') return false;
            if (query.length > 500) return false; // Reasonable limit

            // Check for SQL injection patterns (basic)
            const sqlPatterns = /(\bUNION\b|\bSELECT\b|\bINSERT\b|\bDELETE\b|\bUPDATE\b|\bDROP\b)/i;
            return !sqlPatterns.test(query);
        }
    }

    // Security Headers Helper
    class SecurityHeaders {
        constructor(config) {
            this.config = config;
        }

        /**
         * Set security headers
         */
        setHeaders() {
            if (typeof document === 'undefined') return;

            // Set meta tags for additional security
            const metaTags = [
                { name: 'referrer', content: 'strict-origin-when-cross-origin' },
                { 'http-equiv': 'X-UA-Compatible', content: 'IE=edge' }
            ];

            metaTags.forEach(tag => {
                const meta = document.createElement('meta');
                Object.keys(tag).forEach(key => {
                    meta.setAttribute(key, tag[key]);
                });
                document.head.appendChild(meta);
            });
        }

        /**
         * Generate CSP header
         */
        generateCSP() {
            if (!this.config.csp.enabled) return '';

            const csp = this.config.csp;
            const directives = [];

            if (csp.defaultSrc) directives.push(`default-src ${csp.defaultSrc.join(' ')}`);
            if (csp.scriptSrc) directives.push(`script-src ${csp.scriptSrc.join(' ')}`);
            if (csp.styleSrc) directives.push(`style-src ${csp.styleSrc.join(' ')}`);
            if (csp.imgSrc) directives.push(`img-src ${csp.imgSrc.join(' ')}`);
            if (csp.connectSrc) directives.push(`connect-src ${csp.connectSrc.join(' ')}`);
            if (csp.frameSrc) directives.push(`frame-src ${csp.frameSrc.join(' ')}`);
            if (csp.objectSrc) directives.push(`object-src ${csp.objectSrc.join(' ')}`);

            return directives.join('; ');
        }
    }

    // Rate Limiting Class
    class RateLimiter {
        constructor(config) {
            this.config = config.rateLimit;
            this.requests = [];
        }

        /**
         * Check if request should be allowed
         */
        checkLimit() {
            if (!this.config.enabled) return true;

            const now = Date.now();
            const windowStart = now - this.config.windowMs;

            // Remove old requests
            this.requests = this.requests.filter(time => time > windowStart);

            // Check if under limit
            if (this.requests.length >= this.config.maxRequests) {
                return false;
            }

            // Add current request
            this.requests.push(now);
            return true;
        }

        /**
         * Get remaining requests in current window
         */
        getRemainingRequests() {
            const now = Date.now();
            const windowStart = now - this.config.windowMs;
            const recentRequests = this.requests.filter(time => time > windowStart);
            return Math.max(0, this.config.maxRequests - recentRequests.length);
        }

        /**
         * Get time until reset
         */
        getTimeUntilReset() {
            if (this.requests.length === 0) return 0;
            const oldestRequest = Math.min(...this.requests);
            const resetTime = oldestRequest + this.config.windowMs;
            return Math.max(0, resetTime - Date.now());
        }
    }

    // Initialize Security Components
    const csrfProtection = new CSRFProtection(SECURITY_CONFIG);
    const sessionManager = new SessionManager(SECURITY_CONFIG);
    const inputValidator = new InputValidator(SECURITY_CONFIG);
    const securityHeaders = new SecurityHeaders(SECURITY_CONFIG);
    const rateLimiter = new RateLimiter(SECURITY_CONFIG);

    // Global Security API
    window.EnvySecurity = {
        // CSRF Protection
        getCSRFToken: () => csrfProtection.getToken(),
        validateCSRFToken: (token) => csrfProtection.validateToken(token),

        // Session Management
        initializeSession: () => sessionManager.initialize(),
        logout: () => sessionManager.logout(),
        getSessionInfo: () => sessionManager.getSessionInfo(),

        // Input Validation
        validateFileName: (name) => inputValidator.validateFileName(name),
        validateUrl: (url) => inputValidator.validateUrl(url),
        validateSearchQuery: (query) => inputValidator.validateSearchQuery(query),
        sanitizeHtml: (input) => inputValidator.sanitizeHtml(input),

        // Rate Limiting
        checkRateLimit: () => rateLimiter.checkLimit(),
        getRemainingRequests: () => rateLimiter.getRemainingRequests(),
        getTimeUntilReset: () => rateLimiter.getTimeUntilReset(),

        // Security Headers
        generateCSP: () => securityHeaders.generateCSP(),
        setSecurityHeaders: () => securityHeaders.setHeaders(),

        // Configuration
        config: SECURITY_CONFIG
    };

    // Auto-initialize on page load
    if (typeof document !== 'undefined') {
        document.addEventListener('DOMContentLoaded', function() {
            window.EnvySecurity.initializeSession();
            window.EnvySecurity.setSecurityHeaders();
        });
    }

})();
