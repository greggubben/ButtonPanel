/*
 * Menu
 * 
 * A Menu is made up of 0 to many MenuTop objects.
 *   However 0 MenuTop objects is not very useful.
 * The MenuTop objects contains 0 to many MenuButton objectes.
 * 
 * Both MenuTop and MenuButton objects inherite from MenuItem
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

using namespace std;

// MenuButton Defaults

#define DEFAULT_PADDING_X 2
#define DEFAULT_PADDING_Y 2
#define DEFAULT_BUTTON_CORNER 8
#define DEFAULT_BUTTON_COLOR ILI9341_CYAN
#define DEFAULT_BUTTON_TEXT_COLOR ILI9341_WHITE
#define DEFAULT_BUTTON_TEXT_SIZE 2


// MenuPage Defaults

#define DEFAULT_PAGETOP_BUTTONS 3
#define DEFAULT_PAGETOP_HEIGHT 80
#define DEFAULT_PAGETOP_COLOR ILI9341_CYAN
#define DEFAULT_PAGETOP_TEXT_COLOR ILI9341_WHITE
#define DEFAULT_PAGETOP_TEXT_SIZE 2
#define DEFAULT_PAGETOP_NEXT_BUTTON_SIZE 3


// Menu Defaults

#define DEFAULT_BACKGROUND_COLOR ILI9341_BLACK
#define DEFAULT_BAR_HEIGHT DEFAULT_BUTTON_CORNER


// Define minimum delays for when screen is touched

#define DEBOUNCE_DELAY 50
#define SHORTPRESS_DELAY 60
#define LONGPRESS_DELAY 400
#define SWIPE_MIN_PIXELS 40


// Contains the Events a touch could generate
enum TouchEvent {
  EVENT_NONE = 0,         // No Events occurred
  EVENT_SHORT = 1,        // Short Press Event has occurred
  EVENT_LONG = 2,         // Long Press Event has occurred
  EVENT_SWIPE_RIGHT = 3,  // Finger traveling to Right
  EVENT_SWIPE_LEFT =4     // Finger travelling to Left
};

// Text representation of the Touch Events defined by the TouchEvent enum
static const char* TouchEventStrings[] = {"None", "Short", "Long", "Swipe Right", "Swipe Left"};


// Generic definition for button callbacks
typedef bool (*buttonPressCallback)();
typedef void (*drawPanelFunction)(Adafruit_GFX*, int16_t, int16_t, int16_t, int16_t);
typedef void (*calibrateTouchCallback)(int16_t*, int16_t*);


/*********************
 * MenuItem Class
 *********************/

//
// Generic class inherited by all Menu classes
// Used to hold common data and functions
//
class MenuItem {
  public:

    void setActive() { active = true; };
    void setInactive() { active = false; };
    bool isActive() { return active; };

    void setColor(uint16_t c) {color = c; };
    uint16_t getColor() {return color; };

    String getName() {return name; };
    void setName(String n) { name = n;};

    void setTextSize(int16_t s) { textSize = s;};
    void setTextColor(int16_t c) { textColor = c;};

    void setShortPressCallback(buttonPressCallback onPress) { shortPressCallback = onPress;};
    bool hasShortPressCallback() { return (shortPressCallback != nullptr);};
    bool callbackShortPress();

    void setLongPressCallback(buttonPressCallback onPress) { longPressCallback = onPress;};
    bool hasLongPressCallback() { return (longPressCallback != nullptr);};
    bool callbackLongPress();

  protected:
    MenuItem() {};
    String name;              // Name to put in the Button
    bool active = false;
    uint16_t color;           // Color to use for this menu
    int16_t textSize;         // Size of Font for menu
    uint16_t textColor;       // Text Color for menu
    buttonPressCallback shortPressCallback = nullptr;  // Function to call if the button was pressed
    buttonPressCallback longPressCallback = nullptr;  // Function to call if the button was pressed


};


/*********************
 * MenuButton Class
 *********************/

//
// Class for the buttons displayed in the pannel
//
class MenuButton: public MenuItem {
  public:
    MenuButton(const char* label, int16_t x, int16_t y, uint16_t buttonColor = DEFAULT_BUTTON_COLOR, buttonPressCallback onShortPress = nullptr, buttonPressCallback onLongPress = nullptr);
    void draw(Adafruit_GFX *tft, int16_t buttonX, int16_t buttonY, int16_t buttonWidth, int16_t buttonHeight);
    bool handleTouch();

    int16_t getPositionX() {return positionX;};
    int16_t getPositionY() {return positionY;};
    void getPosition(int16_t *posX, int16_t *posY) {*posX = positionX; *posY = positionY;};

  private:
    int16_t positionX;
    int16_t positionY;


  private:
};


/*********************
 * MenuPage Class
 *********************/

//
// Class for the Buttons across the top
// This changes the buttons displayed in the panel below
//
class MenuPage: public MenuItem {
  public:
    MenuPage(const char* label, int16_t buttonsAcross, int16_t buttonsDown, uint16_t menuColor = DEFAULT_PAGETOP_COLOR, buttonPressCallback onShortPress = nullptr, buttonPressCallback onLongPress = nullptr);
    MenuPage(const char* label, int16_t buttonsAcross, int16_t buttonsDown, vector<MenuButton*> *menuButtons, uint16_t menuColor = DEFAULT_PAGETOP_COLOR, buttonPressCallback onShortPress = nullptr, buttonPressCallback onLongPress = nullptr);

    void draw(Adafruit_GFX *tft, int16_t buttonX, int16_t buttonY, int16_t buttonWidth, int16_t buttonHeight);
    void drawPanelButtons(Adafruit_GFX *tft, int16_t panelX, int16_t panelY, int16_t panelWidth, int16_t panelHeight);
    void setDrawPanel(drawPanelFunction drawPanelFunc) {drawPanel = drawPanelFunc;};

    bool setActiveMenuButton(MenuButton *activeButton);
    static bool setActiveMenuButton(vector<MenuButton*> *mButtons, MenuButton *activeButton);

    MenuButton* findTouchedButton(int16_t pressX, int16_t pressY, int16_t panelX, int16_t panelY, int16_t panelWidth, int16_t panelHeight);


  private:
    void init(const char* label, int16_t buttonsAcross, int16_t buttonsDown, vector<MenuButton*> *menuButtons, uint16_t menuColor, buttonPressCallback onShortPress, buttonPressCallback onLongPress);

    int16_t buttonsX;         // Number of buttons across
    int16_t buttonsY;         // Number of buttons down
    vector<MenuButton*> *buttons = nullptr;
    drawPanelFunction drawPanel = nullptr;
};


/*********************
 * Menu Class
 *********************/

//
// Main class holding all menus to be displayed
//
class Menu {
  public:
    Menu(Adafruit_GFX *tft, XPT2046_Touchscreen *ts);
    Menu(Adafruit_GFX *tft, XPT2046_Touchscreen *ts, vector<MenuPage*> *tMenus = nullptr);
    Menu(Adafruit_GFX *tft, XPT2046_Touchscreen *ts, vector<MenuPage*> *tMenus, uint16_t bgColor);
    
    bool handle(calibrateTouchCallback _calibrateTouch = nullptr);

    void setTopButtons(int16_t buttons);
    void setTopHeight(int16_t height);

    bool setup();
    void draw();


    bool setActiveButton(MenuButton *activeButton);
    bool setActiveTopMenu(MenuPage *activeMenu);
    static bool setActiveTopMenu(vector<MenuPage*> *tMenus, MenuPage *activeMenu);

  private:
    MenuPage* findTouchedTopButton(int16_t pressX, int16_t pressY);
    MenuButton* findTouchedButton(int16_t pressX, int16_t pressY);

    void init(Adafruit_GFX *tft, XPT2046_Touchscreen *ts, vector<MenuPage*> *tMenus = nullptr, uint16_t bgColor = DEFAULT_BACKGROUND_COLOR);
    void calculateTopButtonDimensions();

    Adafruit_GFX *_tft;
    XPT2046_Touchscreen *_ts;

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

    // Manage state of Menu Interactions
    bool wasTouched = false;      // Prevous loop touch state. Used with isTouched 
    bool debounceTouched = false; // Post Debounce Delay last touch state. Used with isTouched 
    TouchEvent touchEvent = EVENT_NONE;
    int16_t lastPressX = 0;
    int16_t lastPressY = 0;
    unsigned long lastPressTime = 0;  // the last time the screen touch state changed
    MenuPage *pressedPage = nullptr;
    MenuButton *pressedButton = nullptr;

};


/*********************
 * Non Class Functions
 *********************/

void drawButtonOutline(Adafruit_GFX *tft, int16_t buttonX, int16_t buttonY, int16_t buttonWidth, int16_t buttonHeight, uint16_t buttonColor, bool buttonActive);
void centerText(Adafruit_GFX *tft, String text, int16_t centerX, int16_t centerY, int16_t textSize, uint16_t textColor);
uint16_t getTextWidth (Adafruit_GFX *tft, String text, int16_t textSize);


#endif