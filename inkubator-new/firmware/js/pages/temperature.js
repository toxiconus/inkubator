const temperatureWsUrl = getWebSocketUrl();
let temperatureWs = null;

function connectTemperatureWebSocket() {
  temperatureWs = createWebSocket();
  temperatureWs.addEventListener('open', () => setText('connectionStatus', 'connected'));
  temperatureWs.addEventListener('message', event => {
    // message handling here
    console.log('temperature message', event.data);
  });
}

window.addEventListener('DOMContentLoaded', () => {
  connectTemperatureWebSocket();
});
