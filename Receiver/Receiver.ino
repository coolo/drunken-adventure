/*
  Copyright 2014 Stephan Kulow
  
  Public Domain 
*/
#include <IRremote.h>

#include <RCSwitch.h>

RCSwitch mySwitch = RCSwitch();

int RECV_PIN = 11;
int LED_PIN = 13;

IRrecv irrecv(RECV_PIN);

decode_results results;

void setup() {
  Serial.begin(9600);
  Serial.println("setup");
  irrecv.enableIRIn(); // Start the receiver
  
  pinMode(LED_PIN, OUTPUT);
  pinMode(7, OUTPUT);
  
  // Transmitter is connected to Arduino Pin #8
  mySwitch.enableTransmit(8);

  digitalWrite(LED_PIN, HIGH);
  delay(100);
  digitalWrite(LED_PIN, LOW);
}

char buffer[10];

boolean powered = false;
boolean mute = false;

boolean ledson = false;
boolean lighton = false;

const long LEDsOn = 0x56D3E4F1L;
const long CornerLight = 0x842EBDB1L;
const long TVoff = 0xF50;
const long TVon = 0x750;
const long ReceiverPower = 0x540c;
const long ReceiverMute = 0x140c;
const long ReceiverVolumeUp = 0x240c;

void doSomething(long value)
{
  boolean touchedReceiver = false;
  
  sprintf(buffer, "IR %lx", value);
  Serial.println(buffer);
  
  if (value == ReceiverPower) { // power
    powered = !powered;
    touchedReceiver = true;
  }
  
  if (value == ReceiverMute) {
    mute = !mute;
    touchedReceiver = true;
  }

  if (value ==  ReceiverVolumeUp && mute) {
    mute = false;
    touchedReceiver = true;
  }
  
  if (value == LEDsOn) {
     ledson = !ledson;
     // give relay a go
     digitalWrite(7, ledson ? HIGH : LOW);
  }
  
  if (value == CornerLight) {
     lighton = !lighton;
   
     if (lighton)
       mySwitch.switchOn('b', 3, 3);
     else
       mySwitch.switchOff('b', 3, 3);
  }

  if (touchedReceiver) {
    if (powered && !mute) {
      mySwitch.switchOn('b', 3, 2);
    } else {
      mySwitch.switchOff('b', 3, 2);
    }
  }
}

void loop() {
  if (irrecv.decode(&results)) {
    digitalWrite(LED_PIN, HIGH);
     // set the cursor to column 0, line 1
    // (note: line 1 is the second row, since counting begins with 0):
    if (results.value != -1)
      doSomething(results.value);

    irrecv.resume(); // Receive the next value
    digitalWrite(LED_PIN, LOW);
  }
}

