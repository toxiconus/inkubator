const WS_PORT = 81;
const WS_PROTOCOL = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
const WS_HOST = window.location.hostname || window.location.host;

function getWebSocketUrl(path = '/') {
  if (!path.startsWith('/')) {
    path = '/' + path;
  }
  return `${WS_PROTOCOL}//${WS_HOST}:${WS_PORT}${path}`;
}

function createWebSocket(path = '/') {
  return new WebSocket(getWebSocketUrl(path));
}

function fetchJson(url, options = {}, timeoutMs = 5000) {
  const controller = new AbortController();
  const timer = setTimeout(() => controller.abort(), timeoutMs);
  return fetch(url, { ...options, signal: controller.signal })
    .then(response => {
      clearTimeout(timer);
      return response.json();
    });
}

function setText(id, value) {
  const el = document.getElementById(id);
  if (el) el.textContent = value;
}

function $(id) {
  return document.getElementById(id);
}

function addClass(id, className) {
  const el = $(id);
  if (el) el.classList.add(className);
}

function removeClass(id, className) {
  const el = $(id);
  if (el) el.classList.remove(className);
}
