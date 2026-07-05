//
// Created by 飞行雪绒 on 2026/6/23.
//
#include "UDS.h"
extern CAN_TxHeaderTypeDef TXHeader;
extern uint32_t pTxMailbox;
UDS_Header_t UDS_Header={&TXHeader,&pTxMailbox};
UDS_Status_t UDS_Status;
static uint8_t seed_valid = 0;
void UDS_Init(void)
{
	isotp_Init();
	UDS_Status.Lock=ENABLE;
	UDS_Status.Current_Session=DefaultSession;
	UDS_Status.Error_Count=0;
	UDS_Status.DownloadActive=0;
}
void UDS_NegativeResponse(uint8_t SID,uint8_t ErrorStatus)
{
	uint8_t Return_Data[3];
	Return_Data[0]=0x7F;
	Return_Data[1]=SID;
	Return_Data[2]=ErrorStatus;
	isotp_Sent(UDS_Header.TxHeader,Return_Data,3,UDS_Header.pTxMailbox);
}
void Get_PWM_Value(uint8_t *RXData)
{
	if (RXData[1]==0xF1&&RXData[2]==0x90)
	{
		//uint16_t Pwm_Value = Get_Pwm();
		uint8_t Return_Data[5];
		Return_Data[0]=0x62;
		Return_Data[1]=0xF1;
		Return_Data[2]=0x90;
		// Return_Data[3]=Pwm_Value>>8;
		// Return_Data[4]=Pwm_Value&0xFF;
		isotp_Sent(UDS_Header.TxHeader,Return_Data,3,UDS_Header.pTxMailbox);
	}
}
void Get_PIN1(uint8_t *RXData)
{
	if (RXData[1]==0x91&&RXData[2]==0xA0)
	{
		uint8_t Return_Data[4];
		Return_Data[0]=0x62;
		Return_Data[1]=0x91;
		Return_Data[2]=0xA0;
		Return_Data[3]=HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0);
		isotp_Sent(UDS_Header.TxHeader,Return_Data,4,UDS_Header.pTxMailbox);
	}
}
void Get_PIN2(uint8_t *RXData)
{
	if (RXData[1]==0x92&&RXData[2]==0xA0)
	{
		uint8_t Return_Data[4];
		Return_Data[0]=0x62;
		Return_Data[1]=0x92;
		Return_Data[2]=0xA0;
		Return_Data[3]=HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_2);
		isotp_Sent(UDS_Header.TxHeader,Return_Data,4,UDS_Header.pTxMailbox);
	}
}
UDS_Driver_t UDS_ReadDriver[3]={
	{{0xF1,0x90},Get_PWM_Value},
	{{0x91,0xA0},Get_PIN1},
	{{0x92,0xA0},Get_PIN2},
};
void UDS_ReadDataByIdentifier(uint8_t *RXData)
{
	uint8_t error_data=1;
	for (int i=0;i<ArraySize(UDS_ReadDriver);i++)
	{
		if (RXData[1]==UDS_ReadDriver[i].Data_Identifier[0]&&RXData[2]==UDS_ReadDriver[i].Data_Identifier[1])
		{
			error_data=0;
			UDS_ReadDriver[i].handler(RXData);
		}
	}
	if (error_data==1)
	{
		error_data=0;
		UDS_NegativeResponse(RXData[0],NRC_RequestOutOfRange);
	}
}
void UDS_DefaultSession(uint8_t *RXData)
{
	uint8_t Return_Data[2];
	Return_Data[0]=0x50;
	Return_Data[1]=0x01;
	isotp_Sent(UDS_Header.TxHeader,Return_Data,sizeof(Return_Data),UDS_Header.pTxMailbox);
	UDS_Status.Current_Session=DefaultSession;
	UDS_Status.Lock=ENABLE;
}
void UDS_ExtendedSession(uint8_t *RXData)
{
	uint8_t Return_Data[2];
	Return_Data[0]=0x50;
	Return_Data[1]=0x02;
	isotp_Sent(UDS_Header.TxHeader,Return_Data,sizeof(Return_Data),UDS_Header.pTxMailbox);
	UDS_Status.Current_Session=ExtendedSession;
	UDS_Status.Lock=ENABLE;
}
void UDS_ProgrammingSession(uint8_t *RXData)
{
	uint8_t Return_Data[2];
	Return_Data[0]=0x50;
	Return_Data[1]=0x03;
	isotp_Sent(UDS_Header.TxHeader,Return_Data,sizeof(Return_Data),UDS_Header.pTxMailbox);
	UDS_Status.Current_Session=ProgrammingSession;
	UDS_Status.Lock=ENABLE;
}
UDS_Driver_t UDS_SessionDriver[3]=
	{
	{{0x01,0x00},UDS_DefaultSession},
	{{0x02,0x00},UDS_ExtendedSession},
	{{0x03,0x00},UDS_ProgrammingSession},
};
void UDS_DiagnosticSessionControl(uint8_t *RXData)
{
	uint8_t error_data=1;
	for (int i=0;i<ArraySize(UDS_SessionDriver);i++)
	{
		if (RXData[1]==UDS_SessionDriver[i].Data_Identifier[0])
		{
			UDS_SessionDriver[i].handler(RXData);
			error_data=0;
		}
	}
	if (error_data==1)
	{
		UDS_NegativeResponse(RXData[0],NRC_SubFunctionNotSupported);
	}
}
void UDS_FlashWrite(uint32_t *FirstAddr,uint8_t *RXData)
{
	while (UDS_Status.ReceivedSize<UDS_Status.DownloadSize)
	{
		uint32_t Arry_Count=2;
		HAL_FLASH_Unlock();
		UDS_Status.AlreadyWriteenAddress=UDS_Status.DownloadAddress+UDS_Status.ReceivedSize;
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,UDS_Status.AlreadyWriteenAddress,(uint16_t)RXData[Arry_Count]|RXData[Arry_Count+1]<<8);
		HAL_FLASH_Lock();
		UDS_Status.ReceivedSize+=2;
		Arry_Count+=2;
	}
}
void UDS_RequestDownloadProgramm(uint8_t *RXData)
{
	if (UDS_Status.Current_Session==ProgrammingSession)
	{
		if (seed_valid==0&&UDS_Status.Lock==DISABLE)
		{
			UDS_Status.DownloadActive=1;
			UDS_Status.DownloadAddress=(uint32_t)RXData[3]<<24|(uint32_t)RXData[4]<<16|(uint32_t)RXData[5]<<8|RXData[6];
			UDS_Status.DownloadSize=(uint32_t)RXData[7]<<24|(uint32_t)RXData[8]<<16|(uint32_t)RXData[9]<<8|RXData[10];
			uint8_t Return_Data[2];
			Return_Data[0]=0x74;
			Return_Data[1]=20;
			isotp_Sent(UDS_Header.TxHeader,Return_Data,2,UDS_Header.pTxMailbox);
		}
	}
	else
	{
		UDS_NegativeResponse(RXData[0],NRC_ConditionsNotCorrect);
	}
}
void UDS_DownloadProgramm(uint8_t *RXData)
{
	if (UDS_Status.DownloadActive==1)
	{
		UDS_FlashWrite(&UDS_Status.DownloadAddress,RXData);
	}
}
void UDS_RequestSeed_Level1(uint8_t *RXData)
{
	if (UDS_Status.Current_Session==DefaultSession||UDS_Status.Current_Session==ExtendedSession)
	{
		UDS_NegativeResponse(RXData[0],NRC_ConditionsNotCorrect);
		return;
	}
	UDS_Status.Lock=ENABLE;
	seed_valid=1;
	uint8_t Return_Data[4];
	Return_Data[0] = 0x67;
	Return_Data[1] = SecurityAccess_RequestSeed_Level1;
	Return_Data[2] = 0x12;
	Return_Data[3] = 0x34;
	isotp_Sent(UDS_Header.TxHeader, Return_Data, 4, UDS_Header.pTxMailbox);
}
void UDS_SendKey_Level1(uint8_t *RXData)
{
	if (seed_valid==0)
	{
		UDS_NegativeResponse(RXData[0],NRC_RequestSequenceError);
		return;
	}
	uint16_t Key = ((uint16_t)RXData[2] << 8) | RXData[3];
	if (Key == 0x1239)
	{
		uint8_t Return_Data[2] = {0x67, SecurityAccess_SendKey_Level1};
		seed_valid=0;
		UDS_Status.Lock=DISABLE;
		UDS_Status.Error_Count=0;
		isotp_Sent(UDS_Header.TxHeader, Return_Data, 2, UDS_Header.pTxMailbox);
	}
	else
	{
		if (UDS_Status.Error_Count<5)
		{
			UDS_NegativeResponse(RXData[0],NRC_InvalidKey);
			UDS_Status.Error_Count++;
		}
		else
		{
			UDS_NegativeResponse(RXData[0],NRC_ExceedNumberOfAttempts);
			UDS_Status.Error_Count=0;
		}
	}
}
UDS_Driver_t UDS_SecurityAccessDriver[2]={
	{{0x01,0x00},UDS_RequestSeed_Level1},
	{{0x02,0x00},UDS_SendKey_Level1},
};
void UDS_SecurityAccess(uint8_t *RXData)
{
	uint8_t error_flag=1;

	for (int i=0;i<ArraySize(UDS_SecurityAccessDriver);i++)
	{
		if (RXData[1]==UDS_SecurityAccessDriver[i].Data_Identifier[0])
		{
			UDS_SecurityAccessDriver[i].handler(RXData);
			error_flag=0;
		}
	}
		if (error_flag==1)
		{
		UDS_NegativeResponse(RXData[0],NRC_ConditionsNotCorrect);
		}
}
UDSDriver_t UDS_Driver[5]={
	{ReadDataByIdentifier,UDS_ReadDataByIdentifier},
	{DiagnosticSessionControl,UDS_DiagnosticSessionControl},
	{SecurityAccess,UDS_SecurityAccess},
	{RequestDownload,UDS_RequestDownloadProgramm},
	{DownloadData,UDS_DownloadProgramm},
};
void UDS_Execute(uint8_t *RXData)
{
		if (RXData[0]&0x40)
		{
			return;
		}
		uint8_t error_data=1;
		for (int i=0;i<ArraySize(UDS_Driver);i++)
		{
			if (RXData[0]==UDS_Driver[i].id)
			{
				UDS_Driver[i].handler(RXData);
				error_data=0;
			}
		}
		if (error_data==1)
		{
			error_data=0;
			UDS_NegativeResponse(RXData[0],NRC_ServiceNotSupported);
		}
}
void UDS_Divide_ID(uint8_t *RXData)
{
	if (frame_ready==1)
	{
		frame_ready=0;
		UDS_Execute(RXData);
	}
}