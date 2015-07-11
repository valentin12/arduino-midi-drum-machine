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
#include "drum-machine.h"
#include <LiquidCrystal.h>
#include <EEPROM.h>

// Patch EEPROM
void EEPROM_update(int pos, int data) {
  if (EEPROM.read(pos) != data) {
    EEPROM.write(pos, data);
  }
}

// LCD display
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

// Joystick
const int up_pin = 2;
const int down_pin = 3;
const int left_pin = 4;
const int right_pin = 5;
const int enter_pin = 6;

// break button
const int break_pin = 22;

// MIDI commands
const int NOTE_ON = 0x90;
const int CONTROL_CHANGE = 0xB0;
const int PROGRAM_CHANGE = 0xC0;
const int PITCH_BEND_CHANGE = 0xE0;


// Styles
enum class Mode : int {
  STD,
  ROCK,
  BLUES,
  JAZZ
};
const int mode_count = 4;
const char mode_names[mode_count][32] = {
  "Standard", "Rock", "Blues", "Jazz"
};
boolean mode_break_mute[] = {
  true, true, true, true
};

// last joystick input
boolean last_up;
boolean last_down;
boolean last_left;
boolean last_right;
boolean last_enter;

// break input variables
boolean last_break;
boolean is_break;

// beats per minute
int last_bpm = 0;
int pre_last_bpm = 0;
int bpm;
const int bmp_pin = A3;

// pitch
int last_pitch = 0;
int pre_last_pitch = 0;
int pitch;
const int pitch_pin = A0;

int drum_channel = 9;

// subdivision of one bar
const int subdivision = 48;
int max_bars = 96;

int step_counter;

int mode = (int) Mode::STD;

int numerator = 4;
int denominator = 4;

// EEPROM addresses
const int mode_pos=0;
const int instruments_pos=1024;

// SCREENS
class MainView: public View {
  void updateDisplay() {
    // clear display
    lcd.clear();
    lcd.setCursor(0, 0);
    // write mode name
    lcd.print(mode_names[mode]);
    lcd.setCursor(0, 1);
    lcd.print("BPM: ");
    lcd.print(bpm);

    displayBeat(step_counter, true);
  }

  void computeUp() {
    if (mode + 1 < mode_count) {
      setMode(mode + 1);
    }
    else {
      setMode(0);
    }
    updateDisplay();
  }

  void computeDown() {
    if (mode > 0) {
      setMode(mode - 1);
    }
    else {
      setMode(mode_count - 1);
    }
    updateDisplay();
  }
} main_view;

class DebugJoystickView: public View {
  void updateDisplay() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Joystick Debug");
  };
  void computeUp() {Serial.println("Up");};
  void computeDown() {Serial.println("Down");};
  void computeLeft() {Serial.println("Left");}
  void computeRight() {Serial.println("Right");}
  void computeEnter() {Serial.println("Enter");}
} debug_joystick_view;

View *cur_view = &main_view;

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
  "Base 1-4", 4, 4, 4, base_drum_rhythm_4_4_notes, 4
};

// Play in triplets, but only on first and last
const int base_drum_rhythm_4_4_jazz_notes[] = {
  0x70, 0x00, 0x60, 0x00, 0x00, 0x00, 0x70, 0x00, 0x60, 0x00, 0x00, 0x00
};
Rhythm base_drum_rhythm_4_4_jazz = {
  "Base one 'let", 4, 4, 12, base_drum_rhythm_4_4_jazz_notes, 12
};

// Standard rhythms
Rhythm base_standard_rhythms_arr[] = {base_drum_rhythm_4_4};
RhythmCollection base_standard_rhythms = {
  base_standard_rhythms_arr, 1, 0
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


void sendMIDI(const int cmd, const int note, const int velocity) {
  Serial.write(cmd);
  Serial.write(note);
  Serial.write(velocity);
};

void sendShortMIDI(const int cmd, const int val) {
  Serial.write(cmd);
  Serial.write(val);
}

void displayBeat(const int step, const boolean force_redraw) {
  int local_step = step / (subdivision / numerator) % numerator;
  if (force_redraw ||
      local_step != (step -1) / (subdivision / numerator) % numerator) {
    lcd.setCursor(14, 0);
    lcd.print(local_step + 1);
  }
}

void computeStep(int step) {
  for (int i=0;i<instrument_count;i++) {
    Instrument instr = instrs[i];
    RhythmCollection rhythms = instr.rhythms[mode];
    RhythmCollection breaks = instr.breaks[mode];
    for (int l=0;l<2;l++) {
      Rhythm r;
      if (l==0) {
        if (is_break && mode_break_mute[mode]) {
          continue;
        }
        else {
          r = rhythms.rhythms[rhythms.cur_rhythm];
        }
      }
      else {
        if (is_break) {
          r = breaks.rhythms[rhythms.cur_rhythm];
        }
        else {
          continue;
        }
      }
      if (step % (subdivision / r.subdivision) != 0)
        continue;
      int local_step = step / (subdivision / r.subdivision) % r.note_count;
      if (r.notes[local_step] > 0) {
        int vol = r.notes[local_step] * (analogRead(instr.input_pin) / 1023.0);
        if (vol > 0) {
          sendMIDI(NOTE_ON | drum_channel, instr.midi_note, vol);
        }
      }
    }
  }
}

boolean computeJoystick() {
  /* Returns wether change occured */
  boolean changed = false;
  // UP
  boolean inp = digitalRead(up_pin);
  // Inputs are connected to pullup
  if (!inp && last_up) {
    // changed to HIGH
    changed = true;
    cur_view->computeUp();
  }
  last_up = inp;
  // DOWN
  inp = digitalRead(down_pin);
  if (!inp && last_down) {
    // changed to HIGH
    changed = true;
    cur_view->computeDown();
  }
  last_down = inp;
  // LEFT
  inp = digitalRead(left_pin);
  if (!inp && last_left) {
    // changed to HIGH
    changed = true;
    cur_view->computeLeft();
  }
  last_left = inp;
  // RIGHT
  inp = digitalRead(right_pin);
  if (!inp && last_right) {
    // changed to HIGH
    changed = true;
    cur_view->computeRight();
  }
  last_right = inp;
  // ENTER
  inp = digitalRead(enter_pin);
  if (!inp && last_enter) {
    // changed to HIGH
    changed = true;
    cur_view->computeEnter();
  }
  last_enter = inp;
  return changed;
}

boolean computeBreakSwitch() {
  is_break = !digitalRead(break_pin);
  if (is_break != last_break) {
    return true;
  }
  return false;
}

/* Getter and setter (for EEPROM) */
int getMode() {
  int mode_eeprom = EEPROM.read(mode_pos);
  if (mode_eeprom >= 0 && mode_eeprom < mode_count) {
    return mode_eeprom;
  }
  return 0;
}

void setMode(int new_mode) {
  if (mode != new_mode) {
    mode = new_mode;
    EEPROM_update(mode_pos, mode);
  }
}

void saveInstrument(Instrument instr) {
  int cur_pos = instruments_pos + instr.uid * INSTR_STORE_MAX_SIZE;
  EEPROM_update(cur_pos, instr.uid);
  cur_pos++;
  // current rhythms for every mode in the instrument
  for (int i=0;i<mode_count;i++) {
    EEPROM_update(cur_pos + i, instr.rhythms[i].cur_rhythm);
  }
  cur_pos += MAX_MODES;
  // current break for every mode in the instrument
  for (int i=0;i<mode_count;i++) {
    EEPROM_update(cur_pos + i, instr.breaks[i].cur_rhythm);
  }
  cur_pos += MAX_MODES;
}

void restoreInstrument(Instrument* instr) {
  int cur_pos = instruments_pos + instr->uid * INSTR_STORE_MAX_SIZE;
  if (EEPROM.read(cur_pos) != instr->uid) {
    // Last write didn't come from this instrument -> don't change anything
    return;
  }
  cur_pos++;
  for (int i=0;i<mode_count;i++) {
    instr->rhythms[i].cur_rhythm = EEPROM.read(cur_pos + i);
  }
  cur_pos += MAX_MODES;
  for (int i=0;i<mode_count;i++) {
    instr->breaks[i].cur_rhythm = EEPROM.read(cur_pos + i);
  }
  cur_pos += MAX_MODES;
}


void setup() {
  pinMode(up_pin, INPUT_PULLUP);
  pinMode(down_pin, INPUT_PULLUP);
  pinMode(left_pin, INPUT_PULLUP);
  pinMode(right_pin, INPUT_PULLUP);
  pinMode(enter_pin, INPUT_PULLUP);

  pinMode(break_pin, INPUT_PULLUP);

  lcd.begin(16, 2);
  lcd.print("Setup");
  Serial.begin(115200);
  while(!Serial) ;
  step_counter = 0;

  // Read EEPROM content
  mode = getMode();
  for (int i=0;i<instrument_count;i++) {
    restoreInstrument(&instrs[i]);
  }
}

void loop() {
  if (step_counter > subdivision * max_bars - 1) step_counter = 0;
  computeBreakSwitch();
  computeStep(step_counter);
  displayBeat(step_counter, false);
  step_counter++;

  computeJoystick();

  bpm = map(analogRead(bmp_pin), 0, 1023, 10, 220);
  if (bpm != last_bpm) {
    if (pre_last_bpm != bpm) {
      cur_view->updateDisplay();
    }
    pre_last_bpm = last_bpm;
    last_bpm = bpm;
  }
  pitch = map(analogRead(pitch_pin), 0, 1023, 0, 0x7f);
  if (pitch != last_pitch) {
    if (pre_last_pitch != pitch) {
      sendMIDI(PITCH_BEND_CHANGE | drum_channel, 0, pitch);
    }
    pre_last_pitch = last_pitch;
    last_pitch = pitch;
  }
  delay(60000 / (bpm * subdivision / numerator));
}
