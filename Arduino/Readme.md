# Arduino ACC CAN Bus Simulator

This project simulates Adaptive Cruise Control ACC behavior using an Arduino and an MCP2515 CAN bus module. The logic handles vehicle conditions such as brake status, ACC engagement, target speed, and fault detection, and transmits corresponding CAN messages.

## 📦 Project Structure

The core logic is in a single `.ino` Arduino sketch. It uses:

- `mcp_can.h` — for CAN communication via MCP2515
- `SPI.h` — for SPI communication

## ⚙️ Hardware Requirements

- Arduino
- MCP2515 CAN Module
- Vector CANoe for receiving/sending CAN frames

## 📡 CAN Configuration

- Baud rate: 500 kbps
- Crystal: 16 MHz

## 🧠 Features

- Sends ACC state, brake deceleration request, and target speed over CAN
- Receives and processes CAN signals for brake status, car presence, ACC state, and speed value
- Switches to ACC standby if fault conditions occur (e.g., speed < 25 or manual brake applied)
- Handles target speed confirmation before activating ACC

## 🔌 Pin Configuration

| Pin       | Function           |
|-----------|--------------------|
| D9        | SPI CS for MCP2515 |
| D7        | Brake input pin    |

## 📝 CAN Message Overview

- **0x100**: ACC_info
- **0x200**: ICM_Commands
- **0x300**: BCM_info
- **0x500**: ACC_Sys_Variables

## 🚦 Logic Highlights

- If brake is applied and no car is present → ACC enters standby
- If target speed is confirmed and all conditions are met → ACC becomes active
- If a fault is detected → ACC forced to standby

## 🛠 Setup

```cpp
#include <mcp_can.h>
#include <SPI.h>
