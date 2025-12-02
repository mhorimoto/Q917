#define  LEN_UECS_BUFFER 8
#define   VERSION_INFO     0x3f0  // Version Info (16 bytes limit)
/////////////////////////////////
// Reset Function goto Address 0
/////////////////////////////////
void(*resetFunc)(void) = 0;

const char RULE1[80] PROGMEM = " ADDR | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F |0123456789ABCDEF";
const char RULE2[80] PROGMEM = "------+-------------------------------------------------+----------------";

void recv16528port(void) {
    //char *xmlDT PROGMEM = CCMFMT;
    char uecsbuf[LEN_UECS_BUFFER];
    int packetSize,i,iaddr,idata;
    char eaddr[4],edata[3];
    unsigned char romd[16];
    const char *EEPROMIMG PROGMEM = "%03X=%02X";
    //  extern void clrM252(int);
    extern char val[];
    
    packetSize = Udp16528.parsePacket();
    if (packetSize>0) {
        IPAddress src = Udp16528.remoteIP();     // 送信元IP
        uint16_t srcPort = Udp16528.remotePort();// 送信元ポート
        Udp16528.read(uecsbuf,LEN_UECS_BUFFER-1);
        uecsbuf[packetSize] = NULL;
        for(i=0;i<LEN_UECS_BUFFER;i++) {
            if (uecsbuf[i]<(char)0x20) {
                val[i] = (char)NULL;
            } else {
                val[i] = uecsbuf[i];
            }
        }
        if (!strcmp(val,"R77")) {
            resetFunc();
        }
        //    if (!strcmp(val,"R71")) {
        //      clrM252(1);
        //    }
        //    if (!strcmp(val,"R72")) {
        //      digitalWrite(9,LOW);
        //      delay(50);
        //      digitalWrite(9,HIGH);
        //    }
        if (val[0]=='S') {
            for(i=0;i<3;i++) {
                eaddr[i]=val[i+1];
            }
            eaddr[i] = (char)NULL;
            for(i=0;i<2;i++) {
                edata[i]=val[i+4];
            }
            edata[i]=(char)NULL;
            iaddr = strtol(eaddr, NULL, 16);
            idata = strtol(edata, NULL, 16);
            EEPROM.update(iaddr,idata);
            sprintf(val,EEPROMIMG,iaddr,idata);
            sendUdp16528(val,src,16528);
        }
        if (val[0]=='D') {
            char lbuf[81];
            int startptr,addh,addl;
            for (int i = 0; i < 80; i++) {
                uint8_t c = pgm_read_byte(&RULE1[i]);      // PROGMEM から1バイト読む
                lbuf[i] = c;
                if (c == 0) {
                    lbuf[i] = '\n';
                    lbuf[i+1] = 0;
                    break;                           // 終端に達したら終了
                }
            }
            lbuf[80] = 0; // Null terminate
            sendUdp16528(lbuf,src,16528);
            for (int i = 0; i < 80; i++) {
                uint8_t c = pgm_read_byte(&RULE2[i]);      // PROGMEM から1バイト読む
                lbuf[i] = c;
                if (c == 0) {
                    lbuf[i] = '\n';
                    lbuf[i+1] = 0;
                    break;                           // 終端に達したら終了
                }
            }
            lbuf[80] = 0; // Null terminate
            sendUdp16528(lbuf,src,16528);
            eaddr[0]=val[1];
            eaddr[1] = (char)NULL;
            iaddr = strtol(eaddr, NULL, 16);
            iaddr *= 0x100;
            startptr = iaddr;
            for(addh=startptr;addh<(startptr+0x100);addh+=0x10) {
                for(addl=0;addl<0x10;addl++) {
                    romd[addl]=EEPROM.read(addh+addl);
                }
                sprintf(lbuf," %04X | %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X |%1c%1c%1c%1c%1c%1c%1c%1c%1c%1c%1c%1c%1c%1c%1c%1c\n",
	            addh,romd[0],romd[1],romd[2],romd[3],romd[4],romd[5],romd[6],romd[7],
	            romd[8],romd[9],romd[10],romd[11],romd[12],romd[13],romd[14],romd[15],
	            bytemap(romd[0]),bytemap(romd[1]),bytemap(romd[2]),bytemap(romd[3]),
	            bytemap(romd[4]),bytemap(romd[5]),bytemap(romd[6]),bytemap(romd[7]),
	            bytemap(romd[8]),bytemap(romd[9]),bytemap(romd[10]),bytemap(romd[11]),
	            bytemap(romd[12]),bytemap(romd[13]),bytemap(romd[14]),bytemap(romd[15]));
                sendUdp16528(lbuf,src,16528);
            }
        }
        if (val[0]=='V') {
            char version_info[17];
            for (i = 0; i < 16; i++) {
                version_info[i] = EEPROM.read(VERSION_INFO + i);
                if (version_info[i] == 0) {
                    break;
                }
            }
            version_info[i] = 0; // Null terminate
            sendUdp16528(version_info,src,16528);
        }
    }
}

void sendUdp16528(char *msg, IPAddress ipaddr, uint16_t port) {
    Udp16528.beginPacket(ipaddr, port);
    Udp16528.write(msg);
    Udp16528.endPacket();
}
