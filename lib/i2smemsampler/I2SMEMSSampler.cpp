#include "I2SMEMSSampler.h"
#include "soc/i2s_reg.h"
#include <algorithm>
#include <driver/i2s.h>

int I2SMEMSSampler::read(int16_t *samples, int count)
{
  int32_t raw_samples[1024];
  int sample_index = 0;
  while (count > 0)
  {
    size_t bytes_read = 0;
    i2s_read(I2S_NUM_0, (void **)raw_samples, sizeof(int32_t) * std::min(count, 1024), &bytes_read, portMAX_DELAY);
    int samples_read = bytes_read / sizeof(int32_t);
    for (int i = 0; i < samples_read; i++)
    {
      samples[sample_index++] = (int16_t)(raw_samples[i] >> 11);
    }
  }
  return sample_index;
}