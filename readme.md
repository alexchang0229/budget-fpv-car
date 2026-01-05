## What?
A budget-friendly, unlimited control range, FPV RC car controlled via WebSocket connection. The system uses an ESP32-S3 microcontroller connected to your WiFi network, communicating with an Electron-based control server on your computer. Control the car using keyboard arrow keys or a gamepad/racing wheel, and get live video feedback through your phone mounted on the car.

### Bill of Materials
| Item | Price ($CAD) | Link |
|---|---|---|
| Waveshare ESP32-S3 MINI | 15 | [Amazon](https://www.amazon.ca/Waveshare-Development-ESP32-C6FH4-Processors-Frequency/dp/B0CZKM6HVH/ref=sr_1_10?crid=3986S01I0ODQR&dib=eyJ2IjoiMSJ9.rwc7Jthi8UOgGLGTdx5ArF-SJgfhuzdkrt-raBK-RKyJg8berLR6tnK_rFTWvUIFNwbRWtgf2gaF3tMxfgdHUpYFL0sCqdHtRVLsUUOKDB0gcgGBZLk9N899jgpEWgiGJj-3IYZGFbN-iuZLonV4wZb7SlQWUwAv4hZYarlRcudPxEUg8DiY8sdBsOLZ3NnkQUWS5FtLxozqkALuuj2S53thyCK557-T9_xw09IOSuqb_nHDYXR1ky2Q7N-55IzHeK9MFhOSLO42QK3w2C9YPwOPXLCSRGYJr-sEzpWp23xUVMAY-82oGh7OMFausGJX2Q_kBvMaxq9xScWokYUUUMCSvdqwlKnhnUchc_vMufT8.TEJ3oA-U_95Hm7j-1m7SM4fexN5oCe8jLTWwvERAge0&dib_tag=se&keywords=esp32s3+mini&qid=1734548848&s=electronics&sprefix=esp32s3+mini%2Celectronics%2C88&sr=1-10) |
| RC Kei truck (WPL D14) | 80 | [Amazon](https://www.amazon.ca/dp/B0CBPXJCB3?ref=ppx_yo2ov_dt_b_fed_asin_title) |
| RC car ESC for WPLC14 | 20 | [Amazon](https://www.amazon.ca/gp/product/B08FBVBKGN/ref=ppx_yo_dt_b_asin_title_o00_s00?ie=UTF8&psc=1) |
| Your old phone | Free.99 | Check your drawers |
| Solder iron | Free.99 | Check tool box |
| Hook-up wires | Free.99 | Check night stand #1 |
| Perfboard | Free.99 | Check drawers |
| Xbox controller or Sim racing wheel (optional) | 50 | Facebook marketplace

## How?

### Architecture
- **ESP32-S3**: Connects to your WiFi network and establishes a WebSocket connection to the server. Receives control commands (throttle and steering) and controls the car's ESC and steering servo.
- **Electron Server**: Runs on your computer, hosts a WebSocket server on port 8000 (bound to 0.0.0.0), and provides a GUI showing connection status and control inputs.
- **Control Options**:
  - Keyboard: Arrow keys for throttle (up/down) and steering (left/right)
  - Gamepad: Analog triggers for throttle/reverse, analog stick for steering, D-pad for steering offset calibration
- **Video Feed**: Use vdo.ninja to stream live video from your phone mounted on the car

### Server Setup
This is the WebSocket server that runs on your computer and controls the RC car. It is built with Electron.js.

1. Install [Node.js](https://nodejs.org/) on your computer
2. Navigate to the `/Server` directory
3. Install dependencies:
   ```bash
   npm install
   ```
4. Start the server:
   ```bash
   npm start
   ```
5. Configure your router to forward port 8000 to your computer's local IP address (required for external access)

**Control Interface:**
- The Electron window displays connection status (green = connected, red = disconnected)
- Visual indicators show throttle, steering, and steering offset values
- Use keyboard arrow keys or connect a gamepad for control

### RC Car Setup

1. **Configure the ESP32:**
   - Copy `TruckScript/config_example.h` to `TruckScript/config.h`
   - Edit `config.h` with your settings:
     ```c
     #define SSID "<Your WiFi network name>"
     #define PASSWORD "<Your WiFi password>"
     #define WS_HOST "<Your computer's IP address>"
     #define WS_PORT 8000
     #define MOTOR_PIN 1    // GPIO pin for ESC
     #define SERVO_PIN 4    // GPIO pin for steering servo
     ```

2. **Upload the firmware:**
   - Install the [Arduino IDE](https://www.arduino.cc/en/software)
   - Install the ESP32 board support and required libraries:
     - ESP32Servo
     - WebSocketsClient
   - Open `TruckScript/TruckScript.ino`
   - Select your ESP32-S3 board from Tools > Board
   - Upload the sketch

3. **Set up FPV video:**
   - Create a room at [vdo.ninja](https://vdo.ninja/)
   - Join the room from your computer (view mode)
   - Join the same room from your old phone (camera mode)
   - Securely mount your phone to the RC car

4. **Wire the hardware:**
   - Connect the steering servo to GPIO 4 (configurable in config.h)
   - Connect the ESC control wire to GPIO 1 (configurable in config.h)
   - Ensure the ESP32 and servos share a common ground

5. Power on and drive! 🚚💨

### How It Works

**Communication Protocol:**
- ESP32 connects to WebSocket server and sends "ping" every second
- Server responds with "pong" to maintain connection
- Control commands are sent as comma-separated values: `<throttle>,<steering>`
  - Throttle range: 10-170 (90 = neutral, <90 = reverse, >90 = forward)
  - Steering range: 0-180 (90 = center)
- When disconnected, the car automatically stops (motor set to neutral)

**Safety Features:**
- Automatic motor stop when WebSocket disconnects
- Forward-to-reverse transition includes double-pulse braking sequence
- Value constraints prevent out-of-range commands

## Who?
This project was made by [Alex Chang](https://www.linkedin.com/in/alexyuchang/). 