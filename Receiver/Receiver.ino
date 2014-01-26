/*
  LiquidCrystal Library - Hello World
 
 Demonstrates the use a 16x2 LCD display.  The LiquidCrystal
 library works with all LCD displays that are compatible with the 
 Hitachi HD44780 driver. There are many of them out there, and you
 can usually tell them by the 16-pin interface.
 
 This sketch prints "Hello World!" to the LCD
 and shows the time.
 
  The circuit:
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * LCD R/W pin to ground
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)
 
 Library originally added 18 Apr 2008
 by David A. Mellis
 library modified 5 Jul 2009
 by Limor Fried (http://www.ladyada.net)
 example added 9 Jul 2009
 by Tom Igoe
 modified 22 Nov 2010
 by Tom Igoe
 
 This example code is in the public domain.

 http://www.arduino.cc/en/Tutorial/LiquidCrystal
 */

// include the library code:
#include <LiquidCrystal.h>

#include <IRremote.h>

#include <RCSwitch.h>

RCSwitch mySwitch = RCSwitch();

int RECV_PIN = 11;

IRrecv irrecv(RECV_PIN);

decode_results results;

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(2, 3, 4, 5, 6, 7);

void setup() {
  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("hello");
  
  Serial.begin(9600);
  irrecv.enableIRIn(); // Start the receiver
  
  pinMode(13, OUTPUT);
  
  // Transmitter is connected to Arduino Pin #8
  mySwitch.enableTransmit(8);

}

char buffer[10];
int line = 0;

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
  
  line = (line + 1) % 4;
  lcd.setCursor((line % 2) * 8, line / 2);
  sprintf(buffer, "%lx", value);
  lcd.print(buffer);
  
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
   
     if (ledson) {
       // give relay a go
     }  
  }
  
  if (value == CornerLight) {
     digitalWrite(13, HIGH);
     delay(50);
     digitalWrite(13, LOW);
     lighton = !lighton;
   
     if (lighton)
       mySwitch.switchOn('b', 3, 3);
     else
       mySwitch.switchOff('b', 3, 3);
  }
 
  if (touchedReceiver) {      
    if (powered && !mute) {
      digitalWrite(13, HIGH);
      mySwitch.switchOn('b', 3, 2);
    } else {
      mySwitch.switchOff('b', 3, 2);
      digitalWrite(13, LOW);
    }
  }
}

void loop() {
  if (irrecv.decode(&results)) {
     // set the cursor to column 0, line 1
    // (note: line 1 is the second row, since counting begins with 0):
    Serial.println(results.value, HEX);
    if (results.value != -1)
      doSomething(results.value);

    irrecv.resume(); // Receive the next value
  }
}

