(function() {
    'use strict';

    const ink = window.Inkubator;
    if (!ink) {
        console.error('[humidity.js] Brak wspólnego modułu Inkubator');
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
    const pumpTimeEl = document.getElementById('pumpTime');
    const waterIntervalEl = document.getElementById('waterInterval');
    const currentHumidityEl = document.getElementById('currentHumidity');

    const saveBtn = document.getElementById('saveBtn');
    const resetBtn = document.getElementById('resetBtn');
    const backBtn = document.getElementById('backBtn');

    // ============================================================
    //  FUNKCJE POMOCNICZE
    // ============================================================
    function getConfig() {
        return {
            setpoint: parseInt(setpointEl.value) || 55,
            hysteresis: parseInt(hysteresisEl.value) || 5,
            alarmMax: parseInt(alarmMaxEl.value) || 75,
            alarmMin: parseInt(alarmMinEl.value) || 30,
            calibration: parseFloat(calibrationEl.value) || 0.0,
            pumpTime: parseInt(pumpTimeEl.value) || 30,
            waterInterval: parseInt(waterIntervalEl.value) || 12
        };
    }

    function applyConfig(cfg) {
        if (cfg.setpoint !== undefined) setpointEl.value = cfg.setpoint;
        if (cfg.hysteresis !== undefined) hysteresisEl.value = cfg.hysteresis;
        if (cfg.alarmMax !== undefined) alarmMaxEl.value = cfg.alarmMax;
        if (cfg.alarmMin !== undefined) alarmMinEl.value = cfg.alarmMin;
        if (cfg.calibration !== undefined) calibrationEl.value = cfg.calibration;
        if (cfg.pumpTime !== undefined) pumpTimeEl.value = cfg.pumpTime;
        if (cfg.waterInterval !== undefined) waterIntervalEl.value = cfg.waterInterval;
    }

    function validateConfig(cfg) {
        if (cfg.setpoint < 30 || cfg.setpoint > 80) {
            ink.setStatus('Setpoint poza zakresem 30–80%', 'err');
            return false;
        }
        if (cfg.hysteresis < 1 || cfg.hysteresis > 20) {
            ink.setStatus('Histereza poza zakresem 1–20%', 'err');
            return false;
        }
        if (cfg.alarmMax <= cfg.setpoint + cfg.hysteresis) {
            ink.setStatus('Alarm max musi być > setpoint + histereza', 'err');
            return false;
        }
        if (cfg.alarmMin >= cfg.setpoint - cfg.hysteresis) {
            ink.setStatus('Alarm min musi być < setpoint – histereza', 'err');
            return false;
        }
        if (cfg.calibration < -10 || cfg.calibration > 10) {
            ink.setStatus('Kalibracja poza zakresem -10..+10%', 'err');
            return false;
        }
        if (cfg.pumpTime < 0 || cfg.pumpTime > 60) {
            ink.setStatus('Czas pompy poza zakresem 0–60s', 'err');
            return false;
        }
        if (cfg.waterInterval < 1 || cfg.waterInterval > 24) {
            ink.setStatus('Interwał poza zakresem 1–24h', 'err');
            return false;
        }
        return true;
    }

    function updateCurrentHumidity(humidity) {
        if (!currentHumidityEl) return;

        if (isNaN(humidity) || humidity < 0 || humidity > 100) {
            currentHumidityEl.innerHTML = '-- <span class="unit">%</span>';
            return;
        }

        // Sprawdź czy wilgotność jest w zakresie alarmów
        const alarmMax = parseInt(alarmMaxEl.value) || 75;
        const alarmMin = parseInt(alarmMinEl.value) || 30;

        let statusClass = 'ok';
        if (humidity > alarmMax || humidity < alarmMin) {
            statusClass = 'err';
        } else if (humidity > alarmMax - 5 || humidity < alarmMin + 5) {
            statusClass = 'warn';
        }

        currentHumidityEl.innerHTML = humidity.toFixed(1) + ' <span class="unit">%</span>';
        currentHumidityEl.className = 'value ' + statusClass;
    }

    // ============================================================
    //  OBSŁUGA WEBSOCKET
    // ============================================================
    ink.onWsEvent(function(type, ws, data) {
        if (type === 'open') {
            ink.setStatus('Połączono, pobieram konfigurację...', 'info');
            ws.send('getHumidityConfig');
            return;
        }

        if (type === 'message') {
            const msg = data;

            // Parsowanie konfiguracji wilgotności
            if (msg.startsWith('setpointUmidade:') || msg.startsWith('setHumidity:')) {
                const val = msg.includes('setpointUmidade:') ? msg.substring(16) : msg.substring(12);
                const parsed = parseInt(val, 10);
                if (!isNaN(parsed)) {
                    setpointEl.value = parsed;
                    ink.setStatus('Odczytano setpoint: ' + parsed + '%', 'ok');
                }
                return;
            }

            if (msg.startsWith('histereseUmidade:') || msg.startsWith('setHumidityHysteresis:')) {
                const val = msg.includes('histereseUmidade:') ? msg.substring(17) : msg.substring(21);
                const parsed = parseInt(val, 10);
                if (!isNaN(parsed)) {
                    hysteresisEl.value = parsed;
                    ink.setStatus('Odczytano histerezę: ' + parsed + '%', 'ok');
                }
                return;
            }

            if (msg.startsWith('alarmMaxUmidade:') || msg.startsWith('setHumidityAlarmMax:')) {
                const val = msg.includes('alarmMaxUmidade:') ? msg.substring(16) : msg.substring(20);
                const parsed = parseInt(val, 10);
                if (!isNaN(parsed)) {
                    alarmMaxEl.value = parsed;
                }
                return;
            }

            if (msg.startsWith('alarmMinUmidade:') || msg.startsWith('setHumidityAlarmMin:')) {
                const val = msg.includes('alarmMinUmidade:') ? msg.substring(16) : msg.substring(20);
                const parsed = parseInt(val, 10);
                if (!isNaN(parsed)) {
                    alarmMinEl.value = parsed;
                }
                return;
            }

            if (msg.startsWith('calibracaoUmidade:') || msg.startsWith('setHumidityCalibration:')) {
                const val = msg.includes('calibracaoUmidade:') ? msg.substring(17) : msg.substring(22);
                const parsed = parseFloat(val);
                if (!isNaN(parsed)) {
                    calibrationEl.value = parsed;
                }
                return;
            }

            if (msg.startsWith('tempoBomba:') || msg.startsWith('setPumpTime:')) {
                const val = msg.includes('tempoBomba:') ? msg.substring(11) : msg.substring(12);
                const parsed = parseInt(val, 10);
                if (!isNaN(parsed)) {
                    pumpTimeEl.value = parsed;
                }
                return;
            }

            if (msg.startsWith('intervaloVerificacaoAgua:') || msg.startsWith('setWaterCheckInterval:')) {
                const val = msg.includes('intervaloVerificacaoAgua:') ? msg.substring(24) : msg.substring(22);
                const parsed = parseInt(val, 10);
                if (!isNaN(parsed)) {
                    waterIntervalEl.value = parsed;
                }
                return;
            }

            // Aktualna wilgotność z broadcastu
            if (msg.startsWith('{') && msg.includes('humidity')) {
                try {
                    const json = JSON.parse(msg);
                    if (json.humidity !== undefined) {
                        updateCurrentHumidity(json.humidity);
                    }
                } catch (e) {
                    // Ignoruj błędy parsowania
                }
                return;
            }

            // Potwierdzenia zapisu
            if (msg.startsWith('ok:setHumidity') || msg.startsWith('ok:setpointUmidade')) {
                ink.setStatus('Setpoint wilgotności zapisany', 'ok');
                return;
            }
            if (msg.startsWith('ok:setHumidityHysteresis')) {
                ink.setStatus('Histereza wilgotności zapisana', 'ok');
                return;
            }
            if (msg.startsWith('ok:setHumidityAlarmMax')) {
                ink.setStatus('Alarm max wilgotności zapisany', 'ok');
                return;
            }
            if (msg.startsWith('ok:setHumidityAlarmMin')) {
                ink.setStatus('Alarm min wilgotności zapisany', 'ok');
                return;
            }
            if (msg.startsWith('ok:setHumidityCalibration')) {
                ink.setStatus('Kalibracja wilgotności zapisana', 'ok');
                return;
            }
            if (msg.startsWith('ok:setPumpTime')) {
                ink.setStatus('Czas pompy zapisany', 'ok');
                return;
            }
            if (msg.startsWith('ok:setWaterCheckInterval')) {
                ink.setStatus('Interwał sprawdzania wody zapisany', 'ok');
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

        ink.wsSend('setHumidity:' + cfg.setpoint);
        ink.wsSend('setHumidityHysteresis:' + cfg.hysteresis);
        ink.wsSend('setHumidityAlarmMax:' + cfg.alarmMax);
        ink.wsSend('setHumidityAlarmMin:' + cfg.alarmMin);
        ink.wsSend('setHumidityCalibration:' + cfg.calibration.toFixed(1));
        ink.wsSend('setPumpTime:' + cfg.pumpTime);
        ink.wsSend('setWaterCheckInterval:' + cfg.waterInterval);

        ink.setStatus('Wysłano ustawienia wilgotności', 'ok');
    }

    function resetToDefaults() {
        if (!confirm('Przywrócić domyślne ustawienia wilgotności?')) return;

        const defaults = {
            setpoint: 55,
            hysteresis: 5,
            alarmMax: 75,
            alarmMin: 30,
            calibration: 0.0,
            pumpTime: 30,
            waterInterval: 12
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
            ink.wsSend('getHumidityConfig');
        }

        console.log('[humidity.js] Zainicjalizowano');
    });

})();