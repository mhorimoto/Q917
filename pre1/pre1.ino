#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <SPI.h>
#include <Ethernet2.h>

LiquidCrystal_I2C lcd(0x27,16,2);

EthernetClient client;

uint8_t mac[6];
char *txt[4][2];  // [screen][rows][columns]


void setup(void) {
  int i;
  char z[17];
  txt[0][0] = "UECS Simulator  ";
  txt[0][1] = "Q917 Ver:0.02   ";
  txt[1][0] = "DEN-NOH01       ";
  txt[1][1] = "                ";
  txt[2][0] = "MAC Address     ";
  txt[2][1] = "IP Address      ";
  txt[3][0] = "0000.0000.0000  ";
  txt[3][1] = "000.000.000.000 ";
  lcd.init();
  lcd.backlight();
  lcdout(0,0,1,1);
  delay(1500);
  sprintf(txt[1][1],"ID:%02X%02X%02X%02X%02X%02X",
	  EEPROM.read(0),EEPROM.read(1),EEPROM.read(2),
	  EEPROM.read(3),EEPROM.read(4),EEPROM.read(5));
  lcdout(1,0,1,1);
  delay(1500);
  lcdout(2,0,1,1);
  delay(500);
  for (i=0;i<6;i++) {
    mac[i] = EEPROM.read(i+6);
  }
  Ethernet.init(10);// CS pin 10
  sprintf(txt[3][0],"%02X%02X.%02X%02X.%02X%02X",
	  mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
  lcdout(3,0,1,1);
  delay(1000);
  if (Ethernet.begin(mac) == 0) {
    lcd.setCursor(0,1);
    
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("T:");
  lcd.setCursor(0,0);
  lcd.print("H:");
  lcd.setCursor(8,0);
  lcd.print("I:");
  lcd.setCursor(8,1);
  lcd.print("C:");
  pinMode(A0,INPUT);
  pinMode(A1,INPUT);
  pinMode(A2,INPUT);
  pinMode(A3,INPUT);
  // start the Ethernet connection:
    //Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
  //  }
  // print your local IP address:
  //  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    //    Serial.print(Ethernet.localIP()[thisByte], DEC);
    //    Serial.print(".");
    //  }
    //  Serial.println();
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

void lcdout(int m,int l1,int l2,int cl) {
  if (cl!=0) {
    lcd.clear();
  }
  lcd.setCursor(0,0);
  lcd.print(txt[m][l1]);
  lcd.setCursor(0,1);
  lcd.print(txt[m][l2]);
}

