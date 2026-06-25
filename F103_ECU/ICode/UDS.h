//
// Created by 飞行雪绒 on 2026/6/23.
//
#ifndef F103_ECU_UDS_H
#define F103_ECU_UDS_H
#include "Can.h"
#include "isotp.h"
#define ReadDataByIdentifier 0x22
void Get_PWM_Value();
void Get_PIN1();
void Get_PIN2();
void UDS_ReadDataByIdentifier();
void UDS_Divide_ID();
void UDS_Execute();
#endif //F103_ECU_UDS_H

