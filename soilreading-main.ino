/// Soil Reading Sketch
// Author: Miguel Buising <miguel.buising1@gmail.com>

/// Sensor Class
#include <Sensor.h>
// Valve Class
#include <Valve.h>
// JSON encode/decode util
#include <ArduinoJson.h>

// insert sensor data here
// Sensor class params: dryValue (calibration value), wetValue (calibration value), id, value
Sensor sensors[] = { 
  Sensor(888, 715, "B3CPS1", 0),
  Sensor(880, 740, "B2CPS1", 0),
  }; 

 // init the number of sensors here, the number of active sensors should be the same as the sensors array up top.
int activeSensors = 2;

// insert valve data here
// Valve class params: id, open (bool), pin 
Valve valves [] = {
  Valve("B1CP", false, 0),
};

// init the number of valves here.
int activeValves = 1;



// jsonbuffer
DynamicJsonDocument data(1024);

/// serial read vars
const byte numChars = 255;
char receivedChars[numChars];
boolean newData = false;

// interval vars, for non-blocking loops
// interval in milis default: 180000 == 30 mins.
const long interval = 600000;
// previous interval
unsigned long previous = 0;


void setup() {
  Serial.begin(9600); 
}

void loop() {
    // code to be executed all the time
    
    // interval time
    unsigned long currentMillis = millis(); 
    
    // functions
    recvWithStartEndMarkers();
    showNewData();

    // interval code
    
    // start interval
    if(currentMillis - previous >= interval){
      previous += interval;
      runReading();
    }
}

// TODO: Make the delay dettached from main delay DONE ✔
// reference: https://randomnerdtutorials.com/why-you-shouldnt-always-use-the-arduino-delay-function/
// TODO: Add JSON type
void runReading(){
  // loop through all the active sensors
  for( int i = 0; i < activeSensors; i++ ){
    data["id"] = sensors[i].id;
    // read the current sensor's analogRead
    int reading = analogRead(i);  

    // get the percentage of the given reading using calibration values
    sensors[i].value = map(reading, sensors[i].max, sensors[i].min, 0, 100);
    // check if the percentage is withing 0 to 100
    if(sensors[i].value >0 && sensors[i].value <= 100){
      // sometimes some values get past the if check above, do a second if check
      if(sensors[i].value > 100){
        data["value"] = "100";
       }
      else{
        data["value"] = sensors[i].value;
        }
    }else{
      data["value"] = "0";    
    }

     data["type"] = "reading";
     serializeJson(data, Serial);
     Serial.println();
  }
}

void singleReading(String id){
for(int i = 0; i<sizeof(sensors);i++){
    if(sensors[i].id == id){
      data["id"] = id;
      Serial.print(sensors[i].id);  
      int reading = analogRead(i);

      sensors[i].value = map(reading, sensors[i].max, sensors[i].min, 0, 100);

      if(sensors[i].value >0 && sensors[i].value <= 100){
      // sometimes some values get past the if check above, do a second if check
      if(sensors[i].value > 100){
        data["value"] = "100";
       }
      else{
        data["value"] = sensors[i].value;
        }
    }else{
      data["value"] = "0";    
    }
     data["type"] = "reading";
     serializeJson(data, Serial);
     Serial.println(); 
     data.clear();
    }
  }
}

void recvWithStartEndMarkers() {
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char rc;
 
    while (Serial.available() > 0 && newData == false) {
        rc = Serial.read();

        if (recvInProgress == true) {
            if (rc != endMarker) {
                receivedChars[ndx] = rc;
                ndx++;
                if (ndx >= numChars) {
                    ndx = numChars - 1;
                }
            }
            else {
                receivedChars[ndx] = '\0'; // terminate the string
                recvInProgress = false;
                ndx = 0;
                newData = true;
            }
        }

        else if (rc == startMarker) {
            recvInProgress = true;
        }
    }
}

// TODO: convert to json response and add type
void showNewData() {
    if (newData == true) {
        StaticJsonDocument<100> doc;  
        StaticJsonDocument<100> res;   
        DeserializationError error = deserializeJson(doc, receivedChars, numChars);
        
        if (error) {
          return;
        }
        
        const char* id = doc["id"]; // "b1cp"
        const char* type = doc["type"]; // "trigger"
        bool value = doc["value"]; // true

        if(id && type){
          toggleValve(id, value);
          res["type"] = "trigger";
          res["status"] = "success";
          res["value"] = doc["value"];
          serializeJson(res, Serial);
          Serial.println();
          res.clear();
          doc.clear();
        }
        newData = false;
    }
}

void toggleValve(String id, bool openValve){
 
  for(int i = 0;i < activeValves; i++){
      if(valves[i].id == id){
        int pin = valves[i].pin;
        pinMode(pin, OUTPUT);
        valves[i].open = openValve; 
        if(openValve == true){
          digitalWrite(pin, HIGH); 
        } else{
          digitalWrite(pin, LOW);
        } 
      }
    }
  }
