/**
 * \file
 *
 * \brief  This module provides interface to memory component for the secure boot.
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
#include <asf.h>


#include "io_protection_key.h"
#include "secure_boot_memory.h"
#include "secure_boot.h"
#include "memory_conf.h"
#include "crypto_device_app.h"
#include "atca_iface.h"
#include "hal/atca_hal.h"
#include "test/atca_test.h"

#define USER_APPLICATION_START_PAGE			(APP_START_ADDRESS / NVMCTRL_PAGE_SIZE)
#define IO_PROTECTION_PAGE_ADDRESS			((USER_APPLICATION_START_PAGE - 1) * NVMCTRL_PAGE_SIZE)
#define USER_APPLICATION_START_ADDRESS		(USER_APPLICATION_START_PAGE * NVMCTRL_PAGE_SIZE)
#define USER_APPLICATION_END_ADDRESS		(USER_APPLICATION_START_ADDRESS + (24*1024))
#define USER_APPLICATION_HEADER_SIZE		(2 * NVMCTRL_PAGE_SIZE)
#define USER_APPLICATION_HEADER_ADDRESS		(USER_APPLICATION_END_ADDRESS - USER_APPLICATION_HEADER_SIZE)


uint32_t flash_read_address;

 /** \brief This module takes care of initializing memory access and updates its parameters
 *	\param[in, out] memory_parameters* memory_params pointer to hold memory parameters
 *  \return ATCA_STATUS
 */
ATCA_STATUS secure_boot_init_memory(memory_parameters* memory_params)
{
	ATCA_STATUS status = ATCA_SUCCESS;

	do
	{
		struct nvm_config config;
		enum status_code nvm_status;
		uint8_t io_prot_key[ATCA_KEY_SIZE];
		uint8_t io_prot_no_bond_value[ATCA_KEY_SIZE];
		uint32_t header_address;
		uint8_t* read_data;
		struct nvm_fusebits fuse_bits;

		/* Get the default configuration */		
		nvm_get_config_defaults(&config);

		/* Set wait state to 1 */
		config.wait_states = 1;

		/* Enable automatic page write mode */
		config.manual_page_write = false;

		/* Set the NVM configuration */
		nvm_status = nvm_set_config(&config);
		if(nvm_status != STATUS_OK)
		{
			status = ATCA_GEN_FAIL;
			break;
		}

		/*Get current IO protection Key on host */
		if((status = io_protection_get_key(io_prot_key)) != ATCA_SUCCESS)
		{
			break;
		}
		else
		{
			/*Host is expected to have all FFs when it is NOT bond with device*/
			memset(io_prot_no_bond_value, 0xFF, sizeof(io_prot_no_bond_value));
		
			/*Check current key is all FFs */
			if(!memcmp(io_prot_no_bond_value, io_prot_key, sizeof(io_prot_key)))
			{
				/*IO protection key is not on host... Get bind with device */

				/* First check Lock status on device, if it is locked it returns error
				Otherwise it generates io protection key in device and in host */
				if((status = bind_host_and_secure_element_with_io_protection(IO_PROTECTION_KEY_SLOT)) != ATCA_SUCCESS)
				{
					break;
				}

				

				/*Lock Bootloader using BOOTPROT... First read current value and then set BOOTPROT */
				nvm_status = nvm_get_fuses(&fuse_bits);
				if(nvm_status != STATUS_OK)
				{
					status = ATCA_GEN_FAIL;
					break;
				}

				fuse_bits.bootloader_size = NVM_BOOTLOADER_SIZE_128;

				nvm_status = nvm_set_fuses(&fuse_bits);
				if(nvm_status != STATUS_OK)
				{
					status = ATCA_GEN_FAIL;
					break;
				}

				/* Set Security bit to disable further external access */
				nvm_status = nvm_execute_command(NVM_COMMAND_SET_SECURITY_BIT,NVMCTRL_AUX0_ADDRESS,0);
				if (nvm_status != STATUS_OK) {
					status = ATCA_GEN_FAIL;
					break;
				}
			}
		}

		/*Clear on stack */
		memset(io_prot_key, 0xFF, sizeof(io_prot_key));

	
		/*Read memory params from the flash*/
		read_data = (uint8_t*)memory_params;
		for(header_address=USER_APPLICATION_HEADER_ADDRESS; header_address<(USER_APPLICATION_HEADER_ADDRESS+USER_APPLICATION_HEADER_SIZE);)
		{
			nvm_status = nvm_read_buffer(header_address, read_data, NVMCTRL_PAGE_SIZE);
			if(nvm_status != STATUS_OK)
			{
				break;
			}
			read_data += NVMCTRL_PAGE_SIZE;
			header_address += NVMCTRL_PAGE_SIZE;
		}

		memory_params->memory_size -= sizeof(memory_params->signature);

		if((nvm_status != STATUS_OK) || (memory_params->start_address != USER_APPLICATION_START_ADDRESS) ||
		(memory_params->memory_size > (USER_APPLICATION_END_ADDRESS - USER_APPLICATION_START_ADDRESS)))
		{
			status = ATCA_GEN_FAIL;
			flash_read_address = 0xFFFFFFFF;
			break;
		}
		else
		{
			flash_read_address = USER_APPLICATION_START_ADDRESS;
		}			

	} while (0);


	return status;
}

/** \brief This module provides interface to read data from memory
*	\param[in, out] uint8_t* data Pointer to hold memory content
*	\param[in] uint8_t* target_length Data bytes length to read from memory
*  \return ATCA_STATUS
*/
ATCA_STATUS secure_boot_read_memory(uint8_t* data, uint32_t* target_length)
{
	ATCA_STATUS status = ATCA_SUCCESS;

	

		uint32_t read_length;
		enum status_code nvm_status;

		/* Set the NVM configuration */
		nvm_status = nvm_read_buffer(flash_read_address, data, *target_length);

		if(nvm_status != STATUS_OK)
		{
			read_length = 0;
			status = ATCA_GEN_FAIL;
		}
		else 
		{
			read_length = *target_length;
		}
		flash_read_address += read_length;

	return status;
}

/** \brief This module provides interface to write data to memory
*	\param[in, out] uint8_t* data Pointer to hold memory content
*	\param[in] uint8_t* target_length Data bytes length to write to memory
*  \return ATCA_STATUS
*/
ATCA_STATUS secure_boot_write_memory(uint8_t* data, uint32_t* target_length)
{
	/*No use case to write memory at this time*/
	return ATCA_UNIMPLEMENTED;
}

/** \brief This module marks flash to indicate Secure Boot verification is completed on Upgrade
*  \return ATCA_STATUS
*/
ATCA_STATUS secure_boot_mark_full_copy_completion(void)
{
	ATCA_STATUS status = ATCA_SUCCESS;
	enum status_code nvm_status;
	uint8_t* const update_notify = "UPDT";

	/*First ATCA_KEY_SIZE bytes contains Key and rest is loaded from ram/stack */
	nvm_status = nvm_update_buffer(USER_APPLICATION_END_ADDRESS, update_notify, 0, sizeof(uint32_t));
	if(nvm_status != STATUS_OK)
	{
		status = ATCA_GEN_FAIL;
	}

	return status;
}

/** \brief This module marks flash to indicate Secure Boot verification is completed on Upgrade
*  \return ATCA_STATUS
*/
bool secure_boot_check_full_copy_completion(void)
{
	bool is_completed;
	uint8_t* const update_notify = "UPDT";
	uint32_t flash_mark;
	enum status_code nvm_status;

	is_completed = false;
	nvm_status = nvm_read_buffer(USER_APPLICATION_END_ADDRESS, (uint8_t* const)&flash_mark, sizeof(flash_mark));
	if((nvm_status == STATUS_OK) && (0 == memcmp(&flash_mark, update_notify, sizeof(flash_mark))))
	{
		is_completed = true;
	}

	return is_completed;
}


/** \brief This module takes care of de-initializing memory access and resets its parameters
*	\param[in, out] memory_parameters* memory_params pointer to hold memory parameters
*  \return ATCA_STATUS
*/
void secure_boot_deinit_memory(memory_parameters* memory_params)
{

}



ATCA_STATUS host_generate_random_number(uint8_t *rand_num)
{
	uint32_t random_num=0;
	
	random_num=rand();
	memcpy(rand_num,&random_num,4);
	
	rand_num+=4;
	random_num=rand();
	memcpy(rand_num,&random_num,4);
	
	rand_num+=4;
	random_num=rand();
	memcpy(rand_num,&random_num,4);
	
	rand_num+=4;
	random_num=rand();
	memcpy(rand_num,&random_num,4);
	
	rand_num+=4;
	random_num=rand();
	memcpy(rand_num,&random_num,4);
	
	return ATCA_SUCCESS;
	
}



