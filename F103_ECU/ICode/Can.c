//
// Created by 飞行雪绒 on 2026/6/11.
//
#include "Can.h"
#include "usart.h"
#include "isotp.h"
GPIO_InitTypeDef can1_gpio;
CAN_HandleTypeDef hcan1;
CAN_FilterTypeDef sFilterConfig;
extern CAN_RxHeaderTypeDef CAN_RxHeader;
uint8_t CAN_RxData[8];
extern uint8_t RXData[1024];
extern uint8_t SN;
extern uint32_t pRxMailbox;
void Can_MSP_Init(void)
{
    __HAL_RCC_CAN1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    can1_gpio.Pin = GPIO_PIN_12;
    can1_gpio.Mode = GPIO_MODE_AF_PP;
    can1_gpio.Pull = GPIO_NOPULL;
    can1_gpio.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(GPIOA,&can1_gpio);
		can1_gpio.Pin=GPIO_PIN_11;
		can1_gpio.Mode = GPIO_MODE_INPUT;
		HAL_GPIO_Init(GPIOA,&can1_gpio);
}
void Can_Init(void)
{
    Can_MSP_Init();
    hcan1.Instance = CAN1;
    hcan1.ErrorCode = HAL_CAN_ERROR_NONE;
    hcan1.Init.AutoRetransmission=ENABLE;
    hcan1.Init.AutoBusOff=ENABLE;
    hcan1.Init.AutoWakeUp=ENABLE;
    hcan1.Init.Mode=CAN_MODE_NORMAL;
    hcan1.Init.ReceiveFifoLocked=ENABLE;
		hcan1.Init.Prescaler    = 4;
		hcan1.Init.SyncJumpWidth= CAN_SJW_1TQ;
		hcan1.Init.TimeSeg1     = CAN_BS1_13TQ;
		hcan1.Init.TimeSeg2     = CAN_BS2_2TQ;
    hcan1.Init.TimeTriggeredMode=DISABLE;
		hcan1.Init.TransmitFifoPriority=ENABLE;
    hcan1.State = HAL_CAN_STATE_RESET;
    HAL_CAN_Init(&hcan1);
    sFilterConfig.FilterActivation=CAN_FILTER_ENABLE;
    sFilterConfig.FilterBank=0;
    sFilterConfig.FilterFIFOAssignment=CAN_FILTER_FIFO0;
    sFilterConfig.FilterIdHigh=0x0000;
    sFilterConfig.FilterIdLow=0x0000;
    sFilterConfig.FilterMaskIdHigh=0x0000;
    sFilterConfig.FilterMaskIdLow=0x0000;
    sFilterConfig.FilterMode=CAN_FILTERMODE_IDMASK;
    sFilterConfig.FilterScale=CAN_FILTERSCALE_32BIT;
    sFilterConfig.SlaveStartFilterBank=14;
    HAL_CAN_ConfigFilter(&hcan1,&sFilterConfig);
    HAL_CAN_Start(&hcan1);
		HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 3, 3);
		HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
		HAL_CAN_ActivateNotification(&hcan1,CAN_IT_RX_FIFO0_MSG_PENDING);
}

void CAN1_Transmit(const CAN_TxHeaderTypeDef *TXpHeader,const uint8_t aData[], uint32_t *pTxMailbox)
{
	uint32_t StartTick=HAL_GetTick();
	while (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1)==0U)
	{
		if ((HAL_GetTick()-StartTick)>100U)
		{
			return;
		}
	}
	HAL_CAN_AddTxMessage(&hcan1,TXpHeader,aData,pTxMailbox);
}
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	HAL_CAN_GetRxMessage(&hcan1,CAN_RX_FIFO0,&CAN_RxHeader,CAN_RxData);
	isotp_Receive(RXData,&SN,&pRxMailbox);
}
