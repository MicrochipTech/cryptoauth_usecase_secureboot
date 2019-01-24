/**
 * \file
 *
 * \brief SSD1306 display controller driver configuration file.
 *
 * Copyright (c) 2014-2015 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */
#ifndef CONF_SSD1306_H_INCLUDED
#define CONF_SSD1306_H_INCLUDED

#include <board.h>

// Interface configuration for SAM D21 Xplained Pro
#  define SSD1306_SPI                 EXT3_SPI_MODULE
#  define CONFIG_SSD1306_FRAMEBUFFER

#  define SSD1306_DC_PIN              EXT3_PIN_5
#  define SSD1306_RES_PIN             EXT3_PIN_10
#  define SSD1306_CS_PIN              EXT3_PIN_15

#  define SSD1306_SPI_PINMUX_SETTING  EXT3_SPI_SERCOM_MUX_SETTING
#  define SSD1306_SPI_PINMUX_PAD0     EXT3_SPI_SERCOM_PINMUX_PAD0
#  define SSD1306_SPI_PINMUX_PAD1     PINMUX_UNUSED
#  define SSD1306_SPI_PINMUX_PAD2     EXT3_SPI_SERCOM_PINMUX_PAD2
#  define SSD1306_SPI_PINMUX_PAD3     EXT3_SPI_SERCOM_PINMUX_PAD3

// Minimum clock period is 50ns@3.3V -> max frequency is 20MHz
#define SSD1306_CLOCK_SPEED           1000000UL
#define SSD1306_DISPLAY_CONTRAST_MAX  40
#define SSD1306_DISPLAY_CONTRAST_MIN  30

#endif /* CONF_SSD1306_H_INCLUDED */
