#include <WiFi.h>
#include <esp_now.h>

#include "Wire.h"
#include <MPU6050_light.h>
#include "Adafruit_ZeroFFT.h"


//SENSOR VARIABLES
int touchValue = 0;
int touch1_cal_value,touch2_cal_value,touch3_cal_value,touch4_cal_value;  //touch calibration reference values
int touch_thres = 20000;

MPU6050 mpu(Wire);     //Acceleration sensor
unsigned long timer = 0;
unsigned long sending_rate = 10;
const int i2c_sda = 17;   //in S3 SDA and SCL can be configured at almost every pin
const int i2c_scl = 18;


//FFT ANALYSIS
#define DATA_SIZE 64
int16_t data[DATA_SIZE];
float FFT_threshold = 40.0;  ////////////// detection threshold  6 = 20
int trig = 1;
int FFT_index = 0;
bool triggered = false;
char off_counter = 0;


//ESP NOW
#define SENDER_ID 3    //////////////////// Sleeper identity 
uint8_t broadcastAddress[] = {0x48,0x27,0xE2,0x50,0xCB,0x4E}; //{0x48,0x27,0xE2,0x50,0x76,0x82};  // {0x48,0x27,0xE2,0x50,0xCB,0x4E}; //(this last one is the original)

// Structure to send data
// Must match the receiver structure
typedef struct struct_message {
  uint8_t id;
  uint8_t piezo;
  float accelX;
  float accelY;
  uint8_t touch;
} struct_message;

// Create a struct_message called myData
struct_message myData;

esp_now_peer_info_t peerInfo;


// callback when data is sent  (nothing is needed here)
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  //Serial.print("Delivery:\t");
  //Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Failed");
  //Serial.println();
}

void setup() {
  delay(3000);  //give some time to remove one's hand from the sleeper when switched on
  
  // Initilize hardware serial:
  Serial.begin(115200);

  //pinMode(2, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  Wire.setPins(i2c_sda, i2c_scl);
  Wire.begin();

  byte status = mpu.begin();
  Serial.print(F("MPU6050 status: "));
  Serial.println(status);
  //while (status != 0) { } // stop everything if could not connect to MPU6050

  Serial.println(F("Calculating offsets, do not move MPU6050"));
  delay(1000);
  // mpu.upsideDownMounting = true; // uncomment this line if the MPU6050 is mounted upside-down
  mpu.calcOffsets(); // gyro and accelero
  Serial.println("Done!\n");

  //Init my id for the struct
  myData.id = SENDER_ID;
  
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  } Serial.println("ESP-NOW initialized");

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  } else Serial.println("Successfully added peer");


  //get touch calibration values
  delay(3000);  //also wait a bit, maybe we still the hand inside
  Serial.println("init touch calibration");
  touch1_cal_value =  touchRead(11);
  touch2_cal_value =  touchRead(12);
  touch3_cal_value =  touchRead(13);
  touch4_cal_value =  touchRead(14);
  Serial.println(touch1_cal_value);
  Serial.println(touch2_cal_value);
  Serial.println(touch3_cal_value);
  Serial.println(touch4_cal_value);
  Serial.println("touch calibration ended");
}




void loop() {
  
    //long measure = millis();   //if we want to check how much the FFT takes

    //FFT ANALYSIS
    //Read piezo samples
    int32_t avg = 0;
    for (int i = 0; i < DATA_SIZE; i++) {
      int16_t val = analogRead(4);
      avg += val;
      data[i] = val;
    }

    //remove DC offset and gain up to 16 bits
    avg = avg / DATA_SIZE;
    for (int i = 0; i < DATA_SIZE; i++) data[i] = (data[i] - avg) * 64;

      //run the FFT
    ZeroFFT(data, DATA_SIZE);

    //Serial.print("Process:" );
    //Serial.println(millis()-measure);
      
     //just for printing the FFT result
     /*
      for(int i=0; i<DATA_SIZE/8; i++) {
      Serial.print(data[i]);
      Serial.print(" ");
      }
      Serial.println(" ");
     */

  //IF there is a knock, send it to the receiver
  if (data[FFT_index] > FFT_threshold) {   //if above the threshold
      
      off_counter = 0;  //wooden panels resonate long, going down the threshold and then up sometimes too. so we wait a few cycles.
      
      if(triggered==false) {  //if first timea above the threshold, send message inmediately
        digitalWrite(LED_BUILTIN, HIGH);  //led on to give feedback
        myData.piezo = data[FFT_index];
        Serial.println(data[FFT_index]);  //print to debug
        
        // Send whole message with the piezo velocity via ESP-NOW
        esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
        
        triggered = true;  //this cannot be triggered again until we have decided that the event has finished (myData.piezo = 0)
      }  
      
   } else {   //send a trigger = 0 , so a noteoff will be produced at the receiver
      off_counter++;
      if (off_counter>3) {
        digitalWrite(LED_BUILTIN, LOW);
        myData.piezo = 0;
        triggered = false;
      }
   }
     


    //Accelerometer and touch
    mpu.update();

    if ((millis() - timer) > sending_rate) { // send data every 10ms

      myData.accelX = mpu.getAngleX();
      
      myData.accelY = mpu.getAngleY();
      //Serial.println(myData.accelY);

      //if one of the touch pads is touched then send the info
      if(touchRead(14) > (touch4_cal_value + touch_thres) || touchRead(13) > (touch3_cal_value + touch_thres) || touchRead(12) > (touch2_cal_value + touch_thres) || touchRead(11) > (touch1_cal_value + touch_thres)  ) {
        myData.touch = 1;
      } else {
        myData.touch = 0;
      }
      
      // Send message via ESP-NOW
      esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
      
      timer = millis();
    }

}
