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

// Pin to display beat
const int metronome_pin = 13;

// Joystick
const char up_pin = 2;
const char down_pin = 3;
const char left_pin = 4;
const char right_pin = 5;
const char enter_pin = 6;

// break button
const char break_pin = 22;

// mute switche
const char mute_switch_pin = 23;

// MIDI commands
const unsigned char NOTE_ON = 0x90;
const unsigned char CONTROL_CHANGE = 0xB0;
const unsigned char PROGRAM_CHANGE = 0xC0;
const unsigned char PITCH_BEND_CHANGE = 0xE0;


// Styles
enum class Mode : int {
  STD,
  ROCK,
  BLUES,
  JAZZ,
  WALTZ
};
const unsigned char mode_count = 5;
const char mode_names[mode_count][17] = {
  "Standard", "Rock", "Blues", "Jazz", "Waltz"
};
boolean mode_break_mute[] = {
  true, true, true, true, true
};

// last joystick input
boolean last_up;
boolean last_down;
boolean last_left;
boolean last_right;
boolean last_enter;

// break input variables
boolean last_break = false;
boolean is_break = false;

// mute input variables
boolean muted = false;
boolean last_muted = false;

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

// volume
int last_vol = 0;
int pre_last_vol = 0;
int vol;
const int vol_pin = A8;

unsigned char drum_channel = 9;

// subdivision of one bar
const int subdivision = 96;
int max_bars = 96;

long step_counter;

int mode = (int) Mode::STD;

int numerator = 4;
int denominator = 4;

// EEPROM addresses
const int mode_pos = 0;
const int instruments_pos = 256;

// last status byte to implement MIDI running status
unsigned char last_status_byte = 0;

// Used to access data from a r_func only needed one time
unsigned char tmp_rhythm_notes[RHYTHM_MAX_NOTES] = {0};
Rhythm tmp_rhythm = {
  "", 4, 4, 1, tmp_rhythm_notes, 0
};

// SCREENS
class MainView: public View {
  void updateDisplay() {
    // clear display
    lcd.clear();
    lcd.home();
    if (muted) {
      lcd.print("#");
    }
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
    // get current rhythm data from r_func
    lcd.print(instrs[cur_instr].cur_rhythm.name);

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
      instrs[cur_instr].rhythms[mode].rhythms[
        instrs[cur_instr].rhythms[mode].cur_rhythm](&instrs[cur_instr].cur_rhythm);
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
      instrs[cur_instr].rhythms[mode].rhythms[
        instrs[cur_instr].rhythms[mode].cur_rhythm](&instrs[cur_instr].cur_rhythm);
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
    lcd.print(instrs[cur_instr].cur_break.name);

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
      instrs[cur_instr].breaks[mode].rhythms[
        instrs[cur_instr].breaks[mode].cur_rhythm](&instrs[cur_instr].cur_break);
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
      instrs[cur_instr].breaks[mode].rhythms[
        instrs[cur_instr].breaks[mode].cur_rhythm](&instrs[cur_instr].cur_break);
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
  // if (cmd != last_status_byte)
  Serial1.write(cmd);
  Serial1.write(note);
  Serial1.write(velocity);
  // if (cmd != last_status_byte)
  //  Serial.write(cmd);
  // do not use running status over USB. The serial to MIDI
  // converter has problems with it
  Serial.write(cmd);
  Serial.write(note);
  Serial.write(velocity);
  last_status_byte = cmd;
};

void sendShortMIDI(const int cmd, const int val) {
  if (cmd != last_status_byte)
    Serial1.write(cmd);
  Serial1.write(val);
  // if (cmd != last_status_byte)
  //   Serial.write(cmd);
  // see above
  Serial.write(cmd);
  Serial.write(val);
  last_status_byte = cmd;
}

void escapeLCDNum(const int number, const int max_digits) {
  for (int i=max_digits-1;number < pow(10, i);i--) {
    lcd.print(" ");
  }
  lcd.print(number);
}

void displayBeat(const int step, const boolean force_redraw) {
  int local_step = (step / subdivision) % numerator;
  if (force_redraw || step == 0 ||
      local_step != ((step - 1) / subdivision) % numerator) {
    // LCD
    lcd.setCursor(14, 0);
    escapeLCDNum(local_step + 1, 2);
  }

  // LED
  if (step == 0 ||
      local_step != ((step - 1) / subdivision) % numerator) {
    if (local_step == 0)
      analogWrite(metronome_pin, 0xff);
    else
      analogWrite(metronome_pin, 0x20);
  }
  else {
    int part = 4;
    if (local_step == 0)
      // decrease to enlight first beat longer
      part = 4;
    if (step % subdivision  > subdivision / part) {
      analogWrite(metronome_pin, 0);
    }
  }
}

void computeStep(int step) {
  if (muted) {
    return;
  }
  for (int i=0;i<instrument_count;i++) {
    Instrument instr = instrs[i];
    for (int l=0;l<2;l++) {
      Rhythm r;
      if (l==0) {
        if (is_break && mode_break_mute[mode]) {
          continue;
        }
        else {
          r = instr.cur_rhythm;
        }
      }
      else {
        if (is_break) {
          r = instr.cur_break;
        }
        else {
          continue;
        }
      }
      if (!isLocalStep(step, r.subdivision) || r.note_count == 0)
        continue;
      int local_step = getLocalStep(step, r.subdivision, r.note_count);
      if (r.notes[local_step] > 0) {
        int note_vol = r.notes[local_step] * (analogRead(instr.input_pin) / 1023.0);
        if (vol > 0 && note_vol > 0) {
          sendMIDI(NOTE_ON | drum_channel, instr.midi_note, note_vol);
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
  last_break = is_break;
  is_break = !digitalRead(break_pin);
  if (is_break != last_break) {
    return true;
  }
  return false;
}

boolean computeMuteSwitch() {
  last_muted = muted;
  muted = !digitalRead(mute_switch_pin);
  if (muted != last_muted) {
    return true;
  }
  return false;
}

long getDelay(long sub) {
  /* sub: Subtract from delay */
  // 1 minute (60000 milliseconds)
  // divide by beats per minute
  // divide by subdivision
  long delay_time = 60000 / bpm / subdivision - sub;
  if (delay_time > 0)
    return delay_time;
  return 0;
}

int getLocalStep(int global_step, int r_subdiv, int r_note_count) {
  /*
   * Step: global step
   * r_subdiv: Subdivision of rhythm
   * r_note_count: Number of notes in rhythm
  */
  // global step (beats since start * subdivision)
  return global_step / (subdivision / (r_subdiv / denominator)) % r_note_count;
}
boolean isLocalStep(int global_step, int r_subdiv) {
  /*
   * Step: global step
   * r_subdiv: Subdivision of rhythm
  */
  return r_subdiv != 0
         && global_step % (subdivision / (r_subdiv / denominator)) == 0;
}

void updateRhythms() {
  for (int i=0;i<instrument_count;i++) {
    instrs[i].rhythms[mode].rhythms[
      instrs[i].rhythms[mode].cur_rhythm](&instrs[i].cur_rhythm);
    instrs[i].breaks[mode].rhythms[
      instrs[i].breaks[mode].cur_rhythm](&instrs[i].cur_break);
  }
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
  if (new_mode == (int) Mode::WALTZ) {
    numerator = 3;
    denominator = 4;
  }
  else {
    numerator = 4;
    denominator = 4;
  }
  if (mode != new_mode) {
    mode = new_mode;
    EEPROM_update(mode_pos, mode);
    updateRhythms();
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
    if (instr->rhythms[i].cur_rhythm >= instr->rhythms[i].rhythm_count) {
      instr->rhythms[i].cur_rhythm = 0;
      EEPROM_update(cur_pos + i, 0);
    }
  }
  instr->rhythms[mode].rhythms[
    instr->rhythms[mode].cur_rhythm](&instr->cur_rhythm);
  cur_pos += MAX_MODES;
  for (int i=0;i<mode_count;i++) {
    instr->breaks[i].cur_rhythm = EEPROM.read(cur_pos + i);
    if (instr->breaks[i].cur_rhythm >= instr->breaks[i].rhythm_count) {
      instr->breaks[i].cur_rhythm = 0;
      EEPROM_update(cur_pos + i, 0);
    }
  }
  instr->breaks[mode].rhythms[
    instr->breaks[mode].cur_rhythm](&instr->cur_break);
  cur_pos += MAX_MODES;
}


void setup() {
  pinMode(up_pin, INPUT_PULLUP);
  pinMode(down_pin, INPUT_PULLUP);
  pinMode(left_pin, INPUT_PULLUP);
  pinMode(right_pin, INPUT_PULLUP);
  pinMode(enter_pin, INPUT_PULLUP);

  pinMode(break_pin, INPUT_PULLUP);
  pinMode(mute_switch_pin, INPUT_PULLUP);

  pinMode(metronome_pin, OUTPUT);

  lcd.begin(16, 2);
  lcd.print("Setup");
  // Setup Serial (TX0 and USB) with the baudrate 115200 to be able to use
  // an Serial to MIDI converter on a PC
  Serial.begin(115200);
  while(!Serial) ;
  // Setup Serial1 with the standard MIDI baud rate of 31250
  // to get MIDI on TX1 (pin 18)
  Serial1.begin(31250);
  while(!Serial1) ;
  step_counter = 0;

  // Read EEPROM content
  mode = getMode();
  setMode(mode);
  for (int i=0;i<instrument_count;i++) {
    restoreInstrument(&instrs[i]);
  }
  muted = !digitalRead(mute_switch_pin);
}


unsigned long start_millis;

void loop() {
  start_millis = millis();
  if (step_counter > subdivision * max_bars - 1) step_counter = 0;
  computeBreakSwitch();
  if (computeMuteSwitch()) {
    cur_view->updateDisplay();
  }
  computeStep(step_counter);
  displayBeat(step_counter, false);
  step_counter++;

  computeJoystick();

  vol = map(analogRead(vol_pin), 0, 1023, 0, 0x7f);
  if (vol != last_vol) {
    if (pre_last_vol != vol) {
      sendMIDI(CONTROL_CHANGE | drum_channel, 0x07, vol);
    }
    pre_last_vol = last_vol;
    last_vol = vol;
  }
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
  delay(getDelay(millis() - start_millis));
}
