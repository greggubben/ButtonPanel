/*
 * Functions for Remotely Controlling the Robot Head
 * using the menu system.
 */
#include <Arduino.h>
#include <ESP8266mDNS.h>        // For running OTA and Web Server
#include <ArduinoJson.h>        // For REST based Web Services
#include <ESP8266HTTPClient.h>  // For REST based calls

#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
//#include "Menu.h"
#include "headControl.h"

bool headControlShortButtonPress0 () {
  headControlShortButtonPress(0);
  return true;
}
bool headControlShortButtonPress1 () {
  headControlShortButtonPress(1);
  return true;
}
bool headControlShortButtonPress2 () {
  headControlShortButtonPress(2);
  return true;
}
bool headControlShortButtonPress3 () {
  headControlShortButtonPress(3);
  return true;
}
bool headControlShortButtonPress4 () {
  headControlShortButtonPress(4);
  return true;
}
bool headControlShortButtonPress5 () {
  headControlShortButtonPress(5);
  return true;
}

MenuButton face0Button = MenuButton("Face0", 0, 0, DEFAULT_BUTTON_COLOR, &headControlShortButtonPress0);
MenuButton face1Button = MenuButton("Face1", 1, 0, DEFAULT_BUTTON_COLOR, &headControlShortButtonPress1);
MenuButton face2Button = MenuButton("Face2", 2, 0, DEFAULT_BUTTON_COLOR, &headControlShortButtonPress2);
MenuButton face3Button = MenuButton("Face3", 0, 1, DEFAULT_BUTTON_COLOR, &headControlShortButtonPress3);
MenuButton face4Button = MenuButton("Face4", 1, 1, DEFAULT_BUTTON_COLOR, &headControlShortButtonPress4);
MenuButton face5Button = MenuButton("Face5", 2, 1, DEFAULT_BUTTON_COLOR, &headControlShortButtonPress5);
vector<MenuButton*> headButtonList = {&face0Button, &face1Button, &face2Button, &face3Button, &face4Button, &face5Button};
MenuPage headTopMenu = MenuPage("Head", 3, 2, &headButtonList, BUTTONPANEL_HEADCONTROL_COLOR, &headControlShortPagePress);


//
// Head Control Variables
//
String headPath = "/face";        // Path to Service
String headName = "headcontrol";  // Must match devicename above
int faceSelected = 0;             // Face selected

// The following will be overwritten when the mDNS query is performed
String   headHost = headName + ".local";  // Default Host
String   headIP = headHost;               // Default to sign hostname
uint16_t headPort = 80;                   // Default Port

/*
 * Add a face to the page
 */
void addFace (unsigned int faceNum, String faceName, bool faceSelected) {
  MenuButton *faceButton = headButtonList.at(faceNum);
  faceButton->setName(faceName);
  if (faceSelected) {
    faceButton->setActive();
    faceButton->setColor(HEADCONTROL_SELECTED_COLOR);
    faceButton->setTextColor(HEADCONTROL_SELECTED_TEXTCOLOR);
  }
  else {
    faceButton->setInactive();
    faceButton->setColor(DEFAULT_BUTTON_COLOR);
    faceButton->setTextColor(DEFAULT_BUTTON_TEXT_COLOR);
  }
}


//
// Find the Sign's IP Address
//
void findHeadIP(Adafruit_GFX *tft) {
  tft->println("Sending mDNS query");
  Serial.println("Sending mDNS query");
  int n = MDNS.queryService(headName, "tcp"); // Send out query for esp tcp services
  Serial.println("mDNS query done");
  if (n == 0) {
    tft->println("no services found");
    Serial.println("no services found");
  } else {
    // Using the last one if multiple are found
    Serial.print(n);
    Serial.println(" service(s) found");
    for (int i = 0; i < n; ++i) {
      headHost = MDNS.hostname(i);
      headIP = MDNS.IP(i).toString();
      headPort = MDNS.port(i);
      // Print details for each service found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(headHost);
      Serial.print(" (");
      Serial.print(headIP);
      Serial.print(":");
      Serial.print(headPort);
      Serial.println(")");
    }
  }
  Serial.print("Using (");
  Serial.print(headIP);
  Serial.print(": ");
  Serial.print(headPort);
  Serial.print(") for ");
  Serial.println(headHost);

  tft->print("Using ");
  tft->print(headIP);
  tft->print(":");
  tft->print(headPort);
  tft->print(" for ");
  tft->println(headHost);
}


//
// Send Commands to Head
//
void sendHeadCommand (const char* type, const String& requestPath) {
  WiFiClient wifiClient;
  HTTPClient http;
  //String serverPath = "http://" + headHost +":80" + requestPath;
  //Serial.println(serverPath);
  //http.begin(wifiClient, serverPath.c_str());
  http.begin(wifiClient, headIP, headPort, requestPath);
  int httpCode = http.sendRequest(type);
  if (httpCode == HTTP_CODE_OK) {
    // Received a good response
    String payload = http.getString();
    Serial.println(payload);
    DynamicJsonDocument requestDoc(2048);
    DeserializationError error = deserializeJson(requestDoc, payload);
    if (error) {
      Serial.println("Bad Request - Parsing JSON Body Failed");
    }
    else {
      // No issues parsing the response as a JSON payload

      // Get the current color of the onair sign 
      if (requestDoc.containsKey("faces")) {
        JsonArray faces = requestDoc["faces"];
        for(JsonVariant face : faces) {
          unsigned int faceNum = face["faceNum"].as<unsigned int>();
          String faceName = face["name"].as<String>();
          bool faceSelected = face["selected"].as<bool>();
          Serial.print("Face Num: ");
          Serial.print(faceNum);
          Serial.print(" Name: ");
          Serial.print(faceName);
          Serial.print(" Selected: ");
          Serial.print(faceSelected);
          Serial.println();
          addFace(faceNum, faceName, faceSelected);
        }
      }

    }

  }
  else {
    Serial.printf("[sendHeadCommand] %s failed, code: %d; error: %s\n", type, httpCode, http.errorToString(httpCode).c_str());
    String payload = http.getString();
    Serial.println(payload);
  }
  
}


//
// Get Head Status
//
void getHeadStatus () {
  sendHeadCommand("GET", headPath);
}


//
// Tell Head to put on this face
//
void selectFace (int face) {
  String faceSelectStr = headPath + "?face=" + String(face);
  sendHeadCommand("PUT", faceSelectStr);
}


/*****************************
 * Head Control Menu functions and callbacks
 *****************************/

//
// Perform Head Control Button Panel first time set up
void headControlSetup (Adafruit_GFX *tft) {
      // Set up On Air
  findHeadIP(tft);
  
  // Initialize to the current status of the Sign
  getHeadStatus();
  tft->println("Got Head Status");

}


//
// Handle a short press callback.
// Get Sign's latest status to update page
//
bool headControlShortPagePress () {
  Serial.println("Head Control Short Page Press");
  getHeadStatus();
  return true;
}


void headControlShortButtonPress (int faceNum) {
  String faceSelectPath = headPath + "?faceNum=" + String(faceNum);
  sendHeadCommand("PUT", faceSelectPath);
}
