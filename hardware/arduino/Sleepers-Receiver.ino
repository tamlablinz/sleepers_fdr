//MAC nuevo s2 48:27:E2:50:76:82


#include <Adafruit_TinyUSB.h>
#include <MIDI.h>

#include <WiFi.h>
//#include </Users/kike/Library/Arduino15/packages/esp32/hardware/esp32/2.0.9/libraries/WiFi/src/WiFi.h>
#include <esp_now.h>
#include "esp_wifi.h"


// USB MIDI object
Adafruit_USBD_MIDI usb_midi;

// Create a new instance of the Arduino MIDI Library,
// and attach usb_midi as the transport.
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, MIDI);

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
  uint8_t id;
  uint8_t piezo;
  float accelX;
  float accelY;
  uint8_t touch;
} struct_message;

// Create a struct_message called myData
struct_message myData;

int ID;

int notes_piezos[] = {60,62,64,65,67,69,71,72};
int factorX[] = {15,25,10,12,15,25,10,12};

float maX = 0;  //variables for acceleration in MIDI format
float maY = 0;

//controlling variables for midi
int touch1, touch1_last, touch2, touch2_last, touch3, touch3_last, touch4, touch4_last = -1;
int touch5, touch5_last, touch6, touch6_last, touch7, touch7_last, touch8, touch8_last = -1;
int touch_last[] = {-1,-1,-1,-1,-1,-1,-1,-1};
int piezo_last[] = {-1,-1,-1,-1,-1,-1,-1,-1};
float accelX_last[] = {-1,-1,-1,-1,-1,-1,-1,-1};
float accelY_last[] = {-1,-1,-1,-1,-1,-1,-1,-1};
float maX_last[] = {-1,-1,-1,-1,-1,-1,-1,-1};
float maY_last[] = {-1,-1,-1,-1,-1,-1,-1,-1};

int accelX1, accelX1_last, accelX2, accelX2_last, accelX3, accelX3_last, accelX4, accelX4_last = -1; 
int accelX5, accelX5_last, accelX6, accelX6_last, accelX7, accelX7_last, accelX8, accelX8_last = -1; 

int accelY1, accelY1_last, accelY2, accelY2_last, accelY3, accelY3_last, accelY4, accelY4_last = -1; 
int accelY5, accelY5_last, accelY6, accelY6_last, accelY7, accelY7_last, accelY8, accelY8_last = -1; 

int piezo1, piezo1_last, piezo2, piezo2_last, piezo3, piezo3_last, piezo4, piezo4_last = -1; 
int piezo5, piezo5_last, piezo6, piezo6_last, piezo7, piezo7_last, piezo8, piezo8_last = -1;

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  
  memcpy(&myData, incomingData, sizeof(myData));

  //GET ID
  ID = myData.id;

  //PRINT INFO
  //Serial.print("ID: ");
  //Serial.println(myData.id);
  //Serial.print("X: ");
  //Serial.println(myData.accelX);
  //Serial.print("mX: ");
  //Serial.println(maX);
  //Serial.print("Y: ");
  //Serial.println(myData.accelY);
  //Serial.print("mY: ");
  //Serial.println(maY);
  //Serial.print("Touch: ");
  //Serial.println(myData.touch);
  //Serial.print("Piezo: ");
  //Serial.println(myData.piezo);
  //Serial.println();
  

  //TOUCH
  if(myData.touch != touch_last[ID-1]) {
      MIDI.sendControlChange(ID*10 + 3, myData.touch * 127, ID); //touch is only a trigger 0 or 1
      touch_last[ID-1] = myData.touch;
  }


  //PIEZO data to MIDI (myData.piezo is a velocity)
  if(myData.piezo != piezo_last[ID-1]){
    if(myData.piezo > 1) {
      MIDI.sendNoteOn(notes_piezos[ID-1], int(round(map(myData.piezo, 0, 1200,0,127))), ID);
    } 
    if(myData.piezo == 0) {
      MIDI.sendNoteOff(notes_piezos[ID-1], 0, ID);
    }
    piezo_last[ID-1] = myData.piezo;
  }

  //ACCELERATION X (VERTICAL)
  /*
  Serial.println("last myData.accelY: ");  
  Serial.println(myData.accelY);
  Serial.println("myData.accelY: ");  
  Serial.println(accelY_last[ID-1]);
  */
  if(myData.accelX != accelX_last[ID-1]) {
    //Serial.println("diferente ");  
    //Use different calibration data per Sleeper
    switch (ID) {
    case 1:
      if(myData.accelX > 0 ) {
        maX = map(myData.accelX,-0.2,44, 0, 127);
       } else {
        maX = 0;
       }
      
      //Serial.print("IDx: ");
      //Serial.println(myData.id);
      //Serial.println("data ");  
      //Serial.println( myData.accelX);
      break;
    case 2:
    
      
      
      if(myData.accelX < 0 ) {
        maX = map(myData.accelX,-0.6,-39, 0, 127);
      } else {
        maX = 0;
       }
      maX = map(myData.accelX,-0.6,-39, 0, 127);
      /*
      Serial.print("IDx: ");
      Serial.println(myData.id);
      Serial.println("data ");  
      Serial.println( myData.accelX);
      Serial.println("midi "); 
      Serial.println(maX);
      */
      break;
    case 3:
      if(myData.accelX > 0 ) {
        maX = map(myData.accelX,-0.8,21, 0, 127); 
      }
      else {
        maX = 0;
       }
      break;
    case 4:
      
      if(myData.accelX > 0 ) {
        maX = map(myData.accelX,-0.7,34, 0, 127); 
      }
      else {
        maX = 0;
       }
      
      break;  
    case 5:
      
      if(myData.accelX < -1.0 ) {
        maX = map(myData.accelX,-1.7,-31, 0, 127);
      }
      else {
        maX = 0;
       }
      break;
    case 6:
      
      if(myData.accelX < 0 ) {
        maX = map(myData.accelX,-0.6,-14, 0, 127);
      }
      else {
        maX = 0;
       }
      break;
    case 7:
      
      if(myData.accelX > 0 ) {
        maX = map(myData.accelX,-1.1,36, 0, 127);
      }
      else {
        maX = 0;
       }
      break;
    case 8:
      
      if(myData.accelX > -1 ) {
        maX = map(myData.accelX,-1.4,49, 0, 127);
      }
      else {
        maX = 0;
       }
      break;    
    default:
      // if nothing else matches, do the default
      // default is optional
      break;
    }
    if(maX != maX_last[ID-1]) {
      //Serial.println("send accel X");
      MIDI.sendControlChange(ID*10 + 1, maX, ID); //accelX
      maX_last[ID-1] = maX; 
    }
    
    accelX_last[ID-1] = myData.accelX;   
  }

  //ACCELERATION Y (HORIZONTAL)
  /*
  Serial.println("last myData.accelY: ");  
  Serial.println(myData.accelY);
  Serial.println("myData.accelY: ");  
  Serial.println(accelY_last[ID-1]);
  */
  
  if(myData.accelY != accelY_last[ID-1]) {
    //Use different calibration data per Sleeper
    float var = 0;
    switch (ID) {
    case 1:
      /*
      Serial.println("IDy: ");  
      Serial.println(myData.id);
      Serial.println("myData.accelY: ");  
      Serial.println(myData.accelY);
      */
      maY = map(myData.accelY,-12,12, 0, 127);
      
      break;
    case 2:
      maY = map(myData.accelY,-10,10, 0, 127);
      break;
    case 3:
      maY = map(myData.accelY,-10,10, 0, 127);
      break;
    case 4:
      maY = map(myData.accelY,-10,10, 0, 127);
      break;  
    case 5:
      maY = map(myData.accelY,-10,10, 0, 127);
      break;
    case 6:
      maY = map(myData.accelY,-8,8, 0, 127);
      break;
    case 7:
      maY = map(myData.accelY,-9,9, 0, 127);
      break;
    case 8:
      maY = map(myData.accelY,-9,9, 0, 127);
      break;    
    default:
      // if nothing else matches, do the default
      // default is optional
      break;
    }
    
    //Serial.println("send accel Y");
    //MIDI.sendControlChange(ID*10 + 2, maY, ID); //accelX 
    
    if(maY != maY_last[ID-1]) {
      //Serial.println("send accel Y");
      MIDI.sendControlChange(ID*10 + 2, maY, ID); //accelX
      maY_last[ID-1] = maY; 
    }
    accelY_last[ID-1] = myData.accelY;
  }

    
}
 
void setup() {
  // Initialize Serial Monitor
  //Serial.begin(115200);
  
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    //Serial.println("Error initializing ESP-NOW");
    return;
  }

  WiFi.setSleep(false);
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);
  //Serial.println("Listening for ESP-NOW");

    // wait until device mounted
  //while( !TinyUSBDevice.mounted() ) delay(1);

  //Comment this or not depending on MIDI
  usb_midi.begin();
  

}
 
void loop() {

}



  //ACCEL CALIBRATIONS
  //sleeper1
  //maX = map(myData.accelX,-1,40, 0, 127);
  //maY = map(myData.accelY,-10,10, 0, 127);
  //sleeper2
  //maX = map(myData.accelX,1,-30, 0, 127);
  //maY = map(myData.accelY,-8,8, 0, 127);
  //sleeper3
  //maX = map(myData.accelX,-1,20, 0, 127);
  //maY = map(myData.accelY,-8,8, 0, 127);
  //sleeper4
  //maX = map(myData.accelX,-1,35, 0, 127);
  //maY = map(myData.accelY,-10,10, 0, 127);
  //sleeper5
  //maX = map(myData.accelX,1,-25, 0, 127);
  //maY = map(myData.accelY,-9,9, 0, 127);
  //sleeper6
  //maX = map(myData.accelX,-0.5,11, 0, 127);
  //maY = map(myData.accelY,-6,6, 0, 127);
  //sleeper7
  //maX = map(myData.accelX,-0.8,31, 0, 127);
  //maY = map(myData.accelY,-7,7, 0, 127);
  //sleeper8
  //maX = map(myData.accelX,-1,41, 0, 127);
  //maY = map(myData.accelY,-7,7, 0, 127);
