#include "rvgMain.h"

ImageEncoder::ImageEncoder()
{
  ptrInfo = NULL;
  ptrGrid = NULL;
  ptrStream = NULL;
  ptrObjects = NULL;
}

int ImageEncoder::atomicAdd (int *ptr, int value)
{
  int oldValue = *ptr;
  *ptr += value;
  return oldValue;
}

int ImageEncoder::atomicExchange (int *ptr, int value)
{
  int oldValue = *ptr;
  *ptr = value;
  return oldValue;
}
