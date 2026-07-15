//
// Created by 飞行雪绒 on 2026/6/16.
//
#include "isotp.h"
CAN_RxHeaderTypeDef CAN_RxHeader;
uint8_t CAN_RxData[8];
uint8_t RXData[1024];
extern UART_HandleTypeDef huart1;
static uint16_t RX_Data_Len;
static uint8_t ExpectedSN;
uint16_t RX_UDSDataLen;
volatile uint8_t frame_ready;
uint8_t SN;
uint32_t pRxMailbox;
volatile HAL_StatusTypeDef isotp_last_can_tx_status = HAL_OK;
void CAN1_Transmit(const CAN_TxHeaderTypeDef *TXpHeader, const uint8_t aData[], uint32_t *pTxMailbox)
{
	uint32_t StartTick=HAL_GetTick();
	while (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1)==0U)
	{
		if ((HAL_GetTick()-StartTick)>100U)
		{
			isotp_last_can_tx_status=HAL_TIMEOUT;
			return;
		}
	}
	isotp_last_can_tx_status = HAL_CAN_AddTxMessage(&hcan1, TXpHeader, aData, pTxMailbox);
	if (isotp_last_can_tx_status!=HAL_OK)
	{
		return;
	}
	StartTick=HAL_GetTick();
	while (HAL_CAN_IsTxMessagePending(&hcan1,*pTxMailbox)!=0U)
	{
		if ((HAL_GetTick()-StartTick)>100U)
		{
			HAL_CAN_AbortTxRequest(&hcan1,*pTxMailbox);
			isotp_last_can_tx_status=HAL_TIMEOUT;
			return;
		}
	}
}
void isotp_Init(void)
{
	MX_CAN1_Init();
}
void isotp_Sent(CAN_TxHeaderTypeDef *pHeader,const uint8_t *TData,uint16_t Len,uint32_t *pTxMailbox)
{
	uint8_t SN=0x1;
	if (Len<=7)
	{
		uint8_t buf[8]={0};
		buf[0]=ISOTP_SF|(Len&0x0F);
		for (uint8_t i=0;i<Len;i++)
		{
			buf[i+1]=TData[i];
		}
		pHeader->DLC=Len+1;
		CAN1_Transmit(pHeader,buf,pTxMailbox);
	}
	if (Len>7)
	{

		uint8_t Already_Sent_Data=6;
		uint16_t Already_Record_Data=Already_Sent_Data;
		uint8_t buf[8]={0};
		buf[0]=ISOTP_FF|((Len>>8)&0x0F);
		buf[1]=Len&0xFF;
		Len=Len-Already_Sent_Data;
		for (uint8_t i=0;i<6;i++)
		{
			buf[i+2]=TData[i];
		}
		pHeader->DLC=8;
		CAN1_Transmit(pHeader,buf,pTxMailbox);
		// while (1)
		// {
		// 	int8_t r = RX_Data();
		// 	if (r == 1)  break;
		// 	if (r == -1) return;
		// }
		while (Len>7)
		{
			buf[0]=ISOTP_CF|(SN&0x0F);
			for (uint8_t i=0;i<7;i++)
			{
				buf[i+1]=TData[Already_Record_Data];
				Already_Record_Data++;
			}
			CAN1_Transmit(pHeader,buf,pTxMailbox);
			Len=Len-7;
			SN++;
		}
		if (Len<=7)
		{
			buf[0]=ISOTP_CF|(SN&0x0F);
			for (uint16_t i=0;i<Len;i++)
			{
				buf[i+1]=TData[i+Already_Record_Data];
			}
			pHeader->DLC=Len+1;
			CAN1_Transmit(pHeader,buf,pTxMailbox);
		}
	}
}
void isotp_Receive(uint8_t *RData,uint8_t *SN,uint32_t *pRxMailbox)
{
	// 单帧
	if ((CAN_RxData[0]&0xF0)==ISOTP_SF)
	{
		RX_Data_Len=CAN_RxData[0]&0x0F;
		for (uint8_t i=0;i<RX_Data_Len;i++)
		{
			RData[i]=CAN_RxData[i+1];
		}
		frame_ready=1;
		RX_UDSDataLen=RX_Data_Len;
	}
	// 多帧
	if ((CAN_RxData[0]&0xF0)==ISOTP_FF)
	{
		RX_Data_Len=((CAN_RxData[0]&0x0F)<<8)|(CAN_RxData[1]);
		for (uint8_t i=0;i<6;i++)
		{
			RData[i]=CAN_RxData[i+2];
		}
		RX_Data_Len-=6;
		RX_UDSDataLen=6;
		ExpectedSN=1U;
	}
	if ((CAN_RxData[0]&0xF0)==ISOTP_CF)
	{
		*SN=CAN_RxData[0]&0x0F;
		if ((*SN!=ExpectedSN)||(RX_Data_Len==0U))
		{
			RX_Data_Len=0U;
			RX_UDSDataLen=0U;
			frame_ready=0U;
			return;
		}
		ExpectedSN=(ExpectedSN+1U)&0x0FU;
		if (RX_Data_Len>7)
		{
			for (uint8_t i=0;i<7;i++)
			{
				RData[RX_UDSDataLen]=CAN_RxData[i+1];
				RX_UDSDataLen++;
			}
			RX_Data_Len-=7;
		}
		else
		{
			for (uint8_t i=0;i<RX_Data_Len;i++)
			{
				RData[RX_UDSDataLen]=CAN_RxData[i+1];
				RX_UDSDataLen++;
			}
			RX_Data_Len=0;
			frame_ready=1;
		}
	}
}
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	if (hcan->Instance == CAN1)
	{
		HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &CAN_RxHeader, CAN_RxData);
		isotp_Receive(RXData,&SN,&pRxMailbox);
	}
}



