// delet ON_VAL after test in Route init()

#include "HardWare.h"
#include "SoftWare.h"
#include <DFRobotDFPlayerMini.h>

// Ethernet settings
const byte      MacAd[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0x90, 0xD5 };
const byte      IPAd[] = { 10, 11, 13, 109 }; // 105
const char      Name[]    = "SISYPHUS-COCKPIT";
// pin constants
const uint8_t   AnalogLimit         = 40,
                NumRlys             = 2,
                NumBrks             = 14;
const int8_t    DoorRels[NumRlys]   = { 2, // 1-2 rooms door reley
                                        3}, // out door reley
                AnalogSwitch        = 57,
                Switches[NumBrks]   = { 22, 23, 24, 25, 26, 27, 28, 29, // floor sensors
                                        30, 31, 32, 34, 37, 39};  // 35 - near exit
const int16_t   FirstRMultiplier    = 200,
                SecondRMultiplier   = -10;
const uint32_t  FirstRDuration      = 45000,
                SecondRDuration     = 45000;

class Route{
  bool*               _brksStat;
  uint8_t             _numRlys,
                      _numBrks,
                      _stat; // status
  uint16_t            _points; // game points
  uint32_t            _time;
  Reley*              _releys;
  Breaker*            _breakers;
  AnalogBreaker       _sisyphusB;
  HardwareSerial*     _serial;
  DFRobotDFPlayerMini _mp3;
  enum { DoorInR, DoorOutR };

  void init( HardwareSerial* SSerial, uint8_t NumRlys, uint8_t NumBrks, int8_t* RlyPins, int8_t* BrkPins,
              int8_t AnlgPin ){
    _serial = SSerial;
    _numRlys = NumRlys;
    _numBrks = NumBrks;
    _releys = new Reley[_numRlys];
    for( int i=0; i<_numRlys; i++ ){
      _releys[i] =  Reley(RlyPins[i]);
    }
    _breakers = new Breaker[_numBrks];
    _brksStat = new bool[_numBrks];
    for( int i=0; i<_numBrks; i++ ){
      _breakers[i] =  Breaker(BrkPins[i], ON_VAL); // delet ON_VAL after test
      _brksStat[i] = 0;
    }
    _sisyphusB = AnalogBreaker(AnlgPin, AnalogLimit);
  }
  
  bool breakerCheck(){
    for( int i=0; i<_numBrks; i++ ){
      if( _breakers[i].getStat() && !_brksStat[i] ){
        _brksStat[i] = 1;
        return 1;
      }
      if( !_breakers[i].getStat() && _brksStat[i] ){
        _brksStat[i] = 0;
      }
    }
    return 0;
  }
  
public:
  
  Route( HardwareSerial* SSerial = NULL, uint8_t NumRlys = 0, uint8_t NumBrks = 0,
          int8_t* RlyPins = NULL, int8_t* BrkPins = NULL, int8_t AnlgPin = -1 ){
    _serial = NULL;
    _releys = NULL;
    _breakers = NULL;
    _brksStat = NULL;
    init( SSerial, NumRlys, NumBrks, RlyPins, BrkPins, AnlgPin );
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
    _sisyphusB.mount();
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
    _time = millis();
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
    _sisyphusB.refresh();
    if( _stat == 1 ){
      if( _sisyphusB.getReleased() ){
        _points += FirstRMultiplier;
        _mp3.play(4);
      }
      if( timeInterval(_time) > FirstRDuration ){
        _stat = 2;
        _releys[DoorInR].setStat(0);
      }
    }
    else if( _stat == 2 ){
      if( breakerCheck() ){
        if( _points > abs(SecondRMultiplier) ){
          _points += SecondRMultiplier;
        }
        else {
          _points = 0;
        }
        _mp3.play(3);
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
    *string += "Sisyphus sensor ";
    switch( _sisyphusB.getStat() ){
      case PRS_VAL: *string += "pressed;"; break;
      case UNP_VAL: *string += "unpressed;"; break;
      case REL_VAL: *string += "released;"; break;
      case HLD_VAL: *string += "held;"; break;
    }
    *string += "\n<br /><br />\n";
    *string += "<tr>Cockpit sensors:</tr>\n";
    for( int i=0; i<_numBrks; i++ ){
      if( !( i % (_numBrks/2) ) ){
        *string += "<tr>";
      }
      switch( _breakers[i].getStat() ){
        case UNP_VAL: *string += "<td>0</td>"; break;
        case PRS_VAL: case REL_VAL:
        case HLD_VAL: *string += "<td>1</td>"; break;
      }
      if( !( i % (_numBrks/2) ) ){
        *string += "</tr>\n";
      }
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
route( &Serial3, NumRlys, NumBrks, DoorRels, Switches, AnalogSwitch );
