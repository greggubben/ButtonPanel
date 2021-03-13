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
#include <vector>

using namespace std;

//
// Menu Definition
//

extern MenuButton frownButton;
extern MenuButton quizicalButton;
extern MenuButton yesButton;
extern vector<MenuButton*> headButtonList;
extern MenuTop headTopMenu;

