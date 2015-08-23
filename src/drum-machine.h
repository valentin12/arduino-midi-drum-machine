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

const int RHYTHM_MAX_NOTES = 128;

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
  unsigned char* notes;
  int note_count; /* length of *notes* */
};

// Functions to set parameters of a Rhythm instance.
// All rhythm data in local function -> less RAM usage
typedef void (*r_func)(Rhythm*);

struct RhythmCollection {
  r_func* rhythms;
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

  Rhythm cur_rhythm;
  Rhythm cur_break;
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
boolean computeMuteSwitch();
long getDelay(long);
int getLocalStep(int, int, int);
boolean isLocalStep(int, int);
void updateRhythms();
// getter and setter (for EEPROM)
int getMode();
void setMode(int);
// (de)serializer for EEPROM
void saveInstrument(Instrument);
void restoreInstrument(Instrument*);


// INSTRUMENTS AND RHYTHMS
void empty_rhythm(Rhythm* r) {
  const unsigned char notes[] = {0x00};
  strcpy(r->name, "None");
  r->numerator = 4;
  r->denominator = 4;
  r->subdivision = 1;
  r->note_count = 1;
  for (int n=0;n < r->note_count;n++) {
    r->notes[n] = notes[n];
  }
}

r_func empty_rhythm_collection_arr[] {
  empty_rhythm
};
RhythmCollection empty_rhythm_collection = {
  empty_rhythm_collection_arr, 1, 0
};
RhythmCollection empty_rhythms[] = {
  empty_rhythm_collection,
  empty_rhythm_collection,
  empty_rhythm_collection,
  empty_rhythm_collection,
  empty_rhythm_collection
};

/* Bass drum */
/* Bass drum rhythms */
void bass_drum_rhythm_4_4(Rhythm* r) {
  const unsigned char notes[] = {
    0x75, 0x60, 0x60, 0x60
  };
  strcpy(r->name, "1-4");
  r->numerator = 4;
  r->denominator = 4;
  r->subdivision = 4;
  r->note_count = 4;
  for (int n=0;n < r->note_count;n++) {
    r->notes[n] = notes[n];
  }
}

void bass_drum_rhythm_offbeat(Rhythm* r) {
  const unsigned char notes[] = {
    0x00, 0x60, 0x00, 0x60
  };
  strcpy(r->name, "Off Beat");
  r->numerator = 4;
  r->denominator = 4;
  r->subdivision = 4;
  r->note_count = 4;
  for (int n=0;n < r->note_count;n++) {
    r->notes[n] = notes[n];
  }
}

void bass_drum_rhythm_beat(Rhythm* r) {
  const unsigned char notes[] = {
    0x75, 0x00, 0x60, 0x00
  };
  strcpy(r->name, "1+3");
  r->numerator = 4;
  r->denominator = 4;
  r->subdivision = 4;
  r->note_count = 4;
  for (int n=0;n < r->note_count;n++) {
    r->notes[n] = notes[n];
  }
}

void bass_drum_rhythm_eigth_feel(Rhythm* r) {
  const unsigned char notes[] = {
    0x75, 0x00, 0x00, 0x60, 0x60, 0x00, 0x00, 0x00
  };
  strcpy(r->name, "1+2(1/2)+3");
  r->numerator = 4;
  r->denominator = 4;
  r->subdivision = 8;
  r->note_count = 8;
  for (int n=0;n < r->note_count;n++) {
    r->notes[n] = notes[n];
  }
}

void bass_drum_rhythm_linear(Rhythm* r) {
  const unsigned char notes[] = {
    0x75, 0x00, 0x00, 0x60, 0x00, 0x00, 0x60, 0x00
  };
  strcpy(r->name, "1+2(1/2)+4");
  r->numerator = 4;
  r->denominator = 4;
  r->subdivision = 8;
  r->note_count = 8;
  for (int n=0;n < r->note_count;n++) {
    r->notes[n] = notes[n];
  }
}

// Play in triplets, but only on first and last
void bass_drum_rhythm_4_4_jazz(Rhythm* r) {
  const unsigned char notes[] = {
    0x70, 0x00, 0x60, 0x00, 0x00, 0x00, 0x70, 0x00, 0x60, 0x00, 0x00, 0x00
  };
  strcpy(r->name, "one 'let");
  r->numerator = 4;
  r->denominator = 4;
  r->subdivision = 12;
  r->note_count = 12;
  for (int n=0;n < r->note_count;n++) {
    r->notes[n] = notes[n];
  }
}
// 3/4
void bass_drum_rhythm_3_4(Rhythm* r) {
  const unsigned char notes[] = {
    0x75, 0x60, 0x60
  };
  strcpy(r->name, "3/4 1-3");
  r->numerator = 3;
  r->denominator = 4;
  r->subdivision = 4;
  r->note_count = 3;
  for (int n=0;n < r->note_count;n++) {
    r->notes[n] = notes[n];
  }
}

// Standard rhythms
r_func bass_standard_rhythms_arr[] = {
  bass_drum_rhythm_4_4,
  bass_drum_rhythm_offbeat,
  bass_drum_rhythm_beat,
  bass_drum_rhythm_eigth_feel,
  bass_drum_rhythm_linear
};
RhythmCollection bass_standard_rhythms = {
  bass_standard_rhythms_arr, 5, 0
};

// Rock rhythms
r_func bass_rock_rhythms_arr[] = {
  bass_drum_rhythm_4_4,
  bass_drum_rhythm_offbeat,
  bass_drum_rhythm_beat,
  bass_drum_rhythm_eigth_feel
};
RhythmCollection bass_rock_rhythms = {
  bass_rock_rhythms_arr, 4, 0
};

// Blues rhythms
r_func bass_blues_rhythms_arr[] = {
  bass_drum_rhythm_4_4,
  bass_drum_rhythm_offbeat,
  bass_drum_rhythm_beat
};
RhythmCollection bass_blues_rhythms = {
  bass_blues_rhythms_arr, 3, 0
};

// Jazz rhythms
r_func bass_jazz_rhythms_arr[] = {
  bass_drum_rhythm_4_4_jazz,
  bass_drum_rhythm_4_4,
  bass_drum_rhythm_offbeat,
  bass_drum_rhythm_beat
};
RhythmCollection bass_jazz_rhythms = {
  bass_jazz_rhythms_arr, 4, 0
};

// Waltz rhythms
r_func bass_waltz_rhythms_arr[] = {
  bass_drum_rhythm_3_4
};
RhythmCollection bass_waltz_rhythms = {
  bass_waltz_rhythms_arr, 1, 0
};

RhythmCollection bass_rhythms[] = {
  bass_standard_rhythms,
  bass_rock_rhythms,
  bass_blues_rhythms,
  bass_jazz_rhythms,
  bass_waltz_rhythms
};

/* bass drum breaks */

// Standard breaks
r_func bass_standard_breaks_arr[] = {bass_drum_rhythm_4_4};
RhythmCollection bass_standard_breaks = {
  bass_standard_breaks_arr, 1, 0
};

// Rock breaks
r_func bass_rock_breaks_arr[] = {bass_drum_rhythm_4_4};
RhythmCollection bass_rock_breaks = {
  bass_rock_breaks_arr, 1, 0
};

// Blues breaks
r_func bass_blues_breaks_arr[] = {bass_drum_rhythm_4_4};
RhythmCollection bass_blues_breaks = {
  bass_blues_breaks_arr, 1, 0
};

// Jazz breaks
r_func bass_jazz_breaks_arr[] = {bass_drum_rhythm_4_4};
RhythmCollection bass_jazz_breaks = {
  bass_jazz_breaks_arr, 1, 0
};

// Waltz breaks
r_func bass_waltz_breaks_arr[] = {bass_drum_rhythm_3_4};
RhythmCollection bass_waltz_breaks = {
  bass_waltz_breaks_arr, 1, 0
};

RhythmCollection bass_breaks[] = {
  bass_standard_breaks,
  bass_rock_breaks,
  bass_blues_breaks,
  bass_jazz_breaks,
  bass_waltz_breaks
};

unsigned char bass_drum_cur_rhythm_notes[RHYTHM_MAX_NOTES] = {0};
Rhythm bass_drum_cur_rhythm = {
  "", 4, 4, 1, bass_drum_cur_rhythm_notes, 0
};
unsigned char bass_drum_cur_break_notes[RHYTHM_MAX_NOTES] = {0};
Rhythm bass_drum_cur_break = {
  "", 4, 4, 1, bass_drum_cur_break_notes, 0
};
Instrument bass_drum = {
  0, "Bass Drum", 36, A4, bass_rhythms, bass_breaks,
  bass_drum_cur_rhythm, bass_drum_cur_break
};

/* Snare drum */
/* Snare drum rhythms */
void snare_drum_rhythm_4_4_offbeat(Rhythm* r) {
  const unsigned char notes[] = {
    0, 0x40, 0, 0x40
  };
  strcpy(r->name, "Off Beat");
  r->numerator = 4;
  r->denominator = 4;
  r->subdivision = 4;
  r->note_count = 4;
  for (int n=0;n < r->note_count;n++) {
    r->notes[n] = notes[n];
  }
}

void snare_drum_rhythm_4_4(Rhythm* r) {
  const unsigned char notes[] = {
    0x75, 0x60, 0x60, 0x60
  };
  strcpy(r->name, "1-4");
  r->numerator = 4;
  r->denominator = 4;
  r->subdivision = 4;
  r->note_count = 4;
  for (int n=0;n < r->note_count;n++) {
    r->notes[n] = notes[n];
  }
}

void snare_drum_rhythm_4_4_jazz(Rhythm* r) {
  const unsigned char notes[] = {
    0x00, 0x00, 0x00, 0x70, 0x00, 0x60, 0x00, 0x00, 0x00, 0x70, 0x00, 0x60
  };
  strcpy(r->name, "2+4: 1+3");
  r->numerator = 4;
  r->denominator = 4;
  r->subdivision = 12;
  r->note_count = 12;
  for (int n=0;n < r->note_count;n++) {
    r->notes[n] = notes[n];
  }
}

// 3/4
void snare_drum_rhythm_3_4_waltz_offbeat(Rhythm* r) {
  const unsigned char notes[] = {
    0x00, 0x60, 0x60
  };
  strcpy(r->name, "3/4 2+3");
  r->numerator = 3;
  r->denominator = 4;
  r->subdivision = 4;
  r->note_count = 3;
  for (int n=0;n < r->note_count;n++) {
    r->notes[n] = notes[n];
  }
}

// Standard rhythms
r_func snare_standard_rhythms_arr[] = {
  snare_drum_rhythm_4_4_offbeat,
  snare_drum_rhythm_4_4
};
RhythmCollection snare_standard_rhythms = {
  snare_standard_rhythms_arr, 2, 0
};

// Rock rhythms
r_func snare_rock_rhythms_arr[] = {
  snare_drum_rhythm_4_4_offbeat,
  snare_drum_rhythm_4_4
};
RhythmCollection snare_rock_rhythms = {
  snare_rock_rhythms_arr, 2, 0
};

// Blues rhythms
r_func snare_blues_rhythms_arr[] = {snare_drum_rhythm_4_4_offbeat};
RhythmCollection snare_blues_rhythms = {
  snare_blues_rhythms_arr, 1, 0
};

// Jazz rhythms
r_func snare_jazz_rhythms_arr[] = {snare_drum_rhythm_4_4_jazz};
RhythmCollection snare_jazz_rhythms = {
  snare_jazz_rhythms_arr, 1, 0
};

// Waltz rhythms
r_func snare_waltz_rhythms_arr[] = {
  snare_drum_rhythm_3_4_waltz_offbeat,
  empty_rhythm
};
RhythmCollection snare_waltz_rhythms = {
  snare_waltz_rhythms_arr, 2, 0
};

RhythmCollection snare_rhythms[] = {
  snare_standard_rhythms,
  snare_rock_rhythms,
  snare_blues_rhythms,
  snare_jazz_rhythms,
  snare_waltz_rhythms
};

/* Snare drum breaks */

void snare_drum_break_standard(Rhythm* r) {
  const unsigned char notes[] = {
    0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x65, 0x00
  };
  strcpy(r->name, "1-7");
  r->numerator = 4;
  r->denominator = 4;
  r->subdivision = 8;
  r->note_count = 8;
  for (int n=0;n < r->note_count;n++) {
    r->notes[n] = notes[n];
  }
}

void snare_drum_break_lets(Rhythm* r) {
  const unsigned char notes[] = {
    0x00, 0x00, 0x60, 0x00, 0x00, 0x60, 0x00, 0x00, 0x60, 0x00, 0x00, 0x60
  };
  strcpy(r->name, "'let");
  r->numerator = 4;
  r->denominator = 4;
  r->subdivision = 12;
  r->note_count = 12;
  for (int n=0;n < r->note_count;n++) {
    r->notes[n] = notes[n];
  }
}

// 3/4
void snare_drum_break_3_4(Rhythm* r) {
  const unsigned char notes[] = {
    0x70, 0x00, 0x60, 0x60, 0x00, 0x60, 0x60, 0x00, 0x00
  };
  strcpy(r->name, "one'let");
  r->numerator = 3;
  r->denominator = 4;
  r->subdivision = 12;
  r->note_count = 9;
  for (int n=0;n < r->note_count;n++) {
    r->notes[n] = notes[n];
  }
}

// Standard breaks
r_func snare_standard_breaks_arr[] = {
  snare_drum_break_standard,
  bass_drum_rhythm_4_4
};
RhythmCollection snare_standard_breaks = {
  snare_standard_breaks_arr, 2, 0
};

// Rock breaks
r_func snare_rock_breaks_arr[] = {
  snare_drum_break_standard,
  bass_drum_rhythm_4_4
};
RhythmCollection snare_rock_breaks = {
  snare_rock_breaks_arr, 2, 0
};

// Blues breaks
r_func snare_blues_breaks_arr[] = {
  empty_rhythm,
  snare_drum_break_lets,
  bass_drum_rhythm_4_4
};
RhythmCollection snare_blues_breaks = {
  snare_blues_breaks_arr, 3, 0
};

// Jazz breaks
r_func snare_jazz_breaks_arr[] = {
  empty_rhythm,
  snare_drum_break_lets,
  bass_drum_rhythm_4_4
};
RhythmCollection snare_jazz_breaks = {
  snare_jazz_breaks_arr, 3, 0
};

// Waltz breaks
r_func snare_waltz_breaks_arr[] = {
  empty_rhythm,
  snare_drum_break_3_4,
};
RhythmCollection snare_waltz_breaks = {
  snare_waltz_breaks_arr, 2, 0
};

RhythmCollection snare_breaks[] = {
  snare_standard_breaks,
  snare_rock_breaks,
  snare_blues_breaks,
  snare_jazz_breaks,
  snare_waltz_breaks
};

unsigned char snare_drum_cur_rhythm_notes[RHYTHM_MAX_NOTES] = {0};
Rhythm snare_drum_cur_rhythm = {
  "", 4, 4, 1, snare_drum_cur_rhythm_notes, 0
};
unsigned char snare_drum_cur_break_notes[RHYTHM_MAX_NOTES] = {0};
Rhythm snare_drum_cur_break = {
  "", 4, 4, 1, snare_drum_cur_break_notes, 0
};
Instrument snare_drum = {
  1, "Snare Drum", 38, A1, snare_rhythms, snare_breaks,
  snare_drum_cur_rhythm, snare_drum_cur_break
};

/* Hi-Hat */
/* Hi-Hat rhythms */
void hi_hat_rhythm_4_4_eights(Rhythm* r) {
  const unsigned char notes[] = {
    0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48
  };
  strcpy(r->name, "1-8");
  r->numerator = 4;
  r->denominator = 4;
  r->subdivision = 8;
  r->note_count = 8;
  for (int n=0;n < r->note_count;n++) {
    r->notes[n] = notes[n];
  }
}
void hi_hat_rhythm_4_4_triplets(Rhythm* r) {
  const unsigned char notes[] = {
    0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48
  };
  strcpy(r->name, "1-12");
  r->numerator = 4;
  r->denominator = 4;
  r->subdivision = 12;
  r->note_count = 12;
  for (int n=0;n < r->note_count;n++) {
    r->notes[n] = notes[n];
  }
}

void hi_hat_rhythm_triplets_1_3(Rhythm* r) {
  const unsigned char notes[] = {
    0x48, 0x00, 0x40, 0x48, 0x00, 0x40, 0x48, 0x00, 0x40, 0x48, 0x00, 0x40
  };
  strcpy(r->name, "One 'let");
  r->numerator = 4;
  r->denominator = 4;
  r->subdivision = 12;
  r->note_count = 12;
  for (int n=0;n < r->note_count;n++) {
    r->notes[n] = notes[n];
  }
}
void hi_hat_rhythm_4_4_offbeat(Rhythm* r) {
  const unsigned char notes[] = {
    0x00, 0x48, 0x00, 0x48
  };
  strcpy(r->name, "Off Beat");
  r->numerator = 4;
  r->denominator = 4;
  r->subdivision = 4;
  r->note_count = 4;
  for (int n=0;n < r->note_count;n++) {
    r->notes[n] = notes[n];
  }
}

// 3/4
void hi_hat_rhythm_3_4_waltz(Rhythm* r) {
  const unsigned char notes[] = {
    0x70, 0x00, 0x00, 0x70, 0x00, 0x60, 0x70, 0x00, 0x00
  };
  strcpy(r->name, "3/4 1+2+23/3+3");
  r->numerator = 3;
  r->denominator = 4;
  r->subdivision = 12;
  r->note_count = 9;
  for (int n=0;n < r->note_count;n++) {
    r->notes[n] = notes[n];
  }
}

void hi_hat_rhythm_3_4_triplets(Rhythm* r) {
  const unsigned char notes[] = {
    0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48
  };
  strcpy(r->name, "1-9");
  r->numerator = 3;
  r->denominator = 4;
  r->subdivision = 12;
  r->note_count = 9;
  for (int n=0;n < r->note_count;n++) {
    r->notes[n] = notes[n];
  }
}

// Standard rhythms
r_func hi_hat_standard_rhythms_arr[] = {
  hi_hat_rhythm_4_4_eights,
  hi_hat_rhythm_4_4_offbeat,
  empty_rhythm
};
RhythmCollection hi_hat_standard_rhythms = {
  hi_hat_standard_rhythms_arr, 3, 0
};

// Rock rhythms
r_func hi_hat_rock_rhythms_arr[] = {
  hi_hat_rhythm_4_4_eights,
  hi_hat_rhythm_4_4_offbeat,
  empty_rhythm
};
RhythmCollection hi_hat_rock_rhythms = {
  hi_hat_rock_rhythms_arr, 3, 0
};

// Blues rhythms
r_func hi_hat_blues_rhythms_arr[] = {
  empty_rhythm,
  hi_hat_rhythm_4_4_triplets,
  hi_hat_rhythm_triplets_1_3
};
RhythmCollection hi_hat_blues_rhythms = {
  hi_hat_blues_rhythms_arr, 3, 0
};

// Jazz rhythms
r_func hi_hat_jazz_rhythms_arr[] = {
  empty_rhythm,
  hi_hat_rhythm_4_4_offbeat,
  hi_hat_rhythm_triplets_1_3,
  hi_hat_rhythm_4_4_triplets
};
RhythmCollection hi_hat_jazz_rhythms = {
  hi_hat_jazz_rhythms_arr, 4, 0
};

// Waltz rhythms
r_func hi_hat_waltz_rhythms_arr[] = {
  empty_rhythm,
  hi_hat_rhythm_3_4_waltz,
  hi_hat_rhythm_3_4_triplets
};
RhythmCollection hi_hat_waltz_rhythms = {
  hi_hat_waltz_rhythms_arr, 3, 0
};

RhythmCollection hi_hat_rhythms[] = {
  hi_hat_standard_rhythms,
  hi_hat_rock_rhythms,
  hi_hat_blues_rhythms,
  hi_hat_jazz_rhythms,
  hi_hat_waltz_rhythms
};

/* Hi-Hat breaks */
void hi_hat_break_standard(Rhythm* r) {
  const unsigned char notes[] = {
    0x60, 0x60, 0x60, 0x65
  };
  strcpy(r->name, "1-4");
  r->numerator = 4;
  r->denominator = 4;
  r->subdivision = 4;
  r->note_count = 4;
  for (int n=0;n < r->note_count;n++) {
    r->notes[n] = notes[n];
  }
}

// Standard breaks
r_func hi_hat_standard_breaks_arr[] = {
  hi_hat_break_standard,
  hi_hat_rhythm_4_4_eights,
  empty_rhythm
};
RhythmCollection hi_hat_standard_breaks = {
  hi_hat_standard_breaks_arr, 3, 0
};

// Rock breaks
r_func hi_hat_rock_breaks_arr[] = {
  hi_hat_break_standard,
  hi_hat_rhythm_4_4_eights,
  empty_rhythm
};
RhythmCollection hi_hat_rock_breaks = {
  hi_hat_rock_breaks_arr, 3, 0
};

// Blues breaks
r_func hi_hat_blues_breaks_arr[] = {
  empty_rhythm,
  hi_hat_break_standard,
  hi_hat_rhythm_4_4_offbeat,
  hi_hat_rhythm_triplets_1_3,
  hi_hat_rhythm_4_4_triplets
};
RhythmCollection hi_hat_blues_breaks = {
  hi_hat_blues_breaks_arr, 5, 0
};

// Jazz breaks
r_func hi_hat_jazz_breaks_arr[] = {
  empty_rhythm,
  hi_hat_break_standard,
  hi_hat_rhythm_4_4_offbeat,
  hi_hat_rhythm_triplets_1_3,
  hi_hat_rhythm_4_4_triplets
};
RhythmCollection hi_hat_jazz_breaks = {
  hi_hat_jazz_breaks_arr, 5, 0
};

// Waltz breaks
r_func hi_hat_waltz_breaks_arr[] = {
  empty_rhythm,
  hi_hat_rhythm_3_4_triplets
};
RhythmCollection hi_hat_waltz_breaks = {
  hi_hat_waltz_breaks_arr, 2, 0
};

RhythmCollection hi_hat_breaks[] = {
  hi_hat_standard_breaks,
  hi_hat_rock_breaks,
  hi_hat_blues_breaks,
  hi_hat_jazz_breaks,
  hi_hat_waltz_breaks
};

unsigned char hi_hat_cur_rhythm_notes[RHYTHM_MAX_NOTES] = {0};
Rhythm hi_hat_cur_rhythm = {
  "", 4, 4, 1, hi_hat_cur_rhythm_notes, 0
};
unsigned char hi_hat_cur_break_notes[RHYTHM_MAX_NOTES] = {0};
Rhythm hi_hat_cur_break = {
  "", 4, 4, 1, hi_hat_cur_break_notes, 0
};
Instrument hi_hat = {
  2, "Hi-Hat", 42, A2, hi_hat_rhythms, hi_hat_breaks,
  hi_hat_cur_rhythm, hi_hat_cur_break
};

/* Splash */
/* Splash rhythms */

// No splash rhythms yet
RhythmCollection* splash_rhythms = empty_rhythms;

/* Splash breaks */
void splash_break_eigth(Rhythm* r) {
  const unsigned char notes[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50
  };
  strcpy(r->name, "8");
  r->numerator = 4;
  r->denominator = 4;
  r->subdivision = 8;
  r->note_count = 8;
  for (int n=0;n < r->note_count;n++) {
    r->notes[n] = notes[n];
  }
}
void splash_break_4_4(Rhythm* r) {
  const unsigned char notes[] = {
    0x00, 0x00, 0x00, 0x50
  };
  strcpy(r->name, "4");
  r->numerator = 4;
  r->denominator = 4;
  r->subdivision = 4;
  r->note_count = 4;
  for (int n=0;n < r->note_count;n++) {
    r->notes[n] = notes[n];
  }
}

// 3/4
void splash_break_3_4(Rhythm* r) {
  const unsigned char notes[] = {
    0x00, 0x00, 0x50
  };
  strcpy(r->name, "4");
  r->numerator = 3;
  r->denominator = 4;
  r->subdivision = 4;
  r->note_count = 3;
  for (int n=0;n < r->note_count;n++) {
    r->notes[n] = notes[n];
  }
}

// Standard breaks
r_func splash_standard_breaks_arr[] = {splash_break_eigth};
RhythmCollection splash_standard_breaks = {
  splash_standard_breaks_arr, 1, 0
};

// Rock breaks
r_func splash_rock_breaks_arr[] = {splash_break_4_4};
RhythmCollection splash_rock_breaks = {
  splash_rock_breaks_arr, 1, 0
};

// Blues breaks
r_func splash_blues_breaks_arr[] = {splash_break_4_4};
RhythmCollection splash_blues_breaks = {
  splash_blues_breaks_arr, 1, 0
};

// Jazz breaks
r_func splash_jazz_breaks_arr[] = {splash_break_4_4};
RhythmCollection splash_jazz_breaks = {
  splash_jazz_breaks_arr, 1, 0
};

// Waltz breaks
r_func splash_waltz_breaks_arr[] = {splash_break_3_4};
RhythmCollection splash_waltz_breaks = {
  splash_waltz_breaks_arr, 1, 0
};

RhythmCollection splash_breaks[] = {
  splash_standard_breaks,
  splash_rock_breaks,
  splash_blues_breaks,
  splash_jazz_breaks,
  splash_waltz_breaks
};

unsigned char splash_cur_rhythm_notes[RHYTHM_MAX_NOTES] = {0};
Rhythm splash_cur_rhythm = {
  "", 4, 4, 1, splash_cur_rhythm_notes, 0
};
unsigned char splash_cur_break_notes[RHYTHM_MAX_NOTES] = {0};
Rhythm splash_cur_break = {
  "", 4, 4, 1, splash_cur_break_notes, 0
};
Instrument splash = {
  3, "Splash", 49, A2, splash_rhythms, splash_breaks,
  splash_cur_rhythm, splash_cur_break
};


/* Ride */
/* Ride rhythms */
// take the ones of hi-hat

// Standard rhythms
r_func ride_standard_rhythms_arr[] = {
  empty_rhythm,
  hi_hat_rhythm_4_4_eights,
  hi_hat_rhythm_4_4_offbeat
};
RhythmCollection ride_standard_rhythms = {
  ride_standard_rhythms_arr, 3, 0
};

// Rock rhythms
r_func ride_rock_rhythms_arr[] = {
  empty_rhythm,
  hi_hat_rhythm_4_4_eights,
  hi_hat_rhythm_4_4_offbeat
};
RhythmCollection ride_rock_rhythms = {
  ride_rock_rhythms_arr, 3, 0
};

// Blues rhythms
r_func ride_blues_rhythms_arr[] = {
  hi_hat_rhythm_4_4_triplets,
  hi_hat_rhythm_triplets_1_3,
  empty_rhythm
};
RhythmCollection ride_blues_rhythms = {
  ride_blues_rhythms_arr, 3, 0
};

// Jazz rhythms
r_func ride_jazz_rhythms_arr[] = {
  hi_hat_rhythm_4_4_offbeat,
  hi_hat_rhythm_triplets_1_3,
  hi_hat_rhythm_4_4_triplets,
  empty_rhythm
};
RhythmCollection ride_jazz_rhythms = {
  ride_jazz_rhythms_arr, 4, 0
};

// Waltz rhythms
r_func ride_waltz_rhythms_arr[] = {
  hi_hat_rhythm_3_4_waltz,
  hi_hat_rhythm_3_4_triplets,
  empty_rhythm
};
RhythmCollection ride_waltz_rhythms = {
  ride_waltz_rhythms_arr, 3, 0
};

RhythmCollection ride_rhythms[] = {
  ride_standard_rhythms,
  ride_rock_rhythms,
  ride_blues_rhythms,
  ride_jazz_rhythms,
  ride_waltz_rhythms
};

/* Ride breaks */
r_func ride_break_standard = hi_hat_break_standard;

// Standard breaks
r_func ride_standard_breaks_arr[] = {
  empty_rhythm,
  ride_break_standard,
  hi_hat_rhythm_4_4_eights
};
RhythmCollection ride_standard_breaks = {
  ride_standard_breaks_arr, 3, 0
};

// Rock breaks
r_func ride_rock_breaks_arr[] = {
  empty_rhythm,
  ride_break_standard,
  hi_hat_rhythm_4_4_eights
};
RhythmCollection ride_rock_breaks = {
  ride_rock_breaks_arr, 3, 0
};

// Blues breaks
r_func ride_blues_breaks_arr[] = {
  empty_rhythm,
  ride_break_standard,
  hi_hat_rhythm_4_4_offbeat,
  hi_hat_rhythm_triplets_1_3,
  hi_hat_rhythm_4_4_triplets
};
RhythmCollection ride_blues_breaks = {
  ride_blues_breaks_arr, 5, 0
};

// Jazz breaks
r_func ride_jazz_breaks_arr[] = {
  empty_rhythm,
  ride_break_standard,
  hi_hat_rhythm_4_4_offbeat,
  hi_hat_rhythm_triplets_1_3,
  hi_hat_rhythm_4_4_triplets
};
RhythmCollection ride_jazz_breaks = {
  ride_jazz_breaks_arr, 5, 0
};

// Waltz breaks
r_func ride_waltz_breaks_arr[] = {
  empty_rhythm,
  hi_hat_rhythm_3_4_triplets
};
RhythmCollection ride_waltz_breaks = {
  ride_waltz_breaks_arr, 2, 0
};

RhythmCollection ride_breaks[] = {
  ride_standard_breaks,
  ride_rock_breaks,
  ride_blues_breaks,
  ride_jazz_breaks,
  ride_waltz_breaks
};

unsigned char ride_cur_rhythm_notes[RHYTHM_MAX_NOTES] = {0};
Rhythm ride_cur_rhythm = {
  "", 4, 4, 1, ride_cur_rhythm_notes, 0
};
unsigned char ride_cur_break_notes[RHYTHM_MAX_NOTES] = {0};
Rhythm ride_cur_break = {
  "", 4, 4, 1, ride_cur_break_notes, 0
};
Instrument ride = {
  4, "Ride", 51, A2, ride_rhythms, ride_breaks,
  ride_cur_rhythm, ride_cur_break
};


/* Instrument list */
Instrument instrs[] = {bass_drum, snare_drum, hi_hat, splash, ride};
const int instrument_count = 5;
#endif
