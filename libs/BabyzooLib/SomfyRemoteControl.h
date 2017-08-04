/*
 * SomfyRemoteControl.h
 *
 *  Created on: Nov 8, 2016
 *      Author: user
 */

#ifndef SOMFYREMOTECONTROL_H_
#define SOMFYREMOTECONTROL_H_

#include <Arduino.h>
#include <vector>
#ifdef CC1100
	#include "CC1100.h"
#else
	#include <RFM69OOK.h>
	#include <SPI.h>
	#include <RFM69OOKregisters.h>
#endif

namespace MRO {

class SomfyRemoteControl {
public:
    enum t_status {
      k_waiting_synchro,
      k_receiving_data,
      k_complete
    };

    enum SomfyCmd
    {
    	SomfyCmd_None       = 0,
        SomfyCmd_My 		= 0x1,	//My		Stop or move to favourite position
    	SomfyCmd_Up 		= 0x2,	//Up		Move up
    	SomfyCmd_MyUp 		= 0x3,	//My + Up	Set upper motor limit in initial programming mode
    	SomfyCmd_Down 		= 0x4,	//Down		Move down
    	SomfyCmd_MyDown 	= 0x5,	//My + Down	Set lower motor limit in initial programming mode
    	SomfyCmd_UpDown 	= 0x6,	//Up + Down	Change motor limit and initial programming mode
    	SomfyCmd_Prog1   	= 0x8,	//Prog		Used for (de-)registering remotes, see below
    	SomfyCmd_SunFlag 	= 0x9,	//Sun + Flag	Enable sun and wind detector (SUN and FLAG symbol on the Telis Soliris RC)
    	SomfyCmd_Flag	 	= 0xA,	//Flag		Disable sun detector (FLAG symbol on the Telis Soliris RC)
    	SomfyCmd_Prog2	 	= 0xB,	//programming (long pulse)
	   	SomfyCmd_Unknown	= 0xC	//


    };
    const String somfyCmdStr[12] = { "none", "my", "up", "my+up","down","my+down","up+down","prog","sun+flag","flag","prog2","unknown"};


    enum t_pulseTimingSet:uint8_t {
      k_forum,
      k_cul,
      k_thomas
    };


    typedef struct
    {
    	//uint8_t numOfSomfyChannels;
    	t_pulseTimingSet pulseTimingSet;
    	byte txPower;//power level 0-31 (coresponds to -18 to +13 dBm)
    	uint16_t freqMhz_x100;
    	uint8_t commandRepetitions;//default 2; worse rf conditions may require more

    } somfyStatusRecord_t;

    struct SomfyChannel
    {
    	uint8_t idx;
    	//uint32_t name;
		uint32_t address;//24bits in use
		uint16_t rolling_code;
    } ;
    typedef SomfyChannel somfyChannel_t;

    typedef struct
    {
    	byte rawData[7];
    	String cmd;
    	//SomfyChannel somfyChannel;
		uint32_t address;
		unsigned long rolling_code;//actually 16bits is enough
    } datagram_t;

    //power level 0-31 (coresponds to -18 to +13 dBm)
	SomfyRemoteControl(uint8_t txPin = 4, uint8_t rstPin = 5, float freqMhz = 433.42, byte powerLevel = 5);
	virtual ~SomfyRemoteControl();
	t_status pulse(word p);
    bool decode();
    bool transmit(byte cmd, uint32_t address, uint16_t rolling_code, byte first);
    bool cmd(String inChar);//blade 'up', 'down', 'stop', 'prog', etc////called after activateSomfyChannel() function
    void receiveBegin();
    void receiveEnd();
    t_status receive(volatile word &pulseLength);
    t_status receive();
    void begin(void (*function)());
    void begin();
    datagram_t getReceivedData();
    void setPulseTiming(t_pulseTimingSet pulseTimingSet);
    void setPulseTimingForum();
    void setPulseTimingCUL();
    void setPulseTimingTh();
    void setRemoteCtrlAddress(uint32_t addr = 0xABCDEF);
    void setRollingCode(uint16_t rc = 0);
    void setRssiThr(int8 rssiThr = 0);
    void setOokFixedThr(uint8_t ookFixThr = 30);
    uint8_t getOokFixedThr();
    word getPulseCount();
    void setTxPower(byte pwr);//power level 0-31 (coresponds to -18 to +13 dBm)
    void setFrequency(float freqMhz);
    void setCmdRepetitions(uint8_t rep = 2);
    t_status getCurrentReceiveStatus();
    byte getCurrentTemperature();
    void addSomfyChannel(somfyChannel_t channel);
    void updateSomfyChannel(somfyChannel_t channel);
    void activateSomfyChannel(uint8_t idx);//called prior cmd() function
    uint16_t getCurrentRollingCode();
protected:



	// in Microseconds
	word _k_tempo_wakeup_pulse ;
	word _k_tempo_wakeup_silence ;
	word _k_tempo_synchro_hw ;
	word _k_tempo_synchro_hw_min ;
	word _k_tempo_synchro_hw_max ;
	word _k_tempo_synchro_sw ;
	word _k_tempo_synchro_sw_min ;
	word _k_tempo_synchro_sw_max ;
	word _k_tempo_half_symbol;
	word _k_tempo_half_symbol_min ;
	word _k_tempo_half_symbol_max ;
	word _k_tempo_symbol ;
	word _k_tempo_symbol_min ;
	word _k_tempo_symbol_max ;
	word _k_tempo_inter_frame_gap ;
	uint8_t  _txPin;
	uint8_t  _rstPin;
    t_status _status;
    byte _cpt_synchro_hw;
    byte _cpt_bits;
    byte _previous_bit;
    bool _waiting_half_symbol;
    byte _payload[7];
    void delayMicros(uint32_t d);
    RFM69OOK * _rf;
    //uint32_t _remoteCtrlAddress;//24bits used
    //uint16_t _rollingCode;
    std::vector <somfyChannel_t> _somfyChannelList;
    uint8_t _activeChannelIdx;
    word _pulseLength;
    word _pulseCount;
    t_status _currentReceiveStatus;
    datagram_t _receivedData;
    uint8_t _ookFixedThresh;
    int8 _rssiThresh;
    uint8_t _cmdRepetitions;
};

} /* namespace MRO */

#endif /* SOMFYREMOTECONTROL_H_ */
