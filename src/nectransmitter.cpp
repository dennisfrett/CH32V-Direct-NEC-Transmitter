#include "nectransmitter.h"

#include "ch32v_atomic.h"
#include "debug.h"

#define NEC_VALUE_PULSE_DURATION 562
#define NEC_0_PAUSE_DURATION 562
#define NEC_1_PAUSE_DURATION 1687

void NECTransmitter::Init(GPIO_TypeDef *peripheral, uint32_t port, uint16_t pin,
                          bool activeLow) {
  GPIO_InitTypeDef GPIO_InitStructure = {0};

  RCC_APB2PeriphClockCmd(port, ENABLE);

  GPIO_InitStructure.GPIO_Pin = pin;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(peripheral, &GPIO_InitStructure);

  this->peripheral = peripheral;
  this->pin = pin;

  this->activeValue = activeLow ? Bit_RESET : Bit_SET;
  this->nonActiveValue = activeLow ? Bit_SET : Bit_RESET;

  SetPin(nonActiveValue);
}

void NECTransmitter::SendPulse(bool value) const {
  SetPin(activeValue);
  Delay_Us(NEC_VALUE_PULSE_DURATION);
  SetPin(nonActiveValue);

  if (value) {
    Delay_Us(NEC_1_PAUSE_DURATION);
  } else {
    Delay_Us(NEC_0_PAUSE_DURATION);
  }
}

void NECTransmitter::SendByte(uint8_t byte, bool includeInverse) const {
  // Send normal.
  for (int bit = 0; bit < 8; bit++) {
    SendPulse((byte >> bit) & 1);
  }

  if (!includeInverse) {
    return;
  }

  // Send logical inverse.
  for (int bit = 0; bit < 8; bit++) {
    SendPulse(!((byte >> bit) & 1));
  }
}

void NECTransmitter::SendNEC(uint8_t address, uint8_t command) const {
  // Ensure we run atomically.
  interrupt_atomic_block() { // Send AGC (9ms pulse).
    SetPin(activeValue);
    Delay_Us(9000);

    // Send space.
    SetPin(nonActiveValue);
    Delay_Us(4500);

    // Send address.
    SendByte(address);

    // Send code.
    SendByte(command);

    // End.
    SendPulse(false);

    // Reset to non active.
    SetPin(nonActiveValue);
  }
}

void NECTransmitter::SendExtendedNEC(uint16_t address, uint8_t command) const {
  // Ensure we run atomically.
  interrupt_atomic_block() { // Send AGC (9ms pulse).
    SetPin(activeValue);
    Delay_Us(9000);

    // Send space.
    SetPin(nonActiveValue);
    Delay_Us(4500);

    // Send address.
    SendByte(address / 256, /* includeInverse */ false);
    SendByte(address & 0xff, /* includeInverse */ false);

    // Send code.
    SendByte(command);

    // End.
    SendPulse(false);

    // Reset to non active.
    SetPin(nonActiveValue);
  }
}

void NECTransmitter::SetPin(BitAction value) const {
  GPIO_WriteBit(peripheral, pin, value);
}
