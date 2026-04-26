# Zenoh-Pico ESP-IDF v5.x Native Wrapper 

A plug-and-play CMake wrapper component to cleanly build the bleeding-edge **Zenoh-Pico (1.0.0+ API)** natively on modern **ESP-IDF v5.x** environments.

## The Problem
Compiling the official `zenoh-pico` C-client directly inside ESP-IDF v5.x fails out-of-the-box due to legacy FreeRTOS header mismatches, SMP core panics, and aggressive CMake dependency chains.

## The Solution
This wrapper acts as a build shield. It isolates the upstream library, bypasses the broken FreeRTOS network abstractions, and dynamically forces the library to use ESP-IDF's modern POSIX threading (`pthread`) and VFS sockets via the `ZENOH_ESPIDF` macro. 

**No manual C-code hacking required. Just drop it in and compile.**

## Verified Stack
* **Hardware:** ESP32 (Xtensa)
* **ESP-IDF:** v5.2.6
* **Zenoh-Pico:** 1.9.0 (Main branch / 1.0.0+ API)
* **Compiler:** GNU GCC 13.2.0

---

## 🛠️ How to use this component

### 1. Clone into your project
Navigate to your ESP-IDF project's `components/` directory and clone this repository. You **must** use `--recursive` to pull the locked submodule of the upstream Zenoh-Pico library.

```bash
cd your_project/components/
git clone --recursive git@github.com:ShahazadAbdulla/zenoh-pico-espidf-v5-wrapper.git zenoh_wrapper
```

### 2. Update your Project CMake

In your project's main/CMakeLists.txt, add zenoh_wrapper to your REQUIRES list:
```CMake
idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES nvs_flash esp_wifi esp_event zenoh_wrapper)
```

### 3. Running the Example (Ping-Pong Reflector)

This repository includes a full working example in the example/ directory. It configures the ESP32 to connect to a Zenoh Router, listen for incoming pings, and bounce the exact payload back.
#### Step 1: Configure the ESP32 Target IP

Open example/main.c and locate the Zenoh configuration block:
```C
zp_config_insert(z_config_loan_mut(&config), Z_CONFIG_CONNECT_KEY, "udp/192.168.0.0:7447");
```
Change 192.168.0.0 to the actual IP address of the machine running your Zenoh router/peer.
#### Step 2: Build and Flash the ESP32

Navigate to the example/ directory, copy the main code and CMakeLists.txt to your project and build the project, and flash it to your ESP32:
``` bash
idf.py set-target esp32
idf.py menuconfig # (Configure your Wi-Fi credentials here under Example Connection Configuration)
idf.py build flash monitor
```
#### Step 3: Run the Python Peer

The example includes a Python script (ping_pong.py) that acts as a Zenoh Peer. It listens on port 7447, accepts the ESP32's connection, and blasts numbered payloads at it to verify bidirectional throughput.

On your PC, install the Zenoh Python library:
``` bash 
pip install eclipse-zenoh
```
Run the stress test:
``` bash
python3 example/ping_pong.py
```
Turn on your ESP32. You should see the Python script sending [SEND] logs and immediately receiving [RECV] reflections back from the microcontroller.
(Time it properly, start the python script and then restart the esp32).
