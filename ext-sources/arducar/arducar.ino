#define IO(pin,state) ((state) == 0) ? IO_L(pin) : IO_H(pin)
#define IO_L(pin)     ((pin)<8) ? PORTD &= ~(1<<(pin)) : PORTB &= ~(1<<((pin)-8))
#define IO_H(pin)     ((pin)<8) ? PORTD |=  (1<<(pin)) : PORTB |=  (1<<((pin)-8))

#define DATA 8
#define CLSH 9
#define CLST 10
#define CLR  11
#define LED  12

#define NCHAR 3
#define NLEVL 4
#define NSEGMENT 8

/*
    0
   ---
 6| 5 |7
   ---
 1| 2 |4
   --- o 3
   
DATA: B01234567
*/

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

const uint8_t punto = B00010000;

const uint8_t a_corchete = B11100010;
const uint8_t c_corchete = B10101001;

// Display data (must be read from RAM)
volatile uint8_t disp[NCHAR];
volatile uint8_t levl[NCHAR];

void setup() {
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
    disp[i]=0xFF;
    levl[i]=NLEVL;
  }

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

// Variables para el refresco
uint8_t current_char = 0;
uint8_t current_levl = 0;

// Interrupcion de refresco
ISR(TIMER2_COMPA_vect) {
  IO(LED,1);
  
  // Seleccionamos el caracter correspondiente
  // Logica inversa: 0-on, 1-off
  for(uint8_t c=0; c < NCHAR; c++) {
    IO(DATA,!((c==current_char) && (current_levl <= levl[c])));
    IO(CLSH,1);
    IO(CLSH,0);
  }
  
  // Establecemos el valor de cada segmento
  // para el caracter actual
  uint8_t current_data = disp[current_char];
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

// Main
unsigned long levl_time = 0;

void loop() {
  /*
  // Decimas de segundo
  unsigned long time = millis() / 100;
  
  for(uint8_t i=0;i<NCHAR;i++) {
    disp[i] = numeros[time%10];
    if(i==1) disp[i] = disp[i] | punto;
    time/=10;
  }
  */
  
  /*
  if(millis() - levl_time > 1000) {
    for(uint8_t i=0;i<NCHAR;i++) {
      levl[i] = (levl[i]+1) % NLEVL;
    }
    
    levl_time = millis();
  }
  */
  
  
  for(uint8_t marcha=1; marcha<=6; marcha++) {
    levl[0] = levl[1] = levl[2] = 1;
    
    for(uint8_t x=0;x<5;x++) {
      disp[2] = numeros[x];
      for(uint8_t y=0;y<10;y++) {
        disp[1] = numeros[y];
        for(uint8_t z=0;z<10;z++) {
          disp[0] = numeros[z];
          delay(5*(x+marcha));
        }
      }
    }
    
    levl[0] = levl[1] = levl[2] = NLEVL;
    
    for(uint8_t x=0;x<5;x++) {
      disp[0] = disp[1] = disp[2] = 0x00;
      
      delay(150);
      
      disp[2] = a_corchete;
      disp[1] = numeros[marcha+1];
      disp[0] = c_corchete;
      
      delay(150);
    }
    
    delay(600);
  }
}
