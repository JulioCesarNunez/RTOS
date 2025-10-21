// ============================================================================
//      SECUENCIA DE 5 LEDS CONTROLADOS POR 5 BOTONES USANDO FREERTOS
// ============================================================================
//  Tarea 1: al presionar y soltar el botón 1 → enciende LED1
//  Tarea 2: al presionar y soltar el botón 2 → enciende LED2 (LED1 sigue encendido)
//  Tarea 3: al presionar y soltar el botón 3 → enciende LED3 (LED1 y LED2 siguen encendidos)
//  Tarea 4: al presionar y soltar el botón 4 → enciende LED4 y apaga LED1 y LED2
//  Tarea 5: al presionar y soltar el botón 5 → enciende LED5 durante 5 segundos y apaga todos
// ============================================================================

#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

// -------------------- Pines --------------------
const int boton1 = 15;
const int boton2 = 2;
const int boton3 = 4;
const int boton4 = 5;
const int boton5 = 18;

const int led1 = 19;
const int led2 = 21;
const int led3 = 22;
const int led4 = 23;
const int led5 = 25;

// -------------------- Handles de tareas --------------------
TaskHandle_t tarea1Handle = NULL;
TaskHandle_t tarea2Handle = NULL;
TaskHandle_t tarea3Handle = NULL;
TaskHandle_t tarea4Handle = NULL;
TaskHandle_t tarea5Handle = NULL;

// ============================================================================
//                              TAREAS
// ============================================================================

// -------- TAREA 1 --------
void tarea1(void *parameter) {
  while (1) {
    if (digitalRead(boton1) == LOW) {          // Botón presionado (Pull-up)
      while (digitalRead(boton1) == LOW);      // Espera a que se suelte
      digitalWrite(led1, HIGH);                // Enciende LED1
      Serial.println("LED1 encendido");
      vTaskDelay(300 / portTICK_PERIOD_MS);    // Pequeño retardo
      vTaskSuspend(tarea1Handle);              // Suspende esta tarea
      vTaskResume(tarea2Handle);               // Activa la siguiente
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

// -------- TAREA 2 --------
void tarea2(void *parameter) {
  while (1) {
    if (digitalRead(boton2) == LOW) {
      while (digitalRead(boton2) == LOW);
      digitalWrite(led2, HIGH);                // Enciende LED2
      Serial.println("LED2 encendido (LED1 sigue encendido)");
      vTaskDelay(300 / portTICK_PERIOD_MS);
      vTaskSuspend(tarea2Handle);
      vTaskResume(tarea3Handle);
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

// -------- TAREA 3 --------
void tarea3(void *parameter) {
  while (1) {
    if (digitalRead(boton3) == LOW) {
      while (digitalRead(boton3) == LOW);
      digitalWrite(led3, HIGH);                // Enciende LED3
      Serial.println("LED3 encendido (LED1 y LED2 siguen encendidos)");
      vTaskDelay(300 / portTICK_PERIOD_MS);
      vTaskSuspend(tarea3Handle);
      vTaskResume(tarea4Handle);
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

// -------- TAREA 4 --------
void tarea4(void *parameter) {
  while (1) {
    if (digitalRead(boton4) == LOW) {
      while (digitalRead(boton4) == LOW);
      digitalWrite(led4, HIGH);                // Enciende LED4
      digitalWrite(led1, LOW);                 // Apaga LED1
      digitalWrite(led2, LOW);                 // Apaga LED2
      Serial.println("LED4 encendido, LED1 y LED2 apagados");
      vTaskDelay(300 / portTICK_PERIOD_MS);
      vTaskSuspend(tarea4Handle);
      vTaskResume(tarea5Handle);
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

// -------- TAREA 5 --------
void tarea5(void *parameter) {
  while (1) {
    if (digitalRead(boton5) == LOW) {
      while (digitalRead(boton5) == LOW);
      digitalWrite(led5, HIGH);                // Enciende LED5
      Serial.println("LED5 encendido por 5 segundos...");
      vTaskDelay(5000 / portTICK_PERIOD_MS);   // Espera 5 segundos
      // Apaga todos los LEDs
      digitalWrite(led1, LOW);
      digitalWrite(led2, LOW);
      digitalWrite(led3, LOW);
      digitalWrite(led4, LOW);
      digitalWrite(led5, LOW);
      Serial.println("Todos los LEDs apagados. Secuencia terminada.");
      vTaskSuspend(tarea5Handle);              // Termina la secuencia
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

// ============================================================================
//                              SETUP
// ============================================================================
void setup() {
  Serial.begin(115200);
  Serial.println("Secuencia de 5 LEDs con 5 botones - ESP32 + FreeRTOS");

  // Configurar botones como entradas con resistencia pull-up interna
  pinMode(boton1, INPUT_PULLUP);
  pinMode(boton2, INPUT_PULLUP);
  pinMode(boton3, INPUT_PULLUP);
  pinMode(boton4, INPUT_PULLUP);
  pinMode(boton5, INPUT_PULLUP);

  // Configurar LEDs como salidas
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);
  pinMode(led5, OUTPUT);

  // Inicialmente todos los LEDs apagados
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
  digitalWrite(led3, LOW);
  digitalWrite(led4, LOW);
  digitalWrite(led5, LOW);

  // Crear tareas
  xTaskCreatePinnedToCore(tarea1, "Tarea1", 2048, NULL, 1, &tarea1Handle, app_cpu);
  xTaskCreatePinnedToCore(tarea2, "Tarea2", 2048, NULL, 1, &tarea2Handle, app_cpu);
  xTaskCreatePinnedToCore(tarea3, "Tarea3", 2048, NULL, 1, &tarea3Handle, app_cpu);
  xTaskCreatePinnedToCore(tarea4, "Tarea4", 2048, NULL, 1, &tarea4Handle, app_cpu);
  xTaskCreatePinnedToCore(tarea5, "Tarea5", 2048, NULL, 1, &tarea5Handle, app_cpu);

  // Solo la primera tarea activa al inicio
  vTaskSuspend(tarea2Handle);
  vTaskSuspend(tarea3Handle);
  vTaskSuspend(tarea4Handle);
  vTaskSuspend(tarea5Handle);
}

// ============================================================================
//                              LOOP
// ============================================================================
void loop() {
  // No se usa loop(), FreeRTOS maneja las tareas
}
