#include <ch32v00x.h>

#include "nectransmitter.h"

NECTransmitter necTransmitter;

void setup() {
  necTransmitter.Init(GPIOD, RCC_APB2Periph_GPIOD, GPIO_Pin_0,
                      /* activeLow */ false);
}

void loop() {
  // Will send 0x00FF 0xAB54.
  necTransmitter.SendNEC(0x00, 0xAB);
  delay(1000);
}
