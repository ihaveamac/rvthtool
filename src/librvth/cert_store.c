/***************************************************************************
 * RVT-H Tool (librvth)                                                    *
 * cert_store.c: Certificate store.                                        *
 *                                                                         *
 * Copyright (c) 2018 by David Korth.                                      *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License       *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.   *
 ***************************************************************************/

// Reference: http://wiibrew.org/wiki/Certificate_chain
#include "cert_store.h"
#include "byteswap.h"

#include <assert.h>
#include <errno.h>
#include <string.h>

// Encryption keys. (AES-128)
const uint8_t RVL_AES_Keys[RVL_KEY_MAX][16] = {
	// RVL_KEY_RETAIL
	{0xEB,0xE4,0x2A,0x22,0x5E,0x85,0x93,0xE4,
	 0x48,0xD9,0xC5,0x45,0x73,0x81,0xAA,0xF7},

	// RVL_KEY_KOREAN
	{0x63,0xB8,0x2B,0xB4,0xF4,0x61,0x4E,0x2E,
	 0x13,0xF2,0xFE,0xFB,0xBA,0x4C,0x9B,0x7E},

	// RVL_KEY_DEBUG
	{0xA1,0x60,0x4A,0x6A,0x71,0x23,0xB5,0x29,
	 0xAE,0x8B,0xEC,0x32,0xC8,0x16,0xFC,0xAA},
};

// Signature issuers.
const char *const RVL_Cert_Issuers[RVL_CERT_ISSUER_MAX] = {
	NULL,				// RVL_CERT_ISSUER_UNKNOWN
	"Root",				// RVL_CERT_ISSUER_ROOT

	// Retail
	"Root-CA00000001",		// RVL_CERT_ISSUER_RETAIL_CA
	"Root-CA00000001-XS00000003",	// RVL_CERT_ISSUER_RETAIL_TICKET
	"Root-CA00000001-CP00000004",	// RVL_CERT_ISSUER_RETAIL_TMD

	// Debug
	"Root-CA00000002",		// RVL_CERT_ISSUER_DEBUG_CA
	"Root-CA00000002-XS00000006",	// RVL_CERT_ISSUER_DEBUG_TICKET
	"Root-CA00000002-CP00000007",	// RVL_CERT_ISSUER_DEBUG_TMD
	"Root-CA00000002-MS00000003",	// RVL_CERT_ISSUER_DEBUG_DEV
};

// Root certificate.
static const RVL_Cert_RSA4096_KeyOnly cert_root = {
	// NOTE: The root certificate is not signed, since it's
	// not included in the standard certificate chain.

	// Signature
	{0, { 0 }, ""},

	// Public key
	{
	 BE32_CONST(RVL_CERT_KEYTYPE_RSA4096),	// Key type
	 "Root",				// Child certificate identity
	 BE32_CONST(0),				// Unknown...
	 // Modulus
	 {
	  0xF8,0x24,0x6C,0x58,0xBA,0xE7,0x50,0x03,0x01,0xFB,0xB7,0xC2,0xEB,0xE0,0x01,0x05,
	  0x71,0xDA,0x92,0x23,0x78,0xF0,0x51,0x4E,0xC0,0x03,0x1D,0xD0,0xD2,0x1E,0xD3,0xD0,
	  0x7E,0xFC,0x85,0x20,0x69,0xB5,0xDE,0x9B,0xB9,0x51,0xA8,0xBC,0x90,0xA2,0x44,0x92,
	  0x6D,0x37,0x92,0x95,0xAE,0x94,0x36,0xAA,0xA6,0xA3,0x02,0x51,0x0C,0x7B,0x1D,0xED,
	  0xD5,0xFB,0x20,0x86,0x9D,0x7F,0x30,0x16,0xF6,0xBE,0x65,0xD3,0x83,0xA1,0x6D,0xB3,
	  0x32,0x1B,0x95,0x35,0x18,0x90,0xB1,0x70,0x02,0x93,0x7E,0xE1,0x93,0xF5,0x7E,0x99,
	  0xA2,0x47,0x4E,0x9D,0x38,0x24,0xC7,0xAE,0xE3,0x85,0x41,0xF5,0x67,0xE7,0x51,0x8C,
	  0x7A,0x0E,0x38,0xE7,0xEB,0xAF,0x41,0x19,0x1B,0xCF,0xF1,0x7B,0x42,0xA6,0xB4,0xED,
	  0xE6,0xCE,0x8D,0xE7,0x31,0x8F,0x7F,0x52,0x04,0xB3,0x99,0x0E,0x22,0x67,0x45,0xAF,
	  0xD4,0x85,0xB2,0x44,0x93,0x00,0x8B,0x08,0xC7,0xF6,0xB7,0xE5,0x6B,0x02,0xB3,0xE8,
	  0xFE,0x0C,0x9D,0x85,0x9C,0xB8,0xB6,0x82,0x23,0xB8,0xAB,0x27,0xEE,0x5F,0x65,0x38,
	  0x07,0x8B,0x2D,0xB9,0x1E,0x2A,0x15,0x3E,0x85,0x81,0x80,0x72,0xA2,0x3B,0x6D,0xD9,
	  0x32,0x81,0x05,0x4F,0x6F,0xB0,0xF6,0xF5,0xAD,0x28,0x3E,0xCA,0x0B,0x7A,0xF3,0x54,
	  0x55,0xE0,0x3D,0xA7,0xB6,0x83,0x26,0xF3,0xEC,0x83,0x4A,0xF3,0x14,0x04,0x8A,0xC6,
	  0xDF,0x20,0xD2,0x85,0x08,0x67,0x3C,0xAB,0x62,0xA2,0xC7,0xBC,0x13,0x1A,0x53,0x3E,
	  0x0B,0x66,0x80,0x6B,0x1C,0x30,0x66,0x4B,0x37,0x23,0x31,0xBD,0xC4,0xB0,0xCA,0xD8,
	  0xD1,0x1E,0xE7,0xBB,0xD9,0x28,0x55,0x48,0xAA,0xEC,0x1F,0x66,0xE8,0x21,0xB3,0xC8,
	  0xA0,0x47,0x69,0x00,0xC5,0xE6,0x88,0xE8,0x0C,0xCE,0x3C,0x61,0xD6,0x9C,0xBB,0xA1,
	  0x37,0xC6,0x60,0x4F,0x7A,0x72,0xDD,0x8C,0x7B,0x3E,0x3D,0x51,0x29,0x0D,0xAA,0x6A,
	  0x59,0x7B,0x08,0x1F,0x9D,0x36,0x33,0xA3,0x46,0x7A,0x35,0x61,0x09,0xAC,0xA7,0xDD,
	  0x7D,0x2E,0x2F,0xB2,0xC1,0xAE,0xB8,0xE2,0x0F,0x48,0x92,0xD8,0xB9,0xF8,0xB4,0x6F,
	  0x4E,0x3C,0x11,0xF4,0xF4,0x7D,0x8B,0x75,0x7D,0xFE,0xFE,0xA3,0x89,0x9C,0x33,0x59,
	  0x5C,0x5E,0xFD,0xEB,0xCB,0xAB,0xE8,0x41,0x3E,0x3A,0x9A,0x80,0x3C,0x69,0x35,0x6E,
	  0xB2,0xB2,0xAD,0x5C,0xC4,0xC8,0x58,0x45,0x5E,0xF5,0xF7,0xB3,0x06,0x44,0xB4,0x7C,
	  0x64,0x06,0x8C,0xDF,0x80,0x9F,0x76,0x02,0x5A,0x2D,0xB4,0x46,0xE0,0x3D,0x7C,0xF6,
	  0x2F,0x34,0xE7,0x02,0x45,0x7B,0x02,0xA4,0xCF,0x5D,0x9D,0xD5,0x3C,0xA5,0x3A,0x7C,
	  0xA6,0x29,0x78,0x8C,0x67,0xCA,0x08,0xBF,0xEC,0xCA,0x43,0xA9,0x57,0xAD,0x16,0xC9,
	  0x4E,0x1C,0xD8,0x75,0xCA,0x10,0x7D,0xCE,0x7E,0x01,0x18,0xF0,0xDF,0x6B,0xFE,0xE5,
	  0x1D,0xDB,0xD9,0x91,0xC2,0x6E,0x60,0xCD,0x48,0x58,0xAA,0x59,0x2C,0x82,0x00,0x75,
	  0xF2,0x9F,0x52,0x6C,0x91,0x7C,0x6F,0xE5,0x40,0x3E,0xA7,0xD4,0xA5,0x0C,0xEC,0x3B,
	  0x73,0x84,0xDE,0x88,0x6E,0x82,0xD2,0xEB,0x4D,0x4E,0x42,0xB5,0xF2,0xB1,0x49,0xA8,
	  0x1E,0xA7,0xCE,0x71,0x44,0xDC,0x29,0x94,0xCF,0xC4,0x4E,0x1F,0x91,0xCB,0xD4,0x95},
	 // Exponent
	 BE32_CONST(0x00010001),
	 // Padding
	 { 0 }
	}
};

/** Retail certificates. **/

// CA certificate. (retail)
static const RVL_Cert_RSA4096_RSA2048 cert_retail_ca = {
	// Signature
	{
	 BE32_CONST(RVL_CERT_SIGTYPE_RSA4096),	// Signature type
	 // Signature
	 {0xB3,0xAD,0xB3,0x22,0x6B,0x3C,0x3D,0xFF,0x1B,0x4B,0x40,0x77,0x16,0xFF,0x4F,0x7A,
	  0xD7,0x64,0x86,0xC8,0x95,0xAC,0x56,0x2D,0x21,0xF1,0x06,0x01,0xD4,0xF6,0x64,0x28,
	  0x19,0x1C,0x07,0x76,0x8F,0xDF,0x1A,0xE2,0xCE,0x7B,0x27,0xC9,0x0F,0xBC,0x0A,0xD0,
	  0x31,0x25,0x78,0xEC,0x07,0x79,0xB6,0x57,0xD4,0x37,0x24,0x13,0xA7,0xF8,0x6F,0x0C,
	  0x14,0xC0,0xEF,0x6E,0x09,0x41,0xED,0x2B,0x05,0xEC,0x39,0x57,0x36,0x07,0x89,0x00,
	  0x4A,0x87,0x8D,0x2E,0x9D,0xF8,0xC7,0xA5,0xA9,0xF8,0xCA,0xB3,0x11,0xB1,0x18,0x79,
	  0x57,0xBB,0xF8,0x98,0xE2,0xA2,0x54,0x02,0xCF,0x54,0x39,0xCF,0x2B,0xBF,0xA0,0xE1,
	  0xF8,0x5C,0x06,0x6E,0x83,0x9A,0xE0,0x94,0xCA,0x47,0xE0,0x15,0x58,0xF5,0x6E,0x6F,
	  0x34,0xE9,0x2A,0xA2,0xDC,0x38,0x93,0x7E,0x37,0xCD,0x8C,0x5C,0x4D,0xFD,0x2F,0x11,
	  0x4F,0xE8,0x68,0xC9,0xA8,0xD9,0xFE,0xD8,0x6E,0x0C,0x21,0x75,0xA2,0xBD,0x7E,0x89,
	  0xB9,0xC7,0xB5,0x13,0xF4,0x1A,0x79,0x61,0x44,0x39,0x10,0xEF,0xF9,0xD7,0xFE,0x57,
	  0x22,0x18,0xD5,0x6D,0xFB,0x7F,0x49,0x7A,0xA4,0xCB,0x90,0xD4,0xF1,0xAE,0xB1,0x76,
	  0xE4,0x68,0x5D,0xA7,0x94,0x40,0x60,0x98,0x2F,0x04,0x48,0x40,0x1F,0xCF,0xC6,0xBA,
	  0xEB,0xDA,0x16,0x30,0xB4,0x73,0xB4,0x15,0x23,0x35,0x08,0x07,0x0A,0x9F,0x4F,0x89,
	  0x78,0xE6,0x2C,0xEC,0x5E,0x92,0x46,0xA5,0xA8,0xBD,0xA0,0x85,0x78,0x68,0x75,0x0C,
	  0x3A,0x11,0x2F,0xAF,0x95,0xE8,0x38,0xC8,0x99,0x0E,0x87,0xB1,0x62,0xCD,0x10,0xDA,
	  0xB3,0x31,0x96,0x65,0xEF,0x88,0x9B,0x54,0x1B,0xB3,0x36,0xBB,0x67,0x53,0x9F,0xAF,
	  0xC2,0xAE,0x2D,0x0A,0x2E,0x75,0xC0,0x23,0x74,0xEA,0x4E,0xAC,0x8D,0x99,0x50,0x7F,
	  0x59,0xB9,0x53,0x77,0x30,0x5F,0x26,0x35,0xC6,0x08,0xA9,0x90,0x93,0xAC,0x8F,0xC6,
	  0xDE,0x23,0xB9,0x7A,0xEA,0x70,0xB4,0xC4,0xCF,0x66,0xB3,0x0E,0x58,0x32,0x0E,0xC5,
	  0xB6,0x72,0x04,0x48,0xCE,0x3B,0xB1,0x1C,0x53,0x1F,0xCB,0x70,0x28,0x7C,0xB5,0xC2,
	  0x7C,0x67,0x4F,0xBB,0xFD,0x8C,0x7F,0xC9,0x42,0x20,0xA4,0x73,0x23,0x1D,0x58,0x7E,
	  0x5A,0x1A,0x1A,0x82,0xE3,0x75,0x79,0xA1,0xBB,0x82,0x6E,0xCE,0x01,0x71,0xC9,0x75,
	  0x63,0x47,0x4B,0x1D,0x46,0xE6,0x79,0xB2,0x82,0x37,0x62,0x11,0xCD,0xC7,0x00,0x2F,
	  0x46,0x87,0xC2,0x3C,0x6D,0xC0,0xD5,0xB5,0x78,0x6E,0xE1,0xF2,0x73,0xFF,0x01,0x92,
	  0x50,0x0F,0xF4,0xC7,0x50,0x6A,0xEE,0x72,0xB6,0xF4,0x3D,0xF6,0x08,0xFE,0xA5,0x83,
	  0xA1,0xF9,0x86,0x0F,0x87,0xAF,0x52,0x44,0x54,0xBB,0x47,0xC3,0x06,0x0C,0x94,0xE9,
	  0x9B,0xF7,0xD6,0x32,0xA7,0xC8,0xAB,0x4B,0x4F,0xF5,0x35,0x21,0x1F,0xC1,0x80,0x47,
	  0xBB,0x7A,0xFA,0x5A,0x2B,0xD7,0xB8,0x84,0xAD,0x8E,0x56,0x4F,0x5B,0x89,0xFF,0x37,
	  0x97,0x37,0xF1,0xF5,0x01,0x3B,0x1F,0x9E,0xC4,0x18,0x6F,0x92,0x2A,0xD5,0xC4,0xB3,
	  0xC0,0xD5,0x87,0x0B,0x9C,0x04,0xAF,0x1A,0xB5,0xF3,0xBC,0x6D,0x0A,0xF1,0x7D,0x47,
	  0x08,0xE4,0x43,0xE9,0x73,0xF7,0xB7,0x70,0x77,0x54,0xBA,0xF3,0xEC,0xD2,0xAC,0x49},
	 { 0 },		// Padding
	 "Root"		// Issuer
	},

	// Public key
	{
	 BE32_CONST(RVL_CERT_KEYTYPE_RSA2048),	// Key type
	 "CA00000001",				// Child certificate identity
	 BE32_CONST(0x5BFA7D5C),		// Unknown...
	 // Modulus
	 {0xB2,0x79,0xC9,0xE2,0xEE,0xE1,0x21,0xC6,0xEA,0xF4,0x4F,0xF6,0x39,0xF8,0x8F,0x07,
	  0x8B,0x4B,0x77,0xED,0x9F,0x95,0x60,0xB0,0x35,0x82,0x81,0xB5,0x0E,0x55,0xAB,0x72,
	  0x11,0x15,0xA1,0x77,0x70,0x3C,0x7A,0x30,0xFE,0x3A,0xE9,0xEF,0x1C,0x60,0xBC,0x1D,
	  0x97,0x46,0x76,0xB2,0x3A,0x68,0xCC,0x04,0xB1,0x98,0x52,0x5B,0xC9,0x68,0xF1,0x1D,
	  0xE2,0xDB,0x50,0xE4,0xD9,0xE7,0xF0,0x71,0xE5,0x62,0xDA,0xE2,0x09,0x22,0x33,0xE9,
	  0xD3,0x63,0xF6,0x1D,0xD7,0xC1,0x9F,0xF3,0xA4,0xA9,0x1E,0x8F,0x65,0x53,0xD4,0x71,
	  0xDD,0x7B,0x84,0xB9,0xF1,0xB8,0xCE,0x73,0x35,0xF0,0xF5,0x54,0x05,0x63,0xA1,0xEA,
	  0xB8,0x39,0x63,0xE0,0x9B,0xE9,0x01,0x01,0x1F,0x99,0x54,0x63,0x61,0x28,0x70,0x20,
	  0xE9,0xCC,0x0D,0xAB,0x48,0x7F,0x14,0x0D,0x66,0x26,0xA1,0x83,0x6D,0x27,0x11,0x1F,
	  0x20,0x68,0xDE,0x47,0x72,0x14,0x91,0x51,0xCF,0x69,0xC6,0x1B,0xA6,0x0E,0xF9,0xD9,
	  0x49,0xA0,0xF7,0x1F,0x54,0x99,0xF2,0xD3,0x9A,0xD2,0x8C,0x70,0x05,0x34,0x82,0x93,
	  0xC4,0x31,0xFF,0xBD,0x33,0xF6,0xBC,0xA6,0x0D,0xC7,0x19,0x5E,0xA2,0xBC,0xC5,0x6D,
	  0x20,0x0B,0xAF,0x6D,0x06,0xD0,0x9C,0x41,0xDB,0x8D,0xE9,0xC7,0x20,0x15,0x4C,0xA4,
	  0x83,0x2B,0x69,0xC0,0x8C,0x69,0xCD,0x3B,0x07,0x3A,0x00,0x63,0x60,0x2F,0x46,0x2D,
	  0x33,0x80,0x61,0xA5,0xEA,0x6C,0x91,0x5C,0xD5,0x62,0x35,0x79,0xC3,0xEB,0x64,0xCE,
	  0x44,0xEF,0x58,0x6D,0x14,0xBA,0xAA,0x88,0x34,0x01,0x9B,0x3E,0xEB,0xEE,0xD3,0x79},
	 // Exponent
	 BE32_CONST(0x00010001),
	 // Padding
	 { 0 }
	}
};

// Ticket signing certificate. (retail)
static const RVL_Cert_RSA2048 cert_retail_ticket = {
	// Signature
	{
	 BE32_CONST(RVL_CERT_SIGTYPE_RSA2048),	// Signature type
	 // Signature
	 {0x7D,0x9D,0x5E,0xBA,0x52,0x81,0xDC,0xA7,0x06,0x5D,0x2F,0x08,0x68,0xDB,0x8A,0xC7,
	  0x3A,0xCE,0x7E,0xA9,0x91,0xF1,0x96,0x9F,0xE1,0xD0,0xF2,0xC1,0x1F,0xAE,0xC0,0xC3,
	  0xF0,0x1A,0xDC,0xB4,0x46,0xAD,0xE5,0xCA,0x03,0xB6,0x25,0x21,0x94,0x62,0xC6,0xE1,
	  0x41,0x0D,0xB9,0xE6,0x3F,0xDE,0x98,0xD1,0xAF,0x26,0x3B,0x4C,0xB2,0x87,0x84,0x27,
	  0x82,0x72,0xEF,0x27,0x13,0x4B,0x87,0xC2,0x58,0xD6,0x7B,0x62,0xF2,0xB5,0xBF,0x9C,
	  0xB6,0xBA,0x8C,0x89,0x19,0x2E,0xC5,0x06,0x89,0xAC,0x74,0x24,0xA0,0x22,0x09,0x40,
	  0x03,0xEE,0x98,0xA4,0xBD,0x2F,0x01,0x3B,0x59,0x3F,0xE5,0x66,0x6C,0xD5,0xEB,0x5A,
	  0xD7,0xA4,0x93,0x10,0xF3,0x4E,0xFB,0xB4,0x3D,0x46,0xCB,0xF1,0xB5,0x23,0xCF,0x82,
	  0xF6,0x8E,0xB5,0x6D,0xB9,0x04,0xA7,0xC2,0xA8,0x2B,0xE1,0x1D,0x78,0xD3,0x9B,0xA2,
	  0x0D,0x90,0xD3,0x07,0x42,0xDB,0x5E,0x7A,0xC1,0xEF,0xF2,0x21,0x51,0x09,0x62,0xCF,
	  0xA9,0x14,0xA8,0x80,0xDC,0xF4,0x17,0xBA,0x99,0x93,0x0A,0xEE,0x08,0xB0,0xB0,0xE5,
	  0x1A,0x3E,0x9F,0xAF,0xCD,0xC2,0xD7,0xE3,0xCB,0xA1,0x2F,0x3A,0xC0,0x07,0x90,0xDE,
	  0x44,0x7A,0xC3,0xC5,0x38,0xA8,0x67,0x92,0x38,0x07,0x8B,0xD4,0xC4,0xB2,0x45,0xAC,
	  0x29,0x16,0x88,0x6D,0x2A,0x0E,0x59,0x4E,0xED,0x5C,0xC8,0x35,0x69,0x8B,0x4D,0x62,
	  0x38,0xDF,0x05,0x72,0x4D,0xCC,0xF6,0x81,0x80,0x8A,0x70,0x74,0x06,0x59,0x30,0xBF,
	  0xF8,0x51,0x41,0x37,0xE8,0x15,0xFA,0xBA,0xA1,0x72,0xB8,0xE0,0x69,0x6C,0x61,0xE4},
	 { 0 },			// Padding
	 "Root-CA00000001"	// Issuer
	},

	// Public key
	{
	 BE32_CONST(RVL_CERT_KEYTYPE_RSA2048),	// Key type
	 "XS00000003",				// Child certificate identity
	 BE32_CONST(0xF1B89FD1),		// Unknown...
	 // Modulus
	 {0xAD,0x07,0xA9,0x37,0x8A,0x7B,0x10,0x0C,0x7D,0xC7,0x39,0xBE,0x9E,0xDD,0xB7,0x32,
	  0x00,0x89,0xAB,0x25,0xB1,0xF8,0x71,0xAF,0x5A,0xA9,0xF4,0x58,0x9E,0xD1,0x83,0x02,
	  0x32,0x8E,0x81,0x1A,0x1F,0xEF,0xD0,0x09,0xC8,0x06,0x36,0x43,0xF8,0x54,0xB9,0xE1,
	  0x3B,0xBB,0x61,0x3A,0x7A,0xCF,0x87,0x14,0x85,0x6B,0xA4,0x5B,0xAA,0xE7,0xBB,0xC6,
	  0x4E,0xB2,0xF7,0x5D,0x87,0xEB,0xF2,0x67,0xED,0x0F,0xA4,0x41,0xA9,0x33,0x66,0x5E,
	  0x57,0x7D,0x5A,0xDE,0xAB,0xFB,0x46,0x2E,0x76,0x00,0xCA,0x9C,0xE9,0x4D,0xC4,0xCB,
	  0x98,0x39,0x92,0xAB,0x7A,0x2F,0xB3,0xA3,0x9E,0xA2,0xBF,0x9C,0x53,0xEC,0xD0,0xDC,
	  0xFA,0x6B,0x8B,0x5E,0xB2,0xCB,0xA4,0x0F,0xFA,0x40,0x75,0xF8,0xF2,0xB2,0xDE,0x97,
	  0x38,0x11,0x87,0x2D,0xF5,0xE2,0xA6,0xC3,0x8B,0x2F,0xDC,0x8E,0x57,0xDD,0xBD,0x5F,
	  0x46,0xEB,0x27,0xD6,0x19,0x52,0xF6,0xAE,0xF8,0x62,0xB7,0xEE,0x9A,0xC6,0x82,0xA2,
	  0xB1,0x9A,0xA9,0xB5,0x58,0xFB,0xEB,0xB3,0x89,0x2F,0xBD,0x50,0xC9,0xF5,0xDC,0x4A,
	  0x6E,0x9C,0x9B,0xFE,0x45,0x80,0x34,0xA9,0x42,0x18,0x2D,0xDE,0xB7,0x5F,0xE0,0xD1,
	  0xB3,0xDF,0x0E,0x97,0xE3,0x99,0x80,0x87,0x70,0x18,0xC2,0xB2,0x83,0xF1,0x35,0x75,
	  0x7C,0x5A,0x30,0xFC,0x3F,0x30,0x84,0xA4,0x9A,0xAA,0xC0,0x1E,0xE7,0x06,0x69,0x4F,
	  0x8E,0x14,0x48,0xDA,0x12,0x3A,0xCC,0x4F,0xFA,0x26,0xAA,0x38,0xF7,0xEF,0xBF,0x27,
	  0x8F,0x36,0x97,0x79,0x77,0x5D,0xB7,0xC5,0xAD,0xC7,0x89,0x91,0xDC,0xF8,0x43,0x8D},
	 // Exponent
	 BE32_CONST(0x00010001),
	 // Padding
	 { 0 }
	}
};

// TMD signing certificate. (retail)
static const RVL_Cert_RSA2048 cert_retail_tmd = {
	// Signature
	{
	 BE32_CONST(RVL_CERT_SIGTYPE_RSA2048),	// Signature type
	 // Signature
	 {0x4E,0x00,0x5F,0xF1,0x3F,0x86,0x75,0x8D,0xB6,0x9C,0x45,0x63,0x0F,0xD4,0x9B,0xF4,
	  0xCC,0x5D,0x54,0xCF,0xCC,0x22,0x34,0x72,0x57,0xAB,0xA4,0xBA,0x53,0xD2,0xB3,0x3D,
	  0xE6,0xEC,0x9E,0xA1,0x57,0x54,0x53,0xAE,0x5F,0x93,0x3D,0x96,0xBF,0xF7,0xCC,0x7A,
	  0x79,0x56,0x6E,0x84,0x7B,0x1B,0x60,0x77,0xC2,0xA9,0x38,0x71,0x30,0x1A,0x8C,0xD3,
	  0xC9,0x3D,0x4D,0xB3,0x26,0xE9,0x87,0x92,0x66,0xE9,0xD3,0xBA,0x9F,0x79,0xBC,0x46,
	  0x38,0xFA,0x2D,0x20,0xA0,0x3A,0x70,0x67,0xA4,0x11,0xA7,0xA0,0xB7,0xD9,0x12,0xAD,
	  0x11,0x6A,0x3A,0xC4,0x6E,0x32,0x42,0x47,0xC2,0x08,0xBA,0xB4,0x94,0x9C,0xC5,0x2E,
	  0xD0,0x2F,0x19,0xF6,0x51,0xE0,0xDF,0x2E,0x36,0x53,0xAA,0xAF,0x97,0xA6,0x92,0xBB,
	  0xA9,0x1D,0xD8,0x6E,0x24,0x2E,0xB3,0x08,0x77,0x55,0x11,0xCE,0x98,0xF6,0xA2,0xF4,
	  0x26,0xC9,0x27,0x04,0xD0,0xFC,0x8D,0xD4,0x80,0x9E,0xD7,0x61,0xBD,0x11,0xB7,0x85,
	  0x94,0x8C,0xD6,0xD0,0x7A,0xDB,0xA4,0x08,0xD0,0xF0,0x86,0xF6,0x5A,0xAE,0x19,0x14,
	  0xB2,0x88,0x9A,0xA8,0xAE,0x4A,0xA2,0xAA,0xC7,0x61,0xA9,0x0D,0x41,0x2C,0xB1,0x50,
	  0x09,0xAB,0x3E,0x93,0xFC,0xA9,0x24,0xDE,0xCE,0x4F,0x7C,0x06,0xAB,0xDC,0x2E,0x60,
	  0x9D,0x68,0xBE,0x00,0x73,0xFA,0x80,0x57,0x6A,0x14,0x5E,0xED,0xC4,0x8B,0x74,0x32,
	  0x87,0x07,0x93,0xC8,0xFC,0xA6,0xD8,0x3E,0x09,0x6E,0xC5,0xF2,0xA9,0xC4,0x21,0xE7,
	  0x48,0xB3,0x73,0x40,0x5B,0xE2,0xFA,0x8A,0xE1,0x58,0x78,0xE9,0xD5,0x23,0x88,0x75},
	 { 0 },			// Padding
	 "Root-CA00000001"	// Issuer
	},

	// Public key
	{
	 BE32_CONST(RVL_CERT_KEYTYPE_RSA2048),	// Key type
	 "CP00000004",				// Child certificate identity
	 BE32_CONST(0xF1B8A064),		// Unknown...
	 // Modulus
	 {0xC1,0x6D,0xF3,0x83,0x29,0x55,0xC3,0x29,0x5B,0x72,0xF0,0x33,0x2E,0x97,0xEF,0x14,
	  0x84,0x8A,0x68,0x04,0x9C,0xA6,0x8E,0xAC,0xDE,0x14,0x50,0x33,0xB8,0x6C,0x10,0x8D,
	  0x48,0x33,0x5C,0x5D,0x0C,0xAB,0x77,0x04,0x62,0x54,0x47,0x55,0x45,0x2A,0x90,0x00,
	  0x70,0xB1,0x56,0x92,0x5C,0x17,0x86,0xE2,0xCD,0x20,0x6D,0xCC,0xDC,0x2C,0x2E,0x37,
	  0x6E,0x27,0xFC,0xB4,0x20,0x66,0xCC,0x0A,0x8C,0xE9,0xFE,0xE8,0x57,0x04,0xE6,0xCA,
	  0x63,0x1A,0x2E,0x7E,0x91,0x7E,0x94,0x7C,0x39,0x91,0x77,0x36,0x29,0xD1,0x55,0x61,
	  0x85,0xBB,0xD7,0xB7,0x73,0xCA,0x37,0x47,0x9E,0x5F,0xAA,0xA3,0xB6,0x05,0xE0,0x01,
	  0xE1,0xAC,0xE5,0x8D,0xD8,0xF8,0x47,0x82,0xD6,0x45,0xFC,0xE3,0xA1,0xCD,0x03,0xAB,
	  0x36,0xF0,0xF3,0x86,0xB1,0xA2,0xD1,0x37,0x40,0xA1,0x94,0x8A,0x53,0xBA,0x1B,0x0D,
	  0x8C,0x48,0x63,0xCD,0x6B,0x2C,0x2E,0x20,0x64,0x94,0x80,0x4C,0x62,0xFA,0xA9,0x3A,
	  0x7E,0x33,0xA9,0xEA,0x78,0x6B,0x59,0xCA,0xE3,0xAB,0x36,0x45,0xF4,0xCB,0x8F,0xD7,
	  0x90,0x6B,0x82,0x68,0xCD,0xAC,0xF1,0x7B,0x3A,0xEC,0x46,0x83,0x1B,0x91,0xF6,0xDE,
	  0x18,0x61,0x83,0xBC,0x4B,0x32,0x67,0x93,0xC7,0x2E,0x50,0xD9,0x1E,0x36,0xA0,0xDC,
	  0xE2,0xB9,0x7D,0xA0,0x21,0x3E,0x46,0x96,0x02,0x1F,0x33,0x1C,0xBE,0xAE,0x8D,0xFC,
	  0x92,0x87,0x32,0xAA,0x44,0xDC,0x78,0xE7,0x19,0x9A,0x3D,0xDD,0x57,0x22,0x7E,0x9E,
	  0x77,0xDE,0x32,0x63,0x86,0x93,0x6C,0x11,0xAC,0xA7,0x0F,0x81,0x19,0xD3,0x3A,0x99},
	 // Exponent
	 BE32_CONST(0x00010001),
	 // Padding
	 { 0 }
	}
};

/** Debug certificates. **/

// CA certificate. (debug)
static const RVL_Cert_RSA4096_RSA2048 cert_debug_ca = {
	// Signature
	{
	 BE32_CONST(RVL_CERT_SIGTYPE_RSA4096),	// Signature type
	 // Signature
	 {0x90,0x56,0xB5,0x0C,0x36,0x30,0xEB,0xF9,0xC8,0x80,0xD7,0xB7,0x74,0x78,0x04,0x14,
	  0xB8,0xA9,0x35,0x89,0x5F,0xD4,0x87,0xBB,0xA0,0x13,0x52,0xC2,0x6F,0x62,0x99,0x18,
	  0x3E,0x68,0xEB,0xF0,0xFF,0x79,0xD4,0x2E,0xF6,0xB1,0x8C,0xB6,0x30,0x5D,0xB3,0x9B,
	  0x18,0xA9,0x12,0x33,0x1A,0x5F,0xA6,0xAB,0x3F,0x50,0xCB,0xB8,0xDD,0xAB,0x4F,0x66,
	  0xC5,0xA3,0x58,0x35,0x8F,0x4A,0x48,0x33,0x4E,0x3A,0x86,0x68,0x6E,0x66,0x8C,0x0C,
	  0xBE,0x84,0x47,0xA1,0x66,0xCB,0xCB,0x4E,0xFD,0xF6,0x25,0x37,0x30,0x07,0x6C,0x2A,
	  0x78,0x51,0x56,0x94,0xCD,0x4C,0xF4,0x91,0x98,0x9F,0xA5,0x09,0x04,0x41,0x72,0x50,
	  0x7A,0x51,0xEA,0x8E,0x71,0xC6,0x1A,0x94,0x33,0xB0,0x2A,0x07,0xA5,0xF1,0x26,0x57,
	  0x42,0x4D,0x44,0x75,0x2A,0xFE,0x65,0x56,0x96,0x36,0x33,0xC7,0xC3,0x97,0x98,0x2A,
	  0x71,0xA4,0x80,0x7F,0x09,0x78,0x9A,0xEE,0xFB,0x2B,0x07,0x69,0xCB,0x1D,0xA4,0xE0,
	  0xCA,0xA7,0x0B,0xB8,0xF6,0xFD,0xCF,0x46,0xBD,0x35,0xC0,0x8A,0x00,0x9C,0x75,0xAB,
	  0x9F,0x52,0x23,0xC9,0x5F,0xF7,0xDA,0xAE,0x2F,0x53,0xE8,0xE1,0xAF,0x25,0x0D,0xAB,
	  0x1D,0x1B,0xB2,0xD7,0x6A,0x4A,0x33,0x38,0x69,0x95,0x38,0xC6,0xCD,0xFE,0xBC,0x72,
	  0xD0,0xC3,0x69,0xEC,0x1F,0x91,0x9E,0x81,0xEE,0x2B,0x40,0x90,0x6C,0xFD,0xD3,0x69,
	  0x97,0xAB,0x49,0xD3,0xA9,0xD5,0x91,0xDD,0xC8,0x42,0xA0,0xE4,0x89,0x7B,0xC8,0x8B,
	  0xD0,0x21,0x67,0x4F,0x7E,0xBC,0xB2,0x00,0xA3,0xC4,0x21,0xB3,0x95,0x7F,0x65,0x50,
	  0x9C,0xD1,0xC3,0x2C,0xB5,0x5E,0xEB,0xB4,0x62,0x99,0xB8,0x27,0xD6,0x2E,0x53,0x63,
	  0x2C,0xFF,0x70,0x07,0xBF,0xFB,0x5D,0xCF,0x84,0x15,0xFD,0x3F,0xC3,0xD8,0x34,0xE3,
	  0x6B,0xF3,0x1F,0x59,0x90,0x9F,0x2F,0x66,0xC6,0xEA,0xC1,0xFD,0xFB,0x34,0x20,0xA8,
	  0xEC,0x94,0xDD,0x25,0xF1,0x04,0x10,0xF3,0x08,0x33,0xD8,0xCB,0x08,0x70,0x2E,0x4A,
	  0x8D,0x27,0xCC,0xA7,0x5B,0xCB,0x5A,0xC3,0xEB,0x30,0x96,0xEC,0x64,0xF8,0xF5,0x28,
	  0xDD,0xFE,0x51,0xF6,0x26,0x9C,0xF6,0x9E,0x8D,0x79,0x4F,0x1A,0xD0,0x46,0x04,0x06,
	  0xFE,0x79,0xEC,0x98,0x00,0x9C,0x6A,0x3E,0xE7,0xBD,0x3D,0x26,0x30,0x44,0x06,0x6B,
	  0xA1,0xF4,0x58,0xB6,0x13,0x40,0x13,0x8B,0x03,0x32,0xA3,0x10,0xB8,0xD3,0xC2,0x3E,
	  0xAC,0x22,0xB0,0xD6,0x06,0xA9,0x8F,0x51,0x98,0xD8,0xCF,0x3A,0xC7,0x63,0x85,0x04,
	  0xFF,0x0A,0x02,0xD1,0x37,0xAF,0x4A,0x7B,0xB8,0x91,0x46,0x95,0x25,0xF3,0x6C,0xEB,
	  0xFD,0xCB,0x0C,0xD7,0xF2,0xD5,0xE7,0x2E,0xCE,0xDC,0x26,0xFE,0x5D,0xB2,0x0B,0x71,
	  0xAB,0x5B,0x47,0xD3,0xA4,0xC9,0xEF,0xEB,0xF4,0x2F,0x48,0x48,0xAB,0xF7,0xF5,0x63,
	  0x79,0x97,0x08,0x8A,0x4C,0x75,0x0D,0xDE,0x29,0x0F,0x18,0x77,0x8B,0x2B,0x8C,0x3D,
	  0x60,0x17,0xEB,0x2A,0xDA,0x37,0x4E,0x1D,0x17,0xEF,0xF9,0x98,0xE8,0xBF,0xB2,0x53,
	  0xDE,0xE2,0xF4,0xB5,0x63,0x02,0x62,0x95,0xEB,0x89,0x49,0xB5,0xB4,0xD1,0x03,0xA2,
	  0xF5,0x00,0x11,0x5F,0x45,0xA1,0xD0,0xF2,0x3B,0x92,0xC0,0x34,0x06,0xE4,0xB6,0xD1},
	 { 0 },		// Padding
	 "Root"		// Issuer
	},

	// Public key
	{
	 BE32_CONST(RVL_CERT_KEYTYPE_RSA2048),	// Key type
	 "CA00000002",				// Child certificate identity
	 BE32_CONST(0x65648F2B),		// Unknown...
	 // Modulus
	 {0xC9,0xCC,0x2D,0xC4,0xDF,0x29,0x30,0xE4,0xDF,0x3F,0x8C,0x70,0xA0,0x78,0x94,0x87,
	  0x75,0xAD,0x5E,0x9A,0xA6,0x04,0xC5,0xB4,0xD8,0xEA,0xFF,0x2A,0xA1,0xD2,0x14,0x67,
	  0x65,0x64,0xEF,0xCA,0x28,0xCC,0x00,0x15,0x45,0x54,0xA1,0xA3,0xEA,0x13,0x79,0xE9,
	  0xE6,0xCA,0xAC,0xED,0x15,0x93,0xFE,0x88,0xD8,0x9A,0xC6,0xB8,0xAC,0xCC,0xAB,0x6E,
	  0x20,0x7C,0xEB,0x7C,0xCA,0x29,0x80,0x9E,0x29,0x80,0x44,0x06,0x62,0xB7,0xD4,0x38,
	  0x2A,0x15,0xDA,0x43,0x08,0x57,0x45,0xA9,0xAA,0xE5,0x9A,0xA0,0x5B,0xDB,0x32,0xF6,
	  0x68,0x69,0xA2,0xDD,0x42,0x95,0x38,0x6C,0x87,0xEC,0xDD,0x35,0x08,0xA2,0xCF,0x60,
	  0xD0,0x1E,0x23,0xEC,0x2F,0xE6,0x98,0xF4,0x70,0xD6,0x00,0x15,0x49,0xA2,0xF0,0x67,
	  0x59,0x13,0x1E,0x53,0x4C,0x70,0x06,0x05,0x7D,0xEF,0x1D,0x18,0xA8,0x3F,0x0A,0xC7,
	  0x9C,0xFE,0x80,0xFF,0x5A,0x91,0xF2,0xBE,0xD4,0xA0,0x83,0x70,0x61,0x19,0x0A,0x03,
	  0x29,0x90,0x21,0x65,0x40,0x3C,0x9A,0x90,0x8F,0xB6,0x15,0x73,0x9F,0x3C,0xE3,0x3B,
	  0xF1,0xBA,0xEA,0x16,0xC2,0x5B,0xCE,0xD7,0x96,0x3F,0xAC,0xC9,0xD2,0x4D,0x9C,0x0A,
	  0xD7,0x6F,0xC0,0x20,0xB2,0xC4,0xB8,0x4C,0x10,0xA7,0x41,0xA2,0xCC,0x7D,0x9B,0xAC,
	  0x3A,0xAC,0xCC,0xA3,0x52,0x9B,0xAC,0x31,0x6A,0x9A,0xA7,0x5D,0x2A,0x26,0xC7,0xD7,
	  0xD2,0x88,0xCB,0xA4,0x66,0xC5,0xFE,0x5F,0x45,0x4A,0xE6,0x79,0x74,0x4A,0x90,0xA1,
	  0x57,0x72,0xDB,0x3B,0x0E,0x47,0xA4,0x9A,0xF0,0x31,0xD1,0x6D,0xBE,0xAB,0x33,0x2B},
	 // Exponent
	 BE32_CONST(0x00010001),
	 // Padding
	 { 0 }
	}
};

// Ticket signing certificate. (debug)
static const RVL_Cert_RSA2048 cert_debug_ticket = {
	// Signature
	{
	 BE32_CONST(RVL_CERT_SIGTYPE_RSA2048),	// Signature type
	 // Signature
	 {0x87,0xC6,0xF8,0x0F,0xAE,0xE2,0xA8,0x5A,0x01,0xA1,0xE0,0x4B,0x04,0x76,0xC9,0x04,
	  0x08,0xE6,0x93,0xF5,0x14,0xEE,0x46,0x96,0xBC,0xFD,0x06,0x91,0xE3,0xE7,0xF9,0x67,
	  0x8F,0x0B,0x9B,0x90,0xAE,0xCB,0x99,0x56,0xA7,0xBB,0x1C,0x9B,0x1E,0xE3,0x96,0x31,
	  0xC8,0xD5,0x5D,0xAE,0x20,0x30,0x48,0xF2,0xCC,0xFF,0xC5,0xCA,0xCA,0x3E,0xBE,0xB0,
	  0xCC,0x1E,0x3A,0x58,0x26,0xF5,0x7E,0xB3,0x3E,0x48,0x44,0x5D,0xD6,0xCA,0x34,0xCD,
	  0x1A,0x1B,0xE6,0x70,0x18,0xA4,0xD8,0xCC,0x85,0x41,0xCE,0x3E,0x0D,0xBA,0xAE,0x1F,
	  0x68,0x3C,0x40,0x87,0xDF,0x84,0x65,0xBA,0x82,0x2A,0xF6,0xBD,0x37,0xC6,0x93,0x92,
	  0xE3,0x2F,0xE2,0xAD,0xD0,0x23,0xF5,0xF3,0x72,0x1A,0x56,0x76,0xAE,0x72,0x47,0x19,
	  0x43,0x71,0xFE,0x87,0x27,0x27,0xAB,0x09,0xE0,0x64,0x87,0x4C,0x1A,0x9B,0xF7,0x0F,
	  0x5E,0x4B,0x6F,0x6D,0x8F,0xE5,0x24,0xF4,0xA8,0x2C,0x28,0xAA,0xFE,0x80,0x69,0x68,
	  0xF4,0x9F,0xEE,0x7E,0x2E,0xB9,0xB8,0xF9,0x1E,0x7B,0xF8,0xB3,0xA9,0xAA,0x1D,0x1E,
	  0xF2,0xC1,0xE3,0xCB,0x88,0xB4,0xBC,0x47,0x29,0x86,0xEB,0xC2,0x1D,0x39,0xF3,0x91,
	  0x3E,0xCD,0x00,0x19,0x71,0x41,0xF2,0xB5,0x3A,0x2F,0x47,0xD6,0xB2,0xFE,0xBE,0x0D,
	  0x77,0xEB,0xC4,0x01,0x35,0x55,0x8E,0xA8,0x86,0xE3,0x5F,0x7B,0xD9,0x0B,0x3F,0x45,
	  0x0B,0x19,0xA4,0x4D,0xA4,0x68,0x28,0x6F,0xA1,0x2D,0x0A,0xBB,0xB7,0x81,0x64,0x0A,
	  0x09,0x5B,0x01,0x36,0x87,0xA3,0xEA,0x61,0x15,0xF4,0xE5,0x88,0xEA,0x95,0x17,0x74},
	 { 0 },			// Padding
	 "Root-CA00000002"	// Issuer
	},

	// Public key
	{
	 BE32_CONST(RVL_CERT_KEYTYPE_RSA2048),	// Key type
	 "XS00000006",				// Child certificate identity
	 BE32_CONST(0xF868289D),		// Unknown...
	 // Modulus
	 {0xCB,0xDA,0x82,0x8B,0xB1,0x9C,0x9F,0xCD,0x0B,0x6E,0xBE,0x65,0xE6,0x85,0x7B,0x29,
	  0x85,0xB1,0x56,0x67,0x16,0xD8,0xEB,0xD1,0x42,0x68,0x50,0x01,0x01,0x04,0x24,0x37,
	  0x73,0x87,0x87,0x9A,0x92,0x7D,0xCD,0x52,0xC2,0x3A,0xA3,0x80,0xB6,0xE8,0x50,0x81,
	  0x13,0xE5,0xEC,0xDE,0xDA,0x5D,0x38,0xB3,0x3D,0x9B,0xFA,0x4A,0x61,0x02,0x00,0xF1,
	  0xC2,0xC8,0x68,0x7F,0x1B,0xCE,0x09,0xDD,0x04,0x95,0x64,0xBD,0xA5,0x9A,0x22,0x14,
	  0x9E,0x3E,0xA0,0x33,0x73,0x31,0xD3,0x93,0x60,0x44,0xB0,0x9C,0x65,0x01,0xF8,0xB8,
	  0x79,0xE9,0x5E,0x8A,0x16,0x0D,0xC6,0x3F,0x1B,0x7D,0x5F,0x3B,0x7E,0x64,0xAA,0xE1,
	  0x51,0x27,0x38,0x91,0x29,0xB4,0xB1,0x13,0xC1,0x2C,0xB7,0x96,0x5C,0x48,0xE4,0x89,
	  0xE8,0x58,0xA5,0x34,0x86,0xFE,0x21,0x11,0x76,0x6E,0x01,0x24,0xCB,0x5F,0x83,0x36,
	  0x99,0x87,0x32,0xCB,0xAC,0xF5,0x6E,0x46,0x4A,0x6C,0xD2,0xCD,0x65,0x27,0x2C,0xD1,
	  0x46,0x30,0x3F,0x31,0xAA,0x39,0x9A,0x44,0xBE,0x5E,0x91,0x93,0x51,0x82,0xF8,0x65,
	  0xF3,0x66,0x5D,0x06,0x10,0x7C,0x9E,0xE3,0x8D,0x91,0x03,0xEE,0x58,0x54,0x8B,0x30,
	  0x1A,0x9E,0x01,0xDB,0x37,0x6B,0x89,0x3D,0xC4,0xE6,0xC7,0x4E,0xBE,0xA2,0x5A,0xCF,
	  0x0D,0x28,0xE7,0xC7,0xB3,0x12,0x99,0x8D,0xE7,0x71,0x93,0x9D,0xD1,0xE3,0xD6,0x6B,
	  0xD4,0x37,0xC0,0xD5,0x7D,0x8B,0xEF,0x33,0xE5,0x27,0xBA,0x04,0xCF,0xD0,0x62,0x61,
	  0x9C,0xD0,0x7E,0x38,0xC1,0xC7,0xE6,0x70,0x49,0x20,0x71,0x5C,0xC6,0x35,0x76,0x15},
	 // Exponent
	 BE32_CONST(0x00010001),
	 // Padding
	 { 0 }
	}
};

// TMD signing certificate. (debug)
static const RVL_Cert_RSA2048 cert_debug_tmd = {
	// Signature
	{
	 BE32_CONST(RVL_CERT_SIGTYPE_RSA2048),	// Signature type
	 // Signature
	 {0x5B,0x02,0xA6,0x86,0x5D,0xE2,0x37,0x0B,0xFB,0x79,0x36,0x2F,0xDE,0x0E,0x40,0xB6,
	  0x03,0x58,0x1E,0xD7,0x94,0xAA,0x6A,0x3E,0xB3,0xC3,0x09,0x43,0xAA,0x0B,0xBD,0xDE,
	  0x69,0x40,0xC0,0x19,0x29,0x09,0xB6,0xE9,0xA2,0x16,0x1A,0x77,0x65,0x11,0x72,0x69,
	  0x0B,0x4C,0xBB,0x74,0x77,0x46,0xC2,0x5E,0x82,0x7C,0x80,0x8B,0x7F,0x56,0x1E,0x51,
	  0x3A,0xAB,0x6A,0x1B,0xEB,0xCE,0x05,0x8D,0x2F,0x47,0x7D,0x80,0x27,0xE8,0x08,0xEA,
	  0x9A,0x00,0x41,0x5C,0xA0,0x32,0xBB,0x9F,0x4C,0xE8,0x15,0x94,0x20,0xF2,0xE9,0x08,
	  0xAB,0x82,0x50,0xAE,0x00,0x56,0x59,0xE4,0xE8,0x27,0x7C,0x8F,0xFB,0x06,0x19,0x19,
	  0xDF,0x61,0x3E,0x84,0x29,0x5B,0x1D,0x30,0x76,0xF5,0x08,0x98,0x7D,0x59,0x8F,0x6D,
	  0x94,0x0D,0x0F,0x37,0xF9,0x2C,0x3F,0xAE,0x6F,0xB3,0x5E,0xCD,0x3D,0xE5,0xF1,0x86,
	  0x0D,0xF1,0xD1,0x45,0x9B,0x98,0xA3,0x3A,0x86,0xDC,0x4B,0x0E,0x1C,0x68,0xFB,0x61,
	  0xF4,0x70,0xD5,0x7C,0x42,0x35,0x8E,0x96,0xB2,0x88,0x32,0x9B,0x52,0x9B,0xE9,0xA5,
	  0x5B,0x17,0x0C,0x8A,0x94,0x6B,0xF6,0x48,0x45,0x76,0x51,0xD3,0x50,0x76,0xC3,0x2D,
	  0xC4,0x28,0x75,0x26,0xB8,0x52,0xDB,0x66,0xE3,0x78,0xA2,0x1C,0x81,0x16,0xE4,0x14,
	  0x08,0xBA,0xD8,0x94,0x18,0xEA,0x4D,0x09,0xA8,0x88,0x73,0x67,0x5E,0xF4,0x2C,0x67,
	  0x38,0x81,0x59,0x4B,0x76,0x2F,0x66,0x6F,0x20,0xBB,0x01,0x17,0xB1,0xFF,0x13,0x64,
	  0xE7,0x14,0x42,0x79,0x69,0x15,0x19,0x50,0x2D,0x6D,0x4E,0x1E,0x62,0x6E,0x78,0xB9},
	 { 0 },			// Padding
	 "Root-CA00000002",	// Issuer
	},

	// Public key
	{
	 BE32_CONST(RVL_CERT_KEYTYPE_RSA2048),	// Key type
	 "CP00000007",				// Child certificate identity
	 BE32_CONST(0xF86828DD),		// Unknown...
	 // Modulus
	 {0xC3,0xD9,0xA5,0xF3,0xC2,0x5D,0x16,0xD2,0x64,0xAD,0x2C,0x0C,0xB7,0xE6,0x5C,0x50,
	  0x6D,0x07,0x63,0x05,0x32,0x6E,0xD9,0xCD,0xEB,0x3A,0x5F,0x23,0xB0,0x3F,0x38,0x80,
	  0x3F,0x60,0x96,0xDC,0xCC,0xDB,0xE3,0xD4,0x6E,0x9A,0x00,0x7B,0x25,0xBE,0xF9,0x59,
	  0xE7,0x90,0x4A,0xDD,0x10,0xF6,0x12,0x00,0x99,0xC6,0xFD,0x3C,0x80,0xBE,0x9B,0xDF,
	  0x74,0x5A,0x04,0x59,0xB2,0x2A,0x7C,0x0C,0xB4,0xE7,0x73,0xD7,0x04,0xF4,0x2B,0x77,
	  0x9A,0x69,0xAC,0x9F,0xDA,0x4E,0x65,0xF1,0x3C,0x30,0x38,0x98,0x4E,0x94,0x67,0xE3,
	  0xA9,0x11,0x2D,0x81,0x5E,0x53,0xF5,0x75,0xD9,0x27,0x52,0x87,0xFB,0x98,0x10,0x98,
	  0x2F,0x62,0x30,0x93,0x5E,0xAF,0xEB,0xC0,0x3A,0x64,0x53,0x6B,0x60,0x2E,0x17,0x22,
	  0x25,0x97,0x5F,0x64,0x8B,0x10,0x67,0xC0,0x14,0x9C,0xBE,0x8F,0x1E,0x15,0xF5,0x73,
	  0xA7,0x50,0x04,0x7F,0x34,0xEB,0x02,0x03,0x32,0x41,0x75,0x20,0x4A,0x40,0x7C,0x79,
	  0xDD,0xE4,0x29,0xD6,0x60,0x73,0x2E,0x02,0xD4,0x79,0x87,0xF1,0x33,0x5A,0xAA,0x95,
	  0xFB,0x55,0x0D,0x29,0x0D,0xA2,0xCF,0xC6,0x04,0xB7,0x8A,0x63,0xCA,0x64,0x52,0x87,
	  0xAA,0xEB,0xE0,0x7D,0x6F,0x6C,0xDF,0x0E,0x6F,0xF1,0xC4,0xC2,0x60,0x6C,0xC4,0x77,
	  0x29,0x03,0x1E,0x6B,0x9C,0x3C,0xA3,0xED,0x2D,0xB4,0xF0,0x58,0x81,0x50,0xB2,0x6E,
	  0x66,0xDB,0x99,0xAC,0x44,0x77,0xA6,0xFE,0x80,0x6E,0x08,0xFA,0xEA,0xC5,0x27,0x92,
	  0x71,0xFF,0xCF,0x29,0xB1,0x13,0xEB,0x14,0x50,0xD1,0x81,0xD8,0xBF,0x48,0x05,0xBD},
	 // Exponent
	 BE32_CONST(0x00010001),
	 // Padding
	 { 0 }
	}
};

// Development certificate. (debug)
static const RVL_Cert_RSA2048_ECC cert_debug_dev = {
	// Signature
	{
	 BE32_CONST(RVL_CERT_SIGTYPE_RSA2048),	// Signature type
	 // Signature
	 {0x92,0x72,0xBB,0xBD,0x15,0x6C,0x59,0xAB,0x4D,0x88,0x0D,0x8D,0xF2,0xA8,0x69,0xF7,
	  0x77,0xDB,0xDB,0x5A,0x27,0xFC,0xB3,0x71,0x22,0x4E,0x06,0xBD,0x13,0x85,0x62,0xAD,
	  0x87,0x0C,0x77,0xD2,0xF1,0x69,0x6E,0x41,0xA7,0x01,0xD0,0x3A,0xEF,0x5B,0xB1,0x54,
	  0x56,0xC4,0x93,0xA9,0x66,0x10,0xFF,0x8E,0x3E,0x61,0x7B,0x42,0xF3,0x63,0x58,0x32,
	  0x26,0xAD,0x2B,0x8F,0x9E,0xE4,0x7E,0xCF,0xAC,0x0D,0x02,0xFE,0xE0,0xA7,0xD1,0xED,
	  0x0C,0x6E,0x7E,0x5A,0xB4,0x04,0x5C,0x8D,0xFC,0x66,0x44,0x1D,0x6D,0xB4,0x0E,0x40,
	  0xE7,0x1E,0x82,0x0D,0xE9,0x9D,0xB2,0x70,0xAF,0x5F,0x27,0x0D,0x47,0x4A,0x9F,0x4F,
	  0xE7,0xDB,0x81,0x08,0x1D,0x1E,0xF0,0xDC,0xEC,0x2A,0xEC,0x8E,0x3D,0xAF,0x60,0xD0,
	  0xAA,0xDD,0x0B,0x0C,0x3E,0x9E,0xC0,0x49,0xEE,0xF6,0x07,0x0E,0xC3,0x97,0x92,0x28,
	  0x0A,0xC4,0x8B,0xF8,0x1F,0x4B,0x16,0x56,0x0D,0xFB,0x40,0xF2,0xC9,0x2A,0xF2,0xCE,
	  0x8F,0x3E,0x1C,0x44,0x3D,0x85,0xE6,0x63,0x09,0x16,0xE2,0x77,0x9A,0x92,0x29,0x47,
	  0x8E,0x4E,0x7D,0xFB,0x22,0x16,0xA9,0x3D,0xD9,0xEC,0x2D,0xD6,0xC3,0x10,0x50,0x15,
	  0xF6,0x99,0x8D,0x65,0xB5,0x04,0x76,0x22,0x1D,0xCA,0x37,0xE2,0x45,0xC6,0x60,0x49,
	  0xA5,0x54,0x0A,0x47,0x5B,0x63,0x27,0xD4,0x97,0x62,0xAD,0x52,0x9E,0x23,0xC2,0x74,
	  0xCF,0x13,0xFA,0x78,0x14,0x0B,0x6A,0x8F,0xFA,0x57,0xC0,0xC6,0x95,0x24,0x83,0x16,
	  0xFC,0x19,0xBC,0x95,0x2B,0xD2,0x4A,0x35,0x51,0x83,0x5A,0xC7,0xC5,0xA9,0x5F,0xEC},
	 { 0 },			// Padding
	 "Root-CA00000002",	// Issuer
	},

	// Public key
	{
	 BE32_CONST(RVL_CERT_KEYTYPE_ECC),	// Key type
	 "MS00000003",				// Child certificate identity
	 BE32_CONST(0xFCF5A9BC),		// Unknown...
	 // Modulus
	 {0x00,0x8C,0xF9,0x5D,0x1A,0x80,0xDB,0x01,0xB2,0xFD,0x61,0x68,0xE0,0x80,0x4A,0xAF,
	  0x95,0xEA,0xF2,0x75,0x69,0x6E,0x40,0x84,0x17,0x2C,0xCE,0x74,0x42,0x05,0x00,0x9B,
	  0xE0,0x20,0x0C,0xC9,0x47,0x68,0x49,0x38,0xFB,0x14,0x54,0x35,0x46,0x80,0xAB,0xDB,
	  0xED,0x0B,0xBF,0x49,0xBA,0x82,0xD0,0x8A,0x8C,0xA5,0x71,0xBF},
	 // Padding
	 { 0 }
	}
};

/** Certificate access functions. **/

/**
 * Convert a certificate issuer name to RVT_Cert_Issuer.
 * @param s_issuer Issuer name.
 * @return RVL_Cert_Issuer, or RVL_CERT_ISSUER_UNKNOWN if invalid.
 */
RVL_Cert_Issuer cert_get_issuer_from_name(const char *s_issuer)
{
	unsigned int i;

	if (!s_issuer) {
		errno = EINVAL;
		return RVL_CERT_ISSUER_UNKNOWN;
	} else if (s_issuer[0] == 0) {
		// Root certificate.
		return RVL_CERT_ISSUER_ROOT;
	}

	for (i = RVL_CERT_ISSUER_ROOT; i < RVL_CERT_ISSUER_MAX; i++) {
		if (!strcmp(s_issuer, RVL_Cert_Issuers[i])) {
			// Found a match!
			return (RVL_Cert_Issuer)i;
		}
	}

	// No match.
	errno = ERANGE;
	return RVL_CERT_ISSUER_UNKNOWN;
}

/**
 * Get a standard certificate.
 * @param issuer RVL_Cert_Issuer
 * @return RVL_Cert*, or NULL if invalid.
 */
const RVL_Cert *cert_get(RVL_Cert_Issuer issuer)
{
	static const RVL_Cert *const certs[RVL_CERT_ISSUER_MAX] = {
		NULL,					// RVL_CERT_ISSUER_UNKNOWN
		(const RVL_Cert*)&cert_root,		// RVL_CERT_ISSUER_ROOT

		// Retail
		(const RVL_Cert*)&cert_retail_ca,	// RVL_CERT_ISSUER_RETAIL_CA
		(const RVL_Cert*)&cert_retail_ticket,	// RVL_CERT_ISSUER_RETAIL_TICKET
		(const RVL_Cert*)&cert_retail_tmd,	// RVL_CERT_ISSUER_RETAIL_TMD

		// Debug
		(const RVL_Cert*)&cert_debug_ca,	// RVL_CERT_ISSUER_DEBUG_CA
		(const RVL_Cert*)&cert_debug_ticket,	// RVL_CERT_ISSUER_DEBUG_TICKET
		(const RVL_Cert*)&cert_debug_tmd,	// RVL_CERT_ISSUER_DEBUG_TMD
		(const RVL_Cert*)&cert_debug_dev,	// RVL_CERT_ISSUER_DEBUG_DEV
	};

	assert(issuer > RVL_CERT_ISSUER_UNKNOWN && issuer < RVL_CERT_ISSUER_MAX);
	if (issuer <= RVL_CERT_ISSUER_UNKNOWN || issuer >= RVL_CERT_ISSUER_MAX) {
		errno = ERANGE;
		return NULL;
	}
	return certs[issuer];
}

/**
 * Get a standard certificate by issuer name.
 * @param s_issuer Issuer name.
 * @return RVL_Cert*, or NULL if invalid.
 */
const RVL_Cert *cert_get_from_name(const char *s_issuer)
{
	RVL_Cert_Issuer issuer = cert_get_issuer_from_name(s_issuer);
	if (issuer == RVL_CERT_ISSUER_UNKNOWN) {
		return NULL;
	}
	return cert_get(issuer);
}

/**
 * Get the size of a certificate.
 * @param issuer RVL_Cert_Issuer
 * @return Certificate size, in bytes. (0 on error)
 */
unsigned int cert_get_size(RVL_Cert_Issuer issuer)
{
	static const unsigned int cert_sizes[RVL_CERT_ISSUER_MAX] = {
		0U,						// RVL_CERT_ISSUER_UNKNOWN
		(unsigned int)sizeof(cert_root),		// RVL_CERT_ISSUER_ROOT

		// Retail
		(unsigned int)sizeof(cert_retail_ca),		// RVL_CERT_ISSUER_RETAIL_CA
		(unsigned int)sizeof(cert_retail_ticket),	// RVL_CERT_ISSUER_RETAIL_TICKET
		(unsigned int)sizeof(cert_retail_tmd),		// RVL_CERT_ISSUER_RETAIL_TMD

		// Debug
		(unsigned int)sizeof(cert_debug_ca),		// RVL_CERT_ISSUER_DEBUG_CA
		(unsigned int)sizeof(cert_debug_ticket),	// RVL_CERT_ISSUER_DEBUG_TICKET
		(unsigned int)sizeof(cert_debug_tmd),		// RVL_CERT_ISSUER_DEBUG_TMD
		(unsigned int)sizeof(cert_debug_dev),		// RVL_CERT_ISSUER_DEBUG_DEV
	};

	assert(issuer > RVL_CERT_ISSUER_UNKNOWN && issuer < RVL_CERT_ISSUER_MAX);
	if (issuer <= RVL_CERT_ISSUER_UNKNOWN || issuer >= RVL_CERT_ISSUER_MAX) {
		errno = ERANGE;
		return 0;
	}
	return cert_sizes[issuer];
}
