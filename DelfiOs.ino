#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C
#define SDA_PIN 8
#define SCL_PIN 9

// 6 кнопок
#define BTN_UP     2
#define BTN_DOWN   4
#define BTN_SELECT 3
#define BTN_BACK   5
#define BTN_LEFT   6
#define BTN_RIGHT  7

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// =============================================
// FORWARD DECLARATIONS
// =============================================
void initDino();   void updateDino();
void initFlappy(); void updateFlappy();
void initCars();   void updateCars();
void initBeer();   void updateBeer();
void initSnake();  void updateSnake();
void init2048();   void update2048();
void initJump();   void updateJump();
void updateMenu();  void drawMenu();
void updateDifficulty(); void drawDifficulty();
void updateCosti(); void drawCosti();
void updateAbout(); void drawAbout();
void drawDolphinAnimation();

// =============================================
// СЛОЖНОСТЬ
// =============================================
enum Difficulty { DIFF_EASY, DIFF_NORMAL, DIFF_HARD };
Difficulty selectedDiff = DIFF_NORMAL;
int pendingGame = 0;
const char* diffLabels[] = {"Easy", "Normal", "Hard"};
int diffSelection = 1;

// =============================================
// СОСТОЯНИЯ
// =============================================
enum GameState {
  STATE_MENU, STATE_DIFF, STATE_ABOUT,
  STATE_DINO, STATE_FLAPPY, STATE_CARS, STATE_BEER,
  STATE_COSTI, STATE_SNAKE, STATE_2048, STATE_JUMP
};
GameState currentState = STATE_MENU;

// =============================================
// КНОПКИ
// =============================================
struct Button { int pin; bool lastState, pressed, held; unsigned long lastTime; };
Button btnUp     = {BTN_UP,     HIGH, false, false, 0};
Button btnDown   = {BTN_DOWN,   HIGH, false, false, 0};
Button btnSelect = {BTN_SELECT, HIGH, false, false, 0};
Button btnBack   = {BTN_BACK,   HIGH, false, false, 0};
Button btnLeft   = {BTN_LEFT,   HIGH, false, false, 0};
Button btnRight  = {BTN_RIGHT,  HIGH, false, false, 0};

void updateButton(Button &btn) {
  bool state = digitalRead(btn.pin);
  btn.pressed = false;
  btn.held = (state == LOW);
  if (state==LOW && btn.lastState==HIGH && millis()-btn.lastTime>80) {
    btn.pressed=true; btn.lastTime=millis();
  }
  btn.lastState=state;
}
void updateAllButtons() {
  updateButton(btnUp);   updateButton(btnDown);
  updateButton(btnSelect); updateButton(btnBack);
  updateButton(btnLeft); updateButton(btnRight);
}

// =============================================
// RNG
// =============================================
uint32_t rngState = 42;
uint32_t myRand() {
  rngState ^= rngState<<13;
  rngState ^= rngState>>17;
  rngState ^= rngState<<5;
  return rngState;
}

// =============================================
// ДЕЛЬФИН АНИМАЦИЯ
// =============================================
void drawDolphin(int x, int y, int frame) {
  display.fillEllipse(x, y+5, 14, 5, WHITE);
  display.fillCircle(x+12, y+5, 5, WHITE);
  display.fillTriangle(x+16,y+4,x+22,y+5,x+16,y+7,WHITE);
  display.fillCircle(x+14,y+3,1,BLACK);
  display.fillTriangle(x+4,y+1,x+9,y-6,x+12,y+1,WHITE);
  if(frame%2==0){
    display.fillTriangle(x-14,y+1,x-6,y+4,x-6,y+8,WHITE);
    display.fillTriangle(x-14,y+9,x-6,y+4,x-6,y+8,WHITE);
  } else {
    display.fillTriangle(x-14,y+3,x-6,y+4,x-6,y+8,WHITE);
    display.fillTriangle(x-14,y+11,x-6,y+4,x-6,y+8,WHITE);
  }
  display.drawLine(x+4,y+9,x+14,y+9,WHITE);
}
void drawWave(int offset,int yBase){
  for(int x=0;x<SCREEN_WIDTH;x++){
    int y=yBase+(int)(2.5*sin((x+offset)*0.15));
    display.drawPixel(x,y,WHITE);
  }
}
void drawDolphinAnimation(){
  int dx=-28,frame=0;float dy=38,vel=0;bool jumping=false;
  unsigned long lastF=0;int wOff=0;
  while(dx<SCREEN_WIDTH+30){
    display.clearDisplay();
    drawWave(wOff,50);drawWave(wOff+30,53);
    if(!jumping){dy=33+sin(millis()*0.008)*2;if(dx>=20&&dx<24){jumping=true;vel=-5.5;}}
    if(jumping){vel+=0.35;dy+=vel;if(dy>=34){jumping=false;dy=34;vel=0;}}
    if(jumping&&vel<0&&dx>18&&dx<30){
      display.drawPixel(dx-2,51,WHITE);display.drawPixel(dx-5,50,WHITE);
      display.drawPixel(dx+2,50,WHITE);display.drawPixel(dx-3,49,WHITE);
    }
    drawDolphin(dx,(int)dy,frame);
    display.setTextSize(2);display.setTextColor(WHITE);
    display.setCursor(24,4);display.print("DelfiOs");
    display.setTextSize(1);display.setCursor(42,55);display.print("v4.0");
    display.display();
    dx+=2;wOff+=3;
    if(millis()-lastF>140){frame++;lastF=millis();}
    delay(28);
    if(digitalRead(BTN_SELECT)==LOW||digitalRead(BTN_UP)==LOW)return;
  }
  delay(300);
}

// =============================================
// ABOUT
// =============================================
void drawAbout(){
  display.clearDisplay();
  display.drawRoundRect(0,0,128,64,4,WHITE);
  display.drawLine(0,12,128,12,WHITE);
  display.setTextSize(1);display.setTextColor(WHITE);
  display.setCursor(38,3);display.print("DelfiOs");
  display.fillEllipse(114,6,7,3,WHITE);display.fillCircle(120,6,3,WHITE);
  display.fillTriangle(122,5,124,6,122,7,WHITE);display.fillCircle(121,5,1,BLACK);
  display.fillTriangle(107,5,110,6,107,8,WHITE);
  display.setCursor(8,17);display.print("Device : ESP32-C3");
  display.drawLine(8,25,120,25,WHITE);
  display.setCursor(8,28);display.print("Version: 2.2");
  display.drawLine(8,36,120,36,WHITE);
  display.setCursor(8,39);display.print("Tt     : ArduinoP");
  display.drawLine(8,47,120,47,WHITE);
  display.setCursor(8,50);display.print("OS     : DelfiOs v4");
  display.fillTriangle(4,61,10,57,10,61,WHITE);
  display.display();
}
void updateAbout(){
  if(btnBack.pressed||btnSelect.pressed)currentState=STATE_MENU;
}

// =============================================
// МЕНЮ — DelfiOs всегда внизу
// =============================================
int menuSelection=0;
// Игры + утилиты (DelfiOs всегда последний)
const int MENU_GAMES=9; // без DelfiOs
const int MENU_ITEMS=10;
const char* menuLabels[]={"Dino Run","Flappy Bird","Cars","Root Beer","Snake","2048","Jump!","Costi","DelfiOs","[About]"};
// 0-6: игры, 7: Costi, 8: DelfiOs->About

void drawMenuIcon(int idx,int x,int y,bool selected){
  uint16_t col=selected?BLACK:WHITE;
  switch(idx){
    case 0: // Dino
      display.fillRect(x+1,y+2,5,4,col);display.fillRect(x+2,y,3,3,col);
      display.fillRect(x+5,y+1,2,2,col);display.drawPixel(x+6,y,col);break;
    case 1: // Flappy
      display.fillCircle(x+2,y+3,2,col);display.fillTriangle(x+3,y+2,x+7,y+3,x+3,y+4,col);
      display.drawLine(x,y,x+2,y+2,col);display.drawLine(x+1,y,x+3,y+2,col);break;
    case 2: // Cars
      display.fillRect(x,y+3,9,3,col);display.fillRect(x+1,y+1,6,3,col);
      display.fillCircle(x+2,y+6,1,col);display.fillCircle(x+7,y+6,1,col);
      display.fillRect(x+2,y+2,2,2,selected?WHITE:BLACK);break;
    case 3: // Beer
      display.drawLine(x,y,x+7,y,col);display.drawLine(x,y,x+1,y+7,col);
      display.drawLine(x+7,y,x+6,y+7,col);display.drawLine(x+1,y+7,x+6,y+7,col);
      display.drawLine(x+7,y+2,x+9,y+2,col);display.drawLine(x+7,y+5,x+9,y+5,col);
      display.drawLine(x+9,y+2,x+9,y+5,col);break;
    case 4: // Snake
      display.drawLine(x,y+3,x+4,y+3,col);display.drawLine(x+4,y+3,x+4,y,col);
      display.drawLine(x+4,y,x+8,y,col);display.drawPixel(x+9,y,col);
      display.fillRect(x,y+4,3,3,col);break;
    case 5: // 2048
      display.drawRect(x,y,5,5,col);display.drawRect(x+5,y,5,5,col);
      display.drawRect(x,y+5,5,5,col);display.drawRect(x+5,y+5,5,5,col);
      display.drawPixel(x+2,y+2,col);display.drawPixel(x+7,y+2,col);break;
    case 6: // Jump/Mario
      display.fillRect(x+2,y,4,2,col);display.fillRect(x,y+2,8,3,col);
      display.fillRect(x+2,y+5,4,3,col);display.fillCircle(x+6,y+6,1,col);
      display.fillCircle(x+2,y+6,1,col);break;
    case 7: // Costi
      display.drawRoundRect(x,y,8,8,1,col);display.fillCircle(x+2,y+2,1,col);
      display.fillCircle(x+5,y+5,1,col);display.fillCircle(x+5,y+2,1,col);break;
    case 8: // DelfiOs
      display.fillEllipse(x+4,y+4,4,2,col);display.fillCircle(x+7,y+4,2,col);
      display.fillTriangle(x+8,y+3,x+10,y+4,x+8,y+5,col);
      display.fillTriangle(x,y+3,x+2,y+3,x,y+5,col);break;
    default: display.drawPixel(x+4,y+4,col);break;
  }
}

void drawMenu(){
  display.clearDisplay();
  display.setTextSize(1);display.setTextColor(WHITE);
  display.drawPixel(0,0,WHITE);display.drawPixel(1,0,WHITE);display.drawPixel(0,1,WHITE);
  display.drawPixel(127,0,WHITE);display.drawPixel(126,0,WHITE);display.drawPixel(127,1,WHITE);
  display.setCursor(26,1);display.print("* DelfiOs *");
  display.drawLine(0,10,128,10,WHITE);

  int startIdx=max(0,min(menuSelection-1,MENU_ITEMS-3));
  for(int i=0;i<3;i++){
    int idx=startIdx+i;
    if(idx>=MENU_ITEMS)break;
    int y=13+i*16;
    bool sel=(idx==menuSelection);
    if(sel){display.fillRoundRect(2,y-1,121,13,2,WHITE);display.setTextColor(BLACK);}
    else display.setTextColor(WHITE);
    drawMenuIcon(idx,10,y,sel);
    display.setCursor(24,y+3);display.print(menuLabels[idx]);
    if(idx==8){uint16_t c=sel?BLACK:WHITE;display.fillTriangle(116,y+3,116,y+9,120,y+6,c);}
    display.setTextColor(WHITE);
  }

  int sbH=48,sbY=11;
  int thumbH=max(8,sbH/MENU_ITEMS);
  int thumbY=sbY+(sbH-thumbH)*menuSelection/max(1,MENU_ITEMS-1);
  display.drawLine(126,sbY,126,sbY+sbH,WHITE);
  display.fillRect(125,thumbY,3,thumbH,WHITE);

  for(int i=0;i<MENU_ITEMS;i++){
    int px=34+i*6;
    if(i==menuSelection)display.fillCircle(px,61,2,WHITE);
    else display.drawCircle(px,61,1,WHITE);
  }
  display.display();
}

void updateMenu(){
  if(btnUp.pressed)   menuSelection=(menuSelection-1+MENU_ITEMS)%MENU_ITEMS;
  if(btnDown.pressed) menuSelection=(menuSelection+1)%MENU_ITEMS;
  if(btnSelect.pressed){
    switch(menuSelection){
      case 7: currentState=STATE_COSTI; return;
      case 8: // DelfiOs -> About
      case 9: currentState=STATE_ABOUT; return;
      default:
        pendingGame=menuSelection;
        diffSelection=1;
        // Snake и 2048 и Jump тоже идут через сложность
        currentState=STATE_DIFF;
    }
  }
}

// =============================================
// СЛОЖНОСТЬ
// =============================================
void drawDifficulty(){
  display.clearDisplay();
  display.drawRoundRect(0,0,128,64,4,WHITE);
  display.setTextSize(1);display.setTextColor(WHITE);
  int nameLen=strlen(menuLabels[pendingGame]);
  display.setCursor(64-nameLen*3,3);display.print(menuLabels[pendingGame]);
  display.drawLine(0,12,128,12,WHITE);
  for(int i=0;i<3;i++){
    int y=17+i*14;bool sel=(i==diffSelection);
    if(sel){display.fillRoundRect(4,y-1,119,13,2,WHITE);display.setTextColor(BLACK);}
    else display.setTextColor(WHITE);
    display.setCursor(16,y+2);display.print(diffLabels[i]);
    for(int s=0;s<=i;s++){
      int sx=78+s*10,sy=y+2;uint16_t c=sel?BLACK:WHITE;
      display.drawLine(sx+2,sy,sx+2,sy+4,c);display.drawLine(sx,sy+2,sx+4,sy+2,c);
      display.drawLine(sx,sy+1,sx+4,sy+3,c);display.drawLine(sx+4,sy+1,sx,sy+3,c);
    }
    display.setTextColor(WHITE);
  }
  display.fillTriangle(4,61,10,57,10,61,WHITE);
  display.display();
}
void updateDifficulty(){
  if(btnBack.pressed){currentState=STATE_MENU;return;}
  if(btnUp.pressed)   diffSelection=(diffSelection-1+3)%3;
  if(btnDown.pressed) diffSelection=(diffSelection+1)%3;
  if(btnSelect.pressed){
    selectedDiff=(Difficulty)diffSelection;
    if     (pendingGame==0){currentState=STATE_DINO;   initDino();}
    else if(pendingGame==1){currentState=STATE_FLAPPY; initFlappy();}
    else if(pendingGame==2){currentState=STATE_CARS;   initCars();}
    else if(pendingGame==3){currentState=STATE_BEER;   initBeer();}
    else if(pendingGame==4){currentState=STATE_SNAKE;  initSnake();}
    else if(pendingGame==5){currentState=STATE_2048;   init2048();}
    else if(pendingGame==6){currentState=STATE_JUMP;   initJump();}
  }
}

// =============================================
// COSTI
// =============================================
int costiResult=0;bool costiRolling=false;
unsigned long costiRollStart=0;int costiAnim=0;
void drawDie(int val,int cx,int cy,int size){
  display.fillRoundRect(cx-size+2,cy-size+2,size*2,size*2,3,WHITE);
  display.fillRoundRect(cx-size,cy-size,size*2,size*2,3,BLACK);
  display.drawRoundRect(cx-size,cy-size,size*2,size*2,3,WHITE);
  int d=size/3;
  switch(val){
    case 1:display.fillCircle(cx,cy,2,WHITE);break;
    case 2:display.fillCircle(cx-d,cy-d,2,WHITE);display.fillCircle(cx+d,cy+d,2,WHITE);break;
    case 3:display.fillCircle(cx-d,cy-d,2,WHITE);display.fillCircle(cx,cy,2,WHITE);display.fillCircle(cx+d,cy+d,2,WHITE);break;
    case 4:display.fillCircle(cx-d,cy-d,2,WHITE);display.fillCircle(cx+d,cy-d,2,WHITE);display.fillCircle(cx-d,cy+d,2,WHITE);display.fillCircle(cx+d,cy+d,2,WHITE);break;
    case 5:display.fillCircle(cx-d,cy-d,2,WHITE);display.fillCircle(cx+d,cy-d,2,WHITE);display.fillCircle(cx,cy,2,WHITE);display.fillCircle(cx-d,cy+d,2,WHITE);display.fillCircle(cx+d,cy+d,2,WHITE);break;
    case 6:display.fillCircle(cx-d,cy-d,2,WHITE);display.fillCircle(cx+d,cy-d,2,WHITE);display.fillCircle(cx-d,cy,2,WHITE);display.fillCircle(cx+d,cy,2,WHITE);display.fillCircle(cx-d,cy+d,2,WHITE);display.fillCircle(cx+d,cy+d,2,WHITE);break;
  }
}
void updateCosti(){
  if(btnBack.pressed){costiResult=0;costiRolling=false;currentState=STATE_MENU;return;}
  if(btnSelect.pressed&&!costiRolling){costiRolling=true;costiRollStart=millis();rngState=millis();}
  if(costiRolling){costiAnim=(myRand()%6)+1;if(millis()-costiRollStart>1200){costiRolling=false;costiResult=(myRand()%6)+1;}}
  display.clearDisplay();display.drawRoundRect(0,0,128,64,4,WHITE);
  display.setTextSize(1);display.setTextColor(WHITE);
  display.setCursor(46,4);display.print("Costi");display.drawLine(0,13,128,13,WHITE);
  if(costiRolling){int off=(millis()/60)%5-2;drawDie(costiAnim,64,35+off,18);}
  else if(costiResult==0)drawDie(1,64,35,18);
  else{
    drawDie(costiResult,64,35,18);
    const char* msgs[]={"1 ...","2 - мало","3 - ок","4 - хорошо","5 - почти!","6 - !!!"};
    int tw=strlen(msgs[costiResult-1])*6;
    display.setCursor(64-tw/2,56);display.print(msgs[costiResult-1]);
  }
  display.display();
}

// =============================================
// ROOT BEER
// =============================================
struct BeerDiff{int fillSpeed,targetMin,targetMax;unsigned long timeLimit;};
BeerDiff beerDiffs[]={{1,50,90,0},{2,60,80,0},{3,65,75,8000}};
int beerLevel=0;bool beerDone=false,beerWin=false,beerFoam=false;
int beerFoamAnim=0,beerRound=1;unsigned long beerStartTime=0,beerLastUpdate=0;
void initBeer(){beerLevel=0;beerDone=false;beerWin=false;beerFoam=false;beerFoamAnim=0;beerRound=1;beerStartTime=millis();beerLastUpdate=millis();}
void drawBeerGlass(int fillPct){
  int gx=38,gy=10,gw=46,gh=46;
  display.fillRect(gx+2,gy+2,gw,gh,WHITE);display.fillRect(gx,gy,gw,gh,BLACK);
  if(fillPct>0){
    int fillH=(gh-2)*fillPct/100,fillY=gy+gh-1-fillH;
    for(int y=fillY;y<gy+gh-1;y++){
      int lx=gx+2+(y-gy)/8,rx=gx+gw-2-(y-gy)/8;
      if((y-fillY)%4==0)display.drawPixel(lx+2+(myRand()%max(1,rx-lx-2)),y,WHITE);
      else display.drawLine(lx,y,rx,y,WHITE);
    }
    if(fillPct>30){int foamY=fillY-1;for(int fx=gx+3;fx<gx+gw-3;fx+=5){int fa=beerFoamAnim%3;display.fillCircle(fx+fa,foamY,3,WHITE);display.fillCircle(fx+3-fa,foamY-1,2,WHITE);}}
  }
  display.drawLine(gx,gy,gx+gw,gy,WHITE);display.drawLine(gx,gy,gx+2,gy+gh,WHITE);
  display.drawLine(gx+gw,gy,gx+gw-2,gy+gh,WHITE);display.drawLine(gx+2,gy+gh,gx+gw-2,gy+gh,WHITE);
  display.drawLine(gx+gw,gy+8,gx+gw+8,gy+8,WHITE);display.drawLine(gx+gw,gy+20,gx+gw+8,gy+20,WHITE);display.drawLine(gx+gw+8,gy+8,gx+gw+8,gy+20,WHITE);
}
void drawBeerZone(int tMin,int tMax){
  int sx=94,sy=10,sh=46;display.drawRect(sx,sy,6,sh,WHITE);
  int zMin=sy+sh-sh*tMax/100,zMax=sy+sh-sh*tMin/100;
  for(int y=zMin;y<=zMax;y++)if(y%2==0)display.drawLine(sx+1,y,sx+4,y,WHITE);
  int arrY=constrain(sy+sh-sh*beerLevel/100,sy,sy+sh);
  display.fillTriangle(sx+7,arrY-2,sx+7,arrY+2,sx+11,arrY,WHITE);
  display.setTextSize(1);display.setCursor(100,sy+sh+3);display.print(beerLevel);display.print("%");
}
void updateBeer(){
  if(btnBack.pressed){currentState=STATE_MENU;return;}
  BeerDiff&bd=beerDiffs[(int)selectedDiff];
  if(beerDone){
    if(btnSelect.pressed){if(beerWin&&beerRound<3){beerRound++;beerLevel=0;beerDone=false;beerWin=false;beerFoam=false;beerStartTime=millis();}else currentState=STATE_MENU;}
    display.clearDisplay();display.drawRoundRect(0,0,128,64,4,WHITE);
    display.setTextSize(1);display.setTextColor(WHITE);
    if(beerWin){display.setCursor(28,10);display.print("Root Beer!");display.drawCircle(64,36,16,WHITE);display.fillCircle(58,31,2,WHITE);display.fillCircle(70,31,2,WHITE);display.drawLine(56,42,60,45,WHITE);display.drawLine(60,45,68,45,WHITE);display.drawLine(68,45,72,42,WHITE);}
    else{display.setCursor(20,10);display.print("Game Over");display.drawCircle(64,36,16,WHITE);display.fillCircle(58,31,2,WHITE);display.fillCircle(70,31,2,WHITE);display.drawLine(56,45,60,42,WHITE);display.drawLine(60,42,68,42,WHITE);display.drawLine(68,42,72,45,WHITE);}
    display.setCursor(50,58);display.print("R:");display.print(beerRound);display.print("/3");display.display();return;
  }
  if(selectedDiff==DIFF_HARD&&bd.timeLimit>0&&millis()-beerStartTime>bd.timeLimit){beerDone=true;beerWin=(beerLevel>=bd.targetMin&&beerLevel<=bd.targetMax);return;}
  bool btnHeld=(digitalRead(BTN_SELECT)==LOW);
  if(millis()-beerLastUpdate>50){
    if(btnHeld){beerLevel+=bd.fillSpeed;if(beerLevel>100)beerLevel=100;beerFoamAnim++;}
    else if(selectedDiff!=DIFF_EASY&&beerLevel>0)beerLevel=max(0,beerLevel-1);
    beerLastUpdate=millis();
  }
  if(btnSelect.pressed){beerDone=true;beerWin=(beerLevel>=bd.targetMin&&beerLevel<=bd.targetMax);}
  display.clearDisplay();display.drawRoundRect(0,0,128,64,4,WHITE);
  display.setTextSize(1);display.setTextColor(WHITE);display.setCursor(28,2);display.print("Root Beer");display.drawLine(0,10,128,10,WHITE);
  drawBeerGlass(beerLevel);drawBeerZone(bd.targetMin,bd.targetMax);
  for(int i=0;i<3;i++){if(i<beerRound)display.fillCircle(10+i*8,57,3,WHITE);else display.drawCircle(10+i*8,57,3,WHITE);}
  if(selectedDiff==DIFF_HARD){int rem=max(0,(int)(bd.timeLimit-(millis()-beerStartTime))/1000);display.setCursor(55,57);display.print(rem);display.print("s");}
  display.display();
}

// =============================================
// SNAKE
// =============================================
#define SNAKE_W 21
#define SNAKE_H 10
#define SNAKE_CELL 5

struct SnakeSegment{int8_t x,y;};
SnakeSegment snake[SNAKE_W*SNAKE_H];
int snakeLen=0;
int8_t snakeDirX=1,snakeDirY=0;
int8_t snakeFoodX=0,snakeFoodY=0;
int snakeScore=0,snakeHigh=0;
bool snakeOver=false;
unsigned long snakeLastMove=0;
int snakeMoveInterval=300;

void placeSnakeFood(){
  bool ok=false;
  while(!ok){
    snakeFoodX=myRand()%SNAKE_W;snakeFoodY=myRand()%SNAKE_H;ok=true;
    for(int i=0;i<snakeLen;i++)if(snake[i].x==snakeFoodX&&snake[i].y==snakeFoodY){ok=false;break;}
  }
}

void initSnake(){
  snakeLen=3;
  snake[0]={5,5};snake[1]={4,5};snake[2]={3,5};
  snakeDirX=1;snakeDirY=0;snakeScore=0;snakeOver=false;
  snakeMoveInterval=(selectedDiff==DIFF_EASY?350:selectedDiff==DIFF_HARD?150:250);
  snakeLastMove=millis();rngState=millis()+333;placeSnakeFood();
}

void updateSnake(){
  if(btnBack.pressed){if(snakeScore>snakeHigh)snakeHigh=snakeScore;currentState=STATE_MENU;return;}
  if(snakeOver){
    if(btnSelect.pressed){if(snakeScore>snakeHigh)snakeHigh=snakeScore;initSnake();}
    display.clearDisplay();display.drawRoundRect(0,0,128,64,4,WHITE);
    display.setTextSize(2);display.setCursor(8,6);display.print("Game Over");
    display.drawLine(0,24,128,24,WHITE);display.setTextSize(1);
    display.setCursor(4,30);display.print("SC");display.setCursor(4,40);display.print(snakeScore);
    display.setCursor(90,30);display.print("HI");display.setCursor(90,40);display.print(max(snakeScore,snakeHigh));
    display.display();return;
  }

  // Управление LEFT/RIGHT/UP/DOWN
  if(btnLeft.pressed  &&snakeDirX==0){snakeDirX=-1;snakeDirY=0;}
  if(btnRight.pressed &&snakeDirX==0){snakeDirX=1;snakeDirY=0;}
  if(btnUp.pressed    &&snakeDirY==0){snakeDirX=0;snakeDirY=-1;}
  if(btnDown.pressed  &&snakeDirY==0){snakeDirX=0;snakeDirY=1;}

  if(millis()-snakeLastMove<(unsigned long)snakeMoveInterval){
    // Только рисуем
  } else {
    snakeLastMove=millis();
    int nx=snake[0].x+snakeDirX;
    int ny=snake[0].y+snakeDirY;
    // Стены
    if(nx<0||nx>=SNAKE_W||ny<0||ny>=SNAKE_H){snakeOver=true;return;}
    // Сам себя
    for(int i=0;i<snakeLen;i++)if(snake[i].x==nx&&snake[i].y==ny){snakeOver=true;return;}
    // Еда?
    bool ate=(nx==snakeFoodX&&ny==snakeFoodY);
    // Сдвиг
    if(!ate){for(int i=snakeLen-1;i>0;i--)snake[i]=snake[i-1];}
    else{if(snakeLen<SNAKE_W*SNAKE_H)snakeLen++;for(int i=snakeLen-1;i>0;i--)snake[i]=snake[i-1];snakeScore++;placeSnakeFood();if(snakeMoveInterval>80)snakeMoveInterval-=5;}
    snake[0]={nx,ny};
  }

  // Рисуем
  display.clearDisplay();
  // Поле
  int ox=5,oy=12;
  display.drawRect(ox-1,oy-1,SNAKE_W*SNAKE_CELL+2,SNAKE_H*SNAKE_CELL+2,WHITE);
  // Змейка
  for(int i=0;i<snakeLen;i++){
    int px=ox+snake[i].x*SNAKE_CELL,py=oy+snake[i].y*SNAKE_CELL;
    if(i==0)display.fillRect(px,py,SNAKE_CELL,SNAKE_CELL,WHITE);
    else display.drawRect(px,py,SNAKE_CELL,SNAKE_CELL,WHITE);
  }
  // Еда — мигающий квадрат
  int fx=ox+snakeFoodX*SNAKE_CELL,fy=oy+snakeFoodY*SNAKE_CELL;
  if((millis()/300)%2==0)display.fillRect(fx,fy,SNAKE_CELL,SNAKE_CELL,WHITE);
  else display.drawRect(fx,fy,SNAKE_CELL,SNAKE_CELL,WHITE);
  // HUD
  display.setTextSize(1);display.setTextColor(WHITE);
  display.setCursor(115,12);display.print(snakeScore);
  display.setCursor(108,24);display.print("HI");display.setCursor(108,32);display.print(max(snakeScore,snakeHigh));
  display.display();
}

// =============================================
// 2048
// =============================================
int board[4][4];
int score2048=0,high2048=0;
bool game2048Over=false,game2048Win=false;

void addRandom2048(){
  int empty[16][2];int cnt=0;
  for(int r=0;r<4;r++)for(int c=0;c<4;c++)if(board[r][c]==0){empty[cnt][0]=r;empty[cnt][1]=c;cnt++;}
  if(cnt==0)return;
  int idx=myRand()%cnt;
  board[empty[idx][0]][empty[idx][1]]=(myRand()%10<9)?2:4;
}

void init2048(){
  for(int r=0;r<4;r++)for(int c=0;c<4;c++)board[r][c]=0;
  score2048=0;game2048Over=false;game2048Win=false;
  rngState=millis()+555;addRandom2048();addRandom2048();
}

bool move2048Left(){
  bool moved=false;
  for(int r=0;r<4;r++){
    int row[4]={0,0,0,0};int pos=0;
    for(int c=0;c<4;c++)if(board[r][c])row[pos++]=board[r][c];
    for(int c=0;c<3;c++)if(row[c]&&row[c]==row[c+1]){row[c]*=2;score2048+=row[c];row[c+1]=0;if(row[c]==2048)game2048Win=true;}
    int row2[4]={0,0,0,0};pos=0;
    for(int c=0;c<4;c++)if(row[c])row2[pos++]=row[c];
    for(int c=0;c<4;c++){if(board[r][c]!=row2[c])moved=true;board[r][c]=row2[c];}
  }
  return moved;
}
bool move2048Right(){
  for(int r=0;r<4;r++){int l=0,ri=3;while(l<ri){int t=board[r][l];board[r][l]=board[r][ri];board[r][ri]=t;l++;ri--;}}
  bool m=move2048Left();
  for(int r=0;r<4;r++){int l=0,ri=3;while(l<ri){int t=board[r][l];board[r][l]=board[r][ri];board[r][ri]=t;l++;ri--;}}
  return m;
}
bool move2048Up(){
  for(int c=0;c<4;c++)for(int r=0;r<2;r++){int t=board[r][c];board[r][c]=board[r+2<4?r+2:r][c];}// transpose
  // proper transpose
  for(int r=0;r<4;r++)for(int c=r+1;c<4;c++){int t=board[r][c];board[r][c]=board[c][r];board[c][r]=t;}
  bool m=move2048Left();
  for(int r=0;r<4;r++)for(int c=r+1;c<4;c++){int t=board[r][c];board[r][c]=board[c][r];board[c][r]=t;}
  return m;
}
bool move2048Down(){
  for(int r=0;r<4;r++)for(int c=r+1;c<4;c++){int t=board[r][c];board[r][c]=board[c][r];board[c][r]=t;}
  bool m=move2048Right();
  for(int r=0;r<4;r++)for(int c=r+1;c<4;c++){int t=board[r][c];board[r][c]=board[c][r];board[c][r]=t;}
  return m;
}

bool hasMovesLeft(){
  for(int r=0;r<4;r++)for(int c=0;c<4;c++){
    if(board[r][c]==0)return true;
    if(c<3&&board[r][c]==board[r][c+1])return true;
    if(r<3&&board[r][c]==board[r+1][c])return true;
  }
  return false;
}

void draw2048Tile(int r,int c,int val){
  int ox=2,oy=12,ts=14;
  int x=ox+c*(ts+2),y=oy+r*(ts+2);
  // Всегда рисуем рамку клетки
  display.drawRect(x,y,ts,ts,WHITE);
  if(val==0)return;
  // Заливка для непустых клеток
  display.fillRect(x+1,y+1,ts-2,ts-2,WHITE);
  display.setTextColor(BLACK);display.setTextSize(1);
  String s=String(val);
  int sw=s.length()*6;
  display.setCursor(x+(ts-sw)/2,y+(ts-6)/2);
  display.print(s);
  display.setTextColor(WHITE);
}

void update2048(){
  if(btnBack.pressed){if(score2048>high2048)high2048=score2048;currentState=STATE_MENU;return;}
  if(game2048Over||game2048Win){
    if(btnSelect.pressed){if(score2048>high2048)high2048=score2048;init2048();}
    display.clearDisplay();display.drawRoundRect(0,0,128,64,4,WHITE);
    display.setTextSize(2);display.setCursor(game2048Win?18:8,8);
    display.print(game2048Win?"2048!":"Game Over");
    display.drawLine(0,26,128,26,WHITE);display.setTextSize(1);
    display.setCursor(10,32);display.print("Score: ");display.print(score2048);
    display.setCursor(10,44);display.print("Best:  ");display.print(max(score2048,high2048));
    display.display();return;
  }

  bool moved=false;
  if(btnLeft.pressed) moved=move2048Left();
  if(btnRight.pressed)moved=move2048Right();
  if(btnUp.pressed)   moved=move2048Up();
  if(btnDown.pressed) moved=move2048Down();
  if(moved){addRandom2048();if(!hasMovesLeft())game2048Over=true;}

  display.clearDisplay();
  for(int r=0;r<4;r++)for(int c=0;c<4;c++)draw2048Tile(r,c,board[r][c]);
  // Score сбоку
  display.setTextSize(1);display.setTextColor(WHITE);
  display.setCursor(64,14);display.print("SC");
  display.setCursor(64,24);display.print(score2048);
  display.setCursor(64,38);display.print("HI");
  display.setCursor(64,48);display.print(max(score2048,high2048));
  display.display();
}

// =============================================
// JUMP! (Платформер типа Марио)
// =============================================
#define JUMP_GRAVITY 1
#define JUMP_STRENGTH -8
#define JUMP_GROUND 54
#define JUMP_PLAYER_X 16

struct Platform{int x,y,w;bool active;};
struct Coin{int x,y;bool active,collected;};
struct Enemy{int x,y,vx;bool active,dead;};

struct JumpPlayer{
  int x,y,vx,vy;
  bool onGround,alive;
  int coins;
};

JumpPlayer jp;
Platform platforms[6];
Coin coins[5];
Enemy enemies[3];
int jumpScore=0,jumpHigh=0;
bool jumpOver=false;
int jumpCamX=0;
unsigned long jumpLastSpawn=0;
int jumpNextPlatX=0;

void spawnPlatform(int x,int y,int w){
  for(int i=0;i<6;i++){if(!platforms[i].active){platforms[i]={x,y,w,true};return;}}
}
void spawnCoin(int x,int y){
  for(int i=0;i<5;i++){if(!coins[i].active){coins[i]={x,y,true,false};return;}}
}
void spawnEnemy(int x,int y){
  for(int i=0;i<3;i++){if(!enemies[i].active){enemies[i]={x,y,-1,true,false};return;}}
}

void initJump(){
  jp={JUMP_PLAYER_X,JUMP_GROUND-12,0,0,true,true,0};
  for(int i=0;i<6;i++)platforms[i].active=false;
  for(int i=0;i<5;i++)coins[i].active=false;
  for(int i=0;i<3;i++)enemies[i].active=false;
  jumpScore=0;jumpOver=false;jumpCamX=0;jumpNextPlatX=60;
  rngState=millis()+222;
  // Начальные платформы
  spawnPlatform(40,42,20);spawnPlatform(80,35,18);spawnPlatform(110,28,16);
  spawnCoin(50,36);spawnCoin(88,29);
}

void drawJumpPlayer(int sx,int sy,bool alive){
  if(!alive)return;
  // Тело (Марио-стайл)
  display.fillRect(sx+2,sy,6,3,WHITE);  // Кепка
  display.fillRect(sx,sy+3,10,6,WHITE); // Тело
  display.fillRect(sx+1,sy+9,3,3,WHITE);// Нога л
  display.fillRect(sx+6,sy+9,3,3,WHITE);// Нога п
  display.fillRect(sx+7,sy+1,3,2,WHITE);// Козырёк
  display.fillRect(sx+3,sy+4,2,2,BLACK);// Глаз
  display.fillRect(sx+6,sy+4,1,1,BLACK);// Нос
  // Рука
  display.fillRect(sx+9,sy+5,2,2,WHITE);
}

void drawJumpEnemy(int sx,int sy){
  display.fillCircle(sx+4,sy+3,4,WHITE);
  display.fillRect(sx,sy+6,8,4,WHITE);
  display.fillCircle(sx+2,sy+2,1,BLACK);
  display.fillCircle(sx+6,sy+2,1,BLACK);
  display.fillTriangle(sx,sy+10,sx+4,sy+7,sx+8,sy+10,BLACK);
}

void updateJump(){
  if(btnBack.pressed){if(jumpScore>jumpHigh)jumpHigh=jumpScore;currentState=STATE_MENU;return;}
  if(jumpOver){
    if(btnSelect.pressed){if(jumpScore>jumpHigh)jumpHigh=jumpScore;initJump();}
    display.clearDisplay();display.drawRoundRect(0,0,128,64,4,WHITE);
    display.setTextSize(2);display.setCursor(8,6);display.print("Game Over");
    display.drawLine(0,24,128,24,WHITE);display.setTextSize(1);
    display.setCursor(4,30);display.print("SC");display.setCursor(4,40);display.print(jumpScore);
    display.setCursor(90,30);display.print("HI");display.setCursor(90,40);display.print(max(jumpScore,jumpHigh));
    display.display();return;
  }

  // Управление
  int speed=(selectedDiff==DIFF_EASY?2:selectedDiff==DIFF_HARD?4:3);
  if(btnLeft.held) jp.vx=-speed;
  else if(btnRight.held)jp.vx=speed;
  else jp.vx=0;

  if((btnUp.pressed||btnSelect.pressed)&&jp.onGround){
    jp.vy=JUMP_STRENGTH-(selectedDiff==DIFF_HARD?1:0);jp.onGround=false;
  }

  // Физика
  jp.vy+=JUMP_GRAVITY;
  jp.x+=jp.vx;jp.y+=jp.vy;

  // Камера
  if(jp.x>JUMP_PLAYER_X+20)jumpCamX=jp.x-JUMP_PLAYER_X-20;

  // Земля
  jp.onGround=false;
  if(jp.y>=JUMP_GROUND-12){jp.y=JUMP_GROUND-12;jp.vy=0;jp.onGround=true;}

  // Платформы
  for(int i=0;i<6;i++){
    if(!platforms[i].active)continue;
    int px=platforms[i].x-jumpCamX;
    if(jp.vy>=0&&jp.y+12<=platforms[i].y+3&&jp.y+12+jp.vy>=platforms[i].y&&
       jp.x+8>px&&jp.x<px+platforms[i].w){
      jp.y=platforms[i].y-12;jp.vy=0;jp.onGround=true;
    }
    if(px<-20)platforms[i].active=false;
  }

  // Монеты
  for(int i=0;i<5;i++){
    if(!coins[i].active||coins[i].collected)continue;
    int cx=coins[i].x-jumpCamX;
    if(abs(jp.x+5-cx)<8&&abs(jp.y+6-coins[i].y)<8){coins[i].collected=true;jumpScore++;jp.coins++;}
    if(cx<-10)coins[i].active=false;
  }

  // Враги
  for(int i=0;i<3;i++){
    if(!enemies[i].active)continue;
    enemies[i].x+=enemies[i].vx;
    int ex=enemies[i].x-jumpCamX;
    if(ex<-10){enemies[i].active=false;continue;}
    if(!enemies[i].dead){
      // Прыгнуть на врага сверху
      if(jp.x+8>ex&&jp.x<ex+8&&jp.y+12>=enemies[i].y&&jp.y+12<=enemies[i].y+4&&jp.vy>0){
        enemies[i].dead=true;jp.vy=-5;jumpScore+=3;
      } else if(jp.x+8>ex+1&&jp.x<ex+7&&jp.y+6>enemies[i].y&&jp.y<enemies[i].y+8){
        jumpOver=true;return;
      }
    }
  }

  // Спавн новых платформ
  if(jp.x+jumpCamX>jumpNextPlatX-60){
    int py=20+myRand()%25;int pw=14+myRand()%16;
    spawnPlatform(jumpNextPlatX,py,pw);
    if(myRand()%3!=0)spawnCoin(jumpNextPlatX+pw/2,py-8);
    if(jumpScore>3&&myRand()%4==0)spawnEnemy(jumpNextPlatX,py-10);
    jumpNextPlatX+=40+myRand()%30;
    jumpScore++;
  }

  // Упал вниз
  if(jp.y>70)jumpOver=true;

  // Рисуем
  display.clearDisplay();

  // Фон — горы пикселями
  for(int mx=0;mx<128;mx+=20){
    int mox=(mx-(jumpCamX/3)%20+200)%128;
    display.drawLine(mox,50,mox+10,35,WHITE);display.drawLine(mox+10,35,mox+20,50,WHITE);
  }

  // Земля
  display.drawLine(0,JUMP_GROUND,128,JUMP_GROUND,WHITE);
  int goff=(jumpCamX/2)%8;
  for(int gx=0;gx<128;gx+=8)display.drawPixel((gx+goff)%128,JUMP_GROUND+1,WHITE);

  // Платформы
  for(int i=0;i<6;i++){
    if(!platforms[i].active)continue;
    int px=platforms[i].x-jumpCamX;
    display.fillRect(px,platforms[i].y,platforms[i].w,3,WHITE);
    // Кирпичный узор
    for(int bx=px;bx<px+platforms[i].w;bx+=6)display.drawLine(bx,platforms[i].y,bx,platforms[i].y+2,BLACK);
  }

  // Монеты
  for(int i=0;i<5;i++){
    if(!coins[i].active||coins[i].collected)continue;
    int cx=coins[i].x-jumpCamX;
    if((millis()/200)%2==0)display.fillCircle(cx,coins[i].y,3,WHITE);
    else display.drawCircle(cx,coins[i].y,3,WHITE);
  }

  // Враги
  for(int i=0;i<3;i++){
    if(!enemies[i].active)continue;
    int ex=enemies[i].x-jumpCamX;
    if(!enemies[i].dead)drawJumpEnemy(ex,enemies[i].y);
    else{display.fillRect(ex,enemies[i].y+6,8,2,WHITE);}// Плоский враг
  }

  // Игрок
  drawJumpPlayer(jp.x-jumpCamX,jp.y,jp.alive);

  // HUD
  display.setTextSize(1);display.setTextColor(WHITE);
  display.fillRect(0,0,128,10,BLACK);
  display.setCursor(2,1);display.print("SC:");display.print(jumpScore);
  display.setCursor(70,1);display.print("HI:");display.print(max(jumpScore,jumpHigh));
  display.setCursor(105,1);
  for(int c=0;c<min(jp.coins,3);c++)display.print("o");

  display.display();
}

// =============================================
// DINO RUN
// =============================================
struct DinoPlayer{int x,y,velY;bool onGround,ducking;};
struct Cactus{int x,y;bool active;int type;};
DinoPlayer dino;Cactus cacti[4];
int dinoGroundY=54,dinoScore=0,dinoHigh=0,dinoSpeed=3;bool dinoGameOver=false;
unsigned long dinoLastSpawn=0,dinoLastSpeed=0;
int birdAnimFrame=0;unsigned long birdLastAnim=0;
const int BIRD_HEIGHTS[]={18,28,36};
struct Cloud{int x,y;bool active;};Cloud clouds[3];unsigned long dinoLastCloud=0;

void spawnCloud(){for(int i=0;i<3;i++){if(!clouds[i].active){clouds[i]={SCREEN_WIDTH+5,(int)(4+myRand()%14),true};return;}}}
void drawCloud(int x,int y){display.fillCircle(x+4,y+3,4,WHITE);display.fillCircle(x+9,y+1,5,WHITE);display.fillCircle(x+14,y+3,4,WHITE);display.fillRect(x+4,y+3,11,4,WHITE);}

void initDino(){
  dino={10,dinoGroundY-16,0,true,false};
  for(int i=0;i<4;i++){cacti[i].x=SCREEN_WIDTH+20+i*50;cacti[i].y=dinoGroundY-20;cacti[i].active=false;cacti[i].type=0;}
  for(int i=0;i<3;i++)clouds[i]={0,0,false};
  dinoScore=0;dinoSpeed=(selectedDiff==DIFF_EASY?2:selectedDiff==DIFF_HARD?5:3);
  dinoGameOver=false;dinoLastSpawn=dinoLastSpeed=dinoLastCloud=millis();rngState=millis();
}
void spawnObstacleDino(){
  for(int i=0;i<4;i++){if(!cacti[i].active){bool b=(dinoScore>5)&&(myRand()%4==0);int t=b?1:0;cacti[i].x=SCREEN_WIDTH+5;cacti[i].y=(t==1)?BIRD_HEIGHTS[myRand()%3]:dinoGroundY-20;cacti[i].active=true;cacti[i].type=t;return;}}
}
void drawBirdDino(int x,int y,int frame){
  display.fillEllipse(x+6,y+3,6,3,WHITE);display.fillCircle(x+11,y+1,3,WHITE);
  display.fillTriangle(x+13,y,x+16,y+1,x+13,y+2,WHITE);display.drawPixel(x+12,y,BLACK);
  if(frame%2==0){display.fillTriangle(x+3,y+3,x+8,y+3,x+5,y-2,WHITE);display.fillTriangle(x+8,y+3,x+12,y+3,x+9,y-1,WHITE);}
  else{display.fillTriangle(x+3,y+3,x+8,y+3,x+5,y+7,WHITE);display.fillTriangle(x+8,y+3,x+12,y+3,x+9,y+6,WHITE);}
  display.fillTriangle(x,y+2,x+4,y+2,x,y+5,WHITE);
}
bool dinoCollision(){
  for(int i=0;i<4;i++){if(!cacti[i].active)continue;
    if(cacti[i].type==0){if(dino.x+9>cacti[i].x+2&&dino.x+1<cacti[i].x+10&&dino.y+14>dinoGroundY-20)return true;}
    else{int bY=cacti[i].y;int dT=dino.ducking?dino.y+8:dino.y;if(dino.x+10>cacti[i].x+1&&dino.x<cacti[i].x+15&&dino.y+16>bY&&dT<bY+7)return true;}
  }return false;
}
void drawDinoPlayer(int x,int y,bool ducking){
  if(ducking){display.fillRect(x,y+6,14,8,WHITE);display.fillRect(x+8,y+4,6,5,WHITE);display.fillRect(x+13,y+5,2,2,WHITE);display.fillRect(x+12,y+4,2,2,BLACK);unsigned long t=millis()/100;if(t%2==0){display.fillRect(x+2,y+14,3,2,WHITE);display.fillRect(x+8,y+14,3,2,WHITE);}else{display.fillRect(x+1,y+14,3,2,WHITE);display.fillRect(x+7,y+14,3,2,WHITE);}}
  else{display.fillRect(x+5,y,7,6,WHITE);display.fillRect(x+11,y+1,2,2,WHITE);display.fillRect(x+2,y+4,10,8,WHITE);display.fillRect(x,y+5,3,4,WHITE);display.fillRect(x+10,y+1,2,2,BLACK);unsigned long t=millis()/120;if(!dino.onGround){display.fillRect(x+3,y+12,3,3,WHITE);display.fillRect(x+7,y+12,3,3,WHITE);}else if(t%2==0){display.fillRect(x+3,y+12,3,4,WHITE);display.fillRect(x+7,y+12,3,3,WHITE);}else{display.fillRect(x+3,y+12,3,3,WHITE);display.fillRect(x+7,y+12,3,4,WHITE);}}
}
void drawCactus(int x){display.fillRect(x+3,dinoGroundY-20,4,20,WHITE);display.fillRect(x,dinoGroundY-16,3,2,WHITE);display.fillRect(x,dinoGroundY-20,2,6,WHITE);display.fillRect(x+7,dinoGroundY-14,4,2,WHITE);display.fillRect(x+9,dinoGroundY-18,2,6,WHITE);}
void drawDinoGround(){display.drawLine(0,dinoGroundY,128,dinoGroundY,WHITE);int off=(millis()/60)%16;for(int i=0;i<128;i+=16){display.drawPixel((i+off)%128,dinoGroundY+2,WHITE);display.drawPixel((i+off+8)%128,dinoGroundY+4,WHITE);}}
void updateDino(){
  if(btnBack.pressed){if(dinoScore>dinoHigh)dinoHigh=dinoScore;currentState=STATE_MENU;return;}
  if(millis()-birdLastAnim>200){birdAnimFrame++;birdLastAnim=millis();}
  if(dinoGameOver){
    if(btnSelect.pressed||btnUp.pressed){if(dinoScore>dinoHigh)dinoHigh=dinoScore;initDino();}
    display.clearDisplay();display.drawRoundRect(0,0,128,64,4,WHITE);display.setTextSize(2);display.setCursor(8,6);display.print("Game Over");display.drawLine(0,24,128,24,WHITE);display.setTextSize(1);display.drawRect(56,28,16,10,WHITE);display.drawLine(60,38,68,38,WHITE);display.drawLine(58,40,70,40,WHITE);display.fillRect(61,38,6,2,WHITE);display.setCursor(4,30);display.print("SC");display.setCursor(4,40);display.print(dinoScore);display.setCursor(90,30);display.print("HI");display.setCursor(90,40);display.print(max(dinoScore,dinoHigh));display.setCursor(42,54);display.print(diffLabels[(int)selectedDiff]);display.display();return;
  }
  if((btnUp.pressed||btnSelect.pressed)&&dino.onGround&&!dino.ducking){dino.velY=-9;dino.onGround=false;}
  dino.ducking=(digitalRead(BTN_DOWN)==LOW)&&dino.onGround;
  dino.velY+=1;dino.y+=dino.velY;if(dino.y>=dinoGroundY-16){dino.y=dinoGroundY-16;dino.velY=0;dino.onGround=true;}
  unsigned long interval=max(400UL,1500UL-dinoSpeed*80UL)+myRand()%600;if(selectedDiff==DIFF_EASY)interval+=300;
  if(millis()-dinoLastSpawn>interval){spawnObstacleDino();dinoLastSpawn=millis();}
  if(millis()-dinoLastCloud>2000+(myRand()%2000)){spawnCloud();dinoLastCloud=millis();}
  for(int i=0;i<4;i++){if(!cacti[i].active)continue;cacti[i].x-=dinoSpeed;if(cacti[i].x<-20){cacti[i].active=false;dinoScore++;}}
  for(int i=0;i<3;i++){if(!clouds[i].active)continue;clouds[i].x-=1;if(clouds[i].x<-22)clouds[i].active=false;}
  int speedUp=(selectedDiff==DIFF_HARD?2000:3000),maxSpd=(selectedDiff==DIFF_EASY?7:12);
  if(millis()-dinoLastSpeed>(unsigned long)speedUp){if(dinoSpeed<maxSpd)dinoSpeed++;dinoLastSpeed=millis();}
  if(dinoCollision()){dinoGameOver=true;return;}
  display.clearDisplay();for(int i=0;i<3;i++)if(clouds[i].active)drawCloud(clouds[i].x,clouds[i].y);drawDinoGround();drawDinoPlayer(dino.x,dino.y,dino.ducking);for(int i=0;i<4;i++){if(!cacti[i].active)continue;if(cacti[i].type==0)drawCactus(cacti[i].x);else drawBirdDino(cacti[i].x,cacti[i].y,birdAnimFrame);}
  display.setTextSize(1);display.setTextColor(WHITE);display.setCursor(0,0);display.print(dinoScore);if(dinoHigh>0){display.setCursor(90,0);display.print("HI ");display.print(max(dinoScore,dinoHigh));}display.display();
}

// =============================================
// FLAPPY BIRD
// =============================================
struct FBird{float y,vel;};struct FPipe{int x,gapY;bool active,scored;};
FBird fbird;FPipe fpipes[3];
int flappyScore=0,flappyHigh=0,flappySpeed=2;bool flappyOver=false,flappyStarted=false;
unsigned long flappyLastPipe=0,flappyLastSpeed=0;
const int PIPE_WIDTH=10,BIRD_X=20;
int getGapSize(){if(selectedDiff==DIFF_EASY)return 28;if(selectedDiff==DIFF_HARD)return 16;return 22;}
void initFlappy(){fbird={32,0};for(int i=0;i<3;i++)fpipes[i]={SCREEN_WIDTH+5,0,false,false};flappyScore=0;flappySpeed=(selectedDiff==DIFF_HARD?3:2);flappyOver=false;flappyStarted=false;flappyLastPipe=flappyLastSpeed=millis();rngState=millis()+999;}
void spawnFPipe(){int gap=getGapSize(),minY=gap/2+4,maxY=SCREEN_HEIGHT-gap/2-6;for(int i=0;i<3;i++)if(!fpipes[i].active){fpipes[i]={SCREEN_WIDTH+5,(int)(minY+myRand()%(maxY-minY)),true,false};return;}}
bool flappyCollision(){int gap=getGapSize();if(fbird.y<=2||fbird.y>=SCREEN_HEIGHT-6)return true;for(int i=0;i<3;i++){if(!fpipes[i].active)continue;int by=(int)fbird.y;if(BIRD_X+5>fpipes[i].x&&BIRD_X<fpipes[i].x+PIPE_WIDTH)if(by-3<fpipes[i].gapY-gap/2||by+3>fpipes[i].gapY+gap/2)return true;}return false;}
void drawFlappyBird(int x,int y){display.fillCircle(x,y,4,WHITE);display.fillTriangle(x+3,y-1,x+7,y,x+3,y+1,WHITE);display.fillCircle(x+1,y-1,1,BLACK);if((millis()/100)%2==0)display.drawLine(x-2,y+2,x+2,y+5,WHITE);else display.drawLine(x-2,y+2,x+2,y-1,WHITE);}
void drawFPipe(int x,int gapY){int gap=getGapSize(),top=gapY-gap/2,bot=gapY+gap/2;display.fillRect(x,0,PIPE_WIDTH,top,WHITE);display.fillRect(x-2,top-4,PIPE_WIDTH+4,4,WHITE);display.fillRect(x,bot,PIPE_WIDTH,SCREEN_HEIGHT-bot,WHITE);display.fillRect(x-2,bot,PIPE_WIDTH+4,4,WHITE);display.drawLine(x+2,0,x+2,top-4,BLACK);display.drawLine(x+2,bot+4,x+2,SCREEN_HEIGHT,BLACK);}
void updateFlappy(){
  if(btnBack.pressed){if(flappyScore>flappyHigh)flappyHigh=flappyScore;currentState=STATE_MENU;return;}
  if(flappyOver){if(btnSelect.pressed||btnUp.pressed){if(flappyScore>flappyHigh)flappyHigh=flappyScore;initFlappy();}display.clearDisplay();display.drawRoundRect(0,0,128,64,4,WHITE);display.setTextSize(2);display.setCursor(8,6);display.print("Game Over");display.drawLine(0,24,128,24,WHITE);display.setTextSize(1);display.setCursor(4,30);display.print("SC");display.setCursor(4,40);display.print(flappyScore);display.setCursor(90,30);display.print("HI");display.setCursor(90,40);display.print(max(flappyScore,flappyHigh));display.display();return;}
  if(!flappyStarted){display.clearDisplay();display.drawRoundRect(0,0,128,64,4,WHITE);display.setTextSize(2);display.setCursor(10,10);display.print("Flappy");display.setCursor(18,30);display.print("Bird");drawFlappyBird(100,36);display.display();if(btnSelect.pressed||btnUp.pressed){flappyStarted=true;fbird.vel=-3;}return;}
  if(btnUp.pressed||btnSelect.pressed)fbird.vel=-4.0;
  float grav=(selectedDiff==DIFF_HARD)?0.6f:0.5f;fbird.vel+=grav;fbird.y+=fbird.vel;
  unsigned long pipeInt=(unsigned long)max(700,2200-flappySpeed*150);if(selectedDiff==DIFF_EASY)pipeInt+=400;
  if(millis()-flappyLastPipe>pipeInt){spawnFPipe();flappyLastPipe=millis();}
  for(int i=0;i<3;i++){if(!fpipes[i].active)continue;fpipes[i].x-=flappySpeed;if(!fpipes[i].scored&&fpipes[i].x+PIPE_WIDTH<BIRD_X){fpipes[i].scored=true;flappyScore++;}if(fpipes[i].x<-PIPE_WIDTH-4)fpipes[i].active=false;}
  if(millis()-flappyLastSpeed>5000){if(flappySpeed<6)flappySpeed++;flappyLastSpeed=millis();}
  if(flappyCollision()){flappyOver=true;return;}
  display.clearDisplay();for(int i=0;i<3;i++)if(fpipes[i].active)drawFPipe(fpipes[i].x,fpipes[i].gapY);drawFlappyBird(BIRD_X,(int)fbird.y);display.drawLine(0,SCREEN_HEIGHT-1,128,SCREEN_HEIGHT-1,WHITE);display.setTextSize(1);display.setTextColor(WHITE);display.setCursor(0,0);display.print(flappyScore);if(flappyHigh>0){display.setCursor(90,0);display.print("HI ");display.print(max(flappyScore,flappyHigh));}display.display();
}

// =============================================
// CARS
// =============================================
void drawPlayerCar(int x,int y){display.fillRect(x+2,y+2,12,8,WHITE);display.fillRect(x,y,12,8,BLACK);display.fillRect(x+1,y+2,12,5,WHITE);display.fillRect(x+3,y,8,3,WHITE);display.fillRect(x+4,y+1,3,2,BLACK);display.fillRect(x+8,y+1,2,2,BLACK);display.fillCircle(x+3,y+7,2,WHITE);display.fillCircle(x+11,y+7,2,WHITE);display.fillCircle(x+3,y+7,1,BLACK);display.fillCircle(x+11,y+7,1,BLACK);display.fillRect(x+13,y+2,1,2,WHITE);}
void drawObstacleCar(int x,int y,int type){if(type==0){display.fillRect(x,y+1,14,6,WHITE);display.fillRect(x+1,y,5,3,WHITE);display.fillRect(x+2,y+1,3,2,BLACK);}else if(type==1){display.fillRect(x+1,y+3,12,4,WHITE);display.fillRect(x+3,y+1,8,3,WHITE);display.fillRect(x+4,y+2,3,2,BLACK);display.fillRect(x+8,y+2,2,2,BLACK);}else{display.fillRect(x,y,14,7,WHITE);display.drawLine(x+5,y,x+5,y+7,BLACK);display.fillRect(x+1,y+1,3,2,BLACK);}display.fillCircle(x+3,y+7,2,WHITE);display.fillCircle(x+11,y+7,2,WHITE);display.fillCircle(x+3,y+7,1,BLACK);display.fillCircle(x+11,y+7,1,BLACK);}
struct ObstacleCar{int x,y,type,lane;bool active;};
const int LANE_Y[]={19,44};const int CAR_PLAYER_X=8;
int carLane=1,carY=LANE_Y[1],carTargetY=LANE_Y[1];bool carMoving=false;
ObstacleCar obstacles[4];int carsScore=0,carsHigh=0,carsSpeed=2;bool carsOver=false;
unsigned long carsLastSpawn=0,carsLastSpeed=0;int roadOffset=0;
void initCars(){carLane=1;carY=LANE_Y[1];carTargetY=LANE_Y[1];carMoving=false;for(int i=0;i<4;i++)obstacles[i].active=false;carsScore=0;carsSpeed=(selectedDiff==DIFF_EASY?2:selectedDiff==DIFF_HARD?4:3);carsOver=false;carsLastSpawn=carsLastSpeed=millis();roadOffset=0;rngState=millis()+777;}
void spawnObstacleCar(){for(int i=0;i<4;i++){if(!obstacles[i].active){int lane=myRand()%2;obstacles[i]={SCREEN_WIDTH+5,LANE_Y[lane],(int)(myRand()%3),lane,true};return;}}}
bool carsCollision(){for(int i=0;i<4;i++){if(!obstacles[i].active)continue;if(CAR_PLAYER_X+13>obstacles[i].x+1&&CAR_PLAYER_X+1<obstacles[i].x+13&&carY+7>obstacles[i].y+1&&carY+1<obstacles[i].y+7)return true;}return false;}
void drawRoad(int offset){display.fillRect(0,15,128,1,WHITE);display.fillRect(0,62,128,1,WHITE);for(int x=0;x<128;x+=20){int ox=(x-offset%20+20)%128;display.drawLine(ox,38,ox+10,38,WHITE);}for(int x=0;x<128;x+=6){int gx=(x+offset/2)%128;display.drawLine(gx,13,gx+2,11,WHITE);display.drawLine((gx+64)%128,63,(gx+66)%128,61,WHITE);}}
void updateCars(){
  if(btnBack.pressed){if(carsScore>carsHigh)carsHigh=carsScore;currentState=STATE_MENU;return;}
  if(carsOver){if(btnSelect.pressed||btnUp.pressed||btnDown.pressed){if(carsScore>carsHigh)carsHigh=carsScore;initCars();}display.clearDisplay();display.drawRoundRect(0,0,128,64,4,WHITE);display.setTextSize(2);display.setCursor(8,6);display.print("Game Over");display.drawLine(0,24,128,24,WHITE);display.setTextSize(1);display.setCursor(4,30);display.print("SC");display.setCursor(4,40);display.print(carsScore);display.setCursor(90,30);display.print("HI");display.setCursor(90,40);display.print(max(carsScore,carsHigh));display.display();return;}
  if(btnUp.pressed&&carLane==1){carLane=0;carTargetY=LANE_Y[0];carMoving=true;}if(btnDown.pressed&&carLane==0){carLane=1;carTargetY=LANE_Y[1];carMoving=true;}
  if(carMoving){if(carY<carTargetY){carY+=3;if(carY>=carTargetY){carY=carTargetY;carMoving=false;}}if(carY>carTargetY){carY-=3;if(carY<=carTargetY){carY=carTargetY;carMoving=false;}}}
  roadOffset+=carsSpeed;unsigned long spawnInt=max(500UL,1600UL-carsSpeed*150UL)+myRand()%400;if(selectedDiff==DIFF_EASY)spawnInt+=400;if(millis()-carsLastSpawn>spawnInt){spawnObstacleCar();carsLastSpawn=millis();}
  for(int i=0;i<4;i++){if(!obstacles[i].active)continue;obstacles[i].x-=carsSpeed+1;if(obstacles[i].x<-15){obstacles[i].active=false;carsScore++;}}
  int speedUp=(selectedDiff==DIFF_HARD?2500:4000),maxSpd=(selectedDiff==DIFF_EASY?6:10);if(millis()-carsLastSpeed>(unsigned long)speedUp){if(carsSpeed<maxSpd)carsSpeed++;carsLastSpeed=millis();}
  if(carsCollision()){carsOver=true;return;}
  display.clearDisplay();drawRoad(roadOffset);drawPlayerCar(CAR_PLAYER_X,carY);for(int i=0;i<4;i++)if(obstacles[i].active)drawObstacleCar(obstacles[i].x,obstacles[i].y,obstacles[i].type);display.setTextSize(1);display.setTextColor(WHITE);display.setCursor(0,0);display.print(carsScore);if(carsHigh>0){display.setCursor(90,0);display.print("HI ");display.print(max(carsScore,carsHigh));}display.display();
}

// =============================================
// SETUP & LOOP
// =============================================
unsigned long frameTimer=0;

void setup(){
  Serial.begin(115200);
  pinMode(BTN_UP,INPUT_PULLUP);    pinMode(BTN_DOWN,INPUT_PULLUP);
  pinMode(BTN_SELECT,INPUT_PULLUP);pinMode(BTN_BACK,INPUT_PULLUP);
  pinMode(BTN_LEFT,INPUT_PULLUP);  pinMode(BTN_RIGHT,INPUT_PULLUP);
  Wire.begin(SDA_PIN,SCL_PIN);
  if(!display.begin(SSD1306_SWITCHCAPVCC,SCREEN_ADDRESS)){Serial.println("OLED!");while(true);}
  display.clearDisplay();display.setTextColor(WHITE);
  drawDolphinAnimation();
}

void loop(){
  if(millis()-frameTimer<30)return;
  frameTimer=millis();
  updateAllButtons();
  switch(currentState){
    case STATE_MENU:   updateMenu();       drawMenu();       break;
    case STATE_DIFF:   updateDifficulty(); drawDifficulty(); break;
    case STATE_ABOUT:  updateAbout();      drawAbout();      break;
    case STATE_DINO:   updateDino();   break;
    case STATE_FLAPPY: updateFlappy(); break;
    case STATE_CARS:   updateCars();   break;
    case STATE_BEER:   updateBeer();   break;
    case STATE_COSTI:  updateCosti();  break;
    case STATE_SNAKE:  updateSnake();  break;
    case STATE_2048:   update2048();   break;
    case STATE_JUMP:   updateJump();   break;
  }
}
