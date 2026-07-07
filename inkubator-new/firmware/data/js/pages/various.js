(function() {
    'use strict';

    const ink = window.Inkubator;
    if (!ink) {
        console.error('[various.js] Brak wspólnego modułu Inkubator');
        return;
    }

    // ============================================================
    //  REFERENCJE DO ELEMENTÓW
    // ============================================================
    const rtcTime = document.getElementById('rtcTime');
    const fwVersion = document.getElementById('fwVersion');
    const langSelect = document.getElementById('langSelect');

    const ntpBtn = document.getElementById('ntpBtn');
    const rtcRefreshBtn = document.getElementById('rtcRefreshBtn');
    const langBtn = document.getElementById('langBtn');
    const otaBtn = document.getElementById('otaBtn');
    const otaFile = document.getElementById('otaFile');
    const resetBtn = document.getElementById('resetBtn');
    const rebootBtn = document.getElementById('rebootBtn');
    const backBtn = document.getElementById('backBtn');

    const otaProgress = document.getElementById('otaProgress');
    const otaProgressBar = document.getElementById('otaProgressBar');
    const otaProgressText = document.getElementById('otaProgressText');

    // ============================================================
    //  FUNKCJE POMOCNICZE
    // ============================================================
    function updateRtcTime(timeStr) {
        if (!rtcTime) return;
        if (timeStr && timeStr !== 'Brak RTC') {
            rtcTime.textContent = timeStr;
            rtcTime.className = 'val ok';
        } else {
            rtcTime.textContent = '--:-- --.--.----';
            rtcTime.className = 'val err';
        }
    }

    function updateFwVersion(version) {
        if (!fwVersion) return;
        if (version) {
            fwVersion.textContent = version;
        } else {
            fwVersion.textContent = '--';
        }
    }

    function updateOtaProgress(pct) {
        if (!otaProgress || !otaProgressBar || !otaProgressText) return;

        if (pct === null || pct === undefined) {
            otaProgress.style.display = 'none';
            return;
        }

        otaProgress.style.display = 'block';
        const clamped = Math.max(0, Math.min(100, pct));
        otaProgressBar.style.width = clamped + '%';
        otaProgressText.textContent = clamped + '%';
    }

    // ============================================================
    //  OBSŁUGA WEBSOCKET
    // ============================================================
    ink.onWsEvent(function(type, ws, data) {
        if (type === 'open') {
            ink.setStatus('Połączono, pobieram dane...', 'info');
            ws.send('carregarRTC');
            ws.send('versaoFirmware');
            ws.send('carregarIdioma');
            return;
        }

        if (type === 'message') {
            const msg = data;

            if (msg.startsWith('rtcTime:')) {
                const time = msg.substring(8);
                updateRtcTime(time);
                ink.setStatus('Odczytano czas RTC', 'ok');
                return;
            }

            if (msg.startsWith('versaoFirmware:')) {
                const version = msg.substring(15);
                updateFwVersion(version);
                ink.setStatus('Wersja firmware: ' + version, 'info');
                return;
            }

            if (msg.startsWith('idiomaAtual:')) {
                const lang = msg.substring(12).trim();
                if (langSelect) {
                    for (const option of langSelect.options) {
                        if (option.value === lang) {
                            option.selected = true;
                            break;
                        }
                    }
                }
                ink.setStatus('Język: ' + lang, 'ok');
                return;
            }

            if (msg.startsWith('ok:idioma')) {
                ink.setStatus('Język zapisany', 'ok');
                return;
            }

            if (msg.startsWith('ok:syncNtp') || msg.startsWith('ok:sincronizarNTP')) {
                ink.setStatus('Synchronizacja NTP zakończona', 'ok');
                // Odśwież czas
                setTimeout(function() {
                    ink.wsSend('carregarRTC');
                }, 500);
                return;
            }

            if (msg.startsWith('ok:redefinir')) {
                ink.setStatus('Przywrócono ustawienia fabryczne', 'ok');
                return;
            }

            if (msg.startsWith('ok:reiniciar')) {
                ink.setStatus('Restart ESP...', 'info');
                return;
            }
        }
    });

    // ============================================================
    //  FUNKCJE AKCJI
    // ============================================================
    function syncNTP() {
        ink.wsSend('sincronizarNTP');
        ink.setStatus('Wysyłam synchronizację NTP...', 'info');
    }

    function refreshRTC() {
        ink.wsSend('carregarRTC');
        ink.setStatus('Odświeżam czas RTC...', 'info');
    }

    function saveLanguage() {
        const lang = langSelect ? langSelect.value : 'pl';
        ink.wsSend('idioma:' + lang);
        ink.setStatus('Wysyłam język: ' + lang, 'info');
    }

    function doOTA() {
        if (!otaFile || !otaFile.files || otaFile.files.length === 0) {
            ink.setStatus('Wybierz plik .bin', 'err');
            return;
        }

        const file = otaFile.files[0];
        if (!file.name.endsWith('.bin')) {
            ink.setStatus('Wybierz plik .bin', 'err');
            return;
        }

        const formData = new FormData();
        formData.append('update', file);

        updateOtaProgress(0);
        ink.setStatus('Rozpoczynam OTA...', 'info');

        const xhr = new XMLHttpRequest();
        xhr.open('POST', '/update', true);

        xhr.upload.onprogress = function(e) {
            if (e.lengthComputable) {
                const pct = Math.round((e.loaded / e.total) * 100);
                updateOtaProgress(pct);
                ink.setStatus('OTA: ' + pct + '%', 'info');
            }
        };

        xhr.onload = function() {
            if (xhr.status === 200) {
                ink.setStatus('OTA zakończony sukcesem – restart...', 'ok');
                updateOtaProgress(100);
                setTimeout(function() {
                    window.location.reload();
                }, 3000);
            } else {
                ink.setStatus('Błąd OTA: ' + xhr.status, 'err');
                updateOtaProgress(null);
            }
        };

        xhr.onerror = function() {
            ink.setStatus('Błąd połączenia przy OTA', 'err');
            updateOtaProgress(null);
        };

        xhr.send(formData);
    }

    function factoryReset() {
        if (!confirm('Przywrócić ustawienia fabryczne? Ta operacja jest nieodwracalna.')) return;
        ink.wsSend('redefinir');
        ink.setStatus('Wysyłam reset fabryczny...', 'info');
    }

    function reboot() {
        if (!confirm('Zrestartować ESP?')) return;
        ink.wsSend('reiniciar');
        ink.setStatus('Wysyłam restart...', 'info');
    }

    function goBack() {
        window.location.href = '/';
    }

    // ============================================================
    //  INICJALIZACJA
    // ============================================================
    document.addEventListener('DOMContentLoaded', function() {
        if (ntpBtn) ntpBtn.addEventListener('click', syncNTP);
        if (rtcRefreshBtn) rtcRefreshBtn.addEventListener('click', refreshRTC);
        if (langBtn) langBtn.addEventListener('click', saveLanguage);
        if (otaBtn) otaBtn.addEventListener('click', doOTA);
        if (resetBtn) resetBtn.addEventListener('click', factoryReset);
        if (rebootBtn) rebootBtn.addEventListener('click', reboot);
        if (backBtn) backBtn.addEventListener('click', goBack);

        // Jeśli WebSocket jest już połączony
        if (ink.wsConnected()) {
            ink.wsSend('carregarRTC');
            ink.wsSend('versaoFirmware');
            ink.wsSend('carregarIdioma');
        }

        // Ukryj pasek postępu OTA na start
        updateOtaProgress(null);

        console.log('[various.js] Zainicjalizowano');
    });

})();