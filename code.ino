/*
 * ============================================================
 *  GUARD SYS — ESP8266 RFID Security System
 *  Web-configurable via built-in HTTP server
 *  Supports: card management, Web Alerts, sensor tuning,
 *            27 alarm tunes, custom message templates
 *  Dependencies: ESP8266WiFi, ESP8266WebServer,
 *                LittleFS, MFRC522, ArduinoJson
 * ============================================================
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ArduinoJson.h>

// ============================================================
//  NOTE DEFINITIONS (full range)
// ============================================================
#define REST     0
#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978

// ============================================================
//  TUNE DEFINITIONS
// ============================================================
const int mel_nokia[] PROGMEM = { NOTE_E5,8, NOTE_D5,8, NOTE_FS4,4, NOTE_GS4,4, NOTE_CS5,8, NOTE_B4,8, NOTE_D4,4, NOTE_E4,4, NOTE_B4,8, NOTE_A4,8, NOTE_CS4,4, NOTE_E4,4, NOTE_A4,2 }; const int tmp_nokia = 180;
const int mel_mario[] PROGMEM = { NOTE_E5,8, NOTE_E5,8, REST,8, NOTE_E5,8, REST,8, NOTE_C5,8, NOTE_E5,4, NOTE_G5,4, REST,4, NOTE_G4,4, REST,4, NOTE_C5,-4, NOTE_G4,8, REST,4, NOTE_E4,-4, NOTE_A4,4, NOTE_B4,4, NOTE_AS4,8, NOTE_A4,4, NOTE_G4,-8, NOTE_E5,-8, NOTE_G5,-8, NOTE_A5,4, NOTE_F5,8, NOTE_G5,8, REST,8, NOTE_E5,4, NOTE_C5,8, NOTE_D5,8, NOTE_B4,-4, NOTE_C5,-4, NOTE_G4,8, REST,4, NOTE_E4,-4, NOTE_A4,4, NOTE_B4,4, NOTE_AS4,8, NOTE_A4,4, NOTE_G4,-8, NOTE_E5,-8, NOTE_G5,-8, NOTE_A5,4, NOTE_F5,8, NOTE_G5,8, REST,8, NOTE_E5,4, NOTE_C5,8, NOTE_D5,8, NOTE_B4,-4 }; const int tmp_mario = 200;
const int mel_tetris[] PROGMEM = { NOTE_E5,4, NOTE_B4,8, NOTE_C5,8, NOTE_D5,4, NOTE_C5,8, NOTE_B4,8, NOTE_A4,4, NOTE_A4,8, NOTE_C5,8, NOTE_E5,4, NOTE_D5,8, NOTE_C5,8, NOTE_B4,-4, NOTE_C5,8, NOTE_D5,4, NOTE_E5,4, NOTE_C5,4, NOTE_A4,4, NOTE_A4,4, REST,4, NOTE_D5,-4, NOTE_F5,8, NOTE_A5,4, NOTE_G5,8, NOTE_F5,8, NOTE_E5,-4, NOTE_C5,8, NOTE_E5,4, NOTE_D5,8, NOTE_C5,8, NOTE_B4,4, NOTE_B4,8, NOTE_C5,8, NOTE_D5,4, NOTE_E5,4, NOTE_C5,4, NOTE_A4,4, NOTE_A4,4, REST,4, NOTE_E5,2, NOTE_C5,2, NOTE_D5,2, NOTE_B4,2, NOTE_C5,2, NOTE_A4,2, NOTE_GS4,2, NOTE_B4,4, REST,8, NOTE_E5,2, NOTE_C5,2, NOTE_D5,2, NOTE_B4,2, NOTE_C5,4, NOTE_E5,4, NOTE_A5,2, NOTE_GS5,2 }; const int tmp_tetris = 144;
const int mel_imperial[] PROGMEM = { NOTE_A4,4, NOTE_A4,4, NOTE_A4,4, NOTE_F4,-8, NOTE_C5,16, NOTE_A4,4, NOTE_F4,-8, NOTE_C5,16, NOTE_A4,2, NOTE_E5,4, NOTE_E5,4, NOTE_E5,4, NOTE_F5,-8, NOTE_C5,16, NOTE_A4,4, NOTE_F4,-8, NOTE_C5,16, NOTE_A4,2, NOTE_A5,4, NOTE_A4,-8, NOTE_A4,16, NOTE_A5,4, NOTE_GS5,-8, NOTE_G5,16, NOTE_DS5,16, NOTE_D5,16, NOTE_DS5,8, REST,8, NOTE_A4,8, NOTE_DS5,4, NOTE_D5,-8, NOTE_CS5,16, NOTE_C5,16, NOTE_B4,16, NOTE_C5,16, REST,8, NOTE_F4,8, NOTE_GS4,4, NOTE_F4,-8, NOTE_A4,16, NOTE_C5,4, NOTE_A4,-8, NOTE_C5,16, NOTE_E5,2, NOTE_A5,4, NOTE_A4,-8, NOTE_A4,16, NOTE_A5,4, NOTE_GS5,-8, NOTE_G5,16, NOTE_DS5,16, NOTE_D5,16, NOTE_DS5,8, REST,8, NOTE_A4,8, NOTE_DS5,4, NOTE_D5,-8, NOTE_CS5,16, NOTE_C5,16, NOTE_B4,16, NOTE_C5,16, REST,8, NOTE_F4,8, NOTE_GS4,4, NOTE_F4,-8, NOTE_A4,16, NOTE_A4,4, NOTE_F4,-8, NOTE_C5,16, NOTE_A4,2 }; const int tmp_imperial = 120;
const int mel_cantina[] PROGMEM = { NOTE_A4,4, NOTE_D5,8, NOTE_A4,8, NOTE_A4,-8, NOTE_GS4,16, NOTE_A4,8, NOTE_D5,8, NOTE_A4,8, NOTE_A4,4, NOTE_E5,8, NOTE_E5,8, NOTE_C5,4, NOTE_B4,8, NOTE_C5,8, NOTE_A4,4, NOTE_D5,8, NOTE_A4,8, NOTE_A4,-8, NOTE_GS4,16, NOTE_A4,8, NOTE_D5,8, NOTE_A4,8, NOTE_A4,2 }; const int tmp_cantina = 114;
const int mel_harry[] PROGMEM = { REST,2, NOTE_D4,4, NOTE_G4,-4, NOTE_AS4,8, NOTE_A4,4, NOTE_G4,2, NOTE_D5,4, NOTE_C5,-2, NOTE_A4,-2, NOTE_G4,-4, NOTE_AS4,8, NOTE_A4,4, NOTE_F4,2, NOTE_GS4,4, NOTE_D4,-2, NOTE_D4,4, NOTE_G4,-4, NOTE_AS4,8, NOTE_A4,4, NOTE_G4,2, NOTE_D5,4, NOTE_F5,-4, NOTE_E5,8, NOTE_DS5,4, NOTE_B4,-4, NOTE_DS5,8, NOTE_D5,4, NOTE_C5,-2, NOTE_A4,-4, NOTE_G4,8, NOTE_AS4,4, NOTE_A4,4, NOTE_G4,2, NOTE_D4,-2 }; const int tmp_harry = 144;
const int mel_starwars[] PROGMEM = { NOTE_AS4,8, NOTE_AS4,8, NOTE_AS4,8, NOTE_F5,2, NOTE_C6,2, NOTE_AS5,8, NOTE_A5,8, NOTE_G5,8, NOTE_F6,2, NOTE_C6,4, NOTE_AS5,8, NOTE_A5,8, NOTE_G5,8, NOTE_F6,2, NOTE_C6,4, NOTE_AS5,8, NOTE_A5,8, NOTE_AS5,8, NOTE_G5,2, NOTE_C5,8, NOTE_C5,8, NOTE_C5,8, NOTE_F5,2, NOTE_C6,2, NOTE_AS5,8, NOTE_A5,8, NOTE_G5,8, NOTE_F6,2, NOTE_C6,4, NOTE_AS5,8, NOTE_A5,8, NOTE_G5,8, NOTE_F6,2, NOTE_C6,4, NOTE_AS5,8, NOTE_A5,8, NOTE_AS5,8, NOTE_G5,2 }; const int tmp_starwars = 108;
const int mel_got[] PROGMEM = { NOTE_G4,8, NOTE_C4,8, NOTE_DS4,16, NOTE_F4,16, NOTE_G4,8, NOTE_C4,8, NOTE_DS4,16, NOTE_F4,16, NOTE_G4,8, NOTE_C4,8, NOTE_DS4,16, NOTE_F4,16, NOTE_G4,4, NOTE_G4,8, NOTE_C4,8, NOTE_DS4,16, NOTE_F4,16, NOTE_G4,8, NOTE_C4,8, NOTE_DS4,16, NOTE_F4,16, NOTE_G4,4 }; const int tmp_got = 85;
const int mel_doom[] PROGMEM = { NOTE_E2,8, NOTE_E2,8, NOTE_E3,8, NOTE_E2,8, NOTE_E2,8, NOTE_D3,8, NOTE_E2,8, NOTE_E2,8, NOTE_C3,8, NOTE_E2,8, NOTE_E2,8, NOTE_AS2,8, NOTE_E2,8, NOTE_E2,8, NOTE_B2,8, NOTE_C3,8, NOTE_E2,8, NOTE_E2,8, NOTE_E3,8, NOTE_E2,8, NOTE_E2,8, NOTE_D3,8, NOTE_E2,8, NOTE_E2,8, NOTE_C3,8, NOTE_E2,8, NOTE_C3,8, NOTE_AS2,2 }; const int tmp_doom = 220;
const int mel_pacman[] PROGMEM = { NOTE_B4,16, NOTE_B5,16, NOTE_FS5,16, NOTE_DS5,16, NOTE_B5,32, NOTE_FS5,-16, NOTE_DS5,8, NOTE_C5,16, NOTE_C6,16, NOTE_G6,16, NOTE_E6,16, NOTE_C6,32, NOTE_G6,-16, NOTE_E6,8, NOTE_B4,16, NOTE_B5,16, NOTE_FS5,16, NOTE_DS5,16, NOTE_B5,32, NOTE_FS5,-16, NOTE_DS5,8, NOTE_DS5,32, NOTE_E5,32, NOTE_F5,32, NOTE_F5,32, NOTE_FS5,32, NOTE_G5,32, NOTE_G5,32, NOTE_GS5,32, NOTE_A5,16, NOTE_B5,8 }; const int tmp_pacman = 105;
const int mel_greenhill[] PROGMEM = { NOTE_A4,8, NOTE_B4,8, NOTE_D5,8, NOTE_B4,8, NOTE_FS5,-4, NOTE_FS5,-4, NOTE_E5,-2, NOTE_A4,8, NOTE_B4,8, NOTE_D5,8, NOTE_B4,8, NOTE_E5,-4, NOTE_E5,-4, NOTE_D5,-4, NOTE_CS5,8, NOTE_B4,4, NOTE_A4,8, NOTE_B4,8, NOTE_D5,8, NOTE_B4,8, NOTE_D5,4, NOTE_E5,4, NOTE_CS5,4, NOTE_B4,4, NOTE_A4,-2 }; const int tmp_greenhill = 130;
const int mel_mii[] PROGMEM = { REST,4, NOTE_FS4,8, NOTE_A4,8, NOTE_CS5,8, NOTE_A4,8, NOTE_FS4,8, REST,4, REST,8, NOTE_B4,8, NOTE_D5,8, NOTE_FS5,8, NOTE_D5,8, NOTE_B4,8, REST,4, REST,8, NOTE_E5,8, NOTE_A5,8, NOTE_E5,8, NOTE_CS5,8, NOTE_A4,-4, REST,4, REST,8, NOTE_FS5,8, NOTE_E5,8, NOTE_CS5,8, NOTE_A4,8, NOTE_FS4,-2 }; const int tmp_mii = 114;
const int mel_songofstorms[] PROGMEM = { NOTE_D4,8, NOTE_F4,8, NOTE_D5,4, NOTE_D4,8, NOTE_F4,8, NOTE_D5,4, NOTE_E5,4, NOTE_F5,8, NOTE_E5,8, NOTE_F5,8, NOTE_E5,8, NOTE_C5,8, NOTE_A4,2, NOTE_D4,8, NOTE_F4,8, NOTE_D5,4, NOTE_D4,8, NOTE_F4,8, NOTE_D5,4, NOTE_E5,4, NOTE_F5,4, NOTE_E5,4, NOTE_C5,4, NOTE_D5,2 }; const int tmp_songofstorms = 160;
const int mel_zelda[] PROGMEM = { NOTE_E4,-4, NOTE_G4,8, NOTE_A4,2, NOTE_E4,-4, NOTE_G4,8, NOTE_A4,2, NOTE_E4,4, NOTE_G4,4, NOTE_A4,4, NOTE_G4,4, NOTE_E4,2, NOTE_D4,2, NOTE_E4,-4, NOTE_G4,8, NOTE_A4,2, NOTE_G4,-4, NOTE_E4,8, NOTE_D4,2 }; const int tmp_zelda = 100;
const int mel_zeldatheme[] PROGMEM = { NOTE_G5,4, NOTE_FS5,8, NOTE_DS5,8, NOTE_A4,4, NOTE_GS4,8, NOTE_DS4,8, NOTE_GS4,-4, NOTE_DS5,8, NOTE_GS5,4, NOTE_AS5,4, NOTE_CS6,-4, NOTE_A5,8, NOTE_AS5,8, NOTE_GS5,8, NOTE_G5,8, NOTE_F5,8, NOTE_G5,2 }; const int tmp_zeldatheme = 150;
const int mel_bloodytears[] PROGMEM = { NOTE_E4,8, NOTE_E4,8, NOTE_E5,4, NOTE_DS5,8, NOTE_D5,8, NOTE_B4,8, NOTE_G4,8, NOTE_A4,8, NOTE_B4,8, NOTE_C5,4, NOTE_E4,4, NOTE_E4,8, NOTE_E4,8, NOTE_E5,4, NOTE_DS5,8, NOTE_D5,8, NOTE_B4,8, NOTE_G4,8, NOTE_A4,8, NOTE_B4,8, NOTE_C5,2 }; const int tmp_bloodytears = 180;
const int mel_pinkpanther[] PROGMEM = { REST,4, NOTE_DS4,8, NOTE_E4,-4, REST,8, NOTE_FS4,8, NOTE_G4,-4, REST,8, NOTE_DS4,8, NOTE_E4,-8, NOTE_FS4,8, NOTE_G4,-8, NOTE_C5,8, NOTE_B4,-8, NOTE_E4,8, NOTE_G4,-8, NOTE_B4,4, NOTE_AS4,4, NOTE_A4,-4, NOTE_G4,-2 }; const int tmp_pinkpanther = 120;
const int mel_takeonme[] PROGMEM = { NOTE_FS5,8, NOTE_FS5,8, NOTE_D5,8, NOTE_B4,8, REST,8, NOTE_B4,8, REST,8, NOTE_E5,8, REST,8, NOTE_E5,8, REST,8, NOTE_E5,8, NOTE_GS5,8, NOTE_GS5,8, NOTE_A5,8, NOTE_B5,8, NOTE_A5,4, NOTE_A5,8, NOTE_A5,8, NOTE_E5,4, NOTE_E5,8, NOTE_E5,8, NOTE_FS5,4, NOTE_FS5,8, NOTE_FS5,8, NOTE_B4,4, NOTE_B4,8, NOTE_B4,8 }; const int tmp_takeonme = 140;
const int mel_rickroll[] PROGMEM = { NOTE_D5,8, NOTE_E5,8, NOTE_A4,4, NOTE_E5,4, NOTE_FS5,4, NOTE_A5,4, NOTE_D5,8, NOTE_E5,8, NOTE_A4,4, NOTE_FS5,2, NOTE_D5,8, NOTE_E5,8, NOTE_A4,4, NOTE_A4,4, NOTE_FS5,4, NOTE_E5,2, NOTE_CS5,4, NOTE_D5,4, NOTE_A4,2 }; const int tmp_rickroll = 113;
const int mel_keyboardcat[] PROGMEM = { NOTE_FS4,8, NOTE_GS4,8, NOTE_FS4,8, NOTE_GS4,8, NOTE_FS4,8, NOTE_GS4,8, NOTE_A4,4, NOTE_FS4,8, NOTE_GS4,8, NOTE_FS4,8, NOTE_GS4,8, NOTE_FS4,8, NOTE_E4,4, REST,8, NOTE_FS4,8, NOTE_GS4,8, NOTE_A4,8, NOTE_B4,8, NOTE_CS5,4, NOTE_A4,4, NOTE_FS4,8, NOTE_GS4,8, NOTE_FS4,8, NOTE_E4,8, NOTE_FS4,2 }; const int tmp_keyboardcat = 100;
const int mel_canon[] PROGMEM = { NOTE_D5,4, NOTE_CS5,8, NOTE_B4,8, NOTE_A4,4, NOTE_B4,4, NOTE_CS5,4, NOTE_D5,4, NOTE_D5,4, NOTE_CS5,8, NOTE_B4,8, NOTE_A4,4, NOTE_B4,4, NOTE_CS5,4, NOTE_D5,2, NOTE_FS5,4, NOTE_E5,4, NOTE_D5,4, NOTE_E5,4, NOTE_FS5,4, NOTE_G5,4, NOTE_FS5,4, NOTE_E5,4, NOTE_D5,4, NOTE_CS5,4, NOTE_B4,4, NOTE_CS5,4, NOTE_D5,2, NOTE_FS5,2 }; const int tmp_canon = 100;
const int mel_odetojoy[] PROGMEM = { NOTE_E4,4, NOTE_E4,4, NOTE_F4,4, NOTE_G4,4, NOTE_G4,4, NOTE_F4,4, NOTE_E4,4, NOTE_D4,4, NOTE_C4,4, NOTE_C4,4, NOTE_D4,4, NOTE_E4,4, NOTE_E4,-4, NOTE_D4,8, NOTE_D4,2, NOTE_E4,4, NOTE_E4,4, NOTE_F4,4, NOTE_G4,4, NOTE_G4,4, NOTE_F4,4, NOTE_E4,4, NOTE_D4,4, NOTE_C4,4, NOTE_C4,4, NOTE_D4,4, NOTE_E4,4, NOTE_D4,-4, NOTE_C4,8, NOTE_C4,2 }; const int tmp_odetojoy = 114;
const int mel_furelise[] PROGMEM = { NOTE_E5,16, NOTE_DS5,16, NOTE_E5,16, NOTE_DS5,16, NOTE_E5,16, NOTE_B4,16, NOTE_D5,16, NOTE_C5,16, NOTE_A4,4, REST,8, NOTE_C4,16, NOTE_E4,16, NOTE_A4,16, NOTE_B4,4, REST,8, NOTE_E4,16, NOTE_GS4,16, NOTE_B4,16, NOTE_C5,4, REST,8, NOTE_E4,16, NOTE_E5,16, NOTE_DS5,16, NOTE_E5,16, NOTE_DS5,16, NOTE_E5,16, NOTE_B4,16, NOTE_D5,16, NOTE_C5,16, NOTE_A4,4, REST,8, NOTE_C4,16, NOTE_E4,16, NOTE_A4,16, NOTE_B4,4, REST,8, NOTE_E4,16, NOTE_C5,16, NOTE_B4,16, NOTE_A4,2 }; const int tmp_furelise = 130;
const int mel_greensleeves[] PROGMEM = { NOTE_A3,4, NOTE_C4,-4, NOTE_D4,8, NOTE_E4,-4, NOTE_F4,8, NOTE_E4,-4, NOTE_D4,8, NOTE_B3,-4, NOTE_G3,8, NOTE_A3,-4, NOTE_B3,8, NOTE_C4,2, NOTE_A3,4, NOTE_E4,-2, NOTE_D4,-4, NOTE_C4,8, NOTE_A3,-4, NOTE_F3,8, NOTE_G3,-4, NOTE_F3,8, NOTE_E3,-4, NOTE_D3,8, NOTE_E3,-2 }; const int tmp_greensleeves = 100;
const int mel_minuet[] PROGMEM = { NOTE_D4,4, NOTE_G4,8, NOTE_A4,8, NOTE_B4,8, NOTE_C5,8, NOTE_D5,4, NOTE_G4,4, NOTE_G4,4, NOTE_E5,4, NOTE_C5,8, NOTE_D5,8, NOTE_E5,8, NOTE_FS5,8, NOTE_G5,4, NOTE_G4,4, NOTE_G4,4, NOTE_C5,4, NOTE_D5,8, NOTE_C5,8, NOTE_B4,8, NOTE_A4,8, NOTE_B4,4, NOTE_C5,8, NOTE_B4,8, NOTE_A4,8, NOTE_G4,8, NOTE_FS4,4, NOTE_G4,8, NOTE_A4,8, NOTE_B4,8, NOTE_G4,8, NOTE_A4,2, NOTE_G4,4 }; const int tmp_minuet = 140;
const int mel_brahms[] PROGMEM = { NOTE_D4,-4, NOTE_E4,8, NOTE_E4,4, NOTE_FS4,4, NOTE_FS4,-4, NOTE_D4,8, NOTE_D4,2, NOTE_A4,-4, NOTE_B4,8, NOTE_B4,4, NOTE_A4,4, NOTE_A4,4, NOTE_FS4,4, NOTE_D4,2, NOTE_A4,-4, NOTE_B4,8, NOTE_B4,4, NOTE_A4,4, NOTE_A4,4, NOTE_FS4,4, NOTE_A4,2 }; const int tmp_brahms = 100;
const int mel_jigglypuff[] PROGMEM = { NOTE_C5,4, NOTE_E5,4, NOTE_G5,4, NOTE_E5,4, NOTE_FS5,4, NOTE_D5,4, NOTE_G5,2, NOTE_G5,8, NOTE_A5,8, NOTE_G5,8, NOTE_F5,8, NOTE_E5,4, NOTE_C5,4, NOTE_D5,4, NOTE_E5,4, NOTE_C5,2 }; const int tmp_jigglypuff = 95;

struct TuneEntry { const int* melody; int tempo; int noteCount; const char name[28]; };
#define TUNE_COUNT 27

const TuneEntry TUNES[TUNE_COUNT] = {
  { mel_nokia,       tmp_nokia,       sizeof(mel_nokia)/sizeof(mel_nokia[0])/2,             "Nokia Ringtone"          },
  { mel_mario,       tmp_mario,       sizeof(mel_mario)/sizeof(mel_mario[0])/2,             "Super Mario Bros"        },
  { mel_tetris,      tmp_tetris,      sizeof(mel_tetris)/sizeof(mel_tetris[0])/2,           "Tetris"                  },
  { mel_imperial,    tmp_imperial,    sizeof(mel_imperial)/sizeof(mel_imperial[0])/2,       "Imperial March"          },
  { mel_cantina,     tmp_cantina,     sizeof(mel_cantina)/sizeof(mel_cantina[0])/2,         "Cantina Band"            },
  { mel_harry,       tmp_harry,       sizeof(mel_harry)/sizeof(mel_harry[0])/2,             "Harry Potter"            },
  { mel_starwars,    tmp_starwars,    sizeof(mel_starwars)/sizeof(mel_starwars[0])/2,       "Star Wars Theme"         },
  { mel_got,         tmp_got,         sizeof(mel_got)/sizeof(mel_got[0])/2,                 "Game of Thrones"         },
  { mel_doom,        tmp_doom,        sizeof(mel_doom)/sizeof(mel_doom[0])/2,               "DOOM"                    },
  { mel_pacman,      tmp_pacman,      sizeof(mel_pacman)/sizeof(mel_pacman[0])/2,           "Pac-Man"                 },
  { mel_greenhill,   tmp_greenhill,   sizeof(mel_greenhill)/sizeof(mel_greenhill[0])/2,     "Green Hill Zone"         },
  { mel_mii,         tmp_mii,         sizeof(mel_mii)/sizeof(mel_mii[0])/2,                 "Mii Channel"             },
  { mel_songofstorms,tmp_songofstorms,sizeof(mel_songofstorms)/sizeof(mel_songofstorms[0])/2,"Song of Storms"         },
  { mel_zelda,       tmp_zelda,       sizeof(mel_zelda)/sizeof(mel_zelda[0])/2,             "Zelda's Lullaby"         },
  { mel_zeldatheme,  tmp_zeldatheme,  sizeof(mel_zeldatheme)/sizeof(mel_zeldatheme[0])/2,   "Zelda Theme NES"         },
  { mel_bloodytears, tmp_bloodytears, sizeof(mel_bloodytears)/sizeof(mel_bloodytears[0])/2, "Bloody Tears"            },
  { mel_pinkpanther, tmp_pinkpanther, sizeof(mel_pinkpanther)/sizeof(mel_pinkpanther[0])/2, "Pink Panther"            },
  { mel_takeonme,    tmp_takeonme,    sizeof(mel_takeonme)/sizeof(mel_takeonme[0])/2,       "Take On Me"              },
  { mel_rickroll,    tmp_rickroll,    sizeof(mel_rickroll)/sizeof(mel_rickroll[0])/2,       "Never Gonna Give You Up" },
  { mel_keyboardcat, tmp_keyboardcat, sizeof(mel_keyboardcat)/sizeof(mel_keyboardcat[0])/2, "Keyboard Cat"            },
  { mel_canon,       tmp_canon,       sizeof(mel_canon)/sizeof(mel_canon[0])/2,             "Canon in D"              },
  { mel_odetojoy,    tmp_odetojoy,    sizeof(mel_odetojoy)/sizeof(mel_odetojoy[0])/2,       "Ode to Joy"              },
  { mel_furelise,    tmp_furelise,    sizeof(mel_furelise)/sizeof(mel_furelise[0])/2,       "Fur Elise"               },
  { mel_greensleeves,tmp_greensleeves,sizeof(mel_greensleeves)/sizeof(mel_greensleeves[0])/2,"Greensleeves"          },
  { mel_minuet,      tmp_minuet,      sizeof(mel_minuet)/sizeof(mel_minuet[0])/2,           "Minuet in G"             },
  { mel_brahms,      tmp_brahms,      sizeof(mel_brahms)/sizeof(mel_brahms[0])/2,           "Brahms Lullaby"          },
  { mel_jigglypuff,  tmp_jigglypuff,  sizeof(mel_jigglypuff)/sizeof(mel_jigglypuff[0])/2,   "Jigglypuff Song"         },
};

// ============================================================
//  CONFIGURATION STRUCT
// ============================================================
#define MAX_CARDS 10

struct CardEntry {
  String uid;
  String name;
  String role;
};

struct Config {
  String  ssid         = "SSID";
  String  wifiPass     = "PASSWD";
  String  panelPass    = "admin";
  int     threshold    = 20;      
  int     scanInterval = 100;     
  bool    buzzOnDetect = true;
  bool    notifyDetect = true;
  bool    cooldown     = true;
  int     tuneIndex    = 0;       
  int     tuneRepeat   = 1;       
  int     tempoOverride= 0;       
  String  msgOnline    = "System Online: Guard Ready";
  String  msgArmed     = "ARMED by {NAME}";
  String  msgDisarmed  = "DISARMED by {NAME}";
  String  msgAlert     = "ALERT! BREACH Distance: {DISTANCE} cm";
  String  msgUnauth    = "Unauthorized Card: {UID}";
  int     cardCount    = 0;
  CardEntry cards[MAX_CARDS];
};

Config cfg;
bool   systemArmed = false;
bool   messageSent = false;

// ACTION QUEUE
bool reqTestBuzz   = false;
bool reqTestTune   = false;
bool reqArmBeep    = false;
bool reqDisarmBeep = false;

// ── Web Alert (modal + siren) ──────────────────────────────────────────────
// Used for INTRUSION and UNAUTHORIZED CARD — triggers red fullscreen modal
String        currentAlertMsg = "";
unsigned long alertCounter    = 1;
unsigned long currentAlertId  = 0;

// ── Web Log (terminal only, no modal) ─────────────────────────────────────
// Used for ARMED / DISARMED — shows in event log terminal, no siren
// [FIX] Added these three variables to carry arm/disarm messages to browser
String        currentLogMsg   = "";
unsigned long logCounter      = 1;
unsigned long currentLogId    = 0;

// ============================================================
//  PIN DEFINITIONS
// ============================================================
const int trigPin   = 5;   // D1
const int echoPin   = 4;   // D2
const int buzzerPin = 16;  // D0
#define SS_PIN  15         // D8
#define RST_PIN 0          // D3

// ============================================================
//  GLOBALS
// ============================================================
MFRC522 mfrc522(SS_PIN, RST_PIN);
ESP8266WebServer server(80);

unsigned long lastScan       = 0;
String        pendingScanUID = "";

void checkRFID();

// ============================================================
//  CONFIG PERSISTENCE
// ============================================================
void loadConfig() {
  if (!LittleFS.exists("/config.json")) return;
  File f = LittleFS.open("/config.json", "r");
  if (!f) return;

  DynamicJsonDocument doc(1024);
  if (deserializeJson(doc, f)) { f.close(); return; }
  f.close();

  cfg.ssid          = doc["ssid"]         | cfg.ssid;
  cfg.wifiPass      = doc["wifiPass"]     | cfg.wifiPass;
  cfg.panelPass     = doc["panelPass"]    | cfg.panelPass;
  cfg.threshold     = doc["threshold"]    | cfg.threshold;
  cfg.scanInterval  = doc["scanInterval"] | cfg.scanInterval;
  cfg.buzzOnDetect  = doc["buzzOnDetect"] | cfg.buzzOnDetect;
  cfg.notifyDetect  = doc["notifyDetect"] | cfg.notifyDetect;
  cfg.cooldown      = doc["cooldown"]     | cfg.cooldown;
  cfg.tuneIndex     = doc["tuneIndex"]    | cfg.tuneIndex;
  cfg.tuneRepeat    = doc["tuneRepeat"]   | cfg.tuneRepeat;
  cfg.tempoOverride = doc["tempoOverride"]| cfg.tempoOverride;
  cfg.msgOnline     = doc["msgOnline"]    | cfg.msgOnline;
  cfg.msgArmed      = doc["msgArmed"]     | cfg.msgArmed;
  cfg.msgDisarmed   = doc["msgDisarmed"]  | cfg.msgDisarmed;
  cfg.msgAlert      = doc["msgAlert"]     | cfg.msgAlert;
  cfg.msgUnauth     = doc["msgUnauth"]    | cfg.msgUnauth;

  JsonArray cards = doc["cards"];
  cfg.cardCount = 0;
  for (JsonObject c : cards) {
    if (cfg.cardCount >= MAX_CARDS) break;
    cfg.cards[cfg.cardCount].uid  = c["uid"]  | String("");
    cfg.cards[cfg.cardCount].name = c["name"] | String("");
    cfg.cards[cfg.cardCount].role = c["role"] | String("admin");
    cfg.cardCount++;
  }
}

void saveConfig() {
  DynamicJsonDocument doc(1024);
  doc["ssid"]          = cfg.ssid;
  doc["wifiPass"]      = cfg.wifiPass;
  doc["panelPass"]     = cfg.panelPass;
  doc["threshold"]     = cfg.threshold;
  doc["scanInterval"]  = cfg.scanInterval;
  doc["buzzOnDetect"]  = cfg.buzzOnDetect;
  doc["notifyDetect"]  = cfg.notifyDetect;
  doc["cooldown"]      = cfg.cooldown;
  doc["tuneIndex"]     = cfg.tuneIndex;
  doc["tuneRepeat"]    = cfg.tuneRepeat;
  doc["tempoOverride"] = cfg.tempoOverride;
  doc["msgOnline"]     = cfg.msgOnline;
  doc["msgArmed"]      = cfg.msgArmed;
  doc["msgDisarmed"]   = cfg.msgDisarmed;
  doc["msgAlert"]      = cfg.msgAlert;
  doc["msgUnauth"]     = cfg.msgUnauth;

  JsonArray cards = doc.createNestedArray("cards");
  for (int i = 0; i < cfg.cardCount; i++) {
    JsonObject c = cards.createNestedObject();
    c["uid"]  = cfg.cards[i].uid;
    c["name"] = cfg.cards[i].name;
    c["role"] = cfg.cards[i].role;
  }

  File f = LittleFS.open("/config.json", "w");
  serializeJson(doc, f);
  f.close();
}

// ============================================================
//  TEMPLATE + EVENT TRIGGERS
// ============================================================
String applyTemplate(String tmpl, String name="", String distance="", String uid="") {
  tmpl.replace("{NAME}",     name);
  tmpl.replace("{DISTANCE}", distance);
  tmpl.replace("{UID}",      uid);
  tmpl.replace("{IP}",       WiFi.localIP().toString());
  tmpl.replace("{TIME}",     String(millis()/1000) + "s");
  return tmpl;
}

// Triggers RED fullscreen modal + siren in browser (intrusion / unauth card)
void triggerWebAlert(String msg) {
  currentAlertMsg = msg;
  currentAlertId  = alertCounter++;
  Serial.println("WEB ALERT: " + msg);
}

// [FIX] Sends message to browser terminal log ONLY — no modal, no siren
// Used for ARM / DISARM events so they appear in the web event log
void triggerWebLog(String msg) {
  currentLogMsg = msg;
  currentLogId  = logCounter++;
  Serial.println("LOG: " + msg);
}

// ============================================================
//  BUZZER CONTROLS
// ============================================================
void stopBuzzer() {
  noTone(buzzerPin);
  digitalWrite(buzzerPin, LOW);
}

void beep(int times) {
  for (int i = 0; i < times; i++) {
    tone(buzzerPin, 2000, 100);
    unsigned long startWait = millis();
    while (millis() - startWait < 200) {
      server.handleClient();
      delay(1);
    }
  }
  stopBuzzer();
}

// ============================================================
//  RFID ACCESS
// ============================================================
String lookupCard(const String& uid) {
  for (int i = 0; i < cfg.cardCount; i++) {
    if (cfg.cards[i].uid == uid) return cfg.cards[i].name;
  }
  return "";
}

void checkAccess(const String& uid) {
  String name = lookupCard(uid);
  if (name != "") {
    systemArmed = !systemArmed;
    if (systemArmed) {
      beep(1);
      Serial.println("ARMED by " + name);
      // [FIX] Send ARM message to web terminal using the user's custom template
      triggerWebLog(applyTemplate(cfg.msgArmed, name));
    } else {
      beep(2);
      Serial.println("DISARMED by " + name);
      // [FIX] Send DISARM message to web terminal using the user's custom template
      triggerWebLog(applyTemplate(cfg.msgDisarmed, name));
    }
  } else {
    tone(buzzerPin, 200, 500);
    Serial.println("Unknown card: " + uid);
    // Unauthorized card → full alert modal (red screen + siren)
    triggerWebAlert(applyTemplate(cfg.msgUnauth, "", "", uid));
    unsigned long startWait = millis();
    while (millis() - startWait < 500) {
      server.handleClient();
      delay(1);
    }
    stopBuzzer();
  }
}

void checkRFID() {
  static unsigned long lastRfidTime = 0;
  if (millis() - lastRfidTime < 1000) return;

  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    lastRfidTime = millis();
    String uid = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      if (i > 0) uid += " ";
      if (mfrc522.uid.uidByte[i] < 0x10) uid += "0";
      uid += String(mfrc522.uid.uidByte[i], HEX);
    }
    uid.toUpperCase();

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();

    if (pendingScanUID == "__waiting__") {
      pendingScanUID = uid;
    } else {
      checkAccess(uid);
    }
  }
}

void playTune(int index, int repeat) {
  if (index < 0 || index >= TUNE_COUNT) return;
  const TuneEntry& t = TUNES[index];
  int useTempo  = (cfg.tempoOverride > 0) ? cfg.tempoOverride : t.tempo;
  int wholenote = (60000 * 4) / useTempo;
  int plays     = (repeat == 0) ? 999 : repeat;
  bool wasArmed = systemArmed;

  for (int p = 0; p < plays; p++) {
    for (int i = 0; i < t.noteCount * 2; i += 2) {
      server.handleClient();
      checkRFID();
      if (wasArmed && !systemArmed) { stopBuzzer(); return; }

      int note     = (int)pgm_read_word_near(t.melody + i);
      int divider  = (int)pgm_read_word_near(t.melody + i + 1);
      int duration = (divider > 0) ? (wholenote / divider) : ((wholenote / abs(divider)) * 3 / 2);

      if (note == REST) stopBuzzer();
      else tone(buzzerPin, note, duration * 9 / 10);

      unsigned long startWait = millis();
      while (millis() - startWait < (unsigned long)duration) {
        server.handleClient();
        checkRFID();
        if (wasArmed && !systemArmed) { stopBuzzer(); return; }
        delay(1);
      }
      stopBuzzer();
    }
    unsigned long startWait = millis();
    while (millis() - startWait < 100) {
      server.handleClient(); checkRFID();
      if (wasArmed && !systemArmed) { stopBuzzer(); return; }
      delay(1);
    }
  }
  stopBuzzer();
}

// ============================================================
//  WEB SERVER HELPERS
// ============================================================
bool isAuthorized() {
  if (server.hasHeader("X-Panel-Pass") && server.header("X-Panel-Pass") == cfg.panelPass) return true;
  if (server.hasArg("pass") && server.arg("pass") == cfg.panelPass) return true;
  return false;
}
void sendUnauth() { server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); }
void sendOK(const String& msg = "ok") { server.send(200, "application/json", "{\"status\":\"" + msg + "\"}"); }

// ============================================================
//  API HANDLERS
// ============================================================

void handleRoot() {
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  server.sendContent(F("<meta http-equiv='refresh' content='0;url=/panel'>"));
}

void handleStatus() {
  if (!isAuthorized()) { sendUnauth(); return; }
  DynamicJsonDocument doc(512);
  doc["armed"]     = systemArmed;
  doc["ip"]        = WiFi.localIP().toString();
  doc["cards"]     = cfg.cardCount;
  doc["threshold"] = cfg.threshold;
  doc["tune"]      = TUNES[cfg.tuneIndex].name;
  doc["uptime"]    = millis() / 1000;

  // Full-screen modal alert (intrusion / unauthorized card)
  if (currentAlertId != 0) {
    doc["alertId"]  = currentAlertId;
    doc["alertMsg"] = currentAlertMsg;
  }

  // [FIX] Terminal-log event (arm / disarm) — browser shows in event log only
  if (currentLogId != 0) {
    doc["logId"]  = currentLogId;
    doc["logMsg"] = currentLogMsg;
  }

  if (pendingScanUID != "") {
    doc["scannedUID"] = pendingScanUID;
    if (pendingScanUID != "__waiting__") pendingScanUID = "";
  }

  String out; serializeJson(doc, out);
  server.send(200, "application/json", out);
}

void handleArm() {
  if (!isAuthorized()) { sendUnauth(); return; }
  bool arm = server.arg("state") == "1";
  systemArmed = arm;
  if (arm) reqArmBeep = true; else reqDisarmBeep = true;
  sendOK(arm ? "armed" : "disarmed");
}

void handleGetCards() {
  if (!isAuthorized()) { sendUnauth(); return; }
  DynamicJsonDocument doc(1024);
  JsonArray arr = doc.createNestedArray("cards");
  for (int i = 0; i < cfg.cardCount; i++) {
    JsonObject o = arr.createNestedObject();
    o["uid"] = cfg.cards[i].uid; o["name"] = cfg.cards[i].name; o["role"] = cfg.cards[i].role;
  }
  String out; serializeJson(doc, out); server.send(200, "application/json", out);
}

void handleAddCard() {
  if (!isAuthorized()) { sendUnauth(); return; }
  if (cfg.cardCount >= MAX_CARDS) { server.send(400, "application/json", "{\"error\":\"Max cards reached\"}"); return; }
  String uid = server.arg("uid"); String name = server.arg("name"); String role = server.arg("role");
  uid.toUpperCase();
  for (int i = 0; i < cfg.cardCount; i++) {
    if (cfg.cards[i].uid == uid) { server.send(400, "application/json", "{\"error\":\"Duplicate UID\"}"); return; }
  }
  cfg.cards[cfg.cardCount] = { uid, name, role }; cfg.cardCount++; saveConfig(); sendOK("added");
}

void handleRemoveCard() {
  if (!isAuthorized()) { sendUnauth(); return; }
  String uid = server.arg("uid"); uid.toUpperCase();
  for (int i = 0; i < cfg.cardCount; i++) {
    if (cfg.cards[i].uid == uid) {
      for (int j = i; j < cfg.cardCount - 1; j++) cfg.cards[j] = cfg.cards[j+1];
      cfg.cardCount--; saveConfig(); sendOK("removed"); return;
    }
  }
  server.send(404, "application/json", "{\"error\":\"Not found\"}");
}

void handleScanRequest()  { if (!isAuthorized()) { sendUnauth(); return; } pendingScanUID = "__waiting__"; sendOK("waiting"); }
void handleSaveSensor()   {
  if (!isAuthorized()) { sendUnauth(); return; }
  cfg.threshold    = server.arg("threshold").toInt();
  cfg.scanInterval = server.arg("interval").toInt();
  cfg.buzzOnDetect = server.arg("buzz") == "1";
  cfg.notifyDetect = server.arg("notify") == "1";
  cfg.cooldown     = server.arg("cooldown") == "1";
  saveConfig(); sendOK();
}
void handleSaveTune() {
  if (!isAuthorized()) { sendUnauth(); return; }
  cfg.tuneIndex     = server.arg("index").toInt();
  cfg.tuneRepeat    = server.arg("repeat").toInt();
  cfg.tempoOverride = server.arg("tempo").toInt();
  if (cfg.tuneIndex < 0 || cfg.tuneIndex >= TUNE_COUNT) cfg.tuneIndex = 0;
  saveConfig(); sendOK();
}
void handleTestTune()     { if (!isAuthorized()) { sendUnauth(); return; } reqTestTune = true; sendOK(); }
void handleGetTunes() {
  if (!isAuthorized()) { sendUnauth(); return; }
  DynamicJsonDocument doc(2048);
  JsonArray arr = doc.createNestedArray("tunes");
  for (int i = 0; i < TUNE_COUNT; i++) {
    JsonObject o = arr.createNestedObject(); o["index"] = i; o["name"] = TUNES[i].name; o["tempo"] = TUNES[i].tempo;
  }
  doc["selected"] = cfg.tuneIndex;
  String out; serializeJson(doc, out); server.send(200, "application/json", out);
}
void handleSaveMessages() {
  if (!isAuthorized()) { sendUnauth(); return; }
  cfg.msgOnline   = server.arg("online"); cfg.msgArmed    = server.arg("armed");
  cfg.msgDisarmed = server.arg("disarmed"); cfg.msgAlert  = server.arg("alert");
  cfg.msgUnauth   = server.arg("unauth"); saveConfig(); sendOK();
}
void handleSaveAccess() {
  if (!isAuthorized()) { sendUnauth(); return; }
  String cur = server.arg("current"); String nw = server.arg("newpass");
  if (cur != cfg.panelPass) { server.send(403, "application/json", "{\"error\":\"Wrong pass\"}"); return; }
  cfg.panelPass = nw; saveConfig(); sendOK();
}
void handleGetConfig() {
  if (!isAuthorized()) { sendUnauth(); return; }
  DynamicJsonDocument doc(1024);
  doc["threshold"]     = cfg.threshold; doc["scanInterval"]  = cfg.scanInterval;
  doc["buzzOnDetect"]  = cfg.buzzOnDetect; doc["notifyDetect"]  = cfg.notifyDetect;
  doc["cooldown"]      = cfg.cooldown; doc["tuneIndex"]     = cfg.tuneIndex;
  doc["tuneRepeat"]    = cfg.tuneRepeat; doc["tempoOverride"] = cfg.tempoOverride;
  doc["msgOnline"]     = cfg.msgOnline; doc["msgArmed"]      = cfg.msgArmed;
  doc["msgDisarmed"]   = cfg.msgDisarmed; doc["msgAlert"]    = cfg.msgAlert;
  doc["msgUnauth"]     = cfg.msgUnauth;
  String out; serializeJson(doc, out); server.send(200, "application/json", out);
}
void handleTestBuzz()       { if (!isAuthorized()) { sendUnauth(); return; } reqTestBuzz = true; sendOK(); }
void handleWebAlertTest()   { if (!isAuthorized()) { sendUnauth(); return; } triggerWebAlert("This is a Web Alert Test!"); sendOK(); }

// ============================================================
//  FRONTEND HTML
// ============================================================
const char PANEL_HTML[] PROGMEM = R"===HTMLBLOCK===(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1.0">
<title>GUARD SYS // CONFIG PANEL</title>
<link href="https://fonts.googleapis.com/css2?family=Share+Tech+Mono&family=Barlow+Condensed:wght@300;400;600;700&display=swap" rel="stylesheet">
<style>
:root{--bg:#0a0b0d;--surface:#111318;--border:#1f2530;--amber:#e8a020;--amber-dim:#7a4f08;--green:#2dce6a;--green-dim:#0e4f26;--red:#e84040;--red-dim:#4f0e0e;--text:#c8cdd8;--muted:#4a5168;--mono:'Share Tech Mono',monospace;--sans:'Barlow Condensed',sans-serif}
*,*::before,*::after{box-sizing:border-box;margin:0;padding:0}
body{background:var(--bg);color:var(--text);font-family:var(--mono);min-height:100vh;overflow-x:hidden}
body::before{content:'';position:fixed;inset:0;background:repeating-linear-gradient(0deg,transparent,transparent 2px,rgba(0,0,0,.15) 2px,rgba(0,0,0,.15) 4px);pointer-events:none;z-index:9999}
#login-screen{position:fixed;inset:0;display:flex;flex-direction:column;align-items:center;justify-content:center;background:var(--bg);z-index:1000}
.login-box{border:1px solid var(--amber);background:var(--surface);padding:40px 48px;width:380px;position:relative;box-shadow:0 0 60px rgba(232,160,32,.12)}
.login-badge{position:absolute;top:-13px;left:16px;font-family:var(--sans);font-size:10px;font-weight:700;letter-spacing:3px;color:var(--amber);background:var(--surface);padding:0 8px;text-transform:uppercase}
.login-logo{font-family:var(--sans);font-size:28px;font-weight:700;letter-spacing:6px;color:var(--amber);text-align:center;margin-bottom:4px}
.login-sub{font-size:11px;color:var(--muted);text-align:center;letter-spacing:2px;margin-bottom:32px}
.login-label{display:block;font-size:10px;letter-spacing:2px;color:var(--muted);text-transform:uppercase;margin-bottom:6px}
.login-input{width:100%;background:var(--bg);border:1px solid var(--border);color:var(--amber);font-family:var(--mono);font-size:14px;padding:10px 14px;outline:none;letter-spacing:2px;margin-bottom:20px}
.login-input:focus{border-color:var(--amber)}
.login-btn{width:100%;background:var(--amber);color:#000;border:none;font-family:var(--sans);font-size:13px;font-weight:700;letter-spacing:4px;padding:12px;cursor:pointer;text-transform:uppercase}
.login-err{color:var(--red);font-size:11px;text-align:center;margin-top:12px;letter-spacing:1px;min-height:16px}
#app{display:none}
.topbar{display:flex;align-items:center;justify-content:space-between;padding:0 32px;height:56px;border-bottom:1px solid var(--border);background:var(--surface);position:sticky;top:0;z-index:100}
.topbar-brand{font-family:var(--sans);font-size:18px;font-weight:700;letter-spacing:5px;color:var(--amber)}
.topbar-ip{font-size:12px;color:var(--muted);letter-spacing:1px}
.topbar-right{display:flex;align-items:center;gap:16px}
.status-dot{width:8px;height:8px;border-radius:50%;background:var(--green);box-shadow:0 0 8px var(--green);animation:blink 2s ease-in-out infinite}
@keyframes blink{0%,100%{opacity:1}50%{opacity:.3}}
.status-label{font-size:11px;color:var(--green);letter-spacing:2px}
.logout-btn{background:none;border:1px solid var(--border);color:var(--muted);font-family:var(--mono);font-size:11px;padding:5px 14px;cursor:pointer;letter-spacing:1px}
.layout{display:grid;grid-template-columns:200px 1fr;min-height:calc(100vh - 56px)}
.sidebar{border-right:1px solid var(--border);background:var(--surface);padding:24px 0}
.sidebar-section{font-family:var(--sans);font-size:9px;font-weight:600;letter-spacing:3px;color:var(--muted);text-transform:uppercase;padding:0 20px 8px;margin-top:20px}
.nav-item{display:flex;align-items:center;gap:10px;padding:10px 20px;cursor:pointer;font-size:12px;letter-spacing:1px;color:var(--muted);border-left:2px solid transparent;transition:all .15s;user-select:none}
.nav-item:hover{color:var(--text)}
.nav-item.active{color:var(--amber);border-left-color:var(--amber);background:rgba(232,160,32,.06)}
.main{padding:32px;overflow-y:auto}
.panel{display:none}.panel.active{display:block}
.section-title{font-family:var(--sans);font-size:22px;font-weight:600;letter-spacing:3px;color:var(--amber);text-transform:uppercase;margin-bottom:4px}
.section-desc{font-size:11px;color:var(--muted);letter-spacing:1px;margin-bottom:28px}
.card{background:var(--surface);border:1px solid var(--border);padding:24px;margin-bottom:20px}
.card-title{font-family:var(--sans);font-size:11px;font-weight:600;letter-spacing:3px;color:var(--muted);text-transform:uppercase;margin-bottom:20px;padding-bottom:12px;border-bottom:1px solid var(--border)}
.field{margin-bottom:18px}
.field label{display:block;font-size:10px;letter-spacing:2px;color:var(--muted);text-transform:uppercase;margin-bottom:6px}
.field input[type=text],.field input[type=password],.field input[type=number],.field select,.field textarea{width:100%;background:var(--bg);border:1px solid var(--border);color:var(--text);font-family:var(--mono);font-size:13px;padding:9px 12px;outline:none;transition:border-color .2s}
.field input:focus,.field select:focus,.field textarea:focus{border-color:var(--amber)}
.field select{appearance:none;background-image:url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' width='10' height='6' fill='%234a5168'%3E%3Cpath d='M0 0l5 6 5-6z'/%3E%3C/svg%3E");background-repeat:no-repeat;background-position:right 12px center;cursor:pointer}
.field select option{background:var(--bg)}
.slider-wrap{display:flex;align-items:center;gap:16px}
.field input[type=range]{-webkit-appearance:none;width:100%;height:4px;background:var(--border);outline:none;cursor:pointer}
.field input[type=range]::-webkit-slider-thumb{-webkit-appearance:none;width:16px;height:16px;background:var(--amber);border-radius:0;cursor:pointer}
.slider-val{font-size:16px;color:var(--amber);min-width:60px;text-align:right}
.btn{font-family:var(--sans);font-size:12px;font-weight:600;letter-spacing:3px;text-transform:uppercase;padding:10px 24px;cursor:pointer;border:none}
.btn-amber{background:var(--amber);color:#000}
.btn-outline{background:transparent;border:1px solid var(--border);color:var(--muted)}
.btn-outline:hover{border-color:var(--amber);color:var(--amber)}
.btn-red{background:var(--red);color:#fff}
.btn-row{display:flex;gap:12px;align-items:center;flex-wrap:wrap}
.grid-2{display:grid;grid-template-columns:1fr 1fr;gap:20px}
.cards-table{width:100%;border-collapse:collapse}
.cards-table th{text-align:left;font-size:9px;letter-spacing:2px;color:var(--muted);text-transform:uppercase;padding:8px 12px;border-bottom:1px solid var(--border);font-family:var(--sans);font-weight:600}
.cards-table td{padding:12px;font-size:12px;border-bottom:1px solid rgba(31,37,48,.5);vertical-align:middle}
.cards-table tr:last-child td{border-bottom:none}
.uid-badge{font-size:11px;color:var(--amber);letter-spacing:1px;background:rgba(232,160,32,.08);padding:3px 8px;border:1px solid var(--amber-dim);display:inline-block}
.role-badge{font-size:10px;letter-spacing:1px;padding:2px 8px;border:1px solid;display:inline-block;text-transform:uppercase;font-family:var(--sans);font-weight:600}
.role-admin{color:var(--green);border-color:var(--green-dim);background:rgba(45,206,106,.06)}
.role-user{color:var(--amber);border-color:var(--amber-dim);background:rgba(232,160,32,.06)}
.del-btn{background:none;border:1px solid var(--red-dim);color:var(--red);font-family:var(--mono);font-size:11px;padding:4px 10px;cursor:pointer;transition:all .2s;letter-spacing:1px}
.del-btn:hover{background:var(--red);color:#fff}
.toggle-wrap{display:flex;align-items:center;gap:12px;margin-bottom:12px}
.toggle{position:relative;width:44px;height:22px;cursor:pointer;flex-shrink:0}
.toggle input{opacity:0;width:0;height:0}
.toggle-slider{position:absolute;inset:0;background:var(--border);transition:background .2s;border:1px solid var(--muted)}
.toggle-slider::before{content:'';position:absolute;top:2px;left:2px;width:16px;height:16px;background:var(--muted);transition:transform .2s,background .2s}
.toggle input:checked+.toggle-slider{background:rgba(45,206,106,.15);border-color:var(--green)}
.toggle input:checked+.toggle-slider::before{transform:translateX(22px);background:var(--green)}
.toggle-label{font-size:12px;color:var(--text);letter-spacing:1px}
#toast{position:fixed;bottom:28px;right:28px;background:var(--surface);border:1px solid var(--green);color:var(--green);font-size:12px;padding:12px 20px;letter-spacing:1px;opacity:0;transform:translateY(10px);transition:all .3s;z-index:9000;pointer-events:none}
#toast.show{opacity:1;transform:translateY(0)}
#toast.err{border-color:var(--red);color:var(--red)}
.stat-grid{display:grid;grid-template-columns:repeat(4,1fr);gap:12px;margin-bottom:20px}
.stat-tile{background:var(--surface);border:1px solid var(--border);padding:16px}
.stat-label{font-size:9px;letter-spacing:2px;color:var(--muted);text-transform:uppercase;font-family:var(--sans);font-weight:600;margin-bottom:8px}
.stat-val{font-size:22px;font-family:var(--sans);font-weight:700;letter-spacing:1px}
.stat-amber{color:var(--amber)}.stat-green{color:var(--green)}.stat-red{color:var(--red)}.stat-muted{color:var(--muted)}
.terminal{background:var(--bg);border:1px solid var(--border);padding:16px;font-size:12px;height:220px;overflow-y:auto;line-height:1.7}
.log-line{display:flex;gap:12px}
.log-time{color:var(--muted);flex-shrink:0}
.log-msg-green{color:var(--green)}.log-msg-amber{color:var(--amber)}.log-msg-red{color:var(--red)}
.song-grid{display:grid;grid-template-columns:repeat(auto-fill,minmax(160px,1fr));gap:8px}
.song-card{border:1px solid var(--border);padding:12px 14px;cursor:pointer;transition:all .15s;position:relative}
.song-card:hover{border-color:var(--amber-dim);background:rgba(232,160,32,.04)}
.song-card.selected{border-color:var(--amber);background:rgba(232,160,32,.08)}
.song-card.selected::before{content:'✔';position:absolute;top:8px;right:10px;color:var(--amber);font-size:11px}
.song-name{font-size:12px;color:var(--text);line-height:1.4}
.msg-vars{display:flex;gap:8px;flex-wrap:wrap;margin-bottom:12px}
.var-chip{background:rgba(232,160,32,.08);border:1px solid var(--amber-dim);color:var(--amber);font-size:11px;padding:3px 10px;cursor:pointer;letter-spacing:.5px}
.field textarea{min-height:60px;resize:vertical;line-height:1.6}
.scan-uid{font-size:14px;color:var(--green);letter-spacing:2px;padding:8px 0;min-height:24px}
/* ALERT MODAL */
#alert-modal{position:fixed;inset:0;background:rgba(232,64,64,0.9);z-index:9999;display:none;flex-direction:column;align-items:center;justify-content:center;text-align:center}
#alert-modal.show{display:flex;animation:flashBg 1s infinite alternate}
@keyframes flashBg{0%{background:rgba(232,64,64,0.95)}100%{background:rgba(180,20,20,0.95)}}
.alert-title{font-family:var(--sans);font-size:64px;font-weight:700;color:#fff;letter-spacing:8px;margin-bottom:20px;text-shadow:0 0 20px rgba(0,0,0,0.5)}
.alert-msg{font-size:24px;color:#fff;margin-bottom:40px;padding:0 40px}
.alert-btn{background:#fff;color:var(--red);border:none;font-family:var(--sans);font-size:20px;font-weight:700;letter-spacing:4px;padding:16px 40px;cursor:pointer}
</style>
</head>
<body>

<div id="alert-modal">
  <div class="alert-title">SYSTEM ALERT</div>
  <div class="alert-msg" id="alert-modal-msg">Intrusion Detected!</div>
  <button class="alert-btn" onclick="dismissAlert()">DISMISS ALERT</button>
</div>

<div id="login-screen">
  <div class="login-box">
    <span class="login-badge">AUTH REQUIRED</span>
    <div class="login-logo">GUARD SYS</div>
    <div class="login-sub">ESP8266 &middot; SECURITY CONTROLLER</div>
    <label class="login-label">Panel Password</label>
    <input class="login-input" type="password" id="login-pwd" placeholder="&bull;&bull;&bull;&bull;&bull;&bull;&bull;&bull;" autocomplete="off">
    <button class="login-btn" onclick="doLogin()">AUTHENTICATE</button>
    <div class="login-err" id="login-err"></div>
  </div>
</div>

<div id="app">
  <div class="topbar">
    <div style="display:flex;align-items:center;gap:20px">
      <div class="topbar-brand">GUARD SYS</div>
      <span style="color:var(--border)">|</span>
      <span class="topbar-ip" id="topbar-ip">—</span>
    </div>
    <div class="topbar-right">
      <div class="status-dot"></div>
      <span class="status-label">ONLINE</span>
      <button class="logout-btn" onclick="doLogout()">LOGOUT</button>
    </div>
  </div>

  <div class="layout">
    <div class="sidebar">
      <div class="sidebar-section">System</div>
      <div class="nav-item active" onclick="nav('dashboard')"><span>⬡</span> Dashboard</div>
      <div class="nav-item" onclick="nav('cards')"><span>▣</span> Cards</div>
      <div class="sidebar-section">Settings</div>
      <div class="nav-item" onclick="nav('sensor')"><span>◎</span> Sensor & Alerts</div>
      <div class="nav-item" onclick="nav('tunes')"><span>♪</span> Alarm Tune</div>
      <div class="nav-item" onclick="nav('messages')"><span>✉</span> Messages</div>
      <div class="sidebar-section">Security</div>
      <div class="nav-item" onclick="nav('access')"><span>⚿</span> Access</div>
    </div>

    <div class="main">

      <!-- DASHBOARD -->
      <div class="panel active" id="panel-dashboard">
        <div class="section-title">Dashboard</div>
        <div class="section-desc">System overview and live status</div>
        <div class="stat-grid">
          <div class="stat-tile"><div class="stat-label">System State</div><div class="stat-val stat-green" id="dash-armed">—</div></div>
          <div class="stat-tile"><div class="stat-label">Card Holders</div><div class="stat-val stat-amber" id="dash-cards">—</div></div>
          <div class="stat-tile"><div class="stat-label">Threshold</div><div class="stat-val stat-amber" id="dash-dist">—</div></div>
          <div class="stat-tile"><div class="stat-label">Uptime</div><div class="stat-val stat-muted" id="dash-uptime">—</div></div>
        </div>
        <div class="card">
          <div class="card-title">Event Log</div>
          <div class="terminal" id="log-terminal">
            <div class="log-line"><span class="log-time">--:--:--</span><span class="log-msg-green">Connected to panel. Awaiting events.</span></div>
          </div>
        </div>
        <div class="card">
          <div class="card-title">Quick Controls</div>
          <div class="btn-row">
            <button class="btn btn-amber" onclick="toggleArm()">ARM / DISARM</button>
            <button class="btn btn-outline" onclick="api('/api/buzz/test','POST').then(()=>toast('Buzzer test sent')).catch(e=>toast('Error: '+e.message,true))">TEST BUZZER</button>
            <button class="btn btn-outline" onclick="api('/api/webalert/test','POST').then(()=>toast('Test alert triggered')).catch(e=>toast('Error: '+e.message,true))">TEST WEB ALERT</button>
            <button class="btn btn-outline" onclick="api('/api/tune/test','POST').then(()=>toast('Playing tune...')).catch(e=>toast('Error: '+e.message,true))">TEST TUNE</button>
          </div>
        </div>
      </div>

      <!-- CARDS -->
      <div class="panel" id="panel-cards">
        <div class="section-title">Card Management</div>
        <div class="section-desc">Add, remove, and manage RFID card holders</div>
        <div class="card">
          <div class="card-title">Registered Cards</div>
          <table class="cards-table"><thead><tr><th>#</th><th>Name</th><th>Card UID</th><th>Role</th><th>Action</th></tr></thead>
          <tbody id="cards-body"><tr><td colspan="5" style="color:var(--muted);text-align:center">Loading...</td></tr></tbody></table>
        </div>
        <div class="card">
          <div class="card-title">Add New Card</div>
          <div class="grid-2">
            <div class="field"><label>Holder Name</label><input type="text" id="new-name" placeholder="e.g. John Doe"></div>
            <div class="field"><label>Card UID (XX XX XX XX)</label><input type="text" id="new-uid" placeholder="e.g. AB 12 CD 34" maxlength="11" oninput="fmtUID(this)"></div>
          </div>
          <div class="field"><label>Role</label><select id="new-role"><option value="admin">Admin</option><option value="user">User</option></select></div>
          <div class="btn-row"><button class="btn btn-amber" onclick="addCard()">ADD CARD</button><button class="btn btn-outline" onclick="startScan()">SCAN NEXT RFID TAP</button></div>
          <div class="scan-uid" id="scan-result"></div>
        </div>
      </div>

      <!-- SENSOR -->
      <div class="panel" id="panel-sensor">
        <div class="section-title">Sensor & Alerts</div>
        <div class="section-desc">Ultrasonic sensor calibration & browser notifications</div>
        <div class="card">
          <div class="card-title">Detection Threshold</div>
          <div class="field"><label>Trigger Distance</label>
            <div class="slider-wrap"><input type="range" id="dist-slider" min="5" max="200" value="20" oninput="document.getElementById('dist-label').textContent=this.value+' cm'"><span class="slider-val" id="dist-label">20 cm</span></div>
          </div>
        </div>
        <div class="card">
          <div class="card-title">Options</div>
          <div class="field"><label>Scan Interval (ms)</label><input type="number" id="scan-interval" value="100" min="50" max="5000"></div>
          <div class="toggle-wrap"><label class="toggle"><input type="checkbox" id="tog-buzz" checked><span class="toggle-slider"></span></label><span class="toggle-label">Buzzer alarm on detection</span></div>
          <div class="toggle-wrap"><label class="toggle"><input type="checkbox" id="tog-notify" checked><span class="toggle-slider"></span></label><span class="toggle-label">Send Web Alerts to browser</span></div>
          <div class="toggle-wrap"><label class="toggle"><input type="checkbox" id="tog-cooldown" checked><span class="toggle-slider"></span></label><span class="toggle-label">Cooldown (one alert per event)</span></div>
          <button class="btn btn-amber" style="margin-top:12px" onclick="saveSensor()">SAVE SETTINGS</button>
        </div>
      </div>

      <!-- TUNES -->
      <div class="panel" id="panel-tunes">
        <div class="section-title">Alarm Tune</div>
        <div class="section-desc">Select melody played on intrusion detection</div>
        <div class="card">
          <div class="card-title">Tune Selection</div>
          <div class="song-grid" id="song-grid"></div>
          <div style="margin-top:16px;padding-top:16px;border-top:1px solid var(--border);font-size:12px"><span style="color:var(--muted)">SELECTED: </span><span style="color:var(--amber)" id="sel-song-label">—</span></div>
        </div>
        <div class="card">
          <div class="card-title">Playback Settings</div>
          <div class="grid-2">
            <div class="field"><label>Tempo Override (0 = default)</label>
              <div class="slider-wrap"><input type="range" id="tempo-slider" min="0" max="300" value="0" oninput="document.getElementById('tempo-label').textContent=this.value==0?'Default':this.value+' BPM'"><span class="slider-val" id="tempo-label">Default</span></div>
            </div>
            <div class="field"><label>Repeat</label>
              <select id="tune-repeat"><option value="1">Play Once</option><option value="2">Repeat 2x</option><option value="0">Loop Until Disarmed</option></select>
            </div>
          </div>
          <div class="btn-row"><button class="btn btn-amber" onclick="saveTune()">SAVE TUNE</button><button class="btn btn-outline" onclick="api('/api/tune/test','POST').then(()=>toast('Playing on device...')).catch(e=>toast('Error: '+e.message,true))">TEST ON DEVICE</button></div>
        </div>
      </div>

      <!-- MESSAGES -->
      <div class="panel" id="panel-messages">
        <div class="section-title">Message Templates</div>
        <div class="section-desc">Customise alert notification text</div>
        <div style="margin-bottom:12px;font-size:11px;color:var(--muted)">Click a chip to insert at cursor:</div>
        <div class="msg-vars">
          <span class="var-chip" onclick="insertVar('{NAME}')">&#123;NAME&#125;</span>
          <span class="var-chip" onclick="insertVar('{DISTANCE}')">&#123;DISTANCE&#125;</span>
          <span class="var-chip" onclick="insertVar('{TIME}')">&#123;TIME&#125;</span>
          <span class="var-chip" onclick="insertVar('{UID}')">&#123;UID&#125;</span>
        </div>
        <div class="card">
          <div class="card-title">Templates</div>
          <div class="field"><label>System Online</label><input type="text" id="msg-online"></div>
          <div class="field"><label>Armed — shown in event log when card arms device</label><input type="text" id="msg-armed"></div>
          <div class="field"><label>Disarmed — shown in event log when card disarms device</label><input type="text" id="msg-disarmed"></div>
          <div class="field"><label>Intrusion Alert — triggers red modal + siren</label><input type="text" id="msg-alert"></div>
          <div class="field"><label>Unauthorized Card — triggers red modal + siren</label><input type="text" id="msg-unauth"></div>
          <button class="btn btn-amber" onclick="saveMessages()">SAVE TEMPLATES</button>
        </div>
      </div>

      <!-- ACCESS -->
      <div class="panel" id="panel-access">
        <div class="section-title">Access Control</div>
        <div class="section-desc">Panel password management</div>
        <div class="card">
          <div class="card-title">Change Password</div>
          <div class="field"><label>Current Password</label><input type="password" id="pwd-current"></div>
          <div class="field"><label>New Password</label><input type="password" id="pwd-new"></div>
          <div class="field"><label>Confirm New Password</label><input type="password" id="pwd-confirm"></div>
          <button class="btn btn-amber" onclick="changePwd()">UPDATE PASSWORD</button>
        </div>
      </div>

    </div>
  </div>
</div>
<div id="toast"></div>

<script>
let AUTH_PASS = '';
let selectedTune = 0;
let allTunes = [];
let lastFocused = null;
let statusTimer = null;
let scanPollTimer = null;
let armed = false;
let uptimeSec = 0;
let lastAlertId = 0;
// [FIX] Track last seen log event ID so arm/disarm only displays once
let lastLogId = 0;
let isInitialLoad = true;

document.addEventListener('DOMContentLoaded', () => {
  if (Notification.permission !== "granted" && Notification.permission !== "denied") {
    Notification.requestPermission();
  }
});

async function doLogin(){
  const p = document.getElementById('login-pwd').value;
  try {
    const r = await fetch('/api/status', {headers:{'X-Panel-Pass':p}});
    if(r.ok){
      AUTH_PASS = p;
      document.getElementById('login-screen').style.display='none';
      document.getElementById('app').style.display='block';
      if(Notification.permission !== "granted") Notification.requestPermission();
      initApp();
    } else {
      const e = document.getElementById('login-err');
      e.textContent='ACCESS DENIED'; setTimeout(()=>e.textContent='',3000);
      document.getElementById('login-pwd').value='';
    }
  } catch(e) {
    const err = document.getElementById('login-err');
    err.textContent='SYSTEM OFFLINE'; setTimeout(()=>err.textContent='',3000);
  }
}
document.addEventListener('DOMContentLoaded',()=>{
  document.getElementById('login-pwd').addEventListener('keydown',e=>{if(e.key==='Enter')doLogin();});
  document.addEventListener('focusin',e=>{if(e.target.tagName==='INPUT'||e.target.tagName==='TEXTAREA')lastFocused=e.target;});
});
function doLogout(){
  AUTH_PASS=''; clearInterval(statusTimer); clearInterval(scanPollTimer);
  document.getElementById('app').style.display='none';
  document.getElementById('login-screen').style.display='flex';
  document.getElementById('login-pwd').value='';
}

async function api(url, method='GET', params={}){
  const opts = {method, headers:{'X-Panel-Pass':AUTH_PASS}};
  if(method==='POST'){
    if(Object.keys(params).length > 0){
      const fd = new FormData(); Object.keys(params).forEach(k=>fd.append(k,params[k])); opts.body = fd;
    } else {
      opts.body = "{}"; opts.headers['Content-Type'] = 'application/json';
    }
  }
  const r = await fetch(url, opts);
  if(!r.ok) throw new Error(await r.text());
  return await r.json();
}

async function initApp(){
  await loadTunes(); await loadConfig(); await loadCards();
  refreshStatus();
  statusTimer = setInterval(refreshStatus, 1500);
  setInterval(()=>{
    uptimeSec++;
    const h=String(Math.floor(uptimeSec/3600)).padStart(2,'0');
    const m=String(Math.floor((uptimeSec%3600)/60)).padStart(2,'0');
    const s=String(uptimeSec%60).padStart(2,'0');
    document.getElementById('dash-uptime').textContent=h+':'+m+':'+s;
  },1000);
}

async function refreshStatus(){
  try{
    const d = await api('/api/status');
    armed = d.armed;
    const armEl = document.getElementById('dash-armed');
    armEl.textContent = d.armed ? 'ARMED' : 'DISARMED';
    armEl.className = 'stat-val ' + (d.armed ? 'stat-red' : 'stat-green');
    document.getElementById('dash-cards').textContent = d.cards;
    document.getElementById('dash-dist').textContent = d.threshold + ' cm';
    document.getElementById('topbar-ip').textContent = d.ip;

    // ── Full-screen modal alert (intrusion / unauth card) ──────────────
    if (d.alertId) {
      if (isInitialLoad) {
        lastAlertId = d.alertId;
        isInitialLoad = false;
      } else if (d.alertId !== lastAlertId) {
        triggerFrontendAlert(d.alertMsg);
        lastAlertId = d.alertId;
      }
    } else {
      isInitialLoad = false;
    }

    // [FIX] ── Terminal-only log (arm / disarm events) ──────────────────
    // Picks up logId + logMsg sent from triggerWebLog() on the device.
    // On first load we just sync the ID without showing anything (avoids
    // replaying old arm/disarm events every time the page is opened).
    if (d.logId) {
      if (lastLogId === 0) {
        // First poll — just record where we are, don't display
        lastLogId = d.logId;
      } else if (d.logId !== lastLogId) {
        // New event since last poll — add to terminal with colour coding
        addLog(d.logMsg, getLogClass(d.logMsg));
        lastLogId = d.logId;
      }
    }

  }catch(e){}
}

// Colour-codes arm/disarm/unauth terminal lines automatically
function getLogClass(msg) {
  const m = (msg || '').toLowerCase();
  if (m.includes('disarm'))         return 'log-msg-green';
  if (m.includes('arm'))            return 'log-msg-red';
  if (m.includes('unauthorized') || m.includes('unauth')) return 'log-msg-amber';
  return 'log-msg-amber';
}

function playWebSiren() {
  try {
    const AudioContext = window.AudioContext || window.webkitAudioContext;
    if(!AudioContext) return;
    const ctx = new AudioContext();
    const osc = ctx.createOscillator(); const gain = ctx.createGain();
    osc.connect(gain); gain.connect(ctx.destination);
    osc.type = 'sawtooth';
    for(let i=0; i<6; i++){
      osc.frequency.setValueAtTime(600, ctx.currentTime + i*0.5);
      osc.frequency.linearRampToValueAtTime(1200, ctx.currentTime + i*0.5 + 0.25);
    }
    gain.gain.setValueAtTime(0.5, ctx.currentTime);
    osc.start(ctx.currentTime); osc.stop(ctx.currentTime + 3.0);
  } catch(e) {}
}

function triggerFrontendAlert(msg) {
  addLog("ALERT: " + msg, "log-msg-red");
  document.getElementById('alert-modal-msg').textContent = msg;
  document.getElementById('alert-modal').classList.add('show');
  playWebSiren();
  if (Notification.permission === 'granted') new Notification('GUARD SYS ALERT', { body: msg });
}

function dismissAlert() {
  document.getElementById('alert-modal').classList.remove('show');
}

function nav(id){
  document.querySelectorAll('.panel').forEach(p=>p.classList.remove('active'));
  document.querySelectorAll('.nav-item').forEach(n=>n.classList.remove('active'));
  document.getElementById('panel-'+id).classList.add('active');
  document.querySelectorAll('.nav-item').forEach(n=>{
    if(n.getAttribute('onclick')&&n.getAttribute('onclick').includes("'"+id+"'"))n.classList.add('active');
  });
}

async function loadCards(){
  const d = await api('/api/cards');
  const tbody = document.getElementById('cards-body'); tbody.innerHTML='';
  if(!d.cards.length){ tbody.innerHTML='<tr><td colspan="5" style="color:var(--muted);text-align:center;padding:16px">No cards registered</td></tr>'; return; }
  d.cards.forEach((c,i)=>{
    tbody.innerHTML+=`<tr><td style="color:var(--muted)">${i+1}</td><td>${c.name}</td><td><span class="uid-badge">${c.uid}</span></td><td><span class="role-badge role-${c.role}">${c.role}</span></td><td><button class="del-btn" onclick="delCard('${c.uid}')">REMOVE</button></td></tr>`;
  });
}

async function addCard(){
  const name = document.getElementById('new-name').value.trim();
  const uid  = document.getElementById('new-uid').value.trim().toUpperCase();
  const role = document.getElementById('new-role').value;
  if(!name||!uid){toast('Fill in name and UID',true);return;}
  if(!/^[0-9A-F]{2}( [0-9A-F]{2}){3}$/.test(uid)){toast('UID must be XX XX XX XX',true);return;}
  try{
    await api('/api/cards/add','POST',{name,uid,role});
    toast('Card added'); document.getElementById('new-name').value=''; document.getElementById('new-uid').value=''; document.getElementById('scan-result').textContent='';
    await loadCards(); await refreshStatus();
  }catch(e){toast('Error: '+e.message,true);}
}

async function delCard(uid){
  if(!confirm('Remove card '+uid+'?'))return;
  try{ await api('/api/cards/remove','POST',{uid}); toast('Card removed'); await loadCards(); await refreshStatus(); }
  catch(e){toast('Error: '+e.message,true);}
}

function fmtUID(el){
  let v=el.value.replace(/[^0-9a-fA-F]/g,'').toUpperCase(); let o='';
  for(let i=0;i<v.length&&i<8;i++){if(i>0&&i%2===0)o+=' ';o+=v[i];} el.value=o;
}

async function startScan(){
  try{
    await api('/api/scan','POST');
    document.getElementById('scan-result').textContent='Waiting for RFID tap...';
    scanPollTimer = setInterval(async()=>{
      const d = await api('/api/status');
      if(d.scannedUID && d.scannedUID !== '__waiting__'){
        clearInterval(scanPollTimer);
        document.getElementById('scan-result').textContent='Scanned: '+d.scannedUID;
        document.getElementById('new-uid').value=d.scannedUID;
      }
    },1000);
  }catch(e){toast('Error',true);}
}

async function saveSensor(){
  try{
    await api('/api/sensor','POST',{
      threshold: document.getElementById('dist-slider').value,
      interval:  document.getElementById('scan-interval').value,
      buzz:      document.getElementById('tog-buzz').checked?'1':'0',
      notify:    document.getElementById('tog-notify').checked?'1':'0',
      cooldown:  document.getElementById('tog-cooldown').checked?'1':'0'
    });
    toast('Sensor settings saved'); await refreshStatus();
  }catch(e){toast('Error',true);}
}

async function loadTunes(){
  const d = await api('/api/tunes');
  allTunes = d.tunes; selectedTune = d.selected;
  const grid = document.getElementById('song-grid'); grid.innerHTML='';
  allTunes.forEach(t=>{
    const el=document.createElement('div');
    el.className='song-card'+(t.index===selectedTune?' selected':'');
    el.innerHTML=`<div class="song-name">${t.name}</div>`;
    el.onclick=()=>{selectedTune=t.index;document.querySelectorAll('.song-card').forEach(c=>c.classList.remove('selected'));el.classList.add('selected'); document.getElementById('sel-song-label').textContent=t.name;};
    grid.appendChild(el);
  });
  document.getElementById('sel-song-label').textContent = (allTunes.find(t=>t.index===selectedTune)||{name:'—'}).name;
}

async function saveTune(){
  try{
    await api('/api/tune','POST',{ index: selectedTune, repeat: document.getElementById('tune-repeat').value, tempo: document.getElementById('tempo-slider').value });
    toast('Tune saved');
  }catch(e){toast('Error',true);}
}

async function saveMessages(){
  try{
    await api('/api/messages','POST',{
      online: document.getElementById('msg-online').value, armed: document.getElementById('msg-armed').value,
      disarmed: document.getElementById('msg-disarmed').value, alert: document.getElementById('msg-alert').value,
      unauth: document.getElementById('msg-unauth').value
    });
    toast('Templates saved');
  }catch(e){toast('Error',true);}
}

function insertVar(v){
  if(!lastFocused)return;
  const el=lastFocused,s=el.selectionStart,e=el.selectionEnd;
  el.value=el.value.substring(0,s)+v+el.value.substring(e);
  el.selectionStart=el.selectionEnd=s+v.length; el.focus();
}

async function changePwd(){
  const cur=document.getElementById('pwd-current').value;
  const nw =document.getElementById('pwd-new').value;
  const cf =document.getElementById('pwd-confirm').value;
  if(nw!==cf){toast('Passwords do not match',true);return;}
  try{
    await api('/api/access','POST',{current:cur,newpass:nw});
    AUTH_PASS=nw; toast('Password updated');
    document.getElementById('pwd-current').value=''; document.getElementById('pwd-new').value=''; document.getElementById('pwd-confirm').value='';
  }catch(e){toast('Wrong password',true);}
}

async function toggleArm(){
  try{
    await api('/api/arm','POST',{state:armed?'0':'1'});
    armed=!armed; await refreshStatus();
    addLog('System '+(armed?'ARMED':'DISARMED')+' via web panel', armed?'log-msg-red':'log-msg-green');
  }catch(e){toast('Connection error',true);}
}

async function loadConfig(){
  try{
    const d = await api('/api/config');
    document.getElementById('dist-slider').value= d.threshold||20; document.getElementById('dist-label').textContent=(d.threshold||20)+' cm';
    document.getElementById('scan-interval').value=d.scanInterval||100;
    document.getElementById('tog-buzz').checked   = d.buzzOnDetect!==false;
    document.getElementById('tog-notify').checked = d.notifyDetect!==false;
    document.getElementById('tog-cooldown').checked=d.cooldown!==false;
    document.getElementById('tune-repeat').value  = String(d.tuneRepeat||1);
    document.getElementById('tempo-slider').value = d.tempoOverride||0;
    document.getElementById('tempo-label').textContent=d.tempoOverride?d.tempoOverride+' BPM':'Default';
    document.getElementById('msg-online').value   = d.msgOnline||'';
    document.getElementById('msg-armed').value    = d.msgArmed||'';
    document.getElementById('msg-disarmed').value = d.msgDisarmed||'';
    document.getElementById('msg-alert').value    = d.msgAlert||'';
    document.getElementById('msg-unauth').value   = d.msgUnauth||'';
  }catch(e){}
}

function addLog(msg,cls){
  const t=document.getElementById('log-terminal'); const now=new Date();
  const ts=[now.getHours(),now.getMinutes(),now.getSeconds()].map(n=>String(n).padStart(2,'0')).join(':');
  const line=document.createElement('div'); line.className='log-line';
  line.innerHTML=`<span class="log-time">${ts}</span><span class="${cls}">${msg}</span>`;
  t.appendChild(line); t.scrollTop=t.scrollHeight;
}

let toastT;
function toast(msg,err=false){
  const el=document.getElementById('toast'); el.textContent=(err?'✖ ':'✔ ')+msg;
  el.className='show'+(err?' err':''); clearTimeout(toastT); toastT=setTimeout(()=>el.className='',3000);
}
</script>
</body>
</html>
)===HTMLBLOCK===";

void handlePanel() { server.send_P(200, "text/html", PANEL_HTML); }

// ============================================================
//  SETUP
// ============================================================
void setup() {
  Serial.begin(115200);

  if (!LittleFS.begin()) {
    Serial.println("LittleFS mount failed — reformatting...");
    LittleFS.format();
    LittleFS.begin();
  }
  loadConfig();

  SPI.begin();
  mfrc522.PCD_Init();
  pinMode(trigPin,   OUTPUT);
  pinMode(echoPin,   INPUT);
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);

  beep(1);

  Serial.print("Connecting to WiFi: ");
  Serial.println(cfg.ssid);
  WiFi.begin(cfg.ssid.c_str(), cfg.wifiPass.c_str());
  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries < 40) {
    delay(500); Serial.print("."); tries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected. IP: " + WiFi.localIP().toString());
    beep(2);
  } else {
    Serial.println("\nWiFi failed.");
  }

  server.on("/",                 HTTP_GET,  handlePanel);
  server.on("/panel",            HTTP_GET,  handlePanel);
  server.on("/api/status",       HTTP_GET,  handleStatus);
  server.on("/api/arm",          HTTP_POST, handleArm);
  server.on("/api/cards",        HTTP_GET,  handleGetCards);
  server.on("/api/cards/add",    HTTP_POST, handleAddCard);
  server.on("/api/cards/remove", HTTP_POST, handleRemoveCard);
  server.on("/api/scan",         HTTP_POST, handleScanRequest);
  server.on("/api/sensor",       HTTP_POST, handleSaveSensor);
  server.on("/api/tune",         HTTP_POST, handleSaveTune);
  server.on("/api/tune/test",    HTTP_POST, handleTestTune);
  server.on("/api/tunes",        HTTP_GET,  handleGetTunes);
  server.on("/api/messages",     HTTP_POST, handleSaveMessages);
  server.on("/api/access",       HTTP_POST, handleSaveAccess);
  server.on("/api/config",       HTTP_GET,  handleGetConfig);
  server.on("/api/buzz/test",    HTTP_POST, handleTestBuzz);
  server.on("/api/webalert/test",HTTP_POST, handleWebAlertTest);

  server.collectHeaders("X-Panel-Pass");
  server.begin();
  Serial.println("HTTP server started.");
}

// ============================================================
//  LOOP
// ============================================================
void loop() {
  server.handleClient();
  checkRFID();

  if (reqTestBuzz)   { reqTestBuzz   = false; beep(3); }
  if (reqTestTune)   { reqTestTune   = false; playTune(cfg.tuneIndex, 1); }
  if (reqArmBeep)    { reqArmBeep    = false; beep(1); }
  if (reqDisarmBeep) { reqDisarmBeep = false; beep(2); }

  if (systemArmed && (millis() - lastScan >= (unsigned long)cfg.scanInterval)) {
    lastScan = millis();

    digitalWrite(trigPin, LOW); delayMicroseconds(2);
    digitalWrite(trigPin, HIGH); delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    long duration = pulseIn(echoPin, HIGH, 15000);
    int  distance = duration > 0 ? (int)(duration * 0.034 / 2) : 999;

    if (distance > 0 && distance < cfg.threshold) {
      Serial.println("INTRUDER DETECTED — " + String(distance) + " cm");

      if (cfg.notifyDetect && (!messageSent || !cfg.cooldown)) {
        if (cfg.buzzOnDetect) tone(buzzerPin, 3000);
        // Intrusion → full alert modal (red screen + siren)
        triggerWebAlert(applyTemplate(cfg.msgAlert, "", String(distance)));
        messageSent = true;
        stopBuzzer();
      }

      if (cfg.buzzOnDetect) {
        stopBuzzer();
        playTune(cfg.tuneIndex, cfg.tuneRepeat);
        stopBuzzer();
      }

    } else {
      if (messageSent) {
        messageSent = false;
        stopBuzzer();
      }
    }
  }
}
