/*
 * BanSimBoard - Banshee 2016
 * 
 * Controlador sobre Arduino Pro Micro (Leonardo)
 * para control de BanSim
 * 
 * Componentes
 * 
 *  - Array Leds principal para rpms
 *  - Doble array de leds para flags
 *  - Tacometro
 *  - Botonera
 *  - Freno de mano
 */

#include <Adafruit_NeoPixel.h>
#include <TimerThree.h>
#include <TimerOne.h>

#define BANSIMBOARD_VERSION           0.7

// Input Modes
#define INPUT_MODE_ASCII              0
#define INPUT_MODE_BINARY             1

// Led1 Modes
#define LED1_INTERRUPT_MODE_NONE      0
#define LED1_INTERRUPT_MODE_BLINK     1
#define LED1_INTERRUPT_MODE_NEUTRAL   2
#define LED1_INTERRUPT_MODE_KIT       3

// Timers Default periods
#define REFRESH_TIMER_MICROSECS       1500L
#define TACHOMETER_TIMER_MICROSECS    6000L

// Led1 Config
#define DEFAULT_LED_ARRAY_PIN         6
#define DEFAULT_LED_ARRAY_SIZE        12
#define DEFAULT_LED_BLINK_MILLIS      100L
#define DEFAULT_LED_NEUTRAL_MILLIS    250L

// Tachometer Config
#define DEFAULT_TACHOMETER_ARRAY_PIN  5
#define TACHOMETER_MAX_RPMS           11000

// TODO: Permitir establecer mediante comandos serie

//////////////
// Constants 
//////////////

// Leds Array Config
const uint8_t LED_ARRAY_SIZE        = DEFAULT_LED_ARRAY_SIZE;
const uint8_t LED_ARRAY_PIN         = DEFAULT_LED_ARRAY_PIN;

// Tachometer config
const uint8_t TACHOMETER_ARRAY_PIN  = DEFAULT_TACHOMETER_ARRAY_PIN;

///////////////////
// Volatile Vars
///////////////////

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


////////////////
// Global Vars
////////////////

// Adafruit leds controller - Necesario instanciar la variable? 
Adafruit_NeoPixel leds(DEFAULT_LED_ARRAY_SIZE, DEFAULT_LED_ARRAY_PIN);

uint8_t inputMode = INPUT_MODE_ASCII;

// Array para la lectura de comandos ASCII
char    cmd[16];
uint8_t cmdlen = 0;
uint8_t echo   = 1;

// Array bytes para la lectura de comandos binarios
// TODO: ...


/* 
 * Main Setup
 */
void setup() {
  int i;
  
  // Serial Initialization
  Serial.begin(9600);
  
  // Generic set pin mode
  for(i= 2; i<=16; i++) {
    setPinMode(i, OUTPUT);
    digitalWrite(i, HIGH); // Set pull up resistor
  }
  
  ////////////////////////////
  // Inicializacion de Timers
  ////////////////////////////
  
  // Timer 1. Rutina de refresco
  Timer1.initialize(REFRESH_TIMER_MICROSECS);
  Timer1.attachInterrupt(refreshCallback);
  
  // Timer 3. Tacometro
  Timer3.initialize(TACHOMETER_TIMER_MICROSECS);
  Timer3.pwm(DEFAULT_TACHOMETER_ARRAY_PIN, 512);
  
  
  // Initializacion de array de leds
  leds.setPin(DEFAULT_LED_ARRAY_PIN);
  leds.setNumPixels(DEFAULT_LED_ARRAY_SIZE);
  leds.setBrightness(50);
  clearLedArray();
  leds.show();
  
  // Leds Mode
  leds1Mode = LED1_INTERRUPT_MODE_NONE;
  
  // Leds1 General Vars
  nLeds1Active = lastNLeds1Active = ledsBlinkState = 0;
  
}


/*
 * Main Loop
 */
void loop() {

  // Read Command. No Block
  if(inputMode == INPUT_MODE_ASCII) {
    if(readASCIICommand()){
      executeASCIICommand();
    }   
  } else {
    /*
    if(readBinaryCommand()){
      executeBinaryCommand();
    }
    */
  }
}

/*
 * Reads serial port ASCII command
 */
boolean readASCIICommand() {
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
 * Execute ASCII command
 */
void executeASCIICommand() {
  uint8_t error;
  cmd[cmdlen] = '\0';
  
  if(echo){
    Serial.write("Execute command : ");
    Serial.write(cmd);
    Serial.write('\n');
  }
  
  if(strcmp(cmd,"VERSION") == 0) {
    error = cmdVersion();
  } else if (strcmp(cmd,"ECHO") == 0) {
    error = cmdEcho();
  } else if(strncmp(cmd,"SET ", 4) == 0) {
    error = cmdSet(cmd+4);
  } else if(strcmp(cmd,"HELP") == 0) {
    error = cmdHelp();
  } else {
    error = 1;
  }
  
  if(echo) {
    if(error == 0) {
      Serial.println("OK");
    } else {
      Serial.println("ERROR");
    }  
  }
  cmdlen = 0;
  
  Serial.flush();
}


/*
 * Parsea y ejecuta un comando ASCII
 */
uint8_t cmdSet(char* v) {
  
  // Leds Number MODE
  if(strncmp(v,"L1N=",4) == 0) {
    setLeds1(atoi(v+4));
    
// 7 Segments     
//  } else if(strncmp(v,"SEG1=",5) == 0) {
//    parseSegValue(seg1, atoi(v+5));

  // Blink Mode
  } else if(strncmp(v,"L1BLINK=",8) == 0){
    setLeds1Blink(atoi(v+8));

  // Neutral Mode
  } else if(strncmp(v,"L1NEUTRAL=",10) == 0){
     setNeutral(atoi(v+10));
     
  // Tachometer
  } else if(strncmp(v,"TC=",3) == 0){
    setTachometer(atoi(v+3));
    
  // Default error
  } else {
    return 1;
  }
  return 0;
}


////////////////////////////
// Manejadores de comandos
////////////////////////////

/* 
 * Establece la tira de leds1 a 'numLeds'
 */
void setLeds1(uint8_t numLeds) {
   leds1Mode    = LED1_INTERRUPT_MODE_NONE;
   nLeds1Active = numLeds;   
}

/*
 * Activa/Desactiva el parpadeo de leds1 
 * 
 * Cualquier otro comando sobre leds1 desactivara el parpadeo
 */
void setLeds1Blink(uint8_t blink) {
  if(blink == 1){
    leds1Mode     = LED1_INTERRUPT_MODE_BLINK;
    lastBlinkTime = millis();
  }else{
    leds1Mode = LED1_INTERRUPT_MODE_NONE;      
  }
}

/*
 * Activa/Desactica el modo neutral de leds1
 * 
 * Cualquier otro comando sobre leds1 desactivara el modo neutral
 * Establece el numero de leds activos a 0. 
 */
void setNeutral(uint8_t neutral) {
  if(neutral == 1){
    leds1Mode       = LED1_INTERRUPT_MODE_NEUTRAL;
    lastNeutralTime = millis();
  }else{
    leds1Mode         = LED1_INTERRUPT_MODE_NONE;      
    nLeds1Active      = 0;
    lastNLeds1Active  = 1; // Diferente del ultimo para que lo trate la interrupcion. Mejorar.
  }
}

/*
 * Establece la señal de tacometro a las rpms indicadas.
 * 
 * Usado Timer3
 */
void setTachometer(int rpms) {
  if(rpms >= 0 && rpms <= TACHOMETER_MAX_RPMS) {
    long period = calculateTachPWMPeriod(rpms);
    
    if(echo){
      Serial.print("Set Period : ");
      Serial.println(period);
    }
    
    Timer3.setPeriod(period);
    Timer3.restart();
  }
}

/*
 * Help Handler
 */
uint8_t cmdHelp() {
  Serial.print("\n\nBanSimBoard - Help  Version : ");
  Serial.println(BANSIMBOARD_VERSION);
  Serial.println(" Commands :\n");
  Serial.println("  HELP\\r      : Show this help");
  Serial.println("  ECHO\\r      : Enable\Disable echo mode");
  Serial.println("  VERSION\\r   : Show Version");
  Serial.println('\n');
  Serial.println(" ASCII MODE :");
  Serial.println("    SET <OPERATION>=<VALUE>");
  Serial.println('\n');
  Serial.println("      L1N=<N>         : Turn on Led1 N Leds");
  Serial.println("      L1BLINK=<0|1>   : Turn on/off Led1 Blink");
  Serial.println("      L1NEUTRAL=<0|1> : Turn on/off Led1 Neutral");
  Serial.println("      KITT=<0|1>      : Turn on/off Kitt Mode");
  Serial.println("      TC=<RPM>        : Set tachometer at RPM");
  return 0;
}


/*
 * Version Handler
 */
uint8_t cmdVersion() {
  Serial.print("\n\nBanSimBoard - Version : ");
  Serial.println(BANSIMBOARD_VERSION);
  return 0;
}

/*
 * Echo Handler
 */
uint8_t cmdEcho() {
  echo = !echo;
  return 0;
}


//-------------------
// Led Array Funcs
//-------------------

/*
 * Enciende 'numLeds' de leds segun el esquema de color predefinido
 */
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

/*
 * Establece la tira de leds en modo neutral en funcion de 'phase' (leds pares o impares)
 */
void loadLedNeutralArray(uint8_t numLeds, uint8_t phase){
  int i;
  
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

/*
 * Apaga la tira de leds
 */
void clearLedArray(){
  int i;  
  for(i = 0; i < DEFAULT_LED_ARRAY_SIZE; i++)
     leds.setPixelColor(i, '\x00', '\x00', '\x00'); // setPixelColor(uint16_t n, uint32_t c), 
}

/*
 * Calcula el periodo de la señal PWM equivalente para las revoluciones dadas.
 */
long calculateTachPWMPeriod(int rpm) {
  // 1 / (x / 60) * 10^6
  // 6.10^7  / x
  return (long) (60000000 / rpm);
}


/*
 * Refresh Callback
 */
void refreshCallback(void){
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
//        ledsNeutralState = 1;
    }else{
       ledsNeutralState = 0;
    }
//    cli();
    leds.show();
//    sei();
  }
  
  
}

