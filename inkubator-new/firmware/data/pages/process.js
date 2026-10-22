document.addEventListener('DOMContentLoaded', function() {
    if (!window.Inkubator) return;

    Inkubator.Events.subscribe('status.update', function(data) {
        const heater = data.heater || { ch0: 0, ch1: 0 };
        const avg = Math.round((Number(heater.ch0 || 0) + Number(heater.ch1 || 0)) / 2);
        const phase = avg > 1200 ? 'Heating' : avg > 400 ? 'Stabilizing' : 'Idle';
        const phaseEl = document.getElementById('procPhase');
        if (phaseEl) phaseEl.textContent = phase;
    });
});
