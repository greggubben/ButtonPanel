/*
 * Functions for Displaying the Status of the button panel
 * using the menu system.
 */
#pragma once

#ifndef _BUTTONPANEL_STATUS_H
#define _BUTTONPANEL_STATUS_H

#include <Arduino.h>
#include "Menu.h"
#include <vector>

using namespace std;

#define BUTTONPANEL_STATUS_PADDING_TOP 2
#define BUTTONPANEL_STATUS_COLOR ILI9341_YELLOW

//
// Menu Definition
//

extern MenuPage statusTopMenu;


//
// Functions
//

void showStatus (Adafruit_GFX *tft, int16_t panelX, int16_t panelY, int16_t panelWidth, int16_t panelHeight);

void statusSetup(Adafruit_GFX *tft, NTPClient *tc);

//
// Callback Definitions
//

bool statusResetShortPress ();
bool statusConfigShortPress ();
bool statusConfigLongPress ();

#endif