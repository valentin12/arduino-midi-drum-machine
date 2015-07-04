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

// MIDI commands
const int NOTE_ON = 0x90;
const int CONTROL_CHANGE = 0xB0;
const int PROGRAM_CHANGE = 0xC0;
const int PITCH_BEND_CHANGE = 0xE0;

int bpm = 120;
int drum_channel = 9;
const int subdivision = 8;
int max_bars = 16;

int numerator = 4;
int denominator = 4;

/* Base drum */
const int base_drum_rhythm_4_4_notes[] = {0x70, 0x60, 0x60, 0x60};
Rhythm base_drum_rhythm_4_4 = {
  4, 4, 4, base_drum_rhythm_4_4_notes, 4
};
const int base_drum_rhythm_4_4_8bars_notes[] = {
  0x70, 0x60, 0x60, 0x60,
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
Rhythm base_drum_rhythms[] = {base_drum_rhythm_4_4,
                              base_drum_rhythm_4_4_8bars};
Instrument base_drum = {36, A0, base_drum_rhythms, 2, 1, 0};

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
Rhythm snare_drum_rhythms[] = {snare_drum_rhythm_4_4_offbeat,
                               snare_drum_rhythm_4_4_8bars};
Instrument snare_drum = {38, A1, snare_drum_rhythms, 2, 1, 0};

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
Rhythm hi_hat_rhythms[] = {hi_hat_rhythm_4_4_eights,
                           hi_hat_rhythm_4_4_8bars};
Instrument hi_hat = {42, A2, hi_hat_rhythms, 2, 1, 0};

/* Splash */
const int splash_rhythm_4_4_break_notes[] = {
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x60
};
Rhythm splash_rhythm_4_4_break = {
  4, 4, 4, splash_rhythm_4_4_break_notes, 32
};
Rhythm splash_rhythms[] = {splash_rhythm_4_4_break};
Instrument splash = {49, A3, splash_rhythms, 1, 0, 0};

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

int step_counter;
void setup() {
  Serial.begin(115200);
  while(!Serial) ;
  step_counter = 0;
}

void loop() {
  if (step_counter > subdivision * max_bars - 1) step_counter = 0;
  computeStep(step_counter);
  step_counter++;
  delay(60000 / (bpm * subdivision / numerator));
}
