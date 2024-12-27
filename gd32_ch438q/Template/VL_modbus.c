#include "VL_modbus.h"
#include "CH438_driver.h"
#include "systick.h"
#include "gd32_timer.h"

ProtocolFrame TX_frame;
ProtocolFrame RX_frame;
static uint8_t i_com = 0;

// CRC 计算函数
uint16_t calculate_crc(uint8_t* data, uint16_t length) {
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001; // 使用 Modbus CRC 多项式
            }
            else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

// 解析协议帧函数
int parse_frame(uint8_t* frame, uint16_t frame_len) {
    // 检查最小帧长度
    if (frame_len < 6) { // 最小长度：1(帧头) + 1(功能码) + 1(数据长度) + 1(从机ID) + 2(CRC)
        return -1;
    }
    memset(&RX_frame,0,sizeof(RX_frame));
    // 提取数据
    RX_frame.frame_header = frame[0];
    RX_frame.function_code = frame[1];
    RX_frame.data_length = frame[2];
    RX_frame.slave_id = frame[3];
    RX_frame.data = &frame[4]; // 数据从从机ID之后开始
    RX_frame.crc = (frame[frame_len - 1] << 8) | frame[frame_len - 2];
    // 计算 CRC
    uint16_t crc_calculated = calculate_crc(frame, frame_len - 2);
    if (crc_calculated != RX_frame.crc) {
        return -1;
    }// 验证帧长度
    else if (frame_len != 5 + RX_frame.data_length) { // 5: 帧头 + 功能码 + 数据长度 + 2CRC
        return -2;
    }
    else if (RX_frame.frame_header != TX_frame.frame_header) {
        return -3; 
    }
    else if (RX_frame.function_code != TX_frame.function_code) {
        return -4;
    }
    else if ( (TX_frame.frame_header != 0x1E ) && (RX_frame.slave_id != TX_frame.slave_id)) {
        return -5;
    }
    else if ( (TX_frame.frame_header == 0x3E) && (RX_frame.slave_id == 0) ) {
        return -6;
    }
    else {

    }

#if 0
    // 打印解析结果
    printf("Frame Header: 0x%02X\n", RX_frame.frame_header);
    printf("Function Code: 0x%02X\n", RX_frame.function_code);
    printf("Data Length: %d\n", RX_frame.data_length);
    printf("Slave ID: 0x%02X\n", RX_frame.slave_id);
    printf("Data: ");
    for (int i = 0; i < RX_frame.data_length - 1; i++) { // -1 因为从机ID已解析
        printf("0x%02X ", RX_frame.data[i]);
    }
    printf("\n");
#endif

    return 0;
}

// 将结构体封装为数组
int frame_to_array(ProtocolFrame* frame, uint8_t* output_array, uint16_t max_len) {
    // 计算封装帧的总长度
    uint16_t frame_len = 5 + frame->data_length; // 帧头 + 功能码 + 数据长度 + 2CRC
    if (frame_len > max_len) {
        printf("Output array size too small.\n");
        return -1;
    }

    // 填充帧头
    output_array[0] = frame->frame_header;
    // 填充功能码
    output_array[1] = frame->function_code;
    // 填充数据长度
    output_array[2] = frame->data_length;
    // 填充从机ID
    output_array[3] = frame->slave_id;
    // 填充数据内容
    memcpy(&output_array[4], frame->data, frame->data_length - 1); // 数据长度包含从机ID，所以减1

    // 计算 CRC 校验
    frame->crc = calculate_crc(output_array, frame_len - 2); // CRC 不包括自身
    output_array[frame_len - 2] = frame->crc & 0xFF;         // CRC 低位
    output_array[frame_len - 1] = (frame->crc >> 8) & 0xFF;  // CRC 高位

    return frame_len;
}

// 主机发送框架

int host_send_frame(uint8_t com , uint8_t frame_header, uint8_t function_code, uint8_t slave_id, uint8_t* data, uint8_t data_len) {
    // 构造帧结构
    memset(&TX_frame, 0, sizeof(TX_frame));
    TX_frame.frame_header = frame_header;
    TX_frame.function_code = function_code;
    TX_frame.data_length = data_len + 1; // 数据内容长度 + 从机ID
    TX_frame.slave_id = slave_id;
    TX_frame.data = data;

    // 封装帧为数组
    uint8_t output_array[256] = { 0 };
    int TX_frame_len = frame_to_array(&TX_frame, output_array, sizeof(output_array));
    i_com = com;
    if (TX_frame_len > 0) {
        // 发送帧
				CH438_SendDatas(i_com,output_array,TX_frame_len);
    }
    else {
        //printf("Failed to create TX_frame.\n");
			return -1;
    }
		return 0;
}

//数组右移动
void rightShift(uint8_t arr[], int n, int shift) {
    shift = shift % n; // In case the shift is greater than the array length
    int temp[shift];
    for (int i = 0; i < shift; i++) {
        temp[i] = arr[n - shift + i];
    }
    for (int i = n - 1; i >= shift; i--) {
        arr[i] = arr[i - shift];
    }
    for (int i = 0; i < shift; i++) {
        arr[i] = temp[i];
    }
}

//数组左移动
void leftShift(uint8_t  arr[], int n, int shift) {
    int temp[shift];
    for (int i = 0; i < shift; i++) {
        temp[i] = arr[i];
    }
    for (int i = 0; i < n - shift; i++) {
        arr[i] = arr[i + shift];
    }
    for (int i = 0; i < shift; i++) {
        arr[n - shift + i] = temp[i];
    }
}
void Data_filtering(uint8_t* frame, uint8_t *frame_len){
			for(int k =0; k < *frame_len;k++){
						if(frame[k] == 0){
							leftShift(frame,sizeof(frame)/sizeof(frame[0]),1);
						}else{
							break;
						}
					}
						
					for(int k = *frame_len; k >= 1;k--){
						if(frame[k-1] == 0){
							*frame_len=k-1;
						}else{
							break;
						}
					}		
}
// 缓存接收到的数据并拼接
void read_and_data(unsigned char hCom) {

    unsigned char buffer[256]={0}; // 缓冲区
    unsigned char bytes_read=0;
		while(1){
			if(RevFlag == 2 ||RevFlag == 1){
			// 从串口读取数据
			RevFlag = 0;
			bytes_read = CH438_RecvDatas(hCom, buffer);
			if (bytes_read > 0) {
				// 将接收到的数据添加到临时缓冲区
				for (unsigned char i = 0; i < bytes_read; i++) {
					Revbuff[RevLen++] = buffer[i];

					// 如果临时缓冲区的数据量超过一个数据包大小或遇到特定标识符
					if (RevLen >= 22 || Revbuff[RevLen-1] == 0xFF) { // 假设每个数据包的最大大小为20字节
						return;
					}}
				
				}
			}
		}
}

void read_Revbuff(unsigned char hCom){
	 int i = 1;
		while(i){
			if(RevFlag == 2 ||RevFlag == 1){
				RevFlag = 0;
				Data_filtering(Revbuff,&RevLen);		
				parse_frame(Revbuff,RevLen);
				break;
			}
			delay_1ms(1);
	}
}

// 示例主函数
int VL_main(void) {

	while(1){
		
		//read_and_data(ch438q_com);
		
#if 1	
		uint8_t data = 0x01;
		for(uint8_t slave_id = 1;slave_id <= 8;slave_id++){
			host_send_frame(3,0x4E,0x03,slave_id,&data,1);
			read_Revbuff(ch438q_com);
			for(uint8_t i=0; i<RX_frame.data_length;i++){
				if(i == 1 || i == 3 || i == 5 || i == 7||i == 10||i == 12||i == 14 || i == 16 ){
					continue;
				}
				if(RX_frame.data[i] == 0 ){
					RX_frame.data[i] = 1;
				}else{
					RX_frame.data[i] = 0;
				}
				
				host_send_frame(3,0x2E,0x06,slave_id,RX_frame.data,RX_frame.data_length);
				read_Revbuff(ch438q_com);
			}
		}
#endif

#if 0
    unsigned char light_data[16] = {0};
		for(uint8_t slave_id = 1;slave_id <= 8;slave_id++){
			
			for(uint8_t j=0; j < sizeof(light_data);j++){
				host_send_frame(3,0x2E,0x06,slave_id,light_data,sizeof(light_data));
				if(light_data[j] == 1)
				light_data[j] = 0;
				else
				light_data[j] = 1 ;	
			}
			
			for(uint8_t j=0; j < sizeof(light_data);j++){
				host_send_frame(3,0x2E,0x06,slave_id,light_data,sizeof(light_data));
				if(light_data[j] == 1)
				light_data[j] = 0;
				else
				light_data[j] = 1 ;	
			}
		}
#endif 
	}

   return 0;
}
