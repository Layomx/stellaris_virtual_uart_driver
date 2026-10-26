/*
  Vector table and reset handler for a Cortex-M3

  On reset the core reads two words from address 0x00000000:
    word 0: initial main stack pointer
    word 1: address of the reset hander (bit 0 set = Thumb)
  Everything else here prepares teh C runtime: .data copy, .bss clear.
*/

#include <stdint.h>

extern uint32_t _estack;                 // top of SRAM
extern uint32_t _sidata, _sdata, _edata; // .data: load address / run range
extern uint32_t _sbss, _ebss;            // .bss range

void Reset_Handler(void);
void Default_Handler(void);
int main(void);

void NMI_Handler(void) __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void MemManage_Handler(void) __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void) __attribute__((weak, alias("Default_Handler")));
void DebugMon_Handler(void) __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void) __attribute__((weak, alias("Default_Handler")));

/*

    Core exceptions only (16 entries). This polling-only project enables

*/
__attribute__((section(".isr_vector"), used))
const uintptr_t vector_table[16] = {
    (uintptr_t)&_estack,      // 0: initial SP
    (uintptr_t)Reset_Handler, // 1: Reset
    (uintptr_t)NMI_Handler,
    (uintptr_t)HardFault_Handler,
    (uintptr_t)MemManage_Handler,
    (uintptr_t)BusFault_Handler,
    (uintptr_t)UsageFault_Handler,
    0, // reserved
    0,
    0,
    0,
    (uintptr_t)SVC_Handler,
    (uintptr_t)DebugMon_Handler,
    0, // reserved
    (uintptr_t)PendSV_Handler,
    (uintptr_t)SysTick_Handler,
};

void Reset_Handler(void) {
  uint32_t *src = &_sidata;
  for (uint32_t *dst = _sdata; dst < &_edata;) {
    *dst++ = *src++;
  }
  for (uint32_t *dst = &_sbss; dst < &_ebss;) {
    *dst++ = 0u;
  }
  (void)main();
  for (;;) {
  }
}

void Default_Handler(void) {
  for (;;) {
  }
}
