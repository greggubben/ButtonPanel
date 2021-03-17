/*
 * Functions for Remotely Controlling the On Air sign
 * using the menu system.
 */

#include <Arduino.h>
#include <ESP8266mDNS.h>        // For running OTA and Web Server
#include <ArduinoJson.h>        // For REST based Web Services
#include <ESP8266HTTPClient.h>  // For REST based calls

#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include "Menu.h"
#include "onair.h"

//
// Menu Definition
//
MenuButton onairButton = MenuButton("On Air", 0, 1, ILI9341_RED, &onairShortButtonPress, &onairLongButtonPress);
vector<MenuButton*> onairButtonList = {&onairButton};
MenuPage onairTopMenu = MenuPage("OnAir", 1, 3, &onairButtonList, ILI9341_RED, &onairShortPagePress);


//
// On Air Sign Variables
//
String signPath = "/light";               // Path to Service
String signNextPath = signPath + "?next"; // Path to Service
String signName = "OnAir";                // Must match devicename above
boolean lightOn = false;                  // State of the sign's LEDs

// The following will be overwritten when the mDNS query is performed
String   signHost = signName + ".local";  // Default Host
String   signIP = signHost;               // Default to sign hostname
uint16_t signPort = 80;                   // Default Port


/*****************************
 * On Air utilities
 *****************************/

//
// Convert an 8-bit individual R G B color to an equivalent 16-bit RGB565 value
//
uint16_t colorRGBtoRGB565(uint8_t r, uint8_t g, uint8_t b)
{
  return ((r / 8) << 11) | ((g / 4) << 5) | (b / 8);
}

//
// Convert a 24-bit RGB color to an equivalent 16-bit RGB565 value
//
uint16_t colorRGB24toRGB565(uint32_t rgb) {
  uint8_t r = (rgb & 0xFF0000) >> 16;
  uint8_t g = (rgb & 0x00FF00) >> 8;
  uint8_t b = (rgb & 0x00FF);

  return colorRGBtoRGB565(r, g, b);
}

//
// Convert a 16-bit RGB565 color to a standard 24-bit RGB Color
//
uint32_t colorRGB565toRGB24(uint16_t color)
{
  uint32_t bits = (uint32_t)color;
  uint32_t blue = bits & 0x001F;     // 5 bits blue
  uint32_t green = bits & 0x07E0;    // 6 bits green
  uint32_t red = bits & 0xF800;      // 5 bits red

  // Return shifted bits with alpha set to 0xFF
  //return (red << 8) | (green << 5) | (blue << 3) | 0xFF000000;
  return (red << 8) | (green << 5) | (blue << 3);
}


/*****************************
 * On Air Sign functions
 *****************************/

//
// Turn the Button on
//
void turnButtonOn() {
  lightOn = true;
  onairButton.setActive();
}


//
// Turn the Button off
//
void turnButtonOff() {
  lightOn = false;
  onairButton.setInactive();
}


//
// Send Commands to Sign
//
void sendSignCommand (const char* type, const String& requestPath) {
  WiFiClient wifiClient;
  HTTPClient http;
  http.begin(wifiClient, signIP, signPort, requestPath);
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
        uint16_t color565 = colorRGB24toRGB565(color24);

        //Serial.printf("JSON Color = %s\n", color_c);
        //Serial.printf("  Color 24 = %x\n", color24);
        //Serial.printf(" Color RBG = %x\n", color565);

        // Set color of Button to match color of sign
        onairButton.setColor(color565);
      }

      // Get the current on/off state of the onair sign
      if (requestDoc.containsKey("lightOn")) {
        lightOn = requestDoc["lightOn"];
        if (lightOn) {
          turnButtonOn();
        }
        else {
          turnButtonOff();
        }
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
// Get Sign Status
//
void getSignStatus () {
  sendSignCommand("GET", signPath);
}


//
// Tell Sign to turn light on or advance to next color
//
void turnSignOn () {
  if (lightOn) {
    sendSignCommand("PUT", signNextPath);
  }
  else {
    sendSignCommand("PUT", signPath);
  }
}


//
// Tell Sign to turn light off
//
void turnSignOff () {
  sendSignCommand("DELETE", signPath);
}


//
// Find the Sign's IP Address
//
void findSignIP(Adafruit_GFX *tft) {
  tft->println("Sending mDNS query");
  Serial.println("Sending mDNS query");
  int n = MDNS.queryService(signName, "tcp"); // Send out query for esp tcp services
  Serial.println("mDNS query done");
  if (n == 0) {
    tft->println("no services found");
    Serial.println("no services found");
  } else {
    // Using the last one if multiple are found
    Serial.print(n);
    Serial.println(" service(s) found");
    for (int i = 0; i < n; ++i) {
      signHost = MDNS.hostname(i);
      signIP = MDNS.IP(i).toString();
      signPort = MDNS.port(i);
      // Print details for each service found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(signHost);
      Serial.print(" (");
      Serial.print(signIP);
      Serial.print(":");
      Serial.print(signPort);
      Serial.println(")");
    }
  }
  Serial.print("Using (");
  Serial.print(signIP);
  Serial.print(": ");
  Serial.print(signPort);
  Serial.print(") for ");
  Serial.println(signHost);

  tft->print("Using ");
  tft->print(signIP);
  tft->print(":");
  tft->print(signPort);
  tft->print(" for ");
  tft->println(signHost);
}


/*****************************
 * On Air Menu functions and callbacks
 *****************************/

//
// Perform OnAir Button Panel first time set up
void onairSetup (Adafruit_GFX *tft) {
      // Set up On Air
  findSignIP(tft);
  
  // Initialize to the current status of the Sign
  getSignStatus();
  tft->println("Got Sign Status");

}


//
// Handle a short press callback.
// Turn the sign on or move to next color
// State of button is changed based on response from sign
//
bool onairShortButtonPress () {
  Serial.println("On Air Short Button Press");
  turnSignOn();
  return true;
}

//
// Handle a long press callback.
// Turn sign off
// State of button is changed based on response from sign
//
bool onairLongButtonPress () {
  Serial.println("On Air Long Button Press");
  turnSignOff();
  return true;
}

//
// Handle a short press callback.
// Get Sign's latest status to update page
//
bool onairShortPagePress () {
  Serial.println("On Air Short Page Press");
  getSignStatus();
  return true;
}
