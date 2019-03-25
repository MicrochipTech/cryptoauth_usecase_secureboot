# \page License
# © 2019 Microchip Technology Inc. and its subsidiaries.
# Subject to your compliance with these terms, you may use Microchip software and
# any derivatives exclusively with Microchip products. It is your responsibility to
# comply with third party license terms applicable to your use of third party software
# (including open source software) that may accompany Microchip software.
#
# THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER EXPRESS, IMPLIED
# OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT,
# MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE LIABLE
# FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE
# OF ANY KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN
# ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE FULLEST EXTENT ALLOWED BY LAW,
# MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED
# THE AMOUNT OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

from cryptoauthlib import *
from cryptoauthlib.iface import *
import binascii, argparse, time
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives import serialization, hashes
from cryptography.hazmat.primitives.asymmetric import ec, utils

ATCA_SUCCESS = 0x00
LOCK_ZONE_NO_CRC = 0x80
ATCA_ZONE_CONFIG = 0x00
LOCK_ZONE_CONFIG = 0x00
ATCA_ZONE_DATA = 0x02
LOCK_ZONE_DATA = 0x01
SECUREBOOTCONFIG_OFFSET = 70

ECC608A_SBOOT_CONFIG = bytearray([
    0x01,0x23,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x00,
    0x5A,0x00,0x00,0x01,0x85,0x00,0x82,0x00, 0x85,0x20,0x85,0x20,0x85,0x20,0x8F,0x46,
    0x8F,0x0F,0x8F,0x0F,0x0F,0x0F,0x9F,0x8F, 0x0F,0x8F,0x0F,0x8F,0x0F,0x8F,0x0F,0x0F,
    0x0D,0x1F,0x0F,0x0F,0xFF,0xFF,0xFF,0xFF, 0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,
    0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xF9, 0x00,0x69,0x76,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0xFF,0xFF,0x0E,0x60,0x00,0x00,0x00,0x00,
    0x53,0x00,0x53,0x00,0x73,0x00,0x73,0x00, 0x73,0x00,0x38,0x00,0x7C,0x00,0x1A,0x00,
    0x3C,0x00,0x1C,0x00,0x1C,0x00,0x10,0x00, 0x1C,0x00,0x30,0x00,0x12,0x00,0x30,0x00,
])

def get_sboot_mode_id(mode):
    """
    Returns the SecureBoot Mode value based on the Mode
    """
    sboot_modes = { 'DISABLED': 0,
                    'FULLBOTH': 1,
                    'FULLSIG' : 2,
                    'FULLDIG' : 3}
    return sboot_modes.get(mode.upper())

def get_device_name(revision):
    """
    Returns the device name based on the info byte array values returned by atcab_info
    """
    devices = {0x10: 'ATECC108A',
               0x50: 'ATECC508A',
               0x60: 'ATECC608A',
               0x00: 'ATSHA204A',
               0x02: 'ATSHA204A'}
    return devices.get(revision[2], 'UNKNOWN')

def get_device_type_id(name):
    """
    Returns the ATCADeviceType value based on the device name
    """
    devices = {'ATSHA204A': 0,
               'ATECC108A': 1,
               'ATECC508A': 2,
               'ATECC608A': 3,
               'UNKNOWN': 0x20 }
    return devices.get(name.upper())

def init_device(iface='hid'):
    # Loading cryptoauthlib(python specific)
    load_cryptoauthlib()

    # Get a default config
    if iface is 'i2c':
        cfg = cfg_ateccx08a_i2c_default()
    else:
        cfg = cfg_ateccx08a_kithid_default()

    # Initialize the stack
    assert atcab_init(cfg) == ATCA_SUCCESS

    # Check device type
    info = bytearray(4)
    assert atcab_info(info) == ATCA_SUCCESS
    dev_type = get_device_type_id(get_device_name(info))

    if dev_type in [0, 0x60]:
        raise ValueError('Device does not support SecureBoot operations')
    elif dev_type != cfg.devtype:
        assert atcab_release() == ATCA_SUCCESS
        time.sleep(1)
        assert atcab_init(cfg) == ATCA_SUCCESS

def pretty_print_hex(a, l=16):
    s = ''
    a = bytearray(a)
    for x in range(0, len(a), l):
        s += ''.join(['%02X ' % y for y in a[x:x+l]]) + '\n'
    return s

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description='Provisions the crypto device with input key and secureboot mode selected. If key or mode is not passed, then \
considers key.pem and FullDig as inputs',
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('-k', '--key', default='key.pem', help='Key used to Sign the application ')
    parser.add_argument('-m', '--mode', choices=['Disabled', 'FullBoth', 'FullSig', 'FullDig'], default='FullDig', help='SecureBoot Mode to be used for validation')
    args = parser.parse_args()

    #initialize device interface on hid
    init_device('hid')

    # Get Secureboot mode value from input; update the selected mode in the configuration array
    sboot_mode = get_sboot_mode_id(args.mode)
    ECC608A_SBOOT_CONFIG[SECUREBOOTCONFIG_OFFSET] |= sboot_mode

    with open(args.key, 'rb') as f:
        # Load the public key from key file
        priv_key = serialization.load_pem_private_key(
                data=f.read(),
                password=None,
                backend=default_backend())
        public_key = priv_key.public_key().public_numbers().encode_point()[1:]

    public_key_slot_data = bytearray(4) + public_key[0:32] + bytearray(4) + public_key[32:64]

    # load configuration to crypto device
    #get config zone lock status
    is_locked = AtcaReference(False)
    assert atcab_is_locked(LOCK_ZONE_CONFIG, is_locked) == ATCA_SUCCESS
    if 0 == bool(is_locked.value):
        #config zone is unlocked... Write data and lock data
        assert atcab_write_config_zone(ECC608A_SBOOT_CONFIG) == ATCA_SUCCESS
        assert atcab_lock(LOCK_ZONE_NO_CRC | LOCK_ZONE_CONFIG, 0) == ATCA_SUCCESS
        print("Crypto Device Configuration Zone is locked!!!")
    else:
        print("Crypto Device Configuration Zone is already locked!!!")

    #check data zone lock status
    assert atcab_is_locked(LOCK_ZONE_DATA, is_locked) == ATCA_SUCCESS
    if 0 == bool(is_locked.value):
        #data zone is unlocked... lock it
        assert atcab_lock(LOCK_ZONE_NO_CRC | LOCK_ZONE_DATA, 0) == ATCA_SUCCESS
        print("Crypto Device Data Zone is locked!!!")
    else:
        print("Crypto Device Data Zone is already locked!!!")

    #write secure boot public key to crypto device
    secureboot_config_mode = bytearray(2)
    assert atcab_read_bytes_zone(ATCA_ZONE_CONFIG, 0, SECUREBOOTCONFIG_OFFSET, secureboot_config_mode, 2) == ATCA_SUCCESS
    public_key_slot = secureboot_config_mode[1] >> 4
    assert atcab_is_slot_locked(public_key_slot, is_locked) == ATCA_SUCCESS
    if 0 == bool(is_locked.value):
        #write secure boot public key to crypto device
        assert atcab_write_bytes_zone(ATCA_ZONE_DATA, public_key_slot, 0, public_key_slot_data, 72) == ATCA_SUCCESS

        #lock secure boot public key to avoid further updates
        assert atcab_lock_data_slot(public_key_slot) == ATCA_SUCCESS
        print("Crypto Device Public Key Slot is locked!!!")
    else:
        print("Crypto Device Public Key Slot is already locked!!!")

    print("Secure boot public key is loaded to device and locked !!!\n", pretty_print_hex(public_key))

