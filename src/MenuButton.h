/*
 * @file Menu.h
 * 
 * A Menu is made up of 0 to many MenuPages.
 *   However 0 MenuPages objects is not very useful.
 * The MenuPage contains 0 to many MenuButtons.
 * 
 * Both MenuPage and MenuButton  inherit from MenuItem
 * which contains many common data and methods used to display a
 * button on the screen.
 */
#pragma once

#ifndef __MENUBUTTON_H
#define __MENUBUTTON_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>
#include <vector>
#include <tuple>
#include "MenuUtils.h"

using namespace std;

// MenuButton Defaults
#define DEFAULT_PADDING_X 2                       // Default horizontal padding in pixels
#define DEFAULT_PADDING_Y 2                       // Default vertical padding in pixels
#define DEFAULT_BUTTON_CORNER 8                   // Default roundness of corners in pixels
#define DEFAULT_BUTTON_COLOR ILI9341_CYAN         // Default color of the button background
#define DEFAULT_BUTTON_TEXT_COLOR ILI9341_WHITE   // Default color of the button text
#define DEFAULT_BUTTON_TEXT_SIZE 2                // Default size of the button text

// @FIXME move to variable
#define DEFAULT_BUTTON_BACKGROUND_COLOR ILI9341_BLACK    // Default background


/*********************
 * MenuButton Class
 *********************/

//
// Class for the buttons displayed in the pannel
//
class MenuButton: public MenuItem {
  public:
    MenuButton(const char* label, int16_t x, int16_t y, uint16_t buttonColor = DEFAULT_BUTTON_COLOR, ButtonPressCallback onShortPress = nullptr, ButtonPressCallback onLongPress = nullptr);  // Constructor
    void draw(Adafruit_GFX *tft, int16_t buttonX, int16_t buttonY, int16_t buttonWidth, int16_t buttonHeight);
    bool handleTouch();

    /*!
     * @brief return the button number position X
     */
    int16_t getPositionX() {return positionX;};
    /*!
     * @brief return the button number position Y
     */
    int16_t getPositionY() {return positionY;};
    /*!
     * @brief return the button number position
     * @param int16_t* pointer to return the X button position number
     * @param int16_t* pointer to return the Y button position number
     */
    void getPosition(int16_t *posX, int16_t *posY) {*posX = positionX; *posY = positionY;};

  private:
    int16_t positionX;  // Button Number Position X - not pixels
    int16_t positionY;  // Button Number Position Y - not pixels


  private:
};


/*********************
 * Non Class Functions
 *********************/

void drawButtonOutline(Adafruit_GFX *tft, int16_t buttonX, int16_t buttonY, int16_t buttonWidth, int16_t buttonHeight, uint16_t buttonColor, bool buttonActive);

#endif