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
#include "MenuPage.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>
#include <vector>
#include <tuple>

using namespace std;

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
MenuPage::MenuPage(const char* label, int16_t buttonsAcross, int16_t buttonsDown, uint16_t menuColor, ButtonPressCallback onShortPress, ButtonPressCallback onLongPress) {
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
MenuPage::MenuPage(const char* label, int16_t buttonsAcross, int16_t buttonsDown, vector<MenuButton*> *menuButtons, uint16_t menuColor, ButtonPressCallback onShortPress, ButtonPressCallback onLongPress) {
  init(label, buttonsAcross, buttonsDown, menuButtons, menuColor, onShortPress, onLongPress);
}


//
// Initialize the Menu Top variables.
//
void MenuPage::init(const char* label, int16_t buttonsAcross, int16_t buttonsDown, vector<MenuButton*> *menuButtons, uint16_t menuColor, ButtonPressCallback onShortPress, ButtonPressCallback onLongPress) {
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
  if (drawPanel) {
    drawPanel(tft, panelX, panelY, panelWidth, panelHeight);
  }

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
