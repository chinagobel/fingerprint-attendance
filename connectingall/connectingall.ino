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
char server[] = "192.168.1.103";////192.168.137.1

//uint8_t idnum[2];///to store the data 20 id numbers;

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

//////
boolean enrollflag=false; ///check wether the enroll happen
boolean attendanceflag=false; ///check wether the attendance taken
boolean studentids[1000];///to store the ids
boolean attendance[1000];////store the attendance
String classnum;
////for the storing process of fingerprint back to the senosr
String data="_";
uint8_t d1[128];////data packet
uint8_t d2[128];////data packet


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

   //////welcome message
   LCD.setCursor(7,1);
   LCD.print("SAMS");
   delay(3000);

   /////////setting the wifi 
   LCD.clear();
   LCD.setCursor(0,0);
   LCD.print("change wifi-(1)");
   LCD.setCursor(0,1);
   LCD.print("or press back");
   
   
   ///////////////////////////////////enroll button to change wifi back button to exit
   while(1){
       if(digitalRead(enroll)){
        delay(500);
        ///////setting the wifi code goes here
        break;
       }else if(digitalRead(back)){
        break;////go out
       }
    
   }
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

int checkexist(int id){

/*  for(int i=0;i<10000;i++){ ////go through the id number
    if(id==idnum[i]){
      return 1;///valid id
    }
  }
  
  return 0;///not a vali 
  
  */
  if(studentids[id]==1){
    return 1; ////if the id is available
  }else{
    return 0;
  }

  
}



void loop() {
 
  
  if(digitalRead(enroll)==1){
  if(!attendanceflag){/////////////start if attendance upload
  /////empty the fingerprint data base
  finger.emptyDatabase();
  //////selecting the class
  LCD.setCursor(0,1);
  LCD.print("Enter the class");

  classnum=keypadinputString();////get the class form the keypad

  ////////////////////////////////////////////////////////

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
  download();/////////////////download and store the ids
  a=0;/////////reset a for the next id http request
  LCD.clear();
  LCD.setCursor(0,0);//set the cursor
  LCD.print("Completed");
  delay(500);

  /////check wether the student id array is empty
  if(notnull()){ /////if the array contains value enroll
         enrollflag=true;////set the flag true
         LCD.clear();
         LCD.setCursor(0,0);//set the cursor
         LCD.print("Enrolling....");
  
  
        while(1){
              LCD.setCursor(0,1);//second row
              LCD.print("Enter ID number");
              id=keypadinput();
            /////wait until valid id entered
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

       }else{
    LCD.setCursor(0,0);
    LCD.print("Class empty");
    delay(1000);
    LCD.clear();
    LCD.setCursor(0,0);
    LCD.print("Select Mode");
   ////// break;////exit form the enrolling
  }
  }else{
      LCD.clear();
      LCD.setCursor(0,0);
      LCD.print("upload attdnce");
     // LCD.setCursor(0,1);
    //  LCD.print("data.");
      delay(500);
      LCD.clear();
      LCD.setCursor(0,0);
      LCD.print("Select Mode");

    
  }
  
 //////////////////////////////////////////////searching
  }else if(digitalRead(search)==1){
    ////searching goes here
    if(!enrollflag){ //////////////if the enroll does not don
    
    finger.emptyDatabase();////empty the data base
    restatt();///reset the id number and attendance data
    LCD.setCursor(0,0);
    LCD.print("Attendance");
    LCD.setCursor(0,1);
    LCD.print("Enter the class");

    classnum=keypadinputString();////get the class form the keypad

    //////download the id's for the class
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
             download();/////////////////download and store the ids
             a=0;//////rest a for the next id http request
             LCD.clear();
             LCD.setCursor(0,0);//set the cursor
             LCD.print("Completed");
           delay(500);
///////////////downloading id complete
           /////check wether the select class is not null
     if(notnull()) {  ///////////if the class is not null
              attendanceflag=true;///set the attendance flag
              LCD.clear();
              LCD.setCursor(0,0);
              LCD.print("Download Fdata");
              /////////////////start downloading the fingerprints form the db and store in sensor
              for(int i=0;i<1000;i++){
                if(studentids[i]==1){////if the id exsits download the fingerprint
                          downloadandstore(i);
                           data="_";/////reset the data string for the next fingerprint
                }
              }
              
               LCD.clear();
               LCD.setCursor(0,1);
               LCD.print("complete");
               delay(2000);
               LCD.clear();
               LCD.setCursor(0,0);
               LCD.print("Attendance.");
               

                ////start taking attendance
                while(!digitalRead(back)){///////until the back button press take the attendance
                        getFingerprintIDez();delay(50);    ///getFingerprintIDez()    
                    }
              LCD.clear();
              LCD.setCursor(0,0);
              LCD.print("End of marking");
              delay(1000);
              LCD.clear();
              LCD.setCursor(0,0);
              LCD.print("Select Mode");
    }else{//////if the class is empty
      LCD.clear();
      LCD.setCursor(0,0);
      LCD.print("Class empty");
      delay(1000);
      LCD.clear();
      LCD.setCursor(0,0);
      LCD.print("Select Mode");
    }

  
    }else{ ////////upload should done before taking attendance
      LCD.clear();
      LCD.setCursor(0,0);
      LCD.print("upload enroll");
      LCD.setCursor(0,1);
      LCD.print("data.");
      delay(500);
      LCD.clear();
      LCD.setCursor(0,0);
      LCD.print("Select Mode");
    }
   ///////////////////end of search
 /////////////////////////////////////uploading/////////////////////
    }else if(digitalRead(up)==1){

      // enrollflag=false; ///check wether the enroll happen
     //// attendanceflag=false;

      if(enrollflag){////////////////////////////////////upload enroll
      LCD.clear();
      LCD.setCursor(0,0);
      LCD.print("Uploading.....");

       for (int finger = 1; finger <=1000; finger++) {
          if(studentids[finger]==1){////////////////upload the fingerpirnt to database
          downloadFingerprintTemplate(finger);
          }
      }
      enrollflag=false;////reset the enroll flag
      LCD.clear();
      LCD.setCursor(0,0);
      LCD.print("Done uploading"); 
      delay(1000);
      LCD.clear();
      LCD.setCursor(0,0);
      LCD.print("Select Mode");




        
        
      }else if(attendanceflag){/////upload attendance

      LCD.clear();
      LCD.setCursor(0,0);
      LCD.print("Uploading.....");

        ////////attendance
       for (int i =1; i<1000;i++) {
                 if(attendance[i]==1){
                         httpReq(i);
                  }
          
      }
      attendanceflag=false;///reset the attendance flag
      LCD.clear();
      LCD.setCursor(0,0);
      LCD.print("Done uploading"); 
      delay(1000);
      LCD.clear();
      LCD.setCursor(0,0);
      LCD.print("Select Mode");
        
        
      }else{///////no data to upload
        LCD.clear();
        LCD.setCursor(0,0);
        LCD.print("No data to upload");
        delay(1000);
        LCD.clear();
        LCD.setCursor(0,0);
        LCD.print("Select Mode");
        
      }
      /////code to upload the attendace
      
       
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

  for (int i = 0; i < 512; ++i) {
      //Serial.print("0x");
      printHex(fingerTemplate[i], 2);
      //Serial.print(", ");
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
    client.print("Host: ");
    client.println(server);
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

void printHex(int num, int precision) {
    char tmp[16];
    char format[128];
 
    sprintf(format, "%%.%dX", precision);
 
    sprintf(tmp, format, num);
    Serial.print(tmp);
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
  attendance[finger.fingerID]=1;/////mark the attendance
  //Serial.print("Found ID #");
  //Serial.print(finger.fingerID); 
  //Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return finger.fingerID; 
}
/////////////////////download id's form sql database


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
 // int arraypos=0;
  while (client.available()) {
    
   String line = client.readStringUntil('"');
   String data=client.readStringUntil('"');
   if(i%2==0){
       
        //idnum[arraypos]=data.toInt();
        studentids[data.toInt()]=1;//////if there is a id number then set 1 
       /// arraypos++;///increase the array position
      
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
    client.print("Host:  ");
    client.println(server);
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

///////to get a string form keypad
String keypadinputString(){
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
    ///int vbn=input.toInt();
    return input;
}

////////////////////check the boolean studentids wether it has values
boolean notnull(){
  for(int i=0;i<1000;i++){
    if(studentids[i]==1){
      return 1;//////not null arrya
    }
  }
  return 0;//null arary
}

//////////////reset the attendance and id number for attendance
void restatt(){
  for(int i=0;i<1000;i++){
     studentids[i]=0;
     attendance[i]=0;
  }
  
}

////////////reset id for enrolling
void restenroll(){
  for(int i=0;i<1000;i++){
     studentids[i]=0;
    
  }
  
}


/////////////////////////downloading and storing the fingerprint


void downloadandstore(int idnum){

 // String data="_";//////to store the fingerprint that download form the sql
 download(idnum);/////download the fingerprint form the sql database

 /////// Serial.println(data);
  getValue(data,'_');////extract fingerprint data


 ///////////////////////storing the data

  uint8_t packet2[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
  uint8_t packet5[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
  
  int p;
  int id = idnum;   // --> id = FlashMemory Place
  
  Serial.print("\n===> Write Packet");
         
  p = finger.uploadModel(packet2, d1, d2,packet5);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("\n Upload sucess");
      Serial.read();
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_BADPACKET:
      Serial.println("Bad packet");
      return p;      
    default:
  {
  Serial.println("\n==>[SUKSES] UploadModel = ");
      //return p;
  }
  }
  
  Serial.print("StoreModel + ID = ");Serial.print(id);
  p = finger.storeModel(id);                  // taruh di ID = 0 pada flash memory FP
  if (p == FINGERPRINT_OK) {
    Serial.println(" Stored!");
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
 //  int number=0;///count the data size 256
  String a= client.readStringUntil('_');
 
  while (client.available()) {
       data+=client.readStringUntil('_');
       data+="_";
     ////  Serial.println(data);////testing
   
   /*if(number==255){
  
      client.readStringUntil('"');
      break;
    
    }*/
  ///  number++;
    
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

//////////////////////////send attendance to sql
void httpReq(int stid)
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








