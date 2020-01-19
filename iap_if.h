/**
  ******************************************************************************
  * @file    iap_if.h
  * @author  MCD Application Team with modification
  * @author  CSSiot
  * @version V1.4.0
  * @date    29-April-2016
  * @brief   This file provides all the headers of the iap if functions.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __IAP_IF_H
#define __IAP_IF_H

/* Includes ------------------------------------------------------------------*/
#include "flash_if.h"

/* Defines ------------------------------------------------------------------*/
#define IAP_FLASH_INFO_MAGIC 0x33CC

/* Imported variables --------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
typedef void (*pFunc)(void);

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/* Error code */
typedef enum IAPErrCode
{
    IAP_OK = 0, 
    IAP_ERR_DATA,  
    IAP_ERR_FILENAME,
    IAP_ERR_FILESIZE,
    IAP_ERR_YMODEM,
    IAP_ERR_FLASH
} iap_err_t;

/* Flash Info
  * 闀垮害蹇呴』灏忎簬 FLASH_INFO_SIZE
  * 鎻忚堪 App 鍜?Backup 鍖哄煙鏁版嵁鏄惁瀹屾暣 */


#if 1
#define APP_MAGIC_NUMBER 0xa55a0aa0
struct IAPFlashInfo{
	int magic;				//if magic = 0xa0, this gyro_setting is useful
	float gyro_offset;
	float angle;
	int app_magic; 		//0xa55a0aa0 list app_settings is useful
	unsigned char app_update_flag;	//1:need to update; 0: no need to update
	unsigned char app_info_valid;	//1:this struct is useful;0:not useful
	unsigned char app_valid;		//1:app0 zone is useful;0:app0 codes not useful
	unsigned char app_backup_valid;  //1:app1 codes is useful;0:app1 codes not useful


//	uint16_t magic_flag; // 鏍囪瘑鏁版嵁鏄惁鏈夋晥鐨勯瓟鏁?
//    uint8_t app_valid;  // Application 鍖哄煙鏈夋晥
//    uint8_t backup_valid;  // Backup 鍖哄煙鏈夋晥
};

#else
struct IAPFlashInfo
{
    uint16_t magic_flag; // 鏍囪瘑鏁版嵁鏄惁鏈夋晥鐨勯瓟鏁?
    uint8_t app_valid;  // Application 鍖哄煙鏈夋晥
    uint8_t backup_valid;  // Backup 鍖哄煙鏈夋晥
};


#endif


/* Exported macro ------------------------------------------------------------*/
#define ME3630_SOCK_ID   (1)

/* Exported functions ------------------------------------------------------- */
iap_err_t IAP_GetAppAddr(uint32_t *app_addr);
iap_err_t IAP_UpdateImage(void);
void IAP_LedRedToggle(int ms);
void IAP_LedBlueToggle(int ms);

#endif  /* __MENU_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

