// Código Simon Dice con niveles infinitos y comentarios añadidos
// Versión mejorada: niveles infinitos usando std::vector y comentarios detallados

#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <vector>

#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else 
  static const BaseType_t app_cpu = 1;
#endif

// -------------------- CONFIGURACIÓN HW --------------------
// I2C para LCD (SDA = GPIO21, SCL = GPIO22 si aplican)
LiquidCrystal_I2C lcd(0x27, 16, 2); // Cambia a 0x3F si tu módulo usa esa dirección

// Pines del teclado matricial 4x4
const byte ROWS = 4;
const byte COLS = 4;
byte rowPins[ROWS] = {27, 14, 12, 13}; // filas - ajústalas si cambias cableado
byte colPins[COLS] = {26, 25, 33, 32}; // columnas - ajústalas si cambias cableado

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// -------------------- PARÁMETROS DEL JUEGO --------------------
// Ya NO hay MAX_LEVELS: el juego crecerá indefinidamente (niveles "infinitos").
std::vector<int> sequenceVec; // guarda la secuencia dinámica (valores 1..9)
int level = 0;                 // nivel actual (1..n)
volatile bool gameActive = false;
volatile bool requestReset = false; // bandera para reinicio solicitado por el usuario

// Tiempos (en ms)
const unsigned long SHOW_DELAY = 700;     // tiempo que se muestra cada número de Simon
const unsigned long BETWEEN_DELAY = 250;  // tiempo entre números cuando Simon muestra
const unsigned long INPUT_TIMEOUT = 6000; // tiempo máximo para que el jugador ingrese cada número

// FreeRTOS semáforos (binary semaphores)
SemaphoreHandle_t semSimonTurn;
SemaphoreHandle_t semPlayerTurn;

// Handles de tareas
TaskHandle_t taskSimonHandle = NULL;
TaskHandle_t taskPlayerHandle = NULL;

// -------------------- PROTOTIPOS --------------------
void taskSimon(void *pvParameters);
void taskPlayer(void *pvParameters);
void startNewGame();
int random1to9();
void showCentered(const char *line1, const char *line2 = NULL);
void showNumberOnLCD(int num);
int mapKeyToNumber(char k);
void doResetRequested();

// -------------------- SETUP --------------------
void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();

  // Seed aleatoria usando un pin ADC no conectado (mejor semilla que constante)
  randomSeed(analogRead(34));

  // Crear semáforos binarios
  semSimonTurn = xSemaphoreCreateBinary();
  semPlayerTurn = xSemaphoreCreateBinary();

  if (semSimonTurn == NULL || semPlayerTurn == NULL) {
    Serial.println("Error creando semaforos");
    while (1) delay(1000);
  }

  // Crear las tareas y fijarlas al mismo core definido por app_cpu
  xTaskCreatePinnedToCore(taskSimon,  "TaskSimon",  4096, NULL, 2, &taskSimonHandle, app_cpu);
  xTaskCreatePinnedToCore(taskPlayer, "TaskPlayer", 4096, NULL, 1, &taskPlayerHandle, app_cpu);

  // Mensaje de bienvenida indicando cómo iniciar
  showCentered("SIMON DICE - ESP32", "Presiona # para INI");
  Serial.println("Esperando que el usuario presione # para iniciar...");

  // Esperar a que el usuario presione '#' para arrancar (bloqueo en setup, está bien)
  while (true) {
    char k = keypad.getKey();
    if (k == '#') {
      startNewGame();
      xSemaphoreGive(semSimonTurn); // dar turno a Simon para comenzar
      break;
    }
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

// loop vacío: toda la lógica corre en tareas FreeRTOS
void loop() {
  vTaskDelay(1000 / portTICK_PERIOD_MS);
}

// -------------------- FUNCIONES AUXILIARES --------------------
// Inicia un nuevo juego: limpia la secuencia y genera el primer elemento
void startNewGame() {
  sequenceVec.clear();        // limpiar secuencia previa
  sequenceVec.push_back(random1to9()); // primer número aleatorio
  level = 1;
  gameActive = true;
  requestReset = false;

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Nivel 1");
  lcd.setCursor(0,1);
  lcd.print("Buena suerte!");
  Serial.println("Nuevo juego iniciado.");
}

// Genera un número aleatorio entre 1 y 9 (inclusive)
int random1to9() {
  return random(1, 10); // random(a,b) devuelve [a, b-1] en Arduino
}

// Muestra dos líneas centradas en la LCD (la segunda es opcional)
void showCentered(const char *line1, const char *line2) {
  lcd.clear();
  int len1 = strlen(line1);
  int pos1 = max(0, (16 - len1) / 2);
  lcd.setCursor(pos1, 0);
  lcd.print(line1);
  if (line2 != NULL) {
    int len2 = strlen(line2);
    int pos2 = max(0, (16 - len2) / 2);
    lcd.setCursor(pos2, 1);
    lcd.print(line2);
  }
}

// Muestra un número grande en la LCD para que Simon "diga" el número
void showNumberOnLCD(int num) {
  char buf[17];
  snprintf(buf, sizeof(buf), "  >>  %d  <<   ", num);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Simon dice:");
  lcd.setCursor(0,1);
  lcd.print(buf);
}

// Mapea teclas '1'..'9' a números 1..9, devuelve 0 si no es relevante
int mapKeyToNumber(char k) {
  if (k >= '1' && k <= '9') return k - '0';
  return 0;
}

// Realiza el reinicio solicitado: muestra mensaje y espera '#'
void doResetRequested() {
  gameActive = false;
  sequenceVec.clear();
  level = 0;
  requestReset = false;
  showCentered("Reiniciando...", "Presiona # para INI");
  Serial.println("Juego reiniciado por usuario. Esperando #...");

  while (true) {
    char k = keypad.getKey();
    if (k == '#') {
      startNewGame();
      xSemaphoreGive(semSimonTurn);
      break;
    }
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

// -------------------- TAREA SIMON --------------------
// Muestra la secuencia actual en pantalla y luego da el turno al jugador
void taskSimon(void *pvParameters) {
  (void) pvParameters;
  for (;;) {
    if (xSemaphoreTake(semSimonTurn, portMAX_DELAY) == pdTRUE) {
      if (requestReset) {
        doResetRequested();
        continue;
      }
      if (!gameActive) continue; // si no hay juego activo, esperar

      // Mostrar mensaje con el nivel actual (sin tope máximo)
      char line2[17];
      snprintf(line2, sizeof(line2), "Nivel %d", level);
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Simon mostrando...");
      lcd.setCursor(0,1);
      lcd.print(line2);
      vTaskDelay(700 / portTICK_PERIOD_MS);

      // Mostrar cada número de la secuencia
      for (size_t i = 0; i < sequenceVec.size(); i++) {
        if (requestReset) break;
        int n = sequenceVec[i];
        showNumberOnLCD(n);
        vTaskDelay(SHOW_DELAY / portTICK_PERIOD_MS);
        // borrar la línea de número y esperar un poco antes del siguiente
        lcd.setCursor(0,1);
        lcd.print("                ");
        vTaskDelay(BETWEEN_DELAY / portTICK_PERIOD_MS);
      }

      if (!requestReset) {
        // Indicar que es turno del jugador
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Tu turno:");
        lcd.setCursor(0,1);
        lcd.print("Presiona 1..9");
        xSemaphoreGive(semPlayerTurn);
      } else {
        doResetRequested();
      }
    }
  }
}

// -------------------- TAREA PLAYER --------------------
// Lee las entradas del jugador y compara con la secuencia
void taskPlayer(void *pvParameters) {
  (void) pvParameters;
  for (;;) {
    if (xSemaphoreTake(semPlayerTurn, portMAX_DELAY) == pdTRUE) {
      if (requestReset) {
        doResetRequested();
        continue;
      }
      if (!gameActive) continue;

      bool ok = true; // bandera que mantendrá si el jugador sigue correcto

      // Para cada elemento en la secuencia, esperar la entrada del jugador
      for (size_t i = 0; i < sequenceVec.size(); i++) {
        unsigned long start = millis();
        bool got = false;
        int playerNum = 0;

        // Esperar la tecla hasta INPUT_TIMEOUT
        while ((millis() - start) < INPUT_TIMEOUT) {
          char k = keypad.getKey();
          if (k != NO_KEY) {
            if (k == '#') {
              // si el jugador presiona '#', solicita reinicio
              requestReset = true;
              ok = false;
              got = false;
              break;
            }
            int mapped = mapKeyToNumber(k);
            if (mapped != 0) {
              playerNum = mapped;
              got = true;
              break;
            }
          }
          vTaskDelay(10 / portTICK_PERIOD_MS);
        }

        if (requestReset) break;

        if (!got) {
          // Timeout: jugador no respondió a tiempo
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Tiempo agotado");
          lcd.setCursor(0,1);
          lcd.print("Fin. Nivel:");
          lcd.print(level);
          Serial.println("Timeout. Juego terminado.");
          ok = false;
          break;
        }

        // Mostrar lo que el jugador pulsó
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Tu presionaste:");
        lcd.setCursor(0,1);
        lcd.print(" -> ");
        lcd.print(playerNum);
        vTaskDelay(250 / portTICK_PERIOD_MS);

        // Comparar con el valor esperado en la secuencia
        if (playerNum != sequenceVec[i]) {
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("INCORRECTO!");
          lcd.setCursor(0,1);
          lcd.print("Nivel: ");
          lcd.print(level);
          Serial.printf("Fallaste en pos %d: esperado %d, presionaste %d", (int)i, sequenceVec[i], playerNum);
          ok = false;
          break;
        } else {
          // Correcto: feedback breve
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Correcto!");
          char buf[16];
          snprintf(buf, sizeof(buf), "Paso %d/%d", (int)(i+1), (int)sequenceVec.size());
          lcd.setCursor(0,1);
          lcd.print(buf);
          vTaskDelay(300 / portTICK_PERIOD_MS);
        }
      }

      if (requestReset) {
        doResetRequested();
        continue;
      }

      if (ok) {
        // Si pasó por toda la secuencia correctamente, agregar un nuevo número y subir de nivel
        level++;
        sequenceVec.push_back(random1to9()); // agregar siguiente número aleatorio

        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Correcto! Nivel:");
        lcd.setCursor(0,1);
        lcd.print(level);
        vTaskDelay(700 / portTICK_PERIOD_MS);

        // Devolver turno a Simon para mostrar la secuencia ampliada
        xSemaphoreGive(semSimonTurn);
      } else {
        // Game over: mostrar mensaje y reiniciar (sin guardar high score porque elegiste B)
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("GAME OVER");
        lcd.setCursor(0,1);
        lcd.print("Nivel alcanzado:");
        lcd.print(level);
        Serial.printf("Game over. Nivel alcanzado: %d", level);
        gameActive = false;
        doResetRequested();
      }
    }
  }
}

// Fin del sketch
