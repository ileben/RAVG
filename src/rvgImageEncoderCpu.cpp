#include "rvgMain.h"

EncoderCpu::EncoderCpu()
{
  ptrInfo = NULL;
  ptrGrid = NULL;
  ptrStream = NULL;
  ptrObjects = NULL;
}

int EncoderCpu::atomicAdd (int *ptr, int value)
{
  int oldValue = *ptr;
  *ptr += value;
  return oldValue;
}

int EncoderCpu::atomicExchange (int *ptr, int value)
{
  int oldValue = *ptr;
  *ptr = value;
  return oldValue;
}
