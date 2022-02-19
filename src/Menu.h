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

#ifndef __MENU_H
#define __MENU_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>
#include <vector>
#include <tuple>
#include "MenuPage.h"
#include "TouchHandler.h"

using namespace std;

// Menu Defaults
#define DEFAULT_BACKGROUND_COLOR ILI9341_BLACK    // Default background
#define DEFAULT_BAR_HEIGHT DEFAULT_BUTTON_CORNER  // Default height of menu page separator bar


/*********************
 * Menu Class
 *********************/

//
// Main class holding all menus to be displayed
//
class Menu {
  public:
    Menu(Adafruit_GFX *tft);
    Menu(Adafruit_GFX *tft, vector<MenuPage*> *tMenus = nullptr);
    Menu(Adafruit_GFX *tft, vector<MenuPage*> *tMenus, uint16_t bgColor);
    

    void setTopButtons(int16_t buttons);
    void setTopHeight(int16_t height);

    bool setup();
    void draw();

    void eventHandler(Event *event);

    bool setActiveButton(MenuButton *activeButton);
    bool setActiveTopMenu(MenuPage *activeMenu);
    static bool setActiveTopMenu(vector<MenuPage*> *tMenus, MenuPage *activeMenu);

  private:
    MenuPage* findTouchedTopButton(int16_t pressX, int16_t pressY);
    MenuButton* findTouchedButton(int16_t pressX, int16_t pressY);

    void init(Adafruit_GFX *tft, vector<MenuPage*> *tMenus = nullptr, uint16_t bgColor = DEFAULT_BACKGROUND_COLOR);
    void calculateTopButtonDimensions();

    Adafruit_GFX *_tft;

    uint16_t backgroundColor;
    int16_t screenWidth;
    int16_t screenHeight;
    int16_t topButtons = DEFAULT_PAGETOP_BUTTONS;
    int16_t topHeight = DEFAULT_PAGETOP_HEIGHT;
    int16_t topButtonWidth = 0;
    int16_t topButtonHeight = 0;
    int16_t panelX = 0;
    int16_t panelY = 0;
    int16_t panelWidth = 0;
    int16_t panelHeight = 0;
    int startPage = 0;
    bool clearScreenBeforeDraw = true;

    vector<MenuPage*> *topMenus = nullptr;

    MenuPage *pressedPage = nullptr;
    MenuButton *pressedButton = nullptr;

};




#endif