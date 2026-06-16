//
// Created by 飞行雪绒 on 2026/6/11.
//
#include "main.h"
#ifndef CAN_TEST_CAN_H
extern CAN_HandleTypeDef hcan1;
extern CAN_FilterTypeDef sFilterConfig;
extern GPIO_InitTypeDef can1_gpio;
extern CAN_RxHeaderTypeDef CAN_RxHeader;
extern uint8_t CAN_RxData[8];
extern uint8_t CAN_RxData[8];
void Can_MSP_Init(void);
void Can_Init(void);
void CAN1_Transmit(const CAN_TxHeaderTypeDef *pHeader,const uint8_t aData[], uint32_t *pTxMailbox);
#define CAN_TEST_CAN_H
#endif //CAN_TEST_CAN_H
