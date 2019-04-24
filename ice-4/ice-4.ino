// ********************************************* \\
//                                               \\ 
//                      MQTT                     \\
//              PUBLISHING & SUBSCRIBING         \\
//                                               \\
// ********************************************* \\

// Program for creating a topic on an MQTT server and publishing potentiometer 
// data to it

//Requisite Libraries 
#include <ESP8266WiFi.h>    
#include "Wire.h"           
#include <PubSubClient.h>   
#include <ArduinoJson.h>    

// wifi and password 
#define wifi_ssid ""  
#define wifi_password "" 

//////////
//So to clarify, we are connecting to and MQTT server
//that has a login and password authentication
//////////

#define mqtt_server "mediatedspaces.net"  //this is its address, unique to the server
#define mqtt_user "hcdeiot"               //this is its server login, unique to the server
#define mqtt_password "esp8266"           //this is it server password, unique to the server

// Potentiometer connected to analog pin 0
#define POTPIN A0

//////////
//We also need to publish and subscribe to topics, for this sketch are going
//to adopt a topic/subtopic addressing scheme: topic/subtopic
//////////

WiFiClient espClient;             
PubSubClient mqtt(espClient);     

// Variable to store potentiometer values 
int potValue = 0;

//////////
//We need a 'truly' unique client ID for our esp8266, all client names on the server must be unique.
//Every device, app, other MQTT server, etc that connects to an MQTT server must have a unique client ID.
//This is the only way the server can keep every device separate and deal with them as individual devices/apps.
//The client ID is unique to the device.
//////////

char mac[6]; //A MAC address is a 'truly' unique ID for each device, lets use that as our 'truly' unique user ID!!!

//////////
//In our loop(), we are going to create a c-string that will be our message to the MQTT server, we will
//be generous and give ourselves 200 characters in our array, if we need more, just change this number
//////////

char message[201]; //201, as last character in the array is the NULL character, denoting the end of the array

// Technique for implementing timing that makea a schedule and keeps an eye on the clock.  
// Instead of a world-stopping delay, just check the clock regularly so and accordingly. 
unsigned long currentMillis, timerOne, timerTwo, timerThree; //we are using these to hold the values of our timers

/////SETUP/////
void setup() {
  Serial.begin(115200);
  setup_wifi();
  mqtt.setServer(mqtt_server, 1883);
  mqtt.setCallback(callback); //register the callback function
  timerOne = timerTwo = timerThree = millis();
}

/////SETUP_WIFI/////
void setup_wifi() {
  delay(10);
  // Start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);
  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");  //get the unique MAC address to use as MQTT client ID, a 'truly' unique ID.
  Serial.println(WiFi.macAddress());  //.macAddress returns a byte array 6 bytes representing the MAC address
}                                     //5C:CF:7F:F0:B0:C1 for example

/////CONNECT/RECONNECT/////Monitor the connection to MQTT server, if down, reconnect
void reconnect() {
  // Loop until we're reconnected
  while (!mqtt.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqtt.connect(mac, mqtt_user, mqtt_password)) { //<<---using MAC as client ID, always unique!!!
      Serial.println("connected");
      mqtt.subscribe("Alaa/+"); //we are subscribing to 'theTopic' and all subtopics below that topic
    } else {                        //please change 'theTopic' to reflect your topic you are subscribing to
      Serial.print("failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

/////LOOP/////
void loop() {
  if (!mqtt.connected()) {
    reconnect();
  }

  mqtt.loop(); //this keeps the mqtt connection 'active'

  // Read data from sensor: potentiometer values every one second
  if (millis() - timerOne > 1000) {
    // Grab the current value of the potentiometer
    potValue = analogRead(POTPIN);

    sprintf(message, "{\"Pot_value\":\"%d%\"}", potValue); // %d is used for an int
    // Print the value to the serial monitor
    Serial.println(potValue);
    //publish message  
    mqtt.publish("Alaa/test", message);
    timerOne = millis();
  }
}//end Loop


/////CALLBACK/////
//The callback is where we attach a listener to the incoming messages from the server.
//By subscribing to a specific channel or topic, we can listen to those topics we wish to hear.
//We place the callback in a separate tab so we can edit it easier . . . (will not appear in separate
//tab on github!)
/////

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println();
  Serial.print("Message arrived [");
  Serial.print(topic); //'topic' refers to the incoming topic name, the 1st argument of the callback function
  Serial.println("] ");

  DynamicJsonBuffer  jsonBuffer; //blah blah blah a DJB
  JsonObject& root = jsonBuffer.parseObject(payload); //parse it!

  if (!root.success()) { //well?
    Serial.println("parseObject() failed, are you sure this message is JSON formatted.");
    return;
  }

  /////
  //We can use strcmp() -- string compare -- to check the incoming topic in case we need to do something
  //special based upon the incoming topic, like move a servo or turn on a light . . .
  //strcmp(firstString, secondString) == 0 <-- '0' means NO differences, they are ==
  /////

  if (strcmp(topic, "theTopic/LBIL") == 0) {
    Serial.println("A message from Batman . . .");
  }

  else if (strcmp(topic, "theTopic/tempHum") == 0) {
    Serial.println("Some weather info has arrived . . .");
  }

  else if (strcmp(topic, "theTopic/switch") == 0) {
    Serial.println("The switch state is being reported . . .");
  }

  root.printTo(Serial); //print out the parsed message
  Serial.println(); //give us some space on the serial monitor read out
}
