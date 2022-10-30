#pragma once
#include "freertos/FreeRTOS.h"

class I2SMEMSSampler
{
public:
  int read(int16_t *samples, int count);
};