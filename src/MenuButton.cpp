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
#include "MenuButton.h"
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
 * Constructor
 *
 * @param label       Label or name display on button
 * @param x,y         X and Y button position (not pixel position)
 * @param buttonColor Optional button color. Default is DEFAULT_BUTTON_COLOR
 * @param onShortPress Optional callback function when a short press is performed
 * @param onLongPress Optional callback function when a long press is performed
 */ 
MenuButton::MenuButton(const char* label, int16_t x, int16_t y, uint16_t buttonColor, ButtonPressCallback onShortPress, ButtonPressCallback onLongPress) {
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
  tft->fillRect(topLeftX, topLeftY, width, height, DEFAULT_BUTTON_BACKGROUND_COLOR);
  if (buttonActive) {
    tft->fillRoundRect(topLeftX, topLeftY, width, height, DEFAULT_BUTTON_CORNER, buttonColor);
  }
  else {
    tft->drawRoundRect(topLeftX, topLeftY, width, height, DEFAULT_BUTTON_CORNER, buttonColor);
  }
}
