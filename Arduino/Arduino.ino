#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Encoder.h>
int pwm = 5;
int setpoint = 310;
int temp;
int ti = 1.5;
int gain = 6;
long lastTime = millis();
long timeFactor;
int percentage;
long oldPosition  = -999;
int changed = 0;
int displayInitialized = 0;
int warningInialized = 0;
Encoder myEnc(2, 3);

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup()
{
  // initialize the LCD
  lcd.begin();
  Serial.begin(9600);
  // Turn on the blacklight and print a message.
  lcd.backlight();
  lcd.print(" T3Engineering");
  lcd.setCursor(0, 1);
  lcd.print("Soldering System");
  delay(2000);
  setPwmFrequency(6, 2);
}

void loop()
{
  setpoint = getSetpoint();
  temp = getTemperature();
  updateDisplay();
  if (tipConnected()) {
    pwm = calculatePWM();
    updateDisplay();
  } else { analogWrite(6, 0); }
}

int getTemperature()
{
  analogWrite(6, 0);    //switch off heater
  delay(40);     //wait for some time (to get low pass filter in steady state)
  int adcValue = analogRead(A0); // read the input on analog pin 7:
  analogWrite(6, pwm); //switch heater back to last value
  //return adcValue;
  return round(((float) adcValue) * 0.415 + 25.0); //apply linear conversion to actual temperature
}
int getSetpoint()
{
  if (myEnc.read() >= 800) {
    myEnc.write(800);
  }
  if (myEnc.read() <= 0) {
    myEnc.write(0);
  }
  return (myEnc.read() / 2);
}
bool tipConnected() {
  if (temp > 440) {
    return false;
  } else { return true; }
}
int calculatePWM() {
  timeFactor = (millis() - lastTime) / 1000 * ti;
  return round(constrain((setpoint - temp) * gain * timeFactor, 0, 255));
  if (temp >= setpoint) {
    lastTime = millis();
  }
}
void setPwmFrequency(int pin, int divisor) {
  byte mode;
  if (pin == 5 || pin == 6 || pin == 9 || pin == 10) {
    switch (divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 64: mode = 0x03; break;
      case 256: mode = 0x04; break;
      case 1024: mode = 0x05; break;
      default: return;
    }
    if (pin == 5 || pin == 6) {
      TCCR0B = TCCR0B & 0b11111000 | mode;
    } else {
      TCCR1B = TCCR1B & 0b11111000 | mode;
    }
  } else if (pin == 3 || pin == 11) {
    switch (divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 32: mode = 0x03; break;
      case 64: mode = 0x04; break;
      case 128: mode = 0x05; break;
      case 256: mode = 0x06; break;
      case 1024: mode = 0x07; break;
      default: return;
    }
    TCCR2B = TCCR2B & 0b11111000 | mode;
  }
}
void updateDisplay() {
  if (tipConnected()) {
    if (displayInitialized == 0)
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Huidig: ");
      lcd.setCursor(0, 1);
      lcd.print("Instelling: ");
      displayInitialized = 1;
      warningInialized = 0;
    }
    lcd.setCursor(12, 0);
    lcd.print(temp);
    lcd.print(" ");
    lcd.setCursor(12, 1);
    lcd.print(setpoint);
    lcd.print(" ");
  } else {
    if (warningInialized == 0)
    {
      displayInitialized = 0;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Geen soldeertip");
      lcd.setCursor(0, 1);
      lcd.print("aangesloten...");
      warningInialized = 1;
    }
  }
}


