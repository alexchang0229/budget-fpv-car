#include <WiFiManager.h>
#include <WebSocketsClient.h>
#include <ESP32Servo.h>
#include <Preferences.h>

#define MOTOR_PIN 1
#define SERVO_PIN 2

// WebSocket server address and port
char ws_host[40];  // e.g., "192.168.1.100" or "wss://example.com"
char ws_port[6];   // Port as string, e.g., "8080"

// Define the PWM channel, frequency, resolution, and pin
const int MotorPin = MOTOR_PIN;  // PWM pin for motor control
const int servoPin = SERVO_PIN;  // Servo control pin
const int bootButtonPin = 0;     // GPIO 0 - Boot button

#define RGB_BRIGHTNESS 64  // RGB LED brightness (max 255)

unsigned long currentMillis;
unsigned long prevMessageMillis = millis();  // Stores the last time a websocket message was recieved
unsigned long prevPingMillis = 0;            // Stores the last time a websocket message was recieved

// Connection status tracking
bool websocketConnected = false;
bool shouldSaveConfig = false;  // Flag for saving config


Servo steeringServo;  // Create Servo object
Servo MotorServo;
WebSocketsClient webSocket;
Preferences preferences;

void saveConfigCallback() {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void handleResetWifi() {
  // Check boot button for WiFi reset
  bool buttonState = digitalRead(bootButtonPin) == LOW;  // Button is active LOW

  if (buttonState) {
    Serial.println("Resetting WiFi settings...");
    rgbLedWrite(RGB_BUILTIN, 0, RGB_BRIGHTNESS, 0);  // Solid red during reset
    WiFiManager wifiManager;
    wifiManager.resetSettings();

    Serial.println("WiFi settings cleared! Restarting...");
    delay(1000);
    ESP.restart();
  }
}

void connectWebsocket() {
  // Setup WebSocket connection
  webSocket.begin(ws_host, atoi(ws_port), "/");
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(1000);  // Reconnect every 5 seconds if disconnected
}

void setup() {
  Serial.begin(115200);

  // Initialize boot button pin
  pinMode(bootButtonPin, INPUT_PULLUP);  // Boot button with internal pull-up

  // Initialize Preferences and load saved values
  preferences.begin("truck-config", false);
  strcpy(ws_host, preferences.getString("ws_host", "").c_str());
  strcpy(ws_port, preferences.getString("ws_port", "8000").c_str());

  // Connect to WiFi
  WiFiManager wifiManager;

  // Add custom parameters for WebSocket configuration with loaded values
  WiFiManagerParameter custom_ws_host("ws_host", "WebSocket Host", ws_host, 40);
  WiFiManagerParameter custom_ws_port("ws_port", "WebSocket Port", ws_port, 6);

  wifiManager.addParameter(&custom_ws_host);
  wifiManager.addParameter(&custom_ws_port);

  // Set callback to notify when config should be saved
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  // Build in LED red before Wifi connects
  rgbLedWrite(RGB_BUILTIN, 0, RGB_BRIGHTNESS, 0);

  Serial.print("Connecting to WiFi...");
  wifiManager.setConnectTimeout(10);
  wifiManager.autoConnect("truck-esp-32");
  Serial.println("\nConnected to WiFi");

  // Change LED to yellow once Wifi is connected
  rgbLedWrite(RGB_BUILTIN, RGB_BRIGHTNESS / 6, RGB_BRIGHTNESS, 0);

  // Get custom parameter values
  strcpy(ws_host, custom_ws_host.getValue());
  strcpy(ws_port, custom_ws_port.getValue());

  // Save config if needed (after user changed settings)
  if (shouldSaveConfig) {
    Serial.println("Saving config...");
    preferences.putString("ws_host", ws_host);
    preferences.putString("ws_port", ws_port);
    Serial.println("Config saved");
  }

  Serial.print("WebSocket Host: ");
  Serial.println(ws_host);
  Serial.print("WebSocket Port: ");
  Serial.println(ws_port);

  // Attach the servo to the servo pin
  steeringServo.attach(servoPin);
  MotorServo.attach(MotorPin);

  MotorServo.writeMicroseconds(1500);  // Neutral signal

  connectWebsocket();
}

void loop() {
  // Handle WebSocket events
  webSocket.loop();

  currentMillis = millis();

  // use boot button for WiFi reset
  handleResetWifi();

  if (currentMillis - prevPingMillis > 1000) {
    // Ping websocket server to check connection
    webSocket.sendTXT("ping");
    prevPingMillis = currentMillis;
  }
  //  if (currentMillis - prevMessageMillis > 3000)
  //  {
  //    // if more than 2 seconds have elapsed since last websocket message, stop motor
  //    MotorServo.write(90);
  //    if (webSocket.isConnected())
  //    {
  //      // When the network changes the websocket doesn't disconnect properly, disconnect here when no more heart beat
  //      webSocket.disconnect();
  //      prevMessageMillis = currentMillis;
  //    }
  //  }
}

int lastMotorSpeedMicroseconds = 90;  // Start at neutral

void webSocketEvent(WStype_t type, uint8_t *payload, size_t length) {

  switch (type) {
    case WStype_ERROR:
      Serial.println("ERROR!");
    case WStype_DISCONNECTED:
      Serial.println("WS Disconnected!");
      websocketConnected = false;
      // LED yellow
      rgbLedWrite(RGB_BUILTIN, RGB_BRIGHTNESS / 6, RGB_BRIGHTNESS, 0);

      // If websocket gets disconnected, stop motor
      MotorServo.write(90);  // Neutral signal
      break;
    case WStype_CONNECTED:
      Serial.println("WS Connected");
      websocketConnected = true;
      // LED green
      rgbLedWrite(RGB_BUILTIN, RGB_BRIGHTNESS, 0, 0);
      // send message to server when Connected
      webSocket.sendTXT("ping");
      break;
    case WStype_TEXT:
      String data = String((char *)payload);
      Serial.print("Received: ");
      Serial.println(data);
      prevMessageMillis = millis();
      // Parse motor speed and servo position
      int commaIndex = data.indexOf(',');
      if (commaIndex != -1) {
        String motorSpeedString = data.substring(0, commaIndex);
        String servoPosString = data.substring(commaIndex + 1);

        int motorSpeed = motorSpeedString.toInt();
        int servoPos = servoPosString.toInt();

        // Constrain values and control motor and servo
        motorSpeed = constrain(motorSpeed, 10, 170);  // ESC range
        servoPos = constrain(servoPos, 0, 180);

        // Check if we're switching from forward to reverse
        if (motorSpeed < 90 && lastMotorSpeedMicroseconds >= 90) {
          // Transition from forward to reverse
          MotorServo.write(80);
          delay(100);
          MotorServo.write(90);
          delay(100);
        }
        // Store the last motor speed
        lastMotorSpeedMicroseconds = motorSpeed;

        // Write the PWM signal in microseconds for motor
        MotorServo.write(motorSpeed);

        // Set servo position
        steeringServo.write(servoPos);

        // Debug output
        Serial.print("Motor speed set to: ");
        Serial.println(motorSpeed);
        Serial.print("Servo position set to: ");
        Serial.println(servoPos);
      }
      break;
  }
}
