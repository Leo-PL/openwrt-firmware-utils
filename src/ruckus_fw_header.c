/*
 * rucks_fw_header.c - partially based on OpenWrt's xorimage.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include "md5.h"

#define BUF_SIZE                0x200
static int data_size;

static int get_file_size(char *name)
{
	struct stat st;
	int res;

	res = stat(name, &st);
	if (res){
		fprintf(stderr,"stat failed on %s", name);
		return -1;
	}

	return st.st_size;
}

static int md5_file(const char *filename, uint8_t *dst)
{
        FILE *fp_src;
        MD5_CTX ctx;
        char buf[BUF_SIZE];
        size_t bytes_read;

        MD5_Init(&ctx);

        fp_src = fopen(filename, "r+b");
        if (!fp_src) {
                return -1;
        }
        while (!feof(fp_src)) {
                bytes_read = fread(&buf, 1, BUF_SIZE, fp_src);
                MD5_Update(&ctx, &buf, bytes_read);
        }
        fclose(fp_src);

        MD5_Final(dst, &ctx);

        return 0;
}

typedef struct __attribute__((packed)) ruckus_fw_header_v3 {
	uint8_t header_magic[4];
	uint32_t next_image_offset;
	uint16_t header_len;
	uint8_t compression_type[2];
	uint32_t entry_point;
	uint32_t data_size;
	uint32_t timestamp;
	uint8_t md5sum[16];
	uint16_t header_version;
	uint16_t header_checksum;
	uint8_t version[16];
	uint8_t architecture;
	uint8_t chipset;
	uint8_t padding_1[34];
	uint8_t product[6];
	uint8_t padding_2[6];
	uint8_t version_2[16];
	uint32_t board_class;
	uint32_t load_address;
	uint8_t padding_3[28];
};


int write_header(FILE *out, char* md5sum){
	struct ruckus_fw_header_v3 header = {
		.header_magic = "RCKS",
		.next_image_offset = htonl(0x130000),
		.header_len = htons(0xE0),
		.compression_type = "l7",
		.entry_point = htonl(0x80060000),
		.data_size = htonl(data_size),
		.timestamp = htonl(0x609a43d2),
		.header_version = htons(3),
		.header_checksum = htons(0x67bb),
		.version = "OPENWRT123456789",
		.architecture = 1,
		.chipset = 1,
		.padding_1 = { 0 },
		.product = "zf7752",
	        .padding_2 = { 0 },
		.version_2 = "OPENWRT123456789",
		.board_class = htonl(3),
		.load_address = htonl(0x80060000),
		.padding_3 = { 0 }
	};

	memcpy(header.md5sum, md5sum, sizeof(header.md5sum));

	if (!fwrite(&header, sizeof(header), 1, out)) {
		fprintf(stderr, "fwrite error\n");
		return EXIT_FAILURE;
	}
	/*
	// lzma kernel offset is ruckus header + uimage header = 160 + 64 = 224 = 0xE0
	char header1[12] = {
		0x52, 0x43, 0x4b, 0x53, 0x00, 0x13, 0x00, 0x00, 0x00, 0xe0, 0x6c, 0x37 };
	char load_address[4] = {	
		0x80, 0x06, 0x00, 0x00 };	
	char fakesize[4] = {
		0x01, 0x76, 0xd7, 0x5c };
	char header_postsize[4] = {
		0x60, 0x9a, 0x43, 0xd2 };
	char fakemd5[16] = {
		0x39, 0x17, 0x73, 0x56, 0xed, 0x87, 0x33, 0x9a, 0xe3, 0xe4, 0xff, 0xc9,
		0xee, 0xcd, 0xd2, 0x9c };
	char header2[4] = {
		0x00, 0x03, 0x08, 0x9A };
	//200.7.10.202.9 - OPENWRT12345678
	char fake_version[16] = {
		0x4f, 0x50, 0x45, 0x4e, 0x57, 0x52, 0x54, 0x31, 0x32, 0x33, 0x34, 0x35, 
		0x36, 0x37, 0x38, 0x39 };
	char header3[48] = {
		0x00, 0x01, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x7a, 0x66, 0x37, 0x37, 0x35, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	//200.7.10.202.9 - OPENWRT12345678
	char header4[4] = {	
		0x00, 0x00, 0x00, 0x03 };
	char entry_point[4] = {	
		0x80, 0x06, 0x00, 0x00 };
	char header_postentry[28] = {
	    0x00, 0x00, 0x00, 0x01, 0x01, 0x76, 0xcf, 0x60, 0x00, 0x00, 0x00, 0x00, 
	    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	    0x00, 0x00, 0x00, 0x00 };

	if (!fwrite(header1, sizeof(header1), 1, out)) {
		fprintf(stderr, "fwrite error\n");
		return EXIT_FAILURE;
	}
	
	if (!fwrite(load_address, sizeof(load_address), 1, out)) {
		fprintf(stderr, "fwrite error\n");
		return EXIT_FAILURE;
	}
	
	u_int32_t ds = htonl(data_size);
	fprintf(stdout, "data_size: %d\n", ds);
	if (!fwrite(&ds, sizeof(ds), 1, out)) {
		fprintf(stderr, "fwrite error\n");
		return EXIT_FAILURE;
	}
	
	if (!fwrite(header_postsize, sizeof(header_postsize), 1, out)) {
		fprintf(stderr, "fwrite error\n");
		return EXIT_FAILURE;
	}

	if (!fwrite(md5sum, 0x10, 1, out)) {
		fprintf(stderr, "fwrite error\n");
		return EXIT_FAILURE;
	}

	if (!fwrite(header2, sizeof(header2), 1, out)) {
		fprintf(stderr, "fwrite error\n");
		return EXIT_FAILURE;
	}
	
	if (!fwrite(fake_version, sizeof(fake_version), 1, out)) {
		fprintf(stderr, "fwrite error\n");
		return EXIT_FAILURE;
	}
	
	if (!fwrite(header3, sizeof(header3), 1, out)) {
		fprintf(stderr, "fwrite error\n");
		return EXIT_FAILURE;
	}
	
	if (!fwrite(fake_version, sizeof(fake_version), 1, out)) {
		fprintf(stderr, "fwrite error\n");
		return EXIT_FAILURE;
	}
	
	if (!fwrite(header4, sizeof(header4), 1, out)) {
		fprintf(stderr, "fwrite error\n");
		return EXIT_FAILURE;
	}
	
	if (!fwrite(entry_point, sizeof(entry_point), 1, out)) {
		fprintf(stderr, "fwrite error\n");
		return EXIT_FAILURE;
	}
	
	if (!fwrite(header_postentry, sizeof(header_postentry), 1, out)) {
		fprintf(stderr, "fwrite error\n");
		return EXIT_FAILURE;
	}
	*/
	return 0;
}

void usage(void) __attribute__ (( __noreturn__ ));

void usage(void)
{
	fprintf(stderr, "Usage: ruckus_fw_header [-i infile] [-o outfile] [-d]\n");
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	char buf[1];	/* keep this at 1k or adjust garbage calc below */
	FILE *in = stdin;
	FILE *out = stdout;
	char *ifn = NULL;
	char *ofn = NULL;
	int c;
	int v0, v1, v2;
	size_t n;
	int p_len, p_off = 0;
    uint8_t  md5sum[0x10];

	while ((c = getopt(argc, argv, "i:o:h")) != -1) {
		switch (c) {
			case 'i':
				ifn = optarg;
				break;
			case 'o':
				ofn = optarg;
				break;
			case 'h':
			default:
				usage();
		}
	}
	
	data_size = get_file_size(ifn);

	if (optind != argc || optind == 1) {
		fprintf(stderr, "illegal arg \"%s\"\n", argv[optind]);
		usage();
	}

	if (ifn && md5_file(ifn, md5sum)) {
		fprintf(stderr, "can not get md5sum for \"%s\"\n", ifn);
		usage();
	}

	if (ifn && !(in = fopen(ifn, "r"))) {
		fprintf(stderr, "can not open \"%s\" for reading\n", ifn);
		usage();
	}

	if (ofn && !(out = fopen(ofn, "w"))) {
		fprintf(stderr, "can not open \"%s\" for writing\n", ofn);
		usage();
	}

	if (ofn && (write_header(out, md5sum))) {
		fprintf(stderr, "can not write header to \"%s\"\n", ofn);
		usage();
	}

	while ((n = fread(buf, 1, sizeof(buf), in)) > 0) {
		if (n < sizeof(buf)) {
			if (ferror(in)) {
			FREAD_ERROR:
				fprintf(stderr, "fread error\n");
				return EXIT_FAILURE;
			}
		}

		if (!fwrite(buf, n, 1, out)) {
		FWRITE_ERROR:
			fprintf(stderr, "fwrite error\n");
			return EXIT_FAILURE;
		}
	}

	if (ferror(in)) {
		goto FREAD_ERROR;
	}

	if (fflush(out)) {
		goto FWRITE_ERROR;
	}

	fclose(in);
	fclose(out);

	return EXIT_SUCCESS;
}
