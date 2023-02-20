# Timer-Tron

The physical timer for your multiple projects

![timer-tron](https://user-images.githubusercontent.com/3164065/220170307-8a8f8147-c7bc-4f06-bf5a-7f0285e9b994.png)

## Introduction

If you're like many people, you might find yourself struggling to stay focused and productive when working on multiple projects. It can be hard to keep track of how much time you're spending on each project, and switching between them can be disruptive to your flow. That's where this project comes in - it's a physical project-timer toggle that lets you start and stop timers for up to three different projects.

The heart of the project is the Arduino WiFi MKR 1010 board, which provides both the microcontroller and WiFi connectivity. However, you could use other boards with WiFi capabilities as well. The board is connected to three buttons, which are used to start and stop the timers for each project. Each button is also connected to an LED, which indicates whether the timer for that project is currently running or not.

To set up the project, you'll need to connect the buttons and LEDs to the appropriate pins on the board. The code for the project is written in Arduino, and is designed to work with the Node.js backend on a remote server to handle the project timing logic. The code can be extended to include more or fewer projects, and you can customize the project names and LED pins as needed.

Once the project is set up and running, you press the button for the project you want to work on, and the timer will start.

You can currently have multiple timers running at the same time. This could be changed so that only one timer can be running at a time, but I haven't done that yet.

Overall, this project is a great way to help you stay on track with your work and make the most of your time. Whether you're working on personal projects or managing multiple clients, having a physical timer toggle can be a helpful tool for improving your productivity and keeping your projects organized. So why not give it a try? With a little bit of time and effort, you can create a project timer toggle that works perfectly for you.

## Setup

### Secrets

Create a file named `secrets.h` in the `src` directory. This file should contain the following:

```c++
  #define WIFI_SSID ""              // Your WiFi SSID (aka. Network Name)
  #define WIFI_PASS ""              // Your WiFi password
  #define API_HOST "YOUR_API_HOST"  // "000.000.000.000"
  #define API_PORT 3000             // Your API port as an integer
```

### Project, Button, and LED Pins

The project is currently set up to work with three projects, but you may change this to work with more or fewer projects. See the comments in the code for more information.
