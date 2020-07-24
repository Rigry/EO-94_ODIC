#pragma once

#include "pin.h"
#include "timers.h"
#include <stdint.h>

/// Class Vertical служит для задания направления движения каретки с помощью методов up и down. 
/// Время паузы между сменой направления, time, задается конструктором.
template <class Control, class Sense_up, class Sense_down, class Emerg_up, class Emerg_down> 
class Vertical : Subscriber 
{
   volatile enum State {Wait, Up, Down, Pause_Wait, Pause_Up, Pause_Down} state {State::Wait};
   void notify() override;
   bool subscribe {false};
   void wake();
   void sleep();
   uint16_t time_count {0};
   Control& control;
public:
   uint16_t time_pause;
   Vertical (Control& control, uint16_t time_pause);
   void stop();
   void up();
   void down();
   void grab_on();
   void grab_off();
   bool isUp();
   bool isDown();
   bool isWorking(){return not ((state == State::Wait) or (state == State::Pause_Wait));}
};

template <class Control, class Sense_up, class Sense_down, class Emerg_up, class Emerg_down>
Vertical<Control, Sense_up, Sense_down, Emerg_up, Emerg_down>::Vertical (Control &control, uint16_t time_pause)
   : control   {control}
   , time_pause{time_pause}
{}

template <class Control, class Sense_up, class Sense_down, class Emerg_up, class Emerg_down>
void Vertical<Control, Sense_up, Sense_down, Emerg_up, Emerg_down>::stop ()
{
   if (state != State::Wait) {
      control.stop_v();
      state = State::Pause_Wait;
   }
} 

template <class Control, class Sense_up, class Sense_down, class Emerg_up, class Emerg_down>
void Vertical<Control, Sense_up, Sense_down, Emerg_up, Emerg_down>::up ()
{
   if ( ((not Sense_up::isSet()) and (not Emerg_up::isSet()) ) and state == State::Wait) {
      control.up();
      state = State::Up;
      wake();
   } else if (state == State::Down) {
         control.stop_v();
         state = State::Pause_Up;
   } else if (state == State::Pause_Up or state == State::Pause_Down) {
         state = State::Pause_Up;
   } else if (state == State::Pause_Wait) {
         state = State::Pause_Wait;
   }

}

template <class Control, class Sense_up, class Sense_down, class Emerg_up, class Emerg_down>
void Vertical<Control, Sense_up, Sense_down, Emerg_up, Emerg_down>::down ()
{
   if ( ((not Sense_down::isSet()) and (not Emerg_down::isSet()) ) and state == State::Wait) {
      control.down();
      state = State::Down;
      wake();
   } else if (state == State::Up) {
         control.stop_v();
         state = State::Pause_Down;
   } else if (state == State::Pause_Up or state == State::Pause_Down) {
         state = State::Pause_Down;
   } else if (state == State::Pause_Wait) {
         state = State::Pause_Wait;
   }

}

template <class Control, class Sense_up, class Sense_down, class Emerg_up, class Emerg_down>
void Vertical<Control, Sense_up, Sense_down, Emerg_up, Emerg_down>::grab_on ()
{
   if (isDown ()) {
      control.grab_on();
   }
}

template <class Control, class Sense_up, class Sense_down, class Emerg_up, class Emerg_down>
void Vertical<Control, Sense_up, Sense_down, Emerg_up, Emerg_down>::grab_off ()
{
   if (isDown ()) {
      control.grab_off();
   }
} 

template <class Control, class Sense_up, class Sense_down, class Emerg_up, class Emerg_down>
void Vertical<Control, Sense_up, Sense_down, Emerg_up, Emerg_down>::wake ()
{
   if (not subscribe) {
      tickUpdater.subscribe(this);
      subscribe = true;
   }
}

template <class Control, class Sense_up, class Sense_down, class Emerg_up, class Emerg_down>
void Vertical<Control, Sense_up, Sense_down, Emerg_up, Emerg_down>::sleep ()
{
   if (subscribe) {
      tickUpdater.unsubscribe(this);
      subscribe = false;
   }
}

template <class Control, class Sense_up, class Sense_down, class Emerg_up, class Emerg_down>
bool Vertical<Control, Sense_up, Sense_down, Emerg_up, Emerg_down>::isUp ()
{
   return (Sense_up::isSet() or Emerg_up::isSet());
}

template <class Control, class Sense_up, class Sense_down, class Emerg_up, class Emerg_down>
bool Vertical<Control, Sense_up, Sense_down, Emerg_up, Emerg_down>::isDown ()
{
   return (Sense_down::isSet() or Emerg_down::isSet());
}

template <class Control, class Sense_up, class Sense_down, class Emerg_up, class Emerg_down>
void Vertical<Control, Sense_up, Sense_down, Emerg_up, Emerg_down>::notify()
{
   switch (state) {
      case Wait:
         sleep();
      break;
      case Up:
         if (Sense_up::isSet() or Emerg_up::isSet()) {
            control.stop_v();
            state = State::Pause_Wait;
         }
      break;
      case Down:
         if (Sense_down::isSet() or Emerg_down::isSet()) {
            control.stop_v();
            state = State::Pause_Wait;
         }
      break;
      case Pause_Up:
         if (++time_count >= time_pause) {
            state = State::Up;
            control.up();
            time_count = 0;
         }
      break;
      case Pause_Down:
         if (++time_count >= time_pause) {
            state = State::Down;
            control.down();
            time_count = 0;
         }
      break;
      case Pause_Wait:
         if (++time_count >= time_pause) {
            state = State::Wait;
            time_count = 0;
         }
      break;
   }
}


