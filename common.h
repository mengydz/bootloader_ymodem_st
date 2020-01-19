
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __COMMON_H__
#define __COMMON_H__

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include "cmsis_os.h"
#include "stm32f1xx_hal.h"

#define GPS_WAIT_TIME       3       // 3s, unit is 1s
#define LTE_WAIT_TIME       3       // 3s, unit is 1s

#define VERSION        {1, 0, 0}

#ifndef uint8_t 
#define uint8_t  unsigned char
#endif
#ifndef uint16_t 
#define uint16_t  unsigned short
#endif
#ifndef uint32_t 
#define uint32_t  unsigned int
#endif

#ifndef int16_t 
#define int16_t  char
#endif
#ifndef int16_t 
#define int16_t  short
#endif
#ifndef int32_t 
#define int32_t  int
#endif
/* Exported types ------------------------------------------------------------*/
/**
  * @brief   环形缓冲区结构
  * @author  pengtian
  * @date    2015-11-11
  */
struct RingBuf
{
    int head;      // head index, 写地址偏移
    int tail;      // tail index, 读地址偏移
    int count;     // 当前有效数据个数
    int size; // buffer 最大体积
    unsigned char *data;
};

/**
  * @brief 简单 buffer，带一个长度
  */
struct SimpBuf
{
    int length;
    int size;
    char *data;
};

/**
  * @brief   用来保存最近几笔数据的 buffer。
  *          老的数据会被覆盖，不知道有没有正式的名字。
  *          接近 pipeline register
  * @author  pengtian
  * @date    2016-11-30
  */
struct PipeBuf
{
    int *data;
    int idx;  // index, 当前地址偏移
    int size; // buffer 最大体积
};

/**
  * @brief Semantic Version semver.org
  */
struct SemVer
{
    uint16_t ver_maj; /// major version
    uint16_t ver_min; /// minor version
    uint16_t ver_patch; /// patch version
};

/**
  * @brief  系统参数
  */
struct Sys_Param
{
    uint8_t dbg_on;          // Debug message ON
    struct SemVer pcb_ver;
    struct SemVer soft_ver;       // code version

};

//extern struct Sys_Param sys_param;	//heyi dis

/* Exported functions ------------------------------------------------------- */
int simp_buf_init(struct SimpBuf * buf, unsigned int size);
int simp_buf_deinit(struct SimpBuf * buf);
void ring_buf_init(struct RingBuf * buf, unsigned int size);
void ring_buf_flush(struct RingBuf * buf, const int clear);
int ring_buf_put(struct RingBuf * buf, const char c);
int ring_buf_get(struct RingBuf * buf, char *c);
void pipe_buf_init(struct PipeBuf * buf, unsigned int size);
void pipe_buf_read(struct PipeBuf * buf, int *dataout, int num);
void pipe_buf_put(struct PipeBuf * buf, int datanew);
void log_min_add(int new_data, int * buf, int num);
void log_min_reset(int reset_val, int * buf, int num);
//char getsum(char *buff);		//heyi dis
uint16_t getcrc16(uint8_t buff[],uint32_t length);
//void dbg_printf(const char* fmt, ...);
//void os_printf(const char* fmt, ...);
//osEvent osSignalWaitSp(int32_t signals, uint32_t millisec);

//int fputc(int ch,FILE *f);

/**
  * @brief   判断环形缓冲区是否为空
  *          head 和 tail 取值范围为 0 - MAX_SIZE_T，到最大值后环回
  * @param   buf 环形缓冲区
  * @retval  1: 为空 0: 不为空
  * @author  pengtian
  * @date    2015-11-11
  */
inline int ring_buf_empty(struct RingBuf * buf)
{
    return ((buf->head - buf->tail) == 0U);
}

/**
  * @brief   判断环形缓冲区是否为满
  *          head 和 tail 取值范围为 0 - MAX_SIZE_T，到最大值后环回
  * @param   buf 环形缓冲区
  * @retval  1: 为满 0: 不为满
  * @author  pengtian
  * @date    2015-11-11
  */
inline int ring_buf_full(struct RingBuf * buf)
{
    return ((buf->head - buf->tail) == buf->size);
}

#endif

