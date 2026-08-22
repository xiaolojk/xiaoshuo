/* TShock Web 控制台 前端逻辑 */
const $ = (id) => document.getElementById(id);

/* ---------- 日志 ---------- */
function log(text, cls = 'ok') {
  const el = $('log');
  const line = document.createElement('div');
  line.className = 'line ' + cls;
  line.textContent = text;
  el.appendChild(line);
  el.scrollTop = el.scrollHeight;
}
function clearLog() { $('log').innerHTML = ''; }

/* ---------- 面板折叠 ---------- */
function toggle(id) { $(id).classList.toggle('collapsed'); }

/* ---------- 连接 ---------- */
async function connect() {
  const restUrl = $('restUrl').value.trim();
  const authMode = $('authMode').value;
  const appToken = $('appKey').value.trim();
  const username = $('acctUser').value.trim();
  const password = $('acctPass').value;

  setState('connecting', '连接中…');
  try {
    const r = await fetch('/api/connect', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ restUrl, authMode, appToken, username, password }),
    });
    const d = await r.json();
    if (!d.ok) { throw new Error(d.error || '连接失败'); }
    if (d.demo) {
      $('connState').textContent = '演示模式';
      setState('on', '演示模式');
      $('connectMsg').textContent = '已进入演示模式（未填 REST 地址），可直接体验界面。';
    } else {
      setState('on', '已连接：' + (d.status && d.status.name || restUrl));
      $('connectMsg').textContent = '连接成功';
    }
    renderStatus(d.status);
    await loadPlayers();
  } catch (e) {
    setState('err', '连接失败');
    $('connectMsg').textContent = String(e.message || e);
    log('连接失败：' + String(e.message || e), 'err');
  }
}
function setState(kind, label) {
  const dot = $('connDot');
  dot.className = 'dot ' + (kind === 'on' ? 'on' : kind === 'err' ? 'err' : '');
  $('connState').textContent = label || '';
}
$('btnConnect').addEventListener('click', connect);
$('btnDemo').addEventListener('click', () => { $('restUrl').value = ''; connect(); });
$('authMode').addEventListener('change', () => {
  const isAcct = $('authMode').value === 'account';
  $('accountWrap').style.display = isAcct ? 'flex' : 'none';
  $('appKeyWrap').style.display = isAcct ? 'none' : 'block';
});

/* ---------- 状态 ---------- */
function fmtUptime(sec) {
  if (!sec && sec !== 0) return '-';
  if (typeof sec === 'string') return sec;
  const d = Math.floor(sec / 86400), h = Math.floor(sec % 86400 / 3600), m = Math.floor(sec % 3600 / 60);
  return (d ? d + '天 ' : '') + h + '时' + m + '分';
}
function renderStatus(s) {
  if (!s) return;
  const pick = (obj, keys, fallback) => {
    for (const k of keys) if (obj[k] !== undefined && obj[k] !== null) return obj[k];
    return fallback;
  };
  $('stPlayers').textContent = pick(s, ['playercount', 'players'], '-');
  $('stMax').textContent = pick(s, ['maxplayers', 'maxPlayers'], '-');
  $('stWorld').textContent = pick(s, ['world', 'worldname'], '-');
  $('stVer').textContent = pick(s, ['serverversion', 'version', 'tsversion'], '-');
  $('stUptime').textContent = fmtUptime(pick(s, ['uptime', 'uptime_seconds'], null));
  $('stPort').textContent = pick(s, ['port'], '-');
}
async function loadStatus() {
  try {
    const r = await fetch('/api/status');
    const d = await r.json().catch(() => ({}));
    renderStatus(d);
    if (r.status >= 400) { log('获取状态失败 HTTP ' + r.status, 'err'); }
  } catch (e) { log('获取状态异常：' + e.message, 'err'); }
}

/* ---------- 玩家 ---------- */
async function loadPlayers() {
  try {
    const r = await fetch('/api/players');
    const d = await r.json();
    const arr = Array.isArray(d) ? d : (d.items || d.players || []);
    $('playerCount').textContent = arr.length;
    const box = $('playerList');
    if (!arr.length) { box.innerHTML = '<div class="empty">暂无在线玩家</div>'; return; }
    box.innerHTML = '';
    arr.forEach(pl => {
      const name = pl.nickname || pl.name || pl.username || '?';
      const active = pl.active !== false;
      const row = document.createElement('div');
      row.className = 'player';
      row.innerHTML =
        '<span class="p-name">' + esc(name) +
          '<span class="p-state ' + (active ? 'active' : '') + '">' + esc(pl.state || (active ? 'Playing' : '离线')) + '</span>' +
        '</span>' +
        '<span class="p-actions">' +
          '<button class="chip" data-cmd="/kick ' + escAttr(name) + ' ">踢出</button>' +
          '<button class="chip" data-cmd="/mute ' + escAttr(name) + ' ">禁言</button>' +
          '<button class="chip" data-cmd="/unmute ' + escAttr(name) + ' ">解禁言</button>' +
          '<button class="chip" data-cmd="/ban ' + escAttr(name) + ' ">封禁</button>' +
        '</span>';
      box.appendChild(row);
    });
    box.querySelectorAll('.chip').forEach(btn => {
      btn.addEventListener('click', () => {
        const cmd = btn.dataset.cmd;
        const reason = prompt('附加理由（可留空）：', '');
        send({ cmd: cmd + (reason || '') });
      });
    });
  } catch (e) { log('玩家列表异常：' + e.message, 'err'); }
}
function esc(s) { return String(s).replace(/[&<>"']/g, c => ({ '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;' }[c])); }
function escAttr(s) { return esc(s); }

/* ---------- 指令 ---------- */
async function send(payload, label) {
  try {
    const r = await fetch('/api/cmd', { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify(payload) });
    const d = await r.json().catch(() => ({}));
    const out = d.response !== undefined ? (typeof d.response === 'object' ? JSON.stringify(d.response) : d.response) : d;
    if (typeof out === 'string') out.split('\n').forEach(l => log(l.trim(), 'srv'));
    else if (out && out.error) log('错误：' + out.error, 'err');
    else log('（无输出）', 'srv');
    if (r.status >= 400 && !(d && d.response)) log('指令失败 HTTP ' + r.status, 'err');
  } catch (e) { log('指令异常：' + e.message, 'err'); }
}
function runCmd() {
  const input = $('cmdInput');
  const cmd = input.value.trim();
  if (!cmd) return;
  log('> ' + cmd, 'cmd');
  input.value = '';
  send({ cmd });
}
$('cmdInput').addEventListener('keydown', e => { if (e.key === 'Enter') runCmd(); });

function doBroadcast() {
  const msg = $('bcMsg').value.trim();
  if (!msg) { $('bcMsg').focus(); return; }
  log('广播：' + msg, 'cmd');
  sendBc(msg);
  $('bcMsg').value = '';
}
async function sendBc(msg) {
  try {
    const r = await fetch('/api/broadcast', { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify({ msg }) });
    const d = await r.json().catch(() => ({}));
    log((d && d.response) ? String(d.response) : '已广播', 'srv');
  } catch (e) { log('广播异常：' + e.message, 'err'); }
}

async function doAction(a) {
  const labels = { save: '保存世界', reload: '重载配置', restart: '重启服务器', off: '停服' };
  if (a === 'off' && !confirm('确定要停服吗？')) return;
  if (a === 'restart' && !confirm('确定要重启服务器吗？')) return;
  log('> ' + labels[a], 'cmd');
  try {
    const r = await fetch('/api/action?do=' + encodeURIComponent(a), { method: 'POST' });
    const d = await r.json().catch(() => ({}));
    const out = d.response !== undefined ? (typeof d.response === 'object' ? JSON.stringify(d.response) : d.response) : '';
    log(['', out || '（无输出）'].filter(Boolean).join(' '), 'srv');
  } catch (e) { log(labels[a] + ' 失败：' + e.message, 'err'); }
}

/* 自动连接（若本地保存过配置） */
(function boot() {
  log('欢迎使用 TShock 服务器控制台。');
  log('请在「连接设置」中填入你的 REST 地址，或点「演示模式」预览。\n', 'srv');
})();