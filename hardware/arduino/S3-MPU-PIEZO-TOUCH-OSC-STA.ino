#include <WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <WiFiAP.h>

#include "Wire.h"
#include <MPU6050_light.h>
#include "Adafruit_ZeroFFT.h"

//IP address to send UDP data to:
// either use the ip address of the server or 
// a network broadcast address

// WiFi network name and password:
const char * networkName = "rockmore";
const char * networkPswd = "clararockmore";

//IPAddress udpAddress(192, 168, 0, 2);  //just a dummy, it can be configured via browser
//const IPAddress udpAddress(192, 168, 0, 100);
IPAddress udpAddress(192, 168, 0, 100);
int udpPort = 44444;                   //just a dummy, it can be configured via browser

// Your name for access point.
const char *APssid = "sleeper";


//The udp library class
WiFiUDP udp;

int touchValue = 0;

boolean connected = false;     //wifi connection
bool accesspoint = true;       //acccess point mode
boolean APconnected = false;   //access point connected
bool registered = false;       //wifi handler registration


// Set your Static IP address (dummy values initialization)
IPAddress staIP(192,168,0,129);         //Board static IP
IPAddress staGateway(192,168,0,1);      //Gateway IP
IPAddress staSubnet(255,255,255,0);     //Subnet range
IPAddress primaryDNS(192, 168, 0, 1);   //optional
IPAddress secondaryDNS(8, 8, 4, 4);     //optional

MPU6050 mpu(Wire);
unsigned long timer = 0;
unsigned long sending_rate = 10;
const int i2c_sda = 42;
const int i2c_scl = 41;


#define DATA_SIZE 128

int16_t data[DATA_SIZE];
int FFT_threshold = 50;
int trig = 1;



void setup(){
  // Initilize hardware serial:
  Serial.begin(115200);

  //pinMode(2, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  Wire.setPins(i2c_sda, i2c_scl);
  Wire.begin();
  
  byte status = mpu.begin();
  Serial.print(F("MPU6050 status: "));
  Serial.println(status);
  while(status!=0){ } // stop everything if could not connect to MPU6050
  
  Serial.println(F("Calculating offsets, do not move MPU6050"));
  delay(1000);
  // mpu.upsideDownMounting = true; // uncomment this line if the MPU6050 is mounted upside-down
  mpu.calcOffsets(); // gyro and accelero
  Serial.println("Done!\n");


  //Connect to the WiFi network
  connectToWiFi(networkName, networkPswd);
  
}

void loop(){
  digitalWrite(LED_BUILTIN, LOW);
  //only send data when connected
  if(connected){
    //Send a packet
    int32_t avg = 0;
    for(int i=0; i<DATA_SIZE; i++){
      int16_t val = analogRead(4);
      avg += val;
      data[i] = val;
    }

    //remove DC offset and gain up to 16 bits
    avg = avg/DATA_SIZE;
    for(int i=0; i<DATA_SIZE; i++) data[i] = (data[i] - avg) * 64;
    
    //run the FFT
    ZeroFFT(data, DATA_SIZE);
    /*
    for(int i=0; i<DATA_SIZE/8; i++) {
      Serial.print(data[i]);
      Serial.print(" ");
    }
    Serial.println(" ");
    */
    if(data[0] > FFT_threshold) {
      digitalWrite(LED_BUILTIN, HIGH);
      OSCMessage msg("/trigger1");
      
      msg.add(trig);

      udp.beginPacket(udpAddress, udpPort);
      msg.send(udp);
      udp.endPacket();
      msg.empty(); 

      delay(5); //to hardcode a small wait
    }

    
    
    mpu.update();
    
    if((millis()-timer)>sending_rate){ // print data every 10ms
      /*
      Serial.print("X : ");
      Serial.print(mpu.getAngleX());
      Serial.print("\tY : ");
      Serial.print(mpu.getAngleY());
      Serial.print("\tZ : ");
      Serial.println(mpu.getAngleZ());
      */
      
      OSCMessage msg("/sleeper1");
      msg.add((float) mpu.getAngleX());
      msg.add((float) mpu.getAngleY());
      msg.add((float) mpu.getAngleZ());
      msg.add((float) touchRead(14));
      msg.add((float) touchRead(13));
      msg.add((float) touchRead(12));
      msg.add((float) touchRead(11));

      //touchValue = touchRead(5);  //GPIO 15
      /*
      Serial.print("touch: ");
      Serial.print(touchValue);
      Serial.println("");
      */
    
      udp.beginPacket(udpAddress, udpPort);
      msg.send(udp);
      udp.endPacket();
      msg.empty();
    
      timer = millis();  
    }
  
    
  }

}



void connectToWiFi(String ssid, String pwd){
  
  //register event handler. This allows receiving messages of connection, disconnections, etc
  if(!registered){
    registered = true;
    WiFi.onEvent(WiFiEvent);
  }
  
  // delete old config if already connected
  WiFi.disconnect(true);
  
  //we will simply use gateway as DNS address
  primaryDNS = staGateway;
  secondaryDNS = staGateway;
  
  
  Serial.print("connecting to: ");
  Serial.println(ssid);

  //connect to wifi
  WiFi.begin(ssid.c_str(), pwd.c_str());  //I use c_str() as we have to convert strings to arrays of characters
  //Comment below for DHCP
  WiFi.config(staIP, staGateway, staSubnet, primaryDNS,secondaryDNS);   //fix IP at network

  //unfortunately the following status is not always well caught, I will comment it
  //until the bug is solved
  //while (WiFi.status() != WL_CONNECTED) { //wait until connected
  //      delay(500);
  //      Serial.print(".");
  //  }
  
  //instead of waiting until connected, let´s give the ESP a bit of time to connect
  //if previous bug is solved, the following line can be removed
  delay(2000);

  //Some info
  Serial.println("");
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("ESP Mac Address: ");
  Serial.println(WiFi.macAddress());
  Serial.print("Subnet Mask: ");
  Serial.println(WiFi.subnetMask());
  Serial.print("Gateway IP: ");
  Serial.println(WiFi.gatewayIP());
  Serial.print("DNS: ");
  Serial.println(WiFi.dnsIP());

  
}

//wifi event handler
void WiFiEvent(WiFiEvent_t event){
    switch(event) {
      case SYSTEM_EVENT_STA_GOT_IP:
          //When connected set 
          Serial.print("WiFi connected! IP address: ");
          Serial.println(WiFi.localIP());  
          //initializes the UDP state
          //This initializes the transfer buffer
          udp.begin(WiFi.localIP(),udpPort);
          connected = true;
          break;
      case SYSTEM_EVENT_STA_DISCONNECTED:
          Serial.println("WiFi lost connection");
          connected = false;
          break;
    }
}
