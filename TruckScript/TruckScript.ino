#include <WiFi.h>
#include <WebSocketsClient.h>
#include <ESP32Servo.h>
#include <config.h>

// WiFi credentials
const char *ssid = SSID;
const char *password = PASSWORD;

// WebSocket server address and port
const char *ws_host = WS_HOST; // e.g., "192.168.1.100" or "wss://example.com"
const uint16_t ws_port = WS_PORT;

// Define the PWM channel, frequency, resolution, and pin
const int MotorPin = MOTOR_PIN; // PWM pin for motor control
const int servoPin = SERVO_PIN; // Servo control pin
const int lightsPin = LIGHTS_PIN;

unsigned long currentMillis;
unsigned long prevMessageMillis = millis(); // Stores the last time a websocket message was recieved
unsigned long prevPingMillis = 0;           // Stores the last time a websocket message was recieved

bool lightState = LOW; // Initial state of the light

Servo steeringServo; // Create Servo object
Servo MotorServo;    // Create Servo object
WebSocketsClient webSocket;

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
  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  // Attach the servo to the servo pin
  steeringServo.attach(servoPin);
  MotorServo.attach(MotorPin);

  MotorServo.writeMicroseconds(1500); // Neutral signal

  pinMode(lightsPin, OUTPUT);

  connectWebsocket();
}

void loop()
{
  // Handle WebSocket events
  webSocket.loop();

  currentMillis = millis();
  if (currentMillis - prevPingMillis > 1000)
  {
    // Ping websocket server to check connection
    webSocket.sendTXT("ping");
    prevPingMillis = currentMillis;
  }
  if (currentMillis - prevMessageMillis > 3000)
  {
    // if more than 2 seconds have elapsed since last websocket message, stop motor
    MotorServo.write(90);
    if (webSocket.isConnected())
    {
      // When the network changes the websocket doesn't disconnect properly, disconnect here when no more heart beat
      webSocket.disconnect();
      prevMessageMillis = currentMillis;
    }
  }
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
    // If websocket gets disconnected, stop motor
    MotorServo.write(90); // Neutral signal
    break;
  case WStype_CONNECTED:
    Serial.println("WS Connected");
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
