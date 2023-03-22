#include <WiFiNINA.h>
#include <ArduinoJson.h>
#include "secrets.h"

const int NUM_BUTTONS = 3;
const int BUTTON_PINS[NUM_BUTTONS] = {6, 7, 8};
int LED_PINS[NUM_BUTTONS] = {1, 2, 3};
const String PROJECTS[NUM_BUTTONS] = {"KraftBank", "MilesDashboard", "Internal"};

bool buttonState[NUM_BUTTONS] = {false};
bool projectIsRunning[NUM_BUTTONS] = {false};
int status = WL_IDLE_STATUS;

void setup()
{
  initializePins();
  Serial.begin(9600);
  connectToWiFi();
}

void initializePins()
{
  for (int i = 0; i < NUM_BUTTONS; i++)
  {
    pinMode(BUTTON_PINS[i], INPUT_PULLUP);
    pinMode(LED_PINS[i], OUTPUT);
  }
}

void connectToWiFi()
{
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
  String url = constructApiUrl(button);
  sendApiRequest(url);
  toggleProjectState(button);
}

String constructApiUrl(int button)
{
  String endpoint = projectIsRunning[button] ? "/stop" : "/start";
  return String("http://") + String(API_HOST) + String(":") + String(API_PORT) + endpoint + "?project=" + String(PROJECTS[button]);
}

void sendApiRequest(String url)
{
  Serial.println("API URL: " + url);
  WiFiClient client;

  if (client.connect(API_HOST, API_PORT))
  {
    Serial.println("Connected to API server");
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + String(API_HOST) + "\r\n" +
                 "Connection: close\r\n" +
                 "\r\n");
    readApiResponse(client);
  }
  else
  {
    Serial.println("Connection to API server failed");
  }
}

void readApiResponse(WiFiClient &client)
{
  while (client.connected())
  {
    String line = client.readStringUntil('\n');
    Serial.println(line);
  }
  client.stop();
}

void toggleProjectState(int button)
{
  projectIsRunning[button] = !projectIsRunning[button];
}

void readButtons()
{
  for (int i = 0; i < NUM_BUTTONS; i++)
  {
    bool currentState = digitalRead(BUTTON_PINS[i]) == LOW;
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
  for (int i = 0; i < NUM_BUTTONS; i++)
  {
    digitalWrite(LED_PINS[i], projectIsRunning[i] ? HIGH : LOW);
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

  String url = String("http://") + String(API_HOST) + String(":") + String(API_PORT) + "/running";
  WiFiClient client;

  if (client.connect(API_HOST, API_PORT))
  {
    sendStatusRequest(client, url);
    String response = readStatusResponse(client);
    updateProjectStatusFromResponse(response);
  }
  else
  {
    Serial.println("Connection failed");
  }
}

void sendStatusRequest(WiFiClient &client, String url)
{
  client.println("GET " + url + " HTTP/1.1");
  client.println("Host: " + String(API_HOST));
  client.println("Connection: close");
  client.println();
}

String readStatusResponse(WiFiClient &client)
{
  Serial.println("Reading response");
  String response = "";
  while (client.connected())
  {
    String line = client.readStringUntil('\n');
    response += line;
  }
  client.stop();
  return response;
}

void updateProjectStatusFromResponse(String response)
{
  for (int i = 0; i < NUM_BUTTONS; i++)
  {
    bool isRunning = false;
    String projectName = "\"" + PROJECTS[i] + "\"";
    if (response.indexOf(projectName) != -1)
    {
      isRunning = true;
    }
    Serial.println(PROJECTS[i] + " is running: " + isRunning);
    projectIsRunning[i] = isRunning;
  }
  Serial.println("Done updating project status");
}

bool needToUpdate = true;
void loop()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    needToUpdate = true;
    Serial.println("WiFi not connected");
    return;
  }

  readButtons();
  updateLEDs();

  if (needToUpdate)
  {
    updateProjectStatus();
    needToUpdate = false;
  }
  delay(12);
}
