/*
 WiFiEsp example: WebClientRepeating

 This sketch connects to a web server and makes an HTTP request
 using an Arduino ESP8266 module.
 It repeats the HTTP call each 10 seconds.

 For more details see: http://yaab-arduino.blogspot.com/p/wifiesp.html
*/

#include "WiFiEsp.h"
#include <ArduinoJson.h>

// Emulate Serial1 on pins 6/7 if not present
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
//SoftwareSerial Serial1(6, 7); // RX, TX
#endif

char ssid[] = "SLT-4G-B11A";            // your network SSID (name)SLT-4G-B11A
char pass[] = "GBJEMDT0700";        // your network password  GBJEMDT0700
int status = WL_IDLE_STATUS;     // the Wifi radio's status
int a=0;//stop the loop
String data="_";
char server[] = "192.168.1.103";////192.168.137.1
uint8_t figarr[256];/////fingerprint data array
uint8_t d1[128];
uint8_t d2[128];

// Initialize the Ethernet client object
WiFiEspClient client;
//String line="";
void setup()
{
  // initialize serial for debugging
  Serial.begin(9600);
  // initialize serial for ESP module
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
                download(1);
 //Serial.println(data);
 
 getValue(data,'_');////extract fingerpritn data
 for(int i=0;i<256;i++){
  Serial.print(figarr[i]);
  Serial.print(",");
 }
     //d1=figarr[0:127];
     //d2=figarr[128:255];


}

void loop()
{
  
}
void download(int num){
  
  while(1){
  if (a==0) {
    a++;
    httpRequestdown(num);
  }
    // Check HTTP status
  char status[32] = {0};
  client.readBytesUntil('\r', status, sizeof(status));
  // It should be "HTTP/1.0 200 OK" or "HTTP/1.1 200 OK"
  if (strcmp(status + 9, "200 OK") != 0) {
    Serial.print(F("Unexpected response: "));
    Serial.println(status);
    return;
  }

  // Skip HTTP headers
  char endOfHeaders[] = "<body>";//"\r\n\r\n";
  if (!client.find(endOfHeaders)) {
    Serial.println(F("Invalid response"));
    return;
  }
   int number=0;///count the data size 256
  String a= client.readStringUntil('_');
 
  while (client.available()) {
       data+=client.readStringUntil('_');
       data+="_";
 
   
   if(number==255){
  
      client.readStringUntil('"');
      break;
    
    }
    number++;
    
  }
  
      client.stop();
  }

  




}

// this method makes a HTTP connection to the server
void httpRequestdown(int num)
{
  Serial.println();
    
  // close any connection before send a new request
  // this will free the socket on the WiFi shield
  client.stop();

  // if there's a successful connection
  if (client.connect(server, 80)) {
    Serial.println("Connecting...");
    
    // send the HTTP PUT request
    client.print("GET /tryjson.php?id=");
    client.print(num);/// 
    client.println(" HTTP/1.0");
    client.println(F("Host: 192.168.1.103"));
    client.println("Connection: close");
    client.println();
  }
  else {
    // if you couldn't make a connection
    Serial.println("Connection failed");
  }
}

void getValue(String data, char separator)
{   
    String d="";
    int maxIndex = data.length() - 1;
    int j=0,p=0;
    int index=0;
    for (int i = 1; i <= maxIndex; i++) {
        if (data.charAt(i) == separator) {
            figarr[index]=d.toInt();index++;
            d="";
        }
        else{
          d+=data.charAt(i);
        }
    }
    
}




