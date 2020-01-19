/*******************************************************************************
 ** 鏂囦欢鍚?                ymodem.c
 ** 鍔熻兘:		鍜孻modem.c鐨勭浉鍏崇殑鍗忚鏂囦欢
                璐熻矗浠庤秴绾х粓绔帴鏀舵暟鎹?浣跨敤Ymodem鍗忚)锛屽苟灏嗘暟鎹姞杞藉埌鍐呴儴RAM涓€?
                濡傛灉鎺ユ敹鏁版嵁姝ｅ父锛屽垯灏嗘暟鎹紪绋嬪埌Flash涓紱濡傛灉鍙戠敓閿欒锛屽垯鎻愮ず鍑洪敊銆?
 ** 鐩稿叧鏂囦欢:	stm32f10x.h
 *******************************************************************************/

/* 鍖呭惈澶存枃浠?*****************************************************************/
#include "ymodem.h"
#include "flash_if.h"
#include "common.h"
#include "usart.h"
#include "main.h"
#include "gpio.h"

/* 鍙橀噺澹版槑 -----------------------------------------------------------------*/
static uint32_t flash_start_addr;
//static uint32_t flash_data_buf[1024 >> 2];
static uint32_t flash_data_buf[1024];

extern char *FILE_NAME;
extern uint8_t tab_1024[1024];
extern int g_Uart3RecvCount;
extern int g_uart2_use_rs485_control;
/*******************************************************************************
 * @函数名称	   ReadFileLength
 * @函数说明   从字符串中获取文件大小
 * @输入参数   c: 接收字符
                timeout: 超时时间
 * @输出参数   无
 * @返回参数   接收的结果
                0：成功接收
                1：时间超时
 *******************************************************************************/
void ReadFileLength (char *src, uint32_t src_length, uint32_t* file_length)
{
	int i = 0;

	uint32_t wLength = 0;

	for(i = 0; i < src_length; i++){

		wLength = wLength * 10 + (src[i]-'0');   // 字符串 0-9
		printf("[%s]: wLength is %u\r\n",__func__, wLength);
	}

	*file_length = wLength;
}


/*******************************************************************************
 * @鍑芥暟鍚嶇О	Receive_Byte
 * @鍑芥暟璇存槑   浠庡彂閫佺鎺ユ敹涓€涓瓧鑺?
 * @杈撳叆鍙傛暟   c: 鎺ユ敹瀛楃
                timeout: 瓒呮椂鏃堕棿
 * @杈撳嚭鍙傛暟   鏃?
 * @杩斿洖鍙傛暟   鎺ユ敹鐨勭粨鏋?
                0锛氭垚鍔熸帴鏀?
                1锛氭椂闂磋秴鏃?
 *******************************************************************************/
static int32_t Receive_Byte (uint8_t *c, uint32_t timeout)
{

    while (timeout-- > 0)
    {
        if( CSS_UART_Read_RB(&huart2, c)== 0)		//updata_huart
        {
            return 0;
        }
    }
    return -1;
}

/*******************************************************************************
 * @鍑芥暟鍚嶇О	Send_Byte
 * @鍑芥暟璇存槑   鍙戦€佷竴涓瓧绗?
 * @杈撳叆鍙傛暟   c: 鍙戦€佺殑瀛楃
 * @杈撳嚭鍙傛暟   鏃?
 * @杩斿洖鍙傛暟   鍙戦€佺殑缁撴灉
                0锛氭垚鍔熷彂閫?
 *******************************************************************************/
static uint32_t Send_Byte (uint8_t c)
{
#if 1
	if(g_uart2_use_rs485_control == 1){
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);//GPIO_PIN_SET); 
		HAL_UART_Transmit(&huart2,&c,1,1000);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);//GPIO_PIN_SET);
	}
	else {
		HAL_UART_Transmit(&huart2,&c,1,1000);
	}
#else
	HAL_UART_Transmit(&huart1,&c,1,1000);
#endif
    return 0;
}

/*******************************************************************************
 * @函数名称	Send_Byte
 * @函数说明   发送一个字符
 * @输入参数   c: 发送的字符
 * @输出参数   无
 * @返回参数   发送的结果
                0：成功发送
 *******************************************************************************/
uint32_t Send_Bytes (uint8_t* c)
{
#if 1
	if(g_uart2_use_rs485_control == 1){

		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);//GPIO_PIN_SET);
	    HAL_UART_Transmit(&huart2,c,strlen(c),1000);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);//GPIO_PIN_SET);
	}
	else {
	    HAL_UART_Transmit(&huart2,c,strlen(c),1000);
	}
#else	
	HAL_UART_Transmit(&huart1,c,strlen(c),1000);
#endif
    return 0;
}



/*******************************************************************************
 * @鍑芥暟鍚嶇О	crc16
 * @鍑芥暟璇存槑   璁＄畻crc16 锛坸16+x12+x5+1锛?
 * @杈撳叆鍙傛暟
 * @杈撳嚭鍙傛暟
 * @杩斿洖鍙傛暟   crc16
 *******************************************************************************/
static unsigned short crc16(const unsigned char *buf, unsigned long count)
{
    unsigned short crc = 0;
    int i;

    while(count--)
    {
        crc = crc ^ *buf++ << 8;
        for (i=0; i<8; i++)
        {
            if(crc & 0x8000)
            {
                crc = crc << 1 ^ 0x1021;
            }
            else
            {
                crc = crc << 1;
            }
        }
    }
    return crc;
}

/*******************************************************************************
 * @鍑芥暟鍚嶇О	Receive_Packet
 * @鍑芥暟璇存槑   浠庡彂閫佺鎺ユ敹涓€涓暟鎹寘
 * @杈撳叆鍙傛暟   data 锛氭暟鎹寚閽?
                length锛氶暱搴?
                timeout 锛氳秴鏃舵椂闂?
 * @杈撳嚭鍙傛暟   鏃?
 * @杩斿洖鍙傛暟   鎺ユ敹鐨勭粨鏋?
                0: 姝ｅ父杩斿洖
                -1: 瓒呮椂鎴栬€呮暟鎹寘閿欒
                1: 鐢ㄦ埛鍙栨秷
 *******************************************************************************/
static iap_err_t Receive_Packet (uint8_t *data, int32_t *length, uint32_t timeout)
{
    uint16_t i, packet_size;
    uint8_t c;
    *length = 0;
    if (Receive_Byte(&c, timeout) != 0)
    {
		//printf("[line %d:%s]err:g_Uart3RecvCount \r\n",__LINE__,__func__);	
		printf("[line %d:%s]err:g_Uart3RecvCount is %d\r\n",__LINE__,__func__, g_Uart3RecvCount);
        return IAP_ERR_YMODEM;
    }
    switch (c)
    {
    case SOH:
        packet_size = PACKET_SIZE;
		printf("[line %d:%s]ok PACKET_SIZE\r\n",__LINE__,__func__);
        break;
    case STX:
        packet_size = PACKET_1K_SIZE;
		printf("[line %d:%s]ok  PACKET_1K_SIZE\r\n",__LINE__,__func__);
        break;
    case EOT:
		printf("[line %d:%s]ok\r\n",__LINE__,__func__);
        return IAP_OK;
    case CAN:
        if ((Receive_Byte(&c, timeout) == 0) && (c == CAN))
        {
            *length = IAP_ERR_DATA;
			printf("[line %d:%s]ok\r\n",__LINE__,__func__);
            return IAP_OK;
        }
        else
        {
			printf("[line %d:%s]err\r\n",__LINE__,__func__);
            return IAP_ERR_DATA;
        }
    default:
			printf("[line %d:%s]err\r\n",__LINE__,__func__);
        return IAP_ERR_DATA;
    }

    /* receive data */
    *data = c;
    for (i = 1; i < (packet_size + PACKET_OVERHEAD); i++)
    {
        if (Receive_Byte(data + i, timeout) != 0)
        {
			printf("[line %d:%s]err\r\n",__LINE__,__func__);
            return IAP_ERR_DATA;
        }
    }
    if (data[PACKET_SEQNO_INDEX] != ((data[PACKET_SEQNO_COMP_INDEX] ^ 0xff) & 0xff))
    {
		printf("[line %d:%s]err\r\n",__LINE__,__func__);
        return IAP_ERR_DATA;
    }
    *length = packet_size;

    uint16_t crc = crc16(data + PACKET_HEADER, packet_size);

    if(crc != ((data[PACKET_HEADER + packet_size]<<8) + data[PACKET_HEADER + packet_size+1]) )
    {
		printf("[line %d:%s]err\r\n",__LINE__,__func__);
        return IAP_ERR_DATA;
    }

	printf("[line %d:%s]ok\r\n",__LINE__,__func__);
    return IAP_OK;
}

/*******************************************************************************
 * @鍑芥暟鍚嶇О	Ymodem_Receive
 * @鍑芥暟璇存槑   閫氳繃 ymodem鍗忚鎺ユ敹涓€涓枃浠?
 * @杈撳叆鍙傛暟   buf: 棣栧湴鍧€鎸囬拡
 * @杈撳嚭鍙傛暟   鏃?
 * @杩斿洖鍙傛暟   鏂囦欢闀垮害
 *******************************************************************************/
iap_err_t Ymodem_Receive (uint8_t *buf)
{
    uint8_t packet_data[PACKET_1K_SIZE + PACKET_OVERHEAD],  *file_ptr;
    int32_t i, packet_length, session_done, file_done, packets_received, errors, session_begin;
    uint32_t firmware_size=0,file_size=0,wLastLength = 0;
    char file_name_str[FILE_NAME_LENGTH];
    char file_size_str[FILE_SIZE_LENGTH];
    uint32_t flash_err;
	uint32_t wPacketTotalNum = 0;
    int time;
    //gsw add 2018.07.09
    //加入超时限制，20S
    // 初始化Flash地址变量
    flash_start_addr = APPLICATION_ADDRESS;
	time = 0;
    for (session_done = 0, errors = 0, session_begin = 0;; )
    {
        for(packets_received = 0, file_done = 0;; )
        {
					if(time>30000)
					{
						break;
					}
					time = HAL_GetTick();
            switch (Receive_Packet(packet_data, &packet_length, NAK_TIMEOUT))
            {
            case IAP_OK:
                errors = 0;
                switch (packet_length)
                {
                /* 鍙戦€佺缁堟 */
                case -1:
                    Send_Byte(ACK);
					//Send_Bytes("ACK\n");
					//printf("[line %d:%s]err\r\n",__LINE__,__func__);
                    return IAP_ERR_YMODEM;

                /* 缁撴潫浼犺緭 */
                case 0:
                    Send_Byte(ACK);
					//Send_Bytes("ACK\n");
					file_done = 1;
					session_done = 1;//heyi add ：表示结束了
                    break;

                /* 姝ｅ父鐨勬暟鎹寘 */
                default:
                    if ((packet_data[PACKET_SEQNO_INDEX] & 0xff) != (packets_received & 0xff))
                    {
						Send_Byte(NAK);
						//Send_Bytes("NAK\n");
						printf("packet_data[PACKET_SEQNO_INDEX] is %d, packets_received is %d will send nak\r\n", packet_data[PACKET_SEQNO_INDEX],packets_received);
							printf("packet%d : \r\n", packet_data[PACKET_SEQNO_INDEX]);
							for(i = 0; i < packet_length; i++){
								printf("0x%2x, ", packet_data[i]);
							}
							printf("\r\n");
						//packets_received--;
						continue;//heyi add
                    }
                    else
                    {
                        if (packets_received == 0)
                        {
							printf("packet0 : \r\n");
							for(i = 0; i < packet_length; i++){
								printf("0x%2x, ", packet_data[i]);
						}
						printf("\r\n");
                            if (packet_data[PACKET_HEADER] != 0)
                            {
                                // 鏂囦欢鍚嶆暟鎹寘鏈夋晥鏁版嵁鍖哄煙
                                for (i = 0, file_ptr = packet_data + PACKET_HEADER; (*file_ptr != 0) && (i < FILE_NAME_LENGTH); )
                                {
                                    file_name_str[i++] = *file_ptr++;
                                }
                                file_name_str[i++] = '\0';
								printf("[%s]:%s \r\n",__func__,file_name_str);
                                if(strcmp(file_name_str,FILE_NAME)!=0)
                                {
                                    // 缁撴潫
                                    Send_Byte(CAN);
									//Send_Bytes("CAN\n");
									Send_Byte(CAN);
									//Send_Bytes("CAN\n");
									printf("[%s:%s]err\r\n",__LINE__,__func__);
                                    return IAP_ERR_FILENAME;
                                }

                                for (i = 0, file_ptr++; (*file_ptr != 0) && (i < FILE_SIZE_LENGTH); )  //*file_ptr != ' '
                                {
                                    file_size_str[i++] = *file_ptr++;
                                }
                                file_size_str[i++] = '\0';

								/* 文件大小转为int */
								//file_size = atoi(file_size_str);
								ReadFileLength(file_size_str, (i-1), &file_size);
								wPacketTotalNum = file_size/PACKET_1K_SIZE;
								wLastLength     = file_size%PACKET_1K_SIZE;
								printf("[%s]:file_size is %u, total packet is %d, last packet len is %d\r\n",__func__,file_size,wPacketTotalNum,wLastLength);
                                // 娴嬭瘯鏁版嵁鍖呮槸鍚﹁繃澶?
                                if (file_size > (APPLICATION_SIZE - 1))
                                {
                                    // 缁撴潫
                                    Send_Byte(CAN);
									//Send_Bytes("CAN\n");
									Send_Byte(CAN);
									//Send_Bytes("CAN\n");
									printf("[line %d:%s]err\r\n",__LINE__,__func__);
                                    return IAP_ERR_DATA;
                                }
								printf("[line %d:%s]receive packet 0,will send ack\r\n",__LINE__,__func__);
                                Send_Byte(ACK);
                                Send_Byte(STARTC);
								//Send_Bytes("ACK\n");
								//Send_Bytes("STARTC\n");
                            }
							// 文件名数据包空，结束传输
                            else
                            {
                                Send_Byte(ACK);
								//Send_Bytes("ACK\n");
                                file_done = 1;
                                session_done = 1;
                                break;
                            }
                        }
                        // 鏁版嵁鍖?
                        else
                        {
                            memcpy(flash_data_buf, packet_data + PACKET_HEADER, packet_length);

                            flash_err = FLASH_If_Write(flash_start_addr, flash_data_buf, packet_length);
                            
                            if(flash_err == FLASHIF_OK)
                            {
                                flash_start_addr += packet_length;

                            }
                            else
                            {
                                // 缁撴潫
                                Send_Byte(CAN);
                                
								//Send_Bytes("CAN\n");
								Send_Byte(CAN);
								//Send_Bytes("CAN\n");
								printf("[line %d:%s]err\r\n",__LINE__,__func__);
                                return IAP_ERR_FLASH;
                            }
                            
														/* ymodem为整包发送，未满的以0x1A补 ,且是最后一帧数据*/
                            if(*(packet_data + PACKET_HEADER + packet_length -1) == 0x1A)
                            {
                                // 涓嶆弧鍖?
                                
								//printf("[line %d:%s]&&&&&&&firmware_size is %u, packet_id is %d, packet_len is %d\r\n",__LINE__,__func__,firmware_size,packets_received,packet_length);
								#if 0
								// 不满包
								for( i = 0, file_ptr = packet_data + PACKET_HEADER; i<= packet_length ;i++  )
								{
									if(*(file_ptr++)==0x1A )
									{
										printf("[line %d:%s]firmware_size is %u\r\n",__LINE__,__func__,firmware_size);
										//firmware_size +=i;
										printf("[line %d:%s]firmware_size is %u\r\n",__LINE__,__func__,(firmware_size+i));
																				
										break;
									}
								}
								#endif		

								//不满包
								for(i =  PACKET_HEADER + packet_length -1; i >  PACKET_HEADER; i--){
						
									if(*(packet_data+i) != 0x1A){
										wLastLength = i-PACKET_HEADER+1;
										printf("[line %d:%s]last packet length is %u\r\n",__LINE__,__func__, wLastLength);
										firmware_size += wLastLength; 														
										printf("[line %d:%s]firmware_size is %u\r\n",__LINE__,__func__, firmware_size);
                                        break;
                                    }
                                }
                            }
                            else
                            {
                                firmware_size += packet_length;
								printf("[line %d:%s]firmware_size is %u, packet_id is %d, packet_len is %d\r\n",__LINE__,__func__,firmware_size,packets_received,packet_length);
                            }
                            
                            Send_Byte(ACK);
							//Send_Bytes("ACK\n");
                        }
                        packets_received++;
                        session_begin = 1;
                    }
                }
                break;
            default:      // 鍙戦€丆 璇锋眰鏂囦欢
                if (session_begin > 0)
                {
                    errors++;
                }
                if (errors > MAX_ERRORS)
                {
                    Send_Byte(CAN);
					//Send_Bytes("CAN\n");
					Send_Byte(CAN);
					//Send_Bytes("CAN\n");
					printf("[line %d:%s]err\r\n",__LINE__,__func__);
                    return IAP_ERR_YMODEM;
                }
                Send_Byte(STARTC);
				printf("STARTC \r\n");
				//Send_Bytes("STARTC\n");
				//Send_Bytes("STARTC\n");				
				//Send_Bytes("STARTC\n");				
				//Send_Bytes("STARTC\n");				

                break;
            }
            if (file_done != 0)
            {
                break;
            }
        }
        if (session_done != 0)
		{
			break;
		}
		if(time>2000)
		{
            break;
        }
    }
    if( file_size == firmware_size)
    {
        if(firmware_size >0)
        {
            return IAP_OK;
        }
        else
        {
			printf("[line %d:%s]err\r\n",__LINE__,__func__);
            return IAP_ERR_DATA;
        }
    }
    else
    {
		printf("[line %d:%s]err file_size is %u, firmware_size is %u\r\n",__LINE__,__func__,file_size,firmware_size);
        return IAP_ERR_FILESIZE;
    }
    
}

/*******************************END***************************************/

