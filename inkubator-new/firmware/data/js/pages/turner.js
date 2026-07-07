(function() {
    'use strict';

    const ink = window.Inkubator;
    if (!ink) {
        console.error('[turner.js] Brak wspólnego modułu Inkubator');
        return;
    }

    // ============================================================
    //  REFERENCJE DO ELEMENTÓW
    // ============================================================
    const mode1 = document.getElementById('mode1');
    const workTime1 = document.getElementById('workTime1');
    const restTime1 = document.getElementById('restTime1');
    const pwm1 = document.getElementById('pwm1');

    const mode2 = document.getElementById('mode2');
    const workTime2 = document.getElementById('workTime2');
    const restTime2 = document.getElementById('restTime2');
    const pwm2 = document.getElementById('pwm2');

    const testTray = document.getElementById('testTray');
    const testStartBtn = document.getElementById('testStartBtn');
    const testStopBtn = document.getElementById('testStopBtn');
    const timerDisplay = document.getElementById('timerDisplay');

    const trayIndicator0 = document.getElementById('trayIndicator0');
    const trayIndicator1 = document.getElementById('trayIndicator1');
    const trayStatus0 = document.getElementById('trayStatus0');
    const trayStatus1 = document.getElementById('trayStatus1');

    const saveBtn = document.getElementById('saveBtn');
    const refreshBtn = document.getElementById('refreshBtn');
    const backBtn = document.getElementById('backBtn');

    // ============================================================
    //  STAN TESTU
    // ============================================================
    let testTimer = null;
    let testSeconds = 0;
    let testRunning = false;

    // ============================================================
    //  FUNKCJE POMOCNICZE
    // ============================================================
    function getConfig() {
        return {
            mode1: parseInt(mode1.value) || 0,
            work1: parseInt(workTime1.value) || 15,
            rest1: parseInt(restTime1.value) || 2,
            pwm1: parseInt(pwm1.value) || 128,
            mode2: parseInt(mode2.value) || 0,
            work2: parseInt(workTime2.value) || 15,
            rest2: parseInt(restTime2.value) || 2,
            pwm2: parseInt(pwm2.value) || 128
        };
    }

    function applyConfig(cfg) {
        if (cfg.mode1 !== undefined) mode1.value = cfg.mode1;
        if (cfg.work1 !== undefined) workTime1.value = cfg.work1;
        if (cfg.rest1 !== undefined) restTime1.value = cfg.rest1;
        if (cfg.pwm1 !== undefined) pwm1.value = cfg.pwm1;
        if (cfg.mode2 !== undefined) mode2.value = cfg.mode2;
        if (cfg.work2 !== undefined) workTime2.value = cfg.work2;
        if (cfg.rest2 !== undefined) restTime2.value = cfg.rest2;
        if (cfg.pwm2 !== undefined) pwm2.value = cfg.pwm2;
    }

    function validateConfig(cfg) {
        if (cfg.work1 < 10 || cfg.work1 > 60 || cfg.work2 < 10 || cfg.work2 > 60) {
            ink.setStatus('Czas pracy musi być 10–60s', 'err');
            return false;
        }
        if (cfg.rest1 < 1 || cfg.rest1 > 4 || cfg.rest2 < 1 || cfg.rest2 > 4) {
            ink.setStatus('Czas przerwy musi być 1–4h', 'err');
            return false;
        }
        if (cfg.pwm1 < 0 || cfg.pwm1 > 255 || cfg.pwm2 < 0 || cfg.pwm2 > 255) {
            ink.setStatus('PWM musi być 0–255', 'err');
            return false;
        }
        return true;
    }

    function updateTrayIndicator(index, status) {
        const indicator = index === 0 ? trayIndicator0 : trayIndicator1;
        const statusText = index === 0 ? trayStatus0 : trayStatus1;

        if (!indicator || !statusText) return;

        indicator.className = 'tray-indicator';
        if (status === 'active') {
            indicator.classList.add('active');
            statusText.textContent = 'AKTYWNA';
        } else if (status === 'rotating') {
            indicator.classList.add('rotating');
            statusText.textContent = 'OBRÓT';
        } else {
            indicator.classList.add('inactive');
            statusText.textContent = '--';
        }
    }

    function updateTimerDisplay(seconds) {
        if (!timerDisplay) return;
        if (seconds === null || seconds === undefined) {
            timerDisplay.textContent = '--';
            return;
        }
        timerDisplay.textContent = seconds + ' s';
    }

    // ============================================================
    //  OBSŁUGA WEBSOCKET
    // ============================================================
    ink.onWsEvent(function(type, ws, data) {
        if (type === 'open') {
            ink.setStatus('Połączono, pobieram konfigurację...', 'info');
            ws.send('getTurnerConfig');
            return;
        }

        if (type === 'message') {
            const msg = data;

            // Parsowanie konfiguracji obrotu
            if (msg.includes('turnerMode0:') || msg.includes('modoVirador0:')) {
                const parts = msg.split(',');
                const cfg = {};

                parts.forEach(function(part) {
                    const [key, val] = part.split(':');
                    if (!key || val === undefined) return;

                    const k = key.trim();
                    const v = val.trim();

                    if (k === 'turnerMode0' || k === 'modoVirador0') cfg.mode1 = parseInt(v, 10);
                    if (k === 'turnerWorkTime0' || k === 'tempoTrabalho0') cfg.work1 = parseInt(v, 10);
                    if (k === 'turnerRestTime0' || k === 'tempoRepouso0') cfg.rest1 = parseInt(v, 10);
                    if (k === 'turnerPwm0' || k === 'pwm0') cfg.pwm1 = parseInt(v, 10);
                    if (k === 'turnerMode1' || k === 'modoVirador1') cfg.mode2 = parseInt(v, 10);
                    if (k === 'turnerWorkTime1' || k === 'tempoTrabalho1') cfg.work2 = parseInt(v, 10);
                    if (k === 'turnerRestTime1' || k === 'tempoRepouso1') cfg.rest2 = parseInt(v, 10);
                    if (k === 'turnerPwm1' || k === 'pwm1') cfg.pwm2 = parseInt(v, 10);
                });

                if (Object.keys(cfg).length > 0) {
                    applyConfig(cfg);
                    ink.setStatus('Konfiguracja obrotu wczytana', 'ok');
                }
                return;
            }

            // Status obrotu z broadcastu
            if (msg.startsWith('{') && msg.includes('tray')) {
                try {
                    const json = JSON.parse(msg);
                    if (json.tray0_status !== undefined) {
                        updateTrayIndicator(0, json.tray0_status);
                    }
                    if (json.tray1_status !== undefined) {
                        updateTrayIndicator(1, json.tray1_status);
                    }
                } catch (e) {
                    // Ignoruj błędy parsowania
                }
                return;
            }

            // Potwierdzenia
            if (msg.startsWith('ok:setTurnerMode0') || msg.startsWith('ok:setTurnerWorkTime0') ||
                msg.startsWith('ok:setTurnerRestTime0') || msg.startsWith('ok:setTurnerPwm0') ||
                msg.startsWith('ok:setTurnerMode1') || msg.startsWith('ok:setTurnerWorkTime1') ||
                msg.startsWith('ok:setTurnerRestTime1') || msg.startsWith('ok:setTurnerPwm1')) {
                ink.setStatus('Ustawienia obrotu zapisane', 'ok');
                return;
            }

            if (msg.startsWith('ok:turnerStart:')) {
                const tray = msg.substring(14);
                ink.setStatus('Test rozpoczęty dla T' + (parseInt(tray) + 1), 'info');
                return;
            }

            if (msg.startsWith('ok:turnerStop:')) {
                const tray = msg.substring(13);
                ink.setStatus('Test zatrzymany dla T' + (parseInt(tray) + 1), 'info');
                return;
            }
        }
    });

    // ============================================================
    //  FUNKCJE AKCJI
    // ============================================================
    function saveConfig() {
        const cfg = getConfig();
        if (!validateConfig(cfg)) return;

        ink.wsSend('setTurnerMode0:' + cfg.mode1);
        ink.wsSend('setTurnerWorkTime0:' + cfg.work1);
        ink.wsSend('setTurnerRestTime0:' + cfg.rest1);
        ink.wsSend('setTurnerPwm0:' + cfg.pwm1);
        ink.wsSend('setTurnerMode1:' + cfg.mode2);
        ink.wsSend('setTurnerWorkTime1:' + cfg.work2);
        ink.wsSend('setTurnerRestTime1:' + cfg.rest2);
        ink.wsSend('setTurnerPwm1:' + cfg.pwm2);

        ink.setStatus('Wysłano ustawienia obrotu', 'ok');
    }

    function refreshConfig() {
        ink.setStatus('Odświeżanie...', 'info');
        ink.wsSend('getTurnerConfig');
    }

    function startTest() {
        if (testRunning) return;

        const tray = parseInt(testTray.value) || 0;
        testRunning = true;
        testSeconds = 0;
        updateTimerDisplay(testSeconds);

        ink.wsSend('turnerStart:' + tray);
        ink.setStatus('Test rozpoczęty dla T' + (tray + 1), 'info');

        if (testTimer) clearInterval(testTimer);
        testTimer = setInterval(function() {
            testSeconds++;
            updateTimerDisplay(testSeconds);
        }, 1000);
    }

    function stopTest() {
        if (!testRunning) return;

        const tray = parseInt(testTray.value) || 0;
        testRunning = false;
        if (testTimer) {
            clearInterval(testTimer);
            testTimer = null;
        }

        ink.wsSend('turnerStop:' + tray);
        ink.setStatus('Test zatrzymany, czas: ' + testSeconds + 's', 'info');

        // Zapytaj czy ustawić czas pracy
        if (testSeconds >= 10 && testSeconds <= 60) {
            if (confirm('Ustawić czas pracy na ' + testSeconds + ' s dla T' + (tray + 1) + '?')) {
                const workEl = tray === 0 ? workTime1 : workTime2;
                if (workEl) {
                    workEl.value = testSeconds;
                    ink.setStatus('Czas pracy ustawiony na ' + testSeconds + 's', 'ok');
                }
            }
        }

        updateTimerDisplay(null);
    }

    function goBack() {
        window.location.href = '/';
    }

    // ============================================================
    //  INICJALIZACJA
    // ============================================================
    document.addEventListener('DOMContentLoaded', function() {
        if (saveBtn) saveBtn.addEventListener('click', saveConfig);
        if (refreshBtn) refreshBtn.addEventListener('click', refreshConfig);
        if (backBtn) backBtn.addEventListener('click', goBack);
        if (testStartBtn) testStartBtn.addEventListener('click', startTest);
        if (testStopBtn) testStopBtn.addEventListener('click', stopTest);

        // Enter w polach – zapisz
        document.querySelectorAll('#configForm input').forEach(function(input) {
            input.addEventListener('keydown', function(e) {
                if (e.key === 'Enter') {
                    e.preventDefault();
                    saveConfig();
                }
            });
        });

        // Jeśli WebSocket jest już połączony, pobierz konfigurację
        if (ink.wsConnected()) {
            ink.wsSend('getTurnerConfig');
        }

        // Domyślny stan wskaźników
        updateTrayIndicator(0, 'inactive');
        updateTrayIndicator(1, 'inactive');
        updateTimerDisplay(null);

        console.log('[turner.js] Zainicjalizowano');
    });

    // Czyszczenie przy wyjściu
    window.addEventListener('beforeunload', function() {
        if (testTimer) {
            clearInterval(testTimer);
            testTimer = null;
        }
    });

})();