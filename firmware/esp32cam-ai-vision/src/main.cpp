/* Includes ---------------------------------------------------------------- */
// <<< Include your Edge Impulse model
#include <roccopagliarulo99-project-1_inferencing.h>
#include "edge-impulse-sdk/dsp/image/image.hpp"

// <<< Include camera logic
#include "esp_camera.h"

// <<< Include WiFi and MQTT logic
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

/* -------------------------------------------------------------------------- */
/* WiFi and MQTT Configuration (TO BE CHANGED)               */
/* -------------------------------------------------------------------------- */

const char* WIFI_SSID = "WIFI_SSID";
const char* WIFI_PASS = "WIFI_PSW";

const char* MQTT_BROKER_IP = "TUO_IP"; 
const int MQTT_PORT = 1883;
const char* MOUTH_ID = "1"; // MUST MATCH the S3 ID

// <<< Confidence threshold for classification
const float CONFIDENCE_THRESHOLD = 0.70; // 70%

/* -------------------------------------------------------------------------- */
/* Camera Configuration (DO NOT TOUCH)                */
/* -------------------------------------------------------------------------- */

// Select camera model
//#define CAMERA_MODEL_ESP_EYE // Has PSRAM
#define CAMERA_MODEL_AI_THINKER // Has PSRAM

#if defined(CAMERA_MODEL_ESP_EYE)
#define PWDN_GPIO_NUM    -1
#define RESET_GPIO_NUM   -1
#define XCLK_GPIO_NUM    4
#define SIOD_GPIO_NUM    18
#define SIOC_GPIO_NUM    23
#define Y9_GPIO_NUM      36
#define Y8_GPIO_NUM      37
#define Y7_GPIO_NUM      38
#define Y6_GPIO_NUM      39
#define Y5_GPIO_NUM      35
#define Y4_GPIO_NUM      14
#define Y3_GPIO_NUM      13
#define Y2_GPIO_NUM      34
#define VSYNC_GPIO_NUM   5
#define HREF_GPIO_NUM    27
#define PCLK_GPIO_NUM    25
#elif defined(CAMERA_MODEL_AI_THINKER)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
#else
#error "Camera model not selected"
#endif

/* Constant defines -------------------------------------------------------- */
#define EI_CAMERA_RAW_FRAME_BUFFER_COLS           320
#define EI_CAMERA_RAW_FRAME_BUFFER_ROWS           240
#define EI_CAMERA_FRAME_BYTE_SIZE                 3

/* Global Variables ------------------------------------------------------- */
// <<< MQTT variables
char MQTT_TOPIC_COMMAND[100];
char MQTT_TOPIC_CLASSIFICATION[100];
WiFiClient espClient;
PubSubClient client(espClient);

// <<< Edge Impulse debug flag
static bool debug_nn = false;
static bool is_initialised = false;
uint8_t *snapshot_buf; // Image buffer

static camera_config_t camera_config = {
    .pin_pwdn = PWDN_GPIO_NUM,
    .pin_reset = RESET_GPIO_NUM,
    .pin_xclk = XCLK_GPIO_NUM,
    .pin_sscb_sda = SIOD_GPIO_NUM,
    .pin_sscb_scl = SIOC_GPIO_NUM,
    .pin_d7 = Y9_GPIO_NUM,
    .pin_d6 = Y8_GPIO_NUM,
    .pin_d5 = Y7_GPIO_NUM,
    .pin_d4 = Y6_GPIO_NUM,
    .pin_d3 = Y5_GPIO_NUM,
    .pin_d2 = Y4_GPIO_NUM,
    .pin_d1 = Y3_GPIO_NUM,
    .pin_d0 = Y2_GPIO_NUM,
    .pin_vsync = VSYNC_GPIO_NUM,
    .pin_href = HREF_GPIO_NUM,
    .pin_pclk = PCLK_GPIO_NUM,
    .xclk_freq_hz = 20000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,
    .pixel_format = PIXFORMAT_JPEG,
    .frame_size = FRAMESIZE_QVGA,
    .jpeg_quality = 12,
    .fb_count = 1,
    .fb_location = CAMERA_FB_IN_PSRAM,
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
};

/* -------------------------------------------------------------------------- */
/* Support Functions (WiFi, Camera, EI)                */
/* -------------------------------------------------------------------------- */

// --- WiFi functions (from "dummy" code) ---
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// --- Camera functions (from EI example) ---
bool ei_camera_init(void) {
    if (is_initialised) return true;
#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
      Serial.printf("Camera initialization failed with error 0x%x\n", err);
      return false;
    }
    sensor_t * s = esp_camera_sensor_get();
    if (s->id.PID == OV3660_PID) {
      s->set_vflip(s, 1); s->set_brightness(s, 1); s->set_saturation(s, 0);
    }
#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_ESP_EYE)
    s->set_vflip(s, 1); s->set_hmirror(s, 1); s->set_awb_gain(s, 1);
#endif
    is_initialised = true;
    return true;
}

bool ei_camera_capture(uint32_t img_width, uint32_t img_height, uint8_t *out_buf) {
    bool do_resize = false;
    if (!is_initialised) {
        ei_printf("ERR: Camera not initialized\r\n");
        return false;
    }
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        ei_printf("Camera capture failed\n");
        return false;
    }
   bool converted = fmt2rgb888(fb->buf, fb->len, PIXFORMAT_JPEG, snapshot_buf);
   esp_camera_fb_return(fb);
   if(!converted){
       ei_printf("Conversion failed\n");
       return false;
   }
    if ((img_width != EI_CAMERA_RAW_FRAME_BUFFER_COLS) || (img_height != EI_CAMERA_RAW_FRAME_BUFFER_ROWS)) {
        do_resize = true;
    }
    if (do_resize) {
        ei::image::processing::crop_and_interpolate_rgb888(
        out_buf, EI_CAMERA_RAW_FRAME_BUFFER_COLS, EI_CAMERA_RAW_FRAME_BUFFER_ROWS,
        out_buf, img_width, img_height);
    }
    return true;
}

static int ei_camera_get_data(size_t offset, size_t length, float *out_ptr) {
    size_t pixel_ix = offset * 3;
    size_t pixels_left = length;
    size_t out_ptr_ix = 0;
    while (pixels_left != 0) {
        // Swap BGR to RGB
        out_ptr[out_ptr_ix] = (snapshot_buf[pixel_ix + 2] << 16) + (snapshot_buf[pixel_ix + 1] << 8) + snapshot_buf[pixel_ix];
        out_ptr_ix++;
        pixel_ix+=3;
        pixels_left--;
    }
    return 0;
}

/* -------------------------------------------------------------------------- */
/* MQTT LOGIC (Core of the Program)                   */
/* -------------------------------------------------------------------------- */

// <<< THIS FUNCTION NOW PERFORMS INFERENCE! >>>
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message received on topic: ");
  Serial.println(topic);

  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';
  Serial.println(message);

  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, message);
  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }

  const char* action = doc["action"];
  
  if (strcmp(action, "take_photo") == 0) {
    Serial.println("Received 'take_photo' command. Starting inference...");

    const char* waste_id = doc["waste_id"];
    if (waste_id == NULL) {
        Serial.println("Error: waste_id missing!");
        return;
    }
    Serial.print("Waste ID: ");
    Serial.println(waste_id);

    // --- START EDGE IMPULSE LOGIC ---
    snapshot_buf = (uint8_t*)malloc(EI_CAMERA_RAW_FRAME_BUFFER_COLS * EI_CAMERA_RAW_FRAME_BUFFER_ROWS * EI_CAMERA_FRAME_BYTE_SIZE);
    if(snapshot_buf == nullptr) {
        Serial.println("ERR: Failed to allocate snapshot buffer!");
        return;
    }

    ei::signal_t signal;
    signal.total_length = EI_CLASSIFIER_INPUT_WIDTH * EI_CLASSIFIER_INPUT_HEIGHT;
    signal.get_data = &ei_camera_get_data;

    if (ei_camera_capture((size_t)EI_CLASSIFIER_INPUT_WIDTH, (size_t)EI_CLASSIFIER_INPUT_HEIGHT, snapshot_buf) == false) {
        Serial.println("Image capture failed");
        free(snapshot_buf);
        return;
    }

    // Run the classifier
    ei_impulse_result_t result = { 0 };
    EI_IMPULSE_ERROR err = run_classifier(&signal, &result, debug_nn);
    
    // <<< Free memory IMMEDIATELY after inference
    free(snapshot_buf); 
    
    if (err != EI_IMPULSE_OK) {
        Serial.printf("ERR: run_classifier failed (%d)\n", err);
        return;
    }

    // Print timings
    Serial.printf("Inference completed (DSP: %d ms., Class.: %d ms., Anomaly: %d ms.): \n",
                result.timing.dsp, result.timing.classification, result.timing.anomaly);

    // <<< Find the best result
    String best_label = "unknown";
    float best_score = 0.0;
    for (uint16_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
        Serial.printf("  %s: %.5f\n", ei_classifier_inferencing_categories[i], result.classification[i].value);
        if (result.classification[i].value > best_score) {
            best_score = result.classification[i].value;
            best_label = ei_classifier_inferencing_categories[i];
        }
    }
    
    // <<< Apply confidence threshold
    if (best_score < CONFIDENCE_THRESHOLD) {
        Serial.printf("No result above threshold (%.2f). Setting 'unknown'.\n", CONFIDENCE_THRESHOLD);
        best_label = "unknown";
    }

    Serial.printf("Best result: %s (Score: %.5f)\n", best_label.c_str(), best_score);
    
    // --- END EDGE IMPULSE LOGIC ---

    // 6. Create response JSON
    StaticJsonDocument<128> responseDoc;
    responseDoc["class"] = best_label; // Invia il risultato migliore
    responseDoc["waste_id"] = waste_id;
    responseDoc["score"] = best_score; // Invia anche il punteggio (utile per debug)

    char responsePayload[128];
    serializeJson(responseDoc, responsePayload);

    // 7. Publish the result
    Serial.print("Publishing result to: ");
    Serial.println(MQTT_TOPIC_CLASSIFICATION);
    Serial.println(responsePayload);
    client.publish(MQTT_TOPIC_CLASSIFICATION, responsePayload);
  
  } else {
    Serial.print("Unrecognized command: ");
    Serial.println(action);
  }
}

// <<< MQTT reconnection function (from "dummy" code)
void reconnect_mqtt() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "esp32cam-mouth-" + String(MOUTH_ID);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Sottoscriviti al topic dei comandi!
      Serial.print("Subscribing to: ");
      Serial.println(MQTT_TOPIC_COMMAND);
      client.subscribe(MQTT_TOPIC_COMMAND);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds");
      delay(5000);
    }
  }
}

/* -------------------------------------------------------------------------- */
/* SETUP and LOOP                                */
/* -------------------------------------------------------------------------- */

void setup()
{
    Serial.begin(115200);
    // <<< DO NOT wait for serial, the board must start on its own
    // while (!Serial);
    Serial.println("ESP32-CAM (Edge Impulse Classifier) Started.");
    
    // <<< Initialize WiFi
    setup_wifi();

    // <<< Build MQTT topics
    snprintf(MQTT_TOPIC_COMMAND, sizeof(MQTT_TOPIC_COMMAND), "smartbin/mouth/%s/command", MOUTH_ID);
    snprintf(MQTT_TOPIC_CLASSIFICATION, sizeof(MQTT_TOPIC_CLASSIFICATION), "smartbin/mouth/%s/state/classification", MOUTH_ID);
    
    // <<< Configure MQTT
    client.setServer(MQTT_BROKER_IP, MQTT_PORT);
    client.setCallback(mqtt_callback); // Imposta la funzione che gestirà i messAGGI

    // <<< Initialize Camera
    if (ei_camera_init() == false) {
        Serial.println("CRITICAL ERROR : Failed to initialize Camera!");
    }
    else {
        Serial.println("Camera initialized");
    }

    ei_printf("Edge Impulse model loaded.\n");
}


void loop()
{
    // <<< The loop now ONLY handles MQTT connection
    if (!client.connected()) {
      reconnect_mqtt();
    }
    client.loop(); // Essential to receive messages
    
    // Do nothing here, wait for command in callback
    delay(10); 
}

#if !defined(EI_CLASSIFIER_SENSOR) || EI_CLASSIFIER_SENSOR != EI_CLASSIFIER_SENSOR_CAMERA
#error "Invalid model for current sensor"
#endif