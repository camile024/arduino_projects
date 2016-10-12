//Simple snake
//Todo: apples (x), growing (x), levels, game-over sequence (x), faster apple creating algorithm (x i guess), more responsive buttons

#include <AberLED.h>
//#define DEBUG

byte g_direction = 0; //0 - stop, 1 - up, 2 - right, 3 - down, 4 - left   /////////////////////////////////////////////////////
byte g_coords[2];                                                         //                                                 //
byte g_coords_tail[30][30];                                               //  MODIFY INITIAL SETTINGS IN VOID SETUP()        //
byte g_taillength;                                                        // ( CAN'T HAVE THEM HERE TO NOT MESS UP GAME )    //
byte g_apple[2];                                                          //           ( OVER BEHAVIOUR )                    //
bool g_newtail = false;                                                   //                                                 //
bool g_incspeed;                                                          //                                                 //
bool g_gameover = false;                                                  /////////////////////////////////////////////////////
byte g_speed_modif;
byte g_speed;
bool g_inclevel;
byte g_level;
byte g_inclevel_modif;
byte g_level_progress = 0;
byte g_score = 0;
///////////////////////
const byte MINX = 0;
const byte MINY = 0;
const byte MAXX = 7;
const byte MAXY = 7;
const byte BTN_UP = 1;
const byte BTN_DOWN = 2;
const byte BTN_LEFT = 4;
const byte BTN_RIGHT = 3;
const byte BTN_STOP = 5;



void drawScore (byte score){
  
  
}



bool checkBackwards(byte x){                                    //CHECKS IF DIRECTION CHANGE IS NOT TRYING TO GO BACKWARDS
  bool returnval;
  returnval = true;
  
  switch (x){
    case 1 :
      if (g_direction == 3){ returnval = false; };
      break;
    case 2 :
      if (g_direction == 4){ returnval = false; };
      break;
    case 3 :
      if (g_direction == 1){ returnval = false; };
      break;
    case 4 :
      if (g_direction == 2){ returnval = false; };
      break;
    default : returnval = true; break;
  }
  return returnval;
}


void setDirection(){                                        //SETS THE DIRECTION ACCORDING TO A BUTTON
    for (int i = 1; i <= 5; i++){
    if (AberLED.getButton(i)){
      switch (i){
        case BTN_UP : 
          if(checkBackwards(1)){
          g_direction = 1;
          }
          break;
        case BTN_RIGHT :
          if(checkBackwards(2)){
          g_direction = 2;
          }
          break;
        case BTN_DOWN :
          if(checkBackwards(3)){
          g_direction = 3;
          }
          break;
        case BTN_LEFT :
        if(checkBackwards(4)){
          g_direction = 4;
        }
          break;   
        case BTN_STOP :
          g_direction = 0;
          break;
      }  
     } 
    }
}


void setPos(byte x, byte y){                                                //CHANGES COORDINATES AND LIGHTS UP THE LED, CHECKS IF COLLISION
  byte badposarr[MAXX+1][MAXY+1];
  bool gameover = false;
  
  if (x == g_apple[0] && y == g_apple[1] && x != 0 && y != 0){
    eatApple();
  }
  
  AberLED.set(x, y, YELLOW);
  
  for(int i = 2; i < g_taillength; i++){
    if ( g_coords_tail[i][0] == x && g_coords_tail[i][1] == y){ 
      gameover = true; 
    }
  }
  
  if (gameover == false){
    setCoords(x,y);
  } else {
    setCoords(x,y);
    forceGameOver();
  }
}

void forceGameOver(){                                                       //GAME OVER BEHAVIOUR
  g_direction = 0;
  AberLED.clear();
  for (int i = 0; i < g_taillength; i++){
      AberLED.set(g_coords_tail[i][0], g_coords_tail[i][1], RED);
    }
  AberLED.set(g_coords[0], g_coords[1], RED);
  
  AberLED.swap();
  delay(10000);
  setup();
}

void moveDir(){                                                           //MOVE THE SNAKE AROUND
   byte x = g_coords[0];
   byte y = g_coords[1];
        switch(g_direction){
        case 0: setPos(x,y); break;
        case 1: setPos(x, y-1); break;
        case 2: setPos(x+1, y); break;
        case 3: setPos(x, y+1); break;
        case 4: setPos(x-1, y); break;
      }
      moveTail();
}



void moveTail(){                                                          //DRAWS THE TAIL AND KEEPS IT UP TO DATE
  byte temp[2];
  byte leng = g_taillength-1;
  
  if (g_direction == 0){
    
    for (int i = 0; i < g_taillength; i++){
      AberLED.set(g_coords_tail[i][0], g_coords_tail[i][1], GREEN);
    }  
  } else {
 /* temp[1] = g_coords_tail[0][0];
    temp[2] = g_coords_tail[0][1];
    g_coords_tail[1][0] = g_coords_tail[0][0];
    g_coords_tail[1][1] = g_coords_tail[0][1];  */
      if (g_newtail == true){
        g_coords_tail[leng][0] = g_coords_tail[leng-1][0];
        g_coords_tail[leng][1] = g_coords_tail[leng-1][1];
        g_newtail = false;
      }
    for (int i = g_taillength - 1; i > 0; i--){
      AberLED.set(g_coords_tail[i][0], g_coords_tail[i][1], GREEN);
      //AberLED.set(g_coords_tail[1][0], g_coords_tail[1-1][1], GREEN);
      g_coords_tail[i][0] = g_coords_tail[i-1][0];
      g_coords_tail[i][1] = g_coords_tail[i-1][1];
    }
    AberLED.set(g_coords_tail[0][0], g_coords_tail[0][1], GREEN);
    g_coords_tail[0][0] = g_coords[0];
    g_coords_tail[0][1] = g_coords[1];
  }
}

void advanceLevel(){
  
}

void eatApple(){                                                                  //EATING APPLE BEHAVIOUR
  
  g_taillength += 1;
  g_newtail = true;
  if (g_incspeed) {
    g_speed = ((g_speed * g_speed_modif) / 100);
  }
  g_level_progress += 1;
  advanceLevel();
  addApple(0);

}



void addApple(byte state){                                                      //ADDING NEW APPLE/REWRITING EXISTING APPLE
  int x,y;
  byte badposarr[MAXX+1][MAXY+1];
  
  for (byte i = 0; i < MAXX; i++){
    for (byte j = 0; j < MAXY; j++){
      badposarr[i][j] = 0;
    }
  }
  
  bool badpos = false;
  
  if (state == 0) {
    
    do {
      if ( badpos == true){
        badposarr[x][y] = 1;
      }
      
      badpos = false;
      
      for(int i = 0; i < g_taillength; i++){
        badposarr[g_coords_tail[i][0]][g_coords_tail[i][1]] =  1;
      }
      
      do{
        x = random(MAXX-1)+1;
        y = random(MAXY-1)+1;
      } while ( badposarr[x][y] == 1);
      
      #ifdef DEBUG
      Serial.println("x: " + String(x) + " | y: " + String(y));
      #endif

      if ( x == g_coords[0] && y == g_coords[1]){
        #ifdef DEBUG
        Serial.println("coords_x: " + String(g_coords[0]) + " | y: " + String(g_coords[1]));
        #endif
        badpos = true;
      }
      
    } while (badpos);
    
    g_apple[0] = x; g_apple[1] = y;
  }else{
    x = g_apple[0]; y = g_apple[1];
  }

  
  AberLED.set(x,y, RED);

}



void doMove(){                                                                        //INITIAL MOVING SEQUENCE - CHECKS IF CROSSING THE BORDER TO ALABAMA
  byte x = g_coords[0];
  byte y = g_coords[1];
  byte dir = g_direction;
  
    if ( x == MAXX && dir == 2){
       setPos(MINX, y);
          moveTail();
     } else if ( y == MAXY && dir == 3 ){
        setPos(x, MINY);
          moveTail();
     } else if ( x == MINX && dir == 4){
       setPos(MAXX, y);
          moveTail();
     } else if ( y == MINY && dir == 1 ){
        setPos(x, MAXY);
          moveTail();
     } else {
      moveDir();
    }
}


void setCoords(byte x, byte y){                                                     //SETS COORDINATES, DOESN'T CHANGE LED STATE
  g_coords[0] = x; g_coords[1] = y;
}



void initializeSnake(){                                                             //DRAWS THE SNAKE IN THE BEGINNING ACCORDING TO SETTINGS
  int x = 0;
  
  for (int i = MINX + g_taillength; i >= MINX; i--){
    setPos(i, 0);
    g_coords_tail[i][0] = i;
    g_coords_tail[i][1] = 0;
    x++;
  }
  
  setPos(x-1,0);
  addApple(0);
}



void setup(){
  Serial.begin(9600);
  
  //RESET ALL THE VARS AFTER GAMEOVER OR DEFINE THEM AT THE START//
  g_direction = 0; //0 - stop, 1 - up, 2 - right, 3 - down, 4 - left
  g_coords[0] = 2;
  g_coords[1] = 0;
  g_taillength = 1;     //the initial length of tail
  g_apple[0] = 0;
  g_apple[1] = 0;
  g_newtail = false;
  g_gameover = false;
  g_incspeed = true;   //increase speed after each 'apple'?  
  g_inclevel = false;   //advance to new levels as you go
  g_inclevel_modif = 5; //how many 'apples' do  you have to eat before next level
  g_level = 0;         
  g_speed = 200;       //how often the snake moves in ms
  g_speed_modif = 95;  //in percentage of previous value
  //RESET ALL THE VARS AFTER GAMEOVER OR DEFINE THEM AT THE START//
  
  AberLED.begin(); 
  AberLED.clear(); 
  
  for(int i=0;i<8;i++){
    for(int j=0; j < 8; j++){
      AberLED.set (i ,j ,YELLOW) ;
    }
  }
  
  //setCoords(MINX, MINY);
  AberLED.swap();
  delay(1000);
  AberLED.clear();
  initializeSnake();
}



void loop(){  
    AberLED.clear();
    addApple(1);
    setDirection();
    doMove();
    delay(g_speed);
    AberLED.swap();
  
}

