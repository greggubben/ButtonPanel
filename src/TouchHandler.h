/*
 * @file TouchEvent.h
 * 
 * Handles detecting a Touch Event.
 */
#pragma once

#ifndef __TOUCHHANDLER_H
#define __TOUCHHANDLER_H

#include <Arduino.h>
#include <XPT2046_Touchscreen.h>

// Define minimum delays for when screen is touched
#define DEBOUNCE_DELAY 50       // Minimum amount of millis to detect a touch change
#define SHORTPRESS_DELAY 60     // Minimum amount of millis to detect a short press
#define LONGPRESS_DELAY 400     // Minimum amount of millis to detect a long press
#define SWIPE_MIN_PIXELS 40     // Minimum distance in pixels to detect a swipe action


// Contains all the Events a touch could generate
enum TouchEvent {
  EVENT_NONE = 0,         // No Events occurred
  EVENT_SHORT = 1,        // Short Press Event has occurred
  EVENT_LONG = 2,         // Long Press Event has occurred
  EVENT_SWIPE_RIGHT = 3,  // Finger traveling to Right
  EVENT_SWIPE_LEFT =4     // Finger travelling to Left
};

// Text representation of the Touch Events defined by the TouchEvent enum
static const char* TouchEventNames[] = {"None", "Short", "Long", "Swipe Right", "Swipe Left"};

// Contains all the possible States of an Event
enum EventStates {
  EVENT_STATE_NONE = 0,       // Nothing going on
  EVENT_STATE_START = 1,      // Starting an Event
  EVENT_STATE_INPROGRESS = 2, // In the middle of an Event State
  EVENT_STATE_LINGER =3,      // Conditions are still occuring even after event has fired
  EVENT_STATE_END = 4         // The Event has been determined and fired
};

// Text representation of the Touch Events defined by the TouchEvent enum
static const char* EventStateNames[] = {"None", "Start", "InProgress", "Linger", "End"};

// Used to pass the touch event to an event handler
typedef struct Event {
  TouchEvent event;
  int16_t pressX;
  int16_t pressY;
} Event;

/*!
 * @brief Function to calibrate the touch position to the display position
 * 
 * @param int16_t* pointer to the raw X position, calibrated result set by function
 * @param int16_t* pointer to the raw Y position, calibrated result set by function
 * 
 * @return calibrated results overwrites input parameters.
 */
typedef void (*CalibrateTouchCallback)(int16_t*, int16_t*);

/*!
 * @brief Function that will handle a raised event
 * 
 * @param Event* pointer to Event detected by the Touch Handler
 * 
 * @return none
 */
typedef void (*EventHandler)(Event*);


class TouchHandler {

  public:
    TouchHandler(XPT2046_Touchscreen *ts);
    bool detectEvent(CalibrateTouchCallback _calibrateTouch = nullptr);
    bool start(EventHandler _eventHandler);

  private:

    void fireEvent();

    XPT2046_Touchscreen *_ts;

    EventHandler eventHandler;    // Function that will handle raised events

    Event currentEvent;         // Information about the current event

    // Manage state of Menu Interactions
    bool wasTouched = false;      // Prevous loop touch state. Used with isTouched 
    bool debounceTouched = false; // Post Debounce Delay last touch state. Used with isTouched 
    TouchEvent touchEvent = EVENT_NONE;
    EventStates currentState = EVENT_STATE_NONE;
    int16_t lastPressX = 0;
    int16_t lastPressY = 0;
    unsigned long lastPressTime = 0;  // the last time the screen touch state changed

};

#endif