/**
 * \file
 *
 * \brief Provides required interface between boot loader and secure boot.
 *
 * \copyright (c) 2015-2019 Microchip Technology Inc. and its subsidiaries.
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip software
 * and any derivatives exclusively with Microchip products. It is your
 * responsibility to comply with third party license terms applicable to your
 * use of third party software (including open source software) that may
 * accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
 * PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT,
 * SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE
 * OF ANY KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF
 * MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE
 * FORESEEABLE. TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL
 * LIABILITY ON ALL CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED
 * THE AMOUNT OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR
 * THIS SOFTWARE.
 */
#include <stdlib.h>
#include <stdio.h>
#include "cryptoauthlib.h"
#include "secure_boot.h"
#include "io_protection_key.h"
#include "crypto_device_app.h"

#define ATECC608A_MAH22_CONFIG_I2C_ADDR         (0x6A)
#define ATECC608A_SECURE_BOOT_DEMO_I2C_ADDR     (0x5A)
#define ATECC608A_DEFAULT_I2C_ADDR              (0xC0)

/** \brief Takes care interface with secure boot and provides status about user
 *         application. This also takes care of device configuration if enabled.
 *  \return ATCA_SUCCESS on success, otherwise an error code.
 *  \Note This api supports only MAH22 I2C address or default I2C address. Otherwise
 *          this api returns with error code
 */
ATCA_STATUS crypto_device_verify_app(void)
{
    ATCA_STATUS status = ATCA_SUCCESS;
    /*Creating interface instance for 608A*/
    ATCAIfaceCfg cfg_atecc608a_i2c_default = {
        .iface_type             = ATCA_I2C_IFACE,
        .devtype                = ATECC608A,
        .atcai2c.slave_address  = ATECC608A_SECURE_BOOT_DEMO_I2C_ADDR,
        .atcai2c.bus            = 2,
        .atcai2c.baud           = 400000,
        //.atcai2c.baud = 100000,
        .wake_delay             = 1500,
        .rx_retries             = 20
    };
    uint8_t addr_list[] = {ATECC608A_SECURE_BOOT_DEMO_I2C_ADDR, ATECC608A_MAH22_CONFIG_I2C_ADDR, ATECC608A_DEFAULT_I2C_ADDR};
	uint8_t sboot_public_key_slot;
	
    do
    {
        #if CRYPTO_DEVICE_ENABLE_SECURE_BOOT
        bool is_locked;

        for(uint8_t addr_index=0; addr_index<(sizeof(addr_list)/sizeof(addr_list[0])); addr_index++)
        {
            cfg_atecc608a_i2c_default.atcai2c.slave_address = addr_list[addr_index];
            if ((status = atcab_init(&cfg_atecc608a_i2c_default)) == ATCA_SUCCESS)
            {
                /*ECC608A with addr_list[addr_index] address is found */
                break;
            }                
        }

        /* No ECC608A with matching address found */
        if(status != ATCA_SUCCESS)
            break;

        /*Check current status of Public Key Slot lock status */
		if((status = atcab_read_bytes_zone(ATCA_ZONE_CONFIG, 0, SECUREBOOTCONFIG_OFFSET+1, &sboot_public_key_slot, sizeof(sboot_public_key_slot))) != ATCA_SUCCESS)
		{
			break;
		}
		sboot_public_key_slot >>= 4;
        if ((status = atcab_is_slot_locked(sboot_public_key_slot, &is_locked)) != ATCA_SUCCESS)
        {
            break;
        }

        /*Before doing secure boot it is expected configuration zone is locked */
        if (!is_locked)
        {
            /*Trigger crypto device configuration */
            #if CRYPTO_DEVICE_LOAD_CONFIG_ENABLED
            if ((status = crypto_device_load_configuration(&cfg_atecc608a_i2c_default)) != ATCA_SUCCESS)
            {
                break;
            }
            #else
            status = ATCA_GEN_FAIL;
            break;
            #endif
        }

        /*Initiate secure boot operation */
        if ((status = secure_boot_process()) != ATCA_SUCCESS)
        {
            break;
        }
        #endif  //CRYPTO_DEVICE_ENABLE_SECURE_BOOT

    }
    while (0);


    return status;
}

#if CRYPTO_DEVICE_LOAD_CONFIG_ENABLED
/** \brief Checks whether configuration is locked or not. if not, it writes
 *         default configuration to device and locks it.
 *  \return ATCA_SUCCESS on success, otherwise an error code.
 */
ATCA_STATUS crypto_device_load_configuration(ATCAIfaceCfg* cfg)
{
    ATCA_STATUS status;
    bool is_locked = false;
	uint8_t sboot_public_key_slot;
    uint8_t test_ecc608_configdata[ATCA_ECC_CONFIG_SIZE] = {
                                0x01, 0x23, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, //15
     ATECC608A_MAH22_CONFIG_I2C_ADDR, 0x00, 0x00, 0x01, 0x85, 0x00, 0x82, 0x00,  0x85, 0x20, 0x85, 0x20, 0x85, 0x20, 0x8F, 0x46, //31, 5
                                0x8F, 0x0F, 0x9F, 0x8F, 0x0F, 0x0F, 0x8F, 0x0F,  0x0F, 0x8F, 0x0F, 0x8F, 0x0F, 0x8F, 0x0F, 0x0F, //47
                                0x0D, 0x1F, 0x0F, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF,  0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, //63
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xF7,  0x00, 0x69, 0x76, 0x00, 0x00, 0x00, 0x00, 0x00, //79
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  0xFF, 0xFF, 0x0E, 0x60, 0x00, 0x00, 0x00, 0x00, //95
                                0x53, 0x00, 0x53, 0x00, 0x73, 0x00, 0x73, 0x00,  0x73, 0x00, 0x38, 0x00, 0x7C, 0x00, 0x1C, 0x00, //111
                                0x3C, 0x00, 0x1A, 0x00, 0x1C, 0x00, 0x10, 0x00,  0x1C, 0x00, 0x30, 0x00, 0x12, 0x00, 0x30, 0x00, //127
    };

    uint8_t public_key_slot_data[72];
    uint8_t public_key_read[ATCA_PUB_KEY_SIZE];
    uint8_t public_key[] = {
        0x21, 0x67, 0x64, 0x1c, 0x9f, 0xc4, 0x13, 0x6c, 0xb4, 0xa9, 0x1a, 0x4f, 0x56, 0xd4, 0x8b, 0x83,
        0x76, 0x9e, 0x3a, 0xd8, 0x1e, 0x0e, 0x01, 0xb7, 0x59, 0xc7, 0xc7, 0x94, 0x74, 0x3f, 0x1a, 0xa6,
        0x30, 0xcc, 0xb7, 0xec, 0xfc, 0xa8, 0x2e, 0xf0, 0x5b, 0xa1, 0x3d, 0x5b, 0x34, 0x53, 0x11, 0x18,
        0xa0, 0x67, 0x73, 0x7b, 0xdb, 0x1e, 0x3d, 0x1b, 0xbc, 0xdd, 0x10, 0x5a, 0x39, 0x23, 0x25, 0x3e
    };

    do
    {
        /*Check current status of configuration lock status */
        if ((status = atcab_is_locked(LOCK_ZONE_CONFIG, &is_locked)) != ATCA_SUCCESS)
        {
            break;
        }

        /*Write configuration if it is not already locked */
        if (!is_locked)
        {
            /*Trigger Configuration write... ignore first 16 bytes*/
            if ((status = atcab_write_bytes_zone(ATCA_ZONE_CONFIG, 0, 16, &test_ecc608_configdata[16], (sizeof(test_ecc608_configdata) - 16))) != ATCA_SUCCESS)
            {
                break;
            }

            /*Lock Configuration Zone on completing configuration*/
            if ((status = atcab_lock(LOCK_ZONE_NO_CRC | LOCK_ZONE_CONFIG, 0)) != ATCA_SUCCESS)
            {
                break;
            }

            /*Check for addr change during Configuration load */
            if(cfg->atcai2c.slave_address != test_ecc608_configdata[16])
            {
                /* Addr changed during configuration load ... bring new address into effect */
                atcab_wakeup();
                atcab_sleep();

                /* Reinit to use new address */
                cfg->atcai2c.slave_address = test_ecc608_configdata[16];
                if ((status = atcab_init(cfg)) != ATCA_SUCCESS)
                {
                    break;
                }
            }
        }

        /*Check current status of Public Key Slot lock status */
		if((status = atcab_read_bytes_zone(ATCA_ZONE_CONFIG, 0, SECUREBOOTCONFIG_OFFSET+1, &sboot_public_key_slot, sizeof(sboot_public_key_slot))) != ATCA_SUCCESS)
		{
			break;
		}
		sboot_public_key_slot >>= 4;
		
		if ((status = atcab_is_slot_locked(sboot_public_key_slot, &is_locked)) != ATCA_SUCCESS)
        {
            break;
        }

        /*Write Slot Data, if it is not already locked */
        if (!is_locked)
        {
            /*Check current status of Data zone lock status */
            if ((status = atcab_is_locked(LOCK_ZONE_DATA, &is_locked)) != ATCA_SUCCESS)
            {
                break;
            }

            if (!is_locked)
            {
                /*Lock Data Zone if it is not */
                if ((status = atcab_lock(LOCK_ZONE_NO_CRC | LOCK_ZONE_DATA, 0)) != ATCA_SUCCESS)
                {
                    break;
                }
            }

            /*Write Pub Key to Slot... Reformat public key into padded format */
            memmove(&public_key_slot_data[40], &public_key[32], 32);    // Move Y to padded position
            memset(&public_key_slot_data[36], 0, 4);                    // Add Y padding bytes
            memmove(&public_key_slot_data[4], &public_key[0], 32);      // Move X to padded position
            memset(&public_key_slot_data[0], 0, 4);                     // Add X padding bytes

            /*Write Public Key to SecureBootPubKey slot*/
            if ((status = atcab_write_bytes_zone(ATCA_ZONE_DATA, sboot_public_key_slot, 0, public_key_slot_data, 72)) != ATCA_SUCCESS)
            {
                break;
            }

            /*Read Public Key*/
            if ((status = atcab_read_pubkey(sboot_public_key_slot, public_key_read)) != ATCA_SUCCESS)
            {
                break;
            }

            if ((status = memcmp(public_key, public_key_read, sizeof(public_key_read))) != ATCA_SUCCESS)
            {
                break;
            }

            /*Lock Public key slot */
            if ((status = atcab_lock_data_slot(sboot_public_key_slot)) != ATCA_SUCCESS)
            {
                break;
            }
        }
    }
    while (0);

    return status;
}
#endif  //#if CRYPTO_DEVICE_CONFIG_ENABLED

