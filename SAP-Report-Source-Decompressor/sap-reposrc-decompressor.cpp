//--------------------------------------------------------------------------------------------------
// SAP Source Code Decompressor
//--------------------------------------------------------------------------------------------------
// Written by Daniel Berlin <mail@daniel-berlin.de> - http://www.daniel-berlin.de/
// Inspired by code from Dennis Yurichev <dennis@conus.info> - http://www.yurichev.com/
// This program uses portions of the MaxDB 7.6.00.34 source code, which are licensed under
// the GNU General Public License (see file headers in "lib/" folder).
// Comments, suggestions and questions are welcome!
//--------------------------------------------------------------------------------------------------
// The source code stored in REPOSRC-DATA is basically compressed using the LZH algorithm
// (Lempel-Ziv-Huffman), for details see "lib/vpa108csulzh.cpp".
// Additionally, I had to add some tweaks to get it to work; see comments below.
// The decompressed source code has a fixed line length of 255 characters (blank-padded).
//--------------------------------------------------------------------------------------------------
// To extract the compressed source code from SAP, use the included report "ZS_REPOSRC_DOWNLOAD".
//--------------------------------------------------------------------------------------------------
// Changes:
//   2012-06-23 - 1.0.0 - Initial version
//   2012-06-24 - 1.0.1 - Revert modifications to MaxDB library
//--------------------------------------------------------------------------------------------------

#define VERSION "1.0.1"

// Silence MS Visual C++ ("... consider using fopen_s instead ...")
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "lib/hpa101saptype.h"
#include "lib/hpa106cslzc.h"
#include "lib/hpa107cslzh.h"
#include "lib/hpa104CsObject.h"
#include "lib/hpa105CsObjInt.h"

int main(int argc, char *argv[]) {
	int ret;						// Return value
	FILE *fin, *fout;				// In-/output file handles
	long fin_size;					// Input file size
	SAP_BYTE *bin, *bout;			// In-/output buffers
	short factor = 10;				// Initial output buffer size factor
	class CsObjectInt o;			// Class containing the decompressor
	SAP_INT byte_read, byte_decomp;	// Number of bytes read and decompressed
	int i;							// Loop counter

	printf("\n------------------------------------\n");
	printf("SAP Source Code Decompressor, v%s\n", VERSION);
	printf("------------------------------------\n\n");

	// Check command line parameters
	if (argc != 3 || argv[1] == NULL || argv[2] == NULL) {
		printf("Usage: %s <infile> <outfile>\n\n", argv[0]);
		return 0;
	}

	// Open input file
	fin = fopen(argv[1], "rb");
	if (fin == NULL) {
		printf("Error: Failed to open input file '%s'\n", argv[1]);
		return 1;
	}

	// Determine size of input file
	ret = fseek(fin, 0, SEEK_END);
	assert(ret == 0);
	fin_size = ftell(fin);
	assert(fin_size != -1L);

	// Rewind to beginning of input file
	ret = fseek(fin, 0, SEEK_SET);
	assert(ret == 0);

	// Create and populate input buffer
	bin = (SAP_BYTE*) malloc(fin_size);
	ret = fread(bin, 1, fin_size, fin);
	assert(ret == fin_size);

	fclose(fin);

	// Create output file
	fout = fopen(argv[2], "wb");
	if (fout == NULL) {
		printf("Error: Failed to create output file '%s'\n", argv[2]);
		return 2;
	}

	printf("Compressed source read from '%s'\n", argv[1]);

	// Determine compression algorithm
	ret = o.CsGetAlgorithm(bin + 1);
	printf("Algorithm: %u (1 = LZC, 2 = LZH)\n", ret);

	for (;;) {
		// Create output buffer with an initial size factor of 10 x input buffer
		bout = (SAP_BYTE*) malloc(fin_size * factor);

		// Perform decompression
		ret = o.CsDecompr(
			bin + 1					// Skip 1st byte (-> strange !)
			, fin_size - 1			// Input size
			, bout					// Output buffer
			, fin_size * factor		// Output buffer size
			, CS_INIT_DECOMPRESS	// Decompression
			, &byte_read			// Bytes read from input buffer
			, &byte_decomp			// Bytes decompressed to output buffer
		);

		// Output buffer too small -> increase size factor and retry
		if(ret == CS_END_OUTBUFFER || ret == CS_E_OUT_BUFFER_LEN) {
			free(bout);
			factor += 10;
			continue;
		}

		free(bin);

		// Handle all other return codes
		switch (ret) {
			case CS_END_OF_STREAM      :
			case CS_END_INBUFFER       : printf("Decompression successful\n"     ); break;
			case CS_E_IN_BUFFER_LEN    : printf("Error: CS_E_IN_BUFFER_LEN\n"    ); return  3;
			case CS_E_MAXBITS_TOO_BIG  : printf("Error: CS_E_MAXBITS_TOO_BIG\n"  ); return  4;
		//	case CS_E_INVALID_LEN      : printf("Error: CS_E_INVALID_LEN\n"      ); return  5;
			case CS_E_FILENOTCOMPRESSED: printf("Error: CS_E_FILENOTCOMPRESSED\n"); return  6;
			case CS_E_IN_EQU_OUT       : printf("Error: CS_E_IN_EQU_OUT\n"       ); return  7;
			case CS_E_INVALID_ADDR     : printf("Error: CS_E_INVALID_ADDR\n"     ); return  8;
			case CS_E_FATAL            : printf("Error: CS_E_FATAL\n"            ); return  9;
			default                    : printf("Error: Unknown status\n"        ); return 10;
		}

		break;
	}

	// Write the buffer to the output file:
	//  - skip the first 2 bytes   (-> strange !)
	//  - only read every 2nd byte (-> strange !)
	//  - replace character 255 with a newline
	for (i = 3; i < byte_decomp; i += 2) {
		if(bout[i] != 255) {
			ret = fwrite(bout + i, 1, 1, fout);
		}
		else {
			ret = fwrite("\n", 1, 1, fout);
		}

		if (ret != 1) {
			printf("Error writing to output file '%s'\n", argv[2]);
			return 11;
		}
	}

	// Add a trailing newline
	fwrite("\n", 1, 1, fout);
	fclose(fout);

	printf("Plain source code written to '%s'\nHave a nice day\n\n", argv[2]);

	free(bout);
	return 0;
}
