/**
 * \file
 *
 * \brief  This module provides required interface to access IO protection key.
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
#include "memory_conf.h"
#include "atca_iface.h"
#include "hal/atca_hal.h"
#include "test/atca_test.h"



/** \brief This function helps to read IO protection value from specified address.
 *	\param[in, out] uint8_t* io_key
 *  \return ATCA_STATUS
 */
ATCA_STATUS io_protection_get_key(uint8_t* io_key)
{
	ATCA_STATUS status = ATCA_SUCCESS;

	enum status_code nvm_status;
	nvm_status = nvm_read_buffer(IO_PROTECTION_PAGE_ADDRESS, io_key, ATCA_KEY_SIZE);
	
	if(nvm_status != STATUS_OK)
	{
		status = ATCA_GEN_FAIL;
	}

	return status;
}

/** \brief This function helps to update IO Protection value to the specified address.
 *	\param[in, out] uint8_t* io_key
 *  \return ATCA_STATUS
*/
ATCA_STATUS io_protection_set_key(uint8_t* io_key)
{
	ATCA_STATUS status = ATCA_SUCCESS;

	enum status_code nvm_status;

	/*First ATCA_KEY_SIZE bytes contains Key and rest is loaded from ram/stack */
	nvm_status = nvm_write_buffer(IO_PROTECTION_PAGE_ADDRESS, io_key, NVMCTRL_PAGE_SIZE);
	if(nvm_status != STATUS_OK)
	{
		status = ATCA_GEN_FAIL;
	}

	return status;
}