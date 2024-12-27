#ifndef _VL_MODBUS_H
#define _VL_MODBUS_H

/* 包含头文件 ---------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
/* 类型定义 ------------------------------------------------------------------*/
// 定义解析帧结构
typedef struct {
    uint8_t frame_header;  // 帧头
    uint8_t function_code; // 功能码
    uint8_t data_length;   // 数据长度
    uint8_t slave_id;      // 从机ID
    uint8_t* data;         // 数据内容
    uint16_t crc;          // CRC校验码
} ProtocolFrame;

/* 宏定义 --------------------------------------------------------------------*/


/**************************modbus master********************/
// 主机发送框架
int host_send_frame(uint8_t com , uint8_t frame_header, uint8_t function_code, uint8_t slave_id, uint8_t* data, uint8_t data_len);
// 解析协议帧函数
int parse_frame(uint8_t* frame, uint16_t frame_len);

int VL_main(void);

#endif 
