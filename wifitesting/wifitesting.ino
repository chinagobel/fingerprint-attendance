/*
 WiFiEsp example: WebClientRepeating

 This sketch connects to a web server and makes an HTTP request
 using an Arduino ESP8266 module.
 It repeats the HTTP call each 10 seconds.

 For more details see: http://yaab-arduino.blogspot.com/p/wifiesp.html
*/

#include "WiFiEsp.h"
#include <ArduinoJson.h>
#include <Adafruit_Fingerprint.h>
// Emulate Serial1 on pins 6/7 if not present
//#ifndef HAVE_HWSERIAL1

//SoftwareSerial Serial1(6, 7); // RX, TX
//#endif
#include "SoftwareSerial.h"
SoftwareSerial mySerial(11,12);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);


char ssid[] = "SLT-4G-B11A";            // your network SSID (name)SLT-4G-B11A
char pass[] = "GBJEMDT0700";        // your network password  GBJEMDT0700
int status = WL_IDLE_STATUS;     // the Wifi radio's status
char server[] = "192.168.1.101";////192.168.137.1



String data="_";
uint8_t d1[128];////data packet
uint8_t d2[128];////data packet

// Initialize the Ethernet client object
WiFiEspClient client;

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
   finger.begin(57600);
  
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1);
  }

         downloadandstore(2);
       //  Serial.println(data);
         data="_";
      
        downloadandstore(1);
       // Serial.println(data);
         
         
         
 
   
///////////////////////////////end of storing/////////////////////

 
     


}

void loop()
{
  
}

void downloadandstore(int idnum){

 
 download(idnum);/////download the fingerprint form the sql database
 getValue(data,'_');

 for(int i=0;i<128;i++){
  Serial.print(d1[i]);
  Serial.print(",");
 }
  for(int i=0;i<128;i++){
  Serial.print(d2[i]);
  Serial.print(",");
 }

}




String download(int num){
 
 int adownfig=0;//stop the loop
  
  while(1){
  if (adownfig==0) {
    adownfig++;
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
  
  String a= client.readStringUntil('_');
 
  while (client.available()) {
      data+=client.readStringUntil('_');
      data+="_";
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

void getValue(String data, char separator)
{   
    String d="";////to store the read numbers
    int maxIndex = data.length() - 1;
    int j=0,p=0;
    int index=0;
    for (int i = 1; i <= maxIndex; i++) {
        if(index<128){///////////////////////////for the first array
        if (data.charAt(i) == separator) {
            d1[index]=d.toInt();index++;
            d="";
        }
        else{
          d+=data.charAt(i);
        }
        }else if(index<256){/////////////////////////////////////////for the second array
              if (data.charAt(i) == separator) {
                      d2[index-128]=d.toInt();index++;
                   d="";
        }
        else{
          d+=data.charAt(i);
        }
        }
    }
    
}






