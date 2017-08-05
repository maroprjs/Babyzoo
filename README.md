# The Babyzoo Project
A Somfy Remote Control for roller shutters, build into a Babyzoo nightlight, using ESP8266 Wifi Module &amp; RFM69

<img src="libs/BabyzooLib/docs/turtle3.png" alt="babyzoo turtle night light" width="490" height="274">

In this example Adafruit Huzzah Feather and self-made RFM69-feather on-top.

<img src="libs/BabyzooLib/docs/turtle7.png" alt="Adafruit Huzzah and RFM69" width="490" height="274">

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
 
 <img src="libs/BabyzooLib/docs/schematic.png" alt="schematic" width="480" height="360">
 
  * Some pins are free for temperature sensor, light sensor and driving a rainbow led:
  
  <img src="libs/BabyzooLib/docs/turtle1.png" alt="web interface" width="240" height="135"><img src="libs/BabyzooLib/docs/turtle4.png" alt="web interface" width="240" height="135"><img src="libs/BabyzooLib/docs/turtle5.png" alt="web interface" width="240" height="135">
   
  * The Web Interface like this:
  
  <img src="libs/BabyzooLib/docs/somfy2.png" alt="web interface" width="230" height="360"> or <img src="libs/BabyzooLib/docs/somfy.png" alt="web interface" width="230" height="360"><br>
  The html source at: https://github.com/maroprjs/Babyzoo/tree/master/libs/BabyzooLib/html
 
 * ..also controllable via apple's voice control Siri using homebridge.....the plugin can be found here: https://github.com/maroprjs/Babyzoo/tree/master/voice_controlled/siri/somfy
 
 Raspberry Pi can be used to host the NodeJs server:<br>
  <img src="libs/BabyzooLib/docs/SiriFlow.png" alt="web interface" width="480" height="360"><br> 
  
  The plugin is modified version of: https://www.npmjs.com/package/homebridge-real-fake-garage-doors
  There are also info's about homebridge installation. 
  
  
 
 
 
 
 *more pics and screen shots are here: https://github.com/maroprjs/Babyzoo/tree/master/libs/BabyzooLib/docs
 
 
 <h2> How to build? </h2>
 If Arduino IDE is used, the content from https://github.com/maroprjs/Babyzoo/tree/master/libs/BabyzooLib and https://github.com/maroprjs/Babyzoo/tree/master/deployment/RCSomfyRFM69 need to be copied into the same project folder. <br>
 Make sure following libraries are installed: <br>
 * ESP8266 board environment<br>
 * OOK library: original https://github.com/kobuki/RFM69OOK/ (original)<br>
 * 	-> but use xmas2016 branch from https://github.com/maroprjs/RFM69OOK.git (this contains adapted changes for ESP8266)<br>
 * myWebServer library and dependent from: https://github.com/nailbuster/myWebServer.git (original)<br>
 *  -> use xmas2016 branch from https://github.com/maroprjs/myWebServer.git (this contains slight changes for password handling)<br>
 *
 * elapsedMillis from:  https://github.com/pfeerick/elapsedMillis<br>
 *
 * DHT & sensor library from Adafruit:<br>
 * 					https://github.com/adafruit/DHT-sensor-library.git<br>
 * 					https://github.com/adafruit/Adafruit_Sensor.git<br>
 <br>
 Once the software is running on ESP8266 module, the html files need to be uploaded to http://<esp8266_ip_address>.browse. <br>
 
 For being able to control the shutter via Apples Homekit App, respectively Siri voice service, a NodeJs server needs to be setup with homebridge installed. Follow the instructions from here https://www.npmjs.com/package/homebridge-real-fake-garage-doors and replace index.js and config file with that one here: https://github.com/maroprjs/Babyzoo/tree/master/voice_controlled/siri/somfy
 <br>
 
  
 <h2> Recognition</h2>
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
 *  -Hombridge contributers (https://github.com/nfarina/homebridge, https://github.com/plasticrake/homebridge-real-fake-garage-doors and precessors of these projects)
 *  -?
 
