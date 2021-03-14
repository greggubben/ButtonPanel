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
  Serial.print("    ButtonMenu: ");
  Serial.println(name);
  Serial.print("    topButtonX:");
  Serial.println(topButtonX);
  Serial.print("    topButtonY:");
  Serial.println(topButtonY);

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
 * MenuTop Class
 *********************/

/*
 * Constructor
 *
 * @param label         Label or name display on top button
 * @param buttonsAcross Number of buttons to display across panel
 * @param buttonsDown   Number of buttons to display down panel
 * @param menuColor     Optional MenuTop color. Default DEFAULT_TOP_COLOR
 */ 
MenuPage::MenuPage(const char* label, int16_t buttonsAcross, int16_t buttonsDown, uint16_t menuColor) {
  init(label, buttonsAcross, buttonsDown, nullptr, menuColor);
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
MenuPage::MenuPage(const char* label, int16_t buttonsAcross, int16_t buttonsDown, vector<MenuButton*> *menuButtons, uint16_t menuColor) {
  init(label, buttonsAcross, buttonsDown, menuButtons, menuColor);
}


//
// Initialize the Menu Top variables.
//
void MenuPage::init(const char* label, int16_t buttonsAcross, int16_t buttonsDown, vector<MenuButton*> *menuButtons, uint16_t menuColor) {
  name = String(label);
  buttonsX = buttonsAcross;
  buttonsY = buttonsDown;
  color = menuColor;
  active = false;
  buttons = menuButtons;
  textSize = DEFAULT_TOP_TEXT_SIZE;
  textColor = DEFAULT_TOP_COLOR;
}


//
// Draw the Top Menu button
//
void MenuPage::draw(Adafruit_GFX *tft, int16_t topButtonX, int16_t topButtonY, int16_t topButtonWidth, int16_t topButtonHeight) {
  Serial.print("  TopMenu: ");
  Serial.println(name);
  Serial.print("  topButtonX:");
  Serial.println(topButtonX);
  Serial.print("  topButtonY:");
  Serial.println(topButtonY);

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
    // Calculate the size of the buttons
    int16_t buttonWidth = panelWidth / buttonsX;
    int16_t buttonHeight = panelHeight / buttonsY;

    // Go through each button
    auto button = buttons->begin();
    for (; button != buttons->end(); button++) {
      // Get the button's button position
      int16_t positionX = (*button)->getPositionX();
      int16_t positionY = (*button)->getPositionY();
      Serial.print("   button->name:");
      Serial.println((*button)->getName());
      Serial.print("   button->x");
      Serial.println(positionX);
      Serial.print("   button->y");
      Serial.println(positionY);
      // Convert button position to pixel position
      int16_t buttonTopLeftX = positionX * buttonWidth + panelX;
      int16_t buttonTopLeftY = positionY * buttonHeight + panelY;

      // Draw the button at the pixel position
      (*button)->draw(tft, buttonTopLeftX, buttonTopLeftY, buttonWidth, buttonHeight);
    }

  }
}


//
// Find and set the button to be active
// Set all other buttons as inactive
//
bool MenuPage::setActiveMenuButton(vector<MenuButton*> *mButtons, MenuButton *activeButton) {
  bool buttonFound = false;

  auto button = mButtons->begin();
  for (; button != mButtons->end(); button++) {
    if ((*button) == activeButton) {
      // This is the button to make active
      if (!(*button)->hasShortPressCallback()) {
        // Only set to active if there is no callback function
        // otherwise expect the callback to handle setting button active state
        (*button)->setActive();
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
tuple <bool, MenuItem *> MenuPage::handleTouch(int16_t pressX, int16_t pressY, int16_t panelX, int16_t panelY, int16_t panelWidth, int16_t panelHeight) {

  MenuItem *buttonPressed = nullptr;
//  *buttonPressed = nullptr;
  bool newButtonPress = false;

  if (buttons) {
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
        if (!(*button)->isActive()) {
          // Button was not active before, new press
          newButtonPress = true;
        }
        setActiveMenuButton(buttons, (*button));
        Serial.print("Button Pressed: ");
        Serial.println((*button)->getName());
        // Remember which button
        buttonPressed = (*button);
      }
    }

  }

  return make_tuple(newButtonPress, buttonPressed);
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

  setTopButtons(DEFAULT_TOP_BUTTONS);
  setTopHeight(DEFAULT_TOP_HEIGHT);

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
  topButtonWidth = screenWidth / topButtons;
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
  shortPress = false;
  longPress = false;
  swipeLeft = false;
  swipeRight = false;
  lastPressTime = millis();

  return true;
}


//
// Draw the top menu and the corresponding button panel
//
void Menu::draw() {
  if (clearScreenBeforeDraw) {
    _tft->fillScreen(DEFAULT_BACKGROUND_COLOR);
    clearScreenBeforeDraw = false;
  }
  Serial.print("Height:");
  Serial.println(screenHeight);
  Serial.print("Width:");
  Serial.println(screenWidth);
  uint16_t barColor = DEFAULT_TOP_COLOR;
  if (topMenus) {
    int16_t pos = 0;
    auto topMenu = topMenus->begin();
    for (; topMenu != topMenus->end(); topMenu++) {
      bool show = true;
      int16_t buttonTopLeftX = pos * topButtonWidth;
      int16_t buttonTopLeftY = 0;
      if (buttonTopLeftX < 0 || buttonTopLeftX > screenWidth) {
        show = false;
      }
      Serial.print("buttonTopLeftX:");
      Serial.println(buttonTopLeftX);
      Serial.print("buttonTopLeftY:");
      Serial.println(buttonTopLeftY);
      if (show) {
        (*topMenu)->draw(_tft, buttonTopLeftX, buttonTopLeftY, topButtonWidth, topButtonHeight);
        if ((*topMenu)->isActive()) {
          barColor = (*topMenu)->getColor();
          (*topMenu)->drawPanelButtons(_tft, panelX, panelY, panelWidth, panelHeight);
        }
      }
      pos++;
    }
  }

  // Draw the separator bar in the color of the active top menu
  int16_t barHeight = DEFAULT_BAR_HEIGHT;
  int16_t barWidth = screenWidth - (2*DEFAULT_PADDING_X);
  int16_t barX = DEFAULT_PADDING_X;
  int16_t barY = topButtonHeight - barHeight;
  _tft->fillRect(barX, barY, barWidth, barHeight, barColor);
}


//
// Find and set the Top Menu button to be active
// Set all other top menu buttons as inactive
//
bool Menu::setActiveTopMenu(vector<MenuPage*> *tMenus, MenuPage *activeMenu) {
  bool menuFound = false;

  auto topMenu = tMenus->begin();
  for (; topMenu != tMenus->end(); topMenu++) {
    // Set as active
    if ((*topMenu) == activeMenu) {
      (*topMenu)->setActive();
      menuFound = true;
    }
    else {
      // Set all others as inactive
      (*topMenu)->setInactive();
    }
  }

  return menuFound;
}


//
// Handle a Touch Event by checking if any of the buttons were touched.
// If a MenuTop was pressed, then change the screen accordingly
// If the touch was in the panel, then check the MenuTop's buttons
//
tuple <bool, MenuItem *> Menu::handleTouch(int16_t pressX, int16_t pressY) {

  MenuItem *buttonPressed = nullptr;
  bool newButtonActive = true;

  if (pressY < topButtonHeight) {
    Serial.println("Top Menu");
    // Top menu button press
    int16_t pos = 0;
    auto topMenu = topMenus->begin();
    for (; topMenu != topMenus->end(); topMenu++) {
      int16_t buttonTopLeftX = pos * topButtonWidth;
      int16_t buttonTopRightX = buttonTopLeftX + topButtonWidth;
      if (buttonTopLeftX < pressX && pressX < buttonTopRightX) {
        if (!(*topMenu)->isActive()) {
          // New Top Menu
          clearScreenBeforeDraw = true;
          Serial.print("Top Menu Button ");
          Serial.println((*topMenu)->getName());
          newButtonActive = setActiveTopMenu(topMenus,(*topMenu));
        }
        buttonPressed = (*topMenu);
      }
      pos++;
    }
  }
  else {
    Serial.println("Panel");
    auto topMenu = topMenus->begin();
    for (; topMenu != topMenus->end(); topMenu++) {
      if ((*topMenu)->isActive()) {
        tie(newButtonActive, buttonPressed) = (*topMenu)->handleTouch(pressX, pressY, panelX, panelY, panelWidth, panelHeight);
      }
    }

  }
  return make_tuple(newButtonActive, buttonPressed);
}


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
      Serial.println("Touch Change");
      debounceTouched = isTouched;

      // New press
      if (isTouched) {
        Serial.println("New Touch");
        // On first press - Don't which of the 4 modes yet
        shortPress = false;
        longPress = false;
        swipeLeft = false;
        swipeRight = false;
        pressedButton = nullptr;  // New press, don't know button yet

        // Find the calibrated X and Y of the press
        TS_Point point = _ts->getPoint();
        pressX = point.x;
        pressY = point.y;
        if (_calibrateTouch) {
          (*_calibrateTouch)(&pressX, &pressY);
        }

        // Figure out which button was pressed
        tie(redraw, pressedButton) = handleTouch(pressX, pressY);
      }
      else {
        Serial.println("New Stop Touch");
        // Stopped touching
        if (!longPress) {
          Serial.println("Lets do the Event");
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

      if (!swipeRight && (swipeX > pressX) && (swipeX - pressX) > SWIPE_MIN_PIXELS) {
        Serial.println("Swipe Right");
        swipeRight = true;
        swipeLeft = false;
        shortPress = false;
        longPress = false;
      }
      if (!swipeLeft && (pressX > swipeX) && (pressX - swipeX) > SWIPE_MIN_PIXELS) {
        Serial.println("Swipe Left");
        swipeLeft = true;
        swipeRight = false;
        shortPress = false;
        longPress = false;
      }

      if (!shortPress && !swipeLeft && !swipeRight &&
        (millis() - lastPressTime) > SHORTPRESS_DELAY) {
        Serial.println("Short Press");
        // New Short Press occurred
        // Wait to see if Long Press occurred
        // otherwise will perform on unTouch
        shortPress = true;
      }

      // Short Press being true ensures the press is still active
      if (shortPress && (millis() - lastPressTime) > LONGPRESS_DELAY) {
        if (!longPress) {
          Serial.println("Long Press");
          // New Long press occurred
          longPress = true;
          // Do Immediately
          doEvent = true;
        }
      }
    }
  }

  if (doEvent) {
    Serial.println("Do Event");
    if (swipeLeft) {
      Serial.println("Do Swipe Left Event");

    }
    else if (swipeRight) {
      Serial.println("Do Swipe Right Event");

    }
    else if (longPress) {
      Serial.println("Do Long Event");
      // Handle any Long Press callback actions
      // Do immediately, do not wait for unTouch
      if (pressedButton) {
        Serial.println("Do Long Event Callback");
        redraw = pressedButton->callbackLongPress();
      }
    }
    else if (shortPress) {
      Serial.println("Do Short Event");
      // Handle any Short Press callback actions
      // Long press is fired as soon as it occurs
      if (pressedButton) {
        Serial.println("Do Short Event Callback");
        redraw = pressedButton->callbackShortPress();
      }
    }
  }

  // Re-draw the menus
  if (redraw) {
    draw();
  }

  if (resetVariables) {
    Serial.println("Reset Variables");
    // Just stopped touching
    // Reset everything for next touch event
    shortPress = false;
    longPress = false;
    swipeLeft = false;
    swipeRight = false;
    pressedButton = nullptr;
    pressX = 0;
    pressY = 0;
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