EEPROM初期化プログラム
=====================

本初期化プログラムが行う初期データは以下の通り。

+------+----------------------------+-------------------------------+
| ADDR |    Description             | Initial Value                 |
|------+----------------------------+-------------------------------|
| 0x00 | UECSID                     | 0x10,0x10,0x0C,0x00,0x00,0x0C |
| 0x06 | MAC_Address                | 0x02,0xA2,0x73,0x0C,0xXX,0xYY |
| 0x0c | Reserved(IP Address)       | 0x00,0x00,0x00,0x00           |
| 0x10 | InAirTemp_Type             | 'T'                           |
|  :   | InAirTemp_Room             | N                             |
|  :   | InAirTemp_Region           | 1                             |
|  :   | InAirTemp_Order            | 1                             |
|  :   | InAirTemp_Priority         | 15                            |
|  :   | InAirTemp_Interval         | 10                            |
|  :   | InAirTemp_Name             | 'InAirTemp',NULLs             |
| 0x30 | InAirHumid_Type            | 'H'                           |
|  :   | InAirHumid_Room            | N                             |
|  :   | InAirHumid_Region          | 1                             |
|  :   | InAirHumid_Order           | 1                             |
|  :   | InAirHumid_Priority        | 15                            |
|  :   | InAirHumid_Interval        | 10                            |
|  :   | InAirHumid_Name            | 'InAirHumid',NULLs            |
| 0x50 | InRadiation_Type           | 'R'                           |
|  :   | InRadiation_Room           | N                             |
|  :   | InRadiation_Region         | 1                             |
|  :   | InRadiation_Order          | 1                             |
|  :   | InRadiation_Priority       | 15                            |
|  :   | InRadiation_Interval       | 10                            |
|  :   | InRadiation_Name           | 'InRadiation',NULLs           |
| 0x70 | InAirCO2_Type              | 'C'                           |
|  :   | InAirCO2_Room              | N                             |
|  :   | InAirCO2_Region            | 1                             |
|  :   | InAirCO2_Order             | 1                             |
|  :   | InAirCO2_Priority          | 15                            |
|  :   | InAirCO2_Interval          | 10                            |
|  :   | InAirCO2_Name              | 'InAirCO2',NULLs              |
| 0x90 | cnd Type (dummy)           | 'c'                           |
|  :   | cnd Room                   | N                             |
|  :   | cnd Region                 | 1                             |
|  :   | cnd Order                  | 1                             |
|  :   | cnd Priority               | 29                            |
|  :   | cnd Interval               | 1                             |
|  :   | cnd Name                   | 'cnd',NULLs                   |
| 0xb0 |  EMPTY                     |                               |
+------+----------------------------+-------------------------------+


N:1から127(nodeのMACアドレスに0x7fをANDした値をNとする)
ROOM=1〜127

このプログラムは、製造時にArduino NANOに1度だけインストールして実行するプログラムである。

起動後、MACアドレスの下2桁のためにシリアル番号を聞いてくるのでそれに答える。
重複の確認は出来ないので、入力時に正確に台帳などで確認管理をする必要がある。
