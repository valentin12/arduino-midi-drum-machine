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

// LCD display
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

// MIDI commands
const int NOTE_ON = 0x90;
const int CONTROL_CHANGE = 0xB0;
const int PROGRAM_CHANGE = 0xC0;
const int PITCH_BEND_CHANGE = 0xE0;

enum class Mode : int {
  STD,
  ROCK,
  BLUES
};

const int mode_count = 3;

const char mode_names[mode_count][32] = {"Standard", "Rock", "Blues"};

int last_bpm = 0;
int pre_last_bpm = 0;
int bpm;
const int bmp_pin = A3;

int last_pitch = 0;
int pre_last_pitch = 0;
int pitch;
const int pitch_pin = A0;

int drum_channel = 9;
const int subdivision = 48;
int max_bars = 96;
int mode = (int) Mode::ROCK;

int numerator = 4;
int denominator = 4;



/* Base drum */
const int base_drum_rhythm_4_4_notes[] = {0x70, 0x60, 0x60, 0x60};
Rhythm base_drum_rhythm_4_4 = {
  4, 4, 4, base_drum_rhythm_4_4_notes, 4
};
const int base_drum_rhythm_4_4_8bars_notes[] = {
  0x80, 0x60, 0x60, 0x60,
  0x70, 0x60, 0x60, 0x60,
  0x70, 0x60, 0x60, 0x60,
  0x70, 0x60, 0x60, 0x60,
  0x70, 0x60, 0x60, 0x60,
  0x70, 0x60, 0x60, 0x60,
  0x70, 0x60, 0x60, 0x60,
  0x70, 0x70, 0x70, 0x70
};
Rhythm base_drum_rhythm_4_4_8bars = {
  4, 4, 4, base_drum_rhythm_4_4_8bars_notes, 32
};
const int base_drum_rhythm_4_4_blues_notes[] = {
  0x7f, 0x60, 0x60, 0x60,
  0x7f, 0x60, 0x60, 0x60,
  0x7f, 0x60, 0x60, 0x60,
  0x7f, 0x60, 0x60, 0x60,
  0x7f, 0x60, 0x60, 0x60,
  0x7f, 0x60, 0x60, 0x60,
  0x7f, 0x60, 0x60, 0x60,
  0x7f, 0x60, 0x60, 0x60,
  0x7f, 0x60, 0x60, 0x60,
  0x7f, 0x60, 0x60, 0x60,
  0x7f, 0x60, 0x60, 0x60,
  0x7f, 0x70, 0x70, 0x70
};
Rhythm base_drum_rhythm_4_4_blues = {
  4, 4, 4, base_drum_rhythm_4_4_blues_notes, 48
};
Rhythm base_drum_rhythms[] = {base_drum_rhythm_4_4,
                              base_drum_rhythm_4_4_8bars,
                              base_drum_rhythm_4_4_blues};
Instrument base_drum = {36, A4, base_drum_rhythms, 3, mode, 0};

/* Snare drum */
const int snare_drum_rhythm_4_4_offbeat_notes[] = {0, 0x40, 0, 0x40};
Rhythm snare_drum_rhythm_4_4_offbeat = {
  4, 4, 4, snare_drum_rhythm_4_4_offbeat_notes, 4
};
const int snare_drum_rhythm_4_4_8bars_notes[] = {
  0, 0x40, 0, 0x40,
  0, 0x40, 0, 0x40,
  0, 0x40, 0, 0x40,
  0, 0x40, 0, 0x40,
  0, 0x40, 0, 0x40,
  0, 0x40, 0, 0x40,
  0, 0x40, 0, 0x40,
  0x50, 0x50, 0x50, 0x50
};
Rhythm snare_drum_rhythm_4_4_8bars = {
  4, 4, 4, snare_drum_rhythm_4_4_8bars_notes, 32
};
const int snare_drum_rhythm_4_4_blues_notes[] = {
  0, 0x40, 0, 0x40,
  0, 0x40, 0, 0x40,
  0, 0x40, 0, 0x40,
  0, 0x40, 0, 0x40,
  0, 0x40, 0, 0x40,
  0, 0x40, 0, 0x40,
  0, 0x40, 0, 0x40,
  0, 0x40, 0, 0x40,
  0, 0x40, 0, 0x40,
  0, 0x40, 0, 0x40,
  0, 0x40, 0, 0x40,
  0x50, 0x50, 0x50, 0x50
};
Rhythm snare_drum_rhythm_4_4_blues = {
  4, 4, 4, snare_drum_rhythm_4_4_blues_notes, 48
};
Rhythm snare_drum_rhythms[] = {snare_drum_rhythm_4_4_offbeat,
                               snare_drum_rhythm_4_4_8bars,
                               snare_drum_rhythm_4_4_blues};
Instrument snare_drum = {38, A1, snare_drum_rhythms, 3, mode, 0};

/* Hi-Hat */
const int hi_hat_rhythm_4_4_eights_notes[] = {
  0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48};
Rhythm hi_hat_rhythm_4_4_eights = {
  4, 4, 8, hi_hat_rhythm_4_4_eights_notes, 8
};
const int hi_hat_rhythm_4_4_8bars_notes[] = {
  0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48,
  0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48,
  0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48,
  0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x60, 0x00,
  0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48,
  0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48,
  0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48,
  0x60, 0x48, 0x60, 0x48, 0x60, 0x48, 0x60, 0x00
};
Rhythm hi_hat_rhythm_4_4_8bars = {
  4, 4, 8, hi_hat_rhythm_4_4_8bars_notes, 64
};
const int hi_hat_rhythm_4_4_blues_notes[] = {
  0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48,
  0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48,
  0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48,
  0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48,
  0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48,
  0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48,
  0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48,
  0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48,
  0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48,
  0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48,
  0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48,
  0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48,
};
Rhythm hi_hat_rhythm_4_4_blues = {
  4, 4, 12, hi_hat_rhythm_4_4_blues_notes, 144
};
Rhythm hi_hat_rhythms[] = {hi_hat_rhythm_4_4_eights,
                           hi_hat_rhythm_4_4_8bars,
                           hi_hat_rhythm_4_4_blues};
Instrument hi_hat = {42, A2, hi_hat_rhythms, 3, mode, 0};

/* Splash */
const int splash_rhythm_4_4_eigth_notes[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50
};
Rhythm splash_rhythm_4_4_eigth = {
  4, 4, 8, splash_rhythm_4_4_eigth_notes, 8
};
const int splash_rhythm_4_4_break_notes[] = {
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x50
};
Rhythm splash_rhythm_4_4_break = {
  4, 4, 4, splash_rhythm_4_4_break_notes, 32
};
const int splash_rhythm_4_4_blues_notes[] = {
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x50
};
Rhythm splash_rhythm_4_4_blues = {
  4, 4, 4, splash_rhythm_4_4_blues_notes, 48
};
Rhythm splash_rhythms[] = {splash_rhythm_4_4_eigth,
                           splash_rhythm_4_4_break,
                           splash_rhythm_4_4_blues};
Instrument splash = {49, A2, splash_rhythms, 3, mode, 0};

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

void computeStep(int step) {
  for (int i=0;i<instrument_count;i++) {
    Instrument instr = instrs[i];
    Rhythm r = instr.rhythms[instr.cur_rhythm];
    if (step % (subdivision / r.subdivision) != 0)
      continue;
    int local_step = step / (subdivision / r.subdivision) % r.note_count;
    if (r.notes[local_step] > 0) {
      int vol = r.notes[local_step] * (analogRead(instr.input_pin) / 1023.0);
      if (vol > 0) {
        sendMIDI(NOTE_ON | drum_channel, instr.midi_note, vol);
      }
      /*Serial.print(instr.midi_note);
      Serial.print(" ");
      Serial.print(r.notes[local_step]);
      Serial.print(" ");
      Serial.println(local_step);*/
    }
  }
}

void updateDisplay() {
  // clear display
  lcd.clear();
  lcd.setCursor(0, 0);
  // write mode name
  lcd.print(mode_names[mode]);
  lcd.setCursor(0, 1);
  lcd.print(bpm);
}

int step_counter;
void setup() {
  lcd.begin(16, 2);
  lcd.print("Setup");
  Serial.begin(115200);
  while(!Serial) ;
  step_counter = 0;
}

void loop() {
  if (step_counter > subdivision * max_bars - 1) step_counter = 0;
  computeStep(step_counter);
  step_counter++;
  bpm = map(analogRead(bmp_pin), 0, 1023, 10, 180);
  if (bpm != last_bpm) {
    if (pre_last_bpm != bpm) {
      updateDisplay();
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
