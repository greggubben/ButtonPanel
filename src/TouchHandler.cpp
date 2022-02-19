/*
 * @file TouchEvent.h
 * 
 * Handles detecting a Touch Event.
 */
#include <Arduino.h>
#include <XPT2046_Touchscreen.h>
#include "TouchHandler.h"

// Uncomment the following define to debug the screen Event Handling
//#define TOUCH_HANDLE_DEBUG

//
// Initialize the Touch Handler
//
TouchHandler::TouchHandler(XPT2046_Touchscreen *ts) {
  _ts = ts;
};

//
// Set up the Touch Handler by initializing variables
// The TFT must be started before this is called
//
bool TouchHandler::start(EventHandler _eventHandler) {

  eventHandler = _eventHandler;
  wasTouched = false;
  debounceTouched = false;
  touchEvent = EVENT_NONE;
  lastPressTime = millis();

  return true;
};


/*
 * Send an Event to the registered Event Handler
 */
void TouchHandler::fireEvent() {
    #ifdef TOUCH_HANDLE_DEBUG
    Serial.print("Fire Event ");
    Serial.print(TouchEventNames[touchEvent]);
    Serial.print(" at (");
    Serial.print(lastPressX);
    Serial.print(" , ");
    Serial.print(lastPressY);
    Serial.println(")");
    #endif

    Event currentEvent;
    currentEvent.event = touchEvent;
    currentEvent.pressX = lastPressX;
    currentEvent.pressY = lastPressY;
    (*eventHandler)(&currentEvent);
}


/*
 * Handle any touch screen actions
 */
bool TouchHandler::detectEvent(CalibrateTouchCallback _calibrateTouch) {
    TS_Point point;
    int16_t swipeX;
    int16_t swipeY;

  #ifdef TOUCH_HANDLE_DEBUG
  EventStates lastState = currentState;
  #endif

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
      #ifdef TOUCH_HANDLE_DEBUG
      Serial.println("Touch Change");
      #endif
      debounceTouched = isTouched;

      // New press
      if (isTouched) {
        #ifdef TOUCH_HANDLE_DEBUG
        Serial.println("New Touch");
        #endif
        // On first press - Don't which of the 4 modes yet
        touchEvent = EVENT_NONE;
        currentState = EVENT_STATE_START;

        // Find the calibrated X and Y of the press
        point = _ts->getPoint();
        lastPressX = point.x;
        lastPressY = point.y;
        if (_calibrateTouch) {
          (*_calibrateTouch)(&lastPressX, &lastPressY);
        }

      }
      else {
        #ifdef TOUCH_HANDLE_DEBUG
        Serial.println("New Stop Touch");
        #endif
        // Stopped touching
        currentState = EVENT_STATE_END;
      }
    }
  }

  #ifdef TOUCH_HANDLE_DEBUG
  if (lastState != currentState) {
      Serial.print("State Change from ");
      Serial.print(EventStateNames[lastState]);
      Serial.print(" to ");
      Serial.println(EventStateNames[currentState]);
      lastState = currentState;
  }
  #endif


  switch(currentState) {
    case EVENT_STATE_START:
        // First touch - set things up for new event
        currentState = EVENT_STATE_INPROGRESS;
        touchEvent = EVENT_SHORT;   // Default to a short press
        break;

    case EVENT_STATE_INPROGRESS:
        // event is happening - not fired yet

        // See if the finger moved indicating a swipe
        point = _ts->getPoint();
        swipeX = point.x;
        swipeY = point.y;
        if (_calibrateTouch) {
            (*_calibrateTouch)(&swipeX, &swipeY);
        }

        // Check for a swipe first
//        if ((abs(swipeX - lastPressX) > SWIPE_MIN_PIXELS) ||
//            (abs(swipeY - lastPressY) > SWIPE_MIN_PIXELS)) {
        if (abs(swipeX - lastPressX) > SWIPE_MIN_PIXELS) {
            // Finger moved so a swipe is happening
            if (swipeX > lastPressX) {
                touchEvent = EVENT_SWIPE_RIGHT;
            }
            else if (swipeX < lastPressX) {
                touchEvent = EVENT_SWIPE_LEFT;
            }
            /*
            else if (swipeY > lastPressY) {
                touchEvent = EVENT_SWIPE_DOWN;
            }
            else if (swipeY < lastPressY) {
                touchEvent = EVENT_SWIPE_UP;
            }
            */
            fireEvent();
            currentState = EVENT_STATE_LINGER;
        }
        // Check for a long press
        else if ((touchEvent == EVENT_SHORT) && ((millis() - lastPressTime) > LONGPRESS_DELAY)) {
            // New Long press occurred
            touchEvent = EVENT_LONG;
            fireEvent();
            currentState = EVENT_STATE_LINGER;
        }
        // Check for a short press
        else if ((touchEvent == EVENT_NONE) && ((millis() - lastPressTime) > SHORTPRESS_DELAY)) {
            // New Long press occurred
            touchEvent = EVENT_SHORT;
            // Only ending the touch will fire a short press event
        }
        break;

    case EVENT_STATE_END:
        // touch stopped - fire event if short press
        if (touchEvent == EVENT_SHORT) {
            fireEvent();
        }
        currentState = EVENT_STATE_NONE;
        lastPressX = 0;
        lastPressY = 0;
        break;

    case EVENT_STATE_NONE:
    case EVENT_STATE_LINGER:
    default:
        break;
  }

  #ifdef TOUCH_HANDLE_DEBUG
  if (lastState != currentState) {
        Serial.print("State Change from ");
        Serial.print(EventStateNames[lastState]);
        Serial.print(" to ");
        Serial.println(EventStateNames[currentState]);
  }
  #endif

  // Keep track of last touch state so changes can be identified.
  wasTouched = isTouched;

  // Provide current state of the screen being touched
  return isTouched;
}
