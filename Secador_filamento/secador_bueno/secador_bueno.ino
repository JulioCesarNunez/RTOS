#include <Arduino.h>
#include <Fuzzy.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>
#include <Keypad.h>

// ---------- DHT11 ----------
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ---------- PWM ----------
#define IN1 18
#define IN2 33
#define PWM_PIN 19
const int pwmFreq = 5000;
const int pwmRes = 8;

// ---------- LCD I2C ----------
#define SDA_PIN 21
#define SCL_PIN 22
LiquidCrystal_PCF8574 lcd(0x27);

// ---------- Fuzzy ----------
Fuzzy *fuzzy = new Fuzzy();
float setPoint = 60;       // valor por defecto
float currentTemp = 0;
float currentHum  = 0;

// ---------- Fuzzy Sets ----------
FuzzySet *err1, *err2, *err3, *err4, *err5;
FuzzySet *apagado, *lento, *medio, *rapido, *full;

// Mutex para proteger variables compartidas
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

// ---------- Teclado Matricial ----------
#define FILAS 4
#define COLUMNAS 4
byte pinesFilas[FILAS]    ={2,23,0,13};
byte pinesColumnas[COLUMNAS] = {12,14,27,26};

char keys[FILAS][COLUMNAS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

Keypad keypad = Keypad(makeKeymap(keys), pinesFilas, pinesColumnas, FILAS, COLUMNAS);

// ---------- Configuración Fuzzy ----------
void setupFuzzy() {
  FuzzyInput *error = new FuzzyInput(1);
  err1 = new FuzzySet(-45,-45,-30,-20);
  err2 = new FuzzySet(-25,-15,-10,-5);
  err3 = new FuzzySet(-8,-3,3,8);
  err4 = new FuzzySet(5,10,15,20);
  err5 = new FuzzySet(18,25,45,45);

  error->addFuzzySet(err1);
  error->addFuzzySet(err2);
  error->addFuzzySet(err3);
  error->addFuzzySet(err4);
  error->addFuzzySet(err5);
  fuzzy->addFuzzyInput(error);

  FuzzyOutput *pwm = new FuzzyOutput(1);
  apagado = new FuzzySet(0,0,5,6.5);
  lento   = new FuzzySet(6,8.2,9.5,9.5);
  medio   = new FuzzySet(8.8,10,11,11);
  rapido  = new FuzzySet(10,10.8,11.9,11.9);
  full    = new FuzzySet(12,12,12,12);

  pwm->addFuzzySet(apagado);
  pwm->addFuzzySet(lento);
  pwm->addFuzzySet(medio);
  pwm->addFuzzySet(rapido);
  pwm->addFuzzySet(full);
  fuzzy->addFuzzyOutput(pwm);

  FuzzyRuleAntecedent *r1 = new FuzzyRuleAntecedent(); r1->joinSingle(err1);
  FuzzyRuleConsequent *r1o = new FuzzyRuleConsequent(); r1o->addOutput(apagado);
  fuzzy->addFuzzyRule(new FuzzyRule(1,r1,r1o));

  FuzzyRuleAntecedent *r2 = new FuzzyRuleAntecedent(); r2->joinSingle(err2);
  FuzzyRuleConsequent *r2o = new FuzzyRuleConsequent(); r2o->addOutput(apagado);
  fuzzy->addFuzzyRule(new FuzzyRule(2,r2,r2o));

  FuzzyRuleAntecedent *r3 = new FuzzyRuleAntecedent(); r3->joinSingle(err3);
  FuzzyRuleConsequent *r3o = new FuzzyRuleConsequent(); r3o->addOutput(apagado);
  fuzzy->addFuzzyRule(new FuzzyRule(3,r3,r3o));

  FuzzyRuleAntecedent *r4 = new FuzzyRuleAntecedent(); r4->joinSingle(err4);
  FuzzyRuleConsequent *r4o = new FuzzyRuleConsequent(); r4o->addOutput(lento);
  fuzzy->addFuzzyRule(new FuzzyRule(4,r4,r4o));

  FuzzyRuleAntecedent *r5 = new FuzzyRuleAntecedent(); r5->joinSingle(err5);
  FuzzyRuleConsequent *r5o = new FuzzyRuleConsequent(); r5o->addOutput(medio);
  fuzzy->addFuzzyRule(new FuzzyRule(5,r5,r5o));
}

// ---------- TAREA: Control Difuso y DHT ----------
void fuzzyTask(void *pv) {
  for (;;) {
    float temp = dht.readTemperature();
    float hum  = dht.readHumidity();

    if (!isnan(temp) && !isnan(hum)) {
      portENTER_CRITICAL(&mux);
      currentTemp = temp;
      currentHum  = hum;
      portEXIT_CRITICAL(&mux);

      float errorTemp = setPoint - currentTemp;
      fuzzy->setInput(1, errorTemp);
      fuzzy->fuzzify();
      float pwmOut = fuzzy->defuzzify(1);
      int pwmFinal = map(pwmOut, 0, 12, 0, 255);
      pwmFinal = constrain(pwmFinal, 0, 255);
      if (errorTemp <= 0) pwmFinal = 0;

      ledcWrite(PWM_PIN, pwmFinal);

      Serial.printf("Temp: %.1f C | Hum: %.1f %% | PWM: %d\n", currentTemp, currentHum, pwmFinal);
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

// ---------- TAREA: LCD ----------
void lcdTask(void *pv) {
  for (;;) {
    float temp, hum;
    portENTER_CRITICAL(&mux);
    temp = currentTemp;
    hum  = currentHum;
    portEXIT_CRITICAL(&mux);

    lcd.setCursor(0,0);
    lcd.printf("Temp: %.1f C   ", temp);
    lcd.setCursor(0,1);
    lcd.printf("Hum: %.1f %%    ", hum);

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

// ---------- TAREA: Teclado Matricial ----------
void keypadTask(void *pv) {
  String inputSP = "";
  for (;;) {
    char key = keypad.getKey();
    if (key) {
      if (key >= '0' && key <= '9') {
        inputSP += key;      // construir número
        lcd.setCursor(0,1);
        lcd.printf("SetPt: %s   ", inputSP.c_str());
      } else if (key == '#') { // confirmar
        if (inputSP.length() > 0) {
          float sp = inputSP.toFloat();
          portENTER_CRITICAL(&mux);
          setPoint = sp;
          portEXIT_CRITICAL(&mux);
          Serial.print("Nuevo setpoint desde teclado: ");
          Serial.println(setPoint);
          inputSP = "";
        }
      } else if (key == '*') { // borrar
        inputSP = "";
        lcd.setCursor(0,1);
        lcd.print("SetPt:        ");
      }
    }
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

// ---------- SETUP ----------
void setup() {
  Serial.begin(115200);

  // PWM
  pinMode(IN1, OUTPUT); digitalWrite(IN1,HIGH);
  pinMode(IN2, OUTPUT); digitalWrite(IN2,LOW);
  ledcAttach(PWM_PIN, pwmFreq, pwmRes);

  // LCD
  Wire.begin(SDA_PIN, SCL_PIN);
  lcd.begin(16,2);
  lcd.setBacklight(HIGH);
  lcd.setCursor(0,0);
  lcd.print("Inicializando...");
  delay(2000);
  lcd.clear();

  // DHT y Fuzzy
  dht.begin();
  setupFuzzy();

  // Crear tareas RTOS
  xTaskCreatePinnedToCore(fuzzyTask, "fuzzyTask", 4096, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(lcdTask, "lcdTask", 4096, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(keypadTask, "keypadTask", 4096, NULL, 1, NULL, 1);
}

void loop() {
  vTaskDelay(portMAX_DELAY);
}
