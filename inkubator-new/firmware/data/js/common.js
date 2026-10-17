/**
 * common.js – wspólny kod dla wszystkich stron inkubatora
 * ============================================================
 * Zawiera: WebSocket, status bar, narzędzia
 */

(function() {
    'use strict';

    // ============================================================
    //  KONFIGURACJA
    // ============================================================
    const WS_PORT = 81;
    const RECONNECT_BASE_MS = 1000;   // pierwsza próba po 1 s
    const RECONNECT_MAX_MS = 10000;   // wykładnicze wydłużanie, max 10 s
    const RECONNECT_WATCHDOG_MS = 30000; // siatka bezpieczeństwa, gdy nic nie działa

    // ============================================================
    //  STAN
    // ============================================================
    let ws = null;
    let wsConnected = false;
    let reconnectTimer = null;
    let reconnectAttempts = 0;
    let statusCallbacks = [];

    // ============================================================
    //  ELEMENTY DOM
    // ============================================================
    const statusMsg = document.getElementById('statusMsg');
    const wsDot = document.getElementById('wsDot');
    const wsStatus = document.getElementById('wsStatus');
    const wsDetail = document.getElementById('wsDetail');

    // ============================================================
    //  FUNKCJE STATUSU
    // ============================================================
    function setStatus(text, type) {
        if (!statusMsg) return;
        statusMsg.textContent = text;
        statusMsg.className = 'msg' + (type ? ' ' + type : '');
    }

    function setWsStatus(connected) {
        wsConnected = connected;
        if (wsDot) {
            wsDot.className = 'dot' + (connected ? ' online' : ' offline');
        }
        if (wsStatus) {
            wsStatus.textContent = connected ? 'Połączono' : 'Rozłączono';
        }
        if (wsDetail) {
            wsDetail.textContent = connected ? 'WebSocket OK' : 'próba ' + reconnectAttempts;
        }
    }

    // ============================================================
    //  WEB SOCKET
    // ============================================================
    function getWsUrl() {
        const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
        return protocol + '//' + window.location.hostname + ':' + WS_PORT + '/';
    }

    function wsSend(cmd) {
        if (ws && ws.readyState === WebSocket.OPEN) {
            ws.send(cmd);
            return true;
        }
        setStatus('Brak połączenia z ESP', 'err');
        return false;
    }

    function wsConnect() {
        if (ws && (ws.readyState === WebSocket.OPEN || ws.readyState === WebSocket.CONNECTING)) {
            return;
        }

        try {
            const url = getWsUrl();
            ws = new WebSocket(url);

            ws.onopen = function() {
                reconnectAttempts = 0;
                setWsStatus(true);
                setStatus('Połączono', 'ok');
                statusCallbacks.forEach(cb => cb('open', ws));
            };

            ws.onmessage = function(event) {
                // Każda odpowiedź błędu z ESP trafia do paska statusu – nic nie znika po cichu.
                if (typeof event.data === 'string' && event.data.indexOf('err:') === 0) {
                    setStatus(event.data.substring(4).replace(/:/g, ' · '), 'err');
                }
                statusCallbacks.forEach(cb => cb('message', ws, event.data));
            };

            ws.onclose = function() {
                setWsStatus(false);
                setStatus('Rozłączono, ponowna próba...', 'err');
                scheduleReconnect();
            };

            ws.onerror = function(err) {
                console.warn('WebSocket error:', err);
                setStatus('Błąd WebSocket', 'err');
                if (ws) ws.close();
            };

        } catch (e) {
            console.warn('WebSocket init error:', e);
            setStatus('Nie można utworzyć połączenia', 'err');
            scheduleReconnect();
        }
    }

    // Ponawianie bez limitu prób: wykładniczo do RECONNECT_MAX_MS.
    // Strona zawsze sama wraca po restarcie ESP; przycisk/kropka wymusza natychmiastową próbę.
    function scheduleReconnect() {
        if (reconnectTimer) clearTimeout(reconnectTimer);
        reconnectAttempts++;
        const delay = Math.min(RECONNECT_MAX_MS, RECONNECT_BASE_MS * Math.pow(1.5, reconnectAttempts - 1));
        reconnectTimer = setTimeout(wsConnect, delay);
    }

    function reconnectManually() {
        if (reconnectTimer) clearTimeout(reconnectTimer);
        reconnectAttempts = 0;
        if (ws) {
            try { ws.close(); } catch (e) {}
            ws = null;
        }
        setStatus('Łączenie...', 'info');
        wsConnect();
    }

    // ============================================================
    //  REJESTRACJA CALLBACKÓW
    // ============================================================
    function onWsEvent(callback) {
        if (typeof callback === 'function') {
            statusCallbacks.push(callback);
        }
    }

    // ============================================================
    //  NARZĘDZIA
    // ============================================================
    function parseFloatSafe(value, fallback) {
        const parsed = parseFloat(value);
        return isNaN(parsed) ? fallback : parsed;
    }

    function parseIntSafe(value, fallback) {
        const parsed = parseInt(value, 10);
        return isNaN(parsed) ? fallback : parsed;
    }

    function clamp(value, min, max) {
        return Math.max(min, Math.min(max, value));
    }

    // ============================================================
    //  INICJALIZACJA
    // ============================================================
    document.addEventListener('DOMContentLoaded', function() {
        if (wsStatus) wsStatus.addEventListener('click', reconnectManually);
        if (wsDot) wsDot.addEventListener('click', reconnectManually);

        wsConnect();

        // Watchdog: jeśli połączenia nie ma, spróbuj ponownie – bez zerowania licznika backoffu.
        setInterval(function() {
            if (!wsConnected) wsConnect();
        }, RECONNECT_WATCHDOG_MS);
    });

    window.addEventListener('beforeunload', function() {
        if (ws) {
            try { ws.close(); } catch (e) {}
        }
        if (reconnectTimer) clearTimeout(reconnectTimer);
    });

    // ============================================================
    //  EKSPORT (globalny)
    // ============================================================
    window.Inkubator = {
        wsSend: wsSend,
        wsConnect: wsConnect,
        wsConnected: function() { return wsConnected; },
        setStatus: setStatus,
        setWsStatus: setWsStatus,
        reconnectManually: reconnectManually,
        onWsEvent: onWsEvent,
        parseFloatSafe: parseFloatSafe,
        parseIntSafe: parseIntSafe,
        clamp: clamp,
        getWsUrl: getWsUrl
    };

})();
