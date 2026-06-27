const { JSDOM } = require('jsdom');
const createDOMPurify = require('dompurify');
const utils = require('../security-utils.js');

describe('security-utils', () => {
  test('generateSecureToken creates base64url token with >=128 bits entropy', () => {
    const token = utils.generateSecureToken(16);
    expect(token).toMatch(/^[A-Za-z0-9_-]+$/);
    expect(token.length).toBeGreaterThanOrEqual(22);
  });

  test('generateSecureToken produces unique values across N calls', () => {
    const count = 200;
    const tokens = new Set(Array.from({ length: count }, () => utils.generateSecureToken(16)));
    expect(tokens.size).toBe(count);
  });

  test('validateRedirectTarget allows only internal allowlisted paths', () => {
    expect(utils.validateRedirectTarget('/remote/home', ['/remote'])).toBe('/remote/home');
    expect(utils.validateRedirectTarget('//evil.com', ['/remote'])).toBeNull();
    expect(utils.validateRedirectTarget('javascript:alert(1)', ['/remote'])).toBeNull();
    expect(utils.validateRedirectTarget('data:text/html,1', ['/remote'])).toBeNull();
    expect(utils.validateRedirectTarget('https://evil.com', ['/remote'])).toBeNull();
  });

  test('validateApiSegment rejects invalid boundary values', () => {
    expect(() => utils.validateApiSegment('', 'stat')).toThrow('length is out of bounds');
    expect(() => utils.validateApiSegment('../etc/passwd', 'stat')).toThrow('invalid characters');
    expect(() => utils.validateApiSegment('valid_stat-1', 'stat')).not.toThrow();
  });

  test('sanitizeHTML strips script execution payloads', () => {
    const dom = new JSDOM('<!DOCTYPE html><html><body></body></html>');
    global.window = dom.window;
    global.DOMPurify = createDOMPurify(dom.window);

    const payload = '<img src=x onerror=alert(1)><script>alert(1)</script><div>ok</div>';
    const sanitized = utils.sanitizeHTML(payload);

    expect(sanitized).not.toContain('onerror');
    expect(sanitized).not.toContain('<script');
    expect(sanitized).toContain('<div>ok</div>');

    delete global.window;
    delete global.DOMPurify;
  });
});
