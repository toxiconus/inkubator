(function() {
    'use strict';

    const ink = window.Inkubator;
    if (!ink) {
        console.error('[temperature.js] Brak wspólnego modułu Inkubator');
        return;
    }

    // ============================================================
    //  REFERENCJE DO ELEMENTÓW
    // ============================================================
    const setpointEl = document.getElementById('setpoint');
    const hysteresisEl = document.getElementById('hysteresis');
    const alarmMaxEl = document.getElementById('alarmMax');
    const alarmMinEl = document.getElementById('alarmMin');
    const calibrationEl = document.getElementById('calibration');
    const controlModeEl = document.getElementById('controlMode');
    const fanDelayEl = document.getElementById('fanDelay');
    const currentTempEl = document.getElementById('currentTemp');

    const saveBtn = document.getElementById('saveBtn');
    const clearMinMaxBtn = document.getElementById('clearMinMaxBtn');
    const resetBtn = document.getElementById('resetBtn');
    const backBtn = document.getElementById('backBtn');

    // ============================================================
    //  FUNKCJE POMOCNICZE
    // ============================================================
    function getConfig() {
        return {
            setpoint: parseFloat(setpointEl.value) || 37.5,
            hysteresis: parseFloat(hysteresisEl.value) || 0.5,
            alarmMax: parseFloat(alarmMaxEl.value) || 40.0,
            alarmMin: parseFloat(alarmMinEl.value) || 36.0,
            calibration: parseFloat(calibrationEl.value) || 0.0,
            controlMode: parseInt(controlModeEl.value) || 0,
            fanDelay: parseInt(fanDelayEl.value) || 5
        };
    }

    function applyConfig(cfg) {
        if (cfg.setpoint !== undefined) setpointEl.value = cfg.setpoint;
        if (cfg.hysteresis !== undefined) hysteresisEl.value = cfg.hysteresis;
        if (cfg.alarmMax !== undefined) alarmMaxEl.value = cfg.alarmMax;
        if (cfg.alarmMin !== undefined) alarmMinEl.value = cfg.alarmMin;
        if (cfg.calibration !== undefined) calibrationEl.value = cfg.calibration;
        if (cfg.controlMode !== undefined) controlModeEl.value = cfg.controlMode;
        if (cfg.fanDelay !== undefined) fanDelayEl.value = cfg.fanDelay;
    }

    function validateConfig(cfg) {
        if (cfg.setpoint < 36 || cfg.setpoint > 39) {
            ink.setStatus('Setpoint poza zakresem 36–39°C', 'err');
            return false;
        }
        if (cfg.hysteresis < 0.2 || cfg.hysteresis > 1.0) {
            ink.setStatus('Histereza poza zakresem 0.2–1.0°C', 'err');
            return false;
        }
        if (cfg.alarmMax <= cfg.setpoint + 1) {
            ink.setStatus('Alarm max musi być > setpoint + 1°C', 'err');
            return false;
        }
        if (cfg.alarmMin >= cfg.setpoint - cfg.hysteresis - 1) {
            ink.setStatus('Alarm min musi być < setpoint – histereza – 1°C', 'err');
            return false;
        }
        if (cfg.calibration < -5 || cfg.calibration > 5) {
            ink.setStatus('Kalibracja poza zakresem -5..+5°C', 'err');
            return false;
        }
        if (cfg.fanDelay < 0 || cfg.fanDelay > 15) {
            ink.setStatus('Opóźnienie wentylatora 0–15s', 'err');
            return false;
        }
        return true;
    }

    function updateCurrentTemp(temp) {
        if (!currentTempEl) return;

        if (isNaN(temp) || temp < -50 || temp > 80) {
            currentTempEl.innerHTML = '-- <span class="unit">°C</span>';
            return;
        }

        // Sprawdź czy temperatura jest w zakresie alarmów
        const alarmMax = parseFloat(alarmMaxEl.value) || 40.0;
        const alarmMin = parseFloat(alarmMinEl.value) || 36.0;

        let statusClass = 'ok';
        if (temp > alarmMax || temp < alarmMin) {
            statusClass = 'err';
        } else if (temp > alarmMax - 0.5 || temp < alarmMin + 0.5) {
            statusClass = 'warn';
        }

        currentTempEl.innerHTML = temp.toFixed(2) + ' <span class="unit">°C</span>';
        currentTempEl.className = 'value ' + statusClass;
    }

    // ============================================================
    //  OBSŁUGA WEBSOCKET
    // ============================================================
    ink.onWsEvent(function(type, ws, data) {
        if (type === 'open') {
            ink.setStatus('Połączono, pobieram konfigurację...', 'info');
            ws.send('carregarConfiguracao');
            return;
        }

        if (type === 'message') {
            const msg = data;

            // Parsowanie konfiguracji
            if (msg.startsWith('setTemp:') || msg.startsWith('setpoint:')) {
                const val = msg.includes('setTemp:') ? msg.substring(7) : msg.substring(9);
                const parsed = parseFloat(val);
                if (!isNaN(parsed)) {
                    setpointEl.value = parsed;
                    ink.setStatus('Odczytano setpoint: ' + parsed + '°C', 'ok');
                }
                return;
            }

            if (msg.startsWith('setHysteresis:') || msg.startsWith('histerese:')) {
                const val = msg.includes('setHysteresis:') ? msg.substring(14) : msg.substring(10);
                const parsed = parseFloat(val);
                if (!isNaN(parsed)) {
                    hysteresisEl.value = parsed;
                    ink.setStatus('Odczytano histerezę: ' + parsed + '°C', 'ok');
                }
                return;
            }

            if (msg.startsWith('setAlarmMaxTemp:') || msg.startsWith('alarmMax:')) {
                const val = msg.includes('setAlarmMaxTemp:') ? msg.substring(16) : msg.substring(9);
                const parsed = parseFloat(val);
                if (!isNaN(parsed)) {
                    alarmMaxEl.value = parsed;
                }
                return;
            }

            if (msg.startsWith('setAlarmMinTemp:') || msg.startsWith('alarmMin:')) {
                const val = msg.includes('setAlarmMinTemp:') ? msg.substring(16) : msg.substring(9);
                const parsed = parseFloat(val);
                if (!isNaN(parsed)) {
                    alarmMinEl.value = parsed;
                }
                return;
            }

            if (msg.startsWith('setTempCalibration:') || msg.startsWith('calibracao:')) {
                const val = msg.includes('setTempCalibration:') ? msg.substring(18) : msg.substring(11);
                const parsed = parseFloat(val);
                if (!isNaN(parsed)) {
                    calibrationEl.value = parsed;
                }
                return;
            }

            if (msg.startsWith('setControlMode:') || msg.startsWith('mode:')) {
                const val = msg.includes('setControlMode:') ? msg.substring(15) : msg.substring(5);
                const parsed = parseInt(val, 10);
                if (!isNaN(parsed)) {
                    controlModeEl.value = parsed;
                }
                return;
            }

            if (msg.startsWith('setFanDelay:') || msg.startsWith('ventiladorAtual:')) {
                const val = msg.includes('setFanDelay:') ? msg.substring(12) : msg.substring(16);
                const parsed = parseInt(val, 10);
                if (!isNaN(parsed)) {
                    fanDelayEl.value = parsed;
                }
                return;
            }

            // Aktualna temperatura z broadcastu
            if (msg.startsWith('{') && msg.includes('temp_chamber')) {
                try {
                    const json = JSON.parse(msg);
                    if (json.temp_chamber !== undefined) {
                        updateCurrentTemp(json.temp_chamber);
                    }
                } catch (e) {
                    // Ignoruj błędy parsowania
                }
                return;
            }

            // Potwierdzenia zapisu
            if (msg.startsWith('ok:setTemp') || msg.startsWith('ok:setpoint')) {
                ink.setStatus('Setpoint zapisany', 'ok');
                return;
            }
            if (msg.startsWith('ok:setHysteresis')) {
                ink.setStatus('Histereza zapisana', 'ok');
                return;
            }
            if (msg.startsWith('ok:setAlarmMaxTemp')) {
                ink.setStatus('Alarm max zapisany', 'ok');
                return;
            }
            if (msg.startsWith('ok:setAlarmMinTemp')) {
                ink.setStatus('Alarm min zapisany', 'ok');
                return;
            }
            if (msg.startsWith('ok:setTempCalibration')) {
                ink.setStatus('Kalibracja zapisana', 'ok');
                return;
            }
            if (msg.startsWith('ok:setControlMode')) {
                ink.setStatus('Tryb sterowania zapisany', 'ok');
                return;
            }
            if (msg.startsWith('ok:setFanDelay')) {
                ink.setStatus('Opóźnienie wentylatora zapisane', 'ok');
                return;
            }
            if (msg.startsWith('ok:resetMinMax')) {
                ink.setStatus('Reset min/max wykonany', 'ok');
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

        ink.wsSend('setpoint:' + cfg.setpoint.toFixed(1));
        ink.wsSend('histerese:' + cfg.hysteresis.toFixed(1));
        ink.wsSend('alarmMax:' + cfg.alarmMax.toFixed(1));
        ink.wsSend('alarmMin:' + cfg.alarmMin.toFixed(1));
        ink.wsSend('calibracao:' + cfg.calibration.toFixed(1));
        ink.wsSend('mode:' + cfg.controlMode);
        ink.wsSend('salvarVentilador:' + cfg.fanDelay);

        ink.setStatus('Wysłano ustawienia', 'ok');
    }

    function clearMinMax() {
        if (ink.wsSend('limparMinMax')) {
            ink.setStatus('Wysłano reset min/max', 'info');
        }
    }

    function resetToDefaults() {
        if (!confirm('Przywrócić domyślne ustawienia temperatury?')) return;

        const defaults = {
            setpoint: 37.5,
            hysteresis: 0.5,
            alarmMax: 40.0,
            alarmMin: 36.0,
            calibration: 0.0,
            controlMode: 0,
            fanDelay: 5
        };

        applyConfig(defaults);
        ink.setStatus('Ustawiono wartości domyślne (zapisz, aby wysłać)', 'info');
    }

    function goBack() {
        window.location.href = '/';
    }

    // ============================================================
    //  INICJALIZACJA
    // ============================================================
    document.addEventListener('DOMContentLoaded', function() {
        if (saveBtn) saveBtn.addEventListener('click', saveConfig);
        if (clearMinMaxBtn) clearMinMaxBtn.addEventListener('click', clearMinMax);
        if (resetBtn) resetBtn.addEventListener('click', resetToDefaults);
        if (backBtn) backBtn.addEventListener('click', goBack);

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
            ink.wsSend('carregarConfiguracao');
        }

        console.log('[temperature.js] Zainicjalizowano');
    });

})();