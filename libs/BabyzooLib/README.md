# Babyzoo
A Somfy Remote Control for roller shutters, build into a Babyzoo nightlight, using ESP8266 &amp; RFM69


 * This project aims to control Somfy branded window roller shutters via ESP8266 Wifi Modul.
 *
 * Traditional Somfy remotes work within 400Mhz ISM band, which makes it difficult
 * to control elements from e.g. mobile phone or computer.
 *
 * Since ESP8266 Wifi Module can implement a whole http server functionality hence its
 * interfaces can be controlled through a WLAN enabled device with Web Browser, it is
 * a good candidate to be used as an emulator for Somfy branded remotes.
 * For that purpose it needs to control an appropriate radio module within 400Mhz band.
 *
 * In this project an RFM69 module is used, bought from some online electronic store. It
 * is controlled via SPI interface.
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
 
