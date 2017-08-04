/*
 * Debug.h
 *
 *  Created on: Dec 23, 2016
 *      Author: user
 */

#ifndef DEBUG_H_
#define DEBUG_H_

#ifdef DEBUG
	#define STARTUP_DELAY 10000
	#define PRINTLN(x) Serial.println((x));
	#define PRINT(x) Serial.print((x));
	#define PRINTF(x,y) Serial.printf((x),(y));
	#define PRINTF2(x,y,z) Serial.printf((x),(y),(z));
	#define PRINTF3(x,y,z,a,b,c,d) Serial.printf((x),(y),(z),(a),(b),(c),(d));
	#define ZVERSION " debug v2 " + String(__DATE__)
	#define ZUP_TIME " " + String(millis()/1000) + " [s]"
#else
	#define STARTUP_DELAY 1000
	#define PRINTLN(...)
	#define PRINT(...)
	#define PRINTF(...)
	#define PRINTF2(...)
	#define PRINTF3(...)
	#define ZVERSION " v2 " + String(__DATE__)
	#define ZUP_TIME " " + String(millis()/1000) + " [s]"
#endif



#endif /* DEBUG_H_ */
