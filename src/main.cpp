#include "Arduino.h"
#include "arduinoFFT.h"
#include "driver/adc.h"
#include "driver/i2s.h"
#include "ESPAsyncWebServer.h"

#define BLOCK_LENGTH 1024
#define SAMPLE_RATE 44100
#define CUTTOFF_FREQ 20000
#define NYQUIST_FREQ SAMPLE_RATE / 2
const double FREQUENCY_RESOLUTION = (double)SAMPLE_RATE / BLOCK_LENGTH;
const int cuttofIndex = (int)CUTTOFF_FREQ / FREQUENCY_RESOLUTION;

i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S_LSB,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 2,
    .dma_buf_len = BLOCK_LENGTH,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0};

int16_t samples[BLOCK_LENGTH];
double vReal[BLOCK_LENGTH];
double vImag[BLOCK_LENGTH];
size_t bytes_read = 0;

char *ssid = "FRITZ!Box 6490 Cable";
char *password = "mozartstrasse1";

arduinoFFT FFT = arduinoFFT();

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void setup()
{
  Serial.begin(115200);
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_11);

  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);

  i2s_set_adc_mode(ADC_UNIT_1, ADC1_CHANNEL_4);

  // WIFI
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println("Connected to the WiFi network");

  // Websocket
  server.addHandler(&ws);
  server.begin();

  delay(2000);
}

double indexToFrequency(int index)
{
  return (double)index * FREQUENCY_RESOLUTION;
}

void loop()
{
  i2s_read(I2S_NUM_0, (void *)samples, sizeof(samples), &bytes_read, portMAX_DELAY);
  for (size_t i = 0; i < BLOCK_LENGTH; i++)
  {
    // Take the 12 LSBs
    samples[i] = (samples[i] & 0b0000111111111111);
  }

  // Copy samples to vReal
  // vImag is zero
  for (size_t i = 0; i < BLOCK_LENGTH; i++)
  {
    vReal[i] = (samples[i] - 2048);
    vImag[i] = 0;
  }

  FFT.DCRemoval();
  FFT.Windowing(vReal, BLOCK_LENGTH, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, BLOCK_LENGTH, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, BLOCK_LENGTH);

  // Send data to websocket
  String data = "[";
  for (size_t i = 0; i < cuttofIndex; i++)
  {
    data += "{\"freq\": " + String(indexToFrequency(i)) + ", \"value\": " + String(vReal[i]) + "}";
    if (i < cuttofIndex - 1)
    {
      data += ",";
    }
  }

  data += "]";
  ws.textAll(data);
}