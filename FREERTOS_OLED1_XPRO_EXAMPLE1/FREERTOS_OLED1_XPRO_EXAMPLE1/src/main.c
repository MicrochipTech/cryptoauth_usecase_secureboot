/**
 * \file
 *
 * \brief FreeRTOS demo application main function.
 *
 * \copyright (c) 2015-2018 Microchip Technology Inc. and its subsidiaries.
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

#include <asf.h>
#include "demotasks.h"

#define APP_START_ADDRESS					0x00008000
#define USER_APPLICATION_START_PAGE			(APP_START_ADDRESS / NVMCTRL_PAGE_SIZE)
#define IO_PROTECTION_PAGE_ADDRESS			((USER_APPLICATION_START_PAGE - 1) * NVMCTRL_PAGE_SIZE)
#define USER_APPLICATION_START_ADDRESS		(USER_APPLICATION_START_PAGE * NVMCTRL_PAGE_SIZE)
#define USER_APPLICATION_END_ADDRESS		(USER_APPLICATION_START_ADDRESS + (24*1024))
#define USER_APPLICATION_HEADER_SIZE		(2 * NVMCTRL_PAGE_SIZE)
#define USER_APPLICATION_HEADER_ADDRESS		(USER_APPLICATION_END_ADDRESS - USER_APPLICATION_HEADER_SIZE)

typedef struct
{
	uint32_t start_address;
	uint32_t memory_size;
	uint32_t version_info;
	uint8_t reserved[52];				//Reserving 20-bytes for Application information
}memory_parameters;

/*Blocking last USER_APPLICATION_HEADER_SIZE bytes for Signature and memory/application specific information*/
__attribute__ ((section(".footer_data")))
const memory_parameters user_application_footer = 
{
	USER_APPLICATION_START_ADDRESS,
	(USER_APPLICATION_END_ADDRESS - USER_APPLICATION_START_ADDRESS),
	0x00010001,
	{0},
};

int main (void)
{
	system_init();
	gfx_mono_init();

	// Initialize the demo..
	demotasks_init();

	// ..and let FreeRTOS run tasks!
	vTaskStartScheduler();

	do {
		// Intentionally left empty
	} while (true);
}
