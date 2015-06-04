#include <EEPROM.h>
#include <avr/wdt.h>
#include <Adafruit_NeoPixel.h>

char    cmd[16];
uint8_t cmdlen = 0;

const uint8_t EEPROM_N    = 0x00;
const uint8_t EEPROM_BL   = 0x01;
const uint8_t EEPROM_L    = 0x02;
const uint8_t EEPROM_T    = 0x03;
const uint8_t EEPROM_R    = 0x04;
const uint8_t EEPROM_B    = 0x05;
const uint8_t EEPROM_IO   = 0x06;
const uint8_t EEPROM_BR   = 0x07;
const uint8_t EEPROM_BAUD = 0x08;

const uint8_t  MAX_BAUD = 12;
const uint8_t  DEF_BAUD = 5;
const uint32_t BAUD[]   = {300,600,1200,2400,4800,9600,14400,19200,28800,31250,38400,57600,115200};

volatile uint8_t p_n;
uint8_t p_bl;
uint8_t p_l;
uint8_t p_t;
uint8_t p_r;
uint8_t p_b;
uint8_t p_io;
uint8_t p_br;
uint8_t p_baud;

uint8_t echo = 0;

unsigned long lastdata = millis();
boolean isOn = false;

Adafruit_NeoPixel leds(0,9);

void setup() {
  digitalWrite(13,1);
  wdt_disable();

  p_n    = EEPROM.read(EEPROM_N);
  p_bl   = EEPROM.read(EEPROM_BL);
  p_l    = EEPROM.read(EEPROM_L);
  p_t    = EEPROM.read(EEPROM_T);
  p_r    = EEPROM.read(EEPROM_R);
  p_b    = EEPROM.read(EEPROM_B);
  p_io   = EEPROM.read(EEPROM_IO);
  p_br   = EEPROM.read(EEPROM_BR);
  p_baud = EEPROM.read(EEPROM_BAUD);

  if(p_baud > MAX_BAUD) {
    p_baud = DEF_BAUD;
  }

  uint32_t baud = BAUD[p_baud];
  Serial.begin(baud);
  
  leds.setPin(p_io);
  leds.setNumPixels(p_n);
  leds.setBrightness(p_br);
  
  digitalWrite(13,0);
}

void loop() {
  if(readCommand()) {
    executeCommand();
  }

  if(isOn && (millis() - lastdata > 10000)) {
    clearLeds();
    leds.show();
    isOn = false;
  }
}

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

void executeCommand() {
  uint8_t error;
  cmd[cmdlen] = '\0';
  if(strcmp(cmd,"VERSION") == 0) {
    error = cmdVersion();
  } else if(strncmp(cmd,"GET",3) == 0) {
    error = cmdGet(cmd+3);
  } else if(strncmp(cmd,"SET",3) == 0) {
    error = cmdSet(cmd+3);
  } else if(strcmp(cmd,"DATA") == 0) {
    error = cmdData();
  } else if(strcmp(cmd,"RESET") == 0) {
    error = cmdReset();
  } else if(strcmp(cmd,"TEST") == 0) {
    error = cmdTest();
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

void clearLeds() {
  for(int i=0; i<p_n; i++) {
    leds.setPixelColor(i,0x000000);
  }
}

uint8_t cmdTest() {
  clearLeds();
  uint16_t led = p_bl;
  // Left
  for(int i=0; i<p_l; i++) {
    leds.setPixelColor((led+i) % p_n, 0xFF0000);
  }
  led+=p_l;
  // Top
  for(int i=0; i<p_t; i++) {
    leds.setPixelColor((led+i) % p_n, 0x00FF00);
  }
  led+=p_t;
  // Right
  for(int i=0; i<p_r; i++) {
    leds.setPixelColor((led+i) % p_n, 0x0000FF);
  }
  led+=p_r;
  // Bottom
  for(int i=0; i<p_b; i++) {
    leds.setPixelColor((led+i) % p_n, 0xFFFF00);
  }
  
  leds.setPixelColor(0    , 0xFFFFFF);
  leds.setPixelColor(p_n-1, 0x00FFFF);
  
  leds.show();
  
  return 0;
}

uint8_t cmdVersion() {
  Serial.println("Leduino v1.0");
  return 0;
}

uint8_t cmdHelp() {
  Serial.println("General:");
  Serial.println("  All the commands are ended with \\r character.");
  Serial.println("  After receive the DATA command, the system will read 3*N bytes with the RGB information of all the leds. (timeout=1s)");
  Serial.println("Commands:");
  Serial.println("  VERSION        Returns the software version.");
  Serial.println("  HELP           Shows this help.");
  Serial.println("  TEST           Turn on the leds to set the geometry.");
  Serial.println("  GET<var>       Gets the current value of <var>.");
  Serial.println("  SET<var>=<val> Sets the value of <var> to <val>.");
  Serial.println("  RESET          Restart the device.");
  Serial.println("  DATA           Listen for the led information starting from the bottom-left led (clockwise).");
  Serial.println("Variables:");
  Serial.println("  N    Number of leds in the strip. (0-255)");
  Serial.println("  BL   Bottom-Left led number (starting in 0). (0-255)");
  Serial.println("  L    Number of leds in the left edge. (0-255)");
  Serial.println("  T    Number of leds in the top edge. (0-255)");
  Serial.println("  R    Number of leds in the right edge. (0-255)");
  Serial.println("  B    Number of leds in the bottom edge. (0-255)");
  Serial.println("  IO   IO Port where the leds are connected.");
  Serial.println("  BR   Brigthness (0-255).");
  Serial.println("  ECHO Enable echo. (1,0)");
  Serial.println("  BAUD Change baud rate. (300,600,1200,2400,4800,9600,14400,19200,28800,31250,38400,57600,115200)");
  return 0;
}

uint8_t cmdGet(char* v) {
  if(strcmp(v,"N") == 0) {
    Serial.print(p_n,DEC);
    Serial.println();
  } else if(strcmp(v,"BL") == 0) {
    Serial.print(p_bl,DEC);
    Serial.println();
  } else if(strcmp(v,"L") == 0) {
    Serial.print(p_l,DEC);
    Serial.println();
  } else if(strcmp(v,"T") == 0) {
    Serial.print(p_t,DEC);
    Serial.println();
  } else if(strcmp(v,"R") == 0) {
    Serial.print(p_r,DEC);
    Serial.println();
  } else if(strcmp(v,"B") == 0) {
    Serial.print(p_b,DEC);
    Serial.println();
  } else if(strcmp(v,"IO") == 0) {
    Serial.print(p_io,DEC);
    Serial.println();
  } else if(strcmp(v,"BR") == 0) {
    Serial.print(p_br,DEC);
    Serial.println();
  } else if(strcmp(v,"ECHO") == 0) {
    Serial.print(echo,DEC);
    Serial.println();
  } else if(strcmp(v,"BAUD") == 0) {
    Serial.print(BAUD[p_baud],DEC);
    Serial.println();
  } else {
    return 1;
  }
  
  return 0;
}

uint8_t cmdSet(char* v) {
  if(strncmp(v,"N=",2) == 0) {
    p_n = atoi(v+2);
    EEPROM.write(EEPROM_N,p_n);
    clearLeds();
    leds.show();
    leds.setNumPixels(p_n);
  } else if(strncmp(v,"BL=",3) == 0) {
    p_bl = atoi(v+3);
    EEPROM.write(EEPROM_BL,p_bl);
  } else if(strncmp(v,"L=",2) == 0) {
    p_l = atoi(v+2);
    EEPROM.write(EEPROM_L,p_l);
  } else if(strncmp(v,"T=",2) == 0) {
    p_t = atoi(v+2);
    EEPROM.write(EEPROM_T,p_t);
  } else if(strncmp(v,"R=",2) == 0) {
    p_r = atoi(v+2);
    EEPROM.write(EEPROM_R,p_r);
  } else if(strncmp(v,"B=",2) == 0) {
    p_b = atoi(v+2);
    EEPROM.write(EEPROM_B,p_b);
  } else if(strncmp(v,"IO=",3) == 0) {
    p_io = atoi(v+3);
    EEPROM.write(EEPROM_IO,p_io);
    leds.setPin(p_io);
  } else if(strncmp(v,"BR=",3) == 0) {
    p_br = atoi(v+3);
    EEPROM.write(EEPROM_BR,p_br);
    leds.setBrightness(p_br);
  } else if(strncmp(v,"ECHO=",5) == 0) {
    echo = atoi(v+5);
  } else if(strncmp(v,"BAUD=",5) == 0) {
    uint32_t tmp = atol(v+5);
    uint8_t i=0;
    while(i <= MAX_BAUD) {
      if(tmp == BAUD[i]) break;
      i++;
    }
    if(i > MAX_BAUD) return 1;
    p_baud = i;
    EEPROM.write(EEPROM_BAUD,p_baud);
  } else {
    return 1;
  }
  
  cmdTest();
  
  return 0;
}

uint8_t cmdData() {
  long t;
  boolean timeout = false;
  uint8_t r,g,b;
  uint16_t i = 0;
  
  while(i < p_n) {
    timeout = false;
    t = millis();
    while(!timeout && Serial.available() < 3) {
      timeout = (millis() - t) >= 1000;
    }
    
    if(timeout) return 1;
    
    r = Serial.read();
    g = Serial.read();
    b = Serial.read();

    if(echo) {
      Serial.write(r);
      Serial.write(g);
      Serial.write(b);
    }
    
    leds.setPixelColor((p_bl + i) % p_n, r, g, b);
    
    i++;
  }
  
  leds.show();
  lastdata = millis();
  isOn = true;
  
  return 0;
}

uint8_t cmdReset() {
  wdt_enable(WDTO_500MS);
  while(1);
  return 1;
}
