/*
 * Menu
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
 * MenuButton Class
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
 * MenuButton Class
 *********************/

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
 * Draw the Top Menu button
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

MenuTop::MenuTop(const char* label, int16_t buttonsAcross, int16_t buttonsDown) {
  init(label, buttonsAcross, buttonsDown, nullptr, DEFAULT_TOP_COLOR);
}
MenuTop::MenuTop(const char* label, int16_t buttonsAcross, int16_t buttonsDown, uint16_t menuColor) {
  init(label, buttonsAcross, buttonsDown, nullptr, menuColor);
}

MenuTop::MenuTop(const char* label, int16_t buttonsAcross, int16_t buttonsDown, vector<MenuButton*> *menuButtons, uint16_t menuColor) {
  init(label, buttonsAcross, buttonsDown, menuButtons, menuColor);
}

void MenuTop::init(const char* label, int16_t buttonsAcross, int16_t buttonsDown, vector<MenuButton*> *menuButtons, uint16_t menuColor) {
  name = String(label);
  buttonsX = buttonsAcross;
  buttonsY = buttonsDown;
  color = menuColor;
  active = false;
  buttons = menuButtons;
  textSize = DEFAULT_TOP_TEXT_SIZE;
  textColor = DEFAULT_TOP_COLOR;
}

/*
 * Draw the Top Menu button
 */
void MenuTop::draw(Adafruit_GFX *tft, int16_t topButtonX, int16_t topButtonY, int16_t topButtonWidth, int16_t topButtonHeight) {
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

/*
 * Draw a button panel if they exist
 */
void MenuTop::drawPanelButtons(Adafruit_GFX *tft, int16_t panelX, int16_t panelY, int16_t panelWidth, int16_t panelHeight) {
  if (buttons) {
    int16_t buttonWidth = panelWidth / buttonsX;
    int16_t buttonHeight = panelHeight / buttonsY;

    auto button = buttons->begin();
    for (; button != buttons->end(); button++) {
      int16_t positionX = (*button)->getPositionX();
      int16_t positionY = (*button)->getPositionY();
      Serial.print("   button->name:");
      Serial.println((*button)->getName());
      Serial.print("   button->x");
      Serial.println(positionX);
      Serial.print("   button->y");
      Serial.println(positionY);
      int16_t buttonTopLeftX = positionX * buttonWidth + panelX;
      int16_t buttonTopLeftY = positionY * buttonHeight + panelY;

      (*button)->draw(tft, buttonTopLeftX, buttonTopLeftY, buttonWidth, buttonHeight);


    }

  }
}

bool MenuTop::setActiveMenuButton(vector<MenuButton*> *mButtons, MenuButton *activeButton) {
  bool buttonFound = false;

  auto button = mButtons->begin();
  for (; button != mButtons->end(); button++) {
    if ((*button) == activeButton) {
      (*button)->setActive();
      buttonFound = true;
    }
    else {
      (*button)->setInactive();
    }
  }

  return buttonFound;
}

tuple <bool, MenuItem *> MenuTop::handleTouch(int16_t pressX, int16_t pressY, int16_t panelX, int16_t panelY, int16_t panelWidth, int16_t panelHeight) {

  MenuItem *buttonPressed = nullptr;
//  *buttonPressed = nullptr;
  bool newButtonPress = false;

  if (buttons) {
    int16_t buttonWidth = panelWidth / buttonsX;
    int16_t buttonHeight = panelHeight / buttonsY;

    auto button = buttons->begin();
    for (; button != buttons->end(); button++) {
      int16_t positionX = (*button)->getPositionX();
      int16_t positionY = (*button)->getPositionY();

      int16_t buttonMinX = positionX * buttonWidth + panelX;
      int16_t buttonMaxX = buttonMinX + buttonWidth;
      int16_t buttonMinY = positionY * buttonHeight + panelY;
      int16_t buttonMaxY = buttonMinY + buttonHeight;

      if ((buttonMinX < pressX && pressX < buttonMaxX) &&
          (buttonMinY < pressY && pressY < buttonMaxY)) {
        if (!(*button)->isActive()) {
          newButtonPress = true;
        }
        setActiveMenuButton(buttons, (*button));
        Serial.print("Button Pressed: ");
        Serial.println((*button)->getName());
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

Menu::Menu(Adafruit_GFX *tft, XPT2046_Touchscreen *ts, vector<MenuTop*> *tMenus) {
  init(tft, ts, tMenus, DEFAULT_BACKGROUND_COLOR);
}

Menu::Menu(Adafruit_GFX *tft, XPT2046_Touchscreen *ts, vector<MenuTop*> *tMenus, uint16_t bgColor) {
  init(tft, ts, tMenus, bgColor);
}

void Menu::init(Adafruit_GFX *tft, XPT2046_Touchscreen *ts, vector<MenuTop*> *tMenus, uint16_t bgColor) {
  _tft = tft;
  _ts = ts;
  backgroundColor = bgColor;
  topMenus = tMenus;

  setTopButtons(DEFAULT_TOP_BUTTONS);
  setTopHeight(DEFAULT_TOP_HEIGHT);

}

void Menu::setTopButtons(int16_t buttons) {
  topButtons = buttons;
  calculateTopButtonDimensions();
}

void Menu::setTopHeight(int16_t height) {
  topHeight = height;
  calculateTopButtonDimensions();
}

void Menu::calculateTopButtonDimensions() {
  topButtonHeight = topHeight;
  topButtonWidth = screenWidth / topButtons;
  panelX = 0;
  panelY = topHeight + 1;
  panelWidth = screenWidth;
  panelHeight = screenHeight - panelY;
}

bool Menu::setup() {
  screenHeight = _tft->height();
  screenWidth  = _tft->width();
  calculateTopButtonDimensions();
  if (screenHeight == 0 || screenWidth == 0) {
    // TFT not started.
    return false;
  }

  return true;
}


void Menu::draw() {
  _tft->fillScreen(ILI9341_BLACK);
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
  int16_t barHeight = DEFAULT_BAR_HEIGHT;
  int16_t barWidth = screenWidth - (2*DEFAULT_PADDING_X);
  int16_t barX = DEFAULT_PADDING_X;
  int16_t barY = topButtonHeight - barHeight;
  _tft->fillRect(barX, barY, barWidth, barHeight, barColor);
}

bool Menu::setActiveTopMenu(vector<MenuTop*> *tMenus, MenuTop *activeMenu) {
  bool menuFound = false;

  auto topMenu = tMenus->begin();
  for (; topMenu != tMenus->end(); topMenu++) {
    if ((*topMenu) == activeMenu) {
      (*topMenu)->setActive();
      menuFound = true;
    }
    else {
      (*topMenu)->setInactive();
    }
  }

  return menuFound;
}

tuple <bool, MenuItem *> Menu::handleTouch(int16_t pressX, int16_t pressY) {

  MenuItem *buttonPressed = nullptr;
//  *buttonPressed = nullptr;
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
          Serial.print("Top Menu Button ");
          Serial.println((*topMenu)->getName());
          setActiveTopMenu(topMenus,(*topMenu));
          buttonPressed = (*topMenu);
        }
      }
      pos++;
    }
  }
  else {
    Serial.println("Canvas");
    auto topMenu = topMenus->begin();
    for (; topMenu != topMenus->end(); topMenu++) {
      if ((*topMenu)->isActive()) {
        tie(newButtonActive, buttonPressed) = (*topMenu)->handleTouch(pressX, pressY, panelX, panelY, panelWidth, panelHeight);
      }
    }

  }
  return make_tuple(newButtonActive, buttonPressed);
}

/*********************
 * Non Class Functions
 *********************/

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
  if (buttonActive) {
    tft->fillRoundRect(topLeftX, topLeftY, width, height, DEFAULT_BUTTON_CORNER, buttonColor);
  }
  else {
    tft->drawRoundRect(topLeftX, topLeftY, width, height, DEFAULT_BUTTON_CORNER, buttonColor);
  }
}


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