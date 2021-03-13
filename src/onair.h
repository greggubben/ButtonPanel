/*
 * Functions for Remotely Controlling the On Air sign
 * using the menu system.
 */
#pragma once

#ifndef _BUTTONPANEL_ONAIR_H
#define _BUTTONPANEL_ONAIR_H

#include <Arduino.h>
#include "Menu.h"
#include <vector>

using namespace std;

//
// Menu Definition
//

extern MenuButton onairButton;
extern vector<MenuButton*> onairButtonList;
extern MenuTop onairTopMenu;

//
// Functions
//

void onairSetup(Adafruit_GFX *tft);

//
// Callback Definitions
//

bool onairShortButtonPress ();
bool onairLongButtonPress ();

#endif