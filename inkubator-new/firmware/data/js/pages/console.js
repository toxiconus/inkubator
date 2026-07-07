(function() {
    'use strict';

    const ink = window.Inkubator;
    if (!ink) {
        console.error('[console.js] Brak wspólnego modułu Inkubator');
        return;
    }

    // ============================================================
    //  REFERENCJE DO ELEMENTÓW
    // ============================================================
    const consoleDiv = document.getElementById('console');
    const cmdInput = document.getElementById('cmdInput');
    const sendBtn = document.getElementById('sendBtn');
    const clearBtn = document.getElementById('clearBtn');
    const backBtn = document.getElementById('backBtn');

    // ============================================================
    //  FUNKCJE POMOCNICZE
    // ============================================================
    function displayMessage(msg, className) {
        if (!consoleDiv) return;

        const timestamp = new Date().toLocaleTimeString();
        const line = document.createElement('div');
        line.className = className || 'system-message';
        line.textContent = '[' + timestamp + '] ' + msg;
        consoleDiv.appendChild(line);
        consoleDiv.scrollTop = consoleDiv.scrollHeight;

        // Ograniczenie liczby linii
        while (consoleDiv.children.length > 200) {
            consoleDiv.removeChild(consoleDiv.firstChild);
        }
    }

    function sendCommand() {
        const cmd = cmdInput ? cmdInput.value.trim() : '';
        if (!cmd) return;

        displayMessage('> ' + cmd, 'cmd-message');
        ink.wsSend(cmd);

        if (cmdInput) cmdInput.value = '';
        if (cmdInput) cmdInput.focus();
    }

    function clearConsole() {
        if (!consoleDiv) return;
        consoleDiv.innerHTML = '';
        displayMessage('--- Konsola wyczyszczona ---', 'system-message');
        ink.setStatus('Konsola wyczyszczona', 'info');
    }

    function goBack() {
        window.location.href = '/';
    }

    // ============================================================
    //  OBSŁUGA WEBSOCKET
    // ============================================================
    ink.onWsEvent(function(type, ws, data) {
        if (type === 'open') {
            ink.setStatus('Połączono', 'ok');
            displayMessage('--- Połączono z ESP ---', 'system-message');
            // Opcjonalnie: wyślij komendę powitalną
            // ws.send('carregarIdioma');
            return;
        }

        if (type === 'message') {
            const msg = data;
            // Pomijamy komunikaty statusu, które nie są interesujące dla konsoli
            if (msg.startsWith('idiomaAtual:')) return;
            if (msg.startsWith('status:')) return;
            if (msg.startsWith('wifiStatus:')) return;

            // Wyświetl wiadomość w konsoli
            displayMessage(msg, 'system-message');
        }

        if (type === 'close') {
            displayMessage('--- Rozłączono ---', 'error-message');
            ink.setStatus('Rozłączono', 'err');
        }

        if (type === 'error') {
            displayMessage('--- Błąd WebSocket ---', 'error-message');
            ink.setStatus('Błąd WebSocket', 'err');
        }
    });

    // ============================================================
    //  EVENT LISTENERY
    // ============================================================
    document.addEventListener('DOMContentLoaded', function() {
        if (sendBtn) sendBtn.addEventListener('click', sendCommand);
        if (clearBtn) clearBtn.addEventListener('click', clearConsole);
        if (backBtn) backBtn.addEventListener('click', goBack);

        if (cmdInput) {
            cmdInput.addEventListener('keydown', function(e) {
                if (e.key === 'Enter') sendCommand();
            });
            // Focus na polu wejściowym
            cmdInput.focus();
        }

        // Wyświetl komunikat powitalny
        if (consoleDiv && consoleDiv.children.length === 0) {
            displayMessage('--- Konsola gotowa ---', 'system-message');
            if (ink.wsConnected()) {
                displayMessage('--- Połączono z ESP ---', 'system-message');
            } else {
                displayMessage('--- Oczekiwanie na połączenie... ---', 'system-message');
            }
        }

        console.log('[console.js] Zainicjalizowano');
    });

})();