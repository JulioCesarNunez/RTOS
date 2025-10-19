// Configuración de núcleos a utilizar 
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

// Pines de los LEDs
#define LED_ROJO     25
#define LED_AMARILLO 26
#define LED_VERDE    27

static TimerHandle_t auto_reload_timer1 = NULL;  //Inicio Rojo
static TimerHandle_t auto_reload_timer2 = NULL;  //Rojo
static TimerHandle_t auto_reload_timer3 = NULL;  //Inicio Verde
static TimerHandle_t auto_reload_timer4 = NULL;  //Verde
static TimerHandle_t auto_reload_timer5 = NULL;  //Inicio Amarillo
static TimerHandle_t auto_reload_timer6 = NULL;  //Amarillo

int amarillo_count = 0;   // contador de parpadeos

//---- ROJO ----
void myTimerCallback1(TimerHandle_t xTimer){
  if ((uint32_t)pvTimerGetTimerID(xTimer)==1){
    Serial.println("ROJO ON");
    digitalWrite(LED_ROJO, HIGH);  // enciende rojo
    xTimerStart(auto_reload_timer2, portMAX_DELAY);
  }
}	

void myTimerCallback2(TimerHandle_t xTimer){
  if ((uint32_t)pvTimerGetTimerID(xTimer)==1){
    Serial.println("ROJO OFF");
    digitalWrite(LED_ROJO, LOW);   // apaga rojo
    // al terminar rojo → amarillo
    xTimerStart(auto_reload_timer5, portMAX_DELAY);
  }
}	

//---- VERDE ----
void myTimerCallback3(TimerHandle_t xTimer){
  if ((uint32_t)pvTimerGetTimerID(xTimer)==1){
    Serial.println("VERDE ON");
    digitalWrite(LED_VERDE, HIGH); // enciende verde
    xTimerStart(auto_reload_timer4, portMAX_DELAY);
  }
}	

void myTimerCallback4(TimerHandle_t xTimer){
  if ((uint32_t)pvTimerGetTimerID(xTimer)==1){
    Serial.println("VERDE OFF");
    digitalWrite(LED_VERDE, LOW);  // apaga verde
    // al terminar verde → regresa a ROJO
    xTimerStart(auto_reload_timer1, portMAX_DELAY);
  }
}	

//---- AMARILLO ----
void myTimerCallback5(TimerHandle_t xTimer){
  if ((uint32_t)pvTimerGetTimerID(xTimer)==1){
    Serial.println("INICIO AMARILLO");
    amarillo_count = 0;
    xTimerStart(auto_reload_timer6, portMAX_DELAY);
  }
}	

void myTimerCallback6(TimerHandle_t xTimer){
  if ((uint32_t)pvTimerGetTimerID(xTimer)==1){
    // alternar ON/OFF para parpadeo
    if (digitalRead(LED_AMARILLO) == LOW) {
      digitalWrite(LED_AMARILLO, HIGH);
      Serial.println("AMARILLO ON");
    } else {
      digitalWrite(LED_AMARILLO, LOW);
      Serial.println("AMARILLO OFF");
      amarillo_count++;
    }

    if(amarillo_count < 3){
      xTimerStart(auto_reload_timer6, portMAX_DELAY); // parpadea 3 veces
    }else{
      digitalWrite(LED_AMARILLO, LOW); // aseguramos apagado
      // después de amarillo → pasa a verde
      xTimerStart(auto_reload_timer3, portMAX_DELAY);
    }
  }
}	

//---- SETUP ----
void setup(){
  Serial.begin(115200);

  // Configurar pines
  pinMode(LED_ROJO, OUTPUT);
  pinMode(LED_AMARILLO, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);

  // apagar todos al inicio
  digitalWrite(LED_ROJO, LOW);
  digitalWrite(LED_AMARILLO, LOW);
  digitalWrite(LED_VERDE, LOW);

  // ROJO inicio cada ciclo
  auto_reload_timer1 = xTimerCreate(
            "Timer1",
            1 / portTICK_PERIOD_MS,  // 2s rojo
            pdFALSE,
            (void *)1,
            myTimerCallback1);
  xTimerStart(auto_reload_timer1, portMAX_DELAY);

  auto_reload_timer2 = xTimerCreate(
            "Timer2",
            2000 / portTICK_PERIOD_MS,  // dura 2s
            pdFALSE,
            (void *)1,
            myTimerCallback2);

  // VERDE
  auto_reload_timer3 = xTimerCreate(
            "Timer3",
            1 / portTICK_PERIOD_MS,  // 2s verde
            pdFALSE,
            (void *)1,
            myTimerCallback3);

  auto_reload_timer4 = xTimerCreate(
            "Timer4",
            2000 / portTICK_PERIOD_MS,
            pdFALSE,
            (void *)1,
            myTimerCallback4);

  // AMARILLO
  auto_reload_timer5 = xTimerCreate(
            "Timer5",
            200 / portTICK_PERIOD_MS,   // arranque del amarillo
            pdFALSE,
            (void *)1,
            myTimerCallback5);

  auto_reload_timer6 = xTimerCreate(
            "Timer6",
            400 / portTICK_PERIOD_MS,   // cada parpadeo ~0.4s
            pdFALSE,
            (void *)1,
            myTimerCallback6);
}

void loop() {
  // Nada en loop, todo se maneja con timers
}
