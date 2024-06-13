#include <IRremote.hpp>

#define silnikp1  2
#define silnikp2  3
#define silnikl1  5
#define silnikl2  4
#define trigger 9
#define echo 10
#define pwm_l  6
#define pwm_p 7
#define ir 8
#define buzzer 11
#define reset 12

// kody odbierane
#define OFF 0x45
#define PRZOD 0x40
#define TYL 0x19
#define LEWO 0x07
#define PRAWO 0x09
#define STOP 0x15
#define V_MNIEJ 0x0C
#define V_WIECEJ 0x16

uint8_t v = 150;

int min_dystans = 20;
unsigned long prev_millis = 0;
unsigned int beep_przerwa = 1000;
unsigned long lastSignalTime = 0;
unsigned long signalTimeout = 200; // Czas w ms, po którym robot się zatrzyma, jeśli nie dostanie sygnału
bool jazda = false;
bool buzzer_enabled = true;  // Initial state of the buzzer

void switch_buzzer() {
  static unsigned long lastSwitchTime = 0;
  unsigned long currentTime = millis();

  // Implement a debounce mechanism
  if (currentTime - lastSwitchTime < 200) {
    return;  // Ignore rapid consecutive presses
  }

  // Toggle the buzzer state
  buzzer_enabled = !buzzer_enabled;

  // Update the last switch time
  lastSwitchTime = currentTime;

  // Update buzzer output based on the new state
  if (buzzer_enabled && dystans() < min_dystans) {
    digitalWrite(buzzer, HIGH); 
  } else {
    digitalWrite(buzzer, LOW); 
  }
}


void stop(){
  jazda = false;
  digitalWrite(silnikl2, LOW);
  digitalWrite(silnikl1, LOW);
  digitalWrite(silnikp2, LOW);
  digitalWrite(silnikp1, LOW);
  analogWrite(pwm_p, 0);
  analogWrite(pwm_l, 0);
}

void prawe_kolo_przod(uint8_t v){
  analogWrite(pwm_p, v);
  digitalWrite(silnikp2, HIGH);
  digitalWrite(silnikp1, LOW);
}

void lewe_kolo_przod(uint8_t v){
  analogWrite(pwm_l, v);
  digitalWrite(silnikl1, HIGH);
  digitalWrite(silnikl2, LOW);
}

void prawe_kolo_tyl(uint8_t v){
  analogWrite(pwm_p, v);
  digitalWrite(silnikp2, LOW);
  digitalWrite(silnikp1, HIGH);
}

void lewe_kolo_tyl(uint8_t v){
  analogWrite(pwm_l, v);
  digitalWrite(silnikl1, LOW);
  digitalWrite(silnikl2, HIGH);
}

void jedz_przod(uint8_t v){
  jazda = true;
  lewe_kolo_przod(v);
  prawe_kolo_przod(v);
}

void jedz_tyl(uint8_t v){  
  jazda = true;
  lewe_kolo_tyl(v);
  prawe_kolo_tyl(v);
}

void skret_lewo(uint8_t v){
  jazda = true;
  lewe_kolo_tyl(v);
  prawe_kolo_przod(v);
}

void skret_prawo(uint8_t v){
  jazda = true;
  lewe_kolo_przod(v);
  prawe_kolo_tyl(v);
}

int dystans() {
  long czas, dystans;
 
  digitalWrite(trigger, LOW);
  delayMicroseconds(2);
  digitalWrite(trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigger, LOW);
 
  czas = pulseIn(echo, HIGH);
  dystans = czas / 58;

  return dystans;
}

void setup() {
  // silniki
  pinMode(silnikl1, OUTPUT);
  pinMode(silnikl2, OUTPUT);
  pinMode(silnikp1, OUTPUT);
  pinMode(silnikp2, OUTPUT);
  pinMode(pwm_l, OUTPUT);
  pinMode(pwm_p, OUTPUT);
  pinMode(buzzer, OUTPUT);

  stop();
  digitalWrite(buzzer, LOW);

  // czujnik odleglosci
  pinMode(trigger, OUTPUT);
  pinMode(echo, INPUT);

  // odbiornik IR
  IrReceiver.begin(ir, ENABLE_LED_FEEDBACK);

  // Serial monitor
  Serial.begin(9600);
  Serial.println(v);
}

void loop() {

  if (IrReceiver.decode()) {

    lastSignalTime = millis();

 
    switch (IrReceiver.decodedIRData.command) {
      case OFF:
        switch_buzzer(); 
        delay(5); 
        break;
      case PRZOD:
        jedz_przod(v);  
        break;
      case TYL:
        jedz_tyl(v);   
        break;
      case LEWO:
        skret_lewo(v);   
        break;
      case PRAWO:
        skret_prawo(v); 
        break;
      case V_WIECEJ:
        v += 5;         
        Serial.println(v);
        break;
      case V_MNIEJ:
        v -= 5;       
        Serial.println(v);
        break;
      default:
        break;
    }


    if (dystans() < min_dystans && buzzer_enabled) {
      digitalWrite(buzzer, HIGH);
    } else {
      digitalWrite(buzzer, LOW); 
    }

    IrReceiver.resume();  
  } else {

    if (millis() - lastSignalTime > signalTimeout) {
      stop();  
    }
  }
}

