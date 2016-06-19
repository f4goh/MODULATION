# Arduino BF beacon library#
F4GOH Anthony f4goh@orange.fr <br>

June 2016

Use this library freely with Arduino 1.6.5

All schematics soon [here](https://hamprojects.wordpress.com/)

## Installation ##
To use the MODULATION library:  
- Go to https://github.com/f4goh/MODULATION, click the [Download ZIP](https://github.com/f4goh/MODULATION/archive/master.zip) button and save the ZIP file to a convenient location on your PC.
- Uncompress the downloaded file.  This will result in a folder containing all the files for the library, that has a name that includes the branch name, usually MODULATION-master.
- Rename the folder to  MODULATION.
- Copy the renamed folder to the Arduino sketchbook\libraries folder.

## Usage notes ##


To use the MODULATION library, the TimerOne, SoftwareSerial, DS3232RTC, Time and WIRE libraries must also be included.


```c++
#include <MODULATION.h>
#include <SoftwareSerial.h>
#include <TimerOne.h>  //https://code.google.com/archive/p/arduino-timerone/downloads
#include <DS3232RTC.h> //http://github.com/JChristensen/DS3232RTC
#include <Wire.h>
#include <Time.h>
```
