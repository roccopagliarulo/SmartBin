## 🗂️ Backend & Orchestration

The backend, hosted in the `/backend` folder, is the operational brain of SmartBin. It was developed on **Node-RED** and manages not only communication but the entire state logic of the distributed system.

### 🛠️ Backend Technologies
*   **Node-RED**: Flow engine for event-driven logic.
*   **SQLite**: Lightweight relational database (local) for data history and statistics.
*   **Aedes MQTT Broker**: MQTT Broker integrated directly into Node-RED to manage IoT messaging without external dependencies.
*   **Node-RED Dashboard**: For the administration graphical interface.
*   **Telegram Bot API**: For real-time alert notifications.

### ⚙️ Flow Architecture

The backend system is structured on advanced logic to ensure data fluidity and integrity:

#### 1. Concurrency Management (Smart Queue System)
Since the conveyor belt is a shared resource (Singleton), the backend implements a FIFO (First-In-First-Out) queuing algorithm.
*   If the belt is **FREE**: The waste is processed immediately.
*   If the belt is **BUSY**: The request is placed in an in-memory queue (`belt_queue`).
*   As soon as the belt completes a cycle (`cleared` event), the backend automatically processes the next item in the queue.

#### 2. Human-in-the-Loop & Data Integrity
The system distinguishes three classification scenarios and saves them in the `waste_history` DB:
*   **AI Confidence**: The AI classifies, the user confirms → saved as `ai_correct = 1`.
*   **Human Correction**: The AI is wrong, the user corrects via the button panel → saved as `ai_correct = 0` (useful for retraining the model).
*   **Timeout/Unknown**: If the user does not interact, the system applies fallback logic after 5 attempts.

#### 3. Monitoring and Notifications
*   **Fail-safe**: Implementation of **Watchdog Timers** (30s) on "Mouth" states to prevent the system from remaining stuck in a "BUSY" state in case of hardware errors.
*   **Alerting**: Integration with **Telegram** to send push notifications to the maintainer when a bin is "FULL" (detected by sensors on the final baskets).

#### 4. Dashboard Analytics
The dashboard visualizes real-time SQL queries to show:
*   Pie Chart: Daily waste types.
*   Bar Chart: Weekly history.
*   Performance Chart: Computer Vision accuracy rate compared to manual corrections.