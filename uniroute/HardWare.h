#ifndef HARDWARE_H_INCLUDED
#define HARDWARE_H_INCLUDED

#include <Arduino.h>


const uint8_t	PWM_SIZE	= 255;	// arduino pwm range
const int8_t	EMP_PIN		= -1,	// dufault pin value
		EMP_VAL		= 0,	// default variable value
		FULL_VAL	= 100,	// pwm class range
		ON_VAL		= 1,	// on value
		OFF_VAL		= 0,	// off value
		PRS_VAL		= 1,	// press value
		UNP_VAL		= 0,	// unpress value
		REL_VAL		= 2,	// released value
		HLD_VAL		= 4,	// held value
		ERR_VAL		= -1;	// error value
const uint16_t	ADC_LIM		= 1023;

const uint32_t BrakerDefauilRattleDelayMillis = 100;
const uint32_t BrakerDefauiltHeldTimeMillis = 1000;

/*
 * Breaker([ uint8_t InputPin = EMP_PIN [, bool Pullup = OFF_VAL
 * 		[, uint32_t RattleDelayMillis = BrakerDefauilRattleDelayMillis
 * 		[, uint32_t HeldTimeMillis = BrakerDefauiltHeldTimeMillis ]]]]
 * 		);
 *
 * int8_t mount(); // as mandatory in setup(): 1 success, 0 error;
 *
 * void setPullup(bool);
 * bool getPullup();
 * void setRattleDelay( uint32_t );
 * uint32_t getRattleDelay();
 * void setHeldTime ( uint32_t );
 * uint32_t getHeldTime();
 * int8_t getStat(); 	// 0 unpressed, 1 pressed, 2 released,
 * 			// 4 held > HeldTimeMillis,  -1 error;
 * bool getPressed();
 * bool getHeld();
 * bool getReleased();
 * bool getUnpressed();
 *
 * void refresh(); // as mandatory in loop()
 */

class Breaker{
protected:
	bool		_pullup,
			_toChange;
	int8_t		_brkPin;
	uint8_t		_brkStat;
	uint32_t	_time,
			_rattleDelayMillis,
			_heldTimeMillis;
	/*
	 * initiate instance
	 */
	void init( int8_t BrkPin, bool Pullup, uint32_t RattleDelayMillis,
			uint32_t HeldTimeMillis );
	/*
	 * get sensor status of the pullup variable
	 */
	bool getSensor();
	/*
	 * change stat variable
	 */
	void changeStat( bool sensorStat );
public:
	/*
	 * create instance
	 */
	Breaker( int8_t BrkPin = EMP_PIN, bool Pullup = OFF_VAL, 
			uint32_t RattleDelayMillis = BrakerDefauilRattleDelayMillis,
			uint32_t HeldTimeMillis = BrakerDefauiltHeldTimeMillis );
	/*
	 * start instance 
	 */
	int8_t mount();
	/*
	 * set pullup
	 */
	void setPullup( bool Pullup );
	/*
	 * get pullup
	 */
	bool getPullup() const;
	/*
	 * set rattle delay
	 */
	void setRattleDelay( uint32_t RattleDelayMillis );
	/*
	 * get rattle delay
	 */
	uint32_t getRattleDelay() const;
	/*
	 * set held time
	 */
	void setHeldTime ( uint32_t HeldTimeMillis );
	/*
	 * get held time
	 */
	uint32_t getHeldTime () const;
	/*
	 * get breaker status: 0 unpressed, 1 pressed, 2 released,
	 * 4 held more then HeldTimeMillis, -1 error;
	 */
	int8_t getStat() const;
	/*
	 * get breaker press (or hold) status
	 */
	bool getPressed() const;
	/*
	 * get breaker release status
	 */
	bool getReleased() const;
	/*
	 * get breaker hold status
	 */
	bool getHeld() const;
	/*
	 * get breaker unpress status
	 */
	bool getUnpressed() const;
	/*
	 * check for breaker status
	 */
	void refresh();
};

const uint32_t ReleyDefaultUnsetTimeMillis = 0;

/*
 * AnalogBreaker([ uint8_t InputPin = EMP_PIN [, uint16_t  Limit = FULL_VAL/2
 * 		[, uint32_t RattleDelayMillis = BrakerDefauilRattleDelayMillis
 * 		[, uint32_t HeldTimeMillis = BrakerDefauiltHeldTimeMillis ]]]]
 * 		)
 * 		: public Breaker;
 *
 * int8_t mount(); // as mandatory in setup(): 1 success, 0 error;
 *
 * void setPullup(bool); // unavailable
 * bool getPullup();
 * void setRattleDelay( uint32_t );
 * uint32_t getRattleDelay();
 * void setHeldTime ( uint32_t );
 * uint32_t getHeldTime();
 * int8_t getStat(); 	// 0 unpressed, 1 pressed, 2 released,
 * 			// 4 held > HeldTimeMillis,  -1 error;
 * bool getPressed();
 * bool getHeld();
 * bool getReleased();
 * bool getUnpressed();
 * uint16_t getAnalog();
 *
 * void refresh(); // as mandatory in loop()
 */

class AnalogBreaker : public Breaker{
	uint8_t		_limit,
			_analog;

	/*
	 * initiate instance
	 */
	void init( int8_t Limit );
	/*
	 * get analog sensor status
	 */
	bool getSensor();
public:
	/*
	 * create instance
	 */
	AnalogBreaker( int8_t BrkPin = EMP_PIN, uint8_t  Limit = FULL_VAL/2,
			uint32_t RattleDelayMillis = BrakerDefauilRattleDelayMillis,
			uint32_t HeldTimeMillis = BrakerDefauiltHeldTimeMillis );
	/*
	 * set pullup
	 */
	void setPullup( bool Pullup );
	/*
	 * get analog sensor value
	 */
	uint8_t getAnalog();
	/*
	 * check for breaker status
	 */
	void refresh();
};


/*
 * Reley([ uint8_t OutputPin = EMP_PIN [, bool DefaultStat = OFF_VAL
 * 		[, bool InversStat = ON_VAL [, uint32_t unsetTimeMillis = 
 * 		ReleyDefauiltUnsetTimeMillis]]]]
 * 		); 
 *
 * int8_t mount(); // as mandatory in setup(): 1 success, 0 error;
 *
 * void setDefaultStat( bool );
 * bool getDefaulStat();
 * void setInverseStat( bool );
 * bool getInverseStat();
 * void setUnsetTime ( uint32_t );
 * uint32_t getUnsetTime();
 * void setStat( bool [, bool = OFF_VAL] ); // 0 unload, 1 loaded;
 * 					 // 0 non-unsetable, 1 unsetable;
 * int8_t getStat(); // 0 unload, 1 loaded, 2 unsetable loaded, -1 error;
 *
 * void refresh(); // as mandatory in loop()
 */

class Reley{
	bool		_inverse,
			_default,
			_unset;
	int8_t		_rlPin;
	uint32_t	_time,
			_unsetTimeMillis;
	/*
	 * initiate instance
	 */
	void init( int8_t RlPin, bool DefStat, bool InvStat, uint32_t UnsetTimeMillis );
public:
	/*
	 * create instance
	 */
	Reley( int8_t RlPin = EMP_PIN, bool DefStat = OFF_VAL, bool InvStat = ON_VAL,
		       	uint32_t UnsetTimeMillis = ReleyDefaultUnsetTimeMillis );
	/*
	 * start instance 
	 */
	int8_t mount();
	/*
	 * set default reley stat
	 */
	void setDefaultStat( bool DefStat );
	/*
	 * get default reley stat
	 */
	bool getDefaultStat();
	/*
	 * inverse output stat
	 */
	void setInverseStat( bool InvStat );
	/*
	 * get output inverse
	 */
	bool getInverseStat();
	/*
	 * set unset time in millis
	 */
	void setUnsetTime ( uint32_t UnsetTimeMillis );
	/*
	 * get unset time in millis
	 */
	uint32_t getUnsetTime();
	/*
	 * set reley status: 0 unload, 1 loaded;
	 * set unsetable status: 0 non-unsetable, 1 unsetable;
	 */
	void setStat( bool Stat, bool Unset = OFF_VAL );
	/*
	 * get reley status: 0 unload, 1 loaded, 2 unsetable loaded, -1 error;
	 */
	int8_t getStat();
	/*
	 * check for unset timeout
	 */
	void refresh();
};

const uint32_t PWMDefaultNextDimmingSpeedMillis = 100;

/*
 * PWM([ uint8_t OutputPin = EMP_PIN [, uint32_t NextDimSpeedMillis = 
 * 		PWMDefaultNextDimmingSpeedMillis]]
 * 		);
 *
 * int8_t mount(); // as mandatory in setup()
 *
 * void setDimmingSpeed( uint32_t );
 * uint32_t getDimmingSpeed();
 * void setStat( uint8_t );
 * void dimStat( uint8_t [, uint32_t] );
 * int8_t getStat();
 *
 * void refresh(); // as mandatory in loop()
 */

class PWM{
	bool		_dimming;
	int8_t		_pwmPin;
	uint8_t		_dimTo,
			_curStat;
	uint32_t	_time,
			_curDimSpeedMillis,
			_nextDimSpeedMillis;

	/*
	 * set arduino analog output
	 */
	void Set( uint8_t val );
	/*
	 * initiate instance
	 */
	void init(  int8_t PwmPin, uint32_t NextDimSpeedMillis );
public:
  	/*
	 * create instance
	 */
	PWM( int8_t PwmPin = EMP_PIN, uint32_t NextDimSpeedMillis = 
			PWMDefaultNextDimmingSpeedMillis );
	/*
	 * start instance
	 */
	int8_t mount();
	/*
	 * set default dimming speed
	 */
	void setDimmingSpeed ( uint32_t NextDimSpeedMillis );
	/*
	 * get default dimming step
	 */
	uint32_t getDimmingSpeed();
	/*
	 * immediately set pwm stat
	 */
	void setStat( uint8_t val );
	/*
	 * dimming pwm to stat
	 */
	void dimStat( uint8_t val );
	/*
	 * dimming pwm to stat with given step
	 */
	void dimStat( uint8_t val, uint32_t GivenDimSpeedMillis );
	/*
	 * get current stat
	 */
	int8_t getStat()const;
	/*
	 * change output stat while dimming
	 */
  	void refresh();
};

#endif // HARDWARE_H_INCLUDED
