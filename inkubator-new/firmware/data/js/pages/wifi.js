(function() {
    'use strict';

    const ink = window.Inkubator;
    const ssidEl = document.getElementById('wifiSsid');
    const passEl = document.getElementById('wifiPass');
    const statusVal = document.getElementById('wifiStatus');
    const saveBtn = document.getElementById('wifiSaveBtn');
    const removeBtn = document.getElementById('wifiRemoveBtn');
    const refreshBtn = document.getElementById('wifiRefreshBtn');
    const backBtn = document.getElementById('backBtn');

    if (!ssidEl || !passEl || !statusVal || !saveBtn || !removeBtn || !refreshBtn || !backBtn) {
        return;
    }

    function updateWifiStatusText(text, type = 'info') {
        statusVal.textContent = text;
        statusVal.style.color = type === 'err' ? '#e05050' : type === 'ok' ? '#70d070' : '#8a9aa8';
    }

    ink.onWsEvent(function(type, ws, data) {
        if (type === 'open') {
            ink.wsSend('carregarRede');
            ink.wsSend('getWifiStatus');
            return;
        }
        if (type !== 'message') return;

        const msg = data;
        if (msg.startsWith('wifiConfig:')) {
            const payload = msg.substring(11);
            const parts = payload.split(',');
            parts.forEach(part => {
                const [key, value] = part.split('=');
                if (key === 'ssid') ssidEl.value = value || '';
            });
            ink.setStatus('Pobrano konfigurację sieci', 'ok');
            return;
        }
        if (msg.startsWith('wifiStatus:')) {
            const status = msg.substring(11);
            updateWifiStatusText(status === 'connected' ? 'Połączono' : 'Brak połączenia', status === 'connected' ? 'ok' : 'err');
            return;
        }
        if (msg.startsWith('ok:setWifi') || msg.startsWith('ok:salvarRede')) {
            ink.setStatus('Sieć zapisana', 'ok');
            return;
        }
        if (msg.startsWith('ok:removeWifi')) {
            ink.setStatus('Sieć usunięta', 'ok');
            ssidEl.value = '';
            passEl.value = '';
            updateWifiStatusText('Usunięto', 'info');
            return;
        }
    });

    saveBtn.addEventListener('click', function() {
        const ssid = ssidEl.value.trim();
        const pass = passEl.value.trim();
        if (!ssid) {
            ink.setStatus('SSID nie może być pusty', 'err');
            return;
        }
        if (pass.length > 0 && pass.length < 8) {
            ink.setStatus('Hasło musi mieć minimum 8 znaków', 'err');
            return;
        }
        ink.wsSend('salvarRede:' + ssid + ',' + pass);
        ink.setStatus('Wysłano konfigurację Wi-Fi', 'info');
    });

    removeBtn.addEventListener('click', function() {
        if (!confirm('Usunąć zapisane dane sieci Wi-Fi?')) return;
        ink.wsSend('removerRede');
        ink.setStatus('Wysłano usunięcie sieci', 'info');
    });

    refreshBtn.addEventListener('click', function() {
        ink.wsSend('carregarRede');
        ink.wsSend('getWifiStatus');
        ink.setStatus('Odswiezanie ustawien...', 'info');
    });

    backBtn.addEventListener('click', function() {
        window.location.href = '/';
    });
})();
