// Pines de LEDs para simulación de bootloader
const int LED1 = 2;   // LED integrado
const int LED2 = 4;   // LED externo
const int LED3 = 5;   // LED externo

void setup() {
  Serial.begin(115200);
  
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);

  // Mensaje inicial tipo "Bootloader"
  Serial.println("----------------------------");
  Serial.println("   ESP32 Bootloader Sim");
  Serial.println("   Inicializando sistema...");
  Serial.println("----------------------------");
  delay(1000);

  // Secuencia de encendido de LEDs
  bootSequence();
  
  Serial.println("Sistema listo. Ejecutando programa principal...");
  delay(500);
}

void loop() {
  // Aquí iría tu programa principal, por ejemplo el control de semáforos
}

// Función que simula la secuencia de arranque de un bootloader
void bootSequence() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED1, HIGH);
    delay(200);
    digitalWrite(LED1, LOW);
    
    digitalWrite(LED2, HIGH);
    delay(200);
    digitalWrite(LED2, LOW);
    
    digitalWrite(LED3, HIGH);
    delay(200);
    digitalWrite(LED3, LOW);
  }

  // Secuencia final: todos encendidos 1 segundo
  digitalWrite(LED1, HIGH);
  digitalWrite(LED2, HIGH);
  digitalWrite(LED3, HIGH);
  delay(1000);

  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
}
