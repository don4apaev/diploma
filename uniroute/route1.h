#include "HardWare.h"
#include "SoftWare.h"
#include <DFRobotDFPlayerMini.h>

// Ethernet settings
const byte      MacAd[]   = { 0xDE, 0xAD, 0xBE, 0xEF, 0x90, 0xD4 };
const byte      IPAd[]    = { 10, 11, 13, 109 }; // 104
const char      Name[]    = "SWINGS-STONES";
// pin constants
const uint8_t   NumRlys             = 2,
                NumBrks             = 3;
const int8_t    DoorRels[NumRlys]   = { 2, // 1-2 rooms door reley
                                        3}, // out door reley
                Switches[NumBrks]   = { 5, // first swing pad switch
                                        6, // second swing pad switch
                                        9 }; // stone pad switch
const int16_t   FirstRMultiplier    = 5,
                SecondRMultiplier   = 50,
                BonusMultiplier     = 5;
const uint32_t  FirstRDuration      = 450000,
                SecondRDuration     = 450000,
                BonusLimit          = 3000;


class Route{
  
  uint8_t             _numRlys,
                      _numBrks,
                      _stat; // status
  uint16_t            _points; // game points
  uint32_t            _time,
                      _bonusTime;
  Reley*              _releys;
  Breaker*            _breakers;
  HardwareSerial*     _serial;
  DFRobotDFPlayerMini _mp3;
  enum { DoorInR, DoorOutR };
  enum { Swing1B, Swing2B, StoneB };

  void init( HardwareSerial* SSerial, uint8_t NumRlys, uint8_t NumBrks, int8_t* RlyPins, int8_t* BrkPins ){
    _serial = SSerial;
    _numRlys = NumRlys;
    _numBrks = NumBrks;
    _releys = new Reley[_numRlys];
    for( int i=0; i<_numRlys; i++ ){
      _releys[i] =  Reley(RlyPins[i]);
    }
    _breakers = new Breaker[_numBrks];
    for( int i=0; i<_numBrks; i++ ){
      _breakers[i] =  Breaker(BrkPins[i], ON_VAL);
    }
  }
  
public:

  Route( HardwareSerial* SSerial = NULL, uint8_t NumRlys = 0, uint8_t NumBrks = 0,
          int8_t* RlyPins = NULL, int8_t* BrkPins = NULL ){
    _serial = NULL;
    _releys = NULL;
    _breakers = NULL;
    init( SSerial, NumRlys, NumBrks, RlyPins, BrkPins );
  }
  
  void mount(){
    _serial->begin(9600);
    _mp3.begin(*_serial);
    _mp3.volume(25);
    _mp3.stop();
    for( int i=0; i<_numRlys; i++ ){
      _releys[i].mount();
    }
    for( int i=0; i<_numBrks; i++ ){
      _breakers[i].mount();
    }
    _stat = 0;
    _points = 0;
  }
  
  void prepare(){
    for( int i=0; i<_numRlys; i++ ){
      _releys[i].setStat(1);
    }
  }
  
  void start(){
    _stat = 1;
    _points = 0;
    for( int i=0; i<_numRlys; i++ ){
      if( _releys[i].getStat() != 1 ) _releys[i].setStat(1);
    }
    _mp3.play(4);
    _bonusTime = _time = millis();
  }
  
  void stop(){
    _stat = 0;
    _mp3.stop();
  }
  
  void alarm(){
    _stat = 0;
    for( int i=0; i<_numRlys; i++ ){
      _releys[i].setStat(0);
    }
    _mp3.stop();
  }
  
  void refresh(){
    for( int i=0; i<_numRlys; i++ ){
      _releys[i].refresh();
    }
    for( int i=0; i<_numBrks; i++ ){
      _breakers[i].refresh();
    }
    if( _stat == 1 ){
      if( _breakers[Swing1B].getReleased() || _breakers[Swing2B].getReleased() ){
        _points += FirstRMultiplier;
        if( timeInterval(_bonusTime) < BonusLimit ){ // if tap pads faster then in 3 sec
          _points += BonusMultiplier;
          _mp3.play(1);
        }
        else {
          _mp3.play(5);
        }
        _bonusTime = millis();
      }
      if( timeInterval(_time) > FirstRDuration ){
        _stat = 2;
        _releys[DoorInR].setStat(0);
      }
    }
    else if( _stat == 2 ){
      if( _breakers[StoneB].getReleased() ){
        _points += SecondRMultiplier;
        _mp3.play(5);
      }
      if( timeInterval(_time) > FirstRDuration + SecondRDuration ){
        _stat = 0;
        _releys[DoorOutR].setStat(0);
        _mp3.play(2);
      }
    }
  }

  uint8_t stat() const{
    return _stat;
  }
  
  uint16_t score() const{
    return _points;
  }

  void giveForm(String *string){
    if( _stat > 0)
      *string += "<a href=\"/?stop\"\">Stop</a>";
    else
      *string += "<a href=\"/?start\"\">Start</a>";
    *string += "\n<br /><br />\n";
    if( _releys[DoorInR].getStat() == 1 && _releys[DoorOutR].getStat() == 1 )
      *string += "Quest prepared";
    else
      *string += "<a href=\"/?prep_on\"\">Prepare</a>";
    *string += "\n<br /><br />\n";
    if( _releys[DoorInR].getStat() != 1 )
      *string += "<a href=\"/?doorin_on\"\">Turn inner lock ON</a>";
    else
      *string += "<a href=\"/?doorin_off\"\">Turn inner lock OFF</a>";
    *string += "\n<br /><br />\n";
    if( _releys[DoorOutR].getStat() != 1 )
      *string += "<a href=\"/?doorout_on\"\">Turn exit lock ON</a>";
    else
      *string += "<a href=\"/?doorout_off\"\">Turn exit lock OFF</a>";
    *string += "\n<br /><br />\n";
    *string += "First swing sensor ";
    switch( _breakers[Swing1B].getStat() ){
      case PRS_VAL: *string += "pressed;"; break;
      case UNP_VAL: *string += "unpressed;"; break;
      case REL_VAL: *string += "released;"; break;
      case HLD_VAL: *string += "held;"; break;
    }
    *string += "\n<br /><br />\n";
    *string += "Second swing sensor ";
    switch( _breakers[Swing2B].getStat() ){
      case PRS_VAL: *string += "pressed;"; break;
      case UNP_VAL: *string += "unpressed;"; break;
      case REL_VAL: *string += "released;"; break;
      case HLD_VAL: *string += "held;"; break;
    }
    *string += "\n<br /><br />\n";
    *string += "Stone sensor ";
    switch( _breakers[StoneB].getStat() ){
      case PRS_VAL: *string += "pressed;"; break;
      case UNP_VAL: *string += "unpressed;"; break;
      case REL_VAL: *string += "released;"; break;
      case HLD_VAL: *string += "held;"; break;
    }
    *string += "\n<br /><br />\n";
  }
  
  void takeGet( String *str ){
    if (str->indexOf("?stop") >0){
      stop();
    }
    if (str->indexOf("?start") >0){
      start();
    }
    if (str->indexOf("?prep_on") >0){
      prepare();
    }
    if (str->indexOf("?doorin_on") >0){
      _releys[DoorInR].setStat(1);
    }
    if (str->indexOf("?doorin_off") >0){
      _releys[DoorInR].setStat(0);
    }
    if (str->indexOf("?doorout_on") >0){
      _releys[DoorOutR].setStat(1);
    }
    if (str->indexOf("?doorout_off") >0){
      _releys[DoorOutR].setStat(0);
    }
  }
  
}
route( &Serial3, NumRlys, NumBrks, DoorRels, Switches );
