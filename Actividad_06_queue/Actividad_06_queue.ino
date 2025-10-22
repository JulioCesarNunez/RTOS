#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

// Tamaños de colas (buffers)
static const uint8_t buffer1_len = 1;
static const uint8_t buffer2_len = 1;

// Definición de colas
static QueueHandle_t buffer1;
static QueueHandle_t buffer2;

// ---------- TAREA 1 ----------
// Envía datos a buffer1 y usa los de buffer2 para calcular el siguiente
void tarea1(void *parameter) {
  int dato_actual = 1;   // Dato inicial
  int dato_recibido = 0;

  while (1) {
    Serial.print("Tarea1 -> Envía a Buffer1: ");
    Serial.println(dato_actual);

    // Envía el dato actual al buffer1 (espera si está lleno)
    xQueueSend(buffer1, &dato_actual, portMAX_DELAY);

    // Espera el resultado de Tarea2 desde buffer2
    xQueueReceive(buffer2, &dato_recibido, portMAX_DELAY);

    Serial.print("Tarea1 <- Recibe desde Buffer2: ");
    Serial.println(dato_recibido);

    // Usa el valor recibido como base para el siguiente cálculo
    // (por ejemplo, suma 1 al resultado para continuar el ciclo)
    dato_actual = dato_recibido ;

    Serial.print("Tarea1 -> Nuevo dato generado: ");
    Serial.println(dato_actual);
    Serial.println("---------------------------------");

    // Pausa entre ciclos (opcional)
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

// ---------- TAREA 2 ----------
// Recibe datos de buffer1, los procesa y los envía a buffer2
void tarea2(void *parameter) {
  int dato_recibido = 0;
  int dato_procesado = 0;

  while (1) {
    // Espera un dato desde buffer1
    xQueueReceive(buffer1, &dato_recibido, portMAX_DELAY);
    Serial.print("Tarea2 <- Recibe desde Buffer1: ");
    Serial.println(dato_recibido);

    // Procesamiento: por ejemplo, multiplicar por 2
    dato_procesado = dato_recibido * 2;

    Serial.print("Tarea2 -> Procesa y envía a Buffer2: ");
    Serial.println(dato_procesado);

    // Envía el dato procesado al buffer2
    xQueueSend(buffer2, &dato_procesado, portMAX_DELAY);

    // Simula tiempo de trabajo
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);
  vTaskDelay(2000 / portTICK_PERIOD_MS);
  Serial.println("=== Sistema FreeRTOS: Ciclo Sincrónico con Realimentación ===");
  Serial.println("Flujo: Tarea1 → Buffer1 → Tarea2 → Buffer2 → Tarea1");
  Serial.println("-------------------------------------------------------------");

  // Crear colas (buffers)
  buffer1 = xQueueCreate(buffer1_len, sizeof(int));
  buffer2 = xQueueCreate(buffer2_len, sizeof(int));

  if (buffer1 == NULL || buffer2 == NULL) {
    Serial.println("Error al crear las colas");
    while (1);
  }

  // Crear tareas
  xTaskCreatePinnedToCore(tarea1, "Tarea1", 2048, NULL, 1, NULL, app_cpu);
  xTaskCreatePinnedToCore(tarea2, "Tarea2", 2048, NULL, 1, NULL, app_cpu);
}

void loop() {
  // No se usa loop()
}
