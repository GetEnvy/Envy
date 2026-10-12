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

  test('validateRedirectTarget rejects traversal and encoded separator variants', () => {
    const invalidTargets = [
      '/remote/../outside',
      '/remote/./home',
      '/remote/%2e%2e/outside',
      '/remote/%2E%2E/outside',
      '/remote/%252e%252e/outside',
      '/remote/%2f%2e%2e%2foutside',
      '/remote\\..\\outside',
      '/remote/%5c..%5coutside',
      '/remote/%252f%252e%252e%252foutside'
    ];

    for (const target of invalidTargets) {
      expect(utils.validateRedirectTarget(target, ['/remote'])).toBeNull();
    }
  });

  test('validateRedirectTarget preserves valid allowlisted path boundaries', () => {
    expect(utils.validateRedirectTarget('/remote', ['/remote'])).toBe('/remote');
    expect(utils.validateRedirectTarget('/remote/home', ['/remote'])).toBe('/remote/home');
    expect(utils.validateRedirectTarget('/remote/%25complete', ['/remote'])).toBe('/remote/%25complete');
    expect(utils.validateRedirectTarget('/remote/search?q=../outside#fragment/..', ['/remote'])).toBe('/remote/search?q=../outside#fragment/..');
    expect(utils.validateRedirectTarget('/remote/search?q=%FF&literal=%#fragment', ['/remote'])).toBe('/remote/search?q=%FF&literal=%#fragment');
    expect(utils.validateRedirectTarget('/remoteish/home', ['/remote'])).toBeNull();
    expect(utils.validateRedirectTarget('/api/v1/items', ['/remote', '/api'])).toBe('/api/v1/items');
  });

  test('validateRedirectTarget rejects traversal normalized across query and fragment boundaries', () => {
    const invalidTargets = [
      '/remote/..?/outside',
      '/remote/..#/outside',
      '/remote/%2e%2e?/outside',
      '/remote/%2e%2e#/outside'
    ];

    for (const target of invalidTargets) {
      expect(new URL(target, 'http://localhost/').pathname).toBe('/');
      expect(utils.validateRedirectTarget(target, ['/remote'])).toBeNull();
    }
  });

  test('validateRedirectTarget rejects URL parser control-character differentials', () => {
    const rawTargets = [
      '/remote/.\t./outside',
      '/remote/.\r./outside',
      '/remote/.\n./outside'
    ];

    for (const target of rawTargets) {
      expect(new URL(target, 'http://localhost/').pathname).toBe('/outside');
      expect(utils.validateRedirectTarget(target, ['/remote'])).toBeNull();
    }

    const encodedTargets = [
      '/remote/.%09./outside',
      '/remote/.%0A./outside',
      '/remote/.%0d./outside',
      '/remote/%00outside',
      '/remote/%1foutside'
    ];

    for (const target of encodedTargets) {
      expect(utils.validateRedirectTarget(target, ['/remote'])).toBeNull();
    }
  });

  test('validateRedirectTarget rejects malformed encodings', () => {
    for (const target of ['/remote/%', '/remote/%2', '/remote/%E0%A4%A']) {
      expect(utils.validateRedirectTarget(target, ['/remote'])).toBeNull();
    }
  });

  test('validateRedirectTarget fails closed at recursive decoding depth limits', () => {
    const encodedTraversal = depth => {
      let value = '%2e%2e';
      for (let i = 1; i < depth; i += 1) {
        value = encodeURIComponent(value);
      }
      return `/remote/${value}/outside`;
    };

    expect(utils.validateRedirectTarget(encodedTraversal(11), ['/remote'])).toBeNull();
    expect(utils.validateRedirectTarget(encodedTraversal(12), ['/remote'])).toBeNull();
    expect(utils.validateRedirectTarget('/remote/%25complete', ['/remote'])).toBe('/remote/%25complete');
    expect(utils.validateRedirectTarget('/remote/search?q=../outside#fragment/..', ['/remote'])).toBe('/remote/search?q=../outside#fragment/..');
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

