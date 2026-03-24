#include <FastLED.h>
#include <NimBLEDevice.h>
#include <queue>

// CONFIGURACION LEDS
#define LED_PIN      48
#define NUM_LEDS     300
#define LED_TYPE     WS2812B
#define COLOR_ORDER  GRB
#define MAX_MA       2800

CRGB leds[NUM_LEDS];

// ZONAS
const int Z_PDI_S = 0,   Z_PDI_N = 36;
const int Z_PTI_S = 36,  Z_PTI_N = 36;
const int Z_PTD_S = 72,  Z_PTD_N = 36;
const int Z_PDD_S = 108, Z_PDD_N = 36;
const int Z_TCH_S = 144, Z_TCH_N = 120;
const int Z_FTW_S = 264, Z_FTW_N = 24;
const int Z_TAB_S = 288, Z_TAB_N = 12;

// COLA DE COMANDOS
struct Cmd {
  uint8_t tipo, r, g, b, brillo, patron;
  uint32_t fireAt;
};

std::queue<Cmd> cmdQueue;
portMUX_TYPE queueMux = portMUX_INITIALIZER_UNLOCKED;

// BLE UUIDs
#define SVC_UUID  "12345678-1234-1234-1234-123456789abc"
#define CHAR_UUID "abcdefab-cdef-abcd-efab-cdefabcdefab"

// ANIMACION ACTUAL
CRGB currentColor = CRGB::Black;
uint8_t currentBrillo = 40;
uint8_t currentPatron = 0;
uint32_t flashOffAt = 0;
bool flashing = false;

void applyCmd(const Cmd& c) {
  Serial.printf("CMD recibido: tipo=%d R=%d G=%d B=%d brillo=%d patron=%d\n",
                c.tipo, c.r, c.g, c.b, c.brillo, c.patron);

  CRGB col(c.r, c.g, c.b);
  currentColor = col;
  currentBrillo = c.brillo;
  currentPatron = c.patron;

  switch (c.patron) {
    case 0: // solido
      fill_solid(leds, NUM_LEDS, col);
      FastLED.setBrightness(c.brillo);
      break;

    case 1: // fade
      fill_solid(leds, NUM_LEDS, col);
      FastLED.setBrightness(c.brillo);
      break;

    case 3: // flash
      fill_solid(leds, NUM_LEDS, col);
      FastLED.setBrightness(c.brillo);
      flashOffAt = millis() + 150;
      flashing = true;
      break;

    default:
      fill_solid(leds, NUM_LEDS, col);
      FastLED.setBrightness(c.brillo);
      break;
  }

  FastLED.show();
}

class CmdCallback : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* c) override {
    std::string v = c->getValue();
    Serial.printf("BLE write recibido: %d bytes\n", v.length());
    if (v.length() < 8) return;

    uint16_t offsetMs = ((uint8_t)v[6] << 8) | (uint8_t)v[7];

    Cmd cmd;
    cmd.tipo = (uint8_t)v[0];
    cmd.r = (uint8_t)v[1];
    cmd.g = (uint8_t)v[2];
    cmd.b = (uint8_t)v[3];
    cmd.brillo = (uint8_t)v[4];
    cmd.patron = (uint8_t)v[5];
    cmd.fireAt = millis() + offsetMs;

    portENTER_CRITICAL(&queueMux);
    cmdQueue.push(cmd);
    portEXIT_CRITICAL(&queueMux);
  }
};

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  delay(1000);
  Serial.println("LedCar firmware iniciando...");

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, MAX_MA);
  FastLED.setBrightness(40);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
  Serial.println("LEDs inicializados");

  NimBLEDevice::init("LedCar");
  NimBLEDevice::setPower(ESP_PWR_LVL_P9);

  NimBLEServer* server = NimBLEDevice::createServer();
  NimBLEService* svc = server->createService(SVC_UUID);
  NimBLECharacteristic* ch = svc->createCharacteristic(
      CHAR_UUID, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);
  ch->setCallbacks(new CmdCallback());
  svc->start();

  NimBLEAdvertising* adv = NimBLEDevice::getAdvertising();
  adv->addServiceUUID(SVC_UUID);
  adv->start();

  Serial.println("BLE listo - esperando conexion...");
}

void loop() {
  uint32_t now = millis();

  portENTER_CRITICAL(&queueMux);
  bool hasCmd = !cmdQueue.empty() && now >= cmdQueue.front().fireAt;
  portEXIT_CRITICAL(&queueMux);

  if (hasCmd) {
    portENTER_CRITICAL(&queueMux);
    Cmd cmd = cmdQueue.front();
    cmdQueue.pop();
    portEXIT_CRITICAL(&queueMux);
    applyCmd(cmd);
  }

  if (flashing && now >= flashOffAt) {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.setBrightness(0);
    FastLED.show();
    flashing = false;
  }

  FastLED.show();
  delay(1);
}
