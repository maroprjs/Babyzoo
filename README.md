# Babyzoo
A Somfy Remote Control for roller shutters, build into a Babyzoo nightlight, using ESP8266 Wifi Module &amp; RFM69

<img src="/docs/turtle3.png" alt="babyzoo turtle" width="326" height="184" >

In this example Adafruit Huzzah Feather and self-made RFM69-feather on-top.

<img src="https://github.com/maroprjs/Babyzoo/blob/master/libs/BabyzooLib/docs/turtle7.png" alt="Adafruit Huzzah and RFM69" width="326" height="184"/>

 * Traditional Somfy remotes work within 400Mhz ISM band, which makes it difficult
 * to control elements from e.g. mobile phone or computer.
 *
 * ESP8266 Wifi Module embeds a whole http server functionality hence its
 * interfaces can be controlled through a WLAN enabled device with Web Browser, it is
 * a good candidate to be used as an emulator for Somfy branded remotes.
 * For that purpose it needs to control an appropriate radio module within 400Mhz band.
 *
 * In this project an RFM69 module is used, bought from some online electronic store. It
 * is controlled via SPI interface.
 
 <img src="https://github.com/maroprjs/Babyzoo/blob/master/libs/BabyzooLib/docs/schematic.png" alt="schematic" width="326" height="184"/>
 
 
 *
 *
 * Thanks to:
 *  -https://pushstack.wordpress.com/somfy-rts-protocol/
 *  -https://forum.arduino.cc/index.php?topic=208346.60
 *  -fhem cul http://culfw.de/culfw.html
 *  -https://github.com/kobuki/RFM69OOK
 *  -https://github.com/nailbuster/myWebServer
 *  -https://github.com/PaulStoffregen/Time
 *  -https://github.com/bblanchon/ArduinoJson.git version <= v5.8.0
 *  -https://github.com/pfeerick/elapsedMillis
 *  -Adafruit (https://github.com/adafruit/DHT-sensor-library)
 *  -Arduino/ESP8266 contributers
 *  -?
 
