/*
 * TShock REST 网页控制台 —— 零依赖 Node 代理后端
 *
 * 作用：
 *   1) 提供静态网页控制台（public/）
 *   2) 把浏览器对 /api/* 的请求代理到用户自己的 tShock REST API
 *      规避浏览器跨域(CORS)限制，并代为持有 REST token。
 *
 * 连接方式（连接面板里二选一）：
 *   - 应用密钥：填 tshock.json 里 Rest.ApplicationRestTokens 的 key，
 *     key 可直接作为 ?token= 使用，无需额外登录。
 *   - 账号密码：用具有 tshock.admin.restapi 权限的 tshock 账号，
 *     通过 /v2/token/create 换取 token。
 *
 * 启动：node server.js   （默认端口 4321，可用 PORT 环境变量覆盖）
 */
'use strict';
const http = require('http');
const fs = require('fs');
const path = require('path');

const PORT = process.env.PORT || 4321;
const PUBLIC = path.join(__dirname, 'public');
const DATADIR = path.join(__dirname, 'data');

// 单一控制目标，运行期内存持有，不落盘
let cfg = { restUrl: '', authMode: 'apikey', appToken: '', username: '', password: '', token: '', demo: false };

const MIME = { '.html': 'text/html; charset=utf-8', '.js': 'application/javascript; charset=utf-8', '.css': 'text/css; charset=utf-8', '.svg': 'image/svg+xml', '.png': 'image/png', '.json': 'application/json; charset=utf-8' };

const json = (res, code, obj) => { res.writeHead(code, { 'Content-Type': 'application/json; charset=utf-8' }); res.end(JSON.stringify(obj)); };
const readBody = (req) => new Promise((r) => { let d = ''; req.on('data', (c) => (d += c)); req.on('end', () => r(d)); });

// —— 直连 tshock REST ——
async function tshockReq(apiPath, { method = 'GET', body } = {}) {
  const token = cfg.authMode === 'account' ? cfg.token : (cfg.token || cfg.appToken || '');
  const base = cfg.restUrl.replace(/\/+$/, '');
  const sep = apiPath.includes('?') ? '&' : '?';
  const full = `${base}${apiPath}${sep}token=${encodeURIComponent(token)}`;
  const res = await fetch(full, {
    method,
    headers: body ? { 'Content-Type': 'application/json' } : undefined,
    body: body ? JSON.stringify(body) : undefined,
    signal: AbortSignal.timeout(8000),
  });
  const text = await res.text();
  let data; try { data = JSON.parse(text); } catch { data = text; }
  return { http: res.status, data };
}

async function obtainToken() {
  const base = cfg.restUrl.replace(/\/+$/, '');
  const full = `${base}/v2/token/create?username=${encodeURIComponent(cfg.username)}&password=${encodeURIComponent(cfg.password)}`;
  const res = await fetch(full, { method: 'POST', signal: AbortSignal.timeout(8000) });
  const text = await res.text();
  let data; try { data = JSON.parse(text); } catch { data = text; }
  if (data && data.token) return data.token;
  const msg = (typeof data === 'object' && (data.error || data.status)) || text || res.status;
  throw new Error('登录失败：' + msg);
}

// —— 演示模式数据（无真实服务器时预览界面用）——
function demoStatus() {
  return {
    status: '200', name: '演示 Terraria 服务器', version: 'TShock 6.1.0',
    port: 7777, playercount: 0, maxplayers: 16, uptime: 60 * 47,
    world: '岛屿之梦.wld', serverversion: '1.4.5.6', worldsize: '2560x768', time: '正午(演示)',
  };
}
function demoPlayers() {
  return [
    { nickname: '林舟', group: 'superadmin', active: true, state: 'Playing' },
    { nickname: '烬火', group: 'superadmin', active: true, state: 'Playing' },
    { nickname: '異客', group: 'guest', active: false, state: 'Inactive' },
  ];
}

// —— 路由 ——
async function handleApi(req, res, p, u) {
  try {
    if (p === '/api/connect' && req.method === 'POST') {
      const b = JSON.parse(await readBody(req) || '{}');
      cfg.restUrl = (b.restUrl || '').trim();
      cfg.authMode = b.authMode === 'account' ? 'account' : 'apikey';
      cfg.appToken = (b.appToken || '').trim();
      cfg.username = b.username || '';
      cfg.password = b.password || '';
      cfg.token = ''; cfg.demo = false;
      if (!cfg.restUrl) {
        cfg.demo = true;   // 未填地址 → 演示模式
        return json(res, 200, { ok: true, demo: true, status: demoStatus() });
      }
      if (cfg.authMode === 'account') cfg.token = await obtainToken();
      const st = await tshockReq('/v2/server/status');
      if (st.http >= 400) json(res, 502, { ok: false, error: 'REST 连接失败：HTTP ' + st.http + ' ' + JSON.stringify(st.data) });
      else json(res, 200, { ok: true, demo: false, status: st.data });
      return;
    }

    // demo 优先
    if (cfg.demo) {
      if (p === '/api/status') return json(res, 200, demoStatus());
      if (p === '/api/players') return json(res, 200, demoPlayers());
      if (p === '/api/cmd') { json(res, 200, { response: '(演示模式) 执行指令：' + (JSON.parse(await readBody(req) || '{}').cmd || '') }); return; }
      if (p === '/api/broadcast') { json(res, 200, { response: '已在演示模式广播' }); return; }
      if (p === '/api/action') { json(res, 200, { response: '(演示模式) 已执行 ' + (u.searchParams.get('do') || '') }); return; }
    }

    switch (p) {
      case '/api/status': { const r = await tshockReq('/v2/server/status'); json(res, r.http, r.data); break; }
      case '/api/players': { const r = await tshockReq('/v2/players/list'); json(res, r.http, r.data); break; }
      case '/api/cmd': {
        if (req.method !== 'POST') { json(res, 405, { error: '需 POST' }); break; }
        const { cmd } = JSON.parse(await readBody(req) || '{}');
        const r = await tshockReq('/v3/server/rawcmd', { method: 'POST', body: { cmd } });
        json(res, r.http, { ok: r.http < 400, response: (typeof r.data === 'object' ? (r.data.response || r.data) : r.data), http: r.http });
        break;
      }
      case '/api/broadcast': {
        if (req.method !== 'POST') { json(res, 405, { error: '需 POST' }); break; }
        const { msg } = JSON.parse(await readBody(req) || '{}');
        const r = await tshockReq('/v2/server/broadcast', { method: 'POST', body: { msg } });
        json(res, r.http, { ok: r.http < 400, response: (typeof r.data === 'object' ? (r.data.response || r.data) : r.data), http: r.http });
        break;
      }
      case '/api/action': {
        if (req.method !== 'POST') { json(res, 405, { error: '需 POST' }); break; }
        const doWhat = u.searchParams.get('do');
        const map = {
          reload: () => tshockReq('/v3/server/reload', { method: 'POST' }),
          save: () => tshockReq('/v2/world/save', { method: 'POST' }),
          off: () => tshockReq('/v2/server/off', { method: 'POST' }),
          restart: () => tshockReq('/v3/server/rawcmd', { method: 'POST', body: { cmd: '/restart' } }),
        };
        const r = await map[doWhat]();
        json(res, r.http, { ok: r.http < 400, response: (typeof r.data === 'object' ? (r.data.response || r.data) : r.data), http: r.http });
        break;
      }
      default: json(res, 404, { error: 'Not Found' });
    }
  } catch (e) {
    const msg = e && e.message ? (String(e.message).slice(0, 300)) : String(e);
    json(res, 502, { error: msg });
  }
}

const server = http.createServer(async (req, res) => {
  const u = new URL(req.url, 'http://local');
  const p = decodeURIComponent(u.pathname);
  if (p.startsWith('/api/')) return handleApi(req, res, p, u);

  let fp = path.normalize(path.join(PUBLIC, p === '/' || p === '' ? 'index.html' : p));
  if (!fp.startsWith(PUBLIC)) { res.writeHead(403); return res.end('Forbidden'); }
  try {
    const c = fs.readFileSync(fp);
    res.writeHead(200, { 'Content-Type': MIME[path.extname(fp).toLowerCase()] || 'application/octet-stream' });
    res.end(c);
  } catch {
    res.writeHead(404, { 'Content-Type': 'text/plain; charset=utf-8' });
    res.end('Not Found');
  }
});

server.listen(PORT, '0.0.0.0', () => {
  console.log('TShock Web 控制台已启动:  http://localhost:' + PORT);
  console.log('使用演示模式预览，或在界面填入你自己的 tshock REST 地址连接。');
});