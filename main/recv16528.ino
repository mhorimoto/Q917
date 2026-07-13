#undef LEN_UECS_BUFFER
#define LEN_UECS_BUFFER 48
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
    char tmp[5]; // ★ 4桁の16進数に対応するため 5バイトに拡張
    int base_addr;
    uint16_t order_val;
    unsigned char romd[16];
    const char *EEPROMIMG PROGMEM = "%03X=%02X";
    //  extern void clrM252(int);
    extern char val[];
    
    packetSize = Udp16528.parsePacket();
    if (packetSize>0) {
        IPAddress src = Udp16528.remoteIP();     // 送信元IP
        uint16_t srcPort = Udp16528.remotePort();// 送信元ポート
        int readSize = Udp16528.read(uecsbuf,LEN_UECS_BUFFER-1);
        uecsbuf[readSize] = NULL;
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
        if (val[0] == 'W' && readSize >= 14) {
            // 1. ベースアドレス (例: "10")
            tmp[0] = val[1]; tmp[1] = val[2]; tmp[2] = '\0';
            base_addr = strtol(tmp, NULL, 16);

            // 2. Room (オフセット +0x01)
            tmp[0] = val[3]; tmp[1] = val[4]; tmp[2] = '\0';
            EEPROM.update(base_addr + 0x01, strtol(tmp, NULL, 16));

            // 3. Region (オフセット +0x02)
            tmp[0] = val[5]; tmp[1] = val[6]; tmp[2] = '\0';
            EEPROM.update(base_addr + 0x02, strtol(tmp, NULL, 16));

            // 4. Order (オフセット +0x03 から 2バイト分)
            // 4桁の16進数（例: "012C"）を数値化
            tmp[0] = val[7]; tmp[1] = val[8]; tmp[2] = val[9]; tmp[3] = val[10]; tmp[4] = '\0';
            order_val = (uint16_t)strtol(tmp, NULL, 16);
            
            // Big-Endian または ArduinoのEEPROMの配置に合わせて2バイトに分解して書き込み
            // get/putの挙動に合わせ、下位バイトを+0x03、上位バイトを+0x04 に保存
            EEPROM.update(base_addr + 0x03, lowByte(order_val));
            EEPROM.update(base_addr + 0x04, highByte(order_val));

            // 5. Priority (オフセット +0x05)
            tmp[0] = val[11]; tmp[1] = val[12]; tmp[2] = '\0';
            EEPROM.update(base_addr + 0x05, strtol(tmp, NULL, 16));

            // 6. Interval (オフセット +0x06)
            tmp[0] = val[13]; tmp[1] = val[14]; tmp[2] = '\0';
            EEPROM.update(base_addr + 0x06, strtol(tmp, NULL, 16));

            // 7. CCM TYPE 文字列の書き込み (Orderが2桁増えたため、15文字目からスタート)
            int strIdx = 15;
            int eepromIdx = base_addr + 0x07;
            
            while (val[strIdx] != '\0' && strIdx < readSize && (eepromIdx - base_addr) < 32) {
                EEPROM.update(eepromIdx, val[strIdx]);
                strIdx++;
                eepromIdx++;
            }
            EEPROM.update(eepromIdx, '\0');

            sendUdp16528("OK: UECS PARAM UPDATED", src, Udp16528.remotePort());
        }
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
            sendUdp16528(val,src,srcPort);
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
            sendUdp16528(lbuf,src,srcPort);
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
            sendUdp16528(lbuf,src,srcPort);
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
                sendUdp16528(lbuf,src,srcPort);
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
            sendUdp16528(version_info,src,srcPort);
        }
    }
}

void sendUdp16528(char *msg, IPAddress ipaddr, uint16_t port) {
    Udp16528.beginPacket(ipaddr, port);
    Udp16528.write(msg);
    Udp16528.endPacket();
}
