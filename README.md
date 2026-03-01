# SafeWare

Wearable Fall Detection Device  
ESP32 + MPU6050 + GPS

Developed at Eindhoven University of Technology  
5USUA0 – Concept vs Reality (2022–2023)

---

## Device Overview

### Wearable Design (Figure 1)

![Wearable Device](images/figure1_device.png)

SafeWare is a waist-mounted wearable device designed to detect falls and alert caregivers.  
The device includes:

- IMU-based fall detection
- GPS location tracking
- "I need help" button (Red)
- "I don't need help" button (Blue)
- LED indicators
- Buzzer alert

---

## Hardware Architecture (Figure 6)

![Hardware Architecture](images/figure6_schematic.png)

Main components:

- ESP32 microcontroller (CP2102)
- MPU-6050 IMU sensor
- GY-NEO6MV2 GPS module
- Active buzzer
- 3 LEDs (Red, Blue, Green)
- 2 tactile push buttons
- 18650 Li-ion battery

The system processes acceleration and angular velocity data using a three-trigger fall detection algorithm.  
If a fall is confirmed, the device activates an alarm and retrieves GPS coordinates.

---

## Repository Contents
SafeWare/
└── SafeWare.ino


This firmware handles:

- IMU data acquisition
- Trigger-based fall detection
- Button state management
- LED control
- Buzzer activation
- GPS parsing

---

## Status

Prototype completed as part of academic IoT project.  
Not certified as a medical device.
