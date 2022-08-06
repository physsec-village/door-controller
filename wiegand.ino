#include "wiegand-anycard.h"

#define DEBUG

volatile char wiegand_buf[37];
volatile int wiegand_index = 0;

char *correct_card_id = "0010100010011001001100000000101100011";

// Timers
int unlocked = 0;
int green_led = 0;
int door_propped_time = DOOR_PROPPED_TIME_MAX;

bool door_opened_legitly = false; // Set if the unlock timer expired but the door was opened while it was unlocked
bool door_forced = false;
bool door_propped = false;
bool tamper = false;

void setup(){
  #ifdef DEBUG
  Serial.begin(9600);
  Serial.println("Hi!!");
  #endif

  // Initiate pins
  pinMode(NOT_GRN_LED, OUTPUT);
  digitalWrite(NOT_GRN_LED, HIGH);
  pinMode(NOT_RED_LED, OUTPUT);
  digitalWrite(NOT_RED_LED, HIGH);
  pinMode(NOT_BEEPER, OUTPUT);
  digitalWrite(NOT_BEEPER, HIGH);
  pinMode(DOOR_OPEN, OUTPUT);
  pinMode(PIN_D0, INPUT);
  pinMode(PIN_D1, INPUT);
  pinMode(REX, INPUT);
  pinMode(NOT_FIRE, INPUT_PULLUP);
  pinMode(TAMPER_SW, INPUT);
  pinMode(KEY_SWITCH, INPUT);
  pinMode(INSIDE_BUTTON, INPUT);
  pinMode(OUTSIDE_BUTTON, INPUT);
  
 // if(DOOR_CONTACT_OPEN == HIGH)
 //   pinMode(DOOR_CONTACT, INPUT);
 // else
    pinMode(DOOR_CONTACT, INPUT_PULLUP);

  // Wiegand buffer
  //wiegand_buf = malloc(CARD_ID_LEN + 1);
  //wiegand_buf[CARD_ID_LEN] = '\0';

  // Wiegand interrupts
  attachInterrupt(digitalPinToInterrupt(PIN_D0), wiegand_zero, FALLING);
  attachInterrupt(digitalPinToInterrupt(PIN_D1), wiegand_one, FALLING);
}

void loop(){
  #ifdef DEBUG
    //Serial.print("legit ");
    //Serial.println(door_opened_legitly);
    //Serial.print("digitalRead ");
    //Serial.println(digitalRead(DOOR_CONTACT));
    //Serial.print("forced ");
    //Serial.println(door_forced);
  #endif
  // Handle card reads
  if(wiegand_index >= CARD_ID_LEN){ // If we have all the bits
    char *card_id = wiegand_buf;
    #ifdef DEBUG
      Serial.println(card_id);
      Serial.println(unlocked);
    #endif
    for(int i = 0; i < 26; i++) Serial.print(wiegand_buf[i]);
    Serial.println(" ");
    if(strncmp(card_id, correct_card_id, CARD_ID_LEN) == 0){ // Compare to the correct id and act accordingly
      green_led = UNLOCK_TIME;
      unlocked = UNLOCK_TIME;
      //digitalWrite(DOOR_OPEN, HIGH); // Unlock
      PORTB |= 0b00000001;
      delay(1000);
      //digitalWrite(DOOR_OPEN, LOW); // Lock
      PORTB &= 0b11111110;
      door_opened_legitly = true;
      #ifdef DEBUG
        Serial.println("Yay");
      #endif
    }else{
      #ifdef DEBUG
        Serial.println("No");
      #endif
      //green_led = 0;
      unlocked = 0;
    }
    wiegand_index = 0; // Clear the wiegand buffer since we're done with it
  }
/*
  // Handle REX
  if(digitalRead(REX) == HIGH){ // If we have REX
    unlocked = UNLOCK_TIME; // Unlock
    green_led = UNLOCK_TIME;
  }

  // Handle key switch + outside button
  if(digitalRead(KEY_SWITCH) == HIGH && digitalRead(OUTSIDE_BUTTON) == HIGH){ // If key switch and outside button is activated
    unlocked = UNLOCK_TIME; // Unlock
    green_led = UNLOCK_TIME;
  }

  // Handle inside button
  if(digitalRead(INSIDE_BUTTON) == HIGH){ // If inside button is activated
    unlocked = UNLOCK_TIME; // Unlock
    green_led = UNLOCK_TIME;
  }
*/
  // Handle fire
    if(digitalRead(NOT_FIRE) == LOW){ // If we have the fire signal
      Serial.println("Fire");
    unlocked = -1; // Lock the door open
    green_led = -1;
  }else if(unlocked == -1){ // Otherwise reset the variable if we messed with it
      Serial.println("Not fire");
    unlocked = 0;
    green_led = 0;
  }
/*
  // Handle tamper
  if(digitalRead(TAMPER_SW) == HIGH){ // If the tamper signal is high
    tamper = true; // Set the flag
  }else{ // If it is low
    if(TAMPER_AUTO_CLEAR) tamper = false; // Clear the flag if we want to
  }
  */
  // Handle door unlock
  if(unlocked > 0 || unlocked == -1){
    digitalWrite(DOOR_OPEN, HIGH); // Unlock
    //Serial.println("Open");
    if(unlocked > 0) unlocked--; // Tick down timer
    delay(100);
    if(unlocked == 0) if(!door_forced && digitalRead(DOOR_CONTACT) == DOOR_CONTACT_OPEN) door_opened_legitly = true; // And record that the door was opened legitly if it was opened
  }else{ // If we need to lock it,
    digitalWrite(DOOR_OPEN, LOW); // Do so
    //Serial.println("Close");
  }
/*
  // Handle green LED
  if(green_led != 0){
    #ifdef DEBUG
      //Serial.println("Green LED");
    #endif
    digitalWrite(NOT_GRN_LED, LOW); // Unlock
    if(green_led > 0) green_led--; // Tick down timer
    if(green_led == 0){ // If we need to lock it,
      Serial.println("Not green LED");
      digitalWrite(NOT_GRN_LED, HIGH); // Do so
    }
  }
*/
  // Check the door contact sensor
  if(digitalRead(DOOR_CONTACT) == DOOR_CONTACT_OPEN){ // If the door is currently open,
    door_propped_time--; // Decrement the max door prop timer
    if(!door_opened_legitly && unlocked == 0) door_forced = true; // If the door wasn't held open after the unlock time expired, and if it isn't currently unlocked, it has to have been forced, set the flag
  }else{ // If it isn't,
    door_propped_time = DOOR_PROPPED_TIME_MAX; // Reset the max door prop timer
    door_opened_legitly = false; // Clear the "door held open after unlock timer ran out" flag
    if(CLEAR_FORCED_IF_DOOR_CLOSED) door_forced = false; // Clear the door forced flag if we want to
  }

  // Check if door is propped
  if(door_propped_time <= 0){
    door_propped = true;
  }else{
    door_propped = false;
  }

  // Check alarms
  if(door_forced){
    digitalWrite(NOT_BEEPER, LOW);
    digitalWrite(NOT_RED_LED, LOW);
  }else if(!door_propped && !tamper && unlocked <= 0){
    digitalWrite(NOT_BEEPER, HIGH);
    digitalWrite(NOT_RED_LED, HIGH);
  }
  if(door_propped){
    digitalWrite(NOT_BEEPER, LOW);
    digitalWrite(NOT_GRN_LED, LOW);
    digitalWrite(NOT_RED_LED, LOW);
  }else if(!door_forced && !tamper && unlocked <= 0 && green_led == 0){
    digitalWrite(NOT_BEEPER, HIGH);
    digitalWrite(NOT_GRN_LED, HIGH);
    digitalWrite(NOT_RED_LED, HIGH);
  }
  if(tamper && false){
    digitalWrite(NOT_BEEPER, LOW);
    digitalWrite(NOT_RED_LED, LOW);
  }else if(!door_forced && !door_propped && unlocked <= 0){
    digitalWrite(NOT_BEEPER, HIGH);
    digitalWrite(NOT_RED_LED, HIGH);
  }
  
}

// Interrput service routines for wiegand

void wiegand_zero(){
  #ifdef DEBUG
    //Serial.print("0");
  #endif
  if(wiegand_index >= CARD_ID_LEN) return; // Protect against buffer overflow
  wiegand_buf[wiegand_index] = '0';
  wiegand_index++;
}

void wiegand_one(){
  #ifdef DEBUG
    //Serial.print("1");
  #endif
  if(wiegand_index >= CARD_ID_LEN) return; // Protect against buffer overflow
  wiegand_buf[wiegand_index] = '1';
  wiegand_index++;
}
