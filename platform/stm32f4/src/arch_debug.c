
#include <typedef.h>
#include <stm32f4xx_conf.h>

#define STDIO   UART1

void arch_debug_init(void)
{
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

#if defined(USE_UART2)
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
#else
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
#endif

	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
#if defined(USE_UART2)
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2 | GPIO_Pin_3;
#else
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9 | GPIO_Pin_10;
#endif
	GPIO_Init(GPIOA, &GPIO_InitStructure);

#if defined(USE_UART2)
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);
#else
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);
#endif


	USART_InitStructure.USART_BaudRate   = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits   = USART_StopBits_1;
	USART_InitStructure.USART_Parity     = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;

#if defined(USE_UART2)
	USART_Init(USART2, &USART_InitStructure);
	USART_Cmd(USART2, ENABLE);
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
#else
	USART_Init(USART1, &USART_InitStructure);
	USART_Cmd(USART1, ENABLE);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
#endif
}


void USART_PutChar(USART_TypeDef* USARTx, char c)
{
    USART_SendData(USARTx, (uint8_t)c);

    while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);
}

int USART_PutStr(USART_TypeDef* USARTx, const char *str)
{
    int i=0;

    while (*str != '\0') {
        USART_PutChar(USARTx, *str++);
        i++;
    }

    return i;
}

int arch_debug_print(const char *str, int len)
{
    size_t i = 0;

    while (i < len) {

#if defined(USE_UART2)
        if (*str == '\n') {
            USART_PutChar(USART2, '\r');
        }
        USART_PutChar(USART2, *str++);
#else
        if (*str == '\n') {
            USART_PutChar(USART1, '\r');
        }
        USART_PutChar(USART1, *str++);
#endif

        i++;
    }

	return i;
}

