#include <WiFiNINA.h>
#include <ArduinoJson.h>
#include "secrets.h"

// // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // //
// Add your timer names here, eg: KraftBank, MindFit, Internal
const String timers[] = {"KraftBank", "MindFit", "Internal"};

// Variable to store number of timers, no need to change
const int numTimers = sizeof(timers) / sizeof(timers[0]);

// Add your LED-mapped pins here, eg: KraftBank <=> 1, MindFit <=> 2, Internal <=> 3
int ledPins[numTimers] = {1, 2, 3};

// Add your button-mapped pins here, eg: KraftBank <=> 6, MindFit <=> 7, Internal <=> 8
const int buttonPins[numTimers] = {6, 7, 8};
// // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // //

bool buttonState[numTimers] = {false};      // Array to keep track of button state
bool projectIsRunning[numTimers] = {false}; // Array to keep track of project state
int status = WL_IDLE_STATUS;

/// @brief Arduino setup function
void setup()
{
  for (int i = 0; i < numTimers; i++)
  {
    pinMode(buttonPins[i], INPUT_PULLUP);
    pinMode(ledPins[i], OUTPUT);
  }

  Serial.begin(9600);
}

int WIFI_CONNECTION_TIMEOUT = 60000; // 60 seconds

void blinkLEDs(unsigned long delay_time)
{
  static unsigned long previous_millis = 0;
  static bool led_state = false;

  if (millis() - previous_millis >= delay_time)
  {
    previous_millis = millis();
    led_state = !led_state;
    for (int i = 0; i < numTimers; i++)
    {
      digitalWrite(ledPins[i], led_state);
    }
  }
}
void connectToWiFi()
{
  int status = WiFi.status();
  unsigned long start_time = millis();

  while (status != WL_CONNECTED && millis() - start_time < WIFI_CONNECTION_TIMEOUT)
  {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(WIFI_SSID);
    status = WiFi.begin(WIFI_SSID, WIFI_PASS);
    blinkLEDs(500); // blink LEDs while connecting
  }

  if (status == WL_CONNECTED)
  {
    // Todo: Only print when it changes
    // Serial.print("Wifi connected!");
    blinkLEDs(1000); // blink LEDs twice to indicate connection established
    blinkLEDs(1000);
  }
  else
  {
    Serial.print("Unable to connect to WiFi");
    blinkLEDs(2000); // blink LEDs three times to indicate connection failure
    blinkLEDs(2000);
    blinkLEDs(2000);
  }
}

/// @brief buttonPressed() is called when a button is pressed
/// @param button The button that was pressed
void buttonPressed(int button)
{
  Serial.println("Button " + String(button) + " pressed");

  // Construct URL for API call
  String endpoint = projectIsRunning[button] ? "/stop" : "/start";
  String url = String("http://") + String(API_HOST) + String(":") + String(API_PORT) + endpoint + "?project=" + String(timers[button]);

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

// Button debouncing variables
const int DEBOUNCE_DELAY = 50; // debounce delay in ms
unsigned long lastDebounceTime[numTimers] = {0};

/// @brief Reads the buttons and calls buttonPressed() if a button is pressed
void readButtons()
{
  for (int i = 0; i < numTimers; i++)
  {
    int reading = digitalRead(buttonPins[i]);
    if (reading != buttonState[i])
    {
      lastDebounceTime[i] = millis();
    }
    if ((millis() - lastDebounceTime[i]) > DEBOUNCE_DELAY)
    {
      lastDebounceTime[i] = millis();
      if (reading != buttonState[i])
      {
        buttonState[i] = reading;
        if (buttonState[i] == LOW)
        {
          buttonPressed(i);
        }
      }
    }
  }
}

/// @brief  Updates the LEDs to match the project status
void updateLEDs()
{
  for (int i = 0; i < numTimers; i++)
  {
    digitalWrite(ledPins[i], projectIsRunning[i] ? HIGH : LOW);
  }
}

long lastUpdateTime = 0;
/// @brief Fetches the currently running timers from the API and updates the projectIsRunning array
void fetchAndUpdateTimers()
{
  // Only update every 10 seconds
  if (millis() - lastUpdateTime < 10000)
  {
    return;
  }
  lastUpdateTime = millis();

  String url = "http://" + String(API_HOST) + ":" + String(API_PORT) + "/running";
  WiFiClient client;
  if (client.connect(API_HOST, API_PORT))
  {
    client.print("GET " + url + " HTTP/1.1\r\n" +
                 "Host: " + String(API_HOST) + "\r\n" +
                 "Connection: close\r\n\r\n");
  }
  else
  {
    Serial.println("Connection failed");
    return;
  }

  String response = "";
  while (client.connected())
  {
    response += client.readString();
  }
  client.stop();

  for (int i = 0; i < numTimers; i++)
  {
    bool isRunning = false;
    for (int j = 0; j < response.length(); j++)
    {
      String projectName = "\"" + timers[i] + "\"";
      if (response.indexOf(projectName) != -1)
      {
        isRunning = true;
        break;
      }
    }
    Serial.println(timers[i] + " is running: " + isRunning);
    projectIsRunning[i] = isRunning;
  }
}

/// @brief Arduino loop function - The main loop of the program
void loop()
{
  connectToWiFi();
  fetchAndUpdateTimers();
  readButtons();
  updateLEDs();
}

// Q: The buttons are not working, what can I do?
// A: Try to increase the debounce delay (DEBOUNCE_DELAY) in the code. If that doesn't work, try to add a capacitor between the button and the ground pin.