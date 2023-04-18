#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// SDA - A4
// SCL - A5

const int WIDTH = 20;
const int SERIAL_BAUD = 9600;
const long SERIAL_TIMEOUT = 2147483647;

const bool DEBUG = false;

const bool PRINT_SCORE_TO_SERIAL = true;

const int NUM_PHASES = 6;
const String PHASE[NUM_PHASES] = {
  "Draw Phase",
  "Standby Phase",
  "Main Phase 1",
  "Battle Phase",
  "Main Phase 2",
  "End Phase"
};

// these should be in a separate file but i'm too lazy, so maybe later
byte TROPHY[] = {
  B00000,
  B11111,
  B11111,
  B11111,
  B01110,
  B00100,
  B01110,
  B00000
};

byte PLAYING[] = {
  B11111,
  B10001,
  B10001,
  B10101,
  B10101,
  B10001,
  B10001,
  B11111
};


String player[2];
long lp[2];
short wins[2];
short current_turn, current_phase, current_player;
//short starting_player;
int starting_lp = 8000;

// for some reason it stopped seeing this function out of nowhere, so i'm declaring it now
void draw(int row = -1);

LiquidCrystal_I2C lcd(0x27, WIDTH, 4);

String inputName(String name) {
  Serial.print(F("Please enter "));
  Serial.print(name);
  Serial.println(F("'s name (up to 8 characters!). "));
  String text = Serial.readStringUntil('\n');
  text.trim();
  if (text.length() > 8) {
    text = text.substring(0, 8);
  }
  if (text.length() == 0){
    text = name;
  }
  return text;
}

short askForFirstPlayer(){
  while (true) {
      Serial.println(F("Which player should go first (1/2)? "));
      String text = Serial.readStringUntil('\n');
      text.trim();
      short starting_player = text.toInt();
      if (starting_player == 1 || starting_player == 2 ) {
        return starting_player - 1;
      }
  }
}

void getValues() {
  lcd.clear();
  
  printCenter(F("Setting up..."), 1);

  if (DEBUG) {
    Serial.println(F("Debug mode on - using default values."));
  } else {
    // could be a for loop but this is good enough and faster
    player[0] = inputName(F("Player 1"));
    player[1] = inputName(F("Player 2"));

    Serial.println(F("Please enter starting LP (default = 8000). "));
    String text = Serial.readStringUntil('\n');
    text.trim();
    starting_lp = text.toInt();
    if (starting_lp <= 0) starting_lp = 8000;

    current_player = askForFirstPlayer();
  }
  current_turn = 1;
  current_phase = 0;
  lp[0] = starting_lp;
  lp[1] = starting_lp;
  wins[0] = 0;
  wins[1] = 0;

}

void setup() {
  lcd.init();
  lcd.backlight();
  lcd.createChar(0, TROPHY);
  lcd.createChar(1, PLAYING);
  randomSeed(analogRead(0));
  printCenter(F("Yu-Gi-Oh! Score"), 0);
  printCenter(F("Tracker by Fishie"), 1);
  printCenter(F("Initializing serial"), 3);
  Serial.begin(SERIAL_BAUD);
  Serial.setTimeout(SERIAL_TIMEOUT);
  delay(1000);
  Serial.println();
  Serial.println(F("Yu-Gi-Oh! Score Tracker by Fishie"));

  getValues();
  draw();
  
  Serial.println(F("The game has started. Type 'h' for a list of commands."));
}

void printLeft(String text, int row) {
  lcd.setCursor(0, row);
  lcd.print(text);
}

void printCenter(String text, int row) {
  lcd.setCursor((WIDTH / 2) - (text.length() / 2), row);
  lcd.print(text);
}

void printRight(String text, int row) {
  lcd.setCursor(WIDTH - text.length(), row);
  lcd.print(text);
}

void printLeft(char text, int row) {
  lcd.setCursor(0, row);
  lcd.print(text);
}

void printRight(char text, int row) {
  lcd.setCursor(WIDTH - 1, row);
  lcd.print(text);
}

void clearRow(int row){
  if(row < 0){
    lcd.clear();
  }else{
    lcd.setCursor(0, row);
    for(int i = 0; i < WIDTH; i++){
      lcd.print(' ');
    }
  }
}

void coinToss() {
  lcd.clear();
  printCenter(F("Tossing a coin"), 1);
  printCenter(F(".  "), 2);
  delay(1000);
  printCenter(F(".. "), 2);
  delay(1000);
  printCenter(F("..."), 2);
  delay(1000);
  printCenter(random(0, 2) == 0 ? F("Heads") : F("Tails"), 2);
  delay(2000);
}

void diceRoll() {
  lcd.clear();
  printCenter(F("Rolling a dice"), 1);
  printCenter(F(".  "), 2);
  delay(1000);
  printCenter(F(".. "), 2);
  delay(1000);
  printCenter(F("..."), 2);
  delay(1000);
  printCenter("[" + String(random(1, 7)) + "]", 2);
  delay(2000);
}

void win(char winner) {
  for(int i = 1; i <= 5; i++){
    lcd.noBacklight();
    delay(100);
    lcd.backlight();
    delay(100 * i);
  }

  lcd.clear();
  switch (winner) {
    case '1':
      printCenter(player[0], 1);
      printCenter(F("wins the duel!"), 2);
      wins[0]++;
      break;

    case '2':
      printCenter(player[1], 1);
      printCenter(F("wins the duel!"), 2);
      wins[1]++;
      break;

    case 'a':
      printCenter(F("It's a draw!"), 1);
      break;
  }
  Serial.println(F("A win has occured, press <Enter> to continue to the next duel."));
  String command = Serial.readStringUntil('\n');

  current_turn = 1;
  current_phase = 0;
  current_player = askForFirstPlayer();
  lp[0] = starting_lp;
  lp[1] = starting_lp;

  draw();
}

void draw(int row) {
  clearRow(row);
  switch(row){
    case -1:
    case 0:
      printLeft(player[0], 0);
      printRight(player[1], 0);
      if(row != -1) break;
    case 1:
      printLeft(String(lp[0]), 1);
      printRight(String(lp[1]), 1);
      printCenter(F("vs"), 1);
      if(row != -1) break;
    case 2:
      printCenter("Turn " + String(current_turn), 2);
      switch (current_player) {
        case 0:
          lcd.setCursor(0, 2);
          break;
        case 1:
          lcd.setCursor(WIDTH - 1, 2);
          break;
      }
      lcd.write(1);
      if(row != -1) break;
    case 3:
      printCenter(PHASE[current_phase], 3);

      if (wins[0] > 3 || wins[1] > 3) {
        printLeft(String(wins[0]), 3);
        printRight(String(wins[1]), 3);
      } else {
        for (int i = 0; i < wins[0]; i++) {
          lcd.setCursor(i, 3);
          lcd.write(0);
        }
        for (int i = 0; i < wins[1]; i++) {
          lcd.setCursor(WIDTH - 1 - i, 3);
          lcd.write(0);
        }
      }
      if(row != -1) break;
  }
}

void loop() {
  
  if(PRINT_SCORE_TO_SERIAL){
    Serial.println();
    for(int i=0; i<2; i++){
      Serial.print(F("Player "));
      Serial.print(i+1);
      Serial.print(F(": "));
      Serial.println(player[i]);
      
      Serial.print(F("LP: "));
      Serial.println(lp[i]);
      
      Serial.print(F("Wins: "));
      Serial.println(wins[i]);

      Serial.println();
    }

    Serial.print(F("Turn "));
    Serial.print(current_turn);
    Serial.print(F(" - "));
    Serial.println(player[current_player]);
    Serial.println(PHASE[current_phase]);
    
  }
  
  Serial.println();
  Serial.println(F("Waiting for a command..."));
  String command = Serial.readStringUntil('\n');
  command.trim();
  command.toLowerCase();
  if (command == "") {
    command = "a";
  }

  // temporary variables, declared here in order to avoid initialization crossing
  long lp_change;
  String lp_str;
  bool set;

  switch (command[0]) {
    // advance a phase
    case 'a':
      current_phase++;
      while (current_phase >= NUM_PHASES) {
        current_phase -= NUM_PHASES;
        current_player = (current_player + 1) % 2;
        current_turn++;
      }
      Serial.println(F("[OK] Advanced a phase."));
      draw(2);
      draw(3);
      break;

    // go back a phase
    case 'b':
      current_phase--;
      while (current_phase < 0) {
        current_phase += NUM_PHASES;
        current_player = (current_player + 1) % 2;
        current_turn--;
      }
      if (current_turn < 1) {
        current_turn = 1;
        current_phase = 0;
        //current_player = askForFirstPlayer();
        Serial.println(F("[ERROR] Can't go back any more phases!"));
      } else {
        Serial.println(F("[OK] Went back a phase."));
        draw(2);
        draw(3);
      }
      break;

    // toss a coin
    case 'c':
      Serial.println(F("Tossing a coin..."));
      coinToss();
      Serial.println(F("[OK] Tossed a coin."));
      draw();
      break;

    // roll a dice
    case 'd':
      Serial.println(F("Rolling a dice..."));
      diceRoll();
      Serial.println(F("[OK] Rolled a dice."));
      draw();
      break;

    // reset
    case 'r':
      getValues();
      draw();
      break;

    // switch turns
    case 's':
      current_player = (current_player + 1) % 2;
      break;

    // print help
    case 'h':
      Serial.println(F("List of all available commands: "));
      Serial.println(F("pX-N     subtract N LP from player X (use 'a' for all players)"));
      Serial.println(F("pX+N     add N LP to player X        (use 'a' for all players)"));
      Serial.println(F("pXsN     set player X's LP to N      (use 'a' for all players)"));
      Serial.println(F("pXw      make player X win the game  (use 'a' for all players)"));
      Serial.println(F("<Enter>  advance phase"));
      Serial.println(F("b        go back a phase"));
      Serial.println(F("c        toss a coin"));
      Serial.println(F("d        roll a dice"));
      Serial.println(F("r        reset"));
      Serial.println(F("s        switch turns"));
      Serial.println(F("h        print this menu"));
      break;

    // manipulate LP
    case 'p':
      {
        if (command[2] == 'w') {
          win(command[1]);
        }

        set = command[2] == 's';
        if (set) {
          lp_str = command.substring(3);
        } else {
          lp_str = command.substring(2);
        }
        lp_change = lp_str.toInt();

        switch (command[1]) {
          case '1':
            if (set) {
              lp[0] = lp_change;
            } else {
              lp[0] += lp_change;
            }
            break;

          case '2':
            if (set) {
              lp[1] = lp_change;
            } else {
              lp[1] += lp_change;
            }
            break;

          case 'a':
            if (set) {
              lp[0] = lp_change;
              lp[1] = lp_change;
            } else {
              lp[0] += lp_change;
              lp[1] += lp_change;
            }
            break;
        }
        
        if (lp[0] <= 0 && lp[1] <= 0) {
          lp[0] = 0;
          lp[1] = 0;
          draw();
          win('a');
        } else {
          if (lp[0] <= 0) {
            lp[0] = 0;
            draw();
            win('2');
          }
          if (lp[1] <= 0) {
            lp[1] = 0;
            draw();
            win('1');
          }
        }

        Serial.println(F("[OK] LP updated."));
        draw(1);
        break;
      }

    default:
      Serial.println(F("[ERROR] Invalid command."));
      break;
  }

  Serial.println(F("----------------------------------"));
  Serial.println();

}
