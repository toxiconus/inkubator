document.addEventListener('DOMContentLoaded', function() {
    if (!window.Inkubator) return;

    Inkubator.Events.subscribe('status.update', function(data) {
        Inkubator.UI.updateMetric('tempValue', Inkubator.Utils.round(data.temperature ?? 0, 2));
        Inkubator.UI.updateMetric('rhValue', Inkubator.Utils.round(data.humidity ?? 0, 1));
        Inkubator.UI.updateHealth('healthWifi', !!(data.wifi && data.wifi.connected));
    });
});
