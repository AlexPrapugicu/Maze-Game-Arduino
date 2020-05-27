#include <FastLED.h>
/*
  DEFINIT CHIPSET PENTRU A-L FOLOSI CU BIBLIOTECA FASTLED
  PIN 5 PWM PENTRU CONECTARE DATA IN
  DEFINITE MAX SI MIN BRIGHTNESS PENTRU FUCNTII DIN FASTLED
  CARE PORNESC LUMINOZITATEA SI AFISEAZA LEDURILE

  BRIGHTNESS PIN SE FOLOSESTE DOAR DACA VREI SA CONTROLEZI LUMINOZITATEA
  DINTR-UN POTENTIOMETRU

  PENTRU JOC ESTE LASAT STANDARD SI NU SE MODIFICA PRIN POTENTIOMETRU
*/
#define FAST_LED_CHIPSET WS2812B
#define FAST_LED_DATA_PIN 5
#define MAX_BRIGHTNESS 240  // 255 ESTE MAXIM DAR AM LIMITAT LA 240 LUMINOZITATEA
#define MIN_BRIGHTNESS 37
const int brightnessPIN = A0;

#define BLUEBTN 8  // BUTON INAINTE, PINUL 8
#define REDBTN 7   // BUTON INAPOI, PINUL 7
#define LFTBTN 6   // BUTON STANGA, PINUL 9
#define RGTBTN 4  // BUTON DREAPTA< PINUL 10


#define LINII 15
#define COLOANE 8
#define MAX_MAPS 3  // numar maxim de mape
/*
  Define for colors in hexa
*/
#define  PINK  0x1DEC12
#define  RED    0xFF0000
#define  BLUE   0x0000FF
#define  YELLOW 0xFFFF00
#define  CHOCOLATE  0xD2691E
#define  PURPLE 0x9966CC
#define  WHITE  0XFFFFFF
#define  AQUA   0x00FFFF
#define  HOTPINK 0xFF1493
#define  DARKORANGE 0xFF8C00
#define  BLACK 0x000000

//structura pentru mapa, care transpune matricile pe matricea de LED-uri
typedef struct MAP {
  uint8_t pixels[LINII][COLOANE];
  unsigned int long color[LINII][COLOANE];
} MAP;
MAP field;
MAP activeMap;


//structura pentru a definii labirinturile
typedef struct randomMap {
  byte pixels[LINII][COLOANE];
} RandomMap;

//structura pentru jucator
typedef struct Player {
  //pozitii player in harta
  byte xpos;//pe a cata linie
  byte ypos;//a cata coloana
  unsigned int color;//culoare led jucator
} Player;
Player myPlayer;
Player enemyPlayer;


//un jucator nou este pozitionat la o anumita pozitie in labirint, am ales pozitia prestabilita 1,1
void newPlayer(byte x, byte y) {
  myPlayer.xpos = x;
  myPlayer.ypos = y;
  myPlayer.color = DARKORANGE;
}

//define-uri pentru butoanele folosite
#define BTNUP 1
#define BTNDWN 2
#define BTNLFT 3
#define BTNRGT 4

byte currentControl = 0; // pentru a determina ce buton e apasat, initializat pe niciun buton apasat

//citim starile butoanelor, determinam ce buton s-a apasat
//butoane active pe low
void readInput()
{
  byte upST = digitalRead(BLUEBTN);
  byte downST = digitalRead(REDBTN);
  byte leftST = digitalRead(LFTBTN);
  byte rightST = digitalRead(RGTBTN);
  if (upST == LOW) {
    currentControl = BTNUP;
    delay(300);
  }
  if (downST == LOW) {
    currentControl = BTNDWN;
    delay(300);
  }
  if (leftST == LOW) {
    currentControl = BTNLFT;
    delay(300);
  }
  if (rightST == LOW) {
    currentControl = BTNRGT;
    delay(300);
  }

}

// Functie folosita pentru testarea pozitiilor curente ale player-ului in labirint | Am folosit-o doar pentru testare.
void checkPos() {
  Serial.print(myPlayer.xpos);
  Serial.print(myPlayer.ypos);
}

//in functie de ce buton se apasa, se apeleaza functia corespunzatoare care determina miscarea player-ului
void controlPlayer()
{
  switch (currentControl)
  {
    case BTNUP: moveUp(); checkPos(); break;
    case BTNDWN: moveDown(); checkPos(); break;
    case BTNLFT: moveLeft(); checkPos(); break;
    case BTNRGT: moveRight(); checkPos(); break;
  }
}

//Urmeaza functiile de miscare a player-ului si testarea de coliziuni

void moveUp() {

  myPlayer.ypos++;
  if (checkWallCollision(myPlayer.xpos, myPlayer.ypos)) {
    myPlayer.ypos--;
  }

  if (checkOutOfMap(myPlayer.xpos, myPlayer.ypos)) {
    myPlayer.ypos--;
  }
}

void moveDown() {

  myPlayer.ypos--;
  if (checkWallCollision(myPlayer.xpos, myPlayer.ypos)) {
    myPlayer.ypos++;
  }

  if (checkOutOfMap(myPlayer.xpos, myPlayer.ypos)) {
    myPlayer.ypos++;
  }
}

void moveLeft() {
  myPlayer.xpos--;
  if (checkWallCollision(myPlayer.xpos, myPlayer.ypos)) {
    myPlayer.xpos++;
  }
  if (checkOutOfMap(myPlayer.xpos, myPlayer.ypos)) {
    myPlayer.xpos++;
  }
}


void moveRight() {
  myPlayer.xpos++;
  if (checkWallCollision(myPlayer.xpos, myPlayer.ypos)) {
    myPlayer.xpos--;
  }
  if (checkOutOfMap(myPlayer.xpos, myPlayer.ypos)) {
    myPlayer.xpos--;
  }
}



bool checkWallCollision(byte x, byte y) {
  //daca coordonatele curente reprezinta zid, mutarea nu e permisa
  if (activeMap.pixels[x][y] == 1) {
    return true;
  }
  else {
    return false;
  }
}
bool checkPlayerCollision(byte x, byte y) {
   //daca coordonatele curente sunt egale cu cele ale enemy-ului atunci mutarea nu este permisa
  if (x == myPlayer.xpos && y == myPlayer.ypos) {
    return true;
  }
  else {
    return false;
  }
}

bool checkOutOfMap(byte x, byte y) {
  //daca coordonatele curente sunt in afara hartii de joc, mutarea nu e permisa
  if (x < 0 || x >= LINII || y < 0 || y >= COLOANE) {
    return true;
  }
  else {
    return false;
  }
}

//definirea a trei mape de labirint
/*
   '1': Wall
   '0': Wove Zone
*/
bool visited[LINII][COLOANE];

RandomMap mapLib[3] = {
  {
    { {1, 1, 1, 1, 1, 1, 1, 1},
      {0, 0, 1, 0, 0, 0, 1, 1},
      {1, 0, 1, 0, 1, 0, 1, 1},
      {1, 0, 0, 0, 1, 0, 1, 1},
      {1, 0, 0, 1, 1, 0, 0, 0},
      {1, 1, 1, 0, 0, 1, 1, 0},
      {1, 0, 0, 0, 0, 0, 0, 0},
      {1, 0, 1, 1, 1, 1, 1, 1},
      {1, 0, 0, 0, 0, 0, 0, 1},
      {1, 1, 1, 1, 1, 0, 0, 1},
      {1, 0, 0, 0, 0, 0, 1, 1},
      {1, 0, 1, 1, 1, 1, 1, 1},
      {1, 0, 0, 0, 0, 0, 1, 1},
      {1, 0, 0, 1, 1, 0, 0, 0},
      {1, 1, 1, 1, 1, 1, 1, 0}
    }
  },
  {
    { {1, 1, 1, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 1, 1, 1, 0},
      {0, 0, 0, 1, 0, 0, 0, 0},
      {0, 0, 1, 1, 0, 1, 1, 1},
      {0, 0, 0, 0, 0, 1, 0, 0},
      {0, 0, 0, 0, 1, 0, 0, 0},
      {1, 1, 1, 0, 0, 0, 1, 0},
      {1, 1, 1, 1, 1, 1, 1, 0},
      {1, 0, 0, 0, 0, 0, 0, 0},
      {1, 0, 0, 0, 0, 1, 1, 1},
      {1, 1, 0, 1, 1, 1, 1, 1},
      {1, 0, 0, 1, 1, 0, 0, 0},
      {1, 0, 0, 0, 0, 0, 1, 0},
      {1, 0, 1, 1, 1, 0, 1, 0},
      {1, 1, 1, 1, 1, 1, 1, 0}
    }
  },
  {
    { {1, 0, 0, 1, 1, 1, 1, 1},
      {0, 0, 0, 0, 0, 0, 0, 1},
      {1, 1, 0, 1, 1, 0, 0, 0},
      {0, 0, 0, 1, 1, 0, 1, 0},
      {0, 0, 0, 0, 0, 0, 1, 0},
      {1, 1, 1, 1, 1, 1, 1, 0},
      {1, 0, 0, 0, 0, 0, 0, 0},
      {1, 0, 0, 0, 0, 1, 1, 1},
      {1, 1, 0, 0, 1, 0, 0, 0},
      {1, 1, 0, 0, 0, 0, 1, 0},
      {1, 0, 0, 0, 0, 0, 0, 0},
      {1, 0, 1, 1, 1, 1, 1, 1},
      {1, 0, 0, 0, 0, 0, 1, 1},
      {1, 0, 0, 1, 0, 0, 0, 1},
      {1, 1, 1, 1, 0, 0, 0, 0}
    }
  }
};

// variabila folosita pentru determinarea mapei selectate curent
byte selectedMap = 0;

//punem mapa pe led-uri
void newActiveMap()
{
  uint8_t x, y;
  for (y = 0; y < COLOANE; y++)
  {
    for (x = 0; x < LINII; x++)
    {
      activeMap.pixels[x][y] = (mapLib[selectedMap]).pixels[x][y];
    }
  }
}


//stinge toate led-urile
void clearField()
{
  uint8_t x, y;
  for (y = 0; y < COLOANE; y++)
  {
    for (x = 0; x < LINII; x++)
    {
      field.pixels[y][x] = 0;
      field.color[y][x] = BLACK;
    }
  }
}
#define PIXELS LINII*COLOANE //numarul total de led-uri

CRGB leds[PIXELS];  //led-urile

// initializeaza ledurile cu ajutorul FASTLED library
void initPixels()
{
  FastLED.addLeds<FAST_LED_CHIPSET, FAST_LED_DATA_PIN, GRB>(leds, PIXELS);
}


// Initializeaza jocul - curata mapa, creeaza una noua si creeaza player.
void initGame() {
  clearField();
  newActiveMap();
  newPlayer(1,1);
  initEnemy(13, 4);
}

// Aprinde ledurile cu o valoarea intre minimul si maximul pentru luminozitate
void showPixels()
{
  int value = map(analogRead(brightnessPIN), 0, 1023, 0, MAX_BRIGHTNESS);
  FastLED.setBrightness(constrain(value, MIN_BRIGHTNESS, MAX_BRIGHTNESS));
  FastLED.show();
}


int long getPixel(int pos)
{
  // impacheteaza culorile RGB intr-un long , si ia culoarea de pe ledul curent
  return (leds[pos].red << 16) + (leds[pos].green << 8) + leds[pos].b;
}

//stinge treptat fiecare canal de RGB cu o anumita valoarea dimval
void dim(float dimval)
{
  for (uint8_t i = 0; i < (LINII * COLOANE); i++)
  {
    int currentColor = getPixel(i);
    // aiic se despacheteaza
    int long red = ((currentColor & 0xFF0000) >> 16);
    int green = ((currentColor & 0x00FF00) >> 8);
    int blue = (currentColor & 0x0000FF);

    // pentru fiecare canal de culoare, acea culoare se inmulteste cu coeficientul dat ca parametru functiei, un float intre 0 si 1
    // rezulta prin inmultire repetata efectul de dimming intrucat se micsoreaza intensitatea de pe acel canal de culoare pana ajunge la R=0 G=0 B=0
    red = red * dimval;
    green = green * dimval;
    blue = blue * dimval;

    currentColor = (red << 16) + (green << 8) + blue;
    setPixel(i, currentColor);
  }
}

//fade de final labirint, se aplica pe fiecare led, functia dim prezentata mai sus
void endFade() {
  byte i;
  for (i = 0; i < LINII * COLOANE; i++) {
    dim(0.3);
    showPixels();
    delay(100);
  }
}

//functia care verifica daca player-ul a ajuns la coordonatele de finish
bool endGame() {
  byte x, y;
  if (myPlayer.xpos == (LINII - 1)  && myPlayer.ypos == (COLOANE - 1)) {
    endFade();
    return true;
  }
  return false;
}

//functia de printare a mapei si a player-ului
void Field()
{
  byte x, y;
  for (y = 0; y < COLOANE; y++)
  {
    for (x = 0; x < LINII; x++)
    {

      if (activeMap.pixels[x][y] == 0) {
        setFieldPixel(x, y, BLACK);//move zone=led stins

      }
      else if (activeMap.pixels[x][y] == 1) {
        setFieldPixel(x, y, PURPLE);//perete=led aprins
      }
    }
  }
  setFieldPixel(myPlayer.xpos, myPlayer.ypos, myPlayer.color);//printeaza player-ul la pozitia curenta in harta
  setFieldPixel(enemyPlayer.xpos, enemyPlayer.ypos, enemyPlayer.color);//printeaza enemy player-ul la pozitia curenta in harta
  showPixels();
}

#define NUM_LEDS (COLOANE * LINII)
#define LAST_VISIBLE_LED 119

//functia care ne returneaza numarul led-ului (exemplu:XY(5,4) led-ul 37 de pe banda de led-uri)
byte XY (byte x, byte y) {
  // Daca se depaseste pozitia maxima de leduri posibile se returneaza ultimul led
  if ( (x >= COLOANE) || (y >= LINII) ) {
    return (LAST_VISIBLE_LED + 1);
  }
  // tabela cu numerele ledurilor de la 0-119
  const uint8_t XYTable[] = {
    0,   1,   2,   3,   4,   5,   6,   7,
    15,  14,  13,  12,  11,  10,   9,   8,
    16,  17,  18,  19,  20,  21,  22,  23,
    31,  30,  29,  28,  27,  26,  25,  24,
    32,  33,  34,  35,  36,  37,  38,  39,
    47,  46,  45,  44,  43,  42,  41,  40,
    48,  49,  50,  51,  52,  53,  54,  55,
    63,  62,  61,  60,  59,  58,  57,  56,
    64,  65,  66,  67,  68,  69,  70,  71,
    79,  78,  77,  76,  75,  74,  73,  72,
    80,  81,  82,  83,  84,  85,  86,  87,
    95,  94,  93,  92,  91,  90,  89,  88,
    96,  97,  98,  99, 100, 101, 102, 103,
    111, 110, 109, 108, 107, 106, 105, 104,
    112, 113, 114, 115, 116, 117, 118, 119
  };

  byte i = (y * COLOANE) + x;
  byte j = XYTable[i];
  return j;
}

//seteaza un led de la o anumita pozitie, cu o anumita culoare cu ajutorul tabelului de mai sus pentru pozitie si a functiei setPixel de mai jos
void setFieldPixel(int x, int y, int long color)
{
  byte  pos = XY(y, x);
  setPixel(pos, color);
}

void setPixel(int pos, int long color)
{
  leds[pos] = CRGB(color);
}

//functia care citeste butoanele si selecteaza mapa de joc
void mapSelect() {
  clearField();
  bool mapNotSelected = true;
  while (mapNotSelected) {//cat timp mapa nu e selectata se poate alege in continuare mapa pe care se vrea a se juca
    delay(200);
    Field();
    byte blueBtnState = digitalRead(BLUEBTN);
    if (blueBtnState == LOW) {
      selectedMap++;
      if (selectedMap > (MAX_MAPS - 1)) {//mapele merg circular, daca se ajunge la ultima mapa se revine la prima
        selectedMap = 0;
      }
      newActiveMap();
      delay(300);
    }
    byte redBtnState = digitalRead(REDBTN);//la apasarea butonului rosu s-a selectat mapa de joc
    if (redBtnState == LOW) {
      mapNotSelected = false;
    }
  }
  if (selectedMap == 0) {
    initEnemy(6, 2);
  }
  else if (selectedMap == 1) {
    initEnemy(8, 2);
  }
  else {
    initEnemy(10, 2);
  }

}

bool mazeRunning = false;
// variabile folosite pentru determinarea mutarii curente a enemy-ului
byte UP = 0, RIGHT = 1, DOWN = 2, LEFT = 3;
// in last command se retine ultima miscare a enemy-ului pentru a evita un loop intre "move up" si "move down" pe o singura pozitie
byte lastCommand = 0;
void runMaze() {
  initGame();//initializare joc
  mazeRunning = true;
  mapSelect();
  /*
     cat timp jocul ruleaza se tot citeste inputul pentru control player
     si se reprinteaza mapa si player-ul la noile pozitii
  */
  while (mazeRunning) {
    currentControl = 0;
    readInput();
    controlPlayer();
    // O data la 600 de milisecunde se apeleaza functia walkEnemy care muta pozitia enemy-ului
    EVERY_N_MILLISECONDS(600) {
      walkEnemy();
    }
    Field();
    if (endGame()) {
      mazeRunning = false;
    }
  }
}

/*
  Se verifica ultima commanda apoi se trece pe branch-ul potrivit, daca cumva nu se poate face mutarea - fie din cauza coliziunii cu zidul sau iesirea din mapa
  se va face un salt la celalalt if pentru a testa o alta posibila mutare
*/
void walkEnemy() {
  if (lastCommand == UP) {
WALKUP: if (!canWalkUp()) {
      goto WALKDOWN;
    }
  }

  if (lastCommand == DOWN) {
WALKDOWN: if (!canWalkDown()) {
      goto WALKUP;
    }
  }
}




/*
  Se face mutarea dupa un delay convenabil apoi se testeaza :
  Daca enemy atinge player-ul, player-ul revine la pozitia initiala si returneaza false

  Daca enemy atinge zid-ul acesta revine in pozitia de dinainte si returneaza false

  Daca enemy atinge o pozitie dinafara mapei, revine la pozitia precedenta si returneaza false

  In orice alt caz returneaza true si seteaza ultima comanda ca fiind cea executata
*/
bool canWalkDown() {
  delay(200);
  enemyPlayer.ypos--;
  if (checkPlayerCollision(enemyPlayer.xpos, enemyPlayer.ypos))
  {
    newPlayer(1,1);
  }
  if (checkWallCollision(enemyPlayer.xpos, enemyPlayer.ypos))
  {
    enemyPlayer.ypos++;
    return false;
  }
  if (checkOutOfMap(enemyPlayer.xpos, enemyPlayer.ypos))
  {
    enemyPlayer.ypos++;
    return false;
  }
  lastCommand = DOWN;
  return true;
}

// Asemenea functie canWalkUp() doar ca miscarea se face in directia opusa
bool canWalkUp() {
  delay(200);
  enemyPlayer.ypos++;
  if (checkPlayerCollision(enemyPlayer.xpos, enemyPlayer.ypos))
  {
    newPlayer(1,1);
  }
  if (checkWallCollision(enemyPlayer.xpos, enemyPlayer.ypos))
  {
    enemyPlayer.ypos--;
    return false;
  }
  if (checkOutOfMap(enemyPlayer.xpos, enemyPlayer.ypos))
  {
    enemyPlayer.ypos--;
    return false;
  }
  lastCommand = UP;
  return true;
}


// Initializeaza pozitia enemy-ului la xposition si yposition
// De asemenea seteaza si culoarea acestuia
void initEnemy(byte xposition, byte yposition) {
  enemyPlayer.xpos = xposition;
  enemyPlayer.ypos = yposition;
  enemyPlayer.color = PINK;
}



/*
   Animatie din fastled
   Se aprind ledurile unul dupa altul si lasa in urma
   un trace care se dim-uie
   gHue este saturatia care se modifica in Loop o data la 20 de milisecunde
*/
byte gHue = 0;
void RunningLeds()
{
  fadeToBlackBy( leds, 120, 20);
  int pos = beatsin16( 13, 0, 120 - 1 );
  leds[pos] += CHSV( gHue, 255, 192);
}

/*
   Setarea pinilor pentru butoane si initializarea benzii led
*/
void setup() {
  Serial.begin(9600);
  pinMode(BLUEBTN, INPUT_PULLUP);
  pinMode(REDBTN, INPUT_PULLUP);
  pinMode(LFTBTN, INPUT_PULLUP);
  pinMode(RGTBTN, INPUT_PULLUP);
  initPixels();
  showPixels();
}

void loop() {
  // aici se aprinde animatia led pana cand se apasa butonul albastru
  FastLED.show();
  FastLED.delay(1000 / 60);
  EVERY_N_MILLISECONDS( 20 ) {
    gHue = gHue + 5;
  };
  RunningLeds();
  byte blueBtnState = digitalRead(BLUEBTN);
  if (blueBtnState == LOW) {
    clearField();
    runMaze();
  }
}
