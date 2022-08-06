#define PIN_D0 18
#define PIN_D1 19

#define NOT_RED_LED 4
#define NOT_GRN_LED 5
#define NOT_BEEPER 6
#define TAMPER_SW 7
#define DOOR_OPEN 8
#define INSIDE_BUTTON 10
#define OUTSIDE_BUTTON 11
#define KEY_SWITCH 12
#define NOT_FIRE 13
#define DOOR_CONTACT A1
#define REX A2

#define STATUS_LED A0

#define BEEP_ON_TAMPER true

#define DOOR_PROPPED_TIME_MAX 10000
#define DOOR_UNLOCK_TIME 5000
#define CLEAR_FORCED_IF_DOOR_CLOSED true
#define TAMPER_AUTO_CLEAR true

#define CARD_ID_LEN 37

#define UNLOCK_TIME 50

#define DOOR_CONTACT_OPEN HIGH

// Interrupt handlers for the wiegand pins
void wiegand_one();
void wiegand_zero();
