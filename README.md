# ESP32 Based Custom Flight Controller

[![C++](https://img.shields.io/badge/Code-C++-00599C?style=flat-square&logo=c%2B%2B)]() [![Platform](https://img.shields.io/badge/Hardware-ESP32-E7352C?style=flat-square&logo=espressif)]() [![Comms](https://img.shields.io/badge/Network-WebSockets-black?style=flat-square)]()

> This project uses flight controller built entirely from scratch running on budget friendly controller ESP32. Its main feature is also allowing the drone to be flown directly from a web browser via WebSockets.

<div align="center">
  <img width="400" alt="Custom Flight Controller" src="https://github.com/user-attachments/assets/018229d5-8bcd-42ec-a372-ac42ff056f6f" />
  <img width="400" alt="Custom Flight Controller 2" src="https://github.com/user-attachments/assets/485d2c90-e0a3-4e9d-b802-7d17d7096243" />
</div>
---

### System Architecture

As a systems integrator, my goal with this project was not to reinvent the wheel of flight stabilization, but to build a modern, network-capable infrastructure around it. 

* **The Core Stability Loop:** The foundational math for the Kalman filter and PID stabilization is adapted from standard open-source multirotor implementations. I modified and tuned these algorithms to run efficiently on the ESP32 architecture.
* **Custom Web Control (My Contribution):** I coded the communication layer. The ESP32 hosts an embedded HTML/JS web interface. I wrote the logic that takes incoming WebSocket strings from laptop keystrokes (e.g., "Pitch Forward, 1500"), bypasses traditional RC radio inputs, mixes the signals, and pushes the correct PWM values to the brushless motors.

### 🛠️ Hardware Integration

The physical flight controller is a custom-soldered perfboard utilizing:
* **Microcontroller:** ESP32 DevKit V1.
* **IMU:** MPU6050 (Communicating via I2C at a 400kHz clock speed).
* **Actuation:** 4x ESC signal outputs routed to GPIO pins 19, 23, 26, and 27, labeled M1 through M4. The `ESP32Servo` library pushes raw 1000-2000us PWM signals to control the brushless motors.

---

### Web-App Features

<div align="center">
<img width="581" height="450" alt="Custom Flight Controller App" src="https://github.com/user-attachments/assets/e782a9d1-de8b-4dcb-9606-bccb9aa8d8cf" />
</div>

#### 1. Real-Time Sensor Fusion (Kalman Filtering)
The system reads raw accelerometer and gyroscope data from the MPU6050. A 1D Kalman filter runs continuously in the main loop to fuse these inputs, removing vibration noise to calculate accurate `AngleRoll` and `AnglePitch` values.

<div align="center">
  <img width="300" alt="Custom Flight Controller Demo" src="https://github.com/user-attachments/assets/65a259c2-44c4-4e18-815d-9f9cae43c642" />
</div>

*Live Serial output demonstrating the Kalman filter actively tracking Roll and Pitch angles as the physical board is manipulated.*

#### 2. Live PID Tuning via Web Interface

<div align="center">
  <img width="650" alt="PID Tuning of Custom Flight Controller Web-App" src="https://github.com/user-attachments/assets/4ff090f7-ea9f-4d7d-8e82-6f492fcc5ba9" />
</div>


The biggest hurdle in PID Tuning of live systems is to tune and reprogram them, So Instead of flashing the board every time when PID value needs modification, instead the ESP32 hosts a another web route (`/pid`). This allows programmer to update the Proportional, Integral, and Derivative gains for Pitch, Roll, and Yaw wirelessly from a phone or laptop without re-flashing controller again and again.

#### 3. Non-Volatile Memory Storage (SPIFFS)
To make PID values permanently written even after power off, the updated PID variables are instantly written to text files within the ESP32's onboard SPIFFS memory.

#### 4. Motor Failsafe Logic
The code calculates the exact thrust required for each motor based on quadcopter kinematics (CW and CCW rotations). It includes hardcoded failsafes: if the throttle drops below 1050us, the motors immediately drop to a 1000us cutoff state and the PID memory resets to prevent fast spin-ups on the ground.
