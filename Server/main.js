import { app, BrowserWindow, ipcMain } from 'electron';
import WebSocket, { WebSocketServer } from 'ws';
import * as path from 'path';
import { fileURLToPath } from 'url';
import fs from 'fs';

const CONFIG_PATH = path.join(app.getPath('userData'), 'gamepad-config.json');
const DEFAULT_CONFIG = {
    throttleIndex: 7,
    reverseIndex: 6,
    steeringIndex: 0,
    steerOffsetRightIndex: 14,
    steerOffsetLeftIndex: 15
};

// Load or create config
function loadConfig() {
    try {
        if (fs.existsSync(CONFIG_PATH)) {
            return JSON.parse(fs.readFileSync(CONFIG_PATH, 'utf8'));
        }
    } catch (err) {
        console.error('Error loading config:', err);
    }
    return DEFAULT_CONFIG;
}

// Save config
function saveConfig(config) {
    try {
        console.log(CONFIG_PATH)
        fs.writeFileSync(CONFIG_PATH, JSON.stringify(config, null, 2));
        return true;
    } catch (err) {
        console.error('Error saving config:', err);
        return false;
    }
}

let isConnected = false;
let win;

const createWindow = () => {
    const __filename = fileURLToPath(import.meta.url);
    const __dirname = path.dirname(__filename);

    win = new BrowserWindow({
        width: 800,
        height: 600,
        webPreferences: {
            preload: `${__dirname}/preload.js`,
            contextIsolation: true,
            enableRemoteModule: false,
            nodeIntegration: false
        }
    })

    win.loadFile('index.html')
}

app.whenReady().then(() => {
    createWindow()
    var throttle = 90;
    var steering = 90;

    // IPC handlers for gamepad config
    ipcMain.handle('load-gamepad-config', () => {
        return loadConfig();
    });

    ipcMain.handle('save-gamepad-config', (_event, config) => {
        return saveConfig(config);
    });

    ipcMain.on('gamepad-data', (event, data) => {
        const { throttle, steering, steeringOffset } = data;
        // console.log(`Gamepad Throttle: ${throttle}, Steering: ${steering}, offset: ${steeringOffset}`);

        wss.clients.forEach((client) => {
            if (client.readyState === WebSocket.OPEN) {
                client.send(`${throttle},${steering + steeringOffset}`);
            }
        });
    });

    const wss = new WebSocketServer({ port: 8000, host: '0.0.0.0' });

    const updateConnectionStatus = () => {
        isConnected = wss.clients.size > 0;
        win.webContents.send('update-connection-status', isConnected);
    };

    wss.on('connection', function connection(ws) {
        console.log('Client connected');
        updateConnectionStatus();

        // Track heartbeat
        ws.isAlive = true;
        ws.lastHeartbeat = Date.now();

        // Check heartbeat every second
        const heartbeatInterval = setInterval(() => {
            const timeSinceLastHeartbeat = Date.now() - ws.lastHeartbeat;
            if (timeSinceLastHeartbeat > 5000) {
                console.log('Client heartbeat timeout - disconnecting');
                clearInterval(heartbeatInterval);
                ws.terminate();
            }
        }, 1000);

        ws.on('close', () => {
            console.log('Client disconnected');
            clearInterval(heartbeatInterval);
            updateConnectionStatus();
        });

        ws.on('error', console.error);

        ws.on('message', function message(data, isBinary) {
            const message = isBinary ? data : data.toString();
            if (message == "ping") {
                ws.isAlive = true;
                ws.lastHeartbeat = Date.now();
                ws.send("pong");
            }
        });
    });

    ipcMain.on('keydown-event', (event, keyData) => {
        console.log(`Key down: ${keyData.key}, Code: ${keyData.code}`);
        if (keyData.code === "ArrowDown") {
            throttle = 60;
        }
        if (keyData.code === "ArrowUp") {
            throttle = 120;
        }
        if (keyData.code === "ArrowLeft") {
            steering = Math.min(180, steering + 20)
        }
        if (keyData.code === "ArrowRight") {
            steering = Math.max(0, steering - 20)
        }

        wss.clients.forEach((client) => {
            if (client.readyState === WebSocket.OPEN) {
                client.send(`${throttle},${steering}`);
            }
        });
        // Handle the keydown event as needed
    });
    ipcMain.on('keyup-event', (event, keyData) => {
        console.log(`Key up: ${keyData.key}, Code: ${keyData.code}`);

        if (keyData.code === "ArrowUp" || keyData.code === "ArrowDown") {
            throttle = 90
        }
        wss.clients.forEach((client) => {
            if (client.readyState === WebSocket.OPEN) {
                client.send(`${throttle},${steering}`);
            }
        });
        // Handle the keydown event as needed
    });

    app.on('activate', () => {
        if (BrowserWindow.getAllWindows().length === 0) createWindow()


    })
})

app.on('window-all-closed', async () => {
    if (process.platform !== 'darwin') app.quit()
})

