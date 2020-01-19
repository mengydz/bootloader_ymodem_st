/**
  ******************************************************************************
  * @file    iap_if.c
  * @author  CSSiot
  * @version V1.4.0
  * @date    29-April-2016
  * @brief   This file provides all the iap if functions.
  *          forked from STM32 iap_if
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

/** @addtogroup STM32F1xx_IAP
  * @{
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "common.h"
#include "iap_if.h"
#include "ymodem.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint32_t FlashProtection = 0;

volatile struct IAPFlashInfo * flash_info = (struct IAPFlashInfo *)FLASH_INFO_ADDRESS;
struct IAPFlashInfo flash_info_shadow;

uint8_t tab_1024[1024] ={0};

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/**
  * @brief IAP get application address
  *       We will check whether application is valid. If not, try recover app
  *       from backup.
  * @param app_addr application address
  *
  * @return iap_err_t
  */
iap_err_t IAP_GetAppAddr(uint32_t *app_addr)
{
    uint32_t app_start_addr = APPLICATION_ADDRESS;
    uint32_t bak_start_addr = FLASH_BACKUP_ADDRESS;
    uint32_t info_start_addr = FLASH_INFO_ADDRESS;
    uint32_t info_size;
    uint8_t app_valid = 0;
    iap_err_t retval = IAP_OK;

    info_size = sizeof(struct IAPFlashInfo);

    /* Flash info is present */
    if(flash_info->app_magic == APP_MAGIC_NUMBER)
    {
        flash_info_shadow = *flash_info;
        /* Recover app from backup */
        if(flash_info->app_valid != 1)
        {
            if(flash_info->app_backup_valid == 1)
            {
                if(FLASH_If_Modify(app_start_addr, (uint32_t *)bak_start_addr, APPLICATION_SIZE) != FLASHIF_OK)
                {
                    return IAP_ERR_FLASH;
                }

                flash_info_shadow.app_valid = 1;
                if(FLASH_If_Modify(info_start_addr, (uint32_t *)&flash_info_shadow, info_size) != FLASHIF_OK)
                {
                    return IAP_ERR_FLASH;
                }
            }
        }
        if(flash_info->app_valid == 1)
        {
            /* Check application zero address content
              * Test if user code is programmed starting from address
              * */
            if (((*(__IO uint32_t*)app_start_addr) & 0x2FFE0000 ) == 0x20000000)
            {
                app_valid = 1;
            }
        }
    }
    /* There is no flash info */
    else
    {
        /* Check application zero address content
          * Test if user code is programmed starting from address
          * */
        if (((*(__IO uint32_t*)app_start_addr) & 0x2FFE0000 ) == 0x20000000)
        {
            app_valid = 1;

            //flash_info_shadow.magic_flag = IAP_FLASH_INFO_MAGIC;
			flash_info_shadow.app_magic = APP_MAGIC_NUMBER;
            flash_info_shadow.app_valid = 1;
            flash_info_shadow.app_backup_valid = 0;
            if(FLASH_If_Modify(info_start_addr, (uint32_t *)&flash_info_shadow, info_size) != FLASHIF_OK)
            {
                return IAP_ERR_FLASH;
            }
        }
    }

    /* Output app addr */
    if(app_valid == 1)
    {
        *app_addr = app_start_addr;
        retval = IAP_OK;
    }
    else
    {
        retval = IAP_ERR_DATA;
    }

    return retval;
}

/**
  * @brief  IAP get binary image from TFTP server, and update the flash
  * @param  None
  * @retval iap_err_t
  */
iap_err_t IAP_UpdateImage(void)
{
    uint32_t app_start_addr = APPLICATION_ADDRESS;
    uint32_t bak_start_addr = FLASH_BACKUP_ADDRESS;
    uint32_t info_start_addr = FLASH_INFO_ADDRESS;
    uint32_t info_size;
    iap_err_t retval = IAP_OK;
    uint32_t flash_status;
    
    info_size = sizeof(struct IAPFlashInfo);

    /* Test if any sector of Flash memory where user application will be loaded is write protected */
    FlashProtection = FLASH_If_GetWRPStatus();

    if (FlashProtection != FLASHIF_PROTECTION_NONE)
    {
        printf("[Flash] flash is locked\n");
        return IAP_ERR_FLASH;
    }

    flash_info_shadow = *flash_info;

    /* App里有代码*/
    if(flash_info->app_valid == 1)
    {
        printf("[Flash] backup application...\n");
        if(FLASH_If_Modify(bak_start_addr, (uint32_t *)app_start_addr, APPLICATION_SIZE) != FLASHIF_OK)
        {
            return IAP_ERR_FLASH;
        }
        flash_info_shadow.app_backup_valid = 1;
    }
    

    /* update Flash Info  into intel rom*/
    printf("[Flash] update info\n");
    //flash_info_shadow.magic_flag = IAP_FLASH_INFO_MAGIC;
    flash_info_shadow.app_valid = 0;
    if(FLASH_If_Modify(info_start_addr, (uint32_t *)&flash_info_shadow, info_size) != FLASHIF_OK)
    {
        return IAP_ERR_FLASH;
    }

    /* erase app flash */
    flash_status = FLASH_If_Erase(app_start_addr, APPLICATION_SIZE);

    if(flash_status != FLASHIF_OK)
    {

        return IAP_ERR_FLASH;
    }   
    
    retval = Ymodem_Receive(tab_1024);

    if(retval != IAP_OK)
    {
        /* receive fail, app recover backup*/
        printf("[Flash] recover backup...\n");
        if(FLASH_If_Modify(app_start_addr, (uint32_t *)bak_start_addr, APPLICATION_SIZE) != FLASHIF_OK)
        {
            return IAP_ERR_FLASH;
        }
    }
 

    /* update Flash Info App */
    flash_info_shadow.app_valid = 1;
	flash_info_shadow.app_update_flag = 0;//不管升级成功还是失败，都清零，否则会一直卡在升级界面，无法运行app程序
    if(FLASH_If_Modify(info_start_addr, (uint32_t *)&flash_info_shadow, info_size) != FLASHIF_OK)
    {
        return IAP_ERR_FLASH;
    }

    return retval;
}


/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

