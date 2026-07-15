#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <SPI.h>
#include <Ethernet2.h>
#include <EthernetUdp2.h> // UDP library from: bjoern@cs.stanford.edu 12/30/2008
#include "q917.h"

LiquidCrystal_I2C lcd(0x27,16,2);

EthernetClient client;

byte   lcdf;
uint8_t mac[6];
//uint8_t ip[] = {192,168,35,2};   // 先頭を0にするとDHCPで決定する。
uint8_t ip[] = {0,168,35,2};   // 先頭を0にするとDHCPで決定する。
uint8_t nmask[]  = {255,255,255,0};
uint8_t nullip[] = {0,0,0,0};
char *txt[4][2];  // [screen][rows][columns]
char strIP[16];
char uecsid[6], uecstext[180],linebuf[80],val[16];
IPAddress localIP,broadcastIP,subnetmaskIP,remoteIP;
EthernetUDP Udp16520,Udp16528;
int period1sec,period10sec,period60sec;
// --- 自動停止のための変数 ---
int lastADCValues[4] = {0, 0, 0, 0}; // A0, A1, A2, A3 の前回の値を保持
unsigned long lastChangeTime = 0;    // 最後に値が変化した時間（ミリ秒）
bool isSendingSuspended = false;     // 送信停止フラグ
unsigned long suspendTimeoutMs = 60000; // ★追加：無入力判定時間（デフォルト1分）
int displayMode = 0;                 // 0: 通常表示, 1: プログラム名/Ver, 2: IP/MAC
int lastSwState = HIGH;              // 前回のスイッチ状態
unsigned long lastSwTime = 0;        // チャタリング防止用タイマー
// --------------------

void setup(void) {
    int i;
    char z[17];
    txt[0][0] = "UECS Simulator  ";
    txt[0][1] = "Q917B Ver:2.16  ";
    txt[1][0] = "DATA DRIVEN     ";
    txt[1][1] = "AGRICULTURE     ";
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
    delay(500);
    if (ip[0]==0) {
        if (Ethernet.begin(mac) == 0) {
            lcd.setCursor(0,1);
            sprintf(txt[3][1],"NFL");
        }
    } else {
        Ethernet.begin(mac,ip,nullip,nullip,nmask);
    }
    localIP = Ethernet.localIP();
    subnetmaskIP = Ethernet.subnetMask();
    for(i=0;i<4;i++) {
        broadcastIP[i] = ~subnetmaskIP[i]|localIP[i];
    }
    sprintf(strIP,"%d.%d.%d.%d",localIP[0],localIP[1],localIP[2],localIP[3]);
    sprintf(txt[3][1],"%s",strIP);
    // ---- Version Info のEEPROM書き込み（16バイトまで） ----
    char *verInfo = txt[0][1];
    for (i = 0; i < 16; i++) {
        EEPROM.update(VERSION_INFO + i, *(verInfo+i));
    }
    // -------------------------------------------------------
    lcdout(3,0,1,1);
    Udp16520.begin(16520);
    Udp16528.begin(16528);
    delay(1000);
    lcd.clear();
    pinMode(A0,INPUT);
    pinMode(A1,INPUT);
    pinMode(A2,INPUT);
    pinMode(A3,INPUT);
    pinMode(9,INPUT);
    lcdf = 0;
    period1sec  = 0;
    period10sec = 0;
    period60sec = 0;
    // --- 自動停止のための初期化 ---
    lastADCValues[0] = analogRead(A0);
    lastADCValues[1] = analogRead(A1);
    lastADCValues[2] = analogRead(A2);
    lastADCValues[3] = analogRead(A3);
    lastChangeTime = millis();
    isSendingSuspended = false;
    // EEPROM(0x0F0)からデモモード設定値を読み出し、判定時間を決定する
    uint8_t demoVal = EEPROM.read(DEMO_MODE_ADDR);
    if (demoVal == 0x00 || demoVal == 0xFF) {
        suspendTimeoutMs = 0; // 無入力判定は行わない（無効）
    } else {
        suspendTimeoutMs = (unsigned long)demoVal * 60000UL; // 分をミリ秒に変換
    }
    // -------------------------
    pinMode(SW_SELECT, INPUT_PULLUP);
    lastSwState = digitalRead(SW_SELECT);
    // ----------------------------------------------------
}

void loop(void) {
    int a1,a2,a3,a4,a1b,a1c,swState;
    char s1[6],s2[6],s3[6],s4[6];
    long li;
    float fl;
    unsigned long msec;
    static unsigned long p1msec,p10msec,p60msec;
    extern void recv16528port(void);

    swState = digitalRead(SW_SELECT);
    msec = millis();
    // スイッチがLOW（押された）かつ、前回から50ms以上経過（チャタリング対策）
    if (swState == LOW && lastSwState == HIGH && (msec - lastSwTime > 50)) {
        displayMode++;
        if (displayMode > 2) {
            displayMode = 0; // 元の表示に戻す
        }
        
        lcd.clear();
        lastSwTime = msec;
    }
    lastSwState = swState;
// モードに応じたLCD表示制御
    if (displayMode == 1) {
        // 1回目LOW: プログラム名称とバージョンを表示 (txt[0] を利用)
        lcd.setCursor(0, 0);
        lcd.print(txt[0][0]); // "UECS Simulator  "
        lcd.setCursor(0, 1);
        lcd.print(txt[0][1]); // "Q917B Ver:2.13  "
    } 
    else if (displayMode == 2) {
        // 2回目LOW: MACアドレスとIPアドレスを表示 (txt[3] を利用)
        lcd.setCursor(0, 0);
        lcd.print(txt[3][0]); // "0002.XXXX... (MACアドレス)"
        lcd.setCursor(0, 1);
        lcd.print(txt[3][1]); // "192.168... (IPアドレス)"
    } else {
        displayMode == 0;
    }

    recv16528port();
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
    a1 = analogRead(A0);               // 温度
    li = map(a1,0,1022,-100,500);
    a1b = (int)(li/10);
    a1c = (int)(li-(a1b*10));
    a2 = analogRead(A1);               // 湿度
    li = map(a2,0,1022,0,100);
    a2 = (int)li;
    a3 = analogRead(A2);               // 照度
    li = map(a3,0,1023,0,1300);
    a3 = (int)li;
    a4 = analogRead(A3);               // CO2
    li = map(a4,0,1023,200,2000);
    a4 = (int)li;

// --- 4個のADC監視とフラグ制御 ---
    int currentADCValues[4] = {a1, a2, a3, a4};
    bool anyChanged = false;

    for (int i = 0; i < 4; i++) {
        // アナログ入力の微小なノイズ（±1の揺らぎ）を無視するため、2以上の変化で判定
        if (abs(currentADCValues[i] - lastADCValues[i]) >= 5) {
            lastADCValues[i] = currentADCValues[i];
            anyChanged = true;
        }
    }

    msec = millis();
    if (anyChanged) {
        lastChangeTime = msec; // いずれかが動いたらタイマーをリセット
        if (isSendingSuspended) {
            lcd.clear();
            isSendingSuspended = false; // 送信再開
        }
    }

    // 1分以上（60000ミリ秒）どれも変化がなければフラグを立てる
    if (suspendTimeoutMs > 0 && !isSendingSuspended && (msec - lastChangeTime >= suspendTimeoutMs)) {
        lcd.clear();
        lcd.setCursor(0,1);
        lcd.print("STANDBY");
        lcd.setCursor(0,0);
        lcd.print("DEMO MODE");
        isSendingSuspended = true; // 送信停止状態へ
    }
    // ---------------------------------------

    sprintf(s1,"%3d.%01d",a1b,abs(a1c));  // Temp
    sprintf(s2,"%3d",a2);                 // Humi
    sprintf(s3,"%4d",a3);                 // Radiation
    sprintf(s4,"%4d",a4);                 // CO2
    if (displayMode == 0) {
        if (lcdf==1) {
            if (!isSendingSuspended) {
                lcd.setCursor(0,0);
                lcd.print("T:");
                lcd.setCursor(10,1);
                lcd.print("R:");
                lcd.setCursor(10,0);
                lcd.print("H:");
                lcd.setCursor(0,1);
                lcd.print("C:");
                lcd.setCursor(2,0);
                lcd.print(s1);
                lcd.setCursor(12,0);
                lcd.print(s2);
                lcd.setCursor(12,1);
                lcd.print(s3);
                lcd.setCursor(2,1);
                lcd.print(s4);
            }
        } else {
            lcd.setCursor(0,0);
            lcd.print("IP Address");
            lcd.setCursor(0,1);
            lcd.print(strIP);
        }
    }
    sprintf(s1,"%d.%01d",a1b,abs(a1c));
    sprintf(s2,"%d",a2);
    sprintf(s3,"%d",a3);
    sprintf(s4,"%d",a4);
    msec = millis();
    if ((msec-p1msec)>=1000) {
        period1sec = 1;
        p1msec = msec;
    }
    if ((msec-p10msec)>=10000) {
        period10sec = 1;
        p10msec = msec;
    }
    if ((msec-p60msec)>=60000) {
        period60sec = 1;
        p60msec = msec;
    }
    if (period1sec==1) UserEvery1Sec(s1,s2,s3,s4);
    if (period10sec==1) UserEvery10Sec();
    if (period60sec==1) UserEveryMinute();
}

void UserEvery1Sec(char s1[],char s2[],char s3[],char s4[]) {
    period1sec = 2;
    if (!isSendingSuspended) {
        uecsSendData(0x10,s1);
        delay(30);
        uecsSendData(0x30,s2);
        delay(30);
        uecsSendData(0x50,s3);
        delay(30);
        uecsSendData(0x70,s4);
        delay(30);
    }
    uecsSendData(0x90,"0");
    delay(30);
    period1sec = 0;
}

void UserEvery10Sec() {
    period10sec = 2;
    period10sec = 0;
}

void UserEveryMinute() {
    period60sec = 2;
    period60sec = 0;
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

void uecsSendData(int a,char *val) {
    const char *xmlDT PROGMEM = "<?xml version=\"1.0\"?><UECS ver=\"1.00-E10\"><DATA type=\"%s\" room=\"%d\" region=\"%d\" order=\"%d\" priority=\"%d\">%s</DATA><IP>%s</IP></UECS>";
    byte room,region,priority,interval;
    uint16_t order;
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
