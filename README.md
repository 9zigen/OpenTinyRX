OpenTinyRX
==========

RX Firmware for openLRS compatible with Expert 2G/Tiny LRS
Manual in Wiki: https://github.com/baychi/OpenTinyRX/wiki/%D0%A0%D1%83%D0%BA%D0%BE%D0%B2%D0%BE%D0%B4%D1%81%D1%82%D0%B2%D0%BE-%D0%BF%D0%BE%D0%BB%D1%8C%D0%B7%D0%BE%D0%B2%D0%B0%D1%82%D0%B5%D0%BB%D1%8F

7 sep 2013 Vesion 2 F 5

- added support WDT
- addes support original Tiny board
- minor bugfixes
- minor changing in menu

8 sep 2013 Vers 2 F5
- some bugfix in statistics;
- minor correction in main constants
- menu corrections (deleted P2-P4 for beacon). All powers calculated form P1
- Add servo 150% strech mode (reg N3 - servo number (1-12), if needed).

10 sep 2013

- Added analog RSSI averaging. Reg 40 in 1-255 value is averaging factor
- Reg 4 chaged with reg3 for original Expert compatibility

15 sep 2013 (version F6)
- Added noise level measuring from menu
- added noise level output without recieving
- minor constant corrections

6 nov 2013 (version F8)
- Added SBUS out mode
- Added autobinding to transmitter
- Added discrete outs
- Added 11 bit/10 ch mode
- Added autoreset to defaults in 1-st start with error settings

15 nov 2013 (version F8)
- Bugfix in rebind function. Void deadloop.

29 nov 2013 (version F9)
- Added Futaba mode channel coding (880-1520-2160 mks);
- Added RSSI (ch15) & noise (ch14) in Sbus packet;
- some bugfixes;

19 dec 13 (version F10)
- bugfix PWM width - remove adition 18 mks per impulse
- noise level now save in statistisc even without signal
- show version in menu header

24 dec 13 (version 10 - not changed)
- bugfix in statistics.
