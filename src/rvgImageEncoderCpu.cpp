#include "rvgMain.h"

ImageEncoderCpu::ImageEncoderCpu()
{
  ptrInfo = NULL;
  ptrGrid = NULL;
  ptrStream = NULL;
  ptrObjects = NULL;
}

int ImageEncoderCpu::atomicAdd (int *ptr, int value)
{
  int oldValue = *ptr;
  *ptr += value;
  return oldValue;
}

int ImageEncoderCpu::atomicExchange (int *ptr, int value)
{
  int oldValue = *ptr;
  *ptr = value;
  return oldValue;
}
