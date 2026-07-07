(function() {
    'use strict';

    const ink = window.Inkubator;
    if (!ink) {
        console.error('[alarms.js] Brak wspólnego modułu Inkubator');
        return;
    }

    // ============================================================
    //  KONFIGURACJA ALARMÓW
    // ============================================================
    const ALARM_CONFIG = [
        { id: 0, label: 'Temperatura wysoka', critical: true },
        { id: 1, label: 'Temperatura niska', critical: true },
        { id: 2, label: 'Wilgotność wysoka', critical: true },
        { id: 3, label: 'Wilgotność niska', critical: true },
        { id: 4, label: 'Awaria grzałki', critical: true },
        { id: 5, label: 'Awaria pompy', critical: true },
        { id: 6, label: 'Niski poziom wody', critical: false },
        { id: 7, label: 'Brak Wi-Fi', critical: false },
        { id: 8, label: 'Błąd NTP', critical: false },
        { id: 9, label: 'Błąd czujnika', critical: true }
    ];

    const ALARM_COUNT = ALARM_CONFIG.length;

    // ============================================================
    //  REFERENCJE DO ELEMENTÓW
    // ============================================================
    const alarmList = document.getElementById('alarmList');
    const saveBtn = document.getElementById('saveBtn');
    const refreshBtn = document.getElementById('refreshBtn');
    const backBtn = document.getElementById('backBtn');
    const overallStatus = document.getElementById('alarmOverallStatus');

    // ============================================================
    //  FUNKCJE POMOCNICZE
    // ============================================================
    function getAlarmCheckbox(id) {
        return document.getElementById('alarm' + id);
    }

    function getAlarmStatus(id) {
        const cb = getAlarmCheckbox(id);
        return cb ? cb.checked : false;
    }

    function setAlarmStatus(id, checked) {
        const cb = getAlarmCheckbox(id);
        if (cb) cb.checked = checked;
    }

    function updateOverallStatus() {
        if (!overallStatus) return;

        let anyActive = false;
        for (let i = 0; i < ALARM_COUNT; i++) {
            if (getAlarmStatus(i)) {
                anyActive = true;
                break;
            }
        }

        if (anyActive) {
            overallStatus.textContent = '⚠️ ALARM';
            overallStatus.style.color = '#e05050';
        } else {
            overallStatus.textContent = '✓ OK';
            overallStatus.style.color = '#70d070';
        }
    }

    function renderAlarms() {
        if (!alarmList) return;

        alarmList.innerHTML = '';

        ALARM_CONFIG.forEach(function(config) {
            const group = document.createElement('div');
            group.className = 'alarm-group';

            const label = document.createElement('label');
            label.htmlFor = 'alarm' + config.id;

            const idSpan = document.createElement('span');
            idSpan.className = 'alarm-id';
            idSpan.textContent = config.id + 1;
            label.appendChild(idSpan);

            const textSpan = document.createElement('span');
            textSpan.textContent = config.label;
            label.appendChild(textSpan);

            const input = document.createElement('input');
            input.type = 'checkbox';
            input.id = 'alarm' + config.id;
            input.dataset.critical = config.critical ? 'true' : 'false';

            // Status label
            const statusSpan = document.createElement('span');
            statusSpan.className = 'alarm-status off';
            statusSpan.textContent = 'OFF';
            statusSpan.id = 'alarmStatus' + config.id;

            // Zmiana checkboxa aktualizuje status
            input.addEventListener('change', function() {
                const isChecked = this.checked;
                const statusEl = document.getElementById('alarmStatus' + config.id);
                if (statusEl) {
                    statusEl.textContent = isChecked ? 'ON' : 'OFF';
                    statusEl.className = 'alarm-status ' + (isChecked ? 'on' : 'off');
                }
                updateOverallStatus();
            });

            group.appendChild(label);
            group.appendChild(input);
            group.appendChild(statusSpan);

            alarmList.appendChild(group);
        });

        // Domyślnie wszystkie wyłączone
        for (let i = 0; i < ALARM_COUNT; i++) {
            setAlarmStatus(i, false);
            const statusEl = document.getElementById('alarmStatus' + i);
            if (statusEl) {
                statusEl.textContent = 'OFF';
                statusEl.className = 'alarm-status off';
            }
        }

        updateOverallStatus();
    }

    // ============================================================
    //  OBSŁUGA WEBSOCKET
    // ============================================================
    ink.onWsEvent(function(type, ws, data) {
        if (type === 'open') {
            ink.setStatus('Połączono, pobieram alarmy...', 'info');
            ws.send('carregarAlarmes');
            return;
        }

        if (type === 'message') {
            const msg = data;

            if (msg.startsWith('statusAlarmes:')) {
                const statuses = msg.substring(14).split(',');
                for (let i = 0; i < ALARM_COUNT && i < statuses.length; i++) {
                    const val = parseInt(statuses[i], 10);
                    setAlarmStatus(i, val === 1);

                    const statusEl = document.getElementById('alarmStatus' + i);
                    if (statusEl) {
                        statusEl.textContent = val === 1 ? 'ON' : 'OFF';
                        statusEl.className = 'alarm-status ' + (val === 1 ? 'on' : 'off');
                    }
                }
                updateOverallStatus();
                ink.setStatus('Stan alarmów wczytany', 'ok');
                return;
            }

            if (msg.startsWith('ok:salvarAlarmes')) {
                ink.setStatus('Alarmy zapisane', 'ok');
                return;
            }

            if (msg.startsWith('err:salvarAlarmes')) {
                ink.setStatus('Błąd zapisu alarmów', 'err');
                return;
            }
        }
    });

    // ============================================================
    //  FUNKCJE AKCJI
    // ============================================================
    function saveAlarms() {
        const states = [];
        for (let i = 0; i < ALARM_COUNT; i++) {
            states.push(getAlarmStatus(i) ? 1 : 0);
        }
        const msg = 'salvarAlarmes:' + states.join(',');
        if (ink.wsSend(msg)) {
            ink.setStatus('Wysłano ustawienia alarmów', 'ok');
        }
    }

    function refreshAlarms() {
        ink.setStatus('Odświeżanie alarmów...', 'info');
        ink.wsSend('carregarAlarmes');
    }

    function goBack() {
        window.location.href = '/';
    }

    // ============================================================
    //  INICJALIZACJA
    // ============================================================
    document.addEventListener('DOMContentLoaded', function() {
        renderAlarms();

        if (saveBtn) saveBtn.addEventListener('click', saveAlarms);
        if (refreshBtn) refreshBtn.addEventListener('click', refreshAlarms);
        if (backBtn) backBtn.addEventListener('click', goBack);

        // Jeśli WebSocket jest już połączony, pobierz alarmy
        if (ink.wsConnected()) {
            ink.wsSend('carregarAlarmes');
        }

        console.log('[alarms.js] Zainicjalizowano');
    });

})();