(function() {
    'use strict';

    const ink = window.Inkubator;
    if (!ink) {
        console.error('[ajustes.js] Brak wspólnego modułu Inkubator');
        return;
    }

    // ============================================================
    //  OBSŁUGA WEBSOCKET
    // ============================================================
    ink.onWsEvent(function(type, ws, data) {
        if (type === 'open') {
            ink.setStatus('Połączono', 'ok');
            // Opcjonalnie: pobierz jakieś dane
            // ink.wsSend('carregarConfiguracao');
            return;
        }

        if (type === 'message') {
            const msg = data;
            // Obsługa wiadomości – jeśli potrzebna
            // np. wyświetlenie wersji firmware
            if (msg.startsWith('versaoFirmware:')) {
                const version = msg.substring(15);
                ink.setStatus('Wersja firmware: ' + version, 'info');
            }
        }
    });

    // ============================================================
    //  PRZYCISK POWROTU – już w HTML jako link, ale dodajemy dla spójności
    // ============================================================
    document.addEventListener('DOMContentLoaded', function() {
        // Wszystkie linki w siatce to już <a href="...">, więc nie potrzebują JS
        // Ale możemy dodać logowanie kliknięć dla debugu
        document.querySelectorAll('.btn-primary').forEach(function(btn) {
            btn.addEventListener('click', function(e) {
                console.log('[ajustes] Przejście do:', this.getAttribute('href'));
            });
        });
    });

})();
