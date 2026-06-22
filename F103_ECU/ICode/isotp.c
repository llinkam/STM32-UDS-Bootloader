//
// Created by 飞行雪绒 on 2026/6/16.
//
#include "isotp.h"
extern UART_HandleTypeDef huart1;
int8_t FC_Return_Value;
uint8_t RX_data[8];
int8_t RX_Data()
{
	if ((CAN_RxData[0]&0xF0)!=ISOTP_FC)
	{
		FC_Return_Value=0;
	}
	if ((CAN_RxData[0]&0xF0)==ISOTP_FC&&(CAN_RxData[0]&0x0F)==ISOTP_FC_CTS)
	{
		FC_Return_Value=1;
	}
	if ((CAN_RxData[0]&0xF0)==ISOTP_FC&&(CAN_RxData[0]&0x0F)==ISOTP_FC_WAIT)
	{
		FC_Return_Value=0;
	}
	if ((CAN_RxData[0]&0xF0)==ISOTP_FC&&(CAN_RxData[0]&0x0F)==ISOTP_FC_OVFLW)
	{
		FC_Return_Value=-1;
	}
	return FC_Return_Value;
}
void isotp_Sent(CAN_TxHeaderTypeDef *pHeader,const uint8_t *TData,uint8_t *RData,uint16_t Len,uint32_t *pTxMailbox)
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
void isotp_Receive(uint8_t *RData,uint8_t *SN)
{
	while (HAL_CAN_GetRxFifoFillLevel(&hcan1, CAN_RX_FIFO0) == 0);
	HAL_CAN_GetRxMessage(&hcan1,CAN_RX_FIFO0,&CAN_RxHeader,CAN_RxData);
	// 单帧
	if ((CAN_RxData[0]&0xF0)==ISOTP_SF)
	{
		uint8_t len;
		len=CAN_RxData[0]&0x0F;
		for (uint8_t i=0;i<len;i++)
		{
			RData[i]=CAN_RxData[i+1];
		}
	}
	// 多帧
	if ((CAN_RxData[0]&0xF0)==ISOTP_FF)
	{
		uint16_t len=((CAN_RxData[0]&0x0F)<<8)|(CAN_RxData[1]);
		uint8_t Already_Receive_Data=0;
		for (uint8_t i=0;i<6;i++)
		{
			RData[i]=CAN_RxData[i+2];
		}
		len=len-6;
		Already_Receive_Data=6;
		while (len>7)
		{
			while (HAL_CAN_GetRxFifoFillLevel(&hcan1, CAN_RX_FIFO0) == 0);
			HAL_CAN_GetRxMessage(&hcan1,CAN_RX_FIFO0,&CAN_RxHeader,CAN_RxData);
			if ((CAN_RxData[0]&0xF0)==ISOTP_CF)
			{
				*SN=CAN_RxData[0]&0x0F;
				for (uint8_t i=0;i<7;i++)
				{
					RData[Already_Receive_Data]=CAN_RxData[i+1];
					Already_Receive_Data++;
				}
			}
			len=len-7;
		}
		if (len>0&&len<=7)
		{
			while (HAL_CAN_GetRxFifoFillLevel(&hcan1, CAN_RX_FIFO0) == 0);
			HAL_CAN_GetRxMessage(&hcan1,CAN_RX_FIFO0,&CAN_RxHeader,CAN_RxData);
			if ((CAN_RxData[0]&0xF0)==ISOTP_CF)
			{
				*SN=CAN_RxData[0]&0x0F;
				for (uint8_t i=0;i<len;i++)
				{
					RData[Already_Receive_Data]=CAN_RxData[i+1];
					Already_Receive_Data++;
				}
			}
		}
	}
}

