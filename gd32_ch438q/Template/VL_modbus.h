#ifndef _VL_MODBUS_H
#define _VL_MODBUS_H

/* ����ͷ�ļ� ---------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
/* ���Ͷ��� ------------------------------------------------------------------*/
// �������֡�ṹ
typedef struct {
    uint8_t frame_header;  // ֡ͷ
    uint8_t function_code; // ������
    uint8_t data_length;   // ���ݳ���
    uint8_t slave_id;      // �ӻ�ID
    uint8_t* data;         // ��������
    uint16_t crc;          // CRCУ����
} ProtocolFrame;

/* �궨�� --------------------------------------------------------------------*/


/**************************modbus master********************/
// �������Ϳ��
int host_send_frame(uint8_t com , uint8_t frame_header, uint8_t function_code, uint8_t slave_id, uint8_t* data, uint8_t data_len);
// ����Э��֡����
int parse_frame(uint8_t* frame, uint16_t frame_len);

int VL_main(void);

#endif 
