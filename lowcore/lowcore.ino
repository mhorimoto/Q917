///////////////////////////////////////////////////////
// -*- mode : C++ -*-
// EEPROM editor for Q917
//
//[概要]
//  DEN農01で使用するQ917のEEPROM領域の
//                           初期化を行うプログラム
//
///////////////////////////////////////////////////////

#include <stdio.h>
#include <EEPROM.h>

#define SELEB  9

void setup(void) {
  unsigned int i,j,r,a,u;
  byte t,room;
  char c,name[8],z[4];
  byte uecsid[6]={0x10,0x10,0x0c,0x00,0x00,0x0C},uecsrd[6];
  byte macadd[6]={0x02,0xa2,0x73,0x0C,0xff,0xff};
  byte
    data1[32] = {'T',1,1,1,0,15,10,'I','n','A','i','r','T','e','m','p',0},
    data2[32] = {'H',1,1,1,0,15,10,'I','n','A','i','r','H','u','m','i','d',0},
    data3[32] = {'R',1,1,1,0,15,10,'I','n','R','a','d','i','a','t','i','o','n',0},
    data4[32] = {'C',1,1,1,0,15,10,'I','n','A','i','r','C','O','2',0},
    data5[32] = {'c',1,1,1,0,29, 1,'c','n','d',0};
  
  Serial.begin(115200);
  Serial.println("EEPROM SETTING for DENNOH01 VER 0.05");
  pinMode(SELEB,INPUT); // SELE button
  delay(100);
  EEPROM.get(0x0,uecsrd);
  for(i=0;i<6;i++) {
    if (uecsrd[i]!=uecsid[i]) {
      r = 1;  // 書き込む
      break;
    } else {
      r = 0;  // 書き込まない
    }
  }
  if (digitalRead(SELEB)==LOW) {
    r = 1;
  }
  if (r==0) { // 書き込み済ならば表示だけを行って終了
    Serial.println("FINE DID NOT WRITE");
    Serial.println("HEXDATA:");
    for(j=0;j<11;j++) {
      sprintf(z,"0x%03X:",j*0x10);
      Serial.print(z);
      for(i=0;i<16;i++) {
	sprintf(z,"%02X",EEPROM.read(i+(j*0x10)));
	Serial.print(z);
	if (i<15) Serial.print(",");
      }
      Serial.println();
    }
    Serial.print("Are You modify this ? ");
    u = 0;
    while(u==0) {
      if (Serial.available()>0) {
	u = Serial.read();
	if (u!='y') {
	  Serial.println();
	  Serial.println("END");
	  Serial.end();
	  while(1) {
	    u = 0;    // PROGRAM END HERE
	  }
	}
      }
    }
  }
  //
  // DATA WRITE START HERE
  //
  EEPROM.put(0x0,uecsid);
  Serial.println("WRITE DONE");
  Serial.print("UECSID:");
  for(i=0;i<6;i++) {
    Serial.print(EEPROM.read(i),HEX);
    Serial.print(",");
  }
  Serial.println("");
  Serial.print("ENTER SERIAL NUMBER:");
  r = 0;
  j = 0;
  while(r!=0x0d) {
    if (Serial.available()>0) {
      r =Serial.read();
      if ((r>=0x30)&&(r<=0x39)) {
	r -= 0x30;
	Serial.print(r,DEC);
	j *= 10;
	j += r;
      }
    }
  }
  Serial.println("");
  Serial.print("DATA:");Serial.println(j,DEC);
  t = (j >> 8) & 0xff;
  macadd[4] = t;
  macadd[5] = (byte)(j & 0xff);
  Serial.print("MAC ADDRESS=");
  for(i=0;i<6;i++) {
    sprintf(z,"%02X",macadd[i]);
    Serial.print(z);
    if (i<5) Serial.print(":");
  }
  Serial.println(); 
  EEPROM.put(0x6,macadd);
  Serial.print("MAC=");
  for(i=0;i<6;i++) {
    sprintf(z,"%02X",EEPROM.read(6+i));
    Serial.print(z);
    if (i<5) Serial.print(":");
  }
  wrrec(data1,0x10);
  wrrec(data2,0x30);
  wrrec(data3,0x50);
  wrrec(data4,0x70);
  wrrec(data5,0x90);
  //  room = (macadd[5]+30)&0x7f;
  //  data[0x01] = room ; // InAirTemp
  //  data[0x12] = room ; // InAirHumid
  //  data[0x24] = room ; // InIllumi
  //  data[0x34] = room ; // cnd
  Serial.println("");
  //  EEPROM.put(0x10,data);
  Serial.println("DATA WROTE");
  
  Serial.println("HEXDATA:");
  for(j=0;j<11;j++) {
    sprintf(z,"0x%03X:",j*0x10);
    Serial.print(z);
    for(i=0;i<16;i++) {
      sprintf(z,"%02X",EEPROM.read(i+(j*0x10)));
      Serial.print(z);
      if (i<15) Serial.print(",");
    }
    Serial.println();
  }
  Serial.end();
}


void wrrec(byte d[],int a) {
  for(int i=0;i<0x20;i++) {
    EEPROM.write(a+i,d[i]);
  }
}

void loop(void) {
}
