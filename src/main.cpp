#include <WiFiNINA.h>
#include <ArduinoJson.h>
#include "secrets.h"

// // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // //
// Adjust these values to match your project
const int numButtons = 3;                                                 // Number of buttons
const int buttonPins[numButtons] = {6, 7, 8};                             // Array of button pins
int ledPins[numButtons] = {1, 2, 3};                                      // Array of LED pins
const String projects[numButtons] = {"KraftBank", "MindFit", "Internal"}; // Array of project names
// // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // //

bool buttonState[numButtons] = {false};      // Array to keep track of button state
bool projectIsRunning[numButtons] = {false}; // Array to keep track of project state
int status = WL_IDLE_STATUS;

void setup()
{
  for (int i = 0; i < numButtons; i++)
  {
    pinMode(buttonPins[i], INPUT_PULLUP);
    pinMode(ledPins[i], OUTPUT);
  }

  Serial.begin(9600);

  // attempt to connect to WiFi network
  while (status != WL_CONNECTED)
  {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(WIFI_SSID);
    status = WiFi.begin(WIFI_SSID, WIFI_PASS);
    delay(10000);
  }

  if (status == WL_CONNECTED)
  {
    Serial.print("Wifi connected!");
  }
}

void buttonPressed(int button)
{
  Serial.println("Button " + String(button) + " pressed");

  // Construct URL for API call
  String endpoint = projectIsRunning[button] ? "/stop" : "/start";
  String url = String("http://") + String(API_HOST) + String(":") + String(API_PORT) + endpoint + "?project=" + String(projects[button]);

  Serial.println("API URL: " + url);

  // Send HTTP request to API
  WiFiClient client;
  if (client.connect(API_HOST, API_PORT))
  {
    Serial.println("Connected to API server");
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + String(API_HOST) + "\r\n" +
                 "Connection: close\r\n" +
                 "\r\n");
  }
  else
  {
    Serial.println("Connection to API server failed");
  }

  // Read response from API
  while (client.connected())
  {
    String line = client.readStringUntil('\n');
    Serial.println(line);
  }

  client.stop();

  // Toggle projectIsRunning flag
  projectIsRunning[button] = !projectIsRunning[button];
}

void readButtons()
{
  for (int i = 0; i < numButtons; i++)
  {
    bool currentState = digitalRead(buttonPins[i]) == LOW;
    if (currentState && !buttonState[i])
    {
      buttonState[i] = true;
      buttonPressed(i);
    }
    else if (!currentState && buttonState[i])
    {
      buttonState[i] = false;
    }
  }
}

void updateLEDs()
{
  for (int i = 0; i < numButtons; i++)
  {
    digitalWrite(ledPins[i], projectIsRunning[i] ? HIGH : LOW);
  }
}

long lastUpdateTime = 0;
void updateProjectStatus()
{
  if (millis() - lastUpdateTime < 10000)
  {
    return;
  }
  lastUpdateTime = millis();
  Serial.println("Updating project status");

  // Todo: Do not block the main loop
  // Make GET request to get running projects
  String url = String("http://") + String(API_HOST) + String(":") + String(API_PORT) + "/running";
  WiFiClient client;
  if (client.connect(API_HOST, API_PORT))
  {
    client.println("GET " + url + " HTTP/1.1");
    client.println("Host: " + String(API_HOST));
    client.println("Connection: close");
    client.println();
  }
  else
  {
    Serial.println("Connection failed");
    return;
  }

  // Read response from server
  Serial.println("Reading response");
  String response = "";
  while (client.connected())
  {
    String line = client.readStringUntil('\n');
    response += line;
  }
  client.stop();

  // Parse response and update project status
  for (int i = 0; i < numButtons; i++)
  {
    bool isRunning = false;
    for (int j = 0; j < response.length(); j++)
    {
      String projectName = "\"" + projects[i] + "\"";
      if (response.indexOf(projectName) != -1)
      {
        isRunning = true;
        break;
      }
    }
    Serial.println(projects[i] + " is running: " + isRunning);
    projectIsRunning[i] = isRunning;
  }
  Serial.println("Done updating project status");
}

void loop()
{
  // wait for WiFi connection
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi not connected");
    return;
  }

  readButtons();
  updateLEDs();
  updateProjectStatus();
  delay(12);
}
