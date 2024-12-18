window.api.onConnectionStatusUpdate((event, isConnected) => {
    const statusElement = document.getElementById('connection-status');
    statusElement.textContent = isConnected ? 'Connected' : 'Disconnected';
    statusElement.style.color = isConnected ? 'green' : 'red';
});

document.addEventListener('keydown', (event) => {
    const keyData = { key: event.key, code: event.code };
    window.api.sendKeydownEvent(keyData);
});

document.addEventListener('keyup', (event) => {
    const keyData = { key: event.key, code: event.code };
    window.api.sendKeyupEvent(keyData);
});

window.addEventListener('DOMContentLoaded', () => {
    if (navigator.getGamepads) {
        window.gamepadAPI.startPolling();
    }
});

window.addEventListener('gamepad-data', (event) => {
    const { throttle, steering, steeringOffset } = event.detail;

    const throttlePosition = (throttle - 20) / (160 - 20) * 100;
    document.getElementById('throttle-indicator').style.left = `calc(${throttlePosition}% - 5px)`;

    const steeringPosition = (steering / 180) * 100;
    document.getElementById('steering-indicator').style.right = `calc(${steeringPosition}% - 5px)`;

    const offsetPosition = (steeringOffset + 90) / 180 * 100;
    document.getElementById('offset-indicator').style.right = `calc(${offsetPosition}% - 5px)`;
});
