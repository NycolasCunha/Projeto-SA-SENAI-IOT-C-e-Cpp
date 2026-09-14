// Inicializa o display LCD no endereço I2C 0x27 (16 colunas por 2 linhas)
// Nota: Se estiver usando o Tinkercad, garanta que a biblioteca LiquidCrystal_I2C esteja ativa.
#include <Wire.h>
#include <Adafruit_LiquidCrystal.h>   // troca a biblioteca

// O construtor recebe o OFFSET do endereço (0 a 7), não o hex direto.
// Como o painel mostrou 0x20 (sem jumper = offset 0), fica:
Adafruit_LiquidCrystal lcd(0);

// Definição dos Pinos de Hardware
const int PIN_PING = 7;      // Pino SIG do Sensor Ultrassônico
const int PIN_MOTOR = 9;     // Pino de controle do Motor de Vibração
const int PIN_LED_RED = 11;  // LED Vermelho
const int PIN_LED_YEL = 12;  // LED Amarelo
const int PIN_LED_GRN = 13;  // LED Verde
const int PIN_BUZZER = 8;    // Pino "+" do Buzzer Piezo (o "-" vai no GND)

// Faixa de frequência do som (em Hz)
const int FREQ_LONGE = 200;   // som grave (longe do limite)
const int FREQ_PERTO = 2000;  // som agudo (muito perto)
// Se quiser o efeito INVERTIDO (mais grave quanto mais perto),
// troque os dois valores acima entre si.

// Função para medir a distância em centímetros usando o sensor Ping)))
long readUltrasonicDistance(int pin) {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
  delayMicroseconds(2);
  
  digitalWrite(pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(pin, LOW);
  
  pinMode(pin, INPUT);
  return pulseIn(pin, HIGH);
}

void setup() {
  Serial.begin(9600);

  // Configuração dos Pinos como Saída
  pinMode(PIN_LED_GRN, OUTPUT);
  pinMode(PIN_LED_YEL, OUTPUT);
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_MOTOR, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);

  // Inicialização da Tela LCD
  lcd.begin(16, 2);
  lcd.setBacklight(HIGH);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" Sistema Pronto ");
  delay(1000);
  lcd.clear();
}

void loop() {
  // Leitura da distância em cm
  long duration = readUltrasonicDistance(PIN_PING);
  int cm = 0.01723 * duration;

  // Envia a leitura para o Monitor Serial
  Serial.print("Distancia: ");
  Serial.print(cm);
  Serial.println(" cm");

  //  LÓGICA DO SOM (BUZZER) 
  // O tom fica mais agudo (maior frequência) conforme o objeto se aproxima.
  if (cm > 100) {
    noTone(PIN_BUZZER); // fora da área de alerta: som desligado
  } else {
    int distanciaLimitada = constrain(cm, 0, 100);
    // cm=100 (limite) -> FREQ_LONGE | cm=0 (colisão) -> FREQ_PERTO
    int frequencia = map(distanciaLimitada, 100, 0, FREQ_LONGE, FREQ_PERTO);
    tone(PIN_BUZZER, frequencia);
  }

  // --- LÓGICA DE ALERTAS E NÍVEIS DE APROXIMAÇÃO ---

  if (cm > 100) {
    // Fora do limite de alerta: tudo desligado
    digitalWrite(PIN_LED_GRN, LOW);
    digitalWrite(PIN_LED_YEL, LOW);
    digitalWrite(PIN_LED_RED, LOW);
    digitalWrite(PIN_MOTOR, LOW);

    lcd.setCursor(0, 0);
    lcd.print("   Area Livre   ");
    lcd.setCursor(0, 1);
    lcd.print("                ");

  } else if (cm > 80 && cm <= 100) {
    // 1. Começou a se aproximar (antes do 1º LED acender): Motor LIGADO + "!Atencao!"
    digitalWrite(PIN_LED_GRN, LOW);
    digitalWrite(PIN_LED_YEL, LOW);
    digitalWrite(PIN_LED_RED, LOW);
    digitalWrite(PIN_MOTOR, HIGH);

    lcd.setCursor(0, 0);
    lcd.print("   !Atencao!    ");
    lcd.setCursor(0, 1);
    lcd.print("                ");

  } else if (cm > 50 && cm <= 80) {
    // 2. Entrou na área do sensor (Mais Longe): LED Verde + "!Cuidado!"
    digitalWrite(PIN_LED_GRN, HIGH);
    digitalWrite(PIN_LED_YEL, LOW);
    digitalWrite(PIN_LED_RED, LOW);
    digitalWrite(PIN_MOTOR, LOW);

    lcd.setCursor(0, 0);
    lcd.print("   !Cuidado!    ");
    lcd.setCursor(0, 1);
    lcd.print("                ");

  } else if (cm > 25 && cm <= 50) {
    // 3. Aproximação Média: LED Amarelo + "!Cuidado!"
    digitalWrite(PIN_LED_GRN, LOW);
    digitalWrite(PIN_LED_YEL, HIGH);
    digitalWrite(PIN_LED_RED, LOW);
    digitalWrite(PIN_MOTOR, LOW);

    lcd.setCursor(0, 0);
    lcd.print("   !Cuidado!    ");
    lcd.setCursor(0, 1);
    lcd.print("                ");

  } else if (cm > 10 && cm <= 25) {
    // 4. Muito Próximo: LED Vermelho + Motor LIGADO + "!!Cuidado!!"
    digitalWrite(PIN_LED_GRN, LOW);
    digitalWrite(PIN_LED_YEL, LOW);
    digitalWrite(PIN_LED_RED, HIGH);
    digitalWrite(PIN_MOTOR, HIGH);

    lcd.setCursor(0, 0);
    lcd.print("  !!Cuidado!!   ");
    lcd.setCursor(0, 1);
    lcd.print("                ");

  } else {
    // 5. Passou do limite de aproximação: LED Vermelho + Motor LIGADO + "!!Possivel Colisao!!"
    digitalWrite(PIN_LED_GRN, LOW);
    digitalWrite(PIN_LED_YEL, LOW);
    digitalWrite(PIN_LED_RED, HIGH);
    digitalWrite(PIN_MOTOR, HIGH);

    // Mensagem dividida em duas linhas no visor 16x2
    lcd.setCursor(0, 0);
    lcd.print("  !!Possivel!!  ");
    lcd.setCursor(0, 1);
    lcd.print("  !!Colisao!!   ");
  }

  delay(100); // Aguarda 100ms para a próxima leitura
}
