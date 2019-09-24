/*************************************************** 
  This is an example sketch for our optical Fingerprint sensor

  Designed specifically to work with the Adafruit BMP085 Breakout 
  ----> http://www.adafruit.com/products/751

  These displays use TTL Serial to communicate, 2 pins are required to 
  interface
  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/


#include <Adafruit_Fingerprint.h>
#include "WiFiEsp.h"
// On Leonardo/Micro or others with hardware serial, use those! #0 is green wire, #1 is white
// uncomment this line:
// #define mySerial Serial1

// For UNO and others without hardware serial, we must use software serial...
// pin #2 is IN from sensor (GREEN wire)
// pin #3 is OUT from arduino  (WHITE wire)
// comment these two lines if using hardware serial
SoftwareSerial mySerial(11, 12);

char ssid[] = "SLT-4G-B11A";            // your network SSID (name)SLT-4G-B11A
char pass[] = "GBJEMDT0700";        // your network password  GBJEMDT0700
int status = WL_IDLE_STATUS;     // the Wifi radio's status
char server[] = "192.168.1.102";////192.168.137.1
WiFiEspClient client;
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
boolean attendance[1000];

void setup()  
{
  pinMode(13,INPUT);
  Serial.begin(9600);
  while (!Serial);  // For Yun/Leo/Micro/Zero/...
  delay(100);
  Serial.println("\n\nAdafruit finger detect test");

  // set the data rate for the sensor serial port
  finger.begin(57600);
  
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }

  Serial3.begin(9600);
  // initialize ESP module
  WiFi.init(&Serial3);

  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }

  // attempt to connect to WiFi network
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }

  Serial.println("You're connected to the network");

  finger.getTemplateCount();
  Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  Serial.println("Waiting for valid finger...");
   
   while(!digitalRead(13)){
         getFingerprintIDez();delay(50);        
   }
 Serial.println("End of taking attendance");
 /////////////////////upload the attendance
 for(int i=1;i<=1000;i++){
    if(attendance[i]==1){
      httpRequest(i);
    }
 }
 Serial.println("End of sending data");




  
}


 

void loop()                     // run over and over again
{
      //don't ned to run this at full speed.
}



// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;
  
  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  attendance[finger.fingerID]=1;
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return finger.fingerID; 
}

// this method makes a HTTP connection to the server
void httpRequest(int stid)
{
  Serial.println();
    
  // close any connection before send a new request
  // this will free the socket on the WiFi shield
  client.stop();

  // if there's a successful connection
  if (client.connect(server, 80)) {
    Serial.println("Connecting...");
    
    // send the HTTP PUT request
    client.print("GET /att.php?id=");
    client.print(stid); 
    client.println(" HTTP/1.0");
    client.print("Host: ");
    client.println(server);
    client.println("Connection: close");
    client.println();
  }
  else {
    // if you couldn't make a connection
    Serial.println("Connection failed");
  }
}

