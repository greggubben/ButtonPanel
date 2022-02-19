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

#include <Arduino.h>
#include "Menu.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>
#include <vector>
#include <tuple>

using namespace std;

// Uncomment the following define to debug the Finding of a pressed Button
//#define MENU_FINDBUTTON_DEBUG

// Uncomment the following define to debug the Setting of an Active Button
//#define MENU_ACTIVEBUTTON_DEBUG

// Uncomment the following define to debug the screen Draw
//#define MENU_DRAW_DEBUG

// Uncomment the following define to debug the screen Event Handling
#define MENU_HANDLE_DEBUG


/*********************
 * Menu Class
 *********************/

Menu::Menu(Adafruit_GFX *tft) {
  init(tft, nullptr, DEFAULT_BACKGROUND_COLOR);
}

Menu::Menu(Adafruit_GFX *tft, vector<MenuPage*> *tMenus) {
  init(tft, tMenus, DEFAULT_BACKGROUND_COLOR);
}

Menu::Menu(Adafruit_GFX *tft, vector<MenuPage*> *tMenus, uint16_t bgColor) {
  init(tft, tMenus, bgColor);
}

//
// Initialize the Menu variables
//
void Menu::init(Adafruit_GFX *tft, vector<MenuPage*> *tMenus, uint16_t bgColor) {
  _tft = tft;
  backgroundColor = bgColor;
  topMenus = tMenus;
  startPage = 0;

  setTopButtons(DEFAULT_PAGETOP_BUTTONS);
  setTopHeight(DEFAULT_PAGETOP_HEIGHT);

}


//
// Set the number of buttons across the top
//
void Menu::setTopButtons(int16_t buttons) {
  topButtons = buttons;
  calculateTopButtonDimensions();
}


//
// Set the pixel size of the top menu bar
//
void Menu::setTopHeight(int16_t height) {
  topHeight = height;
  calculateTopButtonDimensions();
}


//
// Calculate the sizes for the Top Menu Buttons
// Also calculate the size of the bottom panel
//
void Menu::calculateTopButtonDimensions() {
  topButtonHeight = topHeight;
  topButtonWidth = (screenWidth - (2* DEFAULT_PAGETOP_NEXT_BUTTON_SIZE)) / topButtons;
  panelX = 0;
  panelY = topHeight + 1;
  panelWidth = screenWidth;
  panelHeight = screenHeight - panelY;
}


//
// Set up the Menu by initializing variables
// The TFT must be started before this is called
//
bool Menu::setup() {
  screenHeight = _tft->height();
  screenWidth  = _tft->width();

  // Check that the screen has a size
  if (screenHeight == 0 || screenWidth == 0) {
    // TFT not started.
    return false;
  }

  calculateTopButtonDimensions();

  return true;
}


//
// Draw the top menu and the corresponding button panel
//
void Menu::draw() {
  if (clearScreenBeforeDraw) {
    #ifdef MENU_DRAW_DEBUG
    Serial.println("Clear Screen");
    #endif
    _tft->fillScreen(DEFAULT_BACKGROUND_COLOR);
    clearScreenBeforeDraw = false;
  }
  #ifdef MENU_DRAW_DEBUG
  Serial.print("Height:");
  Serial.println(screenHeight);
  Serial.print("Width:");
  Serial.println(screenWidth);
  #endif
  uint16_t barColor = DEFAULT_PAGETOP_COLOR;
  if (topMenus) {
    #ifdef MENU_DRAW_DEBUG
    Serial.println("Drawing Top Buttons");
    #endif
    int16_t pos = 0;
    int16_t buttonTopLeftX = DEFAULT_PAGETOP_NEXT_BUTTON_SIZE;
    int16_t buttonTopLeftY = 0;
    bool moreButtonsRightShown = false;

    if (startPage == 0) {
        #ifdef MENU_DRAW_DEBUG
        Serial.println("  No more Buttons on Left Indicator");
        #endif
        // Hide/Remove more buttons left indicator
        _tft->fillRect(0, DEFAULT_BUTTON_CORNER, DEFAULT_PAGETOP_NEXT_BUTTON_SIZE - 1, topButtonHeight - (2 * DEFAULT_BUTTON_CORNER), DEFAULT_BACKGROUND_COLOR);
    }

    auto topMenu = topMenus->begin();
    for (; topMenu != topMenus->end(); topMenu++) {

      if (pos == (startPage - 1)) {
        #ifdef MENU_DRAW_DEBUG
        Serial.println("  Show more Buttons on Left Indicator");
        #endif
        // Show there is more buttons to left
        _tft->fillRect(0, DEFAULT_BUTTON_CORNER, DEFAULT_PAGETOP_NEXT_BUTTON_SIZE - 1, topButtonHeight - (2 * DEFAULT_BUTTON_CORNER), (*topMenu)->getColor());
      }

      if (startPage <= pos && pos < (startPage + DEFAULT_PAGETOP_BUTTONS)) {
        // Button in visible range
        #ifdef MENU_DRAW_DEBUG
        Serial.print("  buttonTopLeftX:");
        Serial.println(buttonTopLeftX);
        Serial.print("  buttonTopLeftY:");
        Serial.println(buttonTopLeftY);
        #endif
        (*topMenu)->draw(_tft, buttonTopLeftX, buttonTopLeftY, topButtonWidth, topButtonHeight);
        buttonTopLeftX += topButtonWidth;
      }

      if (pos == (startPage + DEFAULT_PAGETOP_BUTTONS)) {
        #ifdef MENU_DRAW_DEBUG
        Serial.println("  Show more Buttons on Right Indicator");
        #endif
        // Show there is more buttons to right
        _tft->fillRect(screenWidth - (DEFAULT_PAGETOP_NEXT_BUTTON_SIZE - 1), DEFAULT_BUTTON_CORNER, DEFAULT_PAGETOP_NEXT_BUTTON_SIZE - 1, topButtonHeight - (2 * DEFAULT_BUTTON_CORNER), (*topMenu)->getColor());
        moreButtonsRightShown = true;
      }

      if ((*topMenu)->isActive()) {
        #ifdef MENU_DRAW_DEBUG
        Serial.print("  Active Page ");
        Serial.println((*topMenu)->getName());
        #endif
        barColor = (*topMenu)->getColor();
        (*topMenu)->drawPanelButtons(_tft, panelX, panelY, panelWidth, panelHeight);
      }

      pos++;
    }

    if (!moreButtonsRightShown) {
        #ifdef MENU_DRAW_DEBUG
        Serial.println("  No more Buttons on Right Indicator");
        #endif
      // Hide/Remove more buttons right indicator
      _tft->fillRect(screenWidth - (DEFAULT_PAGETOP_NEXT_BUTTON_SIZE - 1), DEFAULT_BUTTON_CORNER, DEFAULT_PAGETOP_NEXT_BUTTON_SIZE - 1, topButtonHeight - (2 * DEFAULT_BUTTON_CORNER), DEFAULT_BACKGROUND_COLOR);
    }

  }

  // Draw the separator bar in the color of the active top menu
  int16_t barHeight = DEFAULT_BAR_HEIGHT;
  int16_t barWidth = screenWidth;
  int16_t barX = 0;
  int16_t barY = topButtonHeight - barHeight;
  _tft->fillRect(barX, barY, barWidth, barHeight, barColor);
}



bool Menu::setActiveButton(MenuButton *activeButton) {
  bool newButtonActive = false;
  #ifdef MENU_ACTIVEBUTTON_DEBUG
  Serial.println("Set Active Button for Active Page");
  #endif

  auto topMenu = topMenus->begin();
  for (; topMenu != topMenus->end(); topMenu++) {
    if ((*topMenu)->isActive()) {
      #ifdef MENU_ACTIVEBUTTON_DEBUG
      Serial.print("Set Active Button for ");
      Serial.println((*topMenu)->getName());
      #endif
      newButtonActive = (*topMenu)->setActiveMenuButton(activeButton);
    }
  }

  return newButtonActive;
}


bool Menu::setActiveTopMenu(MenuPage *activeMenu) {
  #ifdef MENU_ACTIVEBUTTON_DEBUG
  Serial.print("Set Active Page for ");
  Serial.println(activeMenu->getName());
  #endif

  bool newMenuFound = setActiveTopMenu(topMenus, activeMenu);
  if (newMenuFound) {
    #ifdef MENU_ACTIVEBUTTON_DEBUG
    Serial.println("New Menu Active - Clear Screen");
    #endif
    clearScreenBeforeDraw = true;
  }
  return newMenuFound;
}


//
// Find and set the Top Menu button to be active
// Set all other top menu buttons as inactive
//
bool Menu::setActiveTopMenu(vector<MenuPage*> *tMenus, MenuPage *activeMenu) {
  bool newMenuFound = false;

  #ifdef MENU_ACTIVEBUTTON_DEBUG
  Serial.println("Set Active Page");
  #endif

  auto topMenu = tMenus->begin();
  for (; topMenu != tMenus->end(); topMenu++) {
    // Set as active
    if ((*topMenu) == activeMenu) {
      #ifdef MENU_ACTIVEBUTTON_DEBUG
      Serial.print("Set Active Page ");
      Serial.println((*topMenu)->getName());
      #endif
      if (!(*topMenu)->isActive()) {
        (*topMenu)->setActive();
        newMenuFound = true;
      }
    }
    else {
      // Set all others as inactive
      (*topMenu)->setInactive();
    }
  }

  return newMenuFound;
}

//
// Handle a Touch Event by checking if any of the buttons were touched.
// If a MenuTop was pressed, then change the screen accordingly
// If the touch was in the panel, then check the MenuTop's buttons
//
MenuButton* Menu::findTouchedButton(int16_t pressX, int16_t pressY) {

  MenuButton *buttonPressed = nullptr;

  if (pressY > topButtonHeight) {
    // Panel button press
    #ifdef MENU_FINDBUTTON_DEBUG
    Serial.println("Panel");
    #endif
    auto topMenu = topMenus->begin();
    for (; topMenu != topMenus->end(); topMenu++) {
      if ((*topMenu)->isActive()) {
        #ifdef MENU_FINDBUTTON_DEBUG
        Serial.print("Top Menu Button ");
        Serial.println((*topMenu)->getName());
        #endif
        buttonPressed = (*topMenu)->findTouchedButton(pressX, pressY, panelX, panelY, panelWidth, panelHeight);
      }
    }
  }
  return buttonPressed;
}

//
// Handle a Touch Event by checking if any of the buttons were touched.
// If a MenuTop was pressed, then change the screen accordingly
// If the touch was in the panel, then check the MenuTop's buttons
//
MenuPage* Menu::findTouchedTopButton(int16_t pressX, int16_t pressY) {

  MenuPage *buttonPressed = nullptr;

  if (pressY < topButtonHeight) {
    // Top menu button press
    #ifdef MENU_FINDBUTTON_DEBUG
    Serial.println("Top Menu");
    #endif
    if (topMenus) {
      #ifdef MENU_FINDBUTTON_DEBUG
      Serial.println("Looking for Top Menu Button");
      #endif
      int16_t pos = 0;
      int16_t visiblePos = 0;
      auto topMenu = topMenus->begin();
      for (; topMenu != topMenus->end(); topMenu++) {

        if (startPage <= pos && pos < (startPage + DEFAULT_PAGETOP_BUTTONS)) {
          // Button in visible range

          int16_t buttonTopLeftX = visiblePos * topButtonWidth;
          int16_t buttonTopRightX = buttonTopLeftX + topButtonWidth;
          if (buttonTopLeftX < pressX && pressX < buttonTopRightX) {
            buttonPressed = (*topMenu);
            #ifdef MENU_FINDBUTTON_DEBUG
            Serial.print("Top Menu Button ");
            Serial.println(buttonPressed->getName());
            #endif
          }
          visiblePos++;
        }
        pos++;
      }
    }
  }
  return buttonPressed;
}


/*
 * Handle any touch screen actions
 */
void Menu::eventHandler(Event *event) {
  bool redraw = false;

  pressedButton = nullptr;  // New press, don't know button yet
  pressedPage = nullptr;  // New press, don't know button yet
  // Figure out which button was pressed
  if (event->pressY < topButtonHeight) {
    #ifdef MENU_HANDLE_DEBUG
    Serial.println("Top Button Area");
    #endif
    pressedPage = findTouchedTopButton(event->pressX, event->pressY);
  }
  else {
    #ifdef MENU_HANDLE_DEBUG
    Serial.println("Button Panel Area");
    #endif
    pressedButton = findTouchedButton(event->pressX, event->pressY);
  }

  #ifdef MENU_HANDLE_DEBUG
  Serial.println("Do Event: ");
  Serial.println(TouchEventNames[event->event]);
  #endif

  switch(event->event) {
    case EVENT_SWIPE_LEFT:
        #ifdef MENU_HANDLE_DEBUG
        Serial.println("Do Swipe Left Event");
        #endif
        if (startPage < (int) (topMenus->size() - DEFAULT_PAGETOP_BUTTONS)) {
          // Finger moving to Left
          // Move Top Buttons to Right
          startPage++;
          redraw = true;
        }
        break;

    case EVENT_SWIPE_RIGHT:
        #ifdef MENU_HANDLE_DEBUG
        Serial.println("Do Swipe Right Event");
        #endif
        if (startPage > 0 ) {
          // Finger moving to Right
          // Move Top Buttons to Left
          startPage--;
          redraw = true;
        }
        break;

    case EVENT_LONG:
        #ifdef MENU_HANDLE_DEBUG
        Serial.println("Do Long Event");
        #endif
        // Handle any Long Press callback actions
        // Do immediately, do not wait for unTouch
        if (pressedPage) {
          #ifdef MENU_HANDLE_DEBUG
          Serial.print("Do Page Long Event: ");
          Serial.println(pressedPage->getName());
          #endif
          bool set_redraw = setActiveTopMenu(pressedPage);
          bool callback_redraw = pressedPage->callbackLongPress();
          redraw = set_redraw || callback_redraw;
        }
        else if (pressedButton) {
          #ifdef MENU_HANDLE_DEBUG
          Serial.print("Do Button Long Event: ");
          Serial.println(pressedButton->getName());
          #endif
          bool set_redraw = setActiveButton(pressedButton);
          bool callback_redraw = pressedButton->callbackLongPress();
          redraw = set_redraw || callback_redraw;
        }
        break;

    case EVENT_SHORT:
        // Handle any Short Press callback actions
        // Long press is fired as soon as it occurs
        #ifdef MENU_HANDLE_DEBUG
        Serial.println("Do Short Event");
        #endif
        if (pressedPage) {
          #ifdef MENU_HANDLE_DEBUG
          Serial.print("Do Page Short Event: ");
          Serial.println(pressedPage->getName());
          #endif
          bool set_redraw = setActiveTopMenu(pressedPage);
          bool callback_redraw = pressedPage->callbackShortPress();
          redraw = set_redraw || callback_redraw;
        }
        else if (pressedButton) {
          #ifdef MENU_HANDLE_DEBUG
          Serial.print("Do Button Short Event: ");
          Serial.println(pressedButton->getName());
          #endif
          bool set_redraw = setActiveButton(pressedButton);
          bool callback_redraw = pressedButton->callbackShortPress();
          redraw = set_redraw || callback_redraw;
        }
        break;

    default:
        #ifdef MENU_HANDLE_DEBUG
        Serial.println("Do Nothing");
        #endif
        break;
  }

  // Re-draw the menus
  if (redraw) {
    #if defined(MENU_HANDLE_DEBUG) || defined(MENU_FINDBUTTON_DEBUG) || defined(MENU_ACTIVEBUTTON_DEBUG)
    Serial.println("Re-Draw");
    #endif
    draw();
  }

}

