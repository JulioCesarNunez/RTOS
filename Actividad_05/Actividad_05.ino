// Configuración de núcleos a utilizar
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0; // Si se usa un solo núcleo, se asigna al núcleo 0
#else
  static const BaseType_t app_cpu = 1; // Si se usan dos núcleos, se asigna al núcleo 1
#endif

// ==================================================
// PARPADEO LIBRE DE 3 LEDS
// ==================================================
// Definición de los pines a los que están conectados los LEDs
const int led1 = 5;  // LED que parpadea cada 500ms
const int led2 = 6;  // LED que parpadea cada 325ms
const int led3 = 7;  // LED que parpadea cada 180ms

// Definición de los tiempos de parpadeo de los LEDs
const unsigned long rate_1 = 500; // 500ms para LED1
const unsigned long rate_2 = 325; // 325ms para LED2
const unsigned long rate_3 = 180; // 180ms para LED3

// ==================================================
// SECUENCIA DE BOTONES
// ==================================================
// Definición de los pines para los botones y el LED que indica la secuencia
const int boton1 = 2;
const int boton2 = 3;
const int ledSecuencia = 4;

// Variables para controlar el estado de la secuencia
int estado = 0;
unsigned long inicioPaso = 0;  // Tiempo cuando comienza el paso de la secuencia
unsigned long inicioLed = 0;   // Tiempo cuando se enciende el LED de secuencia

// Función para hacer parpadear el LED 1
void toggleLED1(void *parameter){
  while(1){ // Bucle infinito que se repite mientras la tarea esté activa
    digitalWrite(led1, HIGH);  // Enciende el LED
    vTaskDelay(rate_1 / portTICK_PERIOD_MS);  // Espera el tiempo de parpadeo
    digitalWrite(led1, LOW);   // Apaga el LED
    vTaskDelay(rate_1 / portTICK_PERIOD_MS);  // Espera el tiempo de parpadeo
  }
}

// Función para hacer parpadear el LED 2
void toggleLED2(void *parameter){
  while(1){
    digitalWrite(led2, HIGH);  // Enciende el LED
    vTaskDelay(rate_2 / portTICK_PERIOD_MS);  // Espera el tiempo de parpadeo
    digitalWrite(led2, LOW);   // Apaga el LED
    vTaskDelay(rate_2 / portTICK_PERIOD_MS);  // Espera el tiempo de parpadeo
  }
}

// Función para hacer parpadear el LED 3
void toggleLED3(void *parameter){
  while(1){
    digitalWrite(led3, HIGH);  // Enciende el LED
    vTaskDelay(rate_3 / portTICK_PERIOD_MS);  // Espera el tiempo de parpadeo
    digitalWrite(led3, LOW);   // Apaga el LED
    vTaskDelay(rate_3 / portTICK_PERIOD_MS);  // Espera el tiempo de parpadeo
  }
}

// Función que maneja la secuencia de botones
void SecuenciaBotones(void *parameter){
  while(1){  // Bucle infinito que se repite mientras la tarea esté activa
    unsigned long tiempoActual = millis();  // Obtiene el tiempo actual en milisegundos
    int b1 = digitalRead(boton1); // Lee el estado del botón 1 (1 = no presionado, 0 = presionado)
    int b2 = digitalRead(boton2); // Lee el estado del botón 2

    // Lógica de los diferentes pasos de la secuencia de botones
    switch (estado) {
      case 0: // Paso 1: Ambos botones sin presionar por al menos 1 segundo
        if (b1 == HIGH && b2 == HIGH) {
          if (inicioPaso == 0) inicioPaso = tiempoActual;
          if (tiempoActual - inicioPaso >= 1000) {  // Si ha pasado 1 segundo
            estado = 1;  // Cambia al siguiente paso
            inicioPaso = 0;
            Serial.println("Paso 1 correcto");
          }
        } else {
          inicioPaso = 0; // Reinicia el contador si alguno de los botones se presiona
        }
        break;

      case 1: // Paso 2: Botón 1 presionado, botón 2 sin presionar por al menos 1 segundo
        if (b1 == LOW && b2 == HIGH) {
          if (inicioPaso == 0) inicioPaso = tiempoActual;
          if (tiempoActual - inicioPaso >= 1000) {  // Si ha pasado 1 segundo
            estado = 2;  // Cambia al siguiente paso
            inicioPaso = 0;
            Serial.println("Paso 2 correcto");
          }
        } else {
          inicioPaso = 0;
        }
        break;

      case 2: // Paso 3: Ambos botones presionados por al menos 1 segundo
        if (b1 == LOW && b2 == LOW) {
          if (inicioPaso == 0) inicioPaso = tiempoActual;
          if (tiempoActual - inicioPaso >= 1000) {  // Si ha pasado 1 segundo
            estado = 3;  // Cambia al siguiente paso
            inicioPaso = 0;
            Serial.println("Paso 3 correcto");
          }
        } else {
          inicioPaso = 0;
        }
        break;

      case 3: // Paso 4: Botón 1 presionado, botón 2 NO presionado por al menos 1 segundo
        if (b1 == LOW && b2 == HIGH) {
          if (inicioPaso == 0) inicioPaso = tiempoActual;
          if (tiempoActual - inicioPaso >= 1000) {  // Si ha pasado 1 segundo
            estado = 4;  // Cambia al siguiente paso
            inicioPaso = 0;
            Serial.println("Paso 4 correcto");
          }
        } else {
          inicioPaso = 0;
        }
        break;

      case 4: // Paso 5: Ambos botones NO presionados por al menos 1 segundo
        if (b1 == HIGH && b2 == HIGH) {
          if (inicioPaso == 0) inicioPaso = tiempoActual;
          if (tiempoActual - inicioPaso >= 1000) {  // Si ha pasado 1 segundo
            estado = 5;  // Cambia al siguiente paso
            inicioPaso = 0;
            inicioLed = tiempoActual;
            digitalWrite(ledSecuencia, HIGH);  // Enciende el LED de secuencia
            Serial.println("Secuencia completa - LED encendido 3s");
          }
        } else {
          inicioPaso = 0;
        }
        break;

      case 5: // LED encendido por 3 segundos
        if (tiempoActual - inicioLed >= 3000) {  // Si ha pasado 3 segundos
          digitalWrite(ledSecuencia, LOW);  // Apaga el LED de secuencia
          estado = 0;  // Reinicia la secuencia
          Serial.println("LED apagado, reiniciando secuencia");
        }
        break;
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);  // Espera 10ms para evitar bloqueo de la CPU
  }
}

void setup() {
  pinMode(led1, OUTPUT);  // Configura los pines de los LEDs como salidas
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);

  pinMode(boton1, INPUT_PULLUP);  // Configura los botones con pull-up interno
  pinMode(boton2, INPUT_PULLUP);
  pinMode(ledSecuencia, OUTPUT);  // Configura el LED de secuencia como salida
  digitalWrite(ledSecuencia, LOW);  // Apaga el LED de secuencia

  Serial.begin(9600);  // Inicia la comunicación serial

  // Crea las tareas de FreeRTOS para los LEDs y la secuencia de botones
  xTaskCreatePinnedToCore(toggleLED1, "ToggleLED1", 1024, NULL, 1, NULL, app_cpu);
  xTaskCreatePinnedToCore(toggleLED2, "ToggleLED2", 1024, NULL, 2, NULL, app_cpu);
  xTaskCreatePinnedToCore(toggleLED3, "ToggleLED3", 1024, NULL, 1, NULL, app_cpu);
  xTaskCreatePinnedToCore(SecuenciaBotones, "SecuenciaBotones", 1024, NULL, 3, NULL, app_cpu);
}

void loop() {
  // No es necesario usar loop con FreeRTOS en este caso
}
