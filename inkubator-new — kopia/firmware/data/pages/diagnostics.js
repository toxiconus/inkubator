document.addEventListener('DOMContentLoaded', function() {
    if (!window.Inkubator) return;

    Inkubator.Events.subscribe('status.update', function(data) {
        const esp = data.esp || {};
        Inkubator.UI.updateMetric('diagCpu', esp.cpu ?? '--', '%');
        Inkubator.UI.updateMetric('diagFreeRam', esp.free_ram ?? '--', 'B');
    });
});
