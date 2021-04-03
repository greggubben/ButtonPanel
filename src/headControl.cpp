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
