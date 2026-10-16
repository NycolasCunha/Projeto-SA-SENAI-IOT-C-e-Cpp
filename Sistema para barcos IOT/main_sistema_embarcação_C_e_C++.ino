
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

// --- PINAGEM DO HARDWARE ---
const int PIN_LED_GRN = 13;
const int PIN_LED_YEL = 12;
const int PIN_LED_RED = 11;
const int PIN_BUZZER  = 8;
const int PIN_TRIG    = 9;
const int PIN_ECHO    = 7;
const int PIN_MOTOR   = 10;

// Frequências do buzzer
const int FREQ_LONGE = 200;
const int FREQ_PERTO = 2000;

int estadoAnterior = -1;

// --- LEITURA DO SENSOR ULTRASSÔNICO ---
long readUltrasonicDistance(int trigPin, int echoPin) {
  long soma = 0;
  int leiturasValidas = 0;

  for (int i = 0; i < 5; i++) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);

    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    long duration = pulseIn(echoPin, HIGH, 25000);

    if (duration > 0) {
      long distancia = duration / 58;

      if (distancia >= 2 && distancia <= 400) {
        soma += duration;
        leiturasValidas++;
      }
    }

    delay(5);
  }

  // Nenhuma leitura válida
  if (leiturasValidas == 0) {
    return 0;
  }

  return soma / leiturasValidas;
}

// --- DESLIGA OS ALERTAS ---
void desligarAlertas() {
  digitalWrite(PIN_LED_GRN, LOW);
  digitalWrite(PIN_LED_YEL, LOW);
  digitalWrite(PIN_LED_RED, LOW);
  digitalWrite(PIN_MOTOR, LOW);
  noTone(PIN_BUZZER);
}

void setup() {
  Serial.begin(9600);

  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);

  pinMode(PIN_LED_GRN, OUTPUT);
  pinMode(PIN_LED_YEL, OUTPUT);
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_MOTOR, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);

  lcd.init();
  lcd.backlight();
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print(" Sistema Pronto ");
  delay(1500);
  lcd.clear();
}

void loop() {
  long duration = readUltrasonicDistance(PIN_TRIG, PIN_ECHO);

  // --- 1. SENSOR SEM DETECCAO ---
  if (duration == 0) {
    desligarAlertas();

    if (estadoAnterior != -2) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Sem deteccao!");

      lcd.setCursor(0, 1);
      lcd.print("Verifique sensor");

      estadoAnterior = -2;
    }

    Serial.println("Sem deteccao!");
    delay(100);
    return;
  }

  int cm = duration / 58.2;

  Serial.print("Distancia: ");
  Serial.print(cm);
  Serial.println(" cm");

  // --- 2. AREA LIVRE: ACIMA DE 309 CM ---
  // LED verde aceso continuamente e sem bip.
  if (cm > 309) {
    digitalWrite(PIN_LED_GRN, HIGH);
    digitalWrite(PIN_LED_YEL, LOW);
    digitalWrite(PIN_LED_RED, LOW);
    digitalWrite(PIN_MOTOR, LOW);
    noTone(PIN_BUZZER);

    if (estadoAnterior != 0) {
      lcd.setCursor(0, 0);
      lcd.print("   Area Livre   ");
      estadoAnterior = 0;
    }
  }

  // --- 3. DISTANCIA SEGURA: 100 A 309 CM ---
  // LED verde aceso e sem bip.
  else if (cm >= 100) {
    digitalWrite(PIN_LED_GRN, HIGH);
    digitalWrite(PIN_LED_YEL, LOW);
    digitalWrite(PIN_LED_RED, LOW);
    digitalWrite(PIN_MOTOR, LOW);
    noTone(PIN_BUZZER);

    if (estadoAnterior != 1) {
      lcd.setCursor(0, 0);
      lcd.print("   Distancia OK ");
      estadoAnterior = 1;
    }
  }

  // --- 4. ATENCAO: 40 A 99 CM ---
  else if (cm >= 25 && cm <= 40 ) {
    digitalWrite(PIN_LED_GRN, LOW);
    digitalWrite(PIN_LED_YEL, HIGH);
    digitalWrite(PIN_LED_RED, LOW);
    digitalWrite(PIN_MOTOR, LOW);

    int frequencia = map(cm, 99, 40, FREQ_LONGE, FREQ_PERTO);
    tone(PIN_BUZZER, frequencia);

    if (estadoAnterior != 2) {
      lcd.setCursor(0, 0);
      lcd.print("   ATENCAO!     ");
      estadoAnterior = 2;
    }
  }

  // --- 5. PERIGO: 6 A 39 CM ---
  else if (cm >= 6) {
    digitalWrite(PIN_LED_GRN, LOW);
    digitalWrite(PIN_LED_YEL, LOW);
    digitalWrite(PIN_LED_RED, HIGH);
    digitalWrite(PIN_MOTOR, HIGH);

    int frequencia = map(cm, 39, 6, FREQ_LONGE, FREQ_PERTO);
    tone(PIN_BUZZER, frequencia);

    if (estadoAnterior != 3) {
      lcd.setCursor(0, 0);
      lcd.print("  !! PERIGO !!  ");
      estadoAnterior = 3;
    }
  }

  // --- 6. COLISAO IMINENTE: ABAIXO DE 6 CM ---
  else {
    digitalWrite(PIN_LED_GRN, LOW);
    digitalWrite(PIN_LED_YEL, LOW);
    digitalWrite(PIN_LED_RED, HIGH);
    digitalWrite(PIN_MOTOR, HIGH);

    tone(PIN_BUZZER, FREQ_PERTO);

    if (estadoAnterior != 4) {
      lcd.setCursor(0, 0);
      lcd.print("  !!COLISAO!!   ");
      estadoAnterior = 4;
    }
  }

  // --- 7. DISTANCIA NO LCD ---
  lcd.setCursor(0, 1);
  lcd.print("Dist: ");
  lcd.print(cm);
  lcd.print(" cm     ");

  delay(50);
}
