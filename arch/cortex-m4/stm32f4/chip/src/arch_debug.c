
#include <type_def.h>
#include <stm32f4xx_conf.h>

void arch_debug_init(void)
{
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2; //TX
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_3; //RX
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);


	USART_InitStructure.USART_BaudRate   = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits   = USART_StopBits_1;
	USART_InitStructure.USART_Parity     = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode                = USART_Mode_Tx | USART_Mode_Rx;

	USART_Init(USART2, &USART_InitStructure);

	USART_Cmd(USART2, ENABLE);
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
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

    for (; ;) {
        if (*str == '\n')
            USART_PutChar(USART2, '\r');

        USART_PutChar(USART2, *str++);

        i++;

        if (i >= len)
            break;
    }

	return i;
}

int _write(int fd, const char *str, int len)
{
    size_t i = 0;

    if (fd > 2) {
        USART_PutStr(USART2, "Invalide fd");
        return -1;
    }

    for (; ;) {
        if (*str == '\n')
            USART_PutChar(USART2, '\r');

        USART_PutChar(USART2, *str++);

        i++;

        if (i >= len)
            break;
    }

	return i;
}
