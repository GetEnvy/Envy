(function(global) {
    'use strict';

    class ValidationError extends Error {
        constructor(code, message) {
            super(message);
            this.name = 'ValidationError';
            this.code = code;
        }
    }

    function getCrypto() {
        if (global.crypto && typeof global.crypto.getRandomValues === 'function') {
            return global.crypto;
        }

        if (typeof require === 'function') {
            const nodeCrypto = require('crypto');
            if (nodeCrypto.webcrypto && typeof nodeCrypto.webcrypto.getRandomValues === 'function') {
                return nodeCrypto.webcrypto;
            }
        }

        throw new Error('Secure crypto API is unavailable');
    }

    function toBase64Url(uint8Array) {
        let binary = '';
        for (let i = 0; i < uint8Array.length; i += 1) {
            binary += String.fromCharCode(uint8Array[i]);
        }

        const encoded = typeof btoa === 'function'
            ? btoa(binary)
            : Buffer.from(binary, 'binary').toString('base64');
        return encoded.replace(/\+/g, '-').replace(/\//g, '_').replace(/=+$/g, '');
    }

    function generateSecureToken(byteLength) {
        const length = Number.isInteger(byteLength) ? byteLength : 16;
        if (length < 16) {
            throw new ValidationError('TOKEN_LENGTH', 'Token byte length must be at least 16 bytes');
        }

        const bytes = new Uint8Array(length);
        getCrypto().getRandomValues(bytes);
        return toBase64Url(bytes);
    }

    function validateRedirectTarget(target, allowlist) {
        if (typeof target !== 'string') {
            return null;
        }

        const value = target.trim();
        if (!value || value.startsWith('//')) {
            return null;
        }

        if (/^(?:[a-z][a-z0-9+.-]*:)/i.test(value)) {
            return null;
        }

        if (!value.startsWith('/')) {
            return null;
        }

        const validPrefixes = allowlist || ['/remote', '/api'];
        const isAllowed = validPrefixes.some(prefix => value === prefix || value.startsWith(prefix + '/') || (prefix === '/' && value.startsWith('/')));

        return isAllowed ? value : null;
    }

    function validateApiSegment(segment, fieldName) {
        if (typeof segment !== 'string') {
            throw new ValidationError('INVALID_TYPE', `${fieldName} must be a string`);
        }

        if (!segment || segment.length > 64) {
            throw new ValidationError('INVALID_LENGTH', `${fieldName} length is out of bounds`);
        }

        if (!/^[A-Za-z0-9_-]+$/.test(segment)) {
            throw new ValidationError('INVALID_CHARS', `${fieldName} contains invalid characters`);
        }

        return segment;
    }


    function validateHttpMethod(method) {
        const normalized = String(method || '').toUpperCase();
        if (normalized !== 'GET' && normalized !== 'POST') {
            throw new ValidationError('INVALID_METHOD', 'Only GET/POST are supported');
        }
        return normalized;
    }

    function sanitizeHTML(html) {
        if (typeof html !== 'string') {
            return '';
        }

        if (global.DOMPurify && typeof global.DOMPurify.sanitize === 'function') {
            return global.DOMPurify.sanitize(html, {
                ALLOWED_TAGS: [
                    'div', 'span', 'p', 'br', 'hr', 'small', 'ul', 'ol', 'li', 'dl', 'dt', 'dd',
                    'strong', 'em', 'b', 'i', 'u', 'a', 'table', 'caption', 'thead', 'tbody', 'tfoot',
                    'tr', 'th', 'td', 'button', 'input', 'select', 'option', 'optgroup', 'textarea',
                    'form', 'label', 'fieldset', 'legend', 'section', 'article', 'header', 'footer',
                    'main', 'nav', 'aside', 'h1', 'h2', 'h3', 'h4', 'h5', 'h6'
                ],
                ALLOWED_ATTR: [
                    'class', 'id', 'href', 'title', 'target', 'rel',
                    'aria-label', 'aria-labelledby', 'aria-describedby', 'aria-current', 'aria-hidden',
                    'type', 'name', 'value', 'placeholder', 'checked', 'selected', 'disabled',
                    'readonly', 'required', 'multiple', 'for', 'role', 'method', 'action'
                ],
                ALLOW_DATA_ATTR: true
            });
        }

        return html
            .replace(/&/g, '&amp;')
            .replace(/</g, '&lt;')
            .replace(/>/g, '&gt;');
    }

    const api = {
        ValidationError,
        generateSecureToken,
        validateRedirectTarget,
        validateApiSegment,
        sanitizeHTML,
        validateHttpMethod
    };

    global.EnvySecurityUtils = api;

    if (typeof module !== 'undefined' && module.exports) {
        module.exports = api;
    }
})(typeof window !== 'undefined' ? window : globalThis);
