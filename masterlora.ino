#include <Servo.h>
#include "esp_camera.h"
#include <WiFi.h>



#define CAMERA_MODEL_AI_THINKER

const int soundSensorPin = A0;  // Sound sensor connected to analog pin A0
const int redLEDPin = 8;       // Red LED connected to digital pin 8
const int greenLEDPin = 9;     // Green LED connected to digital pin 9
const int servoPin = 12;       // Servo motor connected to digital pin 12
const int threshold = 500;     // Adjust this threshold value as needed

Servo servoMotor;
bool motorRotating = true;    // Flag to indicate if the motor is rotating



const char* ssid = "Flat 103";
const char* password = "0135792468";

void startCameraServer();

void setup() {
  pinMode(redLEDPin, OUTPUT);
  pinMode(greenLEDPin, OUTPUT);
  pinMode(servoPin, OUTPUT);
  servoMotor.attach(servoPin);
  Serial.begin(9600);
  rotateServoContinuously();  // Start the servo rotation on setup
  Serial.begin(9600);
  Serial.setDebugOutput(true);
  Serial.println();

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1); // flip it back
    s->set_brightness(s, 1); // up the brightness just a bit
    s->set_saturation(s, -2); // lower the saturation
  }
  // drop down frame size for higher initial frame rate
  s->set_framesize(s, FRAMESIZE_QVGA);

#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");

}

void loop() {
  // Read the analog value from the sound sensor
  int sensorValue = analogRead(soundSensorPin);

  // Check if a clap sound is detected
  if (sensorValue > threshold) {
    if (motorRotating) {
      // If the motor is rotating, stop it
      stopServoRotation();
    } else {
      // If the motor is not rotating, start continuous rotation
      rotateServoContinuously();
    }

    // Toggle the motorRotating flag
    motorRotating = !motorRotating;

    // Turn on the green LED
    digitalWrite(greenLEDPin, LOW);
    digitalWrite(redLEDPin, HIGH);

    Serial.println("Gunshot detected!");

    // A small delay to avoid detecting the same clap multiple times in quick succession
    delay(1000);
  } else {
    // Turn off the green LED
    digitalWrite(greenLEDPin, HIGH);
    digitalWrite(redLEDPin, LOW);
  }
}

// Function to rotate the servo motor continuously
void rotateServoContinuously() {
  for (int angle = 0; angle <= 180; angle += 10) {
    servoMotor.write(angle);
    delay(100); // Adjust the delay to control the servo speed
  }
}

// Function to stop the servo motor
void stopServoRotation() {
  for (int angle = 180; angle >= 0; angle -= 10) {
    servoMotor.write(angle);
    delay(100); // Adjust the delay to control the servo speed
  }
}
