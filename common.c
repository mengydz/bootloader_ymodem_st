/**
  ******************************************************************************
  * @file    common.c
  * @author  ptwang
  * @version 0.1
  * @date    12/15/2015
  * @brief   Common Routines.
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "common.h"
#include "usart.h"
#include "crc16.h"

/* Private variables ---------------------------------------------------------*/
struct Sys_Param sys_param = {
    false, {0}, VERSION
};

//extern osMutexId printMutexHandle;

/* Private functions ---------------------------------------------------------*/

/**
  * @brief 鍒濆鍖?SimpBuf
  *
  * @param buf
  * @param size
  *
  * @return 鎴愬姛杩斿洖1锛屽け璐ヨ繑鍥?
  */
int simp_buf_init(struct SimpBuf * buf, unsigned int size)
{
    buf->data = (char*)malloc(size);
    if(buf->data != NULL)
    {
        buf->size = size;
        buf->length = 0;
        return 1;
    }
    else
    {
        buf->size = 0;
        buf->length = 0;
        return 0;
    }
}

int simp_buf_deinit(struct SimpBuf * buf)
{
    if(buf->data != NULL)
    {
        buf->size = 0;
        buf->length = 0;
        free(buf->data);
        return 1;
    }
    else
    {
        return 0;
    }
}
#if 0
int fputc(int ch,FILE *f)
{
    uint8_t temp[1]={ch};
    HAL_UART_Transmit(&huart1,temp,1,1000);
    return 0;
}
#endif
/**
  * @brief   鐜舰缂撳啿鍖哄垵濮嬪寲
  * @param   buf 鐜舰缂撳啿鍖?
  * @param   size 缂撳啿鍖轰綋绉?
  * @retval  None
  * @author  pengtian
  * @date    2015-11-11
  */
void ring_buf_init(struct RingBuf * buf, unsigned int size)
{
    buf->head    = 0;
    buf->tail    = 0;
    buf->data    = (uint8_t*)malloc(size);
    if(buf->data != NULL)
    {
        buf->size    = size;
    }
    else
    {
        buf->size    = 0;
    }
}

/**
  * @brief   浠庣幆褰㈢紦鍐插尯璇诲彇涓€涓暟鎹?
  *          head 鍜?tail 鍙栧€艰寖鍥翠负 0 - MAX_SIZE_T锛屽埌鏈€澶у€煎悗鐜洖
  * @param   buf 鐜舰缂撳啿鍖?
  * @param   c 璇诲嚭鐨勬暟鎹?
  * @retval  浠庣紦鍐插尯璇诲嚭鐨勬暟鎹紝鑻ョ紦鍐插尯涓虹┖锛屽垯杩斿洖 -1
  * @author  pengtian
  * @date    2015-11-11
  */
int ring_buf_get(struct RingBuf * buf, char *c)
{
    int err = 0;
    // 濡傛灉闄愬埗 size 涓?2 鐨勬暣鏁帮紝鍒欏彲灏嗘眰妯℃崲涓?涓庤繍绠?
    unsigned int offset = buf->tail % buf->size;
    if(ring_buf_empty(buf) == 0)
    {
        *c = buf->data[offset];
        buf->tail++;
    }
    else
    {
        err = -1;
    }
    return err;
}

/**
  * @brief   灏嗕竴涓暟鎹啓鍏ョ幆褰㈢紦鍐插尯
  *          head 鍜?tail 鍙栧€艰寖鍥翠负 0 - MAX_SIZE_T锛屽埌鏈€澶у€煎悗鐜洖
  * @param   buf 鐜舰缂撳啿鍖?
  * @param   c 鍐欏叆鐨勬暟鎹?
  * @retval  鏄惁鎴愬姛锛屾垚鍔熻繑鍥?0锛屽け璐ヨ繑鍥?-1
  * @author  pengtian
  * @date    2015-11-11
  */
int ring_buf_put(struct RingBuf * buf, const char c)
{
    int err = 0;
    // 濡傛灉闄愬埗 size 涓?2 鐨勬暣鏁帮紝鍒欏彲灏嗘眰妯℃崲涓?涓庤繍绠?
    unsigned int offset = buf->head % buf->size;
    if(ring_buf_full(buf) == 0)
    {
        buf->data[offset] = c;
        buf->head++;
    }
    else
    {
        /* 缂撳啿鍖哄凡婊?*/
        err = -1;
    }
    return err;
}

/**
  * @brief   娓呴櫎鐜舰缂撳啿鍖烘暟鎹?
  * @param   buf 鐜舰缂撳啿鍖?
  * @param   clear 鏄惁灏嗘暟鎹竻闆讹紝0 鍙慨鏀圭储寮曞拰璁℃暟锛? 鏁版嵁娓呴浂
  * @retval  None
  * @author  pengtian
  * @date    2015-11-11
  */
void ring_buf_flush(struct RingBuf * buf, const int clear)
{
    buf->head    = 0;
    buf->tail    = 0;
    if (clear)
    {
        memset(buf->data, 0, buf->size);
    }
}

/**
  * @brief   妯″姞杩愮畻锛岃秴杩囨ā鍒欏綊闆?
  * @param   value 杈撳叆鏁板€?
  * @param   modulus 妯?
  * @retval  妯″姞杩愮畻鐨勭粨鏋?
  * @author  pengtian
  * @date    2015-11-11
  */
static unsigned int modulo_inc(const unsigned int value, const unsigned int modulus)
{
    unsigned int result = value + 1;
    if (result >= modulus)
    {
        result  = 0;
    }
    return result;
}

/**
  * @brief   妯″噺杩愮畻锛屽噺瓒呰繃闆跺垯璁颁负鏈€澶у€?
  * @param   value 杈撳叆鏁板€?
  * @param   modulus 妯?
  * @retval  妯″噺杩愮畻鐨勭粨鏋?
  * @author  pengtian
  * @date    2015-11-11
  */
static unsigned int modulo_dec(const unsigned int value, const unsigned int modulus)
{
    unsigned int result = (0==value) ? (modulus - 1) : (value - 1);
    return result;
}

/**
  * @brief 鍒濆鍖?pipe buf
  *
  * @param buf
  * @param size
  */
void pipe_buf_init(struct PipeBuf * buf, unsigned int size)
{
    buf->idx     = 0;
    buf->data    = (int *)calloc(size, sizeof(int));
    if(buf->data != NULL)
    {
        buf->size    = size;
    }
    else
    {
        buf->size    = 0;
    }
}

/**
  * @brief 鍙栧嚭 pipe buf 涓殑 n 鏉℃暟鎹紝瀛樺叆涓€涓暟缁?dataout銆?
  *        鑰佺殑鏁版嵁鍦ㄤ綆浣嶏紝鏂扮殑鏁版嵁鍦ㄩ珮浣嶃€?
  *        鍥犱负涓嶅 PipeBuf 鐨勬暟鎹仛淇敼锛岃法绾跨▼闂鏉ユ簮浜?
  *        鍙兘浼氬嚭鐜板皻鏈潵寰楀強鍙栧嚭灏辫鏂版暟鎹鐩栥€?
  *        濡傛灉鏃犳硶鎺ュ彈鏁拌鐩栭棶棰橈紝鍙姞閿併€?
  *
  * @param buf 寰呰鍑虹殑 pipe buf
  * @param dataout 璇诲嚭鏁版嵁瀛樺叆鐨勬暟缁?
  * @param num 璇诲嚭鐨勬暟鎹釜鏁?
  * @author pengtian
  */
void pipe_buf_read(struct PipeBuf * buf, int *dataout, int num)
{
    int i;
    int idx;
    idx = modulo_dec(buf->idx, buf->size);

    for(i = 0; i < num; i++)
    {
        dataout[num - 1 - i] = buf->data[idx];
        idx = modulo_dec(idx, buf->size);
    }
}

/**
  * @brief 灏嗘暟鎹坊鍔犲埌 pipe buf
  *
  * @param buf 鐩爣鐨?pipe buf
  * @param datanew 寰呮坊鍔犵殑鏁版嵁
  */
void pipe_buf_put(struct PipeBuf * buf, int datanew)
{
    buf->data[buf->idx] = datanew;
    buf->idx = modulo_inc(buf->idx, buf->size);
}

/**
  * @brief 璁板綍鏈€灏忕殑 n 绗旀暟鎹?
  *
  * @param new_data 鏂扮殑涓€绗旇緭鍏ユ暟鎹?
  * @param buf 璁板綍鏁版嵁鐨勬暟缁?
  * @param num 鏁扮粍鐨勯暱搴?
  *
  */
void log_min_add(int new_data, int * buf, int num)
{
    int cmp_data = 0;
    int swap_temp = 0;
    int i;

    cmp_data = new_data;
    for(i = 0; i < num; i++)
    {
        if(cmp_data < buf[i])
        {
            swap_temp = buf[i];
            buf[i] = cmp_data;
            cmp_data = swap_temp;
        }
    }
}

/**
  * @brief 灏嗚褰曟渶灏忔暟鎹殑鏁扮粍澶嶄綅鍒版渶澶у€?
  *
  * @param reset_val 鐢ㄤ互澶嶄綅鎵€鏈夊厓绱犵殑鏈€澶у€?
  * @param buf 璁板綍鏁版嵁鐨勬暟缁?
  * @param num 鏁扮粍鐨勯暱搴?
  *
  */
void log_min_reset(int reset_val, int * buf, int num)
{
    int i;

    for(i = 0; i < num; i++)
    {
        buf[i] = reset_val;
    }
}
#if 0
/**
  * @brief osSignalWait 鐨勪慨姝ｇ増鏈紝绛夊緟鐗瑰畾淇″彿鎵嶄細缁х画銆?
  *        ST 鐨?osSignalWait 閬囧埌浠讳綍 signal 閮戒細鍞ら啋骞剁户缁墽琛屻€?
  *
  * @param signals
  * @param millisec
  *
  * @return
  */
osEvent osSignalWaitSp(int32_t signals, uint32_t millisec)
{
    osEvent ret;
    uint32_t time_start = 0; // 寮€濮嬫椂闂磋褰?

    time_start = HAL_GetTick();
    ret.value.signals = 0;
    /* 绛夊緟浠讳竴涓€涓俊鍙峰敜閱掕 task锛岀涓€涓弬鏁?0x00 鐢ㄤ互閬垮厤 signal 琚竻闆?*/
    do {
        ret = osSignalWait(0x00, millisec);
    }
    while(((ret.value.signals & signals) == 0) && ((HAL_GetTick() - time_start) < millisec));

    /* 娓呯┖涔嬪墠绛夊緟鐨?signals */
    osSignalWait(signals, 0);

    return ret;
}
#endif
/**
  * @brief   璁＄畻妫€楠屽拰
  * @param   buff 锛氶渶瑕佽绠楃殑瀛楃涓?
  * @retval  鏍￠獙鍜?
  * @author  yinnnian
  * @date    2015-11-9
  * @modified
  */
char getsum(char *buff)
{
    char sum = 0x00;
    uint16_t i,length;

    length = strlen(buff);

    for(i = 0; i < length; i++)
    {
        sum ^= buff[i];
    }
    return sum;
}

/**
  * @brief   璁＄畻crc妫€楠?
  * @param   buff 锛氶渶瑕佽绠楃殑瀛楃涓?
  * @retval  鏍￠獙鍊硷紙16浣嶏級
  * @modified
  */
uint16_t getcrc16(uint8_t buff[],uint32_t length)
{
    crc_t crc;

    crc = crc_init();
    crc = crc_update(crc, buff, length);
    crc = crc_finalize(crc);

    return crc;
}
#if 0
/**
  * @brief  鎵撳嵃 Debug 娑堟伅锛屽彲浠ョ敤鍏ㄥ眬鍙傛暟鍏抽棴
  * @param  msg: 娑堟伅鍐呭
  * @retval None
  */
void dbg_printf(const char* fmt, ...)
{
    va_list args;

    if(sys_param.dbg_on == true)
    {
        osMutexWait(printMutexHandle, osWaitForever);

        va_start (args, fmt);
        vfprintf (stdout, fmt, args);
        va_end (args);
        fflush(stdout);

        osMutexRelease(printMutexHandle);
    }
}

/* mutex print */
void os_printf(const char* fmt, ...)
{
    va_list args;
    // char str[200];
    osMutexWait(printMutexHandle, osWaitForever);

    va_start (args, fmt);
    vfprintf (stdout, fmt, args);
    va_end (args);
    fflush(stdout);

    //    va_start (args, fmt);
    //    vsprintf (str, fmt, args);
    //    va_end (args);

    // HAL_UART_Transmit(&huart1,(uint8_t *)str,strlen(str),1000);

    osMutexRelease(printMutexHandle);
}
#endif

