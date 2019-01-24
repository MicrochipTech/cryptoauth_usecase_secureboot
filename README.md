# SecureBoot

## Introduction
The Microchip [ATECC608A](https://www.microchip.com/wwwproducts/en/atecc608a) device is a member of the CryptoAuthentication™ family of high-security cryptographic devices, which combine world-class hardware-based key storage with hardware cryptographic accelerators in order to implement various authentication and encryption protocols. ATECC608A provides a mechanism to support secure boot operations in a connected microcontroller unit (MCU) that can help identify situations in which fraudulent code has been installed on the host central processing unit (CPU).

SecureBoot is a new feature on this device compared to earlier Crypto devices from Microchip. This feature helps MCU to identify fraudulent code installed on it. When this feature is implemented, MCU can send either digest/signature or both to ECC608A. Then, ECC608A validates this information and responds to host either with yes/no or with MAC value.

The device provides options to store signature or digest to reduce execution time, in this case digest will be simply compared with preloaded value or signature will be verified using Public Key. It also supports wire protection to protect system when connections between MCU and device is replaced with fraudulent signals.

This use case is built upon [AT88CKSCKTSOIC-XPRO](https://www.microchip.com/developmenttools/ProductDetails/at88ckscktsoic-xpro) and [ATSAMD21-XPRO](http://www.microchip.com/Developmenttools/ProductDetails/ATSAMD21-XPRO) board using [CryptoAuthLib](https://github.com/MicrochipTech/cryptoauthlib) and [ASF](http://www.microchip.com/mplab/avr-support/advanced-software-framework-(asf)). Two ASF examples are used as base projects,
- [SAM-BA Monitor](http://asf.atmel.com/docs/latest/sam0.applications.samba_bootloader.samd21_xplained_pro/html/index.html) as bootloader
- [FreeRTOS + OLED example](http://asf.atmel.com/docs/latest/common.services.freertos.oled1_xpro_example.samd21_xplained_pro/html/index.html) as application

## Updates to bootloader
The SAMD21 acts as host MCU and ATECC608A as CryptoAuthentication device. ASF SAM-BA Monitor application is updated to include CryptoAuthLib and SecureBoot functionality.

- Invoke crypto_device_verify_app....Return value ATCA_SUCCESS indicates application is valid, otherwise application is invalid.

## Hardware Requirements
Following are the hardware modules required to do hands on of this this usecase example

- AT88CKSCKTSOIC-XPRO (Or) [AT88CKSCKTUDFN-XPRO](https://www.microchip.com/developmenttools/ProductDetails/at88ckscktudfn-xpro)
- ATSAMD21-XPRO
- [ATOLED1-XPRO](https://www.microchip.com/developmenttools/ProductDetails/ATOLED1-XPRO)
- [CryptoAuth-XSTK](https://www.microchip.com/DevelopmentTools/ProductDetails/PartNo/DM320109)

[Buy Secure Boot hardware](http://www.microchipdirect.com/product/promo/SecureBootATECC608A)

## Additional Weblinks
For detailed documentation refer to [repository wiki](https://github.com/KalyanCManukonda/ECC608A-SecureBoot-Example/wiki).
