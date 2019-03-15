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

import os, sys
import binascii, base64, argparse
import cryptography
from cryptography.hazmat.primitives import serialization, hashes
from cryptography.hazmat.primitives.asymmetric import ec, utils


BLOCKSIZE = 65536
SIGNATURE_SIZE = 64
APPLICATION_END_ADDRESS = 0x6000
SIGANATURE_ADDRESS = APPLICATION_END_ADDRESS - SIGNATURE_SIZE

# Setup cryptography
crypto_be = cryptography.hazmat.backends.default_backend()


def digest_sign(key_file,bin_file):
	with open(key_file, 'rb') as f:
	    # Loading the private key from key_file
		private_key = serialization.load_pem_private_key(
                data=f.read(),
                password=None,
                backend=crypto_be)

	# Hashing the Application binary file bin_file
	chosen_hash = hashes.SHA256()
	hasher = hashes.Hash(chosen_hash, crypto_be)
	signing_file = open(bin_file, "rb+")
	signing_file.truncate(SIGANATURE_ADDRESS)
	buf = signing_file.read(BLOCKSIZE)
	while len(buf) > 0:
		hasher.update(buf)
		buf = signing_file.read(BLOCKSIZE)
	digest = hasher.finalize()

	# Signing the digest of the Application binary file bin_file
	sign = private_key.sign(
			digest,
			ec.ECDSA(utils.Prehashed(chosen_hash))
			)

	# Append the signature at the end of he binary file
	signing_file.truncate(SIGANATURE_ADDRESS)
	sign = bytearray(sign)
	r_offset = (sign[3]-32)+4
	signing_file.write(sign[r_offset:r_offset+32])

	s_offset = (sign[r_offset+32+1]-32)+(r_offset+32+2)
	signing_file.write(sign[s_offset:s_offset+32])

	for i in range(64):
		signing_file.write(b'\xFF')

	signing_file.close()


if __name__ == "__main__":
	parser = argparse.ArgumentParser(
		description="Signs the User application with input key; if key is not passed, \
generates a key pair and uses it for Sign operation",
		formatter_class=argparse.RawDescriptionHelpFormatter)
	parser.add_argument('-k', '--key', help='Key to Sign the application')
	parser.add_argument("bin", help='User application file to Sign')
	args = parser.parse_args()

	key_file = args.key
	bin_file = args.bin

	if not key_file:
		print ('key file is missing... New key file (generated_key.pem) will be generated and use for sign')
		key_file = "generated_key.pem"
		priv_key = ec.generate_private_key(ec.SECP256R1(), crypto_be)
        # Save private key to file
		with open(key_file, 'wb') as f:
			pem_key = priv_key.private_bytes(
				encoding=serialization.Encoding.PEM,
                format=serialization.PrivateFormat.PKCS8,
                encryption_algorithm=serialization.NoEncryption())
			f.write(pem_key)
			f.close()

	if not bin_file:
		print ('Application binary file is missing... Exiting now')
		sys.exit(2)
	digest_sign(key_file, bin_file)

