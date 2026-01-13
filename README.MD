# C3_FootSwitch_Tito - Controlador de Pedal para Amplificador

Este proyecto implementa un controlador de pedal (footswitch) para el amplificador marshall jcm 2000 tsl 60 de guitarra basado en ESP32-C3.
Permite seleccionar entre tres canales (clean, crunch, lead) y activar/desactivar reverb mediante pulsadores, controlando drivers de relé y LEDs indicadores.

## Características

- **4 entradas con pull-up** activadas con nivel bajo (LOW)
- **3 salidas para drivers** de relé (driver1, driver2, driver4)
- **1 driver reservado** (driver5) para loop de efectos (no implementado en esta versión)
- **4 salidas para LEDs** indicadores (normalmente HIGH, se activan con LOW)
- **Selección de canal** (clean, crunch, lead) con CLEAN por defecto
- **Control de reverb** con toggle (activar/desactivar)
- **Debounce** software para evitar rebotes
- **Comunicación serial** para depuración (115200 baudios)

## Esquema de Pines

### Entradas (INPUT_PULLUP, activas en LOW)
- **GPIO 0**: Botón CLEAN
- **GPIO 1**: Botón CRUNCH
- **GPIO 2**: Botón LEAD
- **GPIO 3**: Botón REVERB (toggle)

### Salidas para Drivers (OUTPUT)
- **GPIO 4**: DRIVER1
- **GPIO 5**: DRIVER2
- **GPIO 6**: DRIVER4 (para reverb)

**Nota importante:** DRIVER5 (para loop de efectos) no está asignado a ningún pin GPIO en esta versión del programa. Está reservado para futuras implementaciones.

### Salidas para LEDs (OUTPUT, normalmente HIGH, activos en LOW)
- **GPIO 7**: LED CLEAN
- **GPIO 10**: LED CRUNCH
- **GPIO 20**: LED LEAD
- **GPIO 21**: LED REVERB

## Tabla de Verdad

### Control de Drivers (DRIVER1 y DRIVER2)

| Canal   | DRIVER1 | DRIVER2             |
|---------|---------|---------------------|
| CLEAN   | 1 (HIGH)| X (mantiene estado anterior) |
| CRUNCH  | 0 (LOW) | 1 (HIGH)            |
| LEAD    | 0 (LOW) | 0 (LOW)             |

### Control de DRIVER4 (Reverb)
- **DRIVER4 = 1 (HIGH)** cuando reverb está activado
- **DRIVER4 = 0 (LOW)** cuando reverb está desactivado

### Control de LEDs
- Cada LED se enciende (LOW) cuando su función correspondiente está activa:
- LED CLEAN: LOW cuando canal = CLEAN
- LED CRUNCH: LOW cuando canal = CRUNCH
- LED LEAD: LOW cuando canal = LEAD
- LED REVERB: LOW cuando reverb = true

## Funcionamiento

### Inicialización
1. Todos los pines se configuran según su función (entradas con pull-up o salidas)
2. LED CLEAN se enciende (LOW), otros LEDs apagados (HIGH)
3. Drivers en estado inicial según tabla de verdad para CLEAN
4. Se inicia la comunicación serial a 115200 baudios

### Lógica Principal
1. **Lectura de entradas**: Se leen los botones cada 10ms con debounce de 50ms
2. **Detección de flancos**: Se detectan transiciones HIGH→LOW (pulsación)
3. **Actualización de estado**:
   - Botones CLEAN/CRUNCH/LEAD: Cambian el canal seleccionado
   - Botón REVERB: Alterna el estado de reverb (toggle)
4. **Control de salidas**:
   - Se aplica la tabla de verdad a los drivers
   - Se actualizan los LEDs según el estado actual
5. **Depuración**: Se envían mensajes por serial cuando cambian los estados

### Comportamiento Especial
- **Modo CLEAN**: DRIVER2 mantiene su estado anterior (no se modifica)
- **Debounce**: Evita lecturas falsas por rebotes mecánicos
- **Estado inicial**: CLEAN seleccionado por defecto, reverb desactivado
- **DRIVER5**: Reservado para loop de efectos (no implementado en esta versión)

## Estructura del Código

### Variables Globales
- `selectedChannel`: Canal actual (CLEAN, CRUNCH, LEAD) - CLEAN por defecto
- `reverbActive`: Estado de reverb (true/false)
- `lastDriver2State`: Estado anterior de DRIVER2 (para modo CLEAN)
- Estados anteriores de botones para detección de flancos

### Funciones Principales
- `setup()`: Configuración inicial de pines y serial
- `loop()`: Bucle principal con lectura y actualización
- `readInputs()`: Lee botones con debounce y detecta pulsaciones
- `updateChannel()`: Cambia el canal seleccionado
- `updateReverb()`: Cambia el estado de reverb
- `updateDrivers()`: Aplica tabla de verdad a drivers
- `updateLEDs()`: Controla LEDs según estado actual

## Requisitos

### Hardware
- Placa ESP32-C3-DevKitM-1
- 4 pulsadores normalmente abiertos
- 3 drivers de relé (o circuitos de conmutación) para driver1, driver2, driver4
- 1 driver adicional (driver5) reservado para loop de efectos (no conectado en esta versión)
- 4 LEDs con resistencias limitadoras
- Fuente de alimentación adecuada

### Software
- PlatformIO IDE o CLI
- Framework Arduino para ESP32
- Visual Studio Code (recomendado)

## Configuración PlatformIO

```ini
[env:esp32-c3-devkitm-1]
platform = espressif32
board = esp32-c3-devkitm-1
framework = arduino
monitor_speed = 115200
```

## Compilación y Carga

1. Clonar el repositorio:
   ```bash
   git clone https://github.com/diegolijo/C3_FootSwitch_Tito.git
   cd C3_FootSwitch_Tito
   ```

2. Compilar el proyecto:
   ```bash
   pio run
   ```

3. Subir a la placa:
   ```bash
   pio run --target upload
   ```

4. Monitor serial (opcional):
   ```bash
   pio device monitor
   ```

## Depuración

El proyecto incluye mensajes por puerto serial (115200 baudios) que muestran:
- Inicialización del sistema
- Cambios de canal
- Cambios de estado de reverb

Ejemplo de salida:
```
Iniciando footswitch...
Canal cambiado a: CLEAN
Canal cambiado a: CRUNCH
Reverb: ACTIVADO
Reverb: DESACTIVADO
```

## Diagrama de Flujo

```
[Inicio]
    |
    v
[Configurar pines]
    |
    v
[Estado inicial: CLEAN]
    |
    v
[Leer botones] ----> [Detectar pulsaciones] ----> [Actualizar estado]
    |                                               |
    v                                               v
[Aplicar tabla verdad] <---- [Estado actual] <---- [Actualizar LEDs]
    |
    v
[Esperar 10ms]
    |
    v
[Repetir]
```

## Notas Técnicas

1. **Pull-up interno**: Los botones usan resistencias pull-up internas del ESP32-C3
2. **Activación en LOW**: Los botones conectan a GND cuando se presionan
3. **Debounce**: 50ms para evitar lecturas falsas
4. **Estado de LEDs**: Activos en LOW (cátodo común)
5. **Modo CLEAN**: Preserva el estado anterior de DRIVER2 para no afectar configuraciones externas
6. **DRIVER5**: No está asignado a ningún pin GPIO en esta versión. Está reservado para control de loop de efectos en futuras implementaciones.

## Licencia

Este proyecto está bajo la licencia MIT. Ver archivo LICENSE para más detalles.

## Autor

Diego Lijo - [GitHub](https://github.com/diegolijo)

## Versión

1.0.0 - Enero 2026
