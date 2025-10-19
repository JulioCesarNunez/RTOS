#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

//definir los leds
const int ROJO_01 = 15;
const int ROJO_02 = 0;
const int VERDE_01 = 2;
const int VERDE_02 = 16;



//definir pin del boton 
const int botonPin = 19;
const int botonPin2 = 22;
const int botonPin3 = 23;
const int botonPin4 = 17 ;


//Estaods actuales
int currentButtonState1 =0; //ariable para almacenar el estado del botón1
int currentButtonState2 =0; //ariable para almacenar el estado del botón3
int currentButtonState3 =0; //ariable para almacenar el estado del botón2
int currentButtonState4 =0; //ariable para almacenar el estado del botón4


//Estados anteriores
int lastButtonState1 = 0; // Vaariable para almacenar el estado del botón2riable para almacenar el estado anterior del botón 1
int lastButtonState2 = 0; // Vaariable para almacenar el estado del botón2riable para almacenar el estado anterior del botón 2
int lastButtonState3 = 0; // Vaariable para almacenar el estado del botón2riable para almacenar el estado anterior del botón 3
int lastButtonState4 = 0; // Vaariable para almacenar el estado del botón2riable para almacenar el estado anterior del botón 4


// Prototipos de tareas
//Checar cual es el primero que toca el primer sensor y dependiento de cual toca primero es la secuencia de estados que va a seguir, los sensores seran swiches para simular el funcionamiento del sensor 
//y checar lo de la lista de espera 
void estado01(void *parameters);
void estado02(void *parameters);
void estado03(void *parameters);
void estado04(void *parameters);

static SemaphoreHandle_t xSemaphore1,xSemaphore2,xSemaphore3,xSemaphore4;


void setup() {
  Serial.begin(115200);

  // Configurar pines de los LEDs como salidas
  pinMode(ROJO_01, OUTPUT);
  pinMode(VERDE_01, OUTPUT);
  pinMode(ROJO_02, OUTPUT);
  pinMode(VERDE_02, OUTPUT);

  //Creamos los semaforos 
  xSemaphore1 = xSemaphoreCreateBinary();
  xSemaphore2 = xSemaphoreCreateBinary();
  xSemaphore3 = xSemaphoreCreateBinary();
  xSemaphore4 = xSemaphoreCreateBinary();
  
  
  // Creamos las tareas 
  xTaskCreatePinnedToCore(estado01,"estado01",1024,NULL,1,NULL,app_cpu);
  xTaskCreatePinnedToCore(estado02,"estado02",1024,NULL,1,NULL,app_cpu);
  xTaskCreatePinnedToCore(estado03,"estado03",1024,NULL,1,NULL,app_cpu);
  xTaskCreatePinnedToCore(estado04,"estado04",1024,NULL,1,NULL,app_cpu);
  
  
  xSemaphoreGive(xSemaphore1); //habilitamos el semaforo 1
   
}

void loop() {
   
   //no usamos esta seccion 
}



void estado01(void *parameters){
 static bool lastButtonState1 = HIGH;  // Estado anterior del botón (con pull-up el estado inicial es HIGH)
  bool currentButtonState1;
   static bool lastButtonState4 = HIGH;  // Estado anterior del botón (con pull-up el estado inicial es HIGH)
  bool currentButtonState4;
  
  while (1) {
   currentButtonState1 = digitalRead(botonPin);  // Leemos el estado del botón
   currentButtonState4 = digitalRead(botonPin4);  // Leemos el estado del botón

    // Verifica si el botón fue presionado (de HIGH a LOW)
    if (lastButtonState1 == HIGH && currentButtonState1 == LOW) {
      // Se presionó el botón
      xSemaphoreTake(xSemaphore1, portMAX_DELAY); // Toma el semáforo
      digitalWrite(ROJO_01, HIGH);
      digitalWrite(VERDE_02, HIGH);
      digitalWrite(ROJO_02, LOW);
      digitalWrite(VERDE_01, LOW);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      }
if (lastButtonState4 == HIGH && currentButtonState4 == LOW) {
    vTaskDelay(100 / portTICK_PERIOD_MS);
    xSemaphoreGive(xSemaphore2);
     digitalWrite(ROJO_01, LOW);
    digitalWrite(VERDE_02, LOW);
    digitalWrite(ROJO_02, LOW);
      digitalWrite(VERDE_01, LOW);

    }
  lastButtonState1 = currentButtonState1;  // Actualiza el estado anterior del botón
  lastButtonState4 = currentButtonState4;  // Actualiza el estado anterior del botón
    vTaskDelay(20 / portTICK_PERIOD_MS);  // Pequeña espera para evitar rebotes
  }
}

void estado02(void *parameters){
static bool lastButtonState2 = HIGH;  // Estado anterior del botón (con pull-up el estado inicial es HIGH)
  bool currentButtonState2;
   static bool lastButtonState3 = HIGH;  // Estado anterior del botón (con pull-up el estado inicial es HIGH)
  bool currentButtonState3;
  while (1) {
   currentButtonState2 = digitalRead(botonPin2);  // Leemos el estado del botón
   currentButtonState3 = digitalRead(botonPin3);  // Leemos el estado del botón
  if (lastButtonState3 == HIGH && currentButtonState3 == LOW) {
    xSemaphoreTake(xSemaphore2, portMAX_DELAY);
    digitalWrite(ROJO_02, HIGH);
    digitalWrite(VERDE_01, HIGH);
    digitalWrite(ROJO_01, LOW);
    digitalWrite(VERDE_02, LOW);
    vTaskDelay(100 / portTICK_PERIOD_MS);}

    if (lastButtonState2 == HIGH && currentButtonState2 == LOW) {
    vTaskDelay(10 / portTICK_PERIOD_MS);
    xSemaphoreGive(xSemaphore3);
     digitalWrite(ROJO_02, LOW);
    digitalWrite(VERDE_01, LOW);
    digitalWrite(ROJO_01, LOW);
      digitalWrite(VERDE_02, LOW);
    
   
    
    }
  lastButtonState2 = currentButtonState2;  // Actualiza el estado anterior del botón
  lastButtonState3 = currentButtonState3;  // Actualiza el estado anterior del botón
    vTaskDelay(20 / portTICK_PERIOD_MS);  // Pequeña espera para evitar rebotes
  }
}

void estado03(void *parameters){
 static bool lastButtonState3 = HIGH;  // Estado anterior del botón (con pull-up el estado inicial es HIGH)
  bool currentButtonState3;
   static bool lastButtonState2 = HIGH;  // Estado anterior del botón (con pull-up el estado inicial es HIGH)
  bool currentButtonState2;
  while (1) {
   currentButtonState3 = digitalRead(botonPin3);  // Leemos el estado del botón
   currentButtonState2 = digitalRead(botonPin2);  // Leemos el estado del botón

    // Verifica si el botón fue presionado (de HIGH a LOW)
    if (lastButtonState3 == HIGH && currentButtonState3 == LOW) {
      // Se presionó el botón
      xSemaphoreTake(xSemaphore3, portMAX_DELAY); // Toma el semáforo
      digitalWrite(ROJO_02, HIGH);
      digitalWrite(VERDE_01, HIGH);
      digitalWrite(ROJO_01, LOW);
    digitalWrite(VERDE_02, LOW);
   
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      }

if (lastButtonState2 == HIGH && currentButtonState2 == LOW) {
    vTaskDelay(100 / portTICK_PERIOD_MS);
    xSemaphoreGive(xSemaphore4);
    digitalWrite(ROJO_02, LOW);
    digitalWrite(VERDE_01, LOW);
    digitalWrite(ROJO_01, LOW);
      digitalWrite(VERDE_02, LOW);
   
  
    }
  lastButtonState3 = currentButtonState3;  // Actualiza el estado anterior del botón
  lastButtonState2 = currentButtonState2;  // Actualiza el estado anterior del botón
    vTaskDelay(20 / portTICK_PERIOD_MS);  // Pequeña espera para evitar rebotes
  }
}

void estado04(void *parameters){
static bool lastButtonState1 = HIGH;  // Estado anterior del botón (con pull-up el estado inicial es HIGH)
  bool currentButtonState1;
   static bool lastButtonState4 = HIGH;  // Estado anterior del botón (con pull-up el estado inicial es HIGH)
  bool currentButtonState4;
  while (1) {
   currentButtonState1 = digitalRead(botonPin);  // Leemos el estado del botón
   currentButtonState4 = digitalRead(botonPin4);  // Leemos el estado del botón
  if (lastButtonState1 == HIGH && currentButtonState1 == LOW) {
    xSemaphoreTake(xSemaphore4, portMAX_DELAY);
    digitalWrite(ROJO_01, HIGH);
    digitalWrite(VERDE_02, HIGH);
    digitalWrite(ROJO_02, LOW);
    digitalWrite(VERDE_01, LOW);
    vTaskDelay(1000 / portTICK_PERIOD_MS);}

    if (lastButtonState4 == HIGH && currentButtonState4 == LOW) {
    vTaskDelay(100 / portTICK_PERIOD_MS);
    xSemaphoreGive(xSemaphore1);
     digitalWrite(ROJO_01, LOW);
    digitalWrite(VERDE_02, LOW);
    digitalWrite(ROJO_02, LOW);
    digitalWrite(VERDE_01, LOW);
    
    }
  lastButtonState1 = currentButtonState1;  // Actualiza el estado anterior del botón
  lastButtonState4 = currentButtonState4;  // Actualiza el estado anterior del botón
    vTaskDelay(20 / portTICK_PERIOD_MS);  // Pequeña espera para evitar rebotes
  }
}

/*
static SemaphoreHandle_t xSemaphore1;
xSemaphore1 = xSemaphoreCreateBinary();
// Libera el semáforo (Reset)
  xSemaphoreGive(xSemaphore1);
  // Espera que el semaforo este disponible (Set)
  xSemaphoreTake(xSemaphore1, portMAX_DELAY);

*/