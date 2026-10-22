/**
 * ws-utils.js – WebSocket utilities for Inkubator HMI
 * ESP32-S3 · WebSocket client with reconnect and event bus
 */

(function() {
    'use strict';

    // ============================================================
    // KONFIGURACJA
    // ============================================================
    const CONFIG = {
        wsPort: 81,
        reconnectBaseDelay: 3000,
        maxReconnectDelay: 30000,
        maxRetries: 10,
        debug: false
    };

    // ============================================================
    // STAN
    // ============================================================
    let ws = null;
    let reconnectTimer = null;
    let retryCount = 0;
    let isConnecting = false;
    let connectionState = 'disconnected'; // 'connected' | 'connecting' | 'disconnected'

    // ============================================================
    // PUBLICZNE FUNKCJE
    // ============================================================

    function createWebSocket(path) {
        if (ws && (ws.readyState === WebSocket.OPEN || ws.readyState === WebSocket.CONNECTING)) {
            return ws;
        }

        if (isConnecting) {
            return ws;
        }

        isConnecting = true;
        setConnectionState('connecting');

        try {
            const url = getWebSocketUrl(path || '/');
            if (CONFIG.debug) console.log('[WS] Łączenie z', url);

            ws = new WebSocket(url);

            ws.onopen = function() {
                isConnecting = false;
                retryCount = 0;
                setConnectionState('connected');
                if (CONFIG.debug) console.log('[WS] Połączono');
                emit('ws.open', { ws: ws });
                emit('connection.changed', { state: 'connected' });
            };

            ws.onmessage = function(event) {
                try {
                    const parsed = parseMessage(event.data);
                    emit('ws.message', { raw: event.data, parsed: parsed });
                    if (parsed && parsed.type === 'status') {
                        emit('status.update', parsed.payload || parsed);
                    } else if (parsed && parsed.payload) {
                        emit('status.update', parsed.payload);
                    }
                } catch (e) {
                    if (CONFIG.debug) console.warn('[WS] Błąd parsowania:', e);
                }
            };

            ws.onclose = function() {
                isConnecting = false;
                setConnectionState('disconnected');
                if (CONFIG.debug) console.warn('[WS] Rozłączono');
                emit('ws.close', {});
                emit('connection.changed', { state: 'disconnected' });
                scheduleReconnect(path);
            };

            ws.onerror = function(error) {
                if (CONFIG.debug) console.error('[WS] Błąd:', error);
                emit('ws.error', { error: error });
            };

        } catch (e) {
            isConnecting = false;
            setConnectionState('disconnected');
            if (CONFIG.debug) console.error('[WS] Błąd tworzenia:', e);
            scheduleReconnect(path);
        }

        return ws;
    }

    function closeWebSocket() {
        clearTimeout(reconnectTimer);
        reconnectTimer = null;
        retryCount = 0;
        if (ws) {
            try { ws.close(); } catch (e) {}
            ws = null;
        }
        setConnectionState('disconnected');
    }

    function sendCommand(command, data) {
        if (!ws || ws.readyState !== WebSocket.OPEN) {
            if (CONFIG.debug) console.warn('[WS] Brak połączenia');
            return false;
        }

        let payload;
        if (typeof command === 'string') {
            payload = data ? { command: command, data: data } : { command: command };
        } else {
            payload = command;
        }

        try {
            ws.send(typeof payload === 'string' ? payload : JSON.stringify(payload));
            emit('ws.send', payload);
            return true;
        } catch (e) {
            if (CONFIG.debug) console.error('[WS] Błąd wysyłania:', e);
            return false;
        }
    }

    function isConnected() {
        return ws && ws.readyState === WebSocket.OPEN;
    }

    function getConnectionState() {
        return connectionState;
    }

    // ============================================================
    // POMOCNICZE
    // ============================================================

    function getWebSocketUrl(path) {
        const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
        const host = getHost();
        return protocol + '//' + host + ':' + CONFIG.wsPort + path;
    }

    function getHost() {
        const query = new URLSearchParams(window.location.search);
        const queryHost = query.get('espHost');
        if (queryHost) return queryHost;

        try {
            const saved = localStorage.getItem('inkubator.wsHost');
            if (saved) return saved;
        } catch (e) {}

        const host = window.location.hostname || window.location.host;
        if (host && host !== 'localhost' && host !== '127.0.0.1' && host !== '') {
            return host;
        }

        return '192.168.5.1';
    }

    function scheduleReconnect(path) {
        clearTimeout(reconnectTimer);
        retryCount++;

        if (retryCount > CONFIG.maxRetries) {
            if (CONFIG.debug) console.warn('[WS] Przekroczono limit prób');
            emit('ws.maxRetries', { retries: retryCount });
            setConnectionState('disconnected');
            return;
        }

        const delay = Math.min(CONFIG.maxReconnectDelay, CONFIG.reconnectBaseDelay * retryCount);
        if (CONFIG.debug) console.log('[WS] Próba ' + retryCount + '/' + CONFIG.maxRetries + ' za ' + (delay/1000) + 's');

        reconnectTimer = setTimeout(function() {
            createWebSocket(path);
        }, delay);
    }

    function parseMessage(message) {
        if (typeof message !== 'string') {
            return message;
        }

        try {
            return JSON.parse(message);
        } catch (e) {
            return parseKeyValue(message);
        }
    }

    function parseKeyValue(text) {
        const result = { type: 'status', payload: {} };
        const lines = text.split('\n');
        for (const line of lines) {
            const eq = line.indexOf('=');
            if (eq > 0) {
                const key = line.substring(0, eq).trim();
                const val = line.substring(eq + 1).trim();
                const num = parseFloat(val);
                result.payload[key] = isNaN(num) ? val : num;
            }
        }
        return result;
    }

    function setConnectionState(state) {
        connectionState = state;

        const dot = document.getElementById('wsDot');
        const status = document.getElementById('wsStatus');

        if (dot) {
            dot.className = 'dot';
            if (state === 'connected') dot.classList.add('ok');
            else if (state === 'connecting') dot.classList.add('loading');
            else dot.classList.add('err');
        }

        if (status) {
            if (state === 'connected') {
                status.textContent = 'WS ON';
                status.style.color = '#30c060';
            } else if (state === 'connecting') {
                status.textContent = 'WS...';
                status.style.color = '#e8b830';
            } else {
                status.textContent = 'WS OFF';
                status.style.color = '#e05050';
            }
        }
    }

    const listeners = {};

    function emit(event, data) {
        const list = listeners[event];
        if (!list) return;
        for (const cb of list) {
            try { cb(data); } catch (e) { if (CONFIG.debug) console.warn('[WS] Event error:', e); }
        }
    }

    function subscribe(event, callback) {
        if (!listeners[event]) listeners[event] = [];
        listeners[event].push(callback);
        return function() { unsubscribe(event, callback); };
    }

    function unsubscribe(event, callback) {
        if (!listeners[event]) return;
        listeners[event] = listeners[event].filter(cb => cb !== callback);
    }

    window.createWebSocket = createWebSocket;
    window.closeWebSocket = closeWebSocket;
    window.sendCommand = sendCommand;
    window.isWsConnected = isConnected;
    window.getWsState = getConnectionState;
    window.wsSubscribe = subscribe;
    window.wsEmit = emit;

    window.Inkubator = window.Inkubator || {};
    window.Inkubator.WS = {
        connect: createWebSocket,
        close: closeWebSocket,
        send: sendCommand,
        isConnected: isConnected,
        getState: getConnectionState,
        subscribe: subscribe,
        emit: emit,
        config: CONFIG,
        // elementy używane przez js/common.js
        getUrl: getWebSocketUrl,
        resolveHost: getHost,
        get _ws() { return ws; },
        get _retryCount() { return retryCount; },
        set _retryCount(value) { retryCount = value; },
        get _reconnectTimer() { return reconnectTimer; }
    };

    if (CONFIG.debug) console.log('[WS] ws-utils.js załadowany');
})();
