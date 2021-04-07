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

using namespace std;

// MenuButton Defaults
#define DEFAULT_PADDING_X 2                       // Default horizontal padding in pixels
#define DEFAULT_PADDING_Y 2                       // Default vertical padding in pixels
#define DEFAULT_BUTTON_CORNER 8                   // Default roundness of corners in pixels
#define DEFAULT_BUTTON_COLOR ILI9341_CYAN         // Default color of the button background
#define DEFAULT_BUTTON_TEXT_COLOR ILI9341_WHITE   // Default color of the button text
#define DEFAULT_BUTTON_TEXT_SIZE 2                // Default size of the button text


// MenuPage Defaults
#define DEFAULT_PAGETOP_BUTTONS 3                 // Number of buttons to show across top
#define DEFAULT_PAGETOP_HEIGHT 80                 // Height of button section on top in pixels
#define DEFAULT_PAGETOP_COLOR ILI9341_CYAN        // Default Color of menu page
#define DEFAULT_PAGETOP_TEXT_COLOR ILI9341_WHITE  // Default Text color for menu page button
#define DEFAULT_PAGETOP_TEXT_SIZE 2               // Default Text Size of menu page button
#define DEFAULT_PAGETOP_NEXT_BUTTON_SIZE 3        // Default width of next button indicator in pixels


// Menu Defaults
#define DEFAULT_BACKGROUND_COLOR ILI9341_BLACK    // Default background
#define DEFAULT_BAR_HEIGHT DEFAULT_BUTTON_CORNER  // Default height of menu page separator bar


// Define minimum delays for when screen is touched
#define DEBOUNCE_DELAY 50       // Minimum amount of millis to detect a touch change
#define SHORTPRESS_DELAY 60     // Minimum amount of millis to detect a short press
#define LONGPRESS_DELAY 400     // Minimum amount of millis to detect a long press
#define SWIPE_MIN_PIXELS 40     // Minimum distance in pixels to detect a swipe action


// Contains all the Events a touch could generate
enum TouchEvent {
  EVENT_NONE = 0,         // No Events occurred
  EVENT_SHORT = 1,        // Short Press Event has occurred
  EVENT_LONG = 2,         // Long Press Event has occurred
  EVENT_SWIPE_RIGHT = 3,  // Finger traveling to Right
  EVENT_SWIPE_LEFT =4     // Finger travelling to Left
};

// Text representation of the Touch Events defined by the TouchEvent enum
static const char* TouchEventStrings[] = {"None", "Short", "Long", "Swipe Right", "Swipe Left"};


/*********************
 * Generic definitions for passing functions as arguments
 *********************/

/*!
 * @brief Callback function for a Short or Long button press
 * 
 * @return bool to indicate the screen should be re-drawn
 */
typedef bool (*ButtonPressCallback)();

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
 * @brief Function to calibrate the touch position to the display position
 * 
 * @param int16_t* pointer to the raw X position, calibrated result set by function
 * @param int16_t* pointer to the raw Y position, calibrated result set by function
 * 
 * @return calibrated results overwrites input parameters.
 */
typedef void (*CalibrateTouchCallback)(int16_t*, int16_t*);

/*!
 * @brief Function to perform setup activities for a menu page.
 * 
 * @param Adafruit_GFX* pointer to touch screen to display setup messages
 */
typedef void (*MenuPageSetup)(Adafruit_GFX*);


/*********************
 * MenuItem Class
 *********************/

//
// Generic class inherited by all Menu classes
// Used to hold common data and functions
//
class MenuItem {
  public:

    /*!
     * @brief Set that this Item is now Active
     */
    void setActive() { active = true; };

    /*!
     * @brief Set that this Item is no longer Active
     */
    void setInactive() { active = false; };

    /*!
     * @brief Return the Active state of this Item
     * @return bool of Active state
     */
    bool isActive() { return active; };

    /*!
     * @brief Set the color of this item
     */
    void setColor(uint16_t c) {color = c; };

    /*!
     * @brief Return the color of this item
     * @return color
     */
    uint16_t getColor() {return color; };

    /*!
     * @brief Set the name of this item
     */
    void setName(String n) { name = n;};

    /*!
     * @brief Return the name of this item
     * @return String
     */
    String getName() {return name; };

    /*!
     * @brief Set the text size of this item
     */
    void setTextSize(int16_t s) { textSize = s;};

    /*!
     * @brief Set the text color of this item
     */
    void setTextColor(int16_t c) { textColor = c;};

    /*!
     * @brief Set the Short Press Callback Function for this item
     */
    void setShortPressCallback(ButtonPressCallback onPress) { shortPressCallback = onPress;};

    /*!
     * @brief Indicate if there is a Short Press Callback Function for this item
     * @return bool True if a Short Press Callback funciton has been defined; false if none
     */
    bool hasShortPressCallback() { return (shortPressCallback != nullptr);};
    bool callbackShortPress();

    /*!
     * @brief Set the Long Press Callback Function for this item
     */
    void setLongPressCallback(ButtonPressCallback onPress) { longPressCallback = onPress;};

    /*!
     * @brief Indicate if there is a Long Press Callback Function for this item
     * @return bool True if a Long Press Callback funciton has been defined; false if none
     */
    bool hasLongPressCallback() { return (longPressCallback != nullptr);};
    bool callbackLongPress();

  protected:
    MenuItem() {};            // Hidden Constructor
    String name;              // Name to put in the Button
    bool active = false;      // Indicates if this Item is active; true = Active; false = inactive
    uint16_t color;           // Color to use for this menu
    int16_t textSize;         // Size of Font for menu
    uint16_t textColor;       // Text Color for menu
    ButtonPressCallback shortPressCallback = nullptr;  // Function to call if the button was pressed
    ButtonPressCallback longPressCallback = nullptr;  // Function to call if the button was pressed


};


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
    
    bool handle(CalibrateTouchCallback _calibrateTouch = nullptr);

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