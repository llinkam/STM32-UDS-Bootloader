//
// Created by 飞行雪绒 on 2026/6/23.
//
#include "UDS.h"
static CAN_TxHeaderTypeDef uds_tx_header={0x7E8,0,CAN_ID_STD,CAN_RTR_DATA,8,DISABLE};
static uint32_t uds_mailbox;
extern uint8_t rxdata[16];
typedef struct
{
	uint8_t id;
	void (*handler)();
}UDSDriver_t;
typedef struct {
	uint8_t Data_Identifier[2];
	void (*handler)();
}UDS_ReadDriver_t;
void Get_PWM_Value()
{
	if (rxdata[1]==0xF1&&rxdata[2]==0x90)
	{
		//uint16_t Pwm_Value = Get_Pwm();
		uint8_t Return_Data[5];
		Return_Data[0]=0x62;
		Return_Data[1]=0xF1;
		Return_Data[2]=0x90;
		// Return_Data[3]=Pwm_Value>>8;
		// Return_Data[4]=Pwm_Value&0xFF;
		isotp_Sent(&uds_tx_header,Return_Data,5,&uds_mailbox);
	}
}
void Get_PIN1()
{
	if (rxdata[1]==0x91&&rxdata[2]==0xA0)
	{
		uint8_t Return_Data[4];
		Return_Data[0]=0x62;
		Return_Data[1]=0x91;
		Return_Data[2]=0xA0;
		Return_Data[3]=HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_11);
		isotp_Sent(&uds_tx_header,Return_Data,4,&uds_mailbox);
	}
}
void Get_PIN2()
{
	if (rxdata[1]==0x92&&rxdata[2]==0xA0)
	{
		uint8_t Return_Data[4];
		Return_Data[0]=0x62;
		Return_Data[1]=0x92;
		Return_Data[2]=0xA0;
		Return_Data[3]=HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_2);
		isotp_Sent(&uds_tx_header,Return_Data,4,&uds_mailbox);
	}
}
UDS_ReadDriver_t UDS_ReadDriver[3]={
	{{0xF1,0x90},Get_PWM_Value},
	{{0x91,0xA0},Get_PIN1},
	{{0x92,0xA0},Get_PIN2},
};
void UDS_ReadDataByIdentifier()
{
	uint8_t error_data=1;
	for (int i=0;i<3;i++)
	{
		if (rxdata[1]==UDS_ReadDriver[i].Data_Identifier[0]&&rxdata[2]==UDS_ReadDriver[i].Data_Identifier[1])
		{
			error_data=0;
			UDS_ReadDriver[i].handler();
		}
	}
	if (error_data==1)
	{
		error_data=0;
		uint8_t Return_Data[3];
		Return_Data[0]=0x7F;
		Return_Data[1]=0x22;
		Return_Data[2]=0x31;
		isotp_Sent(&uds_tx_header,Return_Data,sizeof(Return_Data),&uds_mailbox);
	}

}
UDSDriver_t UDSDriver[3]={
	{ReadDataByIdentifier,UDS_ReadDataByIdentifier}
};
void UDS_Execute()
{
		if (rxdata[0]&0x40)
		{
			return;
		}
		uint8_t error_data=1;
		for (int i=0;i<3;i++)
		{
			if (rxdata[0]==UDSDriver[i].id)
			{
				UDSDriver[i].handler();
				error_data=0;
			}
		}
		if (error_data==1)
		{
			error_data=0;
			uint8_t Return_Data[3];
			Return_Data[0]=0x7F;
			Return_Data[1]=rxdata[0];
			Return_Data[2]=0x11;
			isotp_Sent(&uds_tx_header,Return_Data,sizeof(Return_Data),&uds_mailbox);
		}
}
void UDS_Divide_ID()
{
	if (frame_ready==1)
	{
		frame_ready=0;
		UDS_Execute();
	}
}


