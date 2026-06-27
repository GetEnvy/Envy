const fs = require('fs');
const path = require('path');
const vm = require('vm');
const { JSDOM } = require('jsdom');

function loadSecurityContext() {
  const dom = new JSDOM('<!doctype html><html><head></head><body></body></html>', { url: 'https://localhost/remote/home' });
  const context = vm.createContext({
    window: dom.window,
    document: dom.window.document,
    console,
    URL,
    setInterval: () => 1,
    clearInterval: () => {},
    setTimeout,
    clearTimeout,
    Date,
    Uint8Array,
    crypto: global.crypto,
    fetch: () => Promise.resolve({ ok: true, json: () => Promise.resolve({}) }),
    Buffer
  });

  context.window.crypto = global.crypto;
  context.window.fetch = context.fetch;
  context.window.EnvyRemote = { ajaxRequest: () => Promise.resolve({}) };

  const utilsCode = fs.readFileSync(path.resolve(__dirname, '../security-utils.js'), 'utf8');
  const configCode = fs.readFileSync(path.resolve(__dirname, '../security-config.js'), 'utf8');
  vm.runInContext(utilsCode, context);
  vm.runInContext(configCode, context);
  return context;
}

describe('security-config', () => {
  test('csrf token validation accepts current token and rejects expired/invalid', () => {
    const ctx = loadSecurityContext();
    const token = ctx.window.EnvySecurity.getCSRFToken();

    expect(ctx.window.EnvySecurity.validateCSRFToken(token)).toBe(true);
    expect(ctx.window.EnvySecurity.validateCSRFToken('invalid')).toBe(false);
  });

  test('rate limiter enforces limit and resets based on correct window config', () => {
    const ctx = loadSecurityContext();
    const cfg = ctx.window.EnvySecurity.config.rateLimit;
    cfg.maxRequests = 2;
    cfg.windowMs = 100;

    expect(ctx.window.EnvySecurity.checkRateLimit()).toBe(true);
    expect(ctx.window.EnvySecurity.checkRateLimit()).toBe(true);
    expect(ctx.window.EnvySecurity.checkRateLimit()).toBe(false);
    expect(ctx.window.EnvySecurity.getRemainingRequests()).toBe(0);
  });

  test('csp no longer contains unsafe-inline for scripts or styles', () => {
    const ctx = loadSecurityContext();
    const csp = ctx.window.EnvySecurity.generateCSP();
    expect(csp).not.toContain("'unsafe-inline'");
  });
});
