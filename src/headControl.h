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

#define BUTTONPANEL_HEADCONTROL_COLOR ILI9341_GREEN
#define HEADCONTROL_SELECTED_COLOR ILI9341_GREEN
#define HEADCONTROL_SELECTED_TEXTCOLOR ILI9341_BLACK


//
// Menu Definition
//

extern MenuButton frownButton;
extern MenuButton quizicalButton;
extern MenuButton yesButton;
extern vector<MenuButton*> headButtonList;
extern MenuPage headTopMenu;

//
// Functions
//

void headControlSetup(Adafruit_GFX *tft);

bool headControlShortPagePress();

void headControlShortButtonPress (int faceNum);