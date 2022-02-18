/*
 * @file MenuUtils.h
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

#ifndef __MENUUTILS_H
#define __MENUUTILS_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>
#include <vector>
#include <tuple>

using namespace std;

/*********************
 * Generic definitions for passing functions as arguments
 *********************/

/*!
 * @brief Callback function for a Short or Long button press
 * 
 * @return bool to indicate the screen should be re-drawn
 */
typedef bool (*ButtonPressCallback)();


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
 * Non Class Functions
 *********************/

void centerText(Adafruit_GFX *tft, String text, int16_t centerX, int16_t centerY, int16_t textSize, uint16_t textColor);
uint16_t getTextWidth (Adafruit_GFX *tft, String text, int16_t textSize);

#endif