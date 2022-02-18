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
#include "MenuUtils.h"
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
 * Non Class Functions
 *********************/



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


//
// Function to return the width of a string
//
uint16_t getTextWidth (Adafruit_GFX *tft, String text, int16_t textSize) {
  tft->setTextSize(textSize);
  int16_t stringX, stringY;
  uint16_t stringW, stringH;
  tft->getTextBounds(text, 0, 0, &stringX, &stringY, &stringW, &stringH);

  return stringW;
}