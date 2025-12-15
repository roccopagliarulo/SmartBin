## 💻 Firmware & Embedded

The project firmware is divided into two distinct units that communicate via MQTT. The code was developed using **PlatformIO** (on VS Code) to ensure professional dependency management and efficient compilation.

The source code is organized according to Object-Oriented Programming (OOP) principles to ensure modularity and maintainability.

### 1. Machine Controller (ESP32-S3)
This is the central unit that manages the physical hardware. The code is located in the `esp32s3-machine-controller` folder and is structured into independent modules (Managers):

*   **🎛️ Actuators & Sensors Management**:
    *   `BeltManager`: Manages the motor driver for the conveyor belt (start, stop, speed).
    *   `ServoManager`: Controls the servo motors for the mouth opening and sorting flaps.
    *   `ProximitySensor`: Manages the HC-SR04 and IR sensors to detect the presence of users or objects.
*   **🧠 Control Logic**:
    *   `SorterManager`: Contains the decision-making logic to coordinate mouth and belt based on the waste type.
    *   `InputManager`: Manages physical buttons for human interaction (manual corrections).
*   **📡 Connectivity & UI**:
    *   `NetworkManager`: Manages the WiFi connection and MQTT client in a non-blocking way.
    *   `DisplayManager`: Controls the OLED screen, managing the "eyes" expressions and visual feedback.
*   **⚙️ Configuration**:
    *   `config.h`: Centralized file for pin definitions, network credentials, and system constants.

### 2. AI Vision Eye (ESP32-CAM)
The computer vision firmware resides in the `esp32cam-ai-eye` folder.
*   **Edge AI**: The classification model (quantized for microcontrollers) is statically loaded in the `/lib` folder, allowing offline inference.
*   **Logic**: The `main` handles taking the photo upon sensor trigger, performing image pre-processing, running the inference, and publishing the JSON result to the MQTT topic.

---

### 📦 Dependencies and Libraries
The main libraries used in `platformio.ini` include:
*   `PubSubClient` (for MQTT)
*   `Adafruit SSD1306` / `GFX` (for the OLED display)
*   `ESP32Servo` (for motor control)
*   `ArduinoJson` (for message serialization)
*   `TensorFlow Lite for Microcontrollers` (for AI inference on the CAM)

## ⚙️ Installation and Configuration

To replicate the project, follow these sequential steps to configure the Backend and Firmware.

### 1. Backend Setup (Node-RED)
The server must be active before the microcontrollers.

1.  Ensure you have **Node-RED** installed (local or on Raspberry Pi).
2.  Open the "Palette Manager" and install the following necessary nodes:
    *   `@flowfuse/node-red-dashboard`
    *   `node-red-contrib-aedes` (The integrated MQTT Broker)
    *   `node-red-node-sqlite`
    *   `node-red-contrib-telegrambot-home`
3.  Import the flow file:
    *   Go to *Menu -> Import*.
    *   Select the `backend/flow.json` file present in this repository.
4.  Click on **Deploy**.
    *   *Note:* On first startup, the SQLite node will automatically create the necessary database and tables.

### 2. Firmware Configuration
Before uploading the code to the ESP32 boards, you must set your network credentials.

1.  Open the project with **VS Code** (with PlatformIO extension).
2.  Go to the `esp32s3-machine-controller/include` folder and open the **`config.h`** file.
3.  Modify the following lines with your parameters:
    ```cpp
    const char* WIFI_SSID = "Your_WiFi_Name";
    const char* WIFI_PASSWORD = "Your_WiFi_Password";
    const char* MQTT_SERVER = "192.168.1.XXX"; // The IP address where Node-RED is running
    ```
4.  Perform the same operation for the `esp32cam-ai-eye` firmware.
5.  Connect the boards via USB and click **Upload** in PlatformIO.

### 3. System Test
Once everything is active:

1.  Open the Node-RED dashboard (usually `http://<IP_NODERED>:1880/dashboard`).
2.  Bring an object close to the "Mouth" (proximity sensor).
3.  Verify that:
    *   The "eyes" on the OLED display react.
    *   The detection notification appears on the Dashboard.
    *   The conveyor belt activates at the right moment to sort the object.