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
#include "Menu.h"
#include "headControl.h"


MenuButton frownButton = MenuButton("Frown", 0, 0);
MenuButton quizicalButton = MenuButton("What?", 1, 1);
MenuButton yesButton = MenuButton("Yes", 2, 2);
vector<MenuButton*> headButtonList = {&frownButton, &quizicalButton, &yesButton};
MenuPage headTopMenu = MenuPage("Head", 3, 3, &headButtonList, BUTTONPANEL_HEADCONTROL_COLOR);


//
// Head Control Variables
//
String headPath = "/face";        // Path to Service
String headName = "headControl";  // Must match devicename above
int faceSelected = 0;             // Face selected

// The following will be overwritten when the mDNS query is performed
String   headHost = headName + ".local";  // Default Host
String   headIP = headHost;               // Default to sign hostname
uint16_t headPort = 80;                   // Default Port



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
  http.begin(wifiClient, headIP, headPort, requestPath);
  int httpCode = http.sendRequest(type);
  if (httpCode == HTTP_CODE_OK) {
    // Received a good response
    String payload = http.getString();
    Serial.println(payload);
    DynamicJsonDocument requestDoc(1024);
    DeserializationError error = deserializeJson(requestDoc, payload);
    if (error) {
      Serial.println("Bad Request - Parsing JSON Body Failed");
    }
    else {
      // No issues parsing the response as a JSON payload

      // Get the current color of the onair sign 
      if (requestDoc.containsKey("color")) {
        String colorStr = requestDoc["color"];
        if (colorStr.charAt(0) == '#') {
          colorStr.setCharAt(0, '0');
        }
        char color_c[10] = "";
        colorStr.toCharArray(color_c, 8);
        uint32_t color24 = strtol(color_c, NULL, 16);
        //uint16_t color565 = colorRGB24toRGB565(color24);

        //Serial.printf("JSON Color = %s\n", color_c);
        //Serial.printf("  Color 24 = %x\n", color24);
        //Serial.printf(" Color RBG = %x\n", color565);

        // Set color of Button to match color of sign
        //onairButton.setColor(color565);
      }

      // Get the current on/off state of the onair sign
      if (requestDoc.containsKey("lightOn")) {
        //lightOn = requestDoc["lightOn"];
        //if (lightOn) {
          //turnButtonOn();
        //}
        //else {
          //turnButtonOff();
        //}
      }
    }

  }
  else {
    Serial.printf("[sendSignCommand] %s failed, code: %d; error: %s\n", type, httpCode, http.errorToString(httpCode).c_str());
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


