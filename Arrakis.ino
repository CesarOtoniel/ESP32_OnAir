#include <WiFi.h>
#include <EEPROM.h>

// Define your network credentials
const char* ssid = "MySSID";
const char* password = "SmePasswrd";

// Set your server IP and port
const char* serverAddress = "xxx.xxx.xxx.xxx";
const int serverPort = 6060;

// EEPROM address to store the server URL
const int eepromAddress = 0;

WiFiServer server(80);

int lastPinD0State = HIGH;

void setup() {
  // Start the serial communication
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

  // Read the server URL from EEPROM
  String serverURL;
  EEPROM.get(eepromAddress, serverURL);
  if (serverURL.length() > 0) {
    Serial.print("Server URL loaded: ");
    Serial.println(serverURL);
  } else {
    serverURL = "on"; // Default server URL
    EEPROM.put(eepromAddress, serverURL);
    EEPROM.commit();
  }

  // Start the server
  server.begin();
}

void loop() {
  // Check if pin D0 state has changed
  int currentPinD0State = !digitalRead(D0);
  if (currentPinD0State != lastPinD0State) {
    // Build the URL based on the pin state
    String url;
    if (currentPinD0State == HIGH) {
      url = "/on";
    } else {
      url = "/off";
    }

    // Send an HTTP GET request to the server
    sendHttpRequest(url);

    lastPinD0State = currentPinD0State;
  }

  // Check if a client has connected
  WiFiClient client = server.available();
  if (client) {
    Serial.println("Client connected");

    // Read the HTTP request
    String request = client.readStringUntil('\r');
    client.flush();

    // Check if the request contains a new server URL
    if (request.indexOf("GET /seturl") != -1) {
      String newURL = request.substring(request.indexOf("url=") + 4, request.indexOf("HTTP"));
      EEPROM.put(eepromAddress, newURL);
      EEPROM.commit();
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/plain");
      client.println();
      client.println("Server URL updated");
      client.stop();
      Serial.print("Server URL updated: ");
      Serial.println(newURL);
    }
    else {
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/plain");
      client.println();
      client.print("Current Server URL: ");
      client.println(serverAddress);
      client.stop();
    }
  }
}

void sendHttpRequest(String url) {
  WiFiClient client;
  if (client.connect(serverAddress, serverPort)) {
    Serial.print("Sending GET request to: ");
    Serial.println(url);
    client.print("GET ");
    client.print(url);
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(serverAddress);
    client.println("Connection: close");
    client.println();
    delay(10);
    while (client.connected()) {
      if (client.available()) {
        String line = client.readStringUntil('\r');
        Serial.println(line);
      }
    }
    client.stop();
  }
}
