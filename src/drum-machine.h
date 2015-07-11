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

const int MAX_MODES = 30;
const int INSTR_STORE_MAX_SIZE = 80;

struct Rhythm {
  /* Describes the rhythm for a time signature for an instrument */
  char name[32];
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

struct RhythmCollection {
  Rhythm* rhythms;
  int rhythm_count;
  int cur_rhythm;
};

struct Instrument {
  /* Describes an instrument */
  int uid; // Unique identifier/relative position in EEPROM
  char name[32];
  int midi_note;
  int input_pin;

  RhythmCollection* rhythms;
  RhythmCollection* breaks;
};

class View {
  /* Describes a display screen */
public:
  virtual void updateDisplay() = 0;
  virtual void computeUp() {}
  virtual void computeDown() {}
  virtual void computeLeft() {}
  virtual void computeRight() {}
  virtual void computeEnter() {}
};

void setup();
void loop();
void sendMIDI(const int, const int, const int);
void sendShortMIDI(const int, const int);
void computeStep(int);
void escapeLCDNum(const int, const int);
void displayBeat(const int, boolean);
void nextView();
void prevView();
boolean computeJoystick();
boolean computeBreakSwitch();
// getter and setter (for EEPROM)
int getMode();
void setMode(int);
// (de)serializer for EEPROM
void saveInstrument(Instrument);
void restoreInstrument(Instrument*);


// INSTRUMENTS AND RHYTHMS
const int empty_rhythm_notes[] = {0x00};
Rhythm empty_rhythm = {
  "None", 4, 4, 1, empty_rhythm_notes, 1
};

Rhythm empty_rhythm_collection_arr[] {
  empty_rhythm
};
RhythmCollection empty_rhythm_collection = {
  empty_rhythm_collection_arr, 1, 0
};
RhythmCollection empty_rhythms[] = {
  empty_rhythm_collection,
  empty_rhythm_collection,
  empty_rhythm_collection,
  empty_rhythm_collection
};

/* Base drum */
/* Base drum rhythms */
const int base_drum_rhythm_4_4_notes[] = {
  0x75, 0x60, 0x60, 0x60
};
Rhythm base_drum_rhythm_4_4 = {
  "1-4", 4, 4, 4, base_drum_rhythm_4_4_notes, 4
};

const int base_drum_rhythm_offbeat_notes[] = {
  0x00, 0x60, 0x00, 0x60
};
Rhythm base_drum_rhythm_offbeat = {
  "Off Beat", 4, 4, 4, base_drum_rhythm_offbeat_notes, 4
};

// Play in triplets, but only on first and last
const int base_drum_rhythm_4_4_jazz_notes[] = {
  0x70, 0x00, 0x60, 0x00, 0x00, 0x00, 0x70, 0x00, 0x60, 0x00, 0x00, 0x00
};
Rhythm base_drum_rhythm_4_4_jazz = {
  "one 'let", 4, 4, 12, base_drum_rhythm_4_4_jazz_notes, 12
};

// Standard rhythms
Rhythm base_standard_rhythms_arr[] = {
  base_drum_rhythm_4_4,
  base_drum_rhythm_offbeat
};
RhythmCollection base_standard_rhythms = {
  base_standard_rhythms_arr, 2, 0
};

// Rock rhythms
Rhythm base_rock_rhythms_arr[] = {base_drum_rhythm_4_4};
RhythmCollection base_rock_rhythms = {
  base_rock_rhythms_arr, 1, 0
};

// Blues rhythms
Rhythm base_blues_rhythms_arr[] = {base_drum_rhythm_4_4};
RhythmCollection base_blues_rhythms = {
  base_blues_rhythms_arr, 1, 0
};

// Jazz rhythms
Rhythm base_jazz_rhythms_arr[] = {base_drum_rhythm_4_4_jazz};
RhythmCollection base_jazz_rhythms = {
  base_jazz_rhythms_arr, 1, 0
};

RhythmCollection base_rhythms[] = {
  base_standard_rhythms,
  base_rock_rhythms,
  base_blues_rhythms,
  base_jazz_rhythms
};

/* Base drum breaks */
const int base_drum_break_standard_notes[] = {
  0x60, 0x60, 0x60, 0x65
};
Rhythm base_drum_break_standard = {
  "Break 1-4", 4, 4, 4, base_drum_break_standard_notes, 4
};

// Standard breaks
Rhythm base_standard_breaks_arr[] = {base_drum_break_standard};
RhythmCollection base_standard_breaks = {
  base_standard_breaks_arr, 1, 0
};

// Rock breaks
Rhythm base_rock_breaks_arr[] = {base_drum_break_standard};
RhythmCollection base_rock_breaks = {
  base_rock_breaks_arr, 1, 0
};

// Blues breaks
Rhythm base_blues_breaks_arr[] = {base_drum_break_standard};
RhythmCollection base_blues_breaks = {
  base_blues_breaks_arr, 1, 0
};

// Jazz breaks
Rhythm base_jazz_breaks_arr[] = {base_drum_break_standard};
RhythmCollection base_jazz_breaks = {
  base_jazz_breaks_arr, 1, 0
};

RhythmCollection base_breaks[] = {
  base_standard_breaks,
  base_rock_breaks,
  base_blues_breaks,
  base_jazz_breaks
};

Instrument base_drum = {
  0, "Base Drum", 36, A4, base_rhythms, base_breaks
};

/* Snare drum */
/* Snare drum rhythms */
const int snare_drum_rhythm_4_4_offbeat_notes[] = {0, 0x40, 0, 0x40};
Rhythm snare_drum_rhythm_4_4_offbeat = {
  "Off Beat", 4, 4, 4, snare_drum_rhythm_4_4_offbeat_notes, 4
};
const int snare_drum_rhythm_4_4_jazz_notes[] = {
  0x00, 0x00, 0x00, 0x70, 0x00, 0x60, 0x00, 0x00, 0x00, 0x70, 0x00, 0x60
};
Rhythm snare_drum_rhythm_4_4_jazz = {
  "2+4: 1+3", 4, 4, 12, snare_drum_rhythm_4_4_jazz_notes, 12
};

// Standard rhythms
Rhythm snare_standard_rhythms_arr[] = {snare_drum_rhythm_4_4_offbeat};
RhythmCollection snare_standard_rhythms = {
  snare_standard_rhythms_arr, 1, 0
};

// Rock rhythms
Rhythm snare_rock_rhythms_arr[] = {snare_drum_rhythm_4_4_offbeat};
RhythmCollection snare_rock_rhythms = {
  snare_rock_rhythms_arr, 1, 0
};

// Blues rhythms
Rhythm snare_blues_rhythms_arr[] = {snare_drum_rhythm_4_4_offbeat};
RhythmCollection snare_blues_rhythms = {
  snare_blues_rhythms_arr, 1, 0
};

// Jazz rhythms
Rhythm snare_jazz_rhythms_arr[] = {snare_drum_rhythm_4_4_jazz};
RhythmCollection snare_jazz_rhythms = {
  snare_jazz_rhythms_arr, 1, 0
};

RhythmCollection snare_rhythms[] = {
  snare_standard_rhythms,
  snare_rock_rhythms,
  snare_blues_rhythms,
  snare_jazz_rhythms
};

/* Snare drum breaks */

const int snare_drum_break_4_4_notes[] = {
  0x60, 0x60, 0x60, 0x65
};
Rhythm snare_drum_break_4_4 = {
  "1-4", 4, 4, 4, snare_drum_break_4_4_notes, 4
};

const int snare_drum_break_standard_notes[] = {
  0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x65, 0x00
};
Rhythm snare_drum_break_standard = {
  "1-7", 4, 4, 8, snare_drum_break_standard_notes, 8
};

// Standard breaks
Rhythm snare_standard_breaks_arr[] = {
  snare_drum_break_standard,
  snare_drum_break_4_4
};
RhythmCollection snare_standard_breaks = {
  snare_standard_breaks_arr, 2, 0
};

// Rock breaks
Rhythm snare_rock_breaks_arr[] = {
  snare_drum_break_standard,
  snare_drum_break_4_4
};
RhythmCollection snare_rock_breaks = {
  snare_rock_breaks_arr, 2, 0
};

// Blues breaks
Rhythm snare_blues_breaks_arr[] = {
  empty_rhythm,
  snare_drum_break_4_4
};
RhythmCollection snare_blues_breaks = {
  snare_blues_breaks_arr, 2, 0
};

// Jazz breaks
Rhythm snare_jazz_breaks_arr[] = {
  empty_rhythm,
  snare_drum_break_4_4
};
RhythmCollection snare_jazz_breaks = {
  snare_jazz_breaks_arr, 2, 0
};

RhythmCollection snare_breaks[] = {
  snare_standard_breaks,
  snare_rock_breaks,
  snare_blues_breaks,
  snare_jazz_breaks
};

Instrument snare_drum = {1, "Snare Drum", 38, A1, snare_rhythms, snare_breaks};

/* Hi-Hat */
/* Hi-Hat rhythms */
const int hi_hat_rhythm_4_4_eights_notes[] = {
  0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48};
Rhythm hi_hat_rhythm_4_4_eights = {
  "1-8", 4, 4, 8, hi_hat_rhythm_4_4_eights_notes, 8
};
const int hi_hat_rhythm_4_4_triplets_notes[] = {
  0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48
};
Rhythm hi_hat_rhythm_4_4_triplets = {
  "1-12", 4, 4, 12, hi_hat_rhythm_4_4_triplets_notes, 12
};
const int hi_hat_rhythm_triplets_1_3_notes[] = {
  0x48, 0x00, 0x40, 0x48, 0x00, 0x40, 0x48, 0x00, 0x40, 0x48, 0x00, 0x40
};
Rhythm hi_hat_rhythm_triplets_1_3 = {
  "One 'let'", 4, 4, 12, hi_hat_rhythm_triplets_1_3_notes, 12
};
const int hi_hat_rhythm_offbeat_notes[] = {
  0x00, 0x48, 0x00, 0x48
};
Rhythm hi_hat_rhythm_4_4_offbeat = {
  "Off Beat", 4, 4, 4, hi_hat_rhythm_offbeat_notes, 4
};

// Standard rhythms
Rhythm hi_hat_standard_rhythms_arr[] = {hi_hat_rhythm_4_4_eights};
RhythmCollection hi_hat_standard_rhythms = {
  hi_hat_standard_rhythms_arr, 1, 0
};

// Rock rhythms
Rhythm hi_hat_rock_rhythms_arr[] = {hi_hat_rhythm_4_4_eights};
RhythmCollection hi_hat_rock_rhythms = {
  hi_hat_rock_rhythms_arr, 1, 0
};

// Blues rhythms
Rhythm hi_hat_blues_rhythms_arr[] = {hi_hat_rhythm_4_4_triplets};
RhythmCollection hi_hat_blues_rhythms = {
  hi_hat_blues_rhythms_arr, 1, 0
};

// Jazz rhythms
Rhythm hi_hat_jazz_rhythms_arr[] = {hi_hat_rhythm_4_4_offbeat};
RhythmCollection hi_hat_jazz_rhythms = {
  hi_hat_jazz_rhythms_arr, 1, 0
};

RhythmCollection hi_hat_rhythms[] = {
  hi_hat_standard_rhythms,
  hi_hat_rock_rhythms,
  hi_hat_blues_rhythms,
  hi_hat_jazz_rhythms
};

/* Hi-Hat breaks */
const int hi_hat_break_standard_notes[] = {
  0x60, 0x60, 0x60, 0x65
};
Rhythm hi_hat_break_standard = {
  "1-4", 4, 4, 4, hi_hat_break_standard_notes, 4
};

// Standard breaks
Rhythm hi_hat_standard_breaks_arr[] = {hi_hat_break_standard};
RhythmCollection hi_hat_standard_breaks = {
  hi_hat_standard_breaks_arr, 1, 0
};

// Rock breaks
Rhythm hi_hat_rock_breaks_arr[] = {hi_hat_break_standard};
RhythmCollection hi_hat_rock_breaks = {
  hi_hat_rock_breaks_arr, 1, 0
};

// Blues breaks
Rhythm hi_hat_blues_breaks_arr[] = {hi_hat_rhythm_triplets_1_3};
RhythmCollection hi_hat_blues_breaks = {
  hi_hat_blues_breaks_arr, 1, 0
};

// Jazz breaks
Rhythm hi_hat_jazz_breaks_arr[] = {hi_hat_break_standard};
RhythmCollection hi_hat_jazz_breaks = {
  hi_hat_jazz_breaks_arr, 1, 0
};

RhythmCollection hi_hat_breaks[] = {
  hi_hat_standard_breaks,
  hi_hat_rock_breaks,
  hi_hat_blues_breaks,
  hi_hat_jazz_breaks
};

Instrument hi_hat = {2, "Hi-Hat", 42, A2, hi_hat_rhythms, hi_hat_breaks};

/* Splash */
/* Splash rhythms */

// No splash rhythms yet
RhythmCollection* splash_rhythms = empty_rhythms;

/* Splash breaks */
const int splash_break_eigth_notes[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50
};
Rhythm splash_break_eigth = {
  "8", 4, 4, 8, splash_break_eigth_notes, 8
};
const int splash_break_4_4_notes[] = {
  0x00, 0x00, 0x00, 0x50
};
Rhythm splash_break_4_4 = {
  "4", 4, 4, 4, splash_break_4_4_notes, 4
};

// Standard breaks
Rhythm splash_standard_breaks_arr[] = {splash_break_eigth};
RhythmCollection splash_standard_breaks = {
  splash_standard_breaks_arr, 1, 0
};

// Rock breaks
Rhythm splash_rock_breaks_arr[] = {splash_break_4_4};
RhythmCollection splash_rock_breaks = {
  splash_rock_breaks_arr, 1, 0
};

// Blues breaks
Rhythm splash_blues_breaks_arr[] = {splash_break_4_4};
RhythmCollection splash_blues_breaks = {
  splash_blues_breaks_arr, 1, 0
};

// Jazz breaks
Rhythm splash_jazz_breaks_arr[] = {splash_break_4_4};
RhythmCollection splash_jazz_breaks = {
  splash_jazz_breaks_arr, 1, 0
};

RhythmCollection splash_breaks[] = {
  splash_standard_breaks,
  splash_rock_breaks,
  splash_blues_breaks,
  splash_jazz_breaks
};

Instrument splash = {3, "Splash", 49, A2, splash_rhythms, splash_breaks};

/* Instrument list */
Instrument instrs[] = {base_drum, snare_drum, hi_hat, splash};
const int instrument_count = 4;
#endif
