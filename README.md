# 3D printed LED clock

How does it work?
Inner circle shows hours. Outer circle shows minutes, but there are only 12 LEDs. 
Therefore, for minutes not ending in 0 or 5, there are two adjecent LEDs turned on with various colors, which you can use to identify the exact minute.
After restart and also at midnight,  there is a Stargate inspired effect.
The time zone and daylight saving time is hardcoded in the code - change it according to your needs.

BOM:
ESP8266 board (such as Wemos D1 Mini)
24 LEDs from a WS2812B strip

How to build:
First LED is in the outer ring at 6 o'clock, then it goes clockwise around the outer ring.
From 5 o'clock in the outer ring, wrap around the gap to 5 o'clock of the inner ring and go counter-clockwise. The last LED is 6 o'clock of the inner ring. 

Possible improvements:
It's difficult to identify the exact minute, that can be further improved.
The STL can also be improved with some see-through elements, to easily identify the hour and minute.
By using a different time library, it should be also possible to automate the daylight saving time usage.