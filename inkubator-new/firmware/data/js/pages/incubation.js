(function() {
    'use strict';

    const ink = window.Inkubator;
    if (!ink) {
        console.error('[incubation.js] Brak wspólnego modułu Inkubator');
        return;
    }

    // ============================================================
    //  KONFIGURACJA GATUNKÓW
    // ============================================================
    const SPECIES = {
        CHICKEN: { name: 'Kura', days: 21, lockdown: 19 },
        DUCK: { name: 'Kaczka', days: 28, lockdown: 25 },
        GOOSE: { name: 'Gęś', days: 30, lockdown: 27 },
        QUAIL: { name: 'Przepiórka', days: 17, lockdown: 15 },
        GUINEA: { name: 'Perliczka', days: 26, lockdown: 23 },
        PHEASANT: { name: 'Bażant', days: 24, lockdown: 21 }
    };

    function getSpeciesInfo(id) {
        return SPECIES[id] || SPECIES.CHICKEN;
    }

    // ============================================================
    //  REFERENCJE DO ELEMENTÓW
    // ============================================================
    const species1 = document.getElementById('species1');
    const species2 = document.getElementById('species2');
    const day1 = document.getElementById('day1');
    const day2 = document.getElementById('day2');
    const startDate1 = document.getElementById('startDate1');
    const startDate2 = document.getElementById('startDate2');
    const totalDays1 = document.getElementById('totalDays1');
    const totalDays2 = document.getElementById('totalDays2');
    const currentDay1 = document.getElementById('currentDay1');
    const currentDay2 = document.getElementById('currentDay2');
    const remaining1 = document.getElementById('remaining1');
    const remaining2 = document.getElementById('remaining2');
    const status1 = document.getElementById('status1');
    const status2 = document.getElementById('status2');
    const tray1Day = document.getElementById('tray1Day');
    const tray2Day = document.getElementById('tray2Day');
    const scheduleBody = document.getElementById('scheduleBody');
    const scheduleInfo = document.getElementById('scheduleInfo');

    const setStart1 = document.getElementById('setStart1');
    const setStart2 = document.getElementById('setStart2');
    const resetDay1 = document.getElementById('resetDay1');
    const resetDay2 = document.getElementById('resetDay2');
    const applyBtn = document.getElementById('applyBtn');
    const previewBtn = document.getElementById('previewBtn');
    const refreshBtn = document.getElementById('refreshBtn');
    const backBtn = document.getElementById('backBtn');

    // ============================================================
    //  FUNKCJE POMOCNICZE
    // ============================================================
    function getTrayConfig(trayIndex) {
        const speciesEl = trayIndex === 0 ? species1 : species2;
        const dayEl = trayIndex === 0 ? day1 : day2;
        const info = getSpeciesInfo(speciesEl.value);
        const day = parseInt(dayEl.value) || 1;
        const clamped = Math.max(1, Math.min(info.days, day));

        return {
            species: speciesEl.value,
            speciesInfo: info,
            day: clamped,
            totalDays: info.days,
            lockdown: info.lockdown
        };
    }

    function updateTrayInfo(trayIndex) {
        const cfg = getTrayConfig(trayIndex);

        const totalEl = trayIndex === 0 ? totalDays1 : totalDays2;
        const currentEl = trayIndex === 0 ? currentDay1 : currentDay2;
        const remainEl = trayIndex === 0 ? remaining1 : remaining2;
        const statusEl = trayIndex === 0 ? status1 : status2;
        const trayDayEl = trayIndex === 0 ? tray1Day : tray2Day;

        totalEl.textContent = cfg.totalDays;
        currentEl.textContent = cfg.day;
        remainEl.textContent = cfg.totalDays - cfg.day;
        trayDayEl.textContent = 'Dzień ' + cfg.day + '/' + cfg.totalDays;

        // Status
        const isLockdown = cfg.day >= cfg.lockdown;
        statusEl.textContent = isLockdown ? 'LOCKDOWN' : 'Normalny';
        statusEl.className = 'info-value ' + (isLockdown ? 'critical' : 'ok');

        // Aktualizuj max dnia
        const dayEl = trayIndex === 0 ? day1 : day2;
        dayEl.max = cfg.totalDays;
    }

    function renderSchedule(trayIndex) {
        const cfg = getTrayConfig(trayIndex);
        const info = cfg.speciesInfo;

        scheduleInfo.textContent = 'Taca ' + (trayIndex + 1) + ' · ' + info.name;

        let html = '';
        for (let d = 1; d <= info.days; d++) {
            const isActive = d === cfg.day;
            const isLockdown = d >= info.lockdown;
            const temp = 37.7 + (d > 14 ? 0.3 : 0) + (d > 18 ? 0.2 : 0);
            const rh = isLockdown ? 70 : 55;
            const turn = !isLockdown && d % 3 !== 0;
            const sens = d <= 3 || isLockdown ? 'CRITICAL' : (d <= 5 ? 'HIGH' : 'MEDIUM');
            const sensColor = sens === 'CRITICAL' ? '#e05050' : (sens === 'HIGH' ? '#e8b830' : '#70d070');

            let rowClass = '';
            if (isActive) rowClass = 'active';
            if (isLockdown) rowClass = 'lockdown';
            if (sens === 'CRITICAL') rowClass += ' critical';

            const turnHtml = turn ? '<span class="turn-on">✓</span>' : '<span class="turn-off">✗</span>';
            const lockHtml = isLockdown ? '<span class="lock-icon">🔒</span>' : '—';
            const activeMark = isActive ? '<span class="day-active">▶</span> ' : '';

            html += '<tr class="' + rowClass + '">';
            html += '<td>' + activeMark + d + '</td>';
            html += '<td style="color:' + sensColor + ';">' + temp.toFixed(1) + '</td>';
            html += '<td>' + rh + '</td>';
            html += '<td>' + turnHtml + '</td>';
            html += '<td>' + lockHtml + '</td>';
            html += '<td style="font-size:10px;color:#5a6a7a;">' + (sens === 'CRITICAL' ? '⚠ ' : '') + (isLockdown ? 'LOCKDOWN' : 'Normalny') + '</td>';
            html += '</tr>';
        }

        scheduleBody.innerHTML = html;

        // Przewiń do aktywnego dnia
        const wrap = document.getElementById('scheduleWrap');
        if (wrap) {
            const activeRow = scheduleBody.querySelector('tr.active');
            if (activeRow) {
                wrap.scrollTop = Math.max(0, activeRow.offsetTop - 60);
            }
        }
    }

    function updateAll() {
        updateTrayInfo(0);
        updateTrayInfo(1);
        renderSchedule(0);
    }

    // ============================================================
    //  OBSŁUGA WEBSOCKET
    // ============================================================
    ink.onWsEvent(function(type, ws, data) {
        if (type === 'open') {
            ink.setStatus('Połączono, pobieram konfigurację...', 'info');
            ws.send('getIncubationConfig');
            ws.send('getStartDate:0');
            ws.send('getStartDate:1');
            return;
        }

        if (type === 'message') {
            const msg = data;

            // Data startu
            if (msg.startsWith('startDate:') || msg.startsWith('startUnix:')) {
                const parts = msg.split(':');
                if (parts.length === 3) {
                    const tray = parseInt(parts[1], 10);
                    const unix = parseInt(parts[2], 10);
                    if (!isNaN(tray) && !isNaN(unix) && unix > 0) {
                        const date = new Date(unix * 1000);
                        const input = tray === 0 ? startDate1 : startDate2;
                        if (input) {
                            const month = String(date.getMonth() + 1).padStart(2, '0');
                            const day = String(date.getDate()).padStart(2, '0');
                            input.value = date.getFullYear() + '-' + month + '-' + day;
                        }
                    }
                }
                return;
            }

            // Potwierdzenia
            if (msg.startsWith('ok:setStartDate:') || msg.startsWith('ok:setStartUnix:')) {
                const parts = msg.split(':');
                if (parts.length === 3) {
                    const tray = parts[2];
                    ink.setStatus('Data startu ustawiona dla T' + (parseInt(tray) + 1), 'ok');
                    ws.send('getStartDate:' + tray);
                }
                return;
            }

            if (msg.startsWith('ok:resetDay:')) {
                const parts = msg.split(':');
                if (parts.length === 3) {
                    const tray = parts[2];
                    ink.setStatus('Dzień zresetowany dla T' + (parseInt(tray) + 1), 'ok');
                    ws.send('getStartDate:' + tray);
                }
                return;
            }

            // Status tac z broadcastu
            if (msg.startsWith('{') && msg.includes('tray')) {
                try {
                    const json = JSON.parse(msg);
                    if (json.tray0_day !== undefined) {
                        day1.value = json.tray0_day;
                        updateAll();
                    }
                    if (json.tray1_day !== undefined) {
                        day2.value = json.tray1_day;
                        updateAll();
                    }
                } catch (e) {
                    // Ignoruj błędy parsowania
                }
                return;
            }
        }
    });

    // ============================================================
    //  FUNKCJE AKCJI
    // ============================================================
    function setStartDate(tray) {
        const input = tray === 0 ? startDate1 : startDate2;
        const dateValue = input ? input.value : '';

        if (!dateValue) {
            ink.setStatus('Wybierz datę startu', 'err');
            return;
        }

        const date = new Date(dateValue + 'T00:00:00');
        if (isNaN(date.getTime())) {
            ink.setStatus('Nieprawidłowa data', 'err');
            return;
        }

        const unix = Math.floor(date.getTime() / 1000);
        ink.wsSend('setStartDate:' + tray + ':' + unix);
        ink.setStatus('Wysyłam datę startu...', 'info');
    }

    function resetDay(tray) {
        if (!confirm('Resetować dzień inkubacji dla tacy ' + (tray + 1) + '?')) return;
        ink.wsSend('resetDay:' + tray);
        ink.setStatus('Reset dnia wysłany', 'info');
    }

    function applyToTray() {
        const cfg = getTrayConfig(0);
        ink.setStatus('Zastosowano: ' + cfg.speciesInfo.name + ', dzień ' + cfg.day, 'ok');
        updateAll();
    }

    function previewTray2() {
        renderSchedule(1);
        ink.setStatus('Podgląd dla Tacy 2', 'info');
    }

    function refreshConfig() {
        ink.setStatus('Odświeżanie...', 'info');
        ink.wsSend('getIncubationConfig');
        ink.wsSend('getStartDate:0');
        ink.wsSend('getStartDate:1');
        updateAll();
    }

    function goBack() {
        window.location.href = '/';
    }

    // ============================================================
    //  INICJALIZACJA
    // ============================================================
    document.addEventListener('DOMContentLoaded', function() {
        // Event listenery dla tac
        species1.addEventListener('change', function() { updateAll(); });
        species2.addEventListener('change', function() { updateAll(); });
        day1.addEventListener('input', function() { updateAll(); });
        day2.addEventListener('input', function() { updateAll(); });

        // Przyciski
        if (setStart1) setStart1.addEventListener('click', function() { setStartDate(0); });
        if (setStart2) setStart2.addEventListener('click', function() { setStartDate(1); });
        if (resetDay1) resetDay1.addEventListener('click', function() { resetDay(0); });
        if (resetDay2) resetDay2.addEventListener('click', function() { resetDay(1); });
        if (applyBtn) applyBtn.addEventListener('click', applyToTray);
        if (previewBtn) previewBtn.addEventListener('click', previewTray2);
        if (refreshBtn) refreshBtn.addEventListener('click', refreshConfig);
        if (backBtn) backBtn.addEventListener('click', goBack);

        // Enter w polach dnia
        day1.addEventListener('keydown', function(e) {
            if (e.key === 'Enter') { e.preventDefault(); applyToTray(); }
        });
        day2.addEventListener('keydown', function(e) {
            if (e.key === 'Enter') { e.preventDefault(); applyToTray(); }
        });

        // Jeśli WebSocket jest już połączony
        if (ink.wsConnected()) {
            ink.wsSend('getIncubationConfig');
            ink.wsSend('getStartDate:0');
            ink.wsSend('getStartDate:1');
        }

        updateAll();
        console.log('[incubation.js] Zainicjalizowano');
    });

})();