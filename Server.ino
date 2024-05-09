#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

// Define your network credentials
const char* ssid = "MySSID";
const char* password = "SmePasswrd";

const int ledPin = 0;  // GPIO pin where the LED is connected
bool ledState = HIGH;
ESP8266WebServer server(6061);

// Global variable to store the IP address to forward requests to
IPAddress forwardIP(0, 0, 0, 0;  // Default value

void setup() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, ledState);

  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Initialize EEPROM
  EEPROM.begin(512);

  // Read the stored IP address, if available
  readStoredIPAddress();

  // Define server routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/on", HTTP_GET, handleOn);
  server.on("/off", HTTP_GET, handleOff);
  server.on("/forward", HTTP_POST, handleForward);

  server.begin();
}

void loop() {
  server.handleClient();
  // Your loop code here
}

void handleRoot() {
  String html = "<html><body>";
  html += "<h1>On Air light Main Studio</h1>";
  html += "<p>LED is currently " + String(ledState == LOW ? "On" : "Off") + "</p>";
  html += "<a href='/on'>Turn On</a> | <a href='/off'>Turn Off</a>";
  html += "<form method='POST' action='/forward'>";
  html += "Forward to IP: <input type='text' name='ip' value='" + forwardIP.toString() + "'>";
  html += "<input type='submit' value='Forward'></form>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

void handleOn() {
  ledState = LOW;
  digitalWrite(ledPin, ledState);
  server.send(200, "text/plain", "LED On");
}

void handleOff() {
  ledState = HIGH;
  digitalWrite(ledPin, ledState);
  server.send(200, "text/plain", "LED Off");
}

// Handle the forward request
void handleForward() {
  if (server.hasArg("ip")) {
    String forwardIPStr = server.arg("ip");
    forwardIP.fromString(forwardIPStr);

    // Save the IP address to EEPROM
    writeIPAddressToStorage();
  }

  // Redirect back to the root page after submitting the form
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "");
}

void readStoredIPAddress() {
  EEPROM.get(0, forwardIP);
}

void writeIPAddressToStorage() {
  EEPROM.put(0, forwardIP);
  EEPROM.commit();  // Save the data to EEPROM
}