#include <WiFiManager.h>
#include <WebSocketsClient.h>
#include <ESP32Servo.h>
#include <config.h>


// WebSocket server address and port
char ws_host[40] = WS_HOST; // e.g., "192.168.1.100" or "wss://example.com"
char ws_port_str[6] = "8000";
uint16_t ws_port = WS_PORT;

// Define the PWM channel, frequency, resolution, and pin
const int MotorPin = MOTOR_PIN; // PWM pin for motor control
const int servoPin = SERVO_PIN; // Servo control pin
const int bootButtonPin = 0; // GPIO 0 - Boot button

#define RGB_BRIGHTNESS 64 // RGB LED brightness (max 255)

unsigned long currentMillis;
unsigned long prevMessageMillis = millis(); // Stores the last time a websocket message was recieved
unsigned long prevPingMillis = 0;           // Stores the last time a websocket message was recieved
unsigned long buttonPressStart = 0;         // Stores when button was first pressed
bool buttonPressed = false;                 // Tracks button state
bool ledState = false;                      // LED blink state
unsigned long prevLedToggle = 0;            // Last LED toggle time
const unsigned long resetHoldTime = 3000;   // Hold button for 3 seconds to reset

// Connection status tracking
bool websocketConnected = false;
unsigned long prevStatusLedUpdate = 0;
bool statusLedState = false;


Servo steeringServo; // Create Servo object
Servo MotorServo;    // Create Servo object
WebSocketsClient webSocket;

void handleBootButton()
{
  // Check boot button for WiFi reset
  bool buttonState = digitalRead(bootButtonPin) == LOW; // Button is active LOW

  if (buttonState && !buttonPressed)
  {
    // Button just pressed
    buttonPressed = true;
    buttonPressStart = currentMillis;
    Serial.println("Boot button pressed - hold for 3 seconds to reset WiFi settings");
  }
  else if (!buttonState && buttonPressed)
  {
    // Button released
    buttonPressed = false;
    rgbLedWrite(RGB_BUILTIN, 0, 0, 0); // Turn off RGB LED
    ledState = false;
    Serial.println("Boot button released");
  }

  // If button is being held, flash LED and check for reset
  if (buttonPressed)
  {
    unsigned long holdDuration = currentMillis - buttonPressStart;

    // Flash LED while button is held
    if (currentMillis - prevLedToggle > 100) // Toggle every 100ms
    {
      ledState = !ledState;
      if (ledState)
      {
        rgbLedWrite(RGB_BUILTIN, 0, RGB_BRIGHTNESS, 0); // Red when flashing
      }
      else
      {
        rgbLedWrite(RGB_BUILTIN, 0, 0, 0); // Off
      }
      prevLedToggle = currentMillis;
    }

    // Check if button has been held long enough to reset
    if (holdDuration >= resetHoldTime)
    {
      Serial.println("Resetting WiFi settings...");
      rgbLedWrite(RGB_BUILTIN, RGB_BRIGHTNESS, 0, 0); // Solid red during reset
      WiFiManager wifiManager;
      wifiManager.resetSettings();

      Serial.println("WiFi settings cleared! Restarting...");
      delay(1000);
      ESP.restart();
    }
  }
}

void updateStatusLed()
{
#ifdef RGB_BUILTIN
  // Don't override button press LED indication
  if (buttonPressed)
  {
    return;
  }

  bool wifiConnected = WiFi.status() == WL_CONNECTED;

  if (!wifiConnected)
  {
    // Blink red when WiFi not connected (500ms interval)
    if (currentMillis - prevStatusLedUpdate > 500)
    {
      statusLedState = !statusLedState;
      if (statusLedState)
      {
        rgbLedWrite(RGB_BUILTIN, 0, RGB_BRIGHTNESS, 0); // Red
      }
      else
      {
        rgbLedWrite(RGB_BUILTIN, 0, 0, 0); // Off
      }
      prevStatusLedUpdate = currentMillis;
    }
  }
  else if (!websocketConnected)
  {
    // Solid yellow when WiFi connected but no WebSocket
    rgbLedWrite(RGB_BUILTIN, RGB_BRIGHTNESS / 2, RGB_BRIGHTNESS, 0); // Yellow
  }
  else
  {
    // Solid green when both connected
    rgbLedWrite(RGB_BUILTIN, RGB_BRIGHTNESS, RGB_BRIGHTNESS / 2, 0); // Green
  }
#endif
}

void connectWebsocket()
{
  // Setup WebSocket connection
  webSocket.begin(ws_host, ws_port, "/");
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(1000); // Reconnect every 5 seconds if disconnected
}

void setup()
{
  Serial.begin(115200);

  // Initialize boot button pin
  pinMode(bootButtonPin, INPUT_PULLUP); // Boot button with internal pull-up

  // Connect to WiFi
  WiFiManager wifiManager;

  // Convert WS_PORT to string for default value
  sprintf(ws_port_str, "%d", WS_PORT);

  // Add custom parameters for WebSocket configuration
  WiFiManagerParameter custom_ws_host("ws_host", "WebSocket Host", ws_host, 40);
  WiFiManagerParameter custom_ws_port("ws_port", "WebSocket Port", ws_port_str, 6);

  wifiManager.addParameter(&custom_ws_host);
  wifiManager.addParameter(&custom_ws_port);

  Serial.print("Connecting to WiFi...");
  wifiManager.setConnectTimeout(10);
  wifiManager.autoConnect("truck-esp-32");
  Serial.println("\nConnected to WiFi");

  // Get custom parameter values
  strcpy(ws_host, custom_ws_host.getValue());
  strcpy(ws_port_str, custom_ws_port.getValue());
  ws_port = atoi(ws_port_str);

  Serial.print("WebSocket Host: ");
  Serial.println(ws_host);
  Serial.print("WebSocket Port: ");
  Serial.println(ws_port);

  // Attach the servo to the servo pin
  steeringServo.attach(servoPin);
  MotorServo.attach(MotorPin);

  MotorServo.writeMicroseconds(1500); // Neutral signal

  connectWebsocket();
}

void loop()
{
  // Handle WebSocket events
  webSocket.loop();

  currentMillis = millis();

  // Handle boot button for WiFi reset
  handleBootButton();

  // Update status LED based on connection state
  updateStatusLed();

  if (currentMillis - prevPingMillis > 1000)
  {
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

int lastMotorSpeedMicroseconds = 90; // Start at neutral

void webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{

  switch (type)
  {
  case WStype_ERROR:
    Serial.println("ERROR!");
  case WStype_DISCONNECTED:
    Serial.println("WS Disconnected!");
    websocketConnected = false;
    // If websocket gets disconnected, stop motor
    MotorServo.write(90); // Neutral signal
    break;
  case WStype_CONNECTED:
    Serial.println("WS Connected");
    websocketConnected = true;
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
    if (commaIndex != -1)
    {
      String motorSpeedString = data.substring(0, commaIndex);
      String servoPosString = data.substring(commaIndex + 1);

      int motorSpeed = motorSpeedString.toInt();
      int servoPos = servoPosString.toInt();

      // Constrain values and control motor and servo
      motorSpeed = constrain(motorSpeed, 10, 170); // ESC range
      servoPos = constrain(servoPos, 0, 180);

      // Check if we're switching from forward to reverse
      if (motorSpeed < 90 && lastMotorSpeedMicroseconds >= 90)
      {
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
