#include <Adafruit_NeoPixel.h>
#include <TimerThree.h>

#define LE1_DEFAULT_BRIGHTNESS       25

#define LED1_INTERRUPT_MODE_NONE     0
#define LED1_INTERRUPT_MODE_BLINK    1
#define LED1_INTERRUPT_MODE_NEUTRAL  2
#define LED1_INTERRUPT_MODE_KITT     3

// TODO: Permitir establecer mediante comandos serie
// Leds Array Config
const uint8_t DEFAULT_LED_ARRAY_SIZE     = 12;
const uint8_t DEFAULT_LED_ARRAY_PIN      = 6;
const uint8_t DEFAULT_LED_BLINK_MILLIS   = 100;
const uint8_t DEFAULT_LED_NEUTRAL_MILLIS = 250;
const uint8_t DEFAULT_LED_KITT_MILLIS    = 100;
const uint8_t DEFAULT_LED_KITT_LEN       = 3;


// Adafruit leds controller - Necesario instanciar la variable? 
Adafruit_NeoPixel leds(DEFAULT_LED_ARRAY_SIZE, DEFAULT_LED_ARRAY_PIN);


// Leds1 General Vars
volatile uint8_t leds1Mode;
volatile uint8_t nLeds1Active;
volatile uint8_t lastNLeds1Active;


// Leds Blink Control
volatile unsigned long lastBlinkTime;
volatile uint8_t ledsBlinkState;

// Leds Neutral Control
volatile unsigned long lastNeutralTime;
volatile uint8_t ledsNeutralState;

// Leds Kitt Control
volatile unsigned long lastKittTime;
volatile int8_t ledsKittState;
volatile int8_t ledsKittDirection;



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
  
  // Inicializacion de Timers
  Timer3.initialize(1500);
  Timer3.attachInterrupt(interruptCallback);
  
  // Si ya se ha instanciado con estos valores ...
  leds.setPin(DEFAULT_LED_ARRAY_PIN);
  leds.setNumPixels(DEFAULT_LED_ARRAY_SIZE);
  leds.setBrightness(LE1_DEFAULT_BRIGHTNESS);
  clearLedArray();
  leds.show();
  
  // Leds Mode
  leds1Mode = LED1_INTERRUPT_MODE_NONE;
  
  // Leds1 General Vars
  nLeds1Active = lastNLeds1Active = ledsBlinkState = 0;
    
  //randomSeed(analogRead(0));
}


/*
 * Main Loop
 */
void loop() {

  // Read Command. No Block
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
  
  // Leds Number MODE
  if(strncmp(v,"L1N=",4) == 0) {
    leds1Mode = LED1_INTERRUPT_MODE_NONE;
    nLeds1Active = atoi(v+4);   
    
// 7 Segments     
//  } else if(strncmp(v,"SEG1=",5) == 0) {
//    parseSegValue(seg1, atoi(v+5));

  // Blink Mode
  } else if(strncmp(v,"L1BLINK=",8) == 0){
    if(atoi(v+8) == 1){
      leds1Mode = LED1_INTERRUPT_MODE_BLINK;
      lastBlinkTime = millis();
    }else{
      leds1Mode = LED1_INTERRUPT_MODE_NONE;      
    }
    
  // Neutral Mode
  } else if(strncmp(v,"L1NEUTRAL=",10) == 0){
    if(atoi(v+10) == 1){
      leds1Mode = LED1_INTERRUPT_MODE_NEUTRAL;
      lastNeutralTime = millis();
    }else{
      leds1Mode         = LED1_INTERRUPT_MODE_NONE;      
      nLeds1Active      = 0;
      lastNLeds1Active  = 1; // Diferente del ultimo para que lo trate la interrupcion. Mejorar.
    }
   
  // Kitt Mode
  } else if(strncmp(v,"L1KITT=",7) == 0){
    if(atoi(v+7) == 1){
      leds1Mode          = LED1_INTERRUPT_MODE_KITT;
      lastKittTime       = millis();
      ledsKittState      = -3;
      ledsKittDirection  = 1;
    }else{
      leds1Mode         = LED1_INTERRUPT_MODE_NONE;      
      nLeds1Active      = 0;
      lastNLeds1Active  = 1; // Diferente del ultimo para que lo trate la interrupcion. Mejorar.
    }
  // Default error
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
  uint8_t i;
  
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


void loadLedNeutralArray(uint8_t numLeds, uint8_t phase){
  uint8_t i;
  
  clearLedArray();
  for(i = 0; i < numLeds && i < DEFAULT_LED_ARRAY_SIZE; i++){
    if(i % 2 == phase){
      if(i < 4){
        leds.setPixelColor(i, '\x00', '\xff', '\x00');
      }else if(i < 8){
        leds.setPixelColor(i, '\xff', '\x00', '\x00');
      }else{
        leds.setPixelColor(i, '\x00', '\x00', '\xff');
      }
    }
  }  
}


void loadLedKittArray(uint8_t numLeds, int8_t phase, int8_t dir){
  uint8_t i;
  uint8_t nLed;

  clearLedArray();

  for (i = 0; i < DEFAULT_LED_KITT_LEN; i++){
    if(dir == 1)
      nLed = phase + i;
    else 
      nLed = phase - i;
      
    if(nLed >= 0 && nLed < DEFAULT_LED_ARRAY_SIZE){
      leds.setPixelColor(nLed, '\xff', '\x00', '\x00');        
    }
  }  
}



void clearLedArray(){
  int i;  
  for(i = 0; i < DEFAULT_LED_ARRAY_SIZE; i++)
     leds.setPixelColor(i, '\x00', '\x00', '\x00'); // setPixelColor(uint16_t n, uint32_t c), 
}



// Interrupcion de refresco (libreria Timers)

void interruptCallback(void){
  unsigned long currentTime = millis();
  
  // Leds Array Refresh
  if(leds1Mode == LED1_INTERRUPT_MODE_NONE && nLeds1Active != lastNLeds1Active){
    loadLedArray(nLeds1Active);
    leds.show();
    lastNLeds1Active = nLeds1Active;
  } 
  
  
  // Leds Blink
  if(leds1Mode == LED1_INTERRUPT_MODE_BLINK && abs(currentTime-lastBlinkTime) > DEFAULT_LED_BLINK_MILLIS){
    lastBlinkTime = currentTime;
    
    if(ledsBlinkState == 0){
       loadLedArray(nLeds1Active);  
       ledsBlinkState = 1;
    }else{
       loadLedArray(0);      
       ledsBlinkState = 0;
    }
//    cli();
    leds.show();
//    sei();
  }
  
  // Leds Neutral
  if(leds1Mode == LED1_INTERRUPT_MODE_NEUTRAL && abs(currentTime-lastNeutralTime) > DEFAULT_LED_NEUTRAL_MILLIS){
    lastNeutralTime = currentTime;

    loadLedNeutralArray(DEFAULT_LED_ARRAY_SIZE, ledsNeutralState);  
    
    if(ledsNeutralState == 0){
       ledsNeutralState = 1;
    }else{
       ledsNeutralState = 0;
    }
//    cli();
    leds.show();
//    sei();
  }
  
   // Leds Kitt
  if(leds1Mode == LED1_INTERRUPT_MODE_KITT && abs(currentTime-lastKittTime) > DEFAULT_LED_KITT_MILLIS && DEFAULT_LED_KITT_LEN < DEFAULT_LED_ARRAY_SIZE){
    lastKittTime = currentTime;
    
    if(ledsKittState <= -3 && ledsKittDirection == -1){
      ledsKittDirection = 1;
    } else if(ledsKittState >= (DEFAULT_LED_ARRAY_SIZE + DEFAULT_LED_KITT_LEN) && ledsKittDirection == 1) {
      ledsKittDirection = -1;      
    }
    
    loadLedKittArray(DEFAULT_LED_ARRAY_SIZE, ledsKittState, ledsKittDirection);
    ledsKittState = ledsKittState + ledsKittDirection;
    
    leds.show();
  }
}


