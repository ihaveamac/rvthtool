/***************************************************************************
 * RVT-H Tool (libwiicrypto)                                               *
 * wii_wad.h: Nintendo Wii WAD file data structures.                       *
 *                                                                         *
 * Copyright (c) 2018-2024 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#pragma once

// References:
// - https://wiibrew.org/wiki/WAD_files
// - https://wiibrew.org/wiki/Content.bin

#include <stdint.h>
#include "common.h"

// Wii data structures.
#include "wii_structs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Nintendo Wii WAD file header.
 * Reference: https://wiibrew.org/wiki/WAD_files
 * 
 * All fields are big-endian.
 */
#define WII_WAD_TYPE_Is	0x49730000	// 'Is\0\0'
#define WII_WAD_TYPE_ib	0x69620000	// 'ib\0\0'
#define WII_WAD_TYPE_Bk	0x426B0000	// 'Bk\0\0'
typedef struct _Wii_WAD_Header {
	uint32_t header_size;		// [0x000] Header size. (0x0020)
	uint32_t type;			// [0x004] Type. ('Is\0\0', 'ib\0\0', 'Bk\0\0')
	uint32_t cert_chain_size;	// [0x008] Certificate chain size.
	uint32_t crl_size;		// [0x00C] Certificate revocation list size.
	uint32_t ticket_size;		// [0x010] Ticket size. (0x2A4)
	uint32_t tmd_size;		// [0x014] TMD size.
	uint32_t data_size;		// [0x018] Data size.
	uint32_t meta_size;		// [0x01C] Metadata size.
} Wii_WAD_Header;
ASSERT_STRUCT(Wii_WAD_Header, 32);

/**
 * Nintendo Wii WAD file header.
 * BroadOn WAD format.
 *
 * To identify the BroadOn format, check for an invalid WAD type,
 * then the ticket size.
 *
 * NOTE: Sections are NOT 64-byte aligned in BroadOn WADs!
 *
 * All fields are big-endian.
 */
typedef struct _Wii_WAD_Header_BWF {
	uint32_t header_size;		// [0x000] Header size. (0x0020)
	uint32_t data_offset;		// [0x004] Data offset. (usually 0x1140)
	uint32_t cert_chain_size;	// [0x008] Certificate chain size.
	uint32_t ticket_size;		// [0x00C] Ticket size. (0x2A4)
	uint32_t tmd_size;		// [0x010] TMD size.
	uint32_t meta_size;		// [0x014] Metadata size.
	uint32_t meta_cid;		// [0x018] Metadata content index. (Not present in WAD!)
	uint32_t crl_size;		// [0x01C] Certificate revocation list size.
} Wii_WAD_Header_BWF;
ASSERT_STRUCT(Wii_WAD_Header_BWF, 32);

/**
 * content.bin header.
 *
 * This is the encrypted program data in a WAD file and/or
 * copied to an SD card.
 *
 * This is followed by one of the following:
 * - Wii_IMET_t: IMET header. (most WADs)
 * - Wii_WIBN_Header_t: WIBN header. (DLC WADs)
 *
 * Reference: https://wiibrew.org/wiki/Content.bin
 *
 * All fields are big-endian.
 */
#pragma pack(1)
typedef struct PACKED _Wii_Content_Bin_Header {
	RVL_TitleID_t title_id;		// [0x000] Title ID. (tid-hi is usually 0x00010001)
	uint32_t partB_size;		// [0x008] Size of part B.
	uint8_t md5_header[16];		// [0x00C] MD5 hash of the header.
	uint8_t md5_icon[16];		// [0x01C] MD5 hash of the DECRYPTED icon.
	uint8_t unknown_2[4];		// [0x02C]
	RVL_TitleID_t tid_ref[2];	// [0x030] Title dependencies?
} Wii_Content_Bin_Header;
ASSERT_STRUCT(Wii_Content_Bin_Header, 64);
#pragma pack()

#ifdef __cplusplus
}
#endif
