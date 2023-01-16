#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

LiquidCrystal_I2C lcd(0x27,16,2);

void setup(void) {
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,1);
  lcd.print("T:");
  lcd.setCursor(0,0);
  lcd.print("H:");
  lcd.setCursor(8,0);
  lcd.print("I:");
  lcd.setCursor(8,1);
  lcd.print("E:");
  pinMode(A0,INPUT);
  pinMode(A1,INPUT);
  pinMode(A2,INPUT);
  pinMode(A3,INPUT);
}

void loop(void) {
  int a1,a2,a3,a4,a1b,a1c;
  char s1[6],s2[6],s3[6],s4[6];
  long li;
  float fl;

  a1 = analogRead(A0);
  lcd.setCursor(2,1);
  li = map(a1,0,1022,-100,500);
  a1b = (int)(li/10);
  a1c = (int)(li-(a1b*10));
  sprintf(s1,"%3d.%01d",a1b,abs(a1c));
  lcd.print(s1);
  a2 = analogRead(A1);
  lcd.setCursor(2,0);
  li = map(a2,0,1022,0,100);
  a2 = (int)li;
  sprintf(s2,"%3d%%",a2);
  lcd.print(s2);
  a3 = analogRead(A2);
  lcd.setCursor(10,0);
  li = map(a3,0,1023,0,1300);
  sprintf(s3,"%4d",(int)li);
  lcd.print(s3);
  a4 = analogRead(A3);
  lcd.setCursor(10,1);
  sprintf(s4,"%4d",a4);
  lcd.print(s4);
  delay(100);
}
