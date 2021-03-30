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

// Uncomment the following define to debug the screen Event Handling
//#define MENU_HANDLE_DEBUG

// Uncomment the following define to debug the Finding of a pressed Button
//#define MENU_FINDBUTTON_DEBUG

// Uncomment the following define to debug the Setting of an Active Button
//#define MENU_ACTIVEBUTTON_DEBUG

// Uncomment the following define to debug the screen Draw
//#define MENU_DRAW_DEBUG



/*********************
 * MenuButton Class functions
 *********************/

/*
 * Call the Short Press Callback function if defined.
 * return boolean indicating something changed.
 */
bool MenuItem::callbackShortPress() {
  bool changeMade = false;
  Serial.println("Button Short Press Callback");
  if (shortPressCallback) {
    changeMade = shortPressCallback();
  }
  return changeMade;
}

/*
 * Call the Long Press Callback function if defined.
 * return boolean indicating something changed.
 */
bool MenuItem::callbackLongPress() {
  bool changeMade = false;
  Serial.println("Button Long Press Callback");
  if (longPressCallback) {
    changeMade = longPressCallback();
  }
  return changeMade;
}


/*********************
 * MenuButton Class functions
 *********************/

/*
 * Constructor
 *
 * @param label       Label or name display on button
 * @param x,y         X and Y button position (not pixel position)
 * @param buttonColor Optional button color. Default is DEFAULT_BUTTON_COLOR
 * @param onShortPress Optional callback function when a short press is performed
 * @param onLongPress Optional callback function when a long press is performed
 */ 
MenuButton::MenuButton(const char* label, int16_t x, int16_t y, uint16_t buttonColor, buttonPressCallback onShortPress, buttonPressCallback onLongPress) {
  name = String(label);
  positionX = x;
  positionY = y;
  color = buttonColor;
  textSize = DEFAULT_BUTTON_TEXT_SIZE;
  textColor = DEFAULT_BUTTON_COLOR;
  shortPressCallback = onShortPress;
  longPressCallback = onLongPress;
}


/*
 * Draw the Button at the pixel position and size provided
 */
void MenuButton::draw(Adafruit_GFX *tft, int16_t topButtonX, int16_t topButtonY, int16_t topButtonWidth, int16_t topButtonHeight) {
  #ifdef MENU_DRAW_DEBUG
  Serial.print("        ButtonMenu: ");
  Serial.println(name);
  Serial.print("        topButtonX:");
  Serial.println(topButtonX);
  Serial.print("        topButtonY:");
  Serial.println(topButtonY);
  #endif

  drawButtonOutline(tft, topButtonX, topButtonY, topButtonWidth, topButtonHeight, color, active);

  int16_t centerX = topButtonX + (topButtonWidth/2);
  int16_t centerY = topButtonY + (topButtonHeight/2);

  centerText(tft, name, centerX, centerY, textSize, textColor);

}


//
// Check to see if the button was touched.
//
// @return true if the button should be redrawn
//
bool MenuButton::handleTouch() {
  bool touchHandled = true;
  Serial.println("Button Handle Press");
  //if (shortPressCallback) {
  //  touchHandled = shortPressCallback();
  //}
  return touchHandled;
}


/*********************
 * MenuPage Class
 *********************/

/*
 * Constructor
 *
 * @param label         Label or name display on top button
 * @param buttonsAcross Number of buttons to display across panel
 * @param buttonsDown   Number of buttons to display down panel
 * @param menuColor     Optional MenuTop color. Default DEFAULT_TOP_COLOR
 */ 
MenuPage::MenuPage(const char* label, int16_t buttonsAcross, int16_t buttonsDown, uint16_t menuColor, buttonPressCallback onShortPress, buttonPressCallback onLongPress) {
  init(label, buttonsAcross, buttonsDown, nullptr, menuColor, onShortPress, onLongPress);
}

/*
 * Constructor
 *
 * @param label         Label or name display on top button
 * @param buttonsAcross Number of buttons to display across panel
 * @param buttonsDown   Number of buttons to display down panel
 * @param menuButtons   Vector containing the list of buttons to display in the panel
 * @param menuColor     Optional MenuTop color. Default DEFAULT_TOP_COLOR
 */ 
MenuPage::MenuPage(const char* label, int16_t buttonsAcross, int16_t buttonsDown, vector<MenuButton*> *menuButtons, uint16_t menuColor, buttonPressCallback onShortPress, buttonPressCallback onLongPress) {
  init(label, buttonsAcross, buttonsDown, menuButtons, menuColor, onShortPress, onLongPress);
}


//
// Initialize the Menu Top variables.
//
void MenuPage::init(const char* label, int16_t buttonsAcross, int16_t buttonsDown, vector<MenuButton*> *menuButtons, uint16_t menuColor, buttonPressCallback onShortPress, buttonPressCallback onLongPress) {
  name = String(label);
  buttonsX = buttonsAcross;
  buttonsY = buttonsDown;
  color = menuColor;
  active = false;
  buttons = menuButtons;
  textSize = DEFAULT_PAGETOP_TEXT_SIZE;
  textColor = DEFAULT_PAGETOP_COLOR;
  shortPressCallback = onShortPress;
  longPressCallback = onLongPress;

}


//
// Draw the Top Menu button
//
void MenuPage::draw(Adafruit_GFX *tft, int16_t topButtonX, int16_t topButtonY, int16_t topButtonWidth, int16_t topButtonHeight) {
  #ifdef MENU_DRAW_DEBUG
  Serial.print("  TopMenu: ");
  Serial.println(name);
  Serial.print("  topButtonX:");
  Serial.println(topButtonX);
  Serial.print("  topButtonY:");
  Serial.println(topButtonY);
  #endif

  drawButtonOutline(tft, topButtonX, topButtonY, topButtonWidth, topButtonHeight, color, active);

  int16_t centerX = topButtonX + (topButtonWidth/2);
  int16_t centerY = topButtonY + (topButtonHeight/2);

  centerText(tft, name, centerX, centerY, textSize, textColor);

}


//
// Draw a button panel if they exist
//
void MenuPage::drawPanelButtons(Adafruit_GFX *tft, int16_t panelX, int16_t panelY, int16_t panelWidth, int16_t panelHeight) {
  if (buttons) {
    #ifdef MENU_DRAW_DEBUG
    Serial.println("    Show Panel Buttons");
    #endif

    // Calculate the size of the buttons
    int16_t buttonWidth = panelWidth / buttonsX;
    int16_t buttonHeight = panelHeight / buttonsY;

    // Go through each button
    auto button = buttons->begin();
    for (; button != buttons->end(); button++) {
      // Get the button's button position
      int16_t positionX = (*button)->getPositionX();
      int16_t positionY = (*button)->getPositionY();
      #ifdef MENU_DRAW_DEBUG
      Serial.print("     button->name:");
      Serial.println((*button)->getName());
      Serial.print("     button->x");
      Serial.println(positionX);
      Serial.print("     button->y");
      Serial.println(positionY);
      #endif
      // Convert button position to pixel position
      int16_t buttonTopLeftX = positionX * buttonWidth + panelX;
      int16_t buttonTopLeftY = positionY * buttonHeight + panelY;

      // Draw the button at the pixel position
      (*button)->draw(tft, buttonTopLeftX, buttonTopLeftY, buttonWidth, buttonHeight);
    }

  }
}


bool MenuPage::setActiveMenuButton(MenuButton *activeButton) {
  #ifdef MENU_ACTIVEBUTTON_DEBUG
  Serial.print("Set Active Button for ");
  Serial.println(name);
  #endif

  return setActiveMenuButton(buttons, activeButton);
}

//
// Find and set the button to be active
// Set all other buttons as inactive
//
bool MenuPage::setActiveMenuButton(vector<MenuButton*> *mButtons, MenuButton *activeButton) {
  bool buttonFound = false;

  #ifdef MENU_ACTIVEBUTTON_DEBUG
  Serial.println("Set Active Button");
  #endif
  auto button = mButtons->begin();
  for (; button != mButtons->end(); button++) {
    if ((*button) == activeButton) {
      #ifdef MENU_ACTIVEBUTTON_DEBUG
      Serial.print("Found Button ");
      Serial.println((*button)->getName());
      #endif
      // This is the button to make active
      if (!(*button)->hasShortPressCallback()) {
        // Only set to active if there is no callback function
        // otherwise expect the callback to handle setting button active state
        (*button)->setActive();
        #ifdef MENU_ACTIVEBUTTON_DEBUG
        Serial.println("  Setting as Active");
        #endif
      }
      buttonFound = true;
    }
    else {
      // All other buttons are set to inactive
      (*button)->setInactive();
    }
  }

  return buttonFound;
}


//
// Handle a touch event
// First checks to see if a button was touched
// Then sets the touched button as active.
//
MenuButton* MenuPage::findTouchedButton(int16_t pressX, int16_t pressY, int16_t panelX, int16_t panelY, int16_t panelWidth, int16_t panelHeight) {

  MenuButton *buttonPressed = nullptr;

  if (buttons) {
    #ifdef MENU_FINDBUTTON_DEBUG
    Serial.println("Checking Button Panel");
    #endif
    // Calculate the size of the button
    int16_t buttonWidth = panelWidth / buttonsX;
    int16_t buttonHeight = panelHeight / buttonsY;

    // Go through each button to check if it was touched
    auto button = buttons->begin();
    for (; button != buttons->end(); button++) {
      // Get the position of the button
      int16_t positionX = (*button)->getPositionX();
      int16_t positionY = (*button)->getPositionY();

      // Convert the button position to pixel positions
      int16_t buttonMinX = positionX * buttonWidth + panelX;
      int16_t buttonMaxX = buttonMinX + buttonWidth;
      int16_t buttonMinY = positionY * buttonHeight + panelY;
      int16_t buttonMaxY = buttonMinY + buttonHeight;

      // Compare button pixel positions to where the touch occurred
      if ((buttonMinX < pressX && pressX < buttonMaxX) &&
          (buttonMinY < pressY && pressY < buttonMaxY)) {
        // This button was touched
/*
        if (!(*button)->isActive()) {
          // Button was not active before, new press
          newButtonPress = true;
        }
*/
//        setActiveMenuButton(buttons, (*button));
        #ifdef MENU_FINDBUTTON_DEBUG
        Serial.print("Button Pressed: ");
        Serial.println((*button)->getName());
        #endif
        // Remember which button
        buttonPressed = (*button);
      }
    }

  }

  return buttonPressed;
}


/*********************
 * Menu Class
 *********************/

Menu::Menu(Adafruit_GFX *tft, XPT2046_Touchscreen *ts) {
  init(tft, ts, nullptr, DEFAULT_BACKGROUND_COLOR);
}

Menu::Menu(Adafruit_GFX *tft, XPT2046_Touchscreen *ts, vector<MenuPage*> *tMenus) {
  init(tft, ts, tMenus, DEFAULT_BACKGROUND_COLOR);
}

Menu::Menu(Adafruit_GFX *tft, XPT2046_Touchscreen *ts, vector<MenuPage*> *tMenus, uint16_t bgColor) {
  init(tft, ts, tMenus, bgColor);
}

//
// Initialize the Menu variables
//
void Menu::init(Adafruit_GFX *tft, XPT2046_Touchscreen *ts, vector<MenuPage*> *tMenus, uint16_t bgColor) {
  _tft = tft;
  _ts = ts;
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

  wasTouched = false;
  debounceTouched = false;
  touchEvent = EVENT_NONE;
  lastPressTime = millis();

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
bool Menu::handle(calibrateTouchCallback _calibrateTouch) {
  bool redraw = false;
  bool doEvent = false;
  bool resetVariables = false;

  // Get current state of screen being touched
  bool isTouched = _ts->touched();

  // There is a change in touch state
  if (isTouched != wasTouched) {
    // Change in touch state
    lastPressTime = millis();
  }

  // Make sure the screen touch state has stabilized
  if ((millis() - lastPressTime) > DEBOUNCE_DELAY) {

    // Is there a change in touch state?
    if (isTouched != debounceTouched) {
      #ifdef MENU_HANDLE_DEBUG
      Serial.println("Touch Change");
      #endif
      debounceTouched = isTouched;

      // New press
      if (isTouched) {
        #ifdef MENU_HANDLE_DEBUG
        Serial.println("New Touch");
        #endif
        // On first press - Don't which of the 4 modes yet
        touchEvent = EVENT_NONE;
        pressedButton = nullptr;  // New press, don't know button yet
        pressedPage = nullptr;  // New press, don't know button yet

        // Find the calibrated X and Y of the press
        TS_Point point = _ts->getPoint();
        lastPressX = point.x;
        lastPressY = point.y;
        if (_calibrateTouch) {
          (*_calibrateTouch)(&lastPressX, &lastPressY);
        }

        // Figure out which button was pressed
        if (lastPressY < topButtonHeight) {
          #ifdef MENU_HANDLE_DEBUG
          Serial.println("Top Button Area");
          #endif
          pressedPage = findTouchedTopButton(lastPressX, lastPressY);
        }
        else {
          #ifdef MENU_HANDLE_DEBUG
          Serial.println("Button Panel Area");
          #endif
          pressedButton = findTouchedButton(lastPressX, lastPressY);
        }
      }
      else {
        #ifdef MENU_HANDLE_DEBUG
        Serial.println("New Stop Touch");
        #endif
        // Stopped touching
        if (touchEvent != EVENT_LONG) {
          #ifdef MENU_HANDLE_DEBUG
          Serial.println("Lets do the Event");
          #endif
          doEvent = true;
        }
        resetVariables = true;
      }
    }

    // Is the screen still being touched?
    if (isTouched) {

      // Find the calibrated X and Y of the press
      TS_Point point = _ts->getPoint();
      int16_t swipeX = point.x;
      int16_t swipeY = point.y;
      if (_calibrateTouch) {
        (*_calibrateTouch)(&swipeX, &swipeY);
      }

      if ((touchEvent != EVENT_SWIPE_RIGHT) && (swipeX > lastPressX) && (swipeX - lastPressX) > SWIPE_MIN_PIXELS) {
        #ifdef MENU_HANDLE_DEBUG
        Serial.println("Swipe Right");
        #endif
        touchEvent = EVENT_SWIPE_RIGHT;
        pressedPage = nullptr;        // Swiping not pressing
        pressedButton = nullptr;        // Swiping not pressing
      }
      if ((touchEvent != EVENT_SWIPE_LEFT) && (lastPressX > swipeX) && (lastPressX - swipeX) > SWIPE_MIN_PIXELS) {
        #ifdef MENU_HANDLE_DEBUG
        Serial.println("Swipe Left");
        #endif
        touchEvent = EVENT_SWIPE_LEFT;
        pressedPage = nullptr;        // Swiping not pressing
        pressedButton = nullptr;        // Swiping not pressing
      }

      if ((touchEvent != EVENT_SHORT) && 
          (touchEvent != EVENT_SWIPE_LEFT) && 
          (touchEvent != EVENT_SWIPE_RIGHT) &&
          ((millis() - lastPressTime) > SHORTPRESS_DELAY)) {
        #ifdef MENU_HANDLE_DEBUG
        Serial.println("Short Press");
        #endif
        // New Short Press occurred
        // Wait to see if Long Press occurred
        // otherwise will perform on unTouch
        touchEvent = EVENT_SHORT;
      }

      // Short Press being true ensures the press is still active
      if ((touchEvent == EVENT_SHORT) && (millis() - lastPressTime) > LONGPRESS_DELAY) {
        if (touchEvent != EVENT_LONG) {
          #ifdef MENU_HANDLE_DEBUG
          Serial.println("Long Press");
          #endif
          // New Long press occurred
          touchEvent = EVENT_LONG;
          // Do Immediately
          doEvent = true;
        }
      }
    }
  }

  if (doEvent) {
    //#ifdef MENU_HANDLE_DEBUG
    Serial.print("Do Event: ");
    Serial.println(TouchEventStrings[touchEvent]);
    //#endif
    switch(touchEvent) {

//    if (swipeLeft) {
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
//    }
//    else if (swipeRight) {
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
//    }
//    else if (longPress) {
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
//    }
//    else if (shortPress) {
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
//    }
      default:
        #ifdef MENU_HANDLE_DEBUG
        Serial.println("Do Nothing");
        #endif
        break;
    }
  }

  // Re-draw the menus
  if (redraw) {
    #if defined(MENU_HANDLE_DEBUG) || defined(MENU_FINDBUTTON_DEBUG) || defined(MENU_ACTIVEBUTTON_DEBUG)
    Serial.println("Re-Draw");
    #endif
    draw();
  }

  if (resetVariables) {
    #ifdef MENU_HANDLE_DEBUG
    Serial.println("Reset Variables");
    #endif
    // Just stopped touching
    // Reset everything for next touch event
    touchEvent = EVENT_NONE;
    pressedButton = nullptr;
    lastPressX = 0;
    lastPressY = 0;
  }

  // Keep track of last touch state so changes can be identified.
  wasTouched = isTouched;

  // Provide current state of the screen being touched
  return isTouched;
}

/*********************
 * Non Class Functions
 *********************/

//
// Draw the Button
// If the button is inactive only the outline is drawn
// If the button is active a solid button is drawn
//
void drawButtonOutline(Adafruit_GFX *tft, int16_t buttonX, int16_t buttonY, int16_t buttonWidth, int16_t buttonHeight, uint16_t buttonColor, bool buttonActive) {
/*
  if ((buttonX < 0) || (buttonX >= BUTTONS_X )) {
    // X is out of Range
    return;
  }
  if ((buttonY < 0) || (buttonY >= BUTTONS_Y )) {
    // Y is out of Range
    return;
  }
*/
  
  int16_t topLeftX = buttonX + DEFAULT_PADDING_X;
  int16_t topLeftY = buttonY + DEFAULT_PADDING_Y;
  int16_t width = buttonWidth - (2*DEFAULT_PADDING_X);
  int16_t height = buttonHeight - (2*DEFAULT_PADDING_Y);
  tft->fillRect(topLeftX, topLeftY, width, height, DEFAULT_BACKGROUND_COLOR);
  if (buttonActive) {
    tft->fillRoundRect(topLeftX, topLeftY, width, height, DEFAULT_BUTTON_CORNER, buttonColor);
  }
  else {
    tft->drawRoundRect(topLeftX, topLeftY, width, height, DEFAULT_BUTTON_CORNER, buttonColor);
  }
}


//
// Function to print text centered vertically and horizontally to a single pixel
//
void centerText (Adafruit_GFX *tft, String text, int16_t centerX, int16_t centerY, int16_t textSize, uint16_t textColor) {

  tft->setTextSize(textSize);
  tft->setTextColor(textColor);

  int16_t stringX, stringY;
  uint16_t stringW, stringH;
  tft->getTextBounds(text, 0, 0, &stringX, &stringY, &stringW, &stringH);
  int16_t adjX = stringW/2;
  int16_t adjY = stringH/2;
  int16_t textX = centerX - adjX;
  int16_t textY = centerY - adjY;

  tft->setCursor(textX, textY);
  tft->print(text);

}