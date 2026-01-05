const { contextBridge, ipcRenderer } = require('electron');

contextBridge.exposeInMainWorld('api', {
    onConnectionStatusUpdate: (callback) => ipcRenderer.on('update-connection-status', callback),
    sendKeydownEvent: (keyData) => ipcRenderer.send('keydown-event', keyData),
    sendKeyupEvent: (keyData) => ipcRenderer.send('keyup-event', keyData),
});

contextBridge.exposeInMainWorld('configAPI', {
    loadGamepadConfig: () => ipcRenderer.invoke('load-gamepad-config'),
    saveGamepadConfig: (config) => ipcRenderer.invoke('save-gamepad-config', config)
});

// Gamepad config variable
let gamepadConfig = null;
var steeringOffset = 0;

// Load gamepad config
async function loadGamepadConfig() {
    gamepadConfig = await ipcRenderer.invoke('load-gamepad-config');
}

// Initialize config on load
loadGamepadConfig();


contextBridge.exposeInMainWorld('gamepadAPI', {
    startPolling: () => {
        const pollGamepad = () => {
            const gamepads = navigator.getGamepads();
            if (gamepads[0] && gamepadConfig) { // Assuming the first gamepad
                const { axes, buttons } = gamepads[0];

                const throttle = Math.round(buttons[gamepadConfig.throttleIndex].value * 70 + buttons[gamepadConfig.reverseIndex].value * -70 + 90); //map throttle to 20-160
                const steering = Math.round((((-axes[gamepadConfig.steeringIndex] + 1) / 2) * 170)); // map steering to 0-180
                var _steeringOffset = 0
                if (buttons[gamepadConfig.steerOffsetRightIndex]?.pressed == true) {
                    _steeringOffset = 0.5
                }
                if (buttons[gamepadConfig.steerOffsetLeftIndex]?.pressed == true) {
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
    },

    // Get raw gamepad state for settings page
    getGamepadState: () => {
        const gamepads = navigator.getGamepads();
        if (gamepads[0]) {
            const { axes, buttons } = gamepads[0];
            return {
                axes: Array.from(axes).map((value, index) => ({ index, value })),
                buttons: Array.from(buttons).map((button, index) => ({
                    index,
                    value: button.value,
                    pressed: button.pressed
                }))
            };
        }
        return null;
    },

    // Reload config after saving
    reloadConfig: async () => {
        await loadGamepadConfig();
    }
});