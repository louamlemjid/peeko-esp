
#ifndef PEEKODORO_H
#define PEEKODORO_H

#include <Arduino.h>
#include "sound.h"

extern Sound peekoSound;

class PeekoDoro {
  protected:
    int _workPeriodValue;
    int _breakPeriodValue;
    int _workPeriod;
    int _breakPeriod;
    unsigned long _lastUpdate;
    bool _stop;
    int _sets;
    
  
  public:
  PeekoDoro(int workPeriod = 25 * 60, int breakPeriod = 10 * 60,bool stop = true): _workPeriod(workPeriod),_workPeriodValue(workPeriod),_breakPeriod(breakPeriod),_breakPeriodValue(breakPeriod),_lastUpdate(0), _stop(stop),_sets(1){}

    void update(){
      unsigned long now = millis();

      if(now - _lastUpdate > 1000){
        _lastUpdate = now;
        
        if(!_stop){
          
          if(_workPeriod > 0)_workPeriod--;
          else {
           
            if(_breakPeriod > 0)_breakPeriod--;
            else{
              _stop = true;
              _workPeriod = _workPeriodValue;
              _sets++;
              if(_sets % 4 == 0){
                _breakPeriod = _breakPeriodValue * 3;
              }
              else _breakPeriod = _breakPeriodValue;
              
              peekoSound.playMelody();
              
            }
             if(_workPeriod == 0 && _breakPeriod == _breakPeriodValue){
              _stop = true;
              peekoSound.playMelody();
            }
          }
        }



      }
    }
    void reset(){
      _stop = true;
      _workPeriod = 25 * 60;
      _breakPeriod = 10 * 60;
    }

    int getWorkPeriod(){
      return _workPeriod;
    }
    int getWorkPeriodValue(){
      return _workPeriodValue;
    }
    int getBreakPeriodValue(){
      return _breakPeriodValue;
    }
    int getBreakPeriod(){
      return _breakPeriod;
    }
    int getSets(){
      return _sets;
    }
    void toggleStop(){
      _stop = !_stop;
    }
    void setWorkPeriod(int workPeriod){
      _workPeriodValue = workPeriod;
    }
    void setBreakPeriod(int breakPeriod){
      _breakPeriodValue = breakPeriod;
    }
};
#endif