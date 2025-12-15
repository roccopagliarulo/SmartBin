***

# 🗑️ SmartBin

> **A centralized smart waste disposal system for homes and condominiums.**

SmartBin is born from a simple yet revolutionary idea: eliminating the need to have trash cans scattered in every room of the house. Thanks to artificial intelligence and a centralized transport system, SmartBin automates waste sorting, making disposal efficient, hygienic, and interactive.

---

## 📹 Demo

Simulates Cancel to reset the mouth session

![DEMO_CANCEL](/img/output_cancel.gif)

Simulates disposal by confirming the classification

![DEMO_CONFIRM](/img/output_confirm.gif)

---

## 💡 How It Works

The system is designed to cover an entire home or building. Each room is equipped with a smart "Mouth" (collection point).

### 1. Detection and Identification
- When an object is brought near the **Mouth**, a proximity sensor activates the system.
- An **ESP32-CAM** takes a picture and runs inference via a local AI model (Edge AI) to classify the material (e.g., plastic, paper, organic).

### 2. User Interaction (Human-in-the-Loop)
- An **OLED display** with a friendly interface (expressive "eyes") greets the user.
- The display shows the classification detected by the AI.
- Through a control panel, the user can **confirm**, **correct**, or **cancel** the classification if the AI is wrong, ensuring always correct waste sorting.

### 3. Transport and Synchronization
- Once confirmed, the mouth opens (servo motor) and the waste falls into the chute.
- The system manages a **synchronization queue** (via Node-RED) between the various "Mouths" in the house to avoid collisions on the single conveyor belt.
- The conveyor belt receives the waste and, already knowing the material type, sorts it into the correct final container.
---

## 📊 Dashboard & Monitoring

The system includes a control dashboard (built with **Node-RED Dashboard**) that allows for monitoring waste disposal progress in real-time.

Main Features:
*   **Waste Tracking**: Visualization of the amount of trash produced daily and weekly.
*   **AI Performance Monitor**: A dynamic chart compares system accuracy, showing:
    *   ✅ Classifications correctly made autonomously by the AI.
    *   ⚠️ Classifications manually corrected by humans (Feedback Loop).
    
![SmartBin Dashboard](/img/dash1.png)
![SmartBin Dashboard](/img/dash2.png)
![SmartBin Dashboard](/img/dash3.png)
---

## 🛠️ Hardware Setup

The project uses a combination of Espressif microcontrollers, sensors, actuators, and 3D printed supports.

### Main Electronics
*   **ESP32-S3**: Central management of sensors, actuators, and conveyor belt logic.
*   **ESP32-CAM**: Computer vision management, AI model inference, and classification JSON transmission.

### Sensors and Inputs
*   **HC-SR04 Sensor**: Measures distance to detect user presence at the "Mouth".
*   **IR Sensor**: Positioned on the belt to detect the passage of the object.
*   **Physical Buttons**: For user interaction (manual correction of classification).

### Actuators and Movement
*   **3x SG90 Servo Motors (9g)**:
    *   1x Opening/Closing of the "Mouth".
    *   2x Management of sorting/drop flaps.
*   **TT Motor (Yellow) + Motor Driver**: Movement of the conveyor belt.
*   **0.96" OLED Display**: Visual interface for user feedback.

---

## 🧩 Software and Architecture

The "brain" of the system relies on fluid communication between devices.

*   **Protocol**: MQTT. All communications between the Mouths, the Belt, and the Controller occur via lightweight and fast MQTT messages.
*   **Orchestration**: **Node-RED**. Manages complex logic, request synchronization (to prevent two rooms from unloading waste simultaneously), and data flow. 
*   **AI (Edge Computing)**: The classification model runs locally on the ESP32-CAM, ensuring privacy and response speed without depending on the cloud.

The complete Node-RED flow is available in the file backend/flows.json

---

## 🖨️ 3D Components

The physical structure of the prototype was created entirely via 3D printing.
The design is a hybrid composed of:
*   Models designed *ad hoc* for specific functional parts (sensor mounts, totem).
*   Open-source models modified/adapted for standard components.

---

## 👤 Author

Project created by **@roccopagliarulo**.

---

*If you like this project, leave a ⭐ on the repository!*