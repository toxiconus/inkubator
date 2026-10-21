document.addEventListener('DOMContentLoaded', function() {
    if (!window.Inkubator) return;

    const prefs = Inkubator.Storage.load('hmi-settings', { debug: false });
    if (typeof prefs.debug === 'boolean') {
        Inkubator.Config.debug = prefs.debug;
    }
});
