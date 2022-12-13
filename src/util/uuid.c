#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

#define UUID_STR_FORMAT_STD     0
#define UUID_STR_FORMAT_GUID    0
#define UUID_STR_UPPER_CASE     2

#define UUID_STR_LEN            36
#define UUID_BIN_LEN            16

/*
 * UUID - Universally Unique IDentifier - 128 bits unique number.
 *        There are 5 versions and one variant of UUID defined by RFC4122
 *        specification. A UUID contains a set of fields. The set varies
 *        depending on the version of the UUID, as shown below:
 *        - time, MAC address(v1),
 *        - user ID(v2),
 *        - MD5 of name or URL(v3),
 *        - random data(v4),
 *        - SHA-1 of name or URL(v5),
 *
 * Layout of UUID:
 * timestamp - 60-bit: time_low, time_mid, time_hi_and_version
 * version   - 4 bit (bit 4 through 7 of the time_hi_and_version)
 * clock seq - 14 bit: clock_seq_hi_and_reserved, clock_seq_low
 * variant:  - bit 6 and 7 of clock_seq_hi_and_reserved
 * node      - 48 bit
 *
 * source: https://www.ietf.org/rfc/rfc4122.txt
 *
 * UUID binary format (16 bytes):
 *
 * 4B-2B-2B-2B-6B (big endian - network byte order)
 *
 * UUID string is 36 length of characters (36 bytes):
 *
 * 0        9    14   19   24
 * xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
 *    be     be   be   be       be
 *
 * where x is a hexadecimal character. Fields are separated by '-'s.
 * When converting to a binary UUID, le means the field should be converted
 * to little endian and be means it should be converted to big endian.
 *
 * UUID is also used as GUID (Globally Unique Identifier) with the same binary
 * format but it differs in string format like below.
 *
 * GUID:
 * 0        9    14   19   24
 * xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
 *    le     le   le   be       be
 *
 * GUID is used e.g. in GPT (GUID Partition Table) as a partiions unique id.
 */
/*
 * uuid_bin_to_str() - convert big endian binary data to string UUID or GUID.
 *
 * @param uuid_bin:	pointer to binary data of UUID (big endian) [16B]
 * @param uuid_str:	pointer to allocated array for output string [37B]
 * @str_format:		bit 0: 0 - UUID; 1 - GUID
 *			bit 1: 0 - lower case; 2 - upper case
 */
static void uuid_bin_to_str(unsigned char *uuid_bin, char *uuid_str, int str_format){
	const unsigned char uuid_char_order[UUID_BIN_LEN] = {0, 1, 2, 3, 4, 5, 6, 7, 8,
						  9, 10, 11, 12, 13, 14, 15};
	const unsigned char guid_char_order[UUID_BIN_LEN] = {3, 2, 1, 0, 5, 4, 7, 6, 8,
						  9, 10, 11, 12, 13, 14, 15};
	const unsigned char *char_order;
	const char *format;
	int i;

	/*
	 * UUID and GUID bin data - always in big endian:
	 * 4B-2B-2B-2B-6B
	 * be be be be be
	 */
	if (str_format & UUID_STR_FORMAT_GUID)
		char_order = guid_char_order;
	else
		char_order = uuid_char_order;
	if (str_format & UUID_STR_UPPER_CASE)
		format = "%02X";
	else
		format = "%02x";

	for (i = 0; i < 16; i++) {
		sprintf(uuid_str, format, uuid_bin[char_order[i]]);
		uuid_str += 2;
		switch (i) {
		case 3:
		case 5:
		case 7:
		case 9:
			*uuid_str++ = '-';
			break;
		}
	}
}

static uint64_t get_time_ms(void) {
#if defined(_WIN32)
	FILETIME ft;
	long long t;

	GetSystemTimeAsFileTime(&ft);
	t = (((long long)ft.dwHighDateTime) << 32) + ft.dwLowDateTime;
	t -= 116444736000000000;
	t = t / 10000;
	return t;
#else
	return time(NULL);
#endif	
}

/*
 * gen_rand_uuid() - this function generates a random binary UUID version 4.
 *                   In this version all fields beside 4 bits of version and
 *                   2 bits of variant are randomly generated.
 *
 * @param uuid_bin - pointer to allocated array [16B]. Output is in big endian.
*/
static void gen_rand_uuid(unsigned char *uuid_bin){
	unsigned char uuid[16] = {0};
	int* ptr = (int*)uuid;
	int i;

	srand((unsigned int)(get_time_ms() + rand()));

	/* Set all fields randomly */
	for (i = 0; i < 16/sizeof(int); i++)
		ptr[i] = rand();

	/* Set UUID version to 4 --- truly random generation */
	uuid[6] = (uuid[6] & 0x0F) | 0x40;
	/* Set the UUID variant to DCE */
	uuid[8] = (uuid[8] & 0x3F) | 0x80;

	memcpy(uuid_bin, uuid, 16);
}

/*
 * gen_rand_uuid_str() - this function generates UUID v4 (random) in two string
 *                       formats UUID or GUID.
 *
 * @param uuid_str - pointer to allocated array [37B].
 * @param          - uuid output type: UUID - 0, GUID - 1
 */
void gen_rand_uuid_str(char *uuid_str, int str_format){
	unsigned char uuid_bin[UUID_BIN_LEN];

	/* Generate UUID (big endian) */
	gen_rand_uuid(uuid_bin);

	/* Convert UUID bin to UUID or GUID formated STRING  */
	uuid_bin_to_str(uuid_bin, uuid_str, str_format);
}
