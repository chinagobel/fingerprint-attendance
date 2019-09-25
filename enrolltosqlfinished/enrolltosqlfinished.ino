/*************************************************** 
  This is an example sketch for our optical Fingerprint sensor

  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#include "WiFiEsp.h"
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
// Emulate Serial1 on pins 6/7 if not present
//#ifndef HAVE_HWSERIAL1
//#endif

char ssid[] = "SLT-4G-B11A";            // your network SSID (name)
char pass[] = "GBJEMDT0700";        // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status

char server[] = "192.168.1.102";///localhost

// Initialize the Ethernet client object
WiFiEspClient client;

int getFingerprintIDez();

// pin #2 is IN from sensor (GREEN wire)
// pin #3 is OUT from arduino  (WHITE wire)
SoftwareSerial mySerial(11, 12);


Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

boolean idnumbers[1000]; 
//String b="109_29_209_253_114_160_39_189_91_171_86_146_30_47_79_218_18_49_21_82_98_151_103_211_85_171_106_211_22_50_38_248_49_48_14_246_44_176_104_22_48_181_8_246_45_52_38_117_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_0_20_84_3_1_74_32_149_0_255_30_224_6_128_2_128_2_128_2_128_0_128_0_128_0_128_0_128_2_128_2_128_2_128_6_128_30_0_30_0_62_1_254_255_255_255_255_255_255_255_255_255_255_255_0_0_0_0_0_95_134_18_62_98_10_81_30_105_143_38_222_26_149_129_222_30_36_214_190_35_46_148_190_70_7_103_151_78_12_18_31_59_144_85_159_41_26_87_159_73_29_146_95_81_47_82_159_93_160_81_188_104_160_79_188_76_41_18_60_25_50_170_28_99_164_39_29_70_171_41_21_36_169_9_188_76_41_18_60_25_50_170_28_99_164_39_29_70";
void setup()  
{
  while(!Serial);
  Serial.begin(9600);
  Serial.println("Fingerprint template extractor");

  // set the data rate for the sensor serial port
  finger.begin(57600);
  
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1);
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

  // you're connected now, so print out the data
  Serial.println("You're connected to the network");
  
    /////for testing 
    ///id=50,156  ///idnumbers[1000]
    idnumbers[50]=1;
    idnumbers[156]=1;
    
      for(int finger=1;finger<200;finger++){
      Serial.println(finger);
          if(idnumbers[finger]==1){
           downloadFingerprintTemplate(finger);
          }
  }
  Serial.println("out of the loop");
}


uint8_t downloadFingerprintTemplate(uint16_t id)
{ 
 

  
  Serial.println("------------------------------------");
  Serial.print("Attempting to load #"); Serial.println(id);
  uint8_t p = finger.loadModel(id);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.print("Template "); Serial.print(id); Serial.println(" loaded");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    default:
      Serial.print("Unknown error "); Serial.println(p);
      return p;
  }

  // OK success!

  Serial.print("Attempting to get #"); Serial.println(id);
  p = finger.getModel();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.print("Template "); Serial.print(id); Serial.println(" transferring:");
      break;
   default:
      Serial.print("Unknown error "); Serial.println(p);
      return p;
  }
  
  // one data packet is 267 bytes. in one data packet, 11 bytes are 'usesless' :D
  uint8_t bytesReceived[534]; // 2 data packets
  memset(bytesReceived, 0xff, 534);

  uint32_t starttime = millis();
  int i = 0;
  while (i < 534 && (millis() - starttime) < 20000) {
      if (mySerial.available()) {
          bytesReceived[i++] = mySerial.read();
      }
  }
  Serial.print(i); Serial.println(" bytes read.");
  Serial.println("Decoding packet...");

  uint8_t fingerTemplate[512]; // the real template
  memset(fingerTemplate, 0xff, 512);

  // filtering only the data packets
  int uindx = 9, index = 0;
  while (index < 534) {
      while (index < uindx) ++index;
      uindx += 256;
      while (index < uindx) {
          fingerTemplate[index++] = bytesReceived[index];
      }
      uindx += 2;
      while (index < uindx) ++index;
      uindx = index + 9;
  }
   for (int i = 0; i < 512; ++i) {
      //Serial.print("0x");
      printHex(fingerTemplate[i], 2);
      //Serial.print(", ");
  }

   String abc=extractfingerdata(fingerTemplate);
  ////////////////////////////////////////String abc="aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
  
  Serial.println("\ndone.");
 
    if (client.connect(server, 80)) {
    Serial.println("Connected to server");
    // Make a HTTP request
    client.print("GET /info.php?request=");
    client.print(abc);////string 
    client.print("&id=");
    client.print(id);
    client.println(" HTTP/1.1");
    client.println("Host: 192.168.1.102 ");
    client.println("Connection: close");
    client.println();
  }

  Serial.println("End of sending data");

  

  
}

String extractfingerdata(uint8_t arr[]){
  String s="";///data string to send to the server
  int i=0;
  int stop=0;
  boolean start=false;//start storing
  for(i;i<500;i++){ ////go through the array
    if(arr[i]==239&&arr[i+1]==1){ ///check wether data packet or not EF01
        stop++;
        start=true;
        i+=9;/////skip the header file to get to the data
      }
      if(stop==3){///stop after 3 EF01 are found
        break;
        }
      if(start){////start storing data into the string
        s+=arr[i];
        s+='_';
        }
        
  }

  /////////////////////////end of the making data string
  return s;
}

void printHex(int num, int precision) {
    char tmp[16];
    char format[128];
 
    sprintf(format, "%%.%dX", precision);
 
    sprintf(tmp, format, num);
    Serial.print(tmp);
}


void loop()
{}


