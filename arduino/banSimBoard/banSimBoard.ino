#define IO(pin,state) ((state) == 0) ? IO_L(pin) : IO_H(pin)
#define IO_L(pin)     ((pin)<8) ? PORTD &= ~(1<<(pin)) : PORTB &= ~(1<<((pin)-8))
#define IO_H(pin)     ((pin)<8) ? PORTD |=  (1<<(pin)) : PORTB |=  (1<<((pin)-8))

#include <Adafruit_NeoPixel.h>

/*
    0
   ---
 6| 5 |7
   ---
 1| 2 |4
   --- o 3
   
DATA: B01234567
*/


// 7 Segments Config
#define DATA 8
#define CLSH 9
#define CLST 10
#define CLR  11
#define LED  12
#define NCHAR 3
#define NLEVL 4
#define NSEGMENT 8




const uint8_t numeros[] = {
  B11101011,
  B00001001,
  B11100101,
  B10101101,
  B00001111,
  B10101110,
  B11101110,
  B10001001,
  B11101111,
  B10101111
};


// Leds Array Config
const uint8_t DEFAULT_LED_ARRAY_SIZE   = 12;
const uint8_t DEFAULT_LED_ARRAY_PIN    = 6;
const uint8_t DEFAULT_LED_BLINK_MILLIS = 100;


// Necesario instanciar la variable? 
Adafruit_NeoPixel leds(DEFAULT_LED_ARRAY_SIZE, DEFAULT_LED_ARRAY_PIN);
volatile uint8_t ledsA_Non;
volatile uint8_t lastLedsA_Non;
volatile uint8_t ledsBlink;
volatile unsigned long lastBlinkTime;
volatile uint8_t ledsBlinkState;

// Display data (must be read from RAM) (7 Segments)
volatile uint8_t seg1[NCHAR];
volatile uint8_t seg1_levl[NCHAR];

// Array para la lectura de comandos
char    cmd[16];
uint8_t cmdlen = 0;
uint8_t echo   = 0;

/* 
 * Main Setup
 */
void setup() {
  int i;
  
  // put your setup code here, to run once:
  Serial.begin(9600);
  
  // Si ya se ha instanciado con estos valores ...
  leds.setPin(DEFAULT_LED_ARRAY_PIN);
  leds.setNumPixels(DEFAULT_LED_ARRAY_SIZE);
  leds.setBrightness(50);
  clearLedArray();
  leds.show();
  ledsA_Non  = lastLedsA_Non = 0;
  ledsBlink = 0;
  ledsBlinkState = 0;

  
  
  
  // Pines de salida
  pinMode(DATA,OUTPUT);
  pinMode(CLSH,OUTPUT);
  pinMode(CLST,OUTPUT);
  pinMode(CLR,OUTPUT);
  pinMode(LED,OUTPUT);
  
  // Nivel bajo en todas las salidas
  IO(DATA,0);
  IO(CLSH,0);
  IO(CLST,0);
  IO(CLR,1);
  IO(LED,1);
  
  // Inicializo variables
  for(uint8_t i=0;i<NCHAR;i++) {
    seg1[i]=0xFF;
    seg1_levl[i]=NLEVL;
  }
  
  // Temp
  seg1[0] = numeros[8];
  seg1[1] = numeros[8];
  seg1[2] = numeros[8];
  
  
  //------------------------------------
  // Programacion de rutina de refresco
  //------------------------------------
  
  // Desactivamos las interrupciones
  cli();
  // Configurar el timer2 modo CTC
  // Interrupcion 'on Match'
  // CLK = 16 MHz
  // F = CLK/256 = 62500 Hz
  // CNT = 15
  // F/16 =  3906.25 Hz = 0,256 ms = T
  // NCHAR=9
  // NLEVL=4
  // T * NCHAR * NLEVL = 9,216 ms < 10 ms

  TCCR2B =  _BV(CS22) | _BV(CS21);
  TCCR2A = _BV(WGM21);
  OCR2A  = 15;
  TIMSK2 = _BV(OCIE2A);
  
  sei();
  
  randomSeed(analogRead(0));
  
}


/*
 * Main Loop
 */
void loop() {

  if(readCommand()){
    executeCommand();
  } 
}

/*
 * Reads serial port command
 */
boolean readCommand() {
  char c;
  while(Serial.available() > 0) {
    c = Serial.read();
    if(echo) {
      Serial.write(c);
      if(c == '\r') Serial.write('\n');
    }
    if(c == '\r') return true;
    cmd[cmdlen] = c;
    cmdlen++;
  }
  
  return false;
}

/*
 * Execute Command
 */
void executeCommand() {
  uint8_t error;
  cmd[cmdlen] = '\0';
  
  if(echo){
    Serial.write("Execute command : ");
    Serial.write(cmd);
    Serial.write('\n');
  }
  
  if(strcmp(cmd,"VERSION") == 0) {
    error = 0;
  }else if(strncmp(cmd,"SET ", 4) == 0) {
    error = cmdSet(cmd+4);
  } else if(strcmp(cmd,"HELP") == 0) {
    error = cmdHelp();
  } else {
    error = 1;
  }
  
  if(error == 0) {
    Serial.println("OK");
  } else {
    Serial.println("ERROR");
  }
  cmdlen = 0;
}


uint8_t cmdSet(char* v) {
  if(strncmp(v,"L1N=",4) == 0) {
    ledsA_Non = atoi(v+4);   
    ledsBlink = 0;
    //Serial.write("(LEDSANON) : ");
    //Serial.println(ledsA_Non);
  } else if(strncmp(v,"SEG1=",5) == 0){
    parseSegValue(seg1, atoi(v+5));
  } else if(strncmp(v,"BLINK=",6) == 0){
    ledsBlink = atoi(v+6);
  } else {
    return 1;
  }
  
  
  return 0;
}


/*
 * cmdHelp
 */
uint8_t cmdHelp() {
  Serial.println("BanSimBoard - Help");
  return 0;
}

//-------------------
// Led Array Funcs
//-------------------

void loadLedArray(uint8_t numLeds){
  int i;
  
  clearLedArray();
  for(i = 0; i < numLeds && i < DEFAULT_LED_ARRAY_SIZE; i++){
    if(i < 4){
      leds.setPixelColor(i, '\x00', '\xff', '\x00');
    }else if(i < 8){
      leds.setPixelColor(i, '\xff', '\x00', '\x00');
    }else{
      leds.setPixelColor(i, '\x00', '\x00', '\xff');
    }
  }  
}


void clearLedArray(){
  int i;  
  for(i = 0; i < DEFAULT_LED_ARRAY_SIZE; i++)
     leds.setPixelColor(i, '\x00', '\x00', '\x00'); // setPixelColor(uint16_t n, uint32_t c), 
}



//-------------------
// 7 Segments Funcs
//-------------------
void parseSegValue(volatile uint8_t *disp, int value){
  int c, r;
  
  r = value % 10;
  disp[0] = numeros[r];
  
  c = value / 10;
  r = c % 10;
  disp[1] = numeros[r];
  
  c = c / 10;
  r = c % 10;
  disp[2] = numeros[r];
}




// Variables para el refresco
uint8_t current_char = 0;
uint8_t current_levl = 0;

// Interrupcion de refresco
ISR(TIMER2_COMPA_vect) {
  
  // Leds Array Refresh
  if(ledsA_Non != lastLedsA_Non){
    lastLedsA_Non = ledsA_Non;
    
    //Serial.println(ledsA_Non);
    //Serial.println(lastLedsA_Non);
    loadLedArray(ledsA_Non);
    leds.show();
    
    //lastLedsA_Non = ledsA_Non; // Aqui no funciona?? Sale de la interrupcion?
  }
  
  // Leds Blink
  if(ledsBlink && abs(millis()-lastBlinkTime) > DEFAULT_LED_BLINK_MILLIS){
    lastBlinkTime = millis();
    
    if(ledsBlinkState == 0){
       loadLedArray(ledsA_Non);  
       ledsBlinkState = 1;
    }else{
       loadLedArray(0);      
       ledsBlinkState = 0;
    }
    leds.show();
  }
   
   
   
  // 7 Segments Refresh
  IO(LED,1);
  
  // Seleccionamos el caracter correspondiente
  // Logica inversa: 0-on, 1-off
  for(uint8_t c=0; c < NCHAR; c++) {
    IO(DATA,!((c==current_char) && (current_levl <= seg1_levl[c])));
    IO(CLSH,1);
    IO(CLSH,0);
  }
  
  // Establecemos el valor de cada segmento
  // para el caracter actual
  uint8_t current_data = seg1[current_char];
  for(uint8_t s=0; s < NSEGMENT; s++) {
    IO(DATA,(1<<s) & current_data);
    IO(CLSH,1);
    IO(CLSH,0);
  }
  
  // Cargo los latches de salida
  IO(CLST,1);
  IO(CLST,0);
  
  // En el siguiente pulso enendere el siguiente caracter
  current_levl++;
  if(current_levl > NLEVL) {
    current_levl = 0;
    current_char = (current_char+1)%NCHAR;
  }
  
  IO(LED,0);
}


