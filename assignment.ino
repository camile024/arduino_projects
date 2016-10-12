#include <AberLED.h>
#include <EEPROM.h>

//#define DEBUG

//defining states
#define S_INVALID   -1
#define S_INIT      0
#define S_INIT2     1
#define S_START     2
#define S_GAME      3
#define S_PAUSE     4
#define S_GAMEOVER  13


//defining buttons
#define BTN_UP    1
#define BTN_DOWN  2
#define BTN_LEFT  4
#define BTN_RIGHT 3
#define BTN_STOP  5

//define entities
#define ENT_OBSTACLE  1
#define ENT_POWERUP   2


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Constants (changeable here)://
////////////////////////////////
const byte MINX = 0;
const byte MINY = 0;
const byte MAXX = 7;
const byte MAXY = 7;
const int SPEED_CHANGE_TIME = 5;
const int OBSTACLE_CHANGE_TIME = 5;

const byte LETTER_START [] =  {2,1, 2,2, 5,1, 5,2,    1,4, 1,5, 2,6, 3,6, 4,6, 5,6, 6,5, 6,4 };                                 // x,y, x,y, x,y...
const byte LETTER_0 [] = { 0,0, 0,1, 0,2, 0,3, 0,4,   1,0, 1,4,   2,0, 2,1, 2,2, 2,3, 2,4 };
const byte LETTER_1 [] = { 1,0, 1,1, 1,2, 1,3, 1,4 };
const byte LETTER_2 [] = { 1,0, 1,2, 1,3, 1,4,   2,0, 2,1, 2,2, 2,4 };
const byte LETTER_3 [] = { 1,0, 1,2, 1,4,   2,0, 2,1, 2,2, 2,3, 2,4 };
const byte LETTER_4 [] = { 1,0, 1,1, 1,2,   2,2, 2,3, 2,4 };
const byte LETTER_5 [] = { 1,0, 1,1, 1,2, 1,4,   2,0, 2,2, 2,3, 2,4 };
const byte LETTER_6 [] = { 0,0, 0,1, 0,2, 0,3, 0,4,   1,0, 1,2, 1,4,    2,0, 2,2, 2,3, 2,4 };
const byte LETTER_7 [] = { 1,0,   2,0, 2,1, 2,2, 2,3, 2,4 };
const byte LETTER_8 [] = { 0,0, 0,1, 0,2, 0,3, 0,4,   1,0, 1,2, 1,4,   2,0, 2,1, 2,2, 2,3, 2,4 };
const byte LETTER_9 [] = { 0,0, 0,1, 0,2, 0,4,   1,0, 1,2, 1,4,   2,0, 2,1, 2,2, 2,3, 2,4 };
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                                          /////////////////////////////////////////////////////
                                                                          //                                                 //
                                                                          //  MODIFY INITIAL SETTINGS IN initGame()          //
                                                                          // ( CAN'T HAVE THEM HERE TO NOT MESS UP GAME )    //
                                                                          //           ( OVER BEHAVIOUR OR DUPLICATE )       //
                                                                          //           ( THE CODE )                          //
                                                                          //                                                 //
                                                                          /////////////////////////////////////////////////////

int g_speed_modif;
int g_speed;
int g_speedLevel;
int g_prePauseSpeed;

bool g_inclevel;
bool g_player_pause_blink = false;

byte g_state = S_INVALID;
byte g_entities[MAXX+1][MAXY+1];
byte g_player;
byte g_std_speed;

long g_score;

unsigned long g_time;
unsigned long g_loopTime;
unsigned long g_ticks;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////
//Function definitions://
/////////////////////////


unsigned long getLoopTime();
unsigned long getStateTime();
void addTicks(unsigned long n);
void gotoState(int state);
void doRender();
void drawScore ( int score);
void lightPattern(const byte letter[],  size_t letter_length, byte x, byte y, char color);
void drawChar(char c, byte x, byte y, char color);
void eepromScore();
void movePlayer(int i);
void controlSpeed();
void doHandleInput();
void doHitObstacle();
void doHitPowerup();
void doGenerateLine();
void moveEntity(byte x, byte y);
void doMove();
void initGame();
void doUpdate();

//////////////////////SETUP FUNCTION//////////////////////////////////////////////

void setup() {
  AberLED.begin(); 
  AberLED.clear(); 
  Serial.begin(9600);
  for(int i= MINX ; i <= MAXX ; i++){             //see if all LEDs are working properly
     for(int j= MINY ; j <= MAXY ; j++){
       AberLED.set (i ,j ,YELLOW) ;
     }
  }
  AberLED.swap();
  delay(1000);
  AberLED.clear();                        //clear the display after the test
  gotoState(S_INIT);
  initGame();                             //initialise the game
}

//////////////////////LOOP FUNCTION//////////////////////////////////////////////

void loop() {
  #ifdef DEBUG
    Serial.println("Main loop stats:\n \t speed: " + String(g_speed) + " | player: " + g_player + " | state: " + g_state + " | state time: " + String(getStateTime()) + " | score: " + g_score);
  #endif 
    doHandleInput();                                              // handles input, moves player/(un)pauses game                                 
    addTicks(getLoopTime());
    g_loopTime = millis();                                      

  if (getStateTime() > g_speed){                                 //updates game timer and the board by moving everything to the left (if S_GAME)
    gotoState(g_state);
    doUpdate();
  }
    doRender();                                                 //draws everything and swaps the buffers
}

//////////////////////TIMER/STATE FUNCTIONS//////////////////////////////////////////////

unsigned long getLoopTime(){          //gets time between main loop occurences
  return (millis() - g_loopTime);
}



unsigned long getStateTime(){         //gets game time between ticks
  return (millis() - g_time);
}

void addTicks(unsigned long n){       //sets total time since start of the game (excluding pause state)
  if (g_state == S_GAME){
    g_ticks += n;
  }
}

void gotoState(int state){            //changes game state
  g_state = state;
  g_time = millis();
}

//////////////////////INITGAME FUNCTION//////////////////////////////////////////////

void initGame(){
  g_player = 0;      
  g_speed = 250;            //how often the game moves in ms
  g_speed_modif = 50;       //speed modifier in ms, just in case somebody would want to edit it
  g_time = millis();        //last 'tick' time
  g_score = 0;              //reset the score
  g_loopTime = millis();    //last player move 'tick' time
  gotoState(S_START);       //change state from invalid to "START"
  g_std_speed = g_speed;    //store initial speed (used in collisions/gameover)
  g_player_pause_blink = false; //for blinking animation when the game is paused
  g_ticks = 0;               //resets the main timer
  g_speedLevel = 0;
  for (byte x = MINX; x <= MAXX; x++){    //zeroes the array containing entities
    for (byte y = MINY; y <= MAXY; y++){
      g_entities[x][y] = 0;
     } 
   }
 #ifdef DEBUG
  Serial.println("--GAME INITIALISED--");
 #endif 
}


//////////////////////DOHANDLEINPUT FUNCTION//////////////////////////////////////////////

void doHandleInput(){                    //gets the button pressed and moves player/(un)pauses game accordingly
  bool pressed = false;
  byte addr = 0;
  String temp_score;
  switch (g_state){
    
    case S_PAUSE:
    case S_GAME:
      for (int i = 1; i <= 5; i++){
        if (AberLED.getButtonDown(i)){
          switch(i){
            case BTN_UP :
              movePlayer(-1);
              pressed = true;
              break;
            case BTN_DOWN :
              movePlayer(1);
              pressed = true;
              break;
            case BTN_STOP :
              if (g_state == S_PAUSE){
                gotoState(S_GAME);
                g_speed = g_prePauseSpeed;
              } else {                                          //Game paused/resumed using BTN_STOP or resumed if any other button is pressed
                gotoState(S_PAUSE);                             //and go back to old speed when I resume the game
                g_prePauseSpeed = g_speed;
                g_speed = 600;
              }
              break;
            default:
              break;
          }
        }
      }
      if (pressed && g_state == S_PAUSE){
        gotoState(S_GAME);
        g_speed = g_prePauseSpeed;
      }
      break;
      
    case S_GAMEOVER:                                            //I made game save the score to EEPROM when BTN_RIGHT was pressed in GAMEOVER state (if debug was defined) to put a first
      if (AberLED.getButtonDown(BTN_STOP)){                     //value there and for testing purposes, before I implemented it to happen automatically
        gotoState(S_INIT2);  
        Serial.println("TOTAL SCORE: " + String(g_score+1));                                                  
      } else if (AberLED.getButtonDown(BTN_RIGHT)){
     #ifdef DEBUG
        addr = 0;
        temp_score = String(g_score);
        while (temp_score.length() < 4){                        
          temp_score = '0' + temp_score;
        }
        for (int i = 0; i < temp_score.length(); i ++){                     
          EEPROM.write(addr, (temp_score[i]));                            
          Serial.println("--EEPROM WRITE: " + String(temp_score[i]) + " AT " + String(addr));
          addr++;
        }
      #endif 
      }
      break;
      
    case S_INIT2:                                             //restarts the game and all its values when button 5 is pressed at the "top score" stage
      if (AberLED.getButtonDown(BTN_STOP)){
        gotoState(S_INIT);
        initGame();
      }
      break;
      
    case S_START:                                            //removes the welcome screen and proceeds to the game when button 5 is pressed
      if (AberLED.getButtonDown(BTN_STOP)){
        gotoState(S_GAME);
      }
      break;
   default:
      Serial.println("[001] WARNING! INVALID GAME STATE DETECTED: "+ String(g_state) + " !");
      break;
  }
}

//////////////////////DOUPTDATE FUNCTION//////////////////////////////////////////////

void doUpdate(){
  switch (g_state){
    
      break;
    case S_INIT:
      initGame();                       //shouldn't happen since void setup() initialises the game, but...
      break;
    case S_INIT2:
      eepromScore();
      break;
    case S_PAUSE:
      if (g_player_pause_blink){
        g_player_pause_blink = false;                                         
      } else {
        g_player_pause_blink = true;
      }
    case S_GAMEOVER:
    case S_START:                       //doRender() handles gameover and start screens since its just static image
      break; 
    case S_GAME: 
      doMove();
      doGenerateLine();
      controlSpeed();
      g_score ++;
      break;    
    default:
      Serial.println("[001] WARNING! INVALID GAME STATE DETECTED: "+ String(g_state) + " !");
      break;   
  }
}

//////////////////////RENDERING FUNCTION//////////////////////////////////////////////

void doRender(){                      //draws stuff
  String highscore = "";
  char character;
  byte addr = 0;
  String temp_score = "";
  AberLED.clear();
  switch (g_state){

    
    case S_PAUSE :                                                                  //S_PAUSE cascades to S_GAME so I don't have to repeat the code and can use if statement for differences
    case S_GAME :
      
      for (byte x = MINX; x <= MAXX; x++){                                          //draw powerups/obstacles
        for (byte y = MINY; y <= MAXY; y++){
          if (g_entities[x][y] == ENT_POWERUP){ 
            AberLED.set(x,y, YELLOW); 
          } else if ( g_entities[x][y] == ENT_OBSTACLE){ 
            AberLED.set(x,y, RED);
          }
        }
      }
      if (g_state == S_PAUSE){
          if (g_player_pause_blink){                                                //draw the player, make it blinking if the game's paused
            AberLED.set(0, g_player, GREEN);
          } 
      } else {
        AberLED.set(0, g_player, GREEN);
      }
      break;
      
    case S_GAMEOVER :                                                               //drawScore if the game is over
      drawScore(g_score);
      break;

      
    case S_INIT2 :                                                                  //S_INIT2 is the state between S_GAMEOVER and S_START and shows the highscore from EEPROM
      drawScore(g_score);
      break;
      
    
    case S_START :                                                                                      //Draws the smiley face as the start screen
      lightPattern(LETTER_START, sizeof(LETTER_START)  / sizeof(LETTER_START[0]), MINX, MINY, RED);     //I explained how I learned to measure length of an array at the lightPattern() function itself
      break;

    default:
      Serial.println("[001] WARNING! INVALID GAME STATE DETECTED: "+ String(g_state) + " !");
      break;
  }
  AberLED.swap();
}

//////////////////////SPEED CHANGING OVBER TIME FUNCTION//////////////////////////////////////////////

void controlSpeed(){
      if (g_speedLevel < g_ticks / (SPEED_CHANGE_TIME * 1000)){         //we change speed every SPEED_CHANGE_TIME seconds
        #ifdef DEBUG
          Serial.println(".ControlSpeed(): speedlevel: " + String(g_speedLevel) + " | time / 20000: " + String(g_ticks / 20000) + " | speed: " + String(g_speed));
        #endif 
        g_speedLevel = g_ticks / (SPEED_CHANGE_TIME * 1000);
        g_speed -= g_speed_modif / 2;                                   //same value as powerup but makes it go faster
      }
}


//////////////////////LINE GENERATING FUNCTION//////////////////////////////////////////////

void doGenerateLine(){                              //generates a new line with obstacles/powerups
  byte chancePower = 25;                            //set chance for getting a powerup x/100 
  byte chanceAny = 10;                              //set chance for getting anything x/100         
  chanceAny += g_ticks / (OBSTACLE_CHANGE_TIME * 1000);   //increases chance of getting anything by 1 every OBSTACLE_MODIFIER_TIME seconds (increasing chance of getting an obstacle at the same time)
  for (int i = MINY; i <= MAXY; i++){
    byte generator = random(100);                            //two random generators, one for an entity, second one for entity type (obstacle/powerup)
    byte generator2 = random(100);                            
    if (generator <= chanceAny){                              //if we get anything 
      if (generator2 <= chancePower){                         //check if it's a powerup
        g_entities[MAXX][i] = ENT_POWERUP;
      } else {                                              //if not, place an obstacle
        g_entities[MAXX][i] = ENT_OBSTACLE;
      }
    }
  } 
}

//////////////////////MOVEPLAYER FUNCTION//////////////////////////////////////////////

void movePlayer(int i){                    //change coordinates depending on 'i' (which is how many units we go down/up)
  bool pMoved = false;
  if (i > 0 && g_player < MAXY ){          //moving up, considering upper border
    g_player += i;
    pMoved = true;
  } else if ( i < 0 && g_player > MINY){  //moving down, considering bottom border
    g_player += i;
    pMoved = true;
  }
    if (pMoved){                          //if we actually moved, check if there was also an obstacle/powerup (since we can move faster than obstacles/PUps)
      if (g_entities[MINX][g_player] == ENT_OBSTACLE){
        doHitObstacle();
      } else if (g_entities[MINX][g_player] == ENT_POWERUP){
        doHitPowerup();
      }
    }
}

//////////////////////DOMOVE FUNCTION//////////////////////////////////////////////


void doMove(){                                //moves entities on the board (except player)
  for (byte x = MINX; x <= MAXX; x++){
    for (byte y = MINY; y <= MAXY; y++){
      moveEntity(x, y);
    }
  }
}


//////////////////////MOVE ENTITY FUNCTION//////////////////////////////////////////////

void moveEntity(byte x, byte y){         //moves an obstacle/PUp left each tick (this way of implementation allows us to add different kinds
  byte ent_type = g_entities[x][y];      // of entities as we go. I used to have two separate functions for PUps and obstacles but merged them into this one)
  switch (g_entities[x][y]){                                                   
    case 0 : break;                      //if there's no ent at given coordinates, skip                
    case ENT_POWERUP:
    case ENT_OBSTACLE:                  
      if ( x == 0 ){                                   
        
      } else if ( x == MINX+1 && g_player == y ){       //if there's an ent at the coordinates we're checking do the following
        switch (ent_type){
          case ENT_OBSTACLE:
            doHitObstacle();                            //if ent "bumps" on player, do the hitting function
            break;
          case ENT_POWERUP:
            doHitPowerup();
            break;
          default:
            break;
        }
      } else {
        g_entities[x-1][y] = g_entities[x][y];                                //if ent wasn't removed due to collision or going out of bounds, make a new ent on the left
      }
        g_entities[x][y] = 0;                                                 //remove old ent
      break;
    default:
      Serial.println("[003] WARNING! WRONG ENT STATE AT (" + String(x) + " , " + String(y) + ") !");
      break;
    }
}

//////////////////////HITTING/COLLISION FUNCTIONS//////////////////////////////////////////////

void doHitObstacle(){               //hits obstacle, self-explanatory
  if (g_speed > 0){                 // can't go below 0
    g_speed -= g_speed_modif;       // by default -50
  }
  if (g_speed <= 0){                //if last reduction made us go to 0 or below 0, change game state to Game Over
    gotoState(S_GAMEOVER);
    g_speed = 0;
    Serial.println("TOTAL SCORE: " + String(g_score+1));
  }
}



void doHitPowerup(){                //hits powerup
  if (g_speed < g_std_speed) {      //can't go higher than initial speed
    g_speed += g_speed_modif / 2  ; //by default: 50 / 2 = 25
  } else {
    g_speed = g_std_speed;          //in case g_speed isn't dividable by 25 (let's say 251 for some reason), just make it back to standard (default: 250)
  }
}


//////////////////////DRAWING NUMBERS FUNCTIONS//////////////////////////////////////////////

void drawScore ( int score){                                                            //draws the score on the display. Since it's too small to show more than 2 numbers at once
  byte place = 0;                                                                       //without making them look very ugly, I decided to reduce available scores to 3 ranges, so:
  char color;                                                                           //red will show full score from 0 to 99,
  String stringScore;                                                                   //yellow will show first two characters of the score from 100 to 999 and 
        if (score >= 1000) {                                                            //green will show first two characters of the score from 1000 to 9999
          color = GREEN;                                                                  
          stringScore = String(score / 100);                                            //I was thinking of just making the score go slower and slower over time, but I don't think 99 numbers
        } else if (score >= 100){                                                       //are enough to represent any reasonable and easily comparable scores (since the difference between 98 and 99 would be
          color = YELLOW;                                                               //MUCH higher than for example the difference between 40 and 60)
          stringScore = String(score / 10);
        } else if (score < 10){
          color = RED;
          stringScore = "0" + String(score / 10); 
        } else {
          color = RED;
          stringScore = String(score);
      }
      
      for (byte i = MINX; i <= ((stringScore.length()+1) * 3); i += 4){                 //draws the numbers neatly
        drawChar(stringScore[place], i, MINY+1, color);
        place++;
       }
}


void lightPattern(const byte letter[],  size_t letter_length, byte x, byte y, char color){  //http://stackoverflow.com/questions/37538/how-do-i-determine-the-size-of-my-array-in-c
  for (int i = 0; i < letter_length; i += 2){                                               //that's where I learned how to measure length of an array (Mark Harrison's post, rated best answer)
    AberLED.set(letter[i] + x, letter[i+1] + y, color);                                     //code might not be the same and it was just the idea, but I thought I should leave it here
   #ifdef DEBUG                                                                             //I use for loop to light up different LEDs which coordinates I get from arrays at the top
      Serial.println("#DRAWLETTER: (" + String(i) + ", " + String(i+1) + ")"); 
    #endif */
  }
}

void drawChar(char c, byte x, byte y, char color){                                        //This function draws characters at given position (so I could put a number in the corner if
  switch(c){                                                                              //I wanted to, but I have to remember that letters are made of 3x4 boxes and consider that
    case '0':                                                                             //if I were to put one after another)
      lightPattern(LETTER_0, sizeof(LETTER_0)  / sizeof(LETTER_0[0]), x, y, color);
      break;
    case '1':
      lightPattern(LETTER_1, sizeof(LETTER_1)  / sizeof(LETTER_1[0]), x, y, color);
      break;
    case '2':
      lightPattern(LETTER_2, sizeof(LETTER_2)  / sizeof(LETTER_2[0]), x, y, color);
      break;
    case '3':
      lightPattern(LETTER_3, sizeof(LETTER_3)  / sizeof(LETTER_3[0]), x, y, color);
      break;
    case '4':
      lightPattern(LETTER_4, sizeof(LETTER_4)  / sizeof(LETTER_4[0]), x, y, color);
      break;
    case '5':
      lightPattern(LETTER_5, sizeof(LETTER_5)  / sizeof(LETTER_5[0]), x, y, color);
      break;
    case '6':
      lightPattern(LETTER_6, sizeof(LETTER_6)  / sizeof(LETTER_6[0]), x, y, color);
      break;
    case '7':
      lightPattern(LETTER_7, sizeof(LETTER_7)  / sizeof(LETTER_7[0]), x, y, color);
      break;
    case '8':
      lightPattern(LETTER_8, sizeof(LETTER_8)  / sizeof(LETTER_8[0]), x, y, color);
      break;
    case '9':
      lightPattern(LETTER_9, sizeof(LETTER_9)  / sizeof(LETTER_9[0]), x, y, color);
      break;
    default:
      break;
  }
}
//////////////////////READING/WRITING HIGHSCORE FROM/TO EEPROM//////////////////////////////////////////////

void eepromScore(){
  String highscore = "";
  char character;
  byte addr = 0;
  String temp_score = "";
      for (int i = 0; i < 4; i ++){                                        //read characters representing score from EEPROM
        character = EEPROM.read(addr);
        highscore = highscore + String(character);
        #ifdef DEBUG
          Serial.println("--EEPROMREAD: " + String(character) + " AT " + String(addr));
        #endif 
        addr++;
      }
      if (highscore.toInt() < g_score){                                     //check if new score is higher than the old highscore and if true, replace the value in EEPROM
        addr = 0;
        temp_score = String(g_score);
        while (temp_score.length() < 4){                                    //Add 0s to the beginning of the score so it's always first 4 bytes from EEPROM, no matter how long the number is
          temp_score = '0' + temp_score;
        }
        for (int i = 0; i < temp_score.length(); i ++){                     //at first I treated score as a number and tried to make the loop divide it by 10 to put numbers in separate
          EEPROM.write(addr, (temp_score[i]));                              //places, but then I realised it's easier to treat them as a string here
          Serial.println("--EEPROM WRITE: " + String(temp_score[i]) + " AT " + String(addr));
          addr++;
        }
      }
      //#ifdef DEBUG
          Serial.println(".EEPROMFUNC:  highscore:" + String(highscore.toInt()) + " | g_score: " + String(g_score));
      //#endif 
      g_score = highscore.toInt();
}






