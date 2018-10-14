#ifndef EXCEPTION_H
#define EXCEPTION_H

void HardFault_Handler();

void MemManage_Handler(void);

void BusFault_Handler(void);

void UsageFault_Handler(void);

#endif // EXCEPTION_H

