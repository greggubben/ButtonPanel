/*
 * Menu
 */

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
#define DEFAULT_BUTTONS_X 3
#define DEFAULT_BUTTONS_Y 4
#define DEFAULT_BUTTON_CORNER 8
#define DEFAULT_BUTTON_COLOR ILI9341_CYAN
#define DEFAULT_BUTTON_TEXT_COLOR ILI9341_WHITE
#define DEFAULT_BUTTON_TEXT_SIZE 2

// MenuTop Defaults
#define DEFAULT_TOP_BUTTONS 3
#define DEFAULT_TOP_HEIGHT 80
#define DEFAULT_TOP_COLOR ILI9341_CYAN
#define DEFAULT_TOP_TEXT_COLOR ILI9341_WHITE
#define DEFAULT_TOP_TEXT_SIZE 2

// Menu Defaults
#define DEFAULT_BACKGROUND_COLOR ILI9341_BLACK
#define DEFAULT_BAR_HEIGHT DEFAULT_BUTTON_CORNER

typedef bool (*buttonPressCallback)();


/*********************
 * MenuItem Class
 *********************/

class MenuItem {
  public:
    bool callbackShortPress();
    bool callbackLongPress();

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
    void setLongPressCallback(buttonPressCallback onPress) { longPressCallback = onPress;};
    bool hasLongPressCallback() { return (longPressCallback != nullptr);};

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

class MenuButton: public MenuItem {
  public:
//    MenuButton(const char* label, int16_t x, int16_t y, uint16_t buttonColor = DEFAULT_BUTTON_COLOR);
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
 * MenuTop Class
 *********************/

class MenuTop: public MenuItem {
  public:
    MenuTop(const char* label, int16_t buttonsAcross, int16_t buttonsDown);
    MenuTop(const char* label, int16_t buttonsAcross, int16_t buttonsDown, uint16_t menuColor);
    MenuTop(const char* label, int16_t buttonsAcross, int16_t buttonsDown, vector<MenuButton*> *menuButtons, uint16_t menuColor);
    void draw(Adafruit_GFX *tft, int16_t buttonX, int16_t buttonY, int16_t buttonWidth, int16_t buttonHeight);
    void drawPanelButtons(Adafruit_GFX *tft, int16_t panelX, int16_t panelY, int16_t panelWidth, int16_t panelHeight);
    tuple<bool, MenuItem *> handleTouch(int16_t pressX, int16_t pressY, int16_t panelX, int16_t panelY, int16_t panelWidth, int16_t panelHeight);
    static bool setActiveMenuButton(vector<MenuButton*> *mButtons, MenuButton *activeButton);


  private:
    void init(const char* label, int16_t buttonsAcross, int16_t buttonsDown, vector<MenuButton*> *menuButtons, uint16_t menuColor);

    int16_t buttonsX;         // Number of buttons across
    int16_t buttonsY;         // Number of buttons down
    vector<MenuButton*> *buttons = nullptr;

};


/*********************
 * Menu Class
 *********************/

class Menu {
  public:
    Menu(Adafruit_GFX *tft, XPT2046_Touchscreen *ts);
    Menu(Adafruit_GFX *tft, XPT2046_Touchscreen *ts, vector<MenuTop*> *tMenus = nullptr);
    Menu(Adafruit_GFX *tft, XPT2046_Touchscreen *ts, vector<MenuTop*> *tMenus, uint16_t bgColor);
    void setTopButtons(int16_t buttons);
    void setTopHeight(int16_t height);

    bool setup();
    void draw();

    tuple <bool, MenuItem *> handleTouch(int16_t pressX, int16_t pressY);

    static bool setActiveTopMenu(vector<MenuTop*> *tMenus, MenuTop *activeMenu);

  private:
    void init(Adafruit_GFX *tft, XPT2046_Touchscreen *ts, vector<MenuTop*> *tMenus = nullptr, uint16_t bgColor = DEFAULT_BACKGROUND_COLOR);
    void calculateTopButtonDimensions();
    Adafruit_GFX *_tft;
    XPT2046_Touchscreen *_ts;
    uint16_t backgroundColor;

    int16_t screenWidth;
    int16_t screenHeight;
    int16_t topButtons = DEFAULT_TOP_BUTTONS;
    int16_t topHeight = DEFAULT_TOP_HEIGHT;
    int16_t topButtonWidth = 0;
    int16_t topButtonHeight = 0;
    int16_t panelX = 0;
    int16_t panelY = 0;
    int16_t panelWidth = 0;
    int16_t panelHeight = 0;

    vector<MenuTop*> *topMenus = nullptr;
};


/*********************
 * Non Class Functions
 *********************/
void drawButtonOutline(Adafruit_GFX *tft, int16_t buttonX, int16_t buttonY, int16_t buttonWidth, int16_t buttonHeight, uint16_t buttonColor, bool buttonActive);
void centerText(Adafruit_GFX *tft, String text, int16_t centerX, int16_t centerY, int16_t textSize, uint16_t textColor);

#endif