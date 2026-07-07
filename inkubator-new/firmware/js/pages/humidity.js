const humidityWsUrl = getWebSocketUrl();
let humidityWs = null;

function connectHumidityWebSocket() {
  humidityWs = createWebSocket();
  humidityWs.addEventListener('open', () => setText('humidityStatus', 'connected'));
  humidityWs.addEventListener('message', event => {
    console.log('humidity message', event.data);
  });
}

window.addEventListener('DOMContentLoaded', () => {
  connectHumidityWebSocket();
});
