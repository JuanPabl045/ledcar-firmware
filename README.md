# LedCar Firmware 🚗✨

Este es el repositorio del firmware para **LedCar**, un sistema de iluminación LED inteligente basado en el microcontrolador **ESP32-S3**. Permite controlar tiras LED RGB (WS2812B) a través de Bluetooth Low Energy (BLE), permitiendo enviar comandos en tiempo real para modificar colores, brillo y reproducir patrones en diferentes zonas.

## 🚀 Características Principales

* **Controlador ESP32-S3**: Potente y con conectividad BLE integrada.
* **Tiras LED WS2812B**: Soporte actual para hasta 300 LEDs (ajustable).
* **Gestión por Zonas**: Los LEDs están divididos lógicamente en zonas (PDI, PTI, PTD, PDD, Techo, Footwell, Tablero) para control independiente.
* **Control Inalámbrico via BLE**: Recibe comandos remotos mediante la librería `NimBLE-Arduino` para aplicar colores, patrones y temporizaciones.
* **Múltiples Patrones**: Soporta iluminación sólida, desvanecimientos (fade) y destellos (flash).
* **Optimización de Energía**: Configurado con un límite de seguridad de 2800 mA (a 5V) mediante la gestión de consumo de `FastLED`.

## 🛠️ Tecnologías Utilizadas

* [PlatformIO](https://platformio.org/) (Entorno de desarrollo)
* Framework: **Arduino**
* [FastLED](https://github.com/FastLED/FastLED) (Control y animaciones de los LEDs)
* [NimBLE-Arduino](https://github.com/h2zero/NimBLE-Arduino) (Pila Bluetooth Low Energy eficiente para ESP32)

## 📋 Estructura de Comandos BLE

El ESP32 actúa como un servidor BLE y escucha en la característica designada. Los comandos se envían como paquetes de 8 bytes con la siguiente estructura:

| Byte | Descripción | Valores / Uso |
|------|-------------|---------------|
| `0`  | Tipo        | Tipo de comando a ejecutar |
| `1`  | R (Red)     | Valor de color rojo (0-255) |
| `2`  | G (Green)   | Valor de color verde (0-255) |
| `3`  | B (Blue)    | Valor de color azul (0-255) |
| `4`  | Brillo      | Intensidad global de la animación (0-255) |
| `5`  | Patrón      | `0`: Sólido, `1`: Fade, `3`: Flash |
| `6`  | Offset Ms 1 | Parte alta del retraso de tiempo de ejecución |
| `7`  | Offset Ms 2 | Parte baja del retraso de tiempo de ejecución |

## ⚙️ Instalación y Compilación

1. Instala **VS Code** y la extensión de **PlatformIO**.
2. Clona este repositorio:
   ```bash
   git clone https://github.com/JuanPabl045/ledcar-firmware.git
   ```
3. Abre la carpeta del proyecto en VS Code.
4. PlatformIO descargará automáticamente las dependencias definidas en `platformio.ini` (`FastLED` y `NimBLE-Arduino`).
5. Conecta tu placa ESP32-S3 (por defecto `esp32-s3-devkitc-1`).
6. Presiona el botón de **Upload** en PlatformIO (o ejecuta `pio run --target upload`).

## 🔌 Conexiones de Hardware

* **Datos LED (Data Pin)**: Pin 48 (Configurable en `src/main.cpp`)
* **Alimentación (VCC / GND)**: 5V y GND. *Nota: Asegúrate de usar una fuente de alimentación externa si pasas del límite de corriente del puerto USB.*
