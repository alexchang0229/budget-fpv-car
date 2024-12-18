const { contextBridge, ipcRenderer } = require('electron');

contextBridge.exposeInMainWorld('api', {
    onConnectionStatusUpdate: (callback) => ipcRenderer.on('update-connection-status', callback),
    sendKeydownEvent: (keyData) => ipcRenderer.send('keydown-event', keyData),
    sendKeyupEvent: (keyData) => ipcRenderer.send('keyup-event', keyData),
});


// Gamepad buttons
const throttleIndex = 7;
const reverseIndex = 6;
const steeringIndex = 0;
const steerOffsetRightIndex = 14;
const steerOffsetLeftIndex = 15;

var steeringOffset = 0;


contextBridge.exposeInMainWorld('gamepadAPI', {
    startPolling: () => {
        const pollGamepad = () => {
            const gamepads = navigator.getGamepads();
            if (gamepads[0]) { // Assuming the first gamepad
                const { axes, buttons } = gamepads[0];

                const throttle = Math.round(buttons[throttleIndex].value * 70 + buttons[reverseIndex].value * -70 + 90); //map throttle to 20-160
                const steering = Math.round((((-axes[steeringIndex] + 1) / 2) * 170)); // map steering to 0-180
                var _steeringOffset = 0
                if (buttons[steerOffsetRightIndex].pressed == true) {
                    _steeringOffset = 0.5
                }
                if (buttons[steerOffsetLeftIndex].pressed == true) {
                    _steeringOffset = -0.5
                }
                if (_steeringOffset) {
                    steeringOffset = _steeringOffset + steeringOffset
                }
                ipcRenderer.send('gamepad-data', { throttle, steering, steeringOffset });
                window.dispatchEvent(new CustomEvent('gamepad-data', {
                    detail: { throttle, steering, steeringOffset }
                }));
            }
            requestAnimationFrame(pollGamepad);
        };
        pollGamepad();
    }
});