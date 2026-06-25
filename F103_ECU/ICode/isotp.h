//
// Created by 飞行雪绒 on 2026/6/16.
//
#include "Can.h"
#ifndef CAN_TEST1_ISOTP_H
#define CAN_TEST1_ISOTP_H

/**
 * @brief   ISO-TP (ISO 15765-2) 传输层:CAN 上的 UDS 报文拆装
 * @note    PCI 首字节高4位=帧类型,低4位含义随帧类型变(见下)
 */

/* PCI 帧类型(首字节高4位) */
#define ISOTP_SF  0x00  /* Single Frame   单帧:低4位=数据长度(0~7) */
#define ISOTP_FF  0x10  /* First Frame    首帧:低4位+次字节=12位总长 */
#define ISOTP_CF  0x20  /* Consecutive    连续帧:低4位=序号SN(0~15) */
#define ISOTP_FC  0x30  /* Flow Control   流控帧:低4位=流控状态FS */

/* 流控状态 FS(FC 帧低4位) */
#define ISOTP_FC_CTS    0x00  /* 继续发 */
#define ISOTP_FC_WAIT   0x01  /* 等待 */
#define ISOTP_FC_OVFLW  0x02  /* 溢出,中止 */
extern volatile uint8_t frame_ready;   /* 中断里收齐一条完整报文后置1,主循环消费后清0 */
void isotp_init(void);
/**
 * @brief  发送一段数据(自动判断单帧还是多帧)
 * @param  pHeader
 * @param  TData  载荷指针
 * @param  Len   载荷长度(0~4095)
 */
void isotp_Sent(CAN_TxHeaderTypeDef *pHeader,const uint8_t *TData,uint16_t Len,uint32_t *pTxMailbox);

/**
 * @brief  喂入一帧 CAN 数据,内部按 SF/FF/CF 自动重组;收齐一条完整报文后交付
 * @param  frame  这一帧的数据(最多8字节,首字节是 PCI)
 * @param  dlc    这一帧的数据长度
 */

void isotp_Receive(CAN_TxHeaderTypeDef *pHeader,uint8_t *RData,uint8_t *SN,uint32_t *pTxMailbox);
#endif //CAN_TEST1_ISOTP_H
