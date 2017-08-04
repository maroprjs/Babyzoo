/*
 * SomfyRemoteControl.cpp
 *
 *  Created on: Nov 8, 2016
 *      Author: user
 */

#include "SomfyRemoteControl.h"

#include "Debug.h"


namespace MRO {

volatile word pulseLength;

void countPulseLength(void) {

    static word last;
    // determine the pulse length in microseconds, for either polarity
    pulseLength = micros() - last;
    last += pulseLength;



}

//power level 0-31 (coresponds to -18 to +13 dBm)
SomfyRemoteControl::SomfyRemoteControl(uint8_t txPin,uint8_t rstPin,float freqMhz, byte powerLevel)
{
 _status = k_waiting_synchro;//used for pulse() funktion internally
 _cpt_synchro_hw = 0;
 _cpt_bits = 0;
 _previous_bit = 0;
 _txPin = txPin;//currently not needed
 _rstPin = rstPin;//currently not needed

 _currentReceiveStatus = k_waiting_synchro;
 if(!_somfyChannelList.empty())
	 _somfyChannelList.clear();
 _activeChannelIdx = 0;

 _rf = new RFM69OOK();
 _rf->initialize();
 setTxPower(powerLevel);
 setFrequency(freqMhz);
 setRssiThr();
 setOokFixedThr(); //initial value


 setPulseTimingTh();
 //setRemoteCtrlAddress();
 //setRollingCode();
 setCmdRepetitions();

 _waiting_half_symbol = false;
 _pulseLength = 0;//to measure received pulses
 //_currentReceiveStatus = k_waiting_synchro;


 for(int i=0; i<7; ++i) _payload[i] = 0;


}

void SomfyRemoteControl::setPulseTiming(t_pulseTimingSet pulseTimingSet)
{
	if (pulseTimingSet == SomfyRemoteControl::k_cul)setPulseTimingCUL();
	if (pulseTimingSet == SomfyRemoteControl::k_thomas)setPulseTimingTh();
	if (pulseTimingSet == SomfyRemoteControl::k_forum)setPulseTimingForum();
}

void SomfyRemoteControl::setPulseTimingForum(){
	 //original
	 _k_tempo_wakeup_pulse = 9415; //in [us]
	 _k_tempo_wakeup_silence = 89565; //in [us]
	 _k_tempo_synchro_hw = 2416; //in [us]
	 _k_tempo_synchro_hw_min = 2416*0.7; //in [us]
	 _k_tempo_synchro_hw_max = 2416*1.3; //in [us]
	 _k_tempo_synchro_sw = 4550; //in [us]
	 _k_tempo_synchro_sw_min = 4550*0.7; //in [us]
	 _k_tempo_synchro_sw_max = 4550*1.3; //in [us]
	 _k_tempo_half_symbol = 604; //in [us]
	 _k_tempo_half_symbol_min = 604*0.7; //in [us]
	 _k_tempo_half_symbol_max = 604*1.3; //in [us]
	 _k_tempo_symbol = 1208; //in [us]
	 _k_tempo_symbol_min = 1208*0.7; //in [us]
	 _k_tempo_symbol_max = 1208*1.3; //in [us]
	 _k_tempo_inter_frame_gap = 30415; //in [us]


}

void SomfyRemoteControl::setPulseTimingCUL()
{
//form CUL for C1100:
 _k_tempo_wakeup_pulse = 9415; //in [us]
 _k_tempo_wakeup_silence = 89565; //in [us]
 _k_tempo_synchro_hw = 2550; //in [us]
 _k_tempo_synchro_hw_min = 2550*0.7; //in [us]
 _k_tempo_synchro_hw_max = 2550*1.3; //in [us]
 _k_tempo_synchro_sw = 4860; //in [us]
 _k_tempo_synchro_sw_min = 4860*0.7; //in [us]
 _k_tempo_synchro_sw_max = 4860*1.3; //in [us]
 _k_tempo_half_symbol = 620; //in [us]
 _k_tempo_half_symbol_min = 620*0.7; //in [us]
 _k_tempo_half_symbol_max = 620*1.3; //in [us]
 _k_tempo_symbol = 1240; //in [us]
 _k_tempo_symbol_min = 1240*0.7; //in [us]
 _k_tempo_symbol_max = 1240*1.3; //in [us]
 _k_tempo_inter_frame_gap = 30415; //in [us]

}

void SomfyRemoteControl::setPulseTimingTh(){
	 //compare sdr recording of real remote
	 _k_tempo_wakeup_pulse = 10240; //in [us]
	 _k_tempo_wakeup_silence = 16640; //in [us]
	 _k_tempo_synchro_hw = 2560; //in [us]
	 _k_tempo_synchro_hw_min = 2560*0.7; //in [us]
	 _k_tempo_synchro_hw_max = 2560*1.3; //in [us]
	 _k_tempo_synchro_sw = 5120; //in [us]
	 _k_tempo_synchro_sw_min = 5120*0.7; //in [us]
	 _k_tempo_synchro_sw_max = 5120*1.3; //in [us]
	 _k_tempo_half_symbol = 640; //in [us]
	 _k_tempo_half_symbol_min = 640*0.7; //in [us]
	 _k_tempo_half_symbol_max = 640*1.3; //in [us]
	 _k_tempo_symbol = 1280; //in [us]
	 _k_tempo_symbol_min = 1280*0.7; //in [us]
	 _k_tempo_symbol_max = 1280*1.3; //in [us]
	 _k_tempo_inter_frame_gap = 26880; //in [us]


}

void SomfyRemoteControl::setCmdRepetitions(uint8_t rep)
{
	_cmdRepetitions = rep;
}

void SomfyRemoteControl::setRssiThr(int8 rssiThr)
{
	 _rssiThresh = rssiThr;

}

void SomfyRemoteControl::setOokFixedThr(uint8_t ookFixThr)
{
	 _ookFixedThresh = ookFixThr;

}

uint8_t SomfyRemoteControl::getOokFixedThr()
{
	 return _ookFixedThresh ;

}

word SomfyRemoteControl::getPulseCount()
{
	 return _pulseCount;

}

void SomfyRemoteControl::setTxPower(byte pwr)//power level 0-31 (coresponds to -18 to +13 dBm)
{
	_rf->setPowerLevel(pwr);//takes effect only after reset
	//_rf->reset();ToDo: check If and where Reset has effect!!!
}

void SomfyRemoteControl::setFrequency(float freqMhz)
{
	 _rf->setFrequencyMHz(freqMhz);
}

void SomfyRemoteControl::setRemoteCtrlAddress(uint32_t addr)
{
	_somfyChannelList[_activeChannelIdx].address = addr;
}

void SomfyRemoteControl::setRollingCode(uint16_t rc)
{
	_somfyChannelList[_activeChannelIdx].rolling_code = rc;
}




void SomfyRemoteControl::addSomfyChannel(somfyChannel_t channel){
	_somfyChannelList.push_back(channel);
}

void SomfyRemoteControl::updateSomfyChannel(somfyChannel_t channel){
	_somfyChannelList[channel.idx].address = channel.address;
	_somfyChannelList[channel.idx].rolling_code = channel.rolling_code;
}

void SomfyRemoteControl::activateSomfyChannel(uint8_t idx){

	_activeChannelIdx = idx;
	PRINT("somfyindex: ");PRINTLN(_activeChannelIdx);
	PRINT("somfy address: ");PRINTLN(_somfyChannelList[idx].address);
	//_remoteCtrlAddress = _somfyChannelList[idx].address;
	//_rollingCode =  _somfyChannelList[idx].rolling_code;

}



void SomfyRemoteControl::begin(void (*function)()) {//used when global pulse counter function defined in ino-file


	_rf->setFixedThreshold(_ookFixedThresh);
	_rf->setRSSIThreshold(_rssiThresh);
	_rf->attachUserInterrupt(function);
	//_rf->receiveBegin();commented 26.12.2016
	//ToDo: calibrate ook here
	_rf->unselect();

}

void SomfyRemoteControl::begin() {


	_rf->setFixedThreshold(_ookFixedThresh);
	_rf->setRSSIThreshold(_rssiThresh);
	_rf->attachUserInterrupt(countPulseLength);
	//_rf->receiveBegin(); commented 26.12.2016
	//calibrateOokThresh()
	//_rf->unselect();

}

void SomfyRemoteControl::receiveBegin() {
	_rf->receiveBegin();
}

void SomfyRemoteControl::receiveEnd() {
	_rf->receiveEnd();
}

SomfyRemoteControl::t_status SomfyRemoteControl::receive(volatile word &pulseLength){
	  cli();
	  _pulseLength = pulseLength;
	  pulseLength = 0;
	  sei();

	  if (_pulseLength != 0) {
		  _currentReceiveStatus =  pulse(_pulseLength);
		 // PRINT("length: ");PRINTLN(_pulseLength);
		  _pulseLength = 0;
		  _pulseCount++;
	  }

	  return _currentReceiveStatus;
}

SomfyRemoteControl::t_status SomfyRemoteControl::receive(){
	  cli();
	  _pulseLength = pulseLength;
	  pulseLength = 0;
	  sei();

	  if (_pulseLength != 0) {
		  _currentReceiveStatus =  pulse(_pulseLength);
		 //PRINT("length: ");PRINTLN(_pulseLength);
		  _pulseLength = 0;
		  _pulseCount++;

	  }
	  return _currentReceiveStatus;
}

SomfyRemoteControl::t_status SomfyRemoteControl::pulse(word p) {
  switch(_status) {
    case k_waiting_synchro:
      if (p > _k_tempo_synchro_hw_min && p < _k_tempo_synchro_hw_max) {
        ++_cpt_synchro_hw;
      }
      else if (p > _k_tempo_synchro_sw_min && p < _k_tempo_synchro_sw_max && _cpt_synchro_hw >= 4) {
        _cpt_bits = 0;
        _previous_bit = 0;
        _waiting_half_symbol = false;
        for(int i=0; i<7; ++i) _payload[i] = 0;
        _status = k_receiving_data;
      } else {
        _cpt_synchro_hw = 0;
      }
      break;

    case k_receiving_data:
      if (p > _k_tempo_symbol_min && p < _k_tempo_symbol_max && !_waiting_half_symbol) {
        _previous_bit = 1 - _previous_bit;
        _payload[_cpt_bits/8] += _previous_bit << (7 - _cpt_bits%8);
        ++_cpt_bits;
      } else if (p > _k_tempo_half_symbol_min && p < _k_tempo_half_symbol_max) {
        if (_waiting_half_symbol) {
          _waiting_half_symbol = false;
          _payload[_cpt_bits/8] += _previous_bit << (7 - _cpt_bits%8);
          ++_cpt_bits;
        } else {
          _waiting_half_symbol = true;
        }
      } else {
        _cpt_synchro_hw = 0;
        _status = k_waiting_synchro;
      }
      break;

    default:
      PRINTLN("Internal error !");
      break;
  }

  t_status retval = _status;

  if (_status == k_receiving_data && _cpt_bits == 56) {
    retval = k_complete;
    _pulseCount = 0;
    decode();
    _status = k_waiting_synchro;
  }

  return retval;
}

bool SomfyRemoteControl::decode() {
  // Dé-obfuscation
	bool unknown = false;
  byte frame[7];
  frame[0] = _payload[0];
  for(int i = 1; i < 7; ++i) frame[i] = _payload[i] ^ _payload[i-1];

 PRINT("received raw data: ");
 for(int i = 0; i < 7; ++i) {
	 PRINT(frame[i] , HEX);_receivedData.rawData[i] = frame[i]; PRINT(" ");
 }
 PRINTLN("");

  // Verification du checksum
  byte cksum = 0;
  for(int i = 0; i < 7; ++i) cksum = cksum ^ frame[i] ^ (frame[i] >> 4);
  cksum = cksum & 0x0F;
  if (cksum != 0) PRINTLN("Checksum incorrect !");

  // Touche de controle
  switch(frame[1] & 0xF0) {
  	case 0x00: PRINTLN("none"); break;
    case 0x10: PRINTLN("My"); break;
    case 0x20: PRINTLN("Up"); break;
    case 0x30: PRINTLN("My+Up"); break;
    case 0x40: PRINTLN("Down"); break;
    case 0x50: PRINTLN("My+Down"); break;
    case 0x60: PRINTLN("Up+Down"); break;
    case 0x80: PRINTLN("Prog1"); break;
    case 0x90: PRINTLN("Sun+Flag"); break;
    case 0xA0: PRINTLN("Flag"); break;
    default: {
    	PRINTLN("???");
    	unknown = true;
    	break;
    }
  }
  //PRINTLN(((frame[1] & 0xF0) >> 4)& 0x0F);
  if (unknown)_receivedData.cmd = String(frame[1] & 0xF0);
  else _receivedData.cmd = somfyCmdStr[((frame[1] & 0xF0) >> 4)& 0x0F];


  // Rolling code
  unsigned long rolling_code = (frame[2] << 8) + frame[3];
  PRINT("Rolling code: "); PRINTLN(rolling_code);
  _receivedData.rolling_code = rolling_code;

  // Adresse
  //unsigned long address = ((unsigned long)frame[4] << 16) + (frame[5] << 8) + frame[6];<-orig
  unsigned long address = ((unsigned long)frame[6] << 16) + (frame[5] << 8) + frame[4];
  PRINT("Adresse: "); PRINTLN(address, HEX);
  _receivedData.address = address;

  return true;//ToDO check this!
}

SomfyRemoteControl::datagram_t SomfyRemoteControl::getReceivedData()
{
	_currentReceiveStatus = _status;

	return _receivedData;
}

byte SomfyRemoteControl::getCurrentTemperature()
{
	return _rf->readTemperature(0) ;
}

//SomfyRemoteControl::t_status SomfyRemoteControl::getCurrentReceiveStatus()
//{
//	return _currentReceiveStatus ;
//}

SomfyRemoteControl::t_status SomfyRemoteControl::getCurrentReceiveStatus()
{
	return _status ;
}

uint16_t SomfyRemoteControl::getCurrentRollingCode()
{
	//return _rollingCode;
	return _somfyChannelList[_activeChannelIdx].rolling_code;
}
////////////////////////////////////////////////////////////////////////////////////

// ***********************************************************************************

bool SomfyRemoteControl::cmd(String in){
	//_rf->select();
	//delay(10);
	_rf->receiveEnd();
	_rf->transmitBegin();
	//_rf->setPowerLevel(powerLevel); doesn't make sense here....needs restart!

    if (in==somfyCmdStr[SomfyCmd_Up]){
    //cmd=0x02; //***************** Up
    transmit(SomfyCmd_Up, _somfyChannelList[_activeChannelIdx].address , _somfyChannelList[_activeChannelIdx].rolling_code,2);
    int pmax=_cmdRepetitions;
    for(int p=0; p<pmax;++p) {
    transmit(SomfyCmd_Up, _somfyChannelList[_activeChannelIdx].address , _somfyChannelList[_activeChannelIdx].rolling_code,7);
    }

    PRINT("Increasing rollingCode: = ");PRINTLN (_somfyChannelList[_activeChannelIdx].rolling_code);
    _somfyChannelList[_activeChannelIdx].rolling_code++;
    }

    if (in==somfyCmdStr[SomfyCmd_Down]){
    //cmd=0x04;//**************** Down
    transmit(SomfyCmd_Down, _somfyChannelList[_activeChannelIdx].address, _somfyChannelList[_activeChannelIdx].rolling_code,2);
    int pmax=_cmdRepetitions;
    for(int p=0; p<pmax;++p) {
    transmit(SomfyCmd_Down, _somfyChannelList[_activeChannelIdx].address, _somfyChannelList[_activeChannelIdx].rolling_code,7);
    }

    PRINT("Increasing _somfyChannelList[_activeChannelIdx].rolling_code= ");PRINTLN (_somfyChannelList[_activeChannelIdx].rolling_code);
    _somfyChannelList[_activeChannelIdx].rolling_code++;
    }

    if (in==somfyCmdStr[SomfyCmd_My]){
    //cmd=0x01;//**************** My/Stop
    transmit(SomfyCmd_My, _somfyChannelList[_activeChannelIdx].address, _somfyChannelList[_activeChannelIdx].rolling_code,2);
    int pmax=_cmdRepetitions;
    for(int p=0; p<pmax;++p) {
    transmit(SomfyCmd_My, _somfyChannelList[_activeChannelIdx].address, _somfyChannelList[_activeChannelIdx].rolling_code,7);
    }
    PRINT("_somfyChannelList[_activeChannelIdx].rolling_code= ");PRINTLN (_somfyChannelList[_activeChannelIdx].rolling_code);
    _somfyChannelList[_activeChannelIdx].rolling_code++;
    }
    if (in==somfyCmdStr[SomfyCmd_MyUp]){
    //cmd=0x03;//**************** My+Up
    transmit(SomfyCmd_MyUp, _somfyChannelList[_activeChannelIdx].address, _somfyChannelList[_activeChannelIdx].rolling_code,2);
    int pmax=_cmdRepetitions;
    for(int p=0; p<pmax;++p) {
    transmit(SomfyCmd_MyUp, _somfyChannelList[_activeChannelIdx].address, _somfyChannelList[_activeChannelIdx].rolling_code,7);
    }
    PRINT("_somfyChannelList[_activeChannelIdx].rolling_code= ");PRINTLN (_somfyChannelList[_activeChannelIdx].rolling_code);
    _somfyChannelList[_activeChannelIdx].rolling_code++;
    }
    if (in==somfyCmdStr[SomfyCmd_MyDown]){
    //cmd=0x05;//**************** My+Down
    transmit(SomfyCmd_MyDown, _somfyChannelList[_activeChannelIdx].address, _somfyChannelList[_activeChannelIdx].rolling_code,2);
    int pmax=_cmdRepetitions;
    for(int p=0; p<pmax;++p) {
    transmit(SomfyCmd_MyDown, _somfyChannelList[_activeChannelIdx].address, _somfyChannelList[_activeChannelIdx].rolling_code,7);
    }
    PRINT("_somfyChannelList[_activeChannelIdx].rolling_code= ");PRINTLN (_somfyChannelList[_activeChannelIdx].rolling_code);
    _somfyChannelList[_activeChannelIdx].rolling_code++;
    }
    if (in==somfyCmdStr[SomfyCmd_UpDown]){
    //cmd=0x06;//**************** UpDown
    transmit(SomfyCmd_UpDown, _somfyChannelList[_activeChannelIdx].address, _somfyChannelList[_activeChannelIdx].rolling_code,2);
    int pmax=_cmdRepetitions;
    for(int p=0; p<pmax;++p) {
    transmit(SomfyCmd_UpDown, _somfyChannelList[_activeChannelIdx].address, _somfyChannelList[_activeChannelIdx].rolling_code,7);
    }
    PRINT("_somfyChannelList[_activeChannelIdx].rolling_code= ");PRINTLN (_somfyChannelList[_activeChannelIdx].rolling_code);
    _somfyChannelList[_activeChannelIdx].rolling_code++;
    }

    if (in=="prog"){
    //cmd=0x8;//**************** Prog
    transmit(0x08, _somfyChannelList[_activeChannelIdx].address, _somfyChannelList[_activeChannelIdx].rolling_code,2);
    int pmax=_cmdRepetitions;
    for(int p=0; p<pmax;++p) {
    transmit(0x08, _somfyChannelList[_activeChannelIdx].address, _somfyChannelList[_activeChannelIdx].rolling_code,7);
    }
    PRINT("Programming/delete (short push): _somfyChannelList[_activeChannelIdx].rolling_code= ");PRINTLN (_somfyChannelList[_activeChannelIdx].rolling_code);
    _somfyChannelList[_activeChannelIdx].rolling_code++;
    }


    if (in==somfyCmdStr[SomfyCmd_SunFlag]){
    //cmd=0x09;//**************** Sun+Flag
    transmit(SomfyCmd_SunFlag, _somfyChannelList[_activeChannelIdx].address, _somfyChannelList[_activeChannelIdx].rolling_code,2);
    int pmax=_cmdRepetitions;
    for(int p=0; p<pmax;++p) {
    transmit(SomfyCmd_SunFlag, _somfyChannelList[_activeChannelIdx].address, _somfyChannelList[_activeChannelIdx].rolling_code,7);
    }
    PRINT("_somfyChannelList[_activeChannelIdx].rolling_code= ");PRINTLN (_somfyChannelList[_activeChannelIdx].rolling_code);
    _somfyChannelList[_activeChannelIdx].rolling_code++;
    }
    if (in==somfyCmdStr[SomfyCmd_Flag]){
    //cmd=0x0A;//**************** Flag
    transmit(SomfyCmd_Flag, _somfyChannelList[_activeChannelIdx].address, _somfyChannelList[_activeChannelIdx].rolling_code,2);
    int pmax=_cmdRepetitions;
    for(int p=0; p<pmax;++p) {
    transmit(SomfyCmd_Flag, _somfyChannelList[_activeChannelIdx].address, _somfyChannelList[_activeChannelIdx].rolling_code,7);
    }
    PRINT("_somfyChannelList[_activeChannelIdx].rolling_code= ");PRINTLN (_somfyChannelList[_activeChannelIdx].rolling_code);
    _somfyChannelList[_activeChannelIdx].rolling_code++;
    }


    if (in==somfyCmdStr[SomfyCmd_Prog2]){
    //cmd=0x08;//**************** Prog
    transmit(SomfyCmd_Prog2-3, _somfyChannelList[_activeChannelIdx].address, _somfyChannelList[_activeChannelIdx].rolling_code,2);
    int pmax= _cmdRepetitions * 10;//long cycle
    for(int p=0; p<pmax;++p) {
    transmit(SomfyCmd_Prog2-3, _somfyChannelList[_activeChannelIdx].address, _somfyChannelList[_activeChannelIdx].rolling_code,7);
    }
    PRINT("Programming (long push): _somfyChannelList[_activeChannelIdx].rolling_code= ");PRINTLN (_somfyChannelList[_activeChannelIdx].rolling_code);
    _somfyChannelList[_activeChannelIdx].rolling_code++;
    }

    _rf->transmitEnd();
    //_rf->receiveBegin();26.12.2016 commented
    //_rf->unselect();
    return true; //TODO: check if error handling necessary!
}

/***************************************************************************************/

bool SomfyRemoteControl::transmit( byte cmd, uint32_t address, uint16_t rolling_code, byte first ) {



  PRINT("cmd= ");PRINTLN(cmd,HEX);
  PRINT("_somfyChannelList[_activeChannelIdx].rolling_code[DEC] = ");PRINTLN(rolling_code);
  // Construction de la trame claire
  byte data[7];
  //Maro: origignal: data[0] = 0xA7;
  data[0] = 0xA0 +  (rolling_code & 0x000F);//from jeedom.com
  data[1] = cmd << 4;
  data[2] = (rolling_code & 0xFF00) >> 8;
  data[3] = rolling_code & 0x00FF;
  // ******************************** Mettre ici l'adresse de votre TC ou de la TC simulée *************
  //Maro: origignal: data[4] = 0xAB; // Address: USE YOUR OWN ADDRESS !!!
  //Maro: origignal: data[5] = 0xCD; // Address: USE YOUR OWN ADDRESS !!!
  //Maro: origignal: data[6] = 0xEF; // Address: USE YOUR OWN ADDRESS !!!
  data[4] = (address & 0x0000FF);       // Address
  data[5] = (address & 0x00FF00) >> 8;  // Address
  data[6] = (address & 0xFF0000) >> 16; // Address


  PRINT("adr= ");PRINT (data[4],HEX);PRINT (data[5],HEX);PRINTLN (data[6],HEX);

   for(int i = 0; i < 7; ++i) PRINT(data[i],HEX);
  PRINTLN("");

  // Calcul du checksum
  byte cksum = 0;
  for(int i = 0; i < 7; ++i) cksum = cksum ^ (data[i]&0xF) ^ (data[i] >> 4);  // ****************
  data[1] = data[1] | (cksum);  // *************************

   for(int i = 0; i < 7; ++i) PRINT(data[i],HEX);
  PRINTLN("");

  // Obsufscation *****************************
  byte datax[7];
  datax[0]=data[0];
  for(int i = 1; i < 7; ++i) datax[i] = datax[i-1] ^ data[i];  // ********************

   for(int i = 0; i < 7; ++i) PRINT(datax[i],HEX);
  PRINTLN("");

  // Emission wakeup, synchro hardware et software

  if (first <= 2)
  {
	  _rf->send( HIGH);
	  delayMicros(_k_tempo_wakeup_pulse);
	  _rf->send( LOW);
	  delayMicros(_k_tempo_wakeup_silence);
  }


  for(int i=0; i<first; ++i) {
    _rf->send( HIGH);
    delayMicros(_k_tempo_synchro_hw);
    _rf->send( LOW);
    delayMicros(_k_tempo_synchro_hw);
  }

  _rf->send( HIGH);
  delayMicros(_k_tempo_synchro_sw);
  _rf->send( LOW);
  delayMicros(_k_tempo_half_symbol);

  // Emission des donnees
  for(int i=0; i<56;++i) {
    byte bit_to_transmit = (datax[i/8] >> (7 - i%8)) & 0x01;
    if (bit_to_transmit == 0) {
      _rf->send( HIGH);
      delayMicros(_k_tempo_half_symbol);
      _rf->send( LOW);
      delayMicros(_k_tempo_half_symbol);
    }
    else
    {
      _rf->send( LOW);
      delayMicros(_k_tempo_half_symbol);
      _rf->send( HIGH);
      delayMicros(_k_tempo_half_symbol);
    }
  }

  _rf->send( LOW);
  delayMicros(_k_tempo_inter_frame_gap);

  return true; //TODO: check this return!!
}

void SomfyRemoteControl::delayMicros(uint32_t d) {
#ifdef SYS_DELAY
  uint32_t t = micros() + d;
  while(micros() < t);
#else
  delayMicroseconds(d);
#endif
}



SomfyRemoteControl::~SomfyRemoteControl() {
	// TODO Auto-generated destructor stub
	delete _rf;
}

} /* namespace MRO */
