# Talking LCD Clock - Adafruit Feather M4 Express
A 'talking' clock using the Adafruit Feather M4 Express and other components. 
## Background
This project started as a digital alarm clock using an Adafruit Feather M0 Express. I started with the example sketch for the 7 segment LED display and then added an alarm sequence playing the sound  via the inbuilt DAC and a small mono amplifier. The system was further enhanced with a routine that automatically adjusted the clock for daylight savings time.
https://github.com/Gambalunga/LCD-Alarm-Clock-Adafruit-Feather-M0-Express

I then further developed the concept using an Adafruit Feather M4 Express and an Adalogger Featherwing which not only has a real time clock but also an SD card reader.

My mother in law is unfortunately loosing her sight. She can see just enough to still manage herself but virtually does not cook any more and can't see well enough to read - thank goodness for audio books.

I asked her if a talking clock would be useful and she thought it would be. She decided that red would be the best colour for the LED display and the push button on the top to have it tell the time. She also decided that 12, not 24 hour time would suit her best. 

I recorded the numbers 1 to 59 and the words "O'clock" and "The time is" with Audacity and saved them in WAV format to the SD card. When she pushes the button on the top of the clock it says something like: The time is 5. 42" or "The time is 6. O'clock". Obviously various other styles could be used, such as "The time is quarter past (or 15 past) 5" etc. She had no need for an alarm so that part of the sketch governed by a boolean constant but I left the code in the sketch.

As the real time clock holds the actual date, as well as the time this could easily be expanded to the day of the week or the full date. The program corrects the time automatically for European daylight savings rules (or US if required). It has a back up LiPo battery (and charger) that will power it for a day in the case of a power failure or being disconnected. The real time clock has its own back up battery that will keep it going for a long time, perhaps years, if the clock stays without power, but of course no display or sound in that case. 

I also modified the DST_RTC library for European DST and it has now been updated on GitHub. See https://github.com/andydoro/DST_RTC

Having used it now for several days she is very happy with it. Above all when she wakes up on these dark mornings, or during the night, she can know what the time is. 

## Components Used:

* [Adafruit Feather M4 Express](https://www.adafruit.com/product/3857)
* [Adalogger FeatherWing - RTC + SD](https://www.adafruit.com/product/2922)
* [CR1220 12mm Diameter - 3V Lithium Coin Cell Battery](https://www.adafruit.com/product/380) (for the RTC)
* [4-Digit 7-Segment Display FeatherWing](https://www.adafruit.com/product/3106)
* [Mono 2.5W Class D Audio Amplifier - PAM8302](https://www.adafruit.com/product/2130)
* [Mono Enclosed Speaker - 3W 4 Ohm](https://www.adafruit.com/product/4445)
* [Stacking Headers for Feather - 12-pin and 16-pin female headers](https://www.adafruit.com/product/2830)(3 sets)
* [Header Kit for Feather - 12-pin and 16-pin Female Header Set](https://www.adafruit.com/product/2886) (2 sets)
* 2 pushbuttons (12 mm threaded with nuts for mounting in the case) 
* 1 LED pushbutton (12mm -  3 - 5V  LED - push for talking time)
* 1 LED latching pushbutton (12mm  -  3 - 5V  LED – up for alarm mode if used)
* a 1000 mAh Lithium battery to keep the clock running for a period in case of a power outage (if it loses power the alarm time would reset to the default time)
* A short Micro USB extension with a 90° downwards connection to lead out the back of the box and provide power
 * Various jumper wires mainly female female which were cut in half to solder to the pushbuttons. 

## Assembly

<p align="center">
  <img src="https://github.com/Gambalunga/Talking-LCD-Clock-Adafruit-Feather-M4-Express/blob/main/Images/Talking%20LCD%20Clock%20M4.jpg">
</p>

Because I required connection points for various cables on the underside of the Feather I reversed the normal assembly by having the female headers on the underside of the components. 

I started by assembling the 7 segment display Featherwing with female headers on the underside instead of the supplied male headers. The Adaloger and the Feather M4 Express were also assembled with the stacking headers inserted from the underside. To provide power and ground for various items I soldered a row of 3 female headers to the additional 3v points and a row of 3 male pins (cut from those supplied) to the additional Gnd points (both the 3v and the Gnd points are at the sides of the prototyping area.

<p align="center">
  <img src="https://github.com/Gambalunga/Talking-LCD-Clock-Adafruit-Feather-M4-Express/blob/main/Images/20201104_095929.jpg">
</p>

The assembly of the Mono PAM8302 Audio Amplifier was done with a row of 5 stacking headers inserted from the top side so that the pins are on the underside as ‘normal’. The pins on the underside for Shutdown and Ground were then snipped off. This allows the amp to piggyback on the Feather in such a way as the A+ and the A- are in correspondence with the A0 and the Gnd on the Feather and the Vin on the amp is in correspondence with the 3V on the Feather. From the female header on the amp jumpers were run from the Ground (remember the pin on the underside was cut off) to the A- to provide the Gnd connection and from the Shutdown to pin 12 on the Feather.
Note that the potentiometer on the amp will probably need to be adjusted to give clear sound as the amp tends to overpower the small speaker.

<p align="center">
  <img src="https://github.com/Gambalunga/Talking-LCD-Clock-Adafruit-Feather-M4-Express/blob/main/Images/20201104_095329.jpg">
</p>

I used jumper wires cut in half to solder to the various buttons. I used female jumpers to connect to the Gnd pins previously soldered to the underside of the Feather. In the case where more Gnd connections where required I could have soldered more pins to the prototyping section on the Feather and connected them to Gnd. The LEDs are powered from the female headers previously soldered to the 3V on the underside of the Feather. 

If the Alarm function is used an LED latching push button is require. When the button is released (up) the alarm time shows for a few seconds and the alarm time can be adjusted forwards or backwards using the advance and retard buttons. After holding an advance or retard button down for 15 seconds the change in time speeds up. If the Alarm button is down (Alarm off) the time can be adjusted but this should be largely unnecessary. The exact time and date should be set with one of the RTC example sketches. 

In this sketch I have not applied a routine to “talk” the alarm time however this could be implemented with a recorded voice such as “The alarm is set for”  followed playing the hors and minutes that the alarm has been set for. The voice should play when entering Alarm mode and when either the advance or retard button is released.

There is also a sketch that is used to accurately adjust the time and calibrate the PCF8523 RTC.

In the end the assembly was installed into a cut and drilled plastic case.

<p align="center">
  <img src="https://github.com/Gambalunga/Talking-LCD-Clock-Adafruit-Feather-M4-Express/blob/main/Images/20201104_175348.jpg">
</p>
