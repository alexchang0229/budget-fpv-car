let config = {};

// Load current config
async function loadConfig() {
    config = await window.configAPI.loadGamepadConfig();
    populateDropdowns();
    setDropdownValues();
}

// Populate dropdown options
function populateDropdowns() {
    // Populate button dropdowns (0-19)
    const buttonSelects = [
        'throttleIndex',
        'reverseIndex',
        'steerOffsetRightIndex',
        'steerOffsetLeftIndex'
    ];

    buttonSelects.forEach(id => {
        const select = document.getElementById(id);
        select.innerHTML = '';
        for (let i = 0; i < 20; i++) {
            const option = document.createElement('option');
            option.value = i;
            option.textContent = `Button ${i}`;
            select.appendChild(option);
        }
    });

    // Populate axis dropdown (0-9)
    const axisSelect = document.getElementById('steeringIndex');
    axisSelect.innerHTML = '';
    for (let i = 0; i < 10; i++) {
        const option = document.createElement('option');
        option.value = i;
        option.textContent = `Axis ${i}`;
        axisSelect.appendChild(option);
    }
}

// Set dropdown values from config
function setDropdownValues() {
    Object.keys(config).forEach(key => {
        const select = document.getElementById(key);
        if (select) {
            select.value = config[key];
        }
    });
}

// Save settings
async function saveSettings() {
    const newConfig = {
        throttleIndex: parseInt(document.getElementById('throttleIndex').value),
        reverseIndex: parseInt(document.getElementById('reverseIndex').value),
        steeringIndex: parseInt(document.getElementById('steeringIndex').value),
        steerOffsetRightIndex: parseInt(document.getElementById('steerOffsetRightIndex').value),
        steerOffsetLeftIndex: parseInt(document.getElementById('steerOffsetLeftIndex').value)
    };

    const success = await window.configAPI.saveGamepadConfig(newConfig);
    await window.gamepadAPI.reloadConfig();

    const statusEl = document.getElementById('status');
    if (success) {
        statusEl.textContent = 'Settings saved successfully!';
        statusEl.className = 'status success';
        config = newConfig;
    } else {
        statusEl.textContent = 'Failed to save settings.';
        statusEl.className = 'status error';
    }

    setTimeout(() => {
        statusEl.className = 'status';
    }, 3000);
}

// Update live values
function updateLiveValues() {
    const gamepadState = window.gamepadAPI.getGamepadState();

    if (!gamepadState) {
        document.getElementById('no-gamepad').style.display = 'block';
        document.getElementById('buttons-container').innerHTML = '';
        document.getElementById('axes-container').innerHTML = '';
        requestAnimationFrame(updateLiveValues);
        return;
    }

    document.getElementById('no-gamepad').style.display = 'none';

    // Update buttons
    const buttonsContainer = document.getElementById('buttons-container');
    buttonsContainer.innerHTML = '';
    gamepadState.buttons.forEach(({ index, value, pressed }) => {
        const box = document.createElement('div');
        box.className = `value-box ${pressed ? 'active' : ''}`;
        box.innerHTML = `
            <div class="label">Button ${index}</div>
            <div class="value">${value.toFixed(2)}</div>
        `;
        buttonsContainer.appendChild(box);
    });

    // Update axes
    const axesContainer = document.getElementById('axes-container');
    axesContainer.innerHTML = '';
    gamepadState.axes.forEach(({ index, value }) => {
        const box = document.createElement('div');
        box.className = `value-box ${Math.abs(value) > 0.1 ? 'active' : ''}`;
        box.innerHTML = `
            <div class="label">Axis ${index}</div>
            <div class="value">${value.toFixed(2)}</div>
        `;
        axesContainer.appendChild(box);
    });

    requestAnimationFrame(updateLiveValues);
}

// Initialize
window.addEventListener('DOMContentLoaded', async () => {
    await loadConfig();

    // Add event listeners for buttons
    document.getElementById('saveButton').addEventListener('click', saveSettings);
    document.getElementById('backButton').addEventListener('click', () => {
        window.location.href = 'index.html';
    });

    if (navigator.getGamepads) {
        updateLiveValues();
    }
});
