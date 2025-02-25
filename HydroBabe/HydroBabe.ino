#include <EEPROM.h>            // Libraries of the program
#include <LiquidCrystal_I2C.h> //https://github.com/fdebrabander/Arduino-LiquidCrystal-I2C-library
#include <Countimer.h>         //https://github.com/inflop/Countimer
#include "DFRobot_PH.h"
#include <Keypad.h>

LiquidCrystal_I2C lcd(0x27, 20, 4); // Declare LCD and its location in memory

Countimer pumpTimer; // Declare the timer

DFRobot_PH waterpH; // Declare the pH variable

const byte relay1 = 10, relay2 = 11, relay3 = 12, relay4 = 13;

const byte ROWS = 4; // Declarations for the keypad to function
const byte COLS = 4;

char keypadKeys[ROWS][COLS] = { // Comment
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};

byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2};

Keypad timerKeypad = Keypad(makeKeymap(keypadKeys), rowPins, colPins, ROWS, COLS);

float pHVolt, pHValue, waterTemp = 25; // Various Variables
int timeS, timeM, timeH, defTimeS = 0, defTimeM = 0, defTimeH = 0, setTime = 0, timing = false;

void setup()
{
  Serial.begin(115200);

  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);

  waterpH.begin(); // Prepare the pH variable for actions

  lcd.init();
  lcd.clear();
  lcd.backlight();
  lcd.print("     Welcome To     ");
  lcd.setCursor(0, 1);
  lcd.print("    Water  Timer    ");

  eeprom_read();

  timeS = defTimeS;
  timeM = defTimeM;
  timeH = defTimeH;

  pumpTimer.setCounter(timeH, timeM, timeS, pumpTimer.COUNT_DOWN, timerComplete);
  pumpTimer.setInterval(minusTime, 1000);
  delay(1000);

  lcd.clear();
}

void timerComplete()
{
  Serial.print("Done");
  lcd.setCursor(0, 1);
  lcd.print("       Done!!!      ");

  digitalWrite(relay1, HIGH);  // Relay IN1 Off
  digitalWrite(relay2, LOW); // Relay IN1 Off
  digitalWrite(relay3, HIGH);  // Relay IN1 Off
  digitalWrite(relay4, HIGH); // Relay IN1 Off

  delay(3000);
  timing = false;
}

void minusTime()
{
  timeS = timeS - 1;
  if (timeS < 0)
  {
    timeS = 59;
    timeM = timeM - 1;
  }
  if (timeM < 0)
  {
    timeM = 59;
    timeH = timeH - 1;
  }

  Serial.print(timeH);
  Serial.print(":");
  Serial.print(timeM);
  Serial.print(":");
  Serial.print(timeS);
  Serial.print("\n");

  lcd.setCursor(6, 1);
  if (timeH <= 9)
  {
    lcd.print("0");
  }
  lcd.print(timeH);
  lcd.print(":");
  if (timeM <= 9)
  {
    lcd.print("0");
  }
  lcd.print(timeM);
  lcd.print(":");
  if (timeS <= 9)
  {
    lcd.print("0");
  }
  lcd.print(timeS);
  lcd.print("   ");
}

void loop()
{
  digitalWrite(relay1, HIGH);  // Relay IN1 On
  digitalWrite(relay2, HIGH);  // Relay IN2 On
  digitalWrite(relay3, HIGH); // Relay IN3 On
  digitalWrite(relay4, HIGH); // Relay IN4 On

  pumpTimer.run(); // Prepare the timer for actions

  char key = timerKeypad.getKey(); // Variable that listens for keypad input
  if (key && !timing)
  {
    switch (key)
    {
    case '*': // Start the timer!
      timing = true;
      Serial.print("Start Timer\n");
      // setTime = 4;
      timeS = defTimeS;
      timeM = defTimeM;
      timeH = defTimeH;

      pumpTimer.setCounter(timeH, timeM, timeS);
      pumpTimer.start();
      break;

    case '#': // Backspace the current inputted time
      Serial.print("BackSpace\n");
      switch (setTime)
      {
      case 1:
        defTimeS = defTimeS / 10;
        break;
      case 2:
        defTimeM = defTimeM / 10;
        break;
      case 3:
        defTimeH = defTimeH / 10;
        break;
      }
      break;

    case 'A':
      setTime = 1;
      Serial.print("Set Timer Seconds\n");

      break;

    case 'B':
      setTime = 2;
      Serial.print("Set Timer Minutes\n");

      break;

    case 'C':
      setTime = 3;
      Serial.print("Set Timer Hours\n");

      break;

    default:
      Serial.print(String(key));
      switch (setTime)
      {
      case 1:
        defTimeS = timeCheck(defTimeS, 59, key);
        break;
      case 2:
        defTimeM = timeCheck(defTimeM, 59, key);
        break;
      case 3:
        defTimeH = timeCheck(defTimeH, 99, key);
        break;
      }
    }
    eeprom_write();
  }

  else if (timing)
  {
    if (key == '*')
    {
      pumpTimer.stop();
      lcd.setCursor(0, 1);
      lcd.print("   Timer stopped.   ");
      delay(3000);
      timing = false;
    }
  }

  lcd.setCursor(0, 0);
  if (setTime == 0)
  {
    lcd.print("       Timer:       ");
    Serial.print("Timer:\n");
  }
  if (setTime == 1)
  {
    lcd.print("    Set Timer SS    ");
    Serial.print("Seconds = ");
    Serial.print(defTimeS);
    Serial.print("\n");
  }
  if (setTime == 2)
  {
    lcd.print("    Set Timer MM    ");
    Serial.print("Minutes = ");
    Serial.print(defTimeM);
    Serial.print("\n");
  }
  if (setTime == 3)
  {
    lcd.print("    Set Timer HH    ");
    Serial.print("Hours = ");
    Serial.print(defTimeH);
    Serial.print("\n");
  }
  /*
  if (setTime == 4)
  {
    Serial.print(timeH);
    Serial.print(":");
    Serial.print(timeM);
    Serial.print(":");
    Serial.print(timeS);
    Serial.print("\n");
  }
  */

  if (!timing)
  {
    lcd.setCursor(0, 1);
    lcd.print("      ");
    if (defTimeH <= 9)
    {
      lcd.print("0");
    }
    lcd.print(defTimeH);
    lcd.print(":");
    if (defTimeM <= 9)
    {
      lcd.print("0");
    }
    lcd.print(defTimeM);
    lcd.print(":");
    if (defTimeS <= 9)
    {
      lcd.print("0");
    }
    lcd.print(defTimeS);
    lcd.print("      ");
  }
}

int timeCheck(int timeValue, int limit, char num)
{
  timeValue = timeValue * 10 + String(num).toInt();
  if (timeValue > limit)
  {
    timeValue = limit;
  }

  return timeValue;
}

void eeprom_write()
{
  EEPROM.write(1, defTimeS);
  EEPROM.write(2, defTimeM);
  EEPROM.write(3, defTimeH);
}

void eeprom_read()
{
  defTimeS = EEPROM.read(1);
  defTimeM = EEPROM.read(2);
  defTimeH = EEPROM.read(3);
}