//Lab 7 Starter
// C++ code
//Note: Upon downloading, this code does nothing except run without errors at a period of 500ms
//      It is up to you to figure out how to manipulate these timing details for your own purposes
#include "Timer.h"
#include "Music.h"
#include <LiquidCrystal.h>

typedef struct task {
  int state;
  unsigned long period;
  unsigned long elapsedTime;
  int (*TickFct)(int);
} task;

const unsigned short taskNum = 2;
task tasks[taskNum];

const unsigned long SP_PERIOD = 62.5;
const unsigned long JS_PERIOD = 750;
const unsigned long tasksPeriodGCD = 62.5;

void TimerISR() {
  for (int i = 0; i < taskNum; i++) {
    if(tasks[i].elapsedTime >= tasks[i].period) {
      tasks[i].state = tasks[i].TickFct(tasks[i].state);
      tasks[i].elapsedTime = 0;
    }
    tasks[i].elapsedTime += tasksPeriodGCD;
  }
}

enum SP_STATES {SP_START = 0, PLAY = 1, PAUSE = 2};
enum JS_STATES {JS_START, STAY, BUTTON_PRESSED, CHANGE_FAN, CHANGE_MUSIC} ;

const char* const joystick_str[] = {
  [JS_START] = "JS_START",
  [STAY] = "STAY",
  [BUTTON_PRESSED] = "BUTTON_PRESSED",
  [CHANGE_FAN] = "CHANGE_FAN",
  [CHANGE_MUSIC] = "CHANGE_MUSIC",
};

const uint8_t block_char[] = {
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF
};
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
const int joy_x = A1, joy_y = A0, button = 6;
const int buzzer_pin = 7, fan_pin = 9;
const char speeds[] = {0, 75, 125, 175, 225, 255};
//const char speeds[] = {0, 60, 68, 75, 83, 90};
const int max_speed = 5;
const char B[] = {2, 3, 4, 5};
const char base_period = 4;

bool isPlaying = false, nextSong, prevSong;
int song, note, duration;
int fan_speed;
int x = 0, y = 0;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


int TickFct_Speaker(int state) {
//  Serial.print("SPEAKER STATE = ");
//  Serial.println(state);
//  Serial.print("song = ");
//  Serial.print(song);
//  Serial.print(", note = ");
//  Serial.print(note);
//  Serial.print(", duration = ");
//  Serial.print(duration);
//  Serial.print(", isPlaying = ");
//  Serial.println(isPlaying);
  switch(state) { //transitions
    case SP_START:
      isPlaying = false;
      song = 0;
      note = 0;
      duration = 1000 / songs[song].durations[note];
      state = PAUSE;
      break;
    case PLAY:
      if(!isPlaying) {
        state = PAUSE;
      } else {
//        if(duration < 0) {
//          noTone(buzzer_pin);
//        }
//        if(duration < (-.03 * 1000) / songs[song].durations[note]) {
//          note++;
//          if(note > songs[song].length) {
//            nextSong = true;
//          }
//          duration = 1000 / songs[song].durations[note];
//        }
        note++;
      }
      break;
    case PAUSE:
      if(isPlaying) {
        state = PLAY;
      }
      break;
    default:
      break;
  }

  switch(state) { //actions
    case SP_START:
      break;
    case PLAY:
//      if( duration == 1000 / songs[song].durations[note]) {
        tone(buzzer_pin, got_melody[note], 1000 / got_durations[note]);
//      }
//      duration -= SP_PERIOD;
      lcd.setCursor(0, 0);
      lcd.print(songs[song].title);
      break;
    case PAUSE:
      lcd.setCursor(0, 0);
      lcd.print("MUSIC PAUSED    ");
      noTone(buzzer_pin);
      break;
    default:
      break;
  }
  return state;
}

int TickFct_Joystick(int state) {
  Serial.print("state = ");
  Serial.println(joystick_str[state]);
//  Serial.print("x = ");
//  Serial.print(analogRead(joy_x));
//  Serial.print(", y = ");
//  Serial.print(analogRead(joy_y));
//  Serial.print(", fan speed = ");
//  Serial.print(fan_speed);
//  Serial.print(", button pressed = ");
//  Serial.println(digitalRead(button));
  switch (state) { // transitions
    case JS_START:
      Serial.println("IN JS_START");
      fan_speed = 0;
      analogWrite(fan_pin, speeds[fan_speed]);
      state = STAY;
      lcd.print("NOW PLAYING: ");
      lcd.setCursor(0, 1);
      lcd.print("FAN OFF");
      break;
    case STAY:
      Serial.println("IN STAY");
      int buttonPressed = digitalRead(button) ? 0 : 1;
//      Serial.println(buttonPressed);
      if((y >= 758 || y <=256) && !buttonPressed) {
        state = CHANGE_FAN;
      } else if ((x >= 758 || x<=256) && !buttonPressed) {
        state = CHANGE_MUSIC;
      } else if (buttonPressed) {
        state = BUTTON_PRESSED;
      }
      break;
    case CHANGE_FAN:
      Serial.println("IN CHANGE_FANE");
      if(y < 758 && y > 256) {
        Serial.println("STATE IS STAY");
        state = STAY;
      }
      break;
    case CHANGE_MUSIC:
      Serial.println("IN CHANGE_MUSIC");
      state = STAY;
      break;
    case BUTTON_PRESSED:
      Serial.println("IN BUTTON_PRESSED");
      state = STAY;
      break;
    default:
    Serial.println("IN DEFAULT");
      break;
  }
  Serial.print("SECOND state = ");
  Serial.println(joystick_str[state]);
  switch (state) { // actions
    case JS_START:
      break;
    case STAY:
      x = analogRead(joy_x);
      y = analogRead(joy_y);
      break;
    case CHANGE_FAN:
      Serial.println("CHANGING FAN");
      if(y <= 256) {
        if (fan_speed > 0) {
          fan_speed--;
        }
      }
      if(y >= 758) {
        if (fan_speed < max_speed) {
          fan_speed++;
        }
      }
      analogWrite(fan_pin, speeds[fan_speed]);
      lcd.setCursor(0, 1);
      
      String speed_string = "FAN SPEED:";
      if (fan_speed > 0) {
        lcd.print(speed_string);
//        lcd.print(fan_speed);
        for(int i = 0; i < fan_speed; i++) {
          lcd.print((char)0);
        }
        for(int i = 0; i < 5 - fan_speed; i++) { // clear end spaces
          lcd.print(' ');
        }
      } else {
        speed_string = "FAN OFF         ";
        lcd.print(speed_string);
      }
      x = analogRead(joy_x);
      y = analogRead(joy_y);
      break;
    case BUTTON_PRESSED:
      isPlaying = !isPlaying;
      break;
    case CHANGE_MUSIC:
      if(x <= 256) {
        prevSong = true;
      }
      if(x >= 758) {
        nextSong = true;
      }
      break;
    default:
      break;
  }
  return state;
}

void setup() {
//  pinMode(d4, INPUT);
//  pinMode(d5, INPUT);
//  pinMode(d6, INPUT);
//  pinMode(d7, INPUT);
//  pinMode(en, OUTPUT);
//  pinMode(rs, OUTPUT);
//  pinMode(13, OUTPUT);
  pinMode(button, INPUT_PULLUP);
  pinMode(fan_pin, OUTPUT);
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.createChar(0, block_char);
  lcd.clear();

  int i = 0;
  tasks[i].state = SP_START;
  tasks[i].period = SP_PERIOD;
  tasks[i].elapsedTime = tasks[i].period;
  tasks[i].TickFct = &TickFct_Speaker;
  i++;
  tasks[i].state = JS_START;
  tasks[i].period = JS_PERIOD;
  tasks[i].elapsedTime = tasks[i].period;
  tasks[i].TickFct = &TickFct_Joystick;

  i = 0;
  songs[i].title = "GOT MAIN THEME  ";
  songs[0].melody = got_melody;
  songs[0].durations = got_durations;
  songs[0].length = sizeof(songs[0].melody);
  
  TimerSet(tasksPeriodGCD); //this value (500) is the period in ms
  TimerOn();
}

void loop()
{
    while(!TimerFlag){}
    TimerFlag = 0;  
}
