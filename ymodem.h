#ifndef _YMODEM_H_
#define _YMODEM_H_

#include "stm32f1xx_hal.h"
#include "iap_if.h"

/* 宏 ------------------------------------------------------------------------*/
#define PACKET_SEQNO_INDEX      (1)
#define PACKET_SEQNO_COMP_INDEX (2)

#define PACKET_HEADER           (3)
#define PACKET_TRAILER          (2)
#define PACKET_OVERHEAD         (PACKET_HEADER + PACKET_TRAILER)
#define PACKET_SIZE             (128)
#define PACKET_1K_SIZE          (1024)

#define FILE_NAME_LENGTH        (32)
#define FILE_SIZE_LENGTH        (16)

#define SOH                     (0x01)      // 128字节数据包开始
#define STX                     (0x02)      // 1024字节的数据包开始
#define EOT                     (0x04)      // 结束传输
#define ACK                     (0x06)      // 回应
#define NAK                     (0x15)      // 没回应
#define CAN                     (0x18)      // 这两个相继中止转移，取消传输
#define STARTC                  (0x43)      // 请求数据包 'C' == 0x43, 需要 16-bit CRC 
#define ACK_NULL                (0x00)      // \n

#define NAK_TIMEOUT             (0x100000)
#define MAX_ERRORS              (5)

/* 函数声明 ------------------------------------------------------------------*/
iap_err_t Ymodem_Receive (uint8_t *buf);
int32_t Receive_Byte (uint8_t *c, uint32_t timeout);
uint32_t Send_Byte (uint8_t c);

#endif  /* _YMODEM_H_ */

/*******************************文件结束***************************************/

