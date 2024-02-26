#pragma once

#include <ch32v00x.h>

// Code copied from
// https://www.eevblog.com/forum/microcontrollers/globally-enablingdisabling-ch32v003-interrupts-with-the-mstatus-csr/.
#define interrupt_atomic_block()                                               \
  for (uint32_t                                                                \
           __interrupt_atomic_block_mie_saved = __interrupt_save_clear_mie(),  \
           __interrupt_atomic_block_loop = 1;                                  \
       __interrupt_atomic_block_loop;                                          \
       __interrupt_restore_mie(__interrupt_atomic_block_mie_saved),            \
           __interrupt_atomic_block_loop = 0)

#define MSTATUS_MIE_MASK 0x8

__attribute__((always_inline)) static inline uint32_t
__interrupt_save_clear_mie(void) {
  uint32_t value;
  __asm volatile("csrrc %0, mstatus, %1"
                 : "=r"(value)           // Outputs
                 : "i"(MSTATUS_MIE_MASK) // Inputs
                 :                       // No clobbers
  );
  return value & MSTATUS_MIE_MASK;
}

__attribute__((always_inline)) static inline void
__interrupt_restore_mie(uint32_t value) {
  value &= MSTATUS_MIE_MASK;
  __asm volatile("csrs mstatus, %0"
                 :            // No outputs
                 : "r"(value) // Inputs
                 :            // No clobbers
  );
}
