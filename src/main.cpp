#include <Arduino.h>

// Definición de pines de entrada (con pull-up, se activan con LOW)
#define PIN_CLEAN    0
#define PIN_CRUNCH   1
#define PIN_LEAD     2
#define PIN_REVERB   3

// Definición de pines de salida para drivers
#define PIN_DRIVER1  4
#define PIN_DRIVER2  5
#define PIN_DRIVER4  6

// Definición de pines de salida para LEDs (se activan con HIGH)
#define PIN_LED_CLEAN   7
#define PIN_LED_CRUNCH  10
#define PIN_LED_LEAD    9
#define PIN_LED_REVERB  21

// Pin para señal de cambio de canal (se activa por 100ms)
#define PIN_CHANGE_SIGNAL 8

// Estados posibles del canal
enum Channel {
  CHANNEL_CLEAN,
  CHANNEL_CRUNCH,
  CHANNEL_LEAD
};

// Variables globales de estado
Channel selectedChannel = CHANNEL_CLEAN;
bool reverbActive = false;

// Variables para almacenar el estado anterior de los botones (para detección de flanco)
bool lastCleanState = HIGH;
bool lastCrunchState = HIGH;
bool lastLeadState = HIGH;
bool lastReverbState = HIGH;

// Variables para el estado anterior de driver2 (para mantenerlo en modo CLEAN)
bool lastDriver2State = LOW;

// Variables para control de señal de cambio de canal
bool changeSignalActive = false;
unsigned long changeSignalStartTime = 0;
const unsigned long changeSignalDuration = 100; // 100ms de activación

// Tiempo de debounce (en milisegundos)
const unsigned long debounceDelay = 50;
unsigned long lastDebounceTime = 0;

void setup() {
  // Inicializar comunicación serial para depuración
  Serial.begin(115200);
  Serial.println("Iniciando footswitch...");

  // Configurar pines de entrada con pull-up interno
  pinMode(PIN_CLEAN, INPUT_PULLUP);
  pinMode(PIN_CRUNCH, INPUT_PULLUP);
  pinMode(PIN_LEAD, INPUT_PULLUP);
  pinMode(PIN_REVERB, INPUT_PULLUP);

  // Configurar pines de salida para drivers
  pinMode(PIN_DRIVER1, OUTPUT);
  pinMode(PIN_DRIVER2, OUTPUT);
  pinMode(PIN_DRIVER4, OUTPUT);

  // Configurar pines de salida para LEDs
  pinMode(PIN_LED_CLEAN, OUTPUT);
  pinMode(PIN_LED_CRUNCH, OUTPUT);
  pinMode(PIN_LED_LEAD, OUTPUT);
  pinMode(PIN_LED_REVERB, OUTPUT);

  // Configurar pin para señal de cambio de canal
  pinMode(PIN_CHANGE_SIGNAL, OUTPUT);
  digitalWrite(PIN_CHANGE_SIGNAL, HIGH); // Inicialmente inactivo (HIGH)

  // Estado inicial: LED clean encendido (HIGH), otros apagados (LOW)
  digitalWrite(PIN_LED_CLEAN, HIGH);    // CLEAN activo por defecto
  digitalWrite(PIN_LED_CRUNCH, LOW);
  digitalWrite(PIN_LED_LEAD, LOW);
  digitalWrite(PIN_LED_REVERB, LOW);

  // Drivers en estado inicial según tabla de verdad para CLEAN
  digitalWrite(PIN_DRIVER1, HIGH);     // CLEAN: driver1 = 1
  digitalWrite(PIN_DRIVER2, LOW);      // Estado inicial LOW
  digitalWrite(PIN_DRIVER4, LOW);      // Reverb desactivado por defecto

  // Leer estado inicial de driver2 para mantenerlo en modo CLEAN
  lastDriver2State = digitalRead(PIN_DRIVER2);
}

void updateChannel(Channel newChannel) {
  // Solo actualizar si el canal ha cambiado
  if (selectedChannel != newChannel) {
    selectedChannel = newChannel;
    
    // Activar señal de cambio de canal (nivel bajo por poco tiempo)
    digitalWrite(PIN_CHANGE_SIGNAL, LOW);
    changeSignalActive = true;
    changeSignalStartTime = millis();
    
    Serial.print("Canal cambiado a: ");
    switch (selectedChannel) {
      case CHANNEL_CLEAN: Serial.println("CLEAN"); break;
      case CHANNEL_CRUNCH: Serial.println("CRUNCH"); break;
      case CHANNEL_LEAD: Serial.println("LEAD"); break;
    }
  }
}

void updateReverb(bool active) {
  if (reverbActive != active) {
    reverbActive = active;
    Serial.print("Reverb: ");
    Serial.println(reverbActive ? "ACTIVADO" : "DESACTIVADO");
  }
}

void updateDrivers() {
  // Aplicar tabla de verdad para driver1 y driver2 según el canal seleccionado
  switch (selectedChannel) {
    case CHANNEL_CLEAN:
      // clean: driver1 = 1, driver2 = X (no cambiar estado anterior)
      digitalWrite(PIN_DRIVER1, HIGH);
      // No cambiamos driver2, mantenemos el estado anterior
      break;
      
    case CHANNEL_CRUNCH:
      // crunch: driver1 = 0, driver2 = 1
      digitalWrite(PIN_DRIVER1, LOW);
      digitalWrite(PIN_DRIVER2, HIGH);
      lastDriver2State = HIGH; // Actualizar estado recordado
      break;
      
    case CHANNEL_LEAD:
      // lead: driver1 = 0, driver2 = 0
      digitalWrite(PIN_DRIVER1, LOW);
      digitalWrite(PIN_DRIVER2, LOW);
      lastDriver2State = LOW; // Actualizar estado recordado
      break;
  }

  // Para el modo CLEAN, necesitamos mantener el estado anterior de driver2
  if (selectedChannel == CHANNEL_CLEAN) {
    digitalWrite(PIN_DRIVER2, lastDriver2State);
  }

  // Control de driver4 según estado de reverb
  // reverb: si reverb está activado, driver4 = 1
  digitalWrite(PIN_DRIVER4, reverbActive ? HIGH : LOW);
}

void updateLEDs() {
  // Actualizar LEDs según el estado actual
  // Los LEDs se activan con HIGH (se encienden cuando el pin está en HIGH)
  digitalWrite(PIN_LED_CLEAN, (selectedChannel == CHANNEL_CLEAN) ? HIGH : LOW);
  digitalWrite(PIN_LED_CRUNCH, (selectedChannel == CHANNEL_CRUNCH) ? HIGH : LOW);
  digitalWrite(PIN_LED_LEAD, (selectedChannel == CHANNEL_LEAD) ? HIGH : LOW);
  digitalWrite(PIN_LED_REVERB, reverbActive ? HIGH : LOW);
}

void readInputs() {
  // Leer estado actual de los botones (con pull-up, activos en LOW)
  bool currentCleanState = digitalRead(PIN_CLEAN);
  bool currentCrunchState = digitalRead(PIN_CRUNCH);
  bool currentLeadState = digitalRead(PIN_LEAD);
  bool currentReverbState = digitalRead(PIN_REVERB);

  // Aplicar debounce simple basado en tiempo
  unsigned long currentTime = millis();
  if (currentTime - lastDebounceTime > debounceDelay) {
    lastDebounceTime = currentTime;

    // Detectar flanco descendente (botón presionado, cambio de HIGH a LOW)
    if (lastCleanState == HIGH && currentCleanState == LOW) {
      updateChannel(CHANNEL_CLEAN);
    }
    if (lastCrunchState == HIGH && currentCrunchState == LOW) {
      updateChannel(CHANNEL_CRUNCH);
    }
    if (lastLeadState == HIGH && currentLeadState == LOW) {
      updateChannel(CHANNEL_LEAD);
    }
    if (lastReverbState == HIGH && currentReverbState == LOW) {
      // El botón de reverb funciona como toggle
      updateReverb(!reverbActive);
    }

    // Actualizar estados anteriores
    lastCleanState = currentCleanState;
    lastCrunchState = currentCrunchState;
    lastLeadState = currentLeadState;
    lastReverbState = currentReverbState;
  }
}

void loop() {
  // Leer entradas y procesar cambios
  readInputs();

  // Actualizar drivers según el estado actual
  updateDrivers();

  // Actualizar LEDs según el estado actual
  updateLEDs();

  // Controlar señal de cambio de canal (activada por tiempo limitado)
  if (changeSignalActive) {
    unsigned long currentTime = millis();
    if (currentTime - changeSignalStartTime >= changeSignalDuration) {
      digitalWrite(PIN_CHANGE_SIGNAL, HIGH); // Desactivar señal
      changeSignalActive = false;
    }
  }

  // Pequeña pausa para estabilidad
  delay(10);
}
