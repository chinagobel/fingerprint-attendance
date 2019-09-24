#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
#include "WiFiEsp.h"
#include <ArduinoJson.h>
#include <Adafruit_Fingerprint.h>

#include <Keypad.h>

char ssid[] = "SLT-4G-B11A";            // your network SSID (name)
char pass[] = "GBJEMDT0700";        // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status

int a=0;//stop the loop
int out=0;
char server[] = "192.168.1.101";////192.168.137.1

uint8_t idnum[2];///to store the data 20 id numbers;

// Initialize the Ethernet client object
WiFiEspClient client;


const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns

//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};


byte rowPins[ROWS] = {22, 24, 26, 28}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {30, 32, 34, 36}; //connect to the column pinouts of the keypad//

//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 





SoftwareSerial mySerial(11,12);///(2,3)//11-purpule 12--white

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
uint8_t id;

LiquidCrystal LCD(10,9,5,4,3,2);
int myCounter=0;
int enroll=6;//for enroll mode push button
int search=7;//for search mode push button
int up=8;//upload mode button
int back=13;//go back or ok button
String sid;///id number in string
int number;////get the id number


void setup() {
  Serial.begin(9600);
  while (!Serial);  // For Yun/Leo/Micro/Zero/...
  delay(100);
  

  Serial3.begin(9600);
  // initialize ESP module
  WiFi.init(&Serial3);
  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }

  // set the data rate for the sensor serial port
  finger.begin(57600);

  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }
  pinMode(enroll,INPUT);
  pinMode(search,INPUT);
  pinMode(up,INPUT);
  pinMode(back,INPUT);
  
  
  LCD.begin(16,4);//lcd has 16 columns 4 rows 
  LCD.clear();
  LCD.setCursor(0,0);
  LCD.print("Select Mode");
  
}

int pushbuttonstate(){
  LCD.clear();
  LCD.setCursor(0,0);
  LCD.print("Enroll or exit");
  while(1){
   if(digitalRead(enroll)==HIGH){
    return 0;
   }else if(digitalRead(back)==HIGH){
      LCD.clear();
      LCD.setCursor(0,0);
      LCD.print("Select Mode");
    return 1;
   }
  }
  
}

uint8_t readnumber(void) {////get the id number form serial
  uint8_t num = 0;
  
  while (num == 0) {
    while (! Serial.available());
    num = Serial.parseInt();
  }
  return num;
}

int checkexist(uint8_t id){

  for(int i=0;i<2;i++){
    if(id==idnum[i]){
      return 1;///valid id
    }
  }
  
  return 0;///not a vali 
  
}



void loop() {

  
  if(digitalRead(enroll)==1){

  ///////set the wifi connection
  LCD.clear();
  LCD.setCursor(0,0);//set the cursor
  LCD.print("Connecting wifi");
  
   while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }
  LCD.clear();
  LCD.setCursor(0,0);//set the cursor
  LCD.print("Connected");
 // Serial.println("You're connected to the network");

  //////////////download id number form sql
  LCD.clear();
  LCD.setCursor(0,0);//set the cursor
  LCD.print("Downloading ID");
  download();

  LCD.clear();
  LCD.setCursor(0,0);//set the cursor
  LCD.print("Completed");
  delay(500);
  
  LCD.clear();
  LCD.setCursor(0,0);//set the cursor
  LCD.print("Enrolling....");
  
  
  while(1){
  LCD.setCursor(0,1);//second row
  LCD.print("Enter ID number");
  id=keypadinput();
 // id=sid.toInt();
   while(!checkexist(id)){
      delay(500);
      LCD.setCursor(0,2);//second row
      LCD.print("Enter valid ID");
      Serial.println("Enter valid input");
      sid=keypadinput();
      id=sid.toInt();
      
    
  }
  LCD.clear();
  LCD.setCursor(0,0);//set the cursor
  LCD.print("Enrolling....");
  LCD.setCursor(0,1);//set the cursor
  LCD.print("ID Number");
  LCD.setCursor(0,2);
  LCD.print(id);
  while (!  getFingerprintEnroll() );
  
  int c=pushbuttonstate();
  if(c==1){
    break;
  }
  }
  
  }else if(digitalRead(search)==1){
    ////searching goes here
    LCD.setCursor(0,0);
    LCD.print("Searching..");
    while(1){
    ////if id=100 admin////the exit
    int admin=getFingerprintIDez();
    if(admin==100){
      LCD.clear();
      LCD.setCursor(0,0);
      LCD.print("Select Mode");
      break;
    }
    delay(50);
    }  
    
    }else if(digitalRead(up)==1){
      /////code to upload the attendace
      LCD.clear();
      LCD.setCursor(0,0);
      LCD.print("Uploading.....");

       for (int finger = 1; finger <=2; finger++) {
          downloadFingerprintTemplate(finger);
      }



      
      LCD.clear();
      LCD.setCursor(0,0);
      LCD.print("Done uploading"); 
      delay(1000);
      LCD.clear();
      LCD.setCursor(0,0);
      LCD.print("Select Mode");
       
      }
  
  // put your main code here, to run repeatedly:

}

////////////////////////get form sensor and upload to sql database


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

  String abc=extractfingerdata(fingerTemplate);
  Serial.println("\ndone.");
    if (client.connect(server, 80)) {
    Serial.println("Connected to server");
    // Make a HTTP request
    client.print("GET /info.php?request=");
    client.print(abc);////string 
    client.print("&id=");
    client.print(id);
    client.println(" HTTP/1.1");
    client.println("Host: 192.168.1.101 ");
    client.println("Connection: close");
    client.println();
  }

  Serial.println("End of sending data");

  

  
}
String extractfingerdata(uint8_t arr[]){
  String s="_";///data string to send to the server starting form _
  int i=0;
  int of=0;
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
        of++;
        s+=arr[i];
        s+='_';
        }
       if(of==128){
        start=false;//stop wirting
        of=0;//reset 
       }
      
        
  }

  /////////////////////////end of the making data string
  return s;
}






////fingerprint Enroll


uint8_t getFingerprintEnroll() {

  int p = -1;
  //Serial.print("Waiting for valid finger to enroll as #"); Serial.println(id);
  LCD.setCursor(0,3);
  LCD.print("Touch");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  LCD.setCursor(0,3);
  LCD.print("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(id);
  p = -1;
  LCD.setCursor(0,3);
  LCD.print("Put same finger");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  
  // OK converted!
  Serial.print("Creating model for #");  Serial.println(id);
  
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }   
  
  Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    LCD.setCursor(0,3);
    LCD.print("Stored!        ");
    delay(500);
    return 1;
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }   
}




/////searching////

uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  
  // OK converted!
  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }   
  
  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID); 
  Serial.print(" with confidence of "); Serial.println(finger.confidence); 

  return finger.fingerID;
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
  LCD.clear();
  LCD.setCursor(0,0);
  LCD.print("Searching..");
  LCD.setCursor(0,1);
  LCD.print(finger.fingerID);
  LCD.setCursor(6,1);
  LCD.print("ID Found");
  //Serial.print("Found ID #");
  //Serial.print(finger.fingerID); 
  //Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return finger.fingerID; 
}

void download(){

 // printWifiStatus();

  while(1){
  if (a==0) {
    a++;
    httpRequest();
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

  
  int i=1;///start form odd
  int arraypos=0;
  while (client.available()) {
    
   String line = client.readStringUntil('"');
   String data=client.readStringUntil('"');
   if(i%2==0){
       
        idnum[arraypos]=data.toInt();
        arraypos++;///increase the array position
      
    }
    i++;///increment i
  
  }
  
  client.stop();



   
  out++;
   } 
}





// this method makes a HTTP connection to the server
void httpRequest()
{
  Serial.println();
    
  // close any connection before send a new request
  // this will free the socket on the WiFi shield
  client.stop();

  // if there's a successful connection
  if (client.connect(server, 80)) {
    Serial.println("Connecting...");
    
    // send the HTTP PUT request
    client.println(F("GET /try.php HTTP/1.0"));
    client.println(F("Host: 192.168.1.101"));
    client.println("Connection: close");
    client.println();
  }
  else {
    // if you couldn't make a connection
    Serial.println("Connection failed");
  }
}





uint8_t keypadinput(){
    String input;
    uint8_t pos=0;
   // LCD.setCursor(0,2);            ////new added
   // LCD.print("                ");
    while(digitalRead(back)==0){////ok button
    char customKey = customKeypad.getKey();
    
    if (customKey){
                LCD.setCursor(pos,2);
                
                LCD.print(customKey);
                pos++;
                input+=customKey;
               
               }
    }
   // Serial.println(input);
    int vbn=input.toInt();
    return vbn;
}
