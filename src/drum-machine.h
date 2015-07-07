/*
 Arduino Drum Machine Firmware
 Copyright (C) 2015 Valentin Pratz

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along
 with this program; if not, write to the Free Software Foundation, Inc.,
 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#ifndef DRUM_MACHINE_H
#define DRUM_MACHINE_H

struct Rhythm {
  /* Describes the rhythm for a time signature for an instrument */
  int numerator;
  int denominator;

  /*
  Describes whether the notes have to be seen as semibreve, half note,
  quarter note, etc.
  */
  int subdivision;
  /*
  Describe whether the instrument has to play.
  0 = not playing.
  1 - 0x7f=playing with volume ~
  */
  const int* notes;
  int note_count; /* length of *notes* */
};

struct Instrument {
  /* Describes an instrument */
  int midi_note;
  int input_pin;

  Rhythm* rhythms;
  int rhythm_count;
  int cur_rhythm;
  int default_rhythm;
};

class View {
  /* Describes a display screen */
public:
  virtual void updateDisplay() = 0;
  virtual void computeUp() {};
  virtual void computeDown() {};
  virtual void computeLeft() {}
  virtual void computeRight() {}
  virtual void computeEnter() {}
};

void setup();
void loop();
void sendMIDI(const int, const int, const int);
void sendShortMIDI(const int, const int);
void computeStep(int);
boolean computeJoystick();


#endif
