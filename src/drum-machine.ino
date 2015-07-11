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
int max_bars = 4;

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
    lcd.home();
    // write mode name
    lcd.print(mode_names[mode]);
    lcd.setCursor(0, 1);
    lcd.print("BPM: ");
    escapeLCDNum(bpm, 3);

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

  void computeLeft() {
    prevView();
  }

  void computeRight() {
    nextView();
  }
} main_view;

class SetRhythmView: public View {
  int cur_instr = 0;
  boolean edit = false;
  void updateDisplay() {
    // clear display
    lcd.clear();
    lcd.home();
    // write mode name
    lcd.print("Rhythm ");
    lcd.print(instrs[cur_instr].name);
    lcd.setCursor(0, 1);
    lcd.print(instrs[cur_instr].rhythms[mode].cur_rhythm + 1);
    lcd.print(": ");
    lcd.print(instrs[cur_instr].rhythms[mode].rhythms[
      instrs[cur_instr].rhythms[mode].cur_rhythm].name);

    if (edit) {
      // Edit mode
      lcd.setCursor(14, 1);
      lcd.print(" E");
    }
    displayBeat(step_counter, true);
  }

  void computeUp() {
    if (edit) {
      if (instrs[cur_instr].rhythms[mode].rhythm_count < 2) {
        return;
      }
      if (instrs[cur_instr].rhythms[mode].cur_rhythm + 1 <
          instrs[cur_instr].rhythms[mode].rhythm_count) {
        instrs[cur_instr].rhythms[mode].cur_rhythm++;
      }
      else {
        instrs[cur_instr].rhythms[mode].cur_rhythm = 0;
      }
      saveInstrument(instrs[cur_instr]);
    }
    else {
      if (cur_instr + 1 < instrument_count) {
        cur_instr++;
      }
      else {
        cur_instr = 0;
      }
    }
    updateDisplay();
  }

  void computeDown() {
    if (edit) {
      if (instrs[cur_instr].rhythms[mode].rhythm_count < 2) {
        return;
      }
      if (instrs[cur_instr].rhythms[mode].cur_rhythm - 1 >= 0) {
        instrs[cur_instr].rhythms[mode].cur_rhythm--;
      }
      else {
        instrs[cur_instr].rhythms[mode].cur_rhythm =
          instrs[cur_instr].rhythms[mode].rhythm_count - 1;
      }
      saveInstrument(instrs[cur_instr]);
    }
    else {
      if (cur_instr - 1 >= 0) {
        cur_instr--;
      }
      else {
        cur_instr = instrument_count - 1;
      }
    }
    updateDisplay();
  }

  void computeEnter() {
    edit = !edit;
    updateDisplay();
  }

  void computeLeft() {
    edit = false;
    prevView();
  }

  void computeRight() {
    edit = false;
    nextView();
  }
} set_rhythm_view;

class SetBreakView: public View {
  int cur_instr = 0;
  boolean edit = false;
  void updateDisplay() {
    // clear display
    lcd.clear();
    lcd.home();
    // write mode name
    lcd.print("Break ");
    lcd.print(instrs[cur_instr].name);
    lcd.setCursor(0, 1);
    lcd.print(instrs[cur_instr].breaks[mode].cur_rhythm + 1);
    lcd.print(": ");
    lcd.print(instrs[cur_instr].breaks[mode].rhythms[
      instrs[cur_instr].breaks[mode].cur_rhythm].name);

    if (edit) {
      // Edit mode
      lcd.setCursor(14, 1);
      lcd.print(" E");
    }
    displayBeat(step_counter, true);
  }

  void computeUp() {
    if (edit) {
      if (instrs[cur_instr].breaks[mode].rhythm_count < 2) {
        return;
      }
      if (instrs[cur_instr].breaks[mode].cur_rhythm + 1 <
          instrs[cur_instr].breaks[mode].rhythm_count) {
        instrs[cur_instr].breaks[mode].cur_rhythm++;
      }
      else {
        instrs[cur_instr].breaks[mode].cur_rhythm = 0;
      }
      saveInstrument(instrs[cur_instr]);
    }
    else {
      if (cur_instr + 1 < instrument_count) {
        cur_instr++;
      }
      else {
        cur_instr = 0;
      }
    }
    updateDisplay();
  }

  void computeDown() {
    if (edit) {
      if (instrs[cur_instr].breaks[mode].rhythm_count < 2) {
        return;
      }
      if (instrs[cur_instr].breaks[mode].cur_rhythm - 1 >= 0) {
        instrs[cur_instr].breaks[mode].cur_rhythm--;
      }
      else {
        instrs[cur_instr].breaks[mode].cur_rhythm =
          instrs[cur_instr].breaks[mode].rhythm_count - 1;
      }
      saveInstrument(instrs[cur_instr]);
    }
    else {
      if (cur_instr - 1 >= 0) {
        cur_instr--;
      }
      else {
        cur_instr = instrument_count - 1;
      }
    }
    updateDisplay();
  }

  void computeEnter() {
    edit = !edit;
    updateDisplay();
  }

  void computeLeft() {
    edit = false;
    prevView();
  }

  void computeRight() {
    edit = false;
    nextView();
  }
} set_break_view;

const int view_count=3;
int view_index=0;
View* views[view_count] = {
  &main_view,
  &set_rhythm_view,
  &set_break_view
};
View* cur_view = views[view_index];


void sendMIDI(const int cmd, const int note, const int velocity) {
  Serial.write(cmd);
  Serial.write(note);
  Serial.write(velocity);
};

void sendShortMIDI(const int cmd, const int val) {
  Serial.write(cmd);
  Serial.write(val);
}

void escapeLCDNum(const int number, const int max_digits) {
  for (int i=max_digits-1;number < pow(10, i);i--) {
    lcd.print(" ");
  }
  lcd.print(number);
}

void displayBeat(const int step, const boolean force_redraw) {
  int local_step = step / (subdivision / numerator) % numerator;
  if (force_redraw || step == 0 ||
      local_step != (step -1) / (subdivision / numerator) % numerator) {
    lcd.setCursor(14, 0);
    escapeLCDNum(local_step + 1, 2);
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
          r = breaks.rhythms[breaks.cur_rhythm];
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

void nextView() {
  if (view_index + 1 < view_count) {
    view_index++;
  }
  else {
    view_index = 0;
  }
  cur_view = views[view_index];
  cur_view->updateDisplay();
}

void prevView() {
  if (view_index - 1 >= 0) {
    view_index--;
  }
  else {
    view_index = view_count - 1;
  }
  cur_view = views[view_index];
  cur_view->updateDisplay();
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
  if (step_counter >= subdivision * max_bars - 1) step_counter = 0;
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
