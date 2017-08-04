/*
 *
 * v0.0.1: 26.07.2017 prepare for git
 * 		-based on local V2 26.12.2016
 *
 * References/Libraries:
 * OOK library but with changed SPI speed and port settings: https://github.com/kobuki/RFM69OOK/
 * 	-> use xmas2016 branch from https://github.com/maroprjs/RFM69OOK.git
 * myWebServer library from: https://github.com/nailbuster/myWebServer.git
 *  -> use xmas2016 branch from https://github.com/maroprjs/myWebServer.git
 *
 * elapsedMillis from:  https://github.com/pfeerick/elapsedMillis
 *
 * DHT & sensor library from Adafruit:
 * 					https://github.com/adafruit/DHT-sensor-library.git
 * 					https://github.com/adafruit/Adafruit_Sensor.git
 *
 * Somfy protocol extracted from somfy_v2_Em.ino:
 * 						https://forum.arduino.cc/index.php?topic=208346.60
 *

 *
 * * Connect ESP12 (Adafruit Huzzah feather) to
 * SPI SCK = GPIO #14 (default)
 * SPI MOSI = GPIO #13 (default)
 * SPI MISO = GPIO #12 (default)
 * SPI CS = GPIO #16 (15 is default, but disturbs sw upload to huzzah)
 *
 * Interrupt PIN: 4(ESP) to Pin DIO2 at RFM69 for continuous mode
 *
 */


//#define DEBUG
#include "Debug.h"
#include "SomfyRemoteControl.h"
using MRO::SomfyRemoteControl;

#define MAX_NUM_OF_SOMFY_CHANNELS 10

#include <EEPROM.h>
#define EEPROM_USED_SIZE 300 //[bytes] reserved: 0-99: rolling codes ; 100-199: addresses; 200-299: status record
#define EEPROM_ROLLING_CODES_START 0
#define EEPROM_REMOTE_ADDRESSES_START 100
#define EEPROM_STATUS_RECORD_START 200

//int gInByte = 0;
//byte gCmd;
SomfyRemoteControl::t_status previousS = SomfyRemoteControl::k_waiting_synchro;
SomfyRemoteControl::t_status currentS = SomfyRemoteControl::k_waiting_synchro;
SomfyRemoteControl::datagram_t gDatagram;

#define RECEIVE_TIME 2  //in minutes
int gReceiveTime = 0;

SomfyRemoteControl gRemote;
//For local tests:
//SomfyRemoteControl::somfyChannel_t channel0 = {0,"test",0x123401,1234};//id, name, address, rolling code
//SomfyRemoteControl::somfyChannel_t channel0 = {0,0x123401,1234};//id, name, address, rolling code


SomfyRemoteControl::somfyStatusRecord_t gSomfySettings;



//for measuruements
#include "elapsedMillis.h"
elapsedMillis gTimeToMeasure = 0;


//Temperature related:
#include "DHT.h"
#define DHTTYPE DHT22
#define DHTPIN 2
DHT dht(DHTPIN, DHTTYPE);
float gTemperature = 0;
float gHumidity = 0;

#define RAINBOW 15
//int gLedState = LOW;

/*
enum e_rbRainbow:uint8_t {
  k_manually,
  k_10min,
  k_20min,
  k_30min,
  k_auto,
  k_undefined
};
*/

typedef struct
{
	int ledState;
	String rbRainbow;
	String thOn;
	String thOff;
	elapsedMillis elapsedTime;

} rainbow_t;


rainbow_t gRainbow = {LOW,"manually","0002","0020",0};

typedef struct
{
	String cbAutoblade;
	String thLight;
	String thTemp;
	String channels;
	String channel[MAX_NUM_OF_SOMFY_CHANNELS];

} autoblade_t;


autoblade_t gAutoblade = {"false","1000","99","00",{"false","false","false","false","false","false","false","false","false","false"}};




#define PHOTOCELL A0
int gPhotocellReading;

//#include "CWebInterface.h"
//using MRO::CWebInterface;
#include <myWebServer.h>

void handleRemote(){
	String reply = "{\"status\":";
	//ToDo make print macro
	String data = server.arg("mode");
	PRINT("Server Arg mode: ");PRINTLN(data);
	data = server.arg("channel");
	PRINT("Server Arg channel: ");PRINTLN(data);
	data = server.arg("cmd");
	PRINT("Server Arg cmd: ");PRINTLN(data);
	data = server.arg("timestamp");
	PRINT("Server Arg time: ");PRINTLN(data);

	int chIdx = 0xFF;
	String sensorValues = "";
	sensorValues = "Temp: " + String(gTemperature) + " ";
	sensorValues = sensorValues + "Luftf: " + String(gHumidity) + " ";
	sensorValues = sensorValues + "Licht: " + String(gPhotocellReading);


	if (server.arg("mode") == "normal"){
		chIdx = server.arg("channel").toInt()-1;//index = channelX-1
		if (chIdx < MAX_NUM_OF_SOMFY_CHANNELS)
		{
			gRemote.activateSomfyChannel(chIdx);
			gRemote.cmd(server.arg("cmd"));
			saveRollingCodeInEeprom(chIdx, gRemote.getCurrentRollingCode());
		}
		reply = reply + "\"OK_normal\"" + "}";

	}else if (server.arg("mode") == "program"){
		chIdx = server.arg("channel").toInt()-1;//index = channelX-1
		if (chIdx < MAX_NUM_OF_SOMFY_CHANNELS)
		{
			gRemote.activateSomfyChannel(chIdx);
			gRemote.cmd("prog");
			saveRollingCodeInEeprom(chIdx, gRemote.getCurrentRollingCode());
		}
		reply = reply + "\"OK_program\"" + "}";

	}else if (server.arg("mode") == "substitute"){
		if (server.arg("action") == "set") saveNewSubstitute();
		reply = reply + "\"OK_substitute\"" + "}";

	}else if (server.arg("mode") == "sensor"){

		reply = reply + "\"OK_sensor\"" + "," + "\"content\":" + "\"" + sensorValues + "\"" + "}";

	}else if (server.arg("mode") == "rainbow"){
		if (server.arg("action") == "set")toggleLED();
		if (server.arg("action") == "save")saveRainbowSettings();
		reply = reply + "\"OK_rainbow\"" + "," + "\"ledstate\":" + "\"" + String(gRainbow.ledState) + "\"" + "," + "\"rb_rainbow\":" + "\"" + gRainbow.rbRainbow + "\"" + "," + "\"th_on\":" + "\"" + gRainbow.thOn + "\"" + "," + "\"th_off\":" + "\"" + gRainbow.thOff + "\"" + "," + "\"sensors\":" + "\"" + sensorValues + "\"" + "}";

	}else if (server.arg("mode") == "autoblade"){
		if (server.arg("action") == "save")saveAutobladeSettings();
		if (server.arg("action") == "check") gAutoblade.channels=server.arg("channels");
		reply = reply + "\"OK_autoblade\"" + "," + "\"cb_autoblade\":" + "" + gAutoblade.cbAutoblade + "" + "," + "\"th_light\":" + "\"" + gAutoblade.thLight + "\"" + "," + "\"th_temp\":" + "\"" + gAutoblade.thTemp + "\""+ "," + autoBladeChannelJson() + "," + "\"sensors\":" + "\"" + sensorValues + "\"" + "}";
		//reply = reply + "\"OK_autoblade\"" + "," + "\"cb_autoblade\":" + "\"" + gAutoblade.cbAutoblade + "\"" + "," + "\"th_light\":" + "\"" + gAutoblade.thLight + "\"" + "," + "\"th_temp\":" + "\"" + gAutoblade.thTemp + "\""+ "," + "\"sensors\":" + "\"" + sensorValues + "\"" + "}";
	}else if (server.arg("mode") == "info"){
		if (server.arg("action") == "read_eeprom"){
		reply = reply + "\"OK_info\"" + "," + "\"content\":" + "\"" + readStatusRecordHumanReadable() + readChannelValuesHumanReadable() + "\"" + "}";
		};
		if (server.arg("action") == "root_info"){
				reply = reply + "\"OK_info\"" + "," + "\"content\":" + "\"" + "SW Version:"  + ZVERSION +  + "UpTime:"  + ZUP_TIME + " OOK calibration: " + readOok() + "\"" + "}";
		};

	}else if (server.arg("mode") == "receive"){
		if (server.arg("action") == "check"){
			gReceiveTime = RECEIVE_TIME;
			gRemote.receiveBegin();
			reply = reply + "\"OK_receive\"" + "," + "\"calibrate\":" + "\"" + readOok() + "\"" + "," + "\"receive\":" + "\"" + readDatagram() + "\"" + "}";
		};
		if (server.arg("action") == "set"){
			setOok(server.arg("ook").toInt());
			reply = reply + "\"OK_receive\"" + "," + "\"calibrate\":" + "\"" + readOok() + "\"" + "," + "\"receive\":" + "\"" + readDatagram() + "\"" + "}";
		};

	}else if (server.arg("mode") == "misc_settings"){
		//power
		data = server.arg("tx");
		PRINT("Server Arg tx: ");PRINTLN(data);
		gSomfySettings.txPower = (byte)data.toInt();
		gRemote.setTxPower(gSomfySettings.txPower);
		//frequency
		data = server.arg("freq");
		gSomfySettings.freqMhz_x100 = data.toInt();
		float f = (float)gSomfySettings.freqMhz_x100/100.00;
		PRINT("Server Arg freq: ");PRINTLN(f);
		//number of symbol repetitions
		data = server.arg("rep");
		PRINT("Server Arg rep: ");PRINTLN(data);
		gSomfySettings.commandRepetitions = (uint8_t)data.toInt();
		gRemote.setCmdRepetitions(gSomfySettings.commandRepetitions);
		//pulse set
		data = server.arg("pulses");
		if (data == "thomas") gSomfySettings.pulseTimingSet = SomfyRemoteControl::k_thomas;
		if (data == "cul") gSomfySettings.pulseTimingSet = SomfyRemoteControl::k_cul;
		if (data == "forum") gSomfySettings.pulseTimingSet = SomfyRemoteControl::k_forum;
		PRINT("Server Arg freq: ");PRINTLN(data);
		saveSomfyStatusRecordInEeprom();
		reply = reply + "\"OK_misc_settings\"" + "," + "\"content\":" + "\"" + readStatusRecordHumanReadable() + "\"" + "}";


	}

	//server.send(200, "text/plain", "OK" ); // Antwort an Internet Browser senden
	//server.send(200, "text/plain", readRawEeprom());
	//server.send(200, "text/plain",readStatusRecordHumanReadable() + readChannelValuesHumanReadable());
	//server.send(200, "text/plain", String(gTemperature) + "/" + String(gHumidity) + "/" + String(gPhotocellReading));
	server.send(200, "text/plain", reply );

}





void loadSomfyStatusRecord()
{
	/*when eeprom not used:
	//gSomfySettings.numOfSomfyChannels = 1;
	gSomfySettings.commandRepetitions = 2;
	gSomfySettings.pulseTimingSet = SomfyRemoteControl::k_cul;
	gSomfySettings.txPower = 5;
	gSomfySettings.freqMhz_x100 = 43342;
	*/
	getStatusRecordFromEeprom();//to global variable gSomfySettings
}

void restoreChannelValues()
{
	SomfyRemoteControl::somfyChannel_t channel;

	EEPROM.begin(EEPROM_USED_SIZE);

	for (int i = 0;i<MAX_NUM_OF_SOMFY_CHANNELS;i++)
	{
		channel.idx = i;
		//channel.address = 0x111101;
		EEPROM.get(EEPROM_REMOTE_ADDRESSES_START + i*sizeof(uint32_t),channel.address);
		//channel.rolling_code = 0;
		EEPROM.get(EEPROM_ROLLING_CODES_START + i*sizeof(uint16_t),channel.rolling_code);
		gRemote.addSomfyChannel(channel);
	}
	EEPROM.end();
}

void saveRollingCodeInEeprom(uint8_t idx, uint16_t roll_code)
{
	EEPROM.begin(EEPROM_USED_SIZE);
	EEPROM.put(EEPROM_ROLLING_CODES_START + idx*sizeof(uint16_t),roll_code);
	EEPROM.commit();
	EEPROM.end();
}

void saveAddressInEeprom(uint8_t idx, uint32_t address)
{
	EEPROM.begin(EEPROM_USED_SIZE);
	EEPROM.put(EEPROM_REMOTE_ADDRESSES_START + idx*sizeof(uint32_t),address);
	EEPROM.commit();
	EEPROM.end();
}

String readStatusRecordHumanReadable()
{
	String str = "rec: ";
	getStatusRecordFromEeprom();
	str = str + "tx:" + String(gSomfySettings.txPower) + " f:" +  String(gSomfySettings.freqMhz_x100) + " rep:" + String(gSomfySettings.commandRepetitions) + " pset:" + String(gSomfySettings.pulseTimingSet) + "///";

	return str;
}

String readChannelValuesHumanReadable()
{
	SomfyRemoteControl::somfyChannel_t channel;
	String str = "ch:";

	EEPROM.begin(EEPROM_USED_SIZE);

	for (int i = 0;i<MAX_NUM_OF_SOMFY_CHANNELS;i++)
	{
		channel.idx = i;
		//channel.address = 0x111101;
		EEPROM.get(EEPROM_REMOTE_ADDRESSES_START + i*sizeof(uint32_t),channel.address);
		//channel.rolling_code = 0;
		EEPROM.get(EEPROM_ROLLING_CODES_START + i*sizeof(uint16_t),channel.rolling_code);
		//gRemote.addSomfyChannel(channel);
		str = str + String(i) + " addr:" + String(channel.address) + " rc:" + String(channel.rolling_code) + "/ ch:";
	}
	EEPROM.end();

	return str;
}


void restoreStatusRecord()
{
	loadSomfyStatusRecord();
	gRemote.setCmdRepetitions(gSomfySettings.commandRepetitions);
	gRemote.setPulseTiming(gSomfySettings.pulseTimingSet);
	gRemote.setTxPower(gSomfySettings.txPower);
	float f = (float)gSomfySettings.freqMhz_x100/100.00;
	PRINTLN(f);
	gRemote.setFrequency(f);
	gRemote.setOokFixedThr(200);
}

void restoreSettings()
{
	restoreStatusRecord();
	restoreChannelValues();
}


void clearEeprom()
{
	EEPROM.begin(EEPROM_USED_SIZE);
	for (int i = 0 ; i < EEPROM_USED_SIZE ; i++) {
		EEPROM.write(i, 0xFF);
	}
	EEPROM.commit();
	EEPROM.end();
}

String readRawEeprom()
{
	EEPROM.begin(EEPROM_USED_SIZE);
	String str = "";
	for (int i = 0 ; i < EEPROM_USED_SIZE ; i++) {
		str = str + String(EEPROM.read(i));

	}
	EEPROM.end();
	return str;

}

void saveSomfyStatusRecordInEeprom()//comes from web interface
{
	EEPROM.begin(EEPROM_USED_SIZE);
	EEPROM.put(EEPROM_STATUS_RECORD_START, gSomfySettings);
	EEPROM.commit();
	EEPROM.end();

}

void createSomfyStatusRecordInEeprom()
{
	EEPROM.begin(EEPROM_USED_SIZE);
	//gSomfySettings.numOfSomfyChannels = 1;
	gSomfySettings.commandRepetitions = 2;
	gSomfySettings.pulseTimingSet = SomfyRemoteControl::k_thomas;
	gSomfySettings.txPower = 5;
	gSomfySettings.freqMhz_x100 = 43342;
	EEPROM.put(EEPROM_STATUS_RECORD_START, gSomfySettings);
	EEPROM.commit();
	EEPROM.end();

}

void createSomfyChannelsInEeprom()
{

	//ToDo: clean up; use address and roll_code as own type!!
	uint16_t roll_code = 0;
	uint32_t address = 0x111100;

	EEPROM.begin(EEPROM_USED_SIZE);

	for (int i = 0;i<MAX_NUM_OF_SOMFY_CHANNELS;i++)
	{
		//channel.address = 0x111101;
		EEPROM.put(EEPROM_REMOTE_ADDRESSES_START + i*sizeof(uint32_t),address);
		address++;
		//channel.rolling_code = 0;
		EEPROM.put(EEPROM_ROLLING_CODES_START + i*sizeof(uint16_t),roll_code);
	}
	EEPROM.commit();
	EEPROM.end();

}

void getStatusRecordFromEeprom()
{
	EEPROM.begin(EEPROM_USED_SIZE);
	EEPROM.get(EEPROM_STATUS_RECORD_START, gSomfySettings);
	EEPROM.end();
}

void saveNewSubstitute()
{
	SomfyRemoteControl::somfyChannel_t channel;
	channel.idx = server.arg("idx").toInt();
	channel.address = server.arg("address").toInt();
	channel.rolling_code = server.arg("roll").toInt();
	saveAddressInEeprom(channel.idx, channel.address);
	saveRollingCodeInEeprom(channel.idx, channel.rolling_code);
	gRemote.updateSomfyChannel(channel);

}

void toggleLED()
{
  if (gRainbow.ledState == LOW){gRainbow.ledState = HIGH;PRINT(" high ");}
  else {gRainbow.ledState = LOW;PRINT(" low ");}
  digitalWrite(RAINBOW, gRainbow.ledState);
  gRainbow.elapsedTime = 0;
}

void saveRainbowSettings()
{
	gRainbow.rbRainbow = server.arg("rb_rainbow");
	gRainbow.thOff = server.arg("th_off");
	gRainbow.thOn = server.arg("th_on");
	gRainbow.elapsedTime = 0;
}

String autoBladeChannelJson()
{
	String str = "";
	str = str + "\"" + "channel\":[";


	for (int i=1; i <= gAutoblade.channels.toInt(); i++)
	{
		str = str + gAutoblade.channel[i-1];
		if (i == (gAutoblade.channels.toInt())) str = str + "]";
		else str = str + ",";
	}

	return str;

}

void saveAutobladeSettings()
{
	String str = "";
	gAutoblade.cbAutoblade = server.arg("cb_autoblade");
	gAutoblade.thLight = server.arg("th_light");
	gAutoblade.thTemp = server.arg("th_temp");
	gAutoblade.channels = server.arg("channels");

	for (int i = 1; i <= gAutoblade.channels.toInt(); i++)
	{
		str = "channel" + String(i);
		gAutoblade.channel[i-1] = server.arg(str);
	}

}

void checkAutoBladeConditions()//blades go down when either light gets to bright or temperature too high
{
	if(gAutoblade.cbAutoblade == "true")
	{
		//autoblade conditions:
		if(gPhotocellReading > gAutoblade.thLight.toInt())
		{
			gAutoblade.cbAutoblade = "false"; //to make sure it happens only once
			for(int i = 0; i < gAutoblade.channels.toInt(); i++)
			{
				if (gAutoblade.channel[i] == "true"){
					if (i < MAX_NUM_OF_SOMFY_CHANNELS)
					{
						gRemote.activateSomfyChannel(i);
						gRemote.cmd("down");
						saveRollingCodeInEeprom(i, gRemote.getCurrentRollingCode());
					}

				}
			}
		}
		if (String(gTemperature) != "")
		{
			if(gTemperature > gAutoblade.thTemp.toInt())
			{
				gAutoblade.cbAutoblade = "false"; //to make sure it happens only once
				for(int i = 0; i < gAutoblade.channels.toInt(); i++)
				{
					if (gAutoblade.channel[i] == "true")
					{
						if (i < MAX_NUM_OF_SOMFY_CHANNELS)
						{
							gRemote.activateSomfyChannel(i);
							gRemote.cmd("down");
							saveRollingCodeInEeprom(i, gRemote.getCurrentRollingCode());
						}

					}
				}
			}
		}
	}
}


void checkRainbowConditions()
{
	bool thOnSmallerthOff;
	//thOnSmallerthOff = gRainbow.thOn.toInt() < gRainbow.thOff.toInt();
	//rainbow led conditions:
	if(gRainbow.rbRainbow == "manually"){
		PRINTLN("gRainbow.rbRainbow == manually");//do nothing
	//}else if ((gRainbow.rbRainbow == "auto") && (thOnSmallerthOff == true)){
	}else if (gRainbow.rbRainbow == "auto"){
		if (gPhotocellReading <= gRainbow.thOn.toInt()) gRainbow.ledState = HIGH;
		if (gPhotocellReading >= gRainbow.thOff.toInt())gRainbow.ledState = LOW;
		digitalWrite(RAINBOW, gRainbow.ledState);
	}else{//turn off in 10min, 20min or 30 min
		if(gRainbow.elapsedTime/1000/60 >= gRainbow.rbRainbow.toInt()){
			gRainbow.ledState = LOW;
			digitalWrite(RAINBOW, gRainbow.ledState);
			//gRainbow.rbRainbow = "manually";
		}
	}
    //if (gRainbow.ledState == LOW){gRainbow.ledState = HIGH;PRINT(" high ");}
    //else {gRainbow.ledState = LOW;PRINT(" low ");}
    //digitalWrite(RAINBOW, gRainbow.ledState);

}

void readSensors()
{

	gPhotocellReading = analogRead(PHOTOCELL);
	PRINT("photocell: ");
	PRINTLN(gPhotocellReading);


	PRINT("Temperature: ");
	gTemperature = dht.readTemperature();
	PRINT(gTemperature);
	PRINT("   Humidity: ");
	gHumidity = dht.readHumidity();
	PRINT(gHumidity);

	if (String(gTemperature) != "")
	{
		gTemperature = gRemote.getCurrentTemperature();
	}

}

void setOok(uint8_t ook)
{
	gRemote.setOokFixedThr(ook);
}

String readOok()
{
	return "Ook: " + String(gRemote.getOokFixedThr()) + " PulseCount: " + String(gRemote.getPulseCount());
}

String readDatagram()
{
	String str;
	str = str + "cmd: " + gDatagram.cmd;
	str = str + " addr: " + String(gDatagram.address);
	str = str + " roll: " + String(gDatagram.rolling_code) + " /";
	return str;
}

void setup()
{
	delay(STARTUP_DELAY);
	Serial.begin(115200);
	delay(100);


	MyWebServer.begin(&handleRemote);
	PRINTLN("MyWebServer initialized!");
	delay(200);


	gRemote.begin();
	restoreSettings();
	PRINTLN("Remote initialized!");
	delay(100);

	dht.begin();
	PRINTLN("Temperature Sesnor initialized!");
	delay(100);


	pinMode(RAINBOW, OUTPUT);
	digitalWrite(RAINBOW, LOW);
	PRINTLN("Rainbow LED initialized!");
	delay(100);

	pinMode(PHOTOCELL, INPUT);
	PRINTLN("Phtotcell initialized!");

	//clearEeprom();<-filled with ...111111111...
	//createSomfyStatusRecordInEeprom();<-for first time somfy
	//createSomfyChannelsInEeprom();<-for first time somfy
	restoreSettings();

}

void loop()
{

	if (gTimeToMeasure >= 60000)
	{
		gTimeToMeasure = gTimeToMeasure - 60000;
		readSensors();
		checkRainbowConditions();
		checkAutoBladeConditions();

		if (gReceiveTime > 0) gReceiveTime--;
	}



	/*This reuires that somfy cahnnel is added and activated to remote channellist!!!!
	 * ->currently this is done only through web interface
	char inCmd = (char)Serial.read();
	 if (inCmd){
		 //PRINTLN(pulseLength);
		 if (inCmd == 's') gRemote.cmd("my");
	     if (inCmd == 'u') gRemote.cmd("up");
	     if (inCmd == 'd') gRemote.cmd("down");
	     if (inCmd == 'p') gRemote.cmd("prog");
	     if (inCmd == 'f') gRemote.setPulseTimingForum();
	     if (inCmd == 'c') gRemote.setPulseTimingCUL();
	     if (inCmd == 't') gRemote.setPulseTimingTh();
	 }
	 */
	 if (gReceiveTime > 0) currentS = gRemote.receive();
	 if (currentS != previousS) {
		 PRINT("receiving state: ");PRINTLN(currentS);
		 previousS = currentS;

	 }
	 if(currentS == SomfyRemoteControl::k_complete)
	 {
		 gDatagram = gRemote.getReceivedData();
		 PRINT("Command [dec]: ");PRINTLN(gDatagram.cmd);
		 PRINT("Address [dec]: ");PRINTLN(gDatagram.address);
		 PRINT("Rolling Code[dec]: ");PRINTLN(gDatagram.rolling_code);
		 PRINTLN("************************************************");
		 currentS = SomfyRemoteControl::k_waiting_synchro;//gRemote.getCurrentReceiveStatus();
	 }

	 MyWebServer.handle();



}
