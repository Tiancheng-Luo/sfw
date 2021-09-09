/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>
#include "fsl_sd.h"
#include "fsl_debug_console.h"
#include "ff.h"
#include "diskio.h"
#include "fsl_sd_disk.h"
#include "board.h"
#include "sdmmc_config.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_common.h"
#include "fsl_iopctl.h"

/* MCUboot */
#include "flexspi_flash.h"
#include "sysflash.h"
#include "image.h"
#include "bootutil_priv.h"

#include "flexspi_flash.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"
#include "sbl_ota_flag.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define BUFFER_SIZE (4096U) 
#define VERSION32(hdr) (((hdr)->ih_ver.iv_major << 24) | ((hdr)->ih_ver.iv_minor << 16) | ((hdr)->ih_ver.iv_revision))

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void SDCARD_DetectCallBack(bool isInserted, void *userData);

const char imageName_sd[] = "newapp";
/*******************************************************************************
 * Variables
 ******************************************************************************/
static volatile bool s_cardInserted     = false;
static volatile bool s_cardInsertStatus = false;

static SemaphoreHandle_t s_CardDetectSemaphore = NULL;

static FATFS g_fileSystem;
static FIL g_fileObject; 

SDK_ALIGN(uint8_t g_bufferRead[SDK_SIZEALIGN(BUFFER_SIZE, SDMMC_DATA_BUFFER_ALIGN_CACHE)],
          MAX(SDMMC_DATA_BUFFER_ALIGN_CACHE, 4));

struct image_header *pri_hdr = (struct image_header *)(FLASH_DEVICE_BASE_ADDR + FLASH_AREA_IMAGE_1_OFFSET);

/*******************************************************************************
 * Code
 ******************************************************************************/
void BOARD_SD_InitPins(void) {
    const uint32_t port1_pin30_config = (/* Pin is configured as SD0_CLK */
                                         IOPCTL_PIO_FUNC1 |
                                         /* Disable pull-up / pull-down function */
                                         IOPCTL_PIO_PUPD_DI |
                                         /* Enable pull-down function */
                                         IOPCTL_PIO_PULLDOWN_EN |
                                         /* Enables input buffer function */
                                         IOPCTL_PIO_INBUF_EN |
                                         /* Normal mode */
                                         IOPCTL_PIO_SLEW_RATE_NORMAL |
                                         /* Normal drive */
                                         IOPCTL_PIO_FULLDRIVE_DI |
                                         /* Analog mux is disabled */
                                         IOPCTL_PIO_ANAMUX_DI |
                                         /* Pseudo Output Drain is disabled */
                                         IOPCTL_PIO_PSEDRAIN_DI |
                                         /* Input function is not inverted */
                                         IOPCTL_PIO_INV_DI);
    /* PORT1 PIN30 (coords: R5) is configured as SD0_CLK */
    IOPCTL_PinMuxSet(IOPCTL, 1U, 30U, port1_pin30_config);

    const uint32_t port1_pin31_config = (/* Pin is configured as SD0_CMD */
                                         IOPCTL_PIO_FUNC1 |
                                         /* Enable pull-up / pull-down function */
                                         IOPCTL_PIO_PUPD_EN |
                                         /* Enable pull-up function */
                                         IOPCTL_PIO_PULLUP_EN |
                                         /* Enables input buffer function */
                                         IOPCTL_PIO_INBUF_EN |
                                         /* Normal mode */
                                         IOPCTL_PIO_SLEW_RATE_NORMAL |
                                         /* Normal drive */
                                         IOPCTL_PIO_FULLDRIVE_DI |
                                         /* Analog mux is disabled */
                                         IOPCTL_PIO_ANAMUX_DI |
                                         /* Pseudo Output Drain is disabled */
                                         IOPCTL_PIO_PSEDRAIN_DI |
                                         /* Input function is not inverted */
                                         IOPCTL_PIO_INV_DI);
    /* PORT1 PIN31 (coords: R6) is configured as SD0_CMD */
    IOPCTL_PinMuxSet(IOPCTL, 1U, 31U, port1_pin31_config);

    const uint32_t port2_pin0_config = (/* Pin is configured as SD0_D0 */
                                        IOPCTL_PIO_FUNC1 |
                                        /* Enable pull-up / pull-down function */
                                        IOPCTL_PIO_PUPD_EN |
                                        /* Enable pull-up function */
                                        IOPCTL_PIO_PULLUP_EN |
                                        /* Enables input buffer function */
                                        IOPCTL_PIO_INBUF_EN |
                                        /* Normal mode */
                                        IOPCTL_PIO_SLEW_RATE_NORMAL |
                                        /* Normal drive */
                                        IOPCTL_PIO_FULLDRIVE_DI |
                                        /* Analog mux is disabled */
                                        IOPCTL_PIO_ANAMUX_DI |
                                        /* Pseudo Output Drain is disabled */
                                        IOPCTL_PIO_PSEDRAIN_DI |
                                        /* Input function is not inverted */
                                        IOPCTL_PIO_INV_DI);
    /* PORT2 PIN0 (coords: U4) is configured as SD0_D0 */
    IOPCTL_PinMuxSet(IOPCTL, 2U, 0U, port2_pin0_config);

    const uint32_t port2_pin1_config = (/* Pin is configured as SD0_D1 */
                                        IOPCTL_PIO_FUNC1 |
                                        /* Enable pull-up / pull-down function */
                                        IOPCTL_PIO_PUPD_EN |
                                        /* Enable pull-up function */
                                        IOPCTL_PIO_PULLUP_EN |
                                        /* Enables input buffer function */
                                        IOPCTL_PIO_INBUF_EN |
                                        /* Normal mode */
                                        IOPCTL_PIO_SLEW_RATE_NORMAL |
                                        /* Normal drive */
                                        IOPCTL_PIO_FULLDRIVE_DI |
                                        /* Analog mux is disabled */
                                        IOPCTL_PIO_ANAMUX_DI |
                                        /* Pseudo Output Drain is disabled */
                                        IOPCTL_PIO_PSEDRAIN_DI |
                                        /* Input function is not inverted */
                                        IOPCTL_PIO_INV_DI);
    /* PORT2 PIN1 (coords: T4) is configured as SD0_D1 */
    IOPCTL_PinMuxSet(IOPCTL, 2U, 1U, port2_pin1_config);

    const uint32_t SD0_RESET_N = (/* Pin is configured as PIO2_10 */
                                  IOPCTL_PIO_FUNC0 |
                                  /* Disable pull-up / pull-down function */
                                  IOPCTL_PIO_PUPD_DI |
                                  /* Enable pull-down function */
                                  IOPCTL_PIO_PULLDOWN_EN |
                                  /* Disable input buffer function */
                                  IOPCTL_PIO_INBUF_DI |
                                  /* Normal mode */
                                  IOPCTL_PIO_SLEW_RATE_NORMAL |
                                  /* Normal drive */
                                  IOPCTL_PIO_FULLDRIVE_DI |
                                  /* Analog mux is disabled */
                                  IOPCTL_PIO_ANAMUX_DI |
                                  /* Pseudo Output Drain is disabled */
                                  IOPCTL_PIO_PSEDRAIN_DI |
                                  /* Input function is not inverted */
                                  IOPCTL_PIO_INV_DI);
    /* PORT2 PIN10 (coords: N5) is configured as PIO2_10 */
    IOPCTL_PinMuxSet(IOPCTL, 2U, 10U, SD0_RESET_N);

    const uint32_t port2_pin2_config = (/* Pin is configured as SD0_D2 */
                                        IOPCTL_PIO_FUNC1 |
                                        /* Enable pull-up / pull-down function */
                                        IOPCTL_PIO_PUPD_EN |
                                        /* Enable pull-up function */
                                        IOPCTL_PIO_PULLUP_EN |
                                        /* Enables input buffer function */
                                        IOPCTL_PIO_INBUF_EN |
                                        /* Normal mode */
                                        IOPCTL_PIO_SLEW_RATE_NORMAL |
                                        /* Normal drive */
                                        IOPCTL_PIO_FULLDRIVE_DI |
                                        /* Analog mux is disabled */
                                        IOPCTL_PIO_ANAMUX_DI |
                                        /* Pseudo Output Drain is disabled */
                                        IOPCTL_PIO_PSEDRAIN_DI |
                                        /* Input function is not inverted */
                                        IOPCTL_PIO_INV_DI);
    /* PORT2 PIN2 (coords: T7) is configured as SD0_D2 */
    IOPCTL_PinMuxSet(IOPCTL, 2U, 2U, port2_pin2_config);

    const uint32_t port2_pin3_config = (/* Pin is configured as SD0_D3 */
                                        IOPCTL_PIO_FUNC1 |
                                        /* Enable pull-up / pull-down function */
                                        IOPCTL_PIO_PUPD_EN |
                                        /* Enable pull-up function */
                                        IOPCTL_PIO_PULLUP_EN |
                                        /* Enables input buffer function */
                                        IOPCTL_PIO_INBUF_EN |
                                        /* Normal mode */
                                        IOPCTL_PIO_SLEW_RATE_NORMAL |
                                        /* Normal drive */
                                        IOPCTL_PIO_FULLDRIVE_DI |
                                        /* Analog mux is disabled */
                                        IOPCTL_PIO_ANAMUX_DI |
                                        /* Pseudo Output Drain is disabled */
                                        IOPCTL_PIO_PSEDRAIN_DI |
                                        /* Input function is not inverted */
                                        IOPCTL_PIO_INV_DI);
    /* PORT2 PIN3 (coords: U6) is configured as SD0_D3 */
    IOPCTL_PinMuxSet(IOPCTL, 2U, 3U, port2_pin3_config);

    const uint32_t SD0_WR_PRT_DS = (/* Pin is configured as PIO2_4 */
                                    IOPCTL_PIO_FUNC0 |
                                    /* Disable pull-up / pull-down function */
                                    IOPCTL_PIO_PUPD_DI |
                                    /* Enable pull-down function */
                                    IOPCTL_PIO_PULLDOWN_EN |
                                    /* Disable input buffer function */
                                    IOPCTL_PIO_INBUF_DI |
                                    /* Normal mode */
                                    IOPCTL_PIO_SLEW_RATE_NORMAL |
                                    /* Normal drive */
                                    IOPCTL_PIO_FULLDRIVE_DI |
                                    /* Analog mux is disabled */
                                    IOPCTL_PIO_ANAMUX_DI |
                                    /* Pseudo Output Drain is disabled */
                                    IOPCTL_PIO_PSEDRAIN_DI |
                                    /* Input function is not inverted */
                                    IOPCTL_PIO_INV_DI);
    /* PORT2 PIN4 (coords: P6) is configured as PIO2_4 */
    IOPCTL_PinMuxSet(IOPCTL, 2U, 4U, SD0_WR_PRT_DS);

    const uint32_t port2_pin9_config = (/* Pin is configured as SD0_CARD_DET_N */
                                        IOPCTL_PIO_FUNC1 |
                                        /* Enable pull-up / pull-down function */
                                        IOPCTL_PIO_PUPD_EN |
                                        /* Enable pull-up function */
                                        IOPCTL_PIO_PULLUP_EN |
                                        /* Enables input buffer function */
                                        IOPCTL_PIO_INBUF_EN |
                                        /* Normal mode */
                                        IOPCTL_PIO_SLEW_RATE_NORMAL |
                                        /* Normal drive */
                                        IOPCTL_PIO_FULLDRIVE_DI |
                                        /* Analog mux is disabled */
                                        IOPCTL_PIO_ANAMUX_DI |
                                        /* Pseudo Output Drain is disabled */
                                        IOPCTL_PIO_PSEDRAIN_DI |
                                        /* Input function is not inverted */
                                        IOPCTL_PIO_INV_DI);
    /* PORT2 PIN9 (coords: T3) is configured as SD0_CARD_DET_N */
    IOPCTL_PinMuxSet(IOPCTL, 2U, 9U, port2_pin9_config);

    const uint32_t port4_pin0_config = (/* Pin is configured as PIO4_0 */
                                        IOPCTL_PIO_FUNC0 |
                                        /* Disable pull-up / pull-down function */
                                        IOPCTL_PIO_PUPD_DI |
                                        /* Enable pull-down function */
                                        IOPCTL_PIO_PULLDOWN_EN |
                                        /* Disable input buffer function */
                                        IOPCTL_PIO_INBUF_DI |
                                        /* Normal mode */
                                        IOPCTL_PIO_SLEW_RATE_NORMAL |
                                        /* Normal drive */
                                        IOPCTL_PIO_FULLDRIVE_DI |
                                        /* Analog mux is disabled */
                                        IOPCTL_PIO_ANAMUX_DI |
                                        /* Pseudo Output Drain is disabled */
                                        IOPCTL_PIO_PSEDRAIN_DI |
                                        /* Input function is not inverted */
                                        IOPCTL_PIO_INV_DI);
    /* PORT4 PIN0 (coords: N15) is configured as PIO4_0 */
    IOPCTL_PinMuxSet(IOPCTL, 4U, 0U, port4_pin0_config);
}

void local_version(void)
{
    if (pri_hdr->ih_magic == IMAGE_MAGIC)
        PRINTF("Local image version: %d.%d.%d\r\n", pri_hdr->ih_ver.iv_major,
	                                                pri_hdr->ih_ver.iv_minor,
                                                    pri_hdr->ih_ver.iv_revision);
    else
        PRINTF("Failed to get version number\r\n");
}

void SDCARD_DetectCallBack(bool isInserted, void *userData)
{
    s_cardInsertStatus = isInserted;
    xSemaphoreGiveFromISR(s_CardDetectSemaphore, NULL);
}

status_t DEMO_SDCardOTAUpgrade(void)
{
    char *image_title = NULL;
    char *dir;
    int dir_len;
    FRESULT error;
    DIR directory = {0};
    FILINFO fileInformation;
    UINT bytesRead, buffer_size;
    
    struct image_version *cur_ver;
    struct image_header *new_hdr;
    uint8_t version_buff[8];
    int8_t cmp_result;
    uint8_t image_position;
    uint32_t dstAddr = FLASH_AREA_IMAGE_2_OFFSET;
   
    status_t status;
    volatile uint32_t primask;
    
    const TCHAR driverNumberBuffer[3U] = {SDDISK + '0', ':', '/'};

    sfw_flash_read(REMAP_FLAG_ADDRESS, &image_position, 1);
    if(image_position == 0x01)
    {
        dstAddr = FLASH_AREA_IMAGE_2_OFFSET;
        sfw_flash_read_ipc(FLASH_AREA_IMAGE_1_OFFSET + IMAGE_VERSION_OFFSET, version_buff, 8);
        cur_ver = (struct image_version *)version_buff;
    }
    else if(image_position == 0x02)
    {
        dstAddr = FLASH_AREA_IMAGE_1_OFFSET;
        sfw_flash_read_ipc(FLASH_AREA_IMAGE_2_OFFSET + IMAGE_VERSION_OFFSET, version_buff, 8);
        cur_ver = (struct image_version *)version_buff;
    }
    else
    {
        return kStatus_Fail;
    }
    
    dir = ".";
    dir_len = strlen(dir);
    if ((dir[dir_len - 1] == '/') || (dir[dir_len - 1] == '\\'))
        dir[dir_len - 1] = '\0';

    if (f_mount(&g_fileSystem, driverNumberBuffer, 0U))
    {
        return kStatus_Fail;
    }

#if (FF_FS_RPATH >= 2U)
    error = f_chdrive((char const *)&driverNumberBuffer[0U]);
    if (error)
    {
        return kStatus_Fail;
    }
#endif
    
    error = f_opendir(&directory, dir);
    if (error)
    {
        return kStatus_Fail;
    }
    
    //PRINTF("dir list:\r\n");
    for (;;)
    {
        error = f_readdir(&directory, &fileInformation);
        if ((error != FR_OK) || (fileInformation.fname[0U] == 0U)) 
        {
            break;
        }
        if (fileInformation.fname[0] == '.') 
        {
            continue;
        }
#if 0        
        if (fileInformation.fattrib & AM_DIR) {
            PRINTF("\033[40;33m%s\033[0m \r\n", fileInformation.fname);
        } else {
            PRINTF("%s \r\n", fileInformation.fname);
        }
#endif   
            if (strncmp(fileInformation.fname, imageName_sd, (sizeof(imageName_sd)-1)) == 0)
            {
                image_title = fileInformation.fname;
                break;
            }
    }
    
    error = f_open(&g_fileObject, _T(image_title), FA_READ);
    if (error) 
    {
        goto exit;
    }
    
    PRINTF("reading...\r\n");
    
    buffer_size = sizeof(g_bufferRead);
    error = f_read(&g_fileObject, g_bufferRead, buffer_size, &bytesRead);
    if (error || (bytesRead != buffer_size) ) 
    {
        PRINTF("failed to read image data\r\n");
        goto exit;
    }
    
    new_hdr = (struct image_header *)g_bufferRead;
    if (new_hdr->ih_magic == IMAGE_MAGIC) 
    {
        if(cur_ver->iv_major == 0xff && cur_ver->iv_minor == 0xff)
        {
            PRINTF("new img verison: %d.%d.%d\r\n", new_hdr->ih_ver.iv_major,
                                new_hdr->ih_ver.iv_minor,
                                new_hdr->ih_ver.iv_revision);
        }
        else
        {
            cmp_result = compare_image_version(&new_hdr->ih_ver, cur_ver);
            if(cmp_result > 0)
            {
                PRINTF("new img verison: %d.%d.%d\r\n", new_hdr->ih_ver.iv_major,
                                            new_hdr->ih_ver.iv_minor,
                                            new_hdr->ih_ver.iv_revision);
            }
            else
            {
                PRINTF("The version number of the new image is not greater than the current image version number, please retry!\r\n");
                goto exit;
            }
        }
    } 
    else 
    {
        PRINTF("the new image is invalid.\r\n");
        goto exit;
    }
    
    primask = DisableGlobalIRQ();
    status = sfw_flash_erase(dstAddr, FLASH_AREA_IMAGE_2_SIZE);
    if (status) 
    {
        goto exit;
    }
    EnableGlobalIRQ(primask);

    PRINTF("updating...\r\n");
    
    primask = DisableGlobalIRQ();
    //already read the first 4KB for image validation checking, write to flash directly
    status = sfw_flash_write(dstAddr, (uint32_t *)g_bufferRead, bytesRead);  
    if (status) 
    {
        goto exit;
    }
    EnableGlobalIRQ(primask);
    
    while((bytesRead == buffer_size)) 
    {  
        dstAddr += bytesRead;
        error = f_read(&g_fileObject, (void *)g_bufferRead, buffer_size, &bytesRead);
        if (error == FR_OK) 
        {
            primask = DisableGlobalIRQ();
            status = sfw_flash_write(dstAddr, (uint32_t *)g_bufferRead, bytesRead);
            if (status) 
            {
                goto exit;
            }
            EnableGlobalIRQ(primask);
        }
        else 
        {
            goto exit;
        }
    }
    
    PRINTF("finished\r\n");
    
    write_update_type(UPDATE_TYPE_SDCARD);
    enable_image();
    //taskEXIT_CRITICAL();
      
    PRINTF("Please remove the SD Card!\r\n");
    PRINTF("sys rst...\r\n\r\n");
    vTaskDelay(5000U);

#if 0
    primask = DisableGlobalIRQ();
    status = flexspi_nor_flash_reset_opi(EXAMPLE_FLEXSPI);
    if (status) 
    {
        goto exit;
    }
    EnableGlobalIRQ(primask);
#endif

    NVIC_SystemReset();
    
exit:
    //FLEXSPI_SoftwareReset(EXAMPLE_FLEXSPI); //bus fault
    if (f_close(&g_fileObject)) 
    {
        return kStatus_Fail;   
    }
    return kStatus_Fail;    
}

void sdcard_ota_app(void *pvParameters)
{
    PRINTF("SD Card updating task enable.\r\n");
    
    s_CardDetectSemaphore = xSemaphoreCreateBinary();
    
    BOARD_SD_Config(&g_sd, SDCARD_DetectCallBack, 5, NULL);
    
    if(SD_HostInit(&g_sd) != kStatus_Success)
    {
        PRINTF("SD host init fail\r\n");
        vTaskDelete(NULL);
    }
    
    SD_SetCardPower(&g_sd, false);

    PRINTF("Please insert a sd card into board.\r\n");
    
    uint8_t last_update_type;
    
    sfw_flash_read(UPDATE_TYPE_FLAG_ADDRESS, &last_update_type, 1);
    
    if(last_update_type == UPDATE_TYPE_SDCARD)
    {
        PRINTF("Update done, the last update type: %s\r\n",
               last_update_type == UPDATE_TYPE_SDCARD ? "SD Card" :
               last_update_type == UPDATE_TYPE_UDISK ? "U-Disk" :
               last_update_type == UPDATE_TYPE_AWS_CLOUD ? 
                       "AWS platform" : "BUG; can't happend");
        write_image_ok();        
    }
    
    while(true)
    {
        if (xSemaphoreTake(s_CardDetectSemaphore, portMAX_DELAY) == pdTRUE) // 0
        {
            if (s_cardInserted != s_cardInsertStatus)
            {
                s_cardInserted = s_cardInsertStatus;
     
                if (s_cardInserted)
                {
                    PRINTF("Card inserted.\r\n");
                    /* power off card */
                    SD_SetCardPower(&g_sd, false);
                    /* power on the card */
                    SD_SetCardPower(&g_sd, true);
                    vTaskDelay(500U);
                    
                    //start your sdcard application here
                    if(DEMO_SDCardOTAUpgrade() != kStatus_Success)
                    {
                        PRINTF("sd card ota task running failed.\r\n");
                    }   
                }
            }
        }
        vTaskDelay(1000U);
    }
}

/*!
 * @brief Main function
 */
int sdcard_ota_main(void)
{
    BOARD_SD_InitPins();

    if (pdPASS != xTaskCreate(sdcard_ota_app, "sdcard_ota_app", 1024U, NULL,
                              configMAX_PRIORITIES - 4, NULL))
    {
        return -1;
    }
    
    return 0;
}