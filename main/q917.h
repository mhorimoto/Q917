#ifndef _Q917_H_
#define _Q917_H_
#define _Q917_H_V 100

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <SPI.h>
#include <Ethernet2.h>
#include <EthernetUdp2.h> // UDP library from: bjoern@cs.stanford.edu 12/30/2008
#include <avr/pgmspace.h>

/*** COMMON ***/

#define TRUE      1
#define FALSE     0
#define ERROR     -1
#define NO        0
#define UNMATCH   0
#define IGNORE    1
#define CHANGE_MAKE  2
#define CHANGE_BREAK 3

/*** Define for Switch ***/

#define SW_SELECT   2   // D2
#define SW_AUTOMODE 3   // D3
#define SW1         4   // D4
#define SW2         5   // D5
#define SW3         6   // D6
#define SW4         7   // D7

/*** Ethernet2 Ports ***/
#define NICSS    10     // CS pin 10

/*** LCD I2C Address ***/
#define LCDADDR  0x27

/*** LCD ***/
#define LCD_ROWS 2
#define LCD_COLS 16

/*** UECS ***/
#define UECS_PORT 16520
#define UECS_VER  "1.00-E10"
#define UECS_XML  "<?xml version=\"1.0\"?><UECS ver=\"%s\"><DATA type=\"%s\" room=\"%d\" region=\"%d\" order=\"%d\" priority=\"%d\">%s</DATA><IP>%s</IP></UECS>"
#define UECS_MAX  180   // Max length of UECS XML
#define UECS_NAME 26    // Max length of UECS Name
#define UECS_VAL   6    // Max length of UECS Value
#define UECS_IP   16    // Max length of IP Address
