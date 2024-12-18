import { app, BrowserWindow, ipcMain } from 'electron';
import WebSocket, { WebSocketServer } from 'ws';
import * as path from 'path';
import { fileURLToPath } from 'url';

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
        updateConnectionStatus();
        ws.on('close', updateConnectionStatus)
        ws.on('error', console.error);
        ws.on('message', function message(data, isBinary) {
            const message = isBinary ? data : data.toString();
            if (message == "ping") {
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

