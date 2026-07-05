//
// Created by 飞行雪绒 on 2026/6/23.
//
#ifndef F103_ECU_UDS_H
#define F103_ECU_UDS_H
#include "Can.h"
#include "isotp.h"
#define ArraySize(arr) (sizeof(arr)/sizeof(arr[0]))
/* UDS 服务 ID：Tester 发请求时用 SID，ECU 正响应一般是 SID + 0x40。 */
#define ReadDataByIdentifier       0x22

#define DiagnosticSessionControl   0x10
#define DefaultSession           0x01
#define ExtendedSession          0x02
#define ProgrammingSession       0x03

#define SecurityAccess             0x27

#define SecurityAccess_RequestSeed_Level1   0x01 // 请求种子 1
#define SecurityAccess_SendKey_Level1       0x02 // 发送密钥 1

#define  RequestDownload 0x34 // 请求下载,要有地址起始位置，下载数据大小
#define  DownloadData 0x36 // 下载数据,要包含下载数据
/* UDS Negative Response Code (NRC)：负响应码
 * ECU 无法执行请求时，返回格式为：7F + 原请求SID + NRC
 */
#define NRC_ServiceNotSupported                    0x11  /* 不支持该服务，比如收到未实现的 SID */
#define NRC_SubFunctionNotSupported                0x12  /* 不支持该子功能，比如 27 05 */
#define NRC_IncorrectMessageLengthOrInvalidFormat  0x13  /* 报文长度错误或格式不合法 */
#define NRC_ConditionsNotCorrect                   0x22  /* 当前条件不满足，比如当前会话不允许执行该服务 */
#define NRC_RequestSequenceError                   0x24  /* 请求顺序错误，比如没先 27 01 就直接 27 02 */
#define NRC_RequestOutOfRange                      0x31  /* 请求参数超范围，比如 DID 不存在、地址越界 */
#define NRC_SecurityAccessDenied                   0x33  /* 安全访问被拒绝，比如未解锁就请求刷写 */
#define NRC_InvalidKey                             0x35  /* SecurityAccess 密钥错误 */
#define NRC_ExceedNumberOfAttempts                 0x36  /* 密钥错误次数超过限制 */
#define NRC_RequiredTimeDelayNotExpired            0x37  /* 错误次数过多后，等待时间未到 */
#define NRC_ResponsePending                        0x78  /* 请求处理中，ECU 暂时无法立即给最终响应 */
typedef struct
{
	uint8_t Lock;                    /* 安全锁状态：ENABLE=未解锁，DISABLE=已解锁 */
	uint8_t Current_Session;         /* 当前诊断会话：Default / Extended / Programming */
	uint8_t Error_Count;             /* SecurityAccess 密钥错误次数 */

	uint8_t DownloadActive;         /* 下载流程状态：1=已经通过 0x34，允许接收 0x36；0=未进入下载 */
	uint32_t DownloadAddress;       /* 本次下载的目标 Flash 起始地址 */
	uint32_t DownloadSize;          /* 本次下载期望接收的总字节数 */
	uint16_t ReceivedSize;          /* 当前已经通过 0x36 接收的数据字节数 */
	uint8_t ExpectedBlockCounter;  /* 下一帧 0x36 期望的 blockSequenceCounter */
	uint32_t AlreadyWriteenAddress; /* 已经写入 Flash 的地址 */
} UDS_Status_t;
/* UDS 顶层服务分发表项。
 * id：UDS 服务 ID，比如 0x22、0x10。
 * handler：对应服务处理函数，输入是 ISO-TP 已经重组好的完整应用层数据 RXData。
 */
typedef struct
{
	uint8_t id;
	void (*handler)(uint8_t *RXData);
}UDSDriver_t;

/* 0x22 ReadDataByIdentifier 的 DID 分发表项。
 * Data_Identifier：0x22 后面的两个字节 DID，比如 F1 90、91 A0。
 * handler：对应 DID 的读取函数，负责组织并发送 UDS 正响应。
 */
typedef struct {
	uint8_t Data_Identifier[2];
	void (*handler)(uint8_t *RXData);
}UDS_Driver_t;
/* UDS 响应发送时共用的 ISO-TP 发送上下文。
 * TxHeader：指向 CAN 发送帧头。
 * pTxMailbox：指向 HAL_CAN_AddTxMessage 返回的发送邮箱编号。
 */
typedef struct {
	CAN_TxHeaderTypeDef *TxHeader;
	uint32_t *pTxMailbox;
} UDS_Header_t;

/* 0x22 服务下的具体 DID 处理函数。 */
void Get_PWM_Value(uint8_t *RXData);
void Get_PIN1(uint8_t *RXData);
void Get_PIN2(uint8_t *RXData);

/* UDS 服务层入口函数。
 * UDS_Divide_ID：检查 frame_ready，收到一条完整 ISO-TP 报文后再分发。
 * UDS_Execute：根据 RXData[0] 的 SID 查 UDSDriver 表并调用对应服务。
 */
void UDS_Init(void);
void UDS_ReadDataByIdentifier(uint8_t *RXData);
void UDS_Divide_ID(uint8_t *RXData);
void UDS_Execute(uint8_t *RXData);
#endif //F103_ECU_UDS_H

