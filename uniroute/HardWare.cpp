#include "HardWare.h"
#include "SoftWare.h"

/*************************Breaker******************************************/
/*
 * initiate instance
 */
void Breaker::init( int8_t BrkPin, bool Pullup, uint32_t RattleDelayMillis,
		uint32_t HeldTimeMillis ){
	_pullup = Pullup;
	_toChange = OFF_VAL;
	_brkPin = BrkPin;
	_brkStat = OFF_VAL;
	_rattleDelayMillis = RattleDelayMillis;
	_heldTimeMillis = HeldTimeMillis;
}
/*
 * get sensor status of the pullup variable
 */
bool Breaker::getSensor(){
	return _pullup == OFF_VAL ? digitalRead( _brkPin ) : !digitalRead( _brkPin );
}
/*
 * change stat variable
 */
void Breaker::changeStat( bool sensorStat ){
	if( _toChange == ON_VAL && timeInterval(_time) > _rattleDelayMillis ){
		if( sensorStat && _brkStat == UNP_VAL ){
			_brkStat = PRS_VAL;
		}
		if( !sensorStat && ( _brkStat == PRS_VAL || _brkStat == HLD_VAL ) ){
			_brkStat = REL_VAL;
		}
		_toChange = OFF_VAL;
		return;
	}
	if( sensorStat != _brkStat && _toChange == OFF_VAL ){
        	if( !sensorStat ){
                	if( _brkStat == REL_VAL ){
			        _brkStat = OFF_VAL;
				return;
			}
                    	else{
				_time = millis();
				_toChange = ON_VAL;
				return;
			}
		}
		else {
			if( _brkStat == UNP_VAL ){
				_time = millis();
				_toChange = ON_VAL;
				return;
			}
		}
	}
	if( _brkStat == PRS_VAL && ( timeInterval(_time) > _heldTimeMillis ) ){
		_brkStat = HLD_VAL;
		return;
	}
}
/*
 * create instance
 */
Breaker::Breaker( int8_t BrkPin, bool Pullup, uint32_t RattleDelayMillis,
		uint32_t HeldTimeMillis ){
	this->init( BrkPin, Pullup, RattleDelayMillis, HeldTimeMillis );
}
/*
 * start instance 
 */
int8_t Breaker::mount(){
	if( _brkPin > EMP_PIN ){
	       if(  _pullup == OFF_VAL ) pinMode( _brkPin, INPUT );
	       else pinMode( _brkPin, INPUT_PULLUP );
	       return ON_VAL;
	}
	return OFF_VAL;
}
/*
 * set pullup
 */
void Breaker::setPullup( bool Pullup ){
	if( _pullup != Pullup && _brkPin > EMP_PIN ){
		_pullup = Pullup;
		if( _pullup == OFF_VAL ) pinMode( _brkPin, INPUT );
	       else pinMode( _brkPin, INPUT_PULLUP );
	}
}
/*
 * get pullup
 */
bool Breaker::getPullup() const{
	return _pullup;
}
/*
 * set rattle delay
 */
void Breaker::setRattleDelay( uint32_t RattleDelayMillis ){
	_rattleDelayMillis = RattleDelayMillis;
}
/*
 * get rattle delay
 */
uint32_t Breaker::getRattleDelay() const{
	return _rattleDelayMillis;
}
/*
 * set held time
 */
void Breaker::setHeldTime ( uint32_t HeldTimeMillis ){
	_heldTimeMillis = HeldTimeMillis;
}
/*
 * get held time
 */
uint32_t Breaker::getHeldTime () const{
	return _heldTimeMillis;
}
/*
 * get breaker status: 0 unpressed, 1 pressed, 2 released,
 * 4 held more then HeldTimeMillis, -1 error;
 */
int8_t Breaker::getStat() const{
	if( _brkPin == EMP_PIN ) return ERR_VAL;
	return _brkStat;
}
/*
 * get breaker press status
 */
bool Breaker::getPressed() const{
	return _brkStat == ON_VAL || _brkStat == HLD_VAL;
}
/*
 * get breaker release status
 */
bool Breaker::getReleased() const{
	return _brkStat == REL_VAL;
}
/*
 * get breaker hold status
 */
bool Breaker::getHeld() const{
	return _brkStat == HLD_VAL;
}
/*
 * get breaker unpress status
 */
bool Breaker::getUnpressed() const{
	return _brkStat == OFF_VAL;
}
/*
 * check for breaker status
 */
void Breaker::refresh(){
	changeStat( getSensor() );
	
}
/*************************Breaker******************************************/

/*************************AnalogBreaker************************************/
/*
 * initiate instance
 */
void AnalogBreaker::init( int8_t Limit ){
	_limit = Limit;
}
/*
 * get analog sensor status
 */
bool AnalogBreaker::getSensor(){
	_analog = map( analogRead( _brkPin ), OFF_VAL, ADC_LIM, OFF_VAL, FULL_VAL );
	return _analog > _limit;
}
/*
 * create instance
 */
AnalogBreaker::AnalogBreaker( int8_t BrkPin, uint8_t  Limit, uint32_t RattleDelayMillis,
			uint32_t HeldTimeMillis):
		Breaker( BrkPin, OFF_VAL, RattleDelayMillis, HeldTimeMillis){
	this->init( Limit );
}
/*
 * set pullup
 */
void AnalogBreaker::setPullup( bool Pullup ){
	return;
}
/*
 * get analog sensor value
 */
uint8_t AnalogBreaker::getAnalog(){
	return  _analog;
}
/*
 * check for breaker status
 */
void AnalogBreaker::refresh(){
	changeStat( getSensor() );
	
}
/*************************AnalogBreaker************************************/

/*************************Reley********************************************/
/*
 * initiate instance
 */
void Reley::init( int8_t RlPin, bool DefStat, bool InvStat, uint32_t UnsetTimeMillis ){
	_inverse = InvStat;
	_default = DefStat;
	_unset = OFF_VAL;
	_rlPin = RlPin;
	_unsetTimeMillis = UnsetTimeMillis;

}
/*
 * create instance
 */
Reley::Reley( int8_t RlPin, bool DefStat, bool InvStat, uint32_t UnsetTimeMillis ){
	this->init( RlPin, DefStat, InvStat, UnsetTimeMillis );
}
/*
 * start instance 
 */
int8_t Reley::mount(){
	if( _rlPin > EMP_PIN ){
		pinMode( _rlPin, OUTPUT );
		digitalWrite(_rlPin, _default ^ _inverse );
		return ON_VAL;
	}
	return OFF_VAL;
}
/*
 * set default reley stat
 */
void Reley::setDefaultStat( bool DefStat ){
	if( _default != DefStat ){
		_default = DefStat;
		if( _rlPin > EMP_PIN ){
			digitalWrite(_rlPin, _default ^ _inverse );
		}
	}
}
/*
 * get default reley stat
 */
bool Reley::getDefaultStat(){
	return _default;
}
/*
 * inverse output stat
 */
void Reley::setInverseStat( bool InvStat ){
	if( _inverse != InvStat ){
		_inverse = InvStat;
		if( _rlPin > EMP_PIN ){
			bool tmpStat = digitalRead(_rlPin) ^ !_inverse;
			digitalWrite(_rlPin, tmpStat ^ _inverse );
		}
	}
}
/*
 * get output inverse
 */
bool Reley::getInverseStat(){
	return _inverse;
}
/*
 * set unset time in millis
 */
void Reley::setUnsetTime ( uint32_t UnsetTimeMillis ){
	_unsetTimeMillis = UnsetTimeMillis;
}
/*
 * get unset time in millis
 */
uint32_t Reley::getUnsetTime(){
	return _unsetTimeMillis;
}
/*
 * set reley status: 0 unload, 1 loaded;
 * set unsetable status: 0 non-unsetable, 1 unsetable
 */
void Reley::setStat( bool Stat, bool Unset ){
	if( _rlPin > EMP_PIN ){
		digitalWrite(_rlPin, Stat ^ _inverse );
		if( Unset ){
			_unset = ON_VAL;
			_time = millis();			
		}
		else {
			_unset = OFF_VAL;
		}
	}
}

/*
 * get reley status: 0 unload, 1 loaded, 2 unsetable loaded, -1 error;
 */
int8_t Reley::getStat(){
	if( _rlPin > EMP_PIN ){
		return ( digitalRead(_rlPin) ^ _inverse ) + _unset;
	}
	else return ERR_VAL;
}
/*
 * check for unset timeout
 */
void Reley::refresh(){
	if( _rlPin > EMP_PIN && _unsetTimeMillis > OFF_VAL && _unset == ON_VAL 
			&& getStat() != _default && timeInterval(_time) > _unsetTimeMillis){
		_unset = OFF_VAL;
		digitalWrite(_rlPin, _default ^ _inverse );
	}
}
/*************************Reley********************************************/

/*************************PWM**********************************************/
/*
 * set arduino analog output
 */
void PWM::Set( uint8_t val ){
	_curStat = val;
	if( _pwmPin > EMP_PIN ){
		analogWrite(_pwmPin, 
			map(_curStat, OFF_VAL, FULL_VAL, OFF_VAL, PWM_SIZE)
			);
	}
}
/*
 * initiate instance
 */
void PWM::init( int8_t PwmPin, uint32_t NextDimSpeedMillis ){
	_dimming = OFF_VAL;
	_pwmPin = PwmPin;
	_curStat = OFF_VAL;
	_nextDimSpeedMillis = NextDimSpeedMillis;
}
/*
 * create instance
 */
PWM::PWM( int8_t PwmPin, uint32_t NextDimSpeedMillis){
	init( PwmPin, NextDimSpeedMillis );
}
/*
 * start instance
 */
int8_t PWM::mount(){
	if( _pwmPin > EMP_PIN ){
		pinMode( _pwmPin, OUTPUT );
		Set( _curStat );
		return ON_VAL;
	}
	return OFF_VAL;	
}
/*
 * set default dimming speed
 */
void PWM::setDimmingSpeed ( uint32_t NextDimSpeedMillis ){
	_nextDimSpeedMillis = NextDimSpeedMillis;	
}
/*
 * get default dimming speed
 */
uint32_t PWM::getDimmingSpeed(){
	return _nextDimSpeedMillis;
}
/*
 * immediately set pwm stat
 */
void PWM::setStat( uint8_t val ){
	_dimming = OFF_VAL;
	Set( val>FULL_VAL ? FULL_VAL : val );
}
/*
 * dimming pwm to stat
 */
void PWM::dimStat( uint8_t val ){
	_curDimSpeedMillis = _nextDimSpeedMillis;
	_dimTo = ( val>FULL_VAL ? FULL_VAL : val );
	_dimming = ON_VAL;
}
/*
 * dimming pwm to stat with given speed
 */
void PWM::dimStat( uint8_t val, uint32_t GivenDimSpeedMillis ){
	_curDimSpeedMillis = GivenDimSpeedMillis;
	_dimTo = ( val>FULL_VAL ? FULL_VAL : val );
	_dimming = ON_VAL;
}
/*
 * get current stat
 */
int8_t PWM::getStat()const{
	if( _pwmPin > EMP_PIN ) return _curStat;
	else return ERR_VAL;
}
/*
 * change output stat while dimming
 */
void PWM::refresh(){
	if( _pwmPin > EMP_PIN && _dimming && timeInterval(_time) > _curDimSpeedMillis ){
		uint8_t tmpStat = getStat();
		if( tmpStat == _dimTo ){
			_dimming = OFF_VAL;
			return;
		} else if( tmpStat < _dimTo ){
			Set( tmpStat + 1 );
		} else {
			Set( tmpStat - 1 );
		}
		_time = millis();
	}
}
/*************************PWM**********************************************/

