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

#ifndef __MENUPAGE_H
#define __MENUPAGE_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>
#include <vector>
#include <tuple>
#include "MenuButton.h"


// MenuPage Defaults
#define DEFAULT_PAGETOP_BUTTONS 3                 // Number of buttons to show across top
#define DEFAULT_PAGETOP_HEIGHT 80                 // Height of button section on top in pixels
#define DEFAULT_PAGETOP_COLOR ILI9341_CYAN        // Default Color of menu page
#define DEFAULT_PAGETOP_TEXT_COLOR ILI9341_WHITE  // Default Text color for menu page button
#define DEFAULT_PAGETOP_TEXT_SIZE 2               // Default Text Size of menu page button
#define DEFAULT_PAGETOP_NEXT_BUTTON_SIZE 3        // Default width of next button indicator in pixels


/*********************
 * Generic definitions for passing functions as arguments
 *********************/

/*!
 * @brief Function to draw a unique panel page.
 * 
 * @param Adafruit_GFX* pointer to touch screen
 * @param int16_t screen upper left corner X position in pixels
 * @param int16_t screen upper left corner Y position in pixels
 * @param int16_t screen width in pixels
 * @param int16_t screen height in pixels
 */
typedef void (*DrawPanelFunction)(Adafruit_GFX*, int16_t, int16_t, int16_t, int16_t);


/*!
 * @brief Function to perform setup activities for a menu page.
 * 
 * @param Adafruit_GFX* pointer to touch screen to display setup messages
 */
typedef void (*MenuPageSetup)(Adafruit_GFX*);



/*********************
 * MenuPage Class
 *********************/

//
// Class for the Buttons across the top
// This changes the buttons displayed in the panel below
//
class MenuPage: public MenuItem {
  public:
    MenuPage(const char* label, int16_t buttonsAcross, int16_t buttonsDown, uint16_t menuColor = DEFAULT_PAGETOP_COLOR, ButtonPressCallback onShortPress = nullptr, ButtonPressCallback onLongPress = nullptr);
    MenuPage(const char* label, int16_t buttonsAcross, int16_t buttonsDown, vector<MenuButton*> *menuButtons, uint16_t menuColor = DEFAULT_PAGETOP_COLOR, ButtonPressCallback onShortPress = nullptr, ButtonPressCallback onLongPress = nullptr);

    void draw(Adafruit_GFX *tft, int16_t buttonX, int16_t buttonY, int16_t buttonWidth, int16_t buttonHeight);
    void drawPanelButtons(Adafruit_GFX *tft, int16_t panelX, int16_t panelY, int16_t panelWidth, int16_t panelHeight);
    void setDrawPanel(DrawPanelFunction drawPanelFunc) {drawPanel = drawPanelFunc;};

    bool setActiveMenuButton(MenuButton *activeButton);
    static bool setActiveMenuButton(vector<MenuButton*> *mButtons, MenuButton *activeButton);

    MenuButton* findTouchedButton(int16_t pressX, int16_t pressY, int16_t panelX, int16_t panelY, int16_t panelWidth, int16_t panelHeight);


  private:
    void init(const char* label, int16_t buttonsAcross, int16_t buttonsDown, vector<MenuButton*> *menuButtons, uint16_t menuColor, ButtonPressCallback onShortPress, ButtonPressCallback onLongPress);

    int16_t buttonsX;         // Number of buttons across
    int16_t buttonsY;         // Number of buttons down
    vector<MenuButton*> *buttons = nullptr;   // List of all the button on this page
    DrawPanelFunction drawPanel = nullptr;    // function to draw unique items on menu page panel
};

#endif