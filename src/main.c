#include "LM3S6965_regs.h"

int main(void)
{
    // Initial smoke test: confirm that the firmware reaches main().
    // UART initialization will be added next.
    (void)SYSCTL_RCC_OFFSET;

    while (1) {
    }
}
