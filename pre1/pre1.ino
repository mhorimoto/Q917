#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <SPI.h>
#include <Ethernet2.h>
#include <EthernetUdp2.h> // UDP library from: bjoern@cs.stanford.edu 12/30/2008

LiquidCrystal_I2C lcd(0x27,16,2);

EthernetClient client;

byte   lcdf;
uint8_t mac[6];
char *txt[4][2];  // [screen][rows][columns]
char strIP[16];
char uecsid[6], uecstext[180],linebuf[80];
IPAddress localIP,broadcastIP,subnetmaskIP,remoteIP;
EthernetUDP Udp16520; //,Udp16529;

void setup(void) {
  int i;
  char z[17];
  txt[0][0] = "UECS Simulator  ";
  txt[0][1] = "Q917 Ver:0.06   ";
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
  delay(800);
  if (Ethernet.begin(mac) == 0) {
    lcd.setCursor(0,1);
    sprintf(txt[3][1],"NFL");
  } else {
    localIP = Ethernet.localIP();
    subnetmaskIP = Ethernet.subnetMask();
    for(i=0;i<4;i++) {
      broadcastIP[i] = ~subnetmaskIP[i]|localIP[i];
    }
    sprintf(strIP,"%d.%d.%d.%d",localIP[0],localIP[1],localIP[2],localIP[3]);
    sprintf(txt[3][1],"%s",strIP);
    lcdout(3,0,1,1);
    Udp16520.begin(16520);
  }
  delay(1000);
  lcd.clear();
  pinMode(A0,INPUT);
  pinMode(A1,INPUT);
  pinMode(A2,INPUT);
  pinMode(A3,INPUT);
  pinMode(9,INPUT);
  lcdf = 0;
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
  const char *xmlDT PROGMEM = "<?xml version=\"1.0\"?><UECS ver=\"1.00-E10\"><DATA type=\"%s\" room=\"%d\" region=\"%d\" order=\"%d\" priority=\"%d\">%s</DATA><IP>%s</IP></UECS>";
  int a1,a2,a3,a4,a1b,a1c;
  char s1[6],s2[6],s3[6],s4[6];
  long li;
  float fl;

  if (digitalRead(9)==LOW) {
    if (lcdf==1) {
      lcdf = 0;
      lcd.clear();
    }
  } else {
    if (lcdf==0) {
      lcdf = 1;
      lcd.clear();
    }
  }
  a1 = analogRead(A0);
  li = map(a1,0,1022,-100,500);
  a1b = (int)(li/10);
  a1c = (int)(li-(a1b*10));
  a2 = analogRead(A1);
  li = map(a2,0,1022,0,100);
  a2 = (int)li;
  a3 = analogRead(A2);
  li = map(a3,0,1023,0,1300);
  a4 = analogRead(A3);

  sprintf(s1,"%3d.%01d",a1b,abs(a1c));
  sprintf(s2,"%3d",a2);
  sprintf(s3,"%4d",(int)li);
  sprintf(s4,"%4d",a4);
  if (lcdf==1) {
    lcd.setCursor(0,1);
    lcd.print("T:");
    lcd.setCursor(0,0);
    lcd.print("H:");
    lcd.setCursor(8,0);
    lcd.print("R:");
    lcd.setCursor(8,1);
    lcd.print("C:");
    lcd.setCursor(2,1);
    lcd.print(s1);
    lcd.setCursor(2,0);
    lcd.print(s2);
    lcd.setCursor(10,0);
    lcd.print(s3);
    lcd.setCursor(10,1);
    lcd.print(s4);
  } else {
    lcd.setCursor(0,0);
    lcd.print("IP Address");
    lcd.setCursor(0,1);
    lcd.print(strIP);
  }
  sprintf(s1,"%d.%01d",a1b,abs(a1c));
  sprintf(s2,"%d",a2);
  sprintf(s3,"%d",(int)li);
  sprintf(s4,"%d",a4);
  uecsSendData(0x10,xmlDT,s1);
  uecsSendData(0x30,xmlDT,s2);
  uecsSendData(0x50,xmlDT,s3);
  uecsSendData(0x70,xmlDT,s4);

  delay(1000);
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

void uecsSendData(int a,char *xmlDT,char *val) {
  byte room,region,order,priority,interval;
  int  i;
  char name[26],dname[26]; // ,val[6];
  EEPROM.get(a+0x01,room);
  EEPROM.get(a+0x02,region);
  EEPROM.get(a+0x03,order);
  EEPROM.get(a+0x05,priority);
  EEPROM.get(a+0x06,interval);
  EEPROM.get(a+0x07,name);
  for(i=0;i<25;i++) {
    dname[i] = name[i];
    if (name[i]==NULL) break;
  }
  dname[i] = NULL;
  sprintf(uecstext,xmlDT,dname,room,region,order,priority,val,strIP);
  Udp16520.beginPacket(broadcastIP,16520);
  Udp16520.write(uecstext);
  Udp16520.endPacket();
}
