const WS_PORT = 81;
const WS_PROTOCOL = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
const WS_HOST = window.location.hostname || window.location.host;

function getWebSocketUrl(path = '/') {
  if (!path.startsWith('/')) {
    path = '/' + path;
  }
  return `${WS_PROTOCOL}//${WS_HOST}:${WS_PORT}${path}`;
}

function createWebSocket(path = '/', protocols) {
  return new WebSocket(getWebSocketUrl(path), protocols);
}

function isWebSocketSecure() {
  return WS_PROTOCOL === 'wss:';
}
