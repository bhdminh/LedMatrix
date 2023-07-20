#include <converter.h>
#include <stdbool.h>

// The type of a single Unicode codepoint
typedef uint32_t codepoint_t;

// The last codepoint of the Basic Multilingual Plane, which is the part of Unicode that
// UTF-16 can encode without surrogates
#define BMP_END 0xFFFF

// The highest valid Unicode codepoint
#define UNICODE_MAX 0x10FFFF

// The codepoint that is used to replace invalid encodings
#define INVALID_CODEPOINT 0xFFFD

// If a character, masked with GENERIC_SURROGATE_MASK, matches this value, it is a surrogate.
#define GENERIC_SURROGATE_VALUE 0xD800
// The mask to apply to a character before testing it against GENERIC_SURROGATE_VALUE
#define GENERIC_SURROGATE_MASK 0xF800

// If a character, masked with SURROGATE_MASK, matches this value, it is a high surrogate.
#define HIGH_SURROGATE_VALUE 0xD800
// If a character, masked with SURROGATE_MASK, matches this value, it is a low surrogate.
#define LOW_SURROGATE_VALUE 0xDC00
// The mask to apply to a character before testing it against HIGH_SURROGATE_VALUE or LOW_SURROGATE_VALUE
#define SURROGATE_MASK 0xFC00

// The value that is subtracted from a codepoint before encoding it in a surrogate pair
#define SURROGATE_CODEPOINT_OFFSET 0x10000
// A mask that can be applied to a surrogate to extract the codepoint value contained in it
#define SURROGATE_CODEPOINT_MASK 0x03FF
// The number of bits of SURROGATE_CODEPOINT_MASK
#define SURROGATE_CODEPOINT_BITS 10


// The highest codepoint that can be encoded with 1 byte in UTF-8
#define UTF8_1_MAX 0x7F
// The highest codepoint that can be encoded with 2 bytes in UTF-8
#define UTF8_2_MAX 0x7FF
// The highest codepoint that can be encoded with 3 bytes in UTF-8
#define UTF8_3_MAX 0xFFFF
// The highest codepoint that can be encoded with 4 bytes in UTF-8
#define UTF8_4_MAX 0x10FFFF

// If a character, masked with UTF8_CONTINUATION_MASK, matches this value, it is a UTF-8 continuation byte
#define UTF8_CONTINUATION_VALUE 0x80
// The mask to a apply to a character before testing it against UTF8_CONTINUATION_VALUE
#define UTF8_CONTINUATION_MASK 0xC0
// The number of bits of a codepoint that are contained in a UTF-8 continuation byte
#define UTF8_CONTINUATION_CODEPOINT_BITS 6

// Represents a UTF-8 bit pattern that can be set or verified
typedef struct
{
    // The mask that should be applied to the character before testing it
    utf8_t mask;
    // The value that the character should be tested against after applying the mask
    utf8_t value;
} utf8_pattern;

// The patterns for leading bytes of a UTF-8 codepoint encoding
// Each pattern represents the leading byte for a character encoded with N UTF-8 bytes,
// where N is the index + 1
static const utf8_pattern utf8_leading_bytes[] =
{
    { 0x80, 0x00 }, // 0xxxxxxx
    { 0xE0, 0xC0 }, // 110xxxxx
    { 0xF0, 0xE0 }, // 1110xxxx
    { 0xF8, 0xF0 }  // 11110xxx
};

// The number of elements in utf8_leading_bytes
#define UTF8_LEADING_BYTES_LEN 4


// Gets a codepoint from a UTF-16 string
// utf16: The UTF-16 string
// len: The length of the UTF-16 string, in UTF-16 characters
// index:
// A pointer to the current index on the string.
// When the function returns, this will be left at the index of the last character
// that composes the returned codepoint.
// For surrogate pairs, this means the index will be left at the low surrogate.
static codepoint_t decode_utf16(utf16_t const* utf16, size_t len, size_t* index)
{
    utf16_t high = utf16[*index];

    // BMP character
    if ((high & GENERIC_SURROGATE_MASK) != GENERIC_SURROGATE_VALUE)
        return high; 

    // Unmatched low surrogate, invalid
    if ((high & SURROGATE_MASK) != HIGH_SURROGATE_VALUE)
        return INVALID_CODEPOINT;

    // String ended with an unmatched high surrogate, invalid
    if (*index == len - 1)
        return INVALID_CODEPOINT;
    
    utf16_t low = utf16[*index + 1];

    // Unmatched high surrogate, invalid
    if ((low & SURROGATE_MASK) != LOW_SURROGATE_VALUE)
        return INVALID_CODEPOINT;

    // Two correctly matched surrogates, increase index to indicate we've consumed
    // two characters
    (*index)++;

    // The high bits of the codepoint are the value bits of the high surrogate
    // The low bits of the codepoint are the value bits of the low surrogate
    codepoint_t result = high & SURROGATE_CODEPOINT_MASK;
    result <<= SURROGATE_CODEPOINT_BITS;
    result |= low & SURROGATE_CODEPOINT_MASK;
    result += SURROGATE_CODEPOINT_OFFSET;
    
    // And if all else fails, it's valid
    return result;
}

// Calculates the number of UTF-8 characters it would take to encode a codepoint
// The codepoint won't be checked for validity, that should be done beforehand.
static int calculate_utf8_len(codepoint_t codepoint)
{
    // An array with the max values would be more elegant, but a bit too heavy
    // for this common function

    if (codepoint <= UTF8_1_MAX)
        return 1;

    if (codepoint <= UTF8_2_MAX)
        return 2;

    if (codepoint <= UTF8_3_MAX)
        return 3;

    return 4;
}

// Encodes a codepoint in a UTF-8 string.
// The codepoint won't be checked for validity, that should be done beforehand.
//
// codepoint: The codepoint to be encoded.
// utf8: The UTF-8 string
// len: The length of the UTF-8 string, in UTF-8 characters
// index: The first empty index on the string.
//
// return: The number of characters written to the string.
static size_t encode_utf8(codepoint_t codepoint, utf8_t* utf8, size_t len, size_t index)
{
    int size = calculate_utf8_len(codepoint);

    // Not enough space left on the string
    if (index + size > len)
        return 0;

    // Write the continuation bytes in reverse order first
    for (int cont_index = size - 1; cont_index > 0; cont_index--)
    {
        utf8_t cont = codepoint & ~UTF8_CONTINUATION_MASK;
        cont |= UTF8_CONTINUATION_VALUE;

        utf8[index + cont_index] = cont;
        codepoint >>= UTF8_CONTINUATION_CODEPOINT_BITS;
    }

    // Write the leading byte
    utf8_pattern pattern = utf8_leading_bytes[size - 1];

    utf8_t lead = codepoint & ~(pattern.mask);
    lead |= pattern.value;

    utf8[index] = lead;

    return size;
}

size_t utf16_to_utf8(utf16_t const* utf16, size_t utf16_len, utf8_t* utf8, size_t utf8_len)
{
    // The next codepoint that will be written in the UTF-8 string
    // or the size of the required buffer if utf8 is NULL
    size_t utf8_index = 0;

    for (size_t utf16_index = 0; utf16_index < utf16_len; utf16_index++)
    {
        codepoint_t codepoint = decode_utf16(utf16, utf16_len, &utf16_index);

        if (utf8 == NULL)
            utf8_index += calculate_utf8_len(codepoint);
        else
            utf8_index += encode_utf8(codepoint, utf8, utf8_len, utf8_index);
    }

    return utf8_index;
}

// Gets a codepoint from a UTF-8 string
// utf8: The UTF-8 string
// len: The length of the UTF-8 string, in UTF-8 characters
// index:
// A pointer to the current index on the string.
// When the function returns, this will be left at the index of the last character
// that composes the returned codepoint.
// For example, for a 3-byte codepoint, the index will be left at the third character.
static codepoint_t decode_utf8(utf8_t const* utf8, size_t len, size_t* index)
{
    utf8_t leading = utf8[*index];

    // The number of bytes that are used to encode the codepoint
    int encoding_len = 0;
    // The pattern of the leading byte
    utf8_pattern leading_pattern;
    // If the leading byte matches the current leading pattern
    bool matches = false;
    
    do
    {
        encoding_len++;
        leading_pattern = utf8_leading_bytes[encoding_len - 1];

        matches = (leading & leading_pattern.mask) == leading_pattern.value;

    } while (!matches && encoding_len < UTF8_LEADING_BYTES_LEN);

    // Leading byte doesn't match any known pattern, consider it invalid
    if (!matches)
        return INVALID_CODEPOINT;

    codepoint_t codepoint = leading & ~leading_pattern.mask;

    for (int i = 0; i < encoding_len - 1; i++)
    {
        // String ended before all continuation bytes were found
        // Invalid encoding
        if (*index + 1 >= len)
            return INVALID_CODEPOINT;

        utf8_t continuation = utf8[*index + 1];

        // Number of continuation bytes not the same as advertised on the leading byte
        // Invalid encoding
        if ((continuation & UTF8_CONTINUATION_MASK) != UTF8_CONTINUATION_VALUE)
            return INVALID_CODEPOINT;

        codepoint <<= UTF8_CONTINUATION_CODEPOINT_BITS;
        codepoint |= continuation & ~UTF8_CONTINUATION_MASK;

        (*index)++;
    }

    int proper_len = calculate_utf8_len(codepoint);

    // Overlong encoding: too many bytes were used to encode a short codepoint
    // Invalid encoding
    if (proper_len != encoding_len)
        return INVALID_CODEPOINT;

    // Surrogates are invalid Unicode codepoints, and should only be used in UTF-16
    // Invalid encoding
    if (codepoint < BMP_END && (codepoint & GENERIC_SURROGATE_MASK) == GENERIC_SURROGATE_VALUE)
        return INVALID_CODEPOINT;

    // UTF-8 can encode codepoints larger than the Unicode standard allows
    // Invalid encoding
    if (codepoint > UNICODE_MAX)
        return INVALID_CODEPOINT;

    return codepoint;
}

// Calculates the number of UTF-16 characters it would take to encode a codepoint
// The codepoint won't be checked for validity, that should be done beforehand.
static int calculate_utf16_len(codepoint_t codepoint)
{
    if (codepoint <= BMP_END)
        return 1;

    return 2;
}

// Encodes a codepoint in a UTF-16 string.
// The codepoint won't be checked for validity, that should be done beforehand.
//
// codepoint: The codepoint to be encoded.
// utf16: The UTF-16 string
// len: The length of the UTF-16 string, in UTF-16 characters
// index: The first empty index on the string.
//
// return: The number of characters written to the string.
static size_t encode_utf16(codepoint_t codepoint, utf16_t* utf16, size_t len, size_t index)
{
    // Not enough space on the string
    if (index >= len)
        return 0;

    if (codepoint <= BMP_END)
    {
        utf16[index] = codepoint;
        return 1;
    }

    // Not enough space on the string for two surrogates
    if (index + 1 >= len)
        return 0;

    codepoint -= SURROGATE_CODEPOINT_OFFSET;

    utf16_t low = LOW_SURROGATE_VALUE;
    low |= codepoint & SURROGATE_CODEPOINT_MASK;

    codepoint >>= SURROGATE_CODEPOINT_BITS;

    utf16_t high = HIGH_SURROGATE_VALUE;
    high |= codepoint & SURROGATE_CODEPOINT_MASK;

    utf16[index] = high;
    utf16[index + 1] = low;

    return 2;
}


size_t utf8_to_utf16(utf8_t const* utf8, size_t utf8_len, utf16_t* utf16, size_t utf16_len)
{
    // The next codepoint that will be written in the UTF-16 string
    // or the size of the required buffer if utf16 is NULL
    size_t utf16_index = 0;

    for (size_t utf8_index = 0; utf8_index < utf8_len; utf8_index++)
    {
        codepoint_t codepoint = decode_utf8(utf8, utf8_len, &utf8_index);

        if (utf16 == NULL)
            utf16_index += calculate_utf16_len(codepoint);
        else
            utf16_index += encode_utf16(codepoint, utf16, utf16_len, utf16_index);
    }

    return utf16_index;
}


// Calculates the number of UTF-16 characters it would take to encode a codepoint
// The codepoint won't be checked for validity, that should be done beforehand.
static int calculate_viscii_len(codepoint_t codepoint)
{
    if (codepoint <= BMP_END)
        return 1;

    return 1;
}

#define TCVN_CHAR_COUNT          (134)
#if 0
static codepoint_t UNICODE_LUT[TCVN_CHAR_COUNT] = {
    193, 225, 192, 224, 194, 226, 258, 259, 195, 227,
    7844, 7845, 7846, 7847, 7854, 7855, 7856, 7857, 7850, 7851,
    7860, 7861, 7842, 7843, 7848, 7849, 7858, 7859, 7840, 7841,
    7852, 7853, 7862, 7863, 272, 273, 201, 233, 200, 232,
    202, 234, 7868, 7869, 7870, 7871, 7872, 7873, 7876, 7877,
    7866, 7867, 7874, 7875, 7864, 7865, 7878, 7879, 205, 237,
    204, 236, 296, 297, 7880, 7881, 7882, 7883, 211, 243,
    210, 242, 212, 244, 213, 245, 7888, 7889, 7890, 7891,
    7894, 7895, 7886, 7887, 416, 417, 7892, 7893, 7884, 7885,
    7898, 7899, 7900, 7901, 7904, 7905, 7896, 7897, 7902, 7903,
    7906, 7907, 218, 250, 217, 249, 360, 361, 7910, 7911,
    431, 432, 7908, 7909, 7912, 7913, 7914, 7915, 7918, 7919,
    7916, 7917, 7920, 7921, 221, 253, 7922, 7923, 7928, 7929,
    7926, 7927, 7924, 7925,
};

static uint8_t TCVN_LUT[TCVN_CHAR_COUNT] = {
    218, 184, 240, 181, 162, 169, 161, 168, 219, 183,
    186, 202, 193, 199, 195, 190, 205, 187, 191, 201,
    196, 189, 224, 182, 192, 200, 197, 188, 217, 185,
    180, 203, 194, 198, 167, 174, 176, 208, 179, 204,
    163, 170, 177, 207, 157, 213, 160, 210, 158, 212,
    178, 206, 159, 211, 175, 209, 156, 214, 152, 221,
    155, 215, 153, 220, 154, 216, 151, 222, 147, 227,
    150, 223, 164, 171, 148, 226, 142, 232, 145, 229,
    143, 231, 149, 225, 165, 172, 144, 230, 146, 228,
    137, 237, 140, 234, 138, 236, 141, 233, 139, 235,
    136, 238, 132, 243, 135, 239, 133, 242, 134, 241,
    166, 173, 131, 244, 127, 248, 130, 245, 128, 247,
    129, 246, 31, 249, 27, 253, 30, 250, 28, 252,
    29, 251, 26, 254,
};
#endif

static const codepoint_t unicode_viscii_map[TCVN_CHAR_COUNT][2] = {
    { 0x00C0, 0xC0 }, // À
    { 0x00C1, 0xC1 }, // Á
    { 0x00C2, 0xC2 }, // Â
    { 0x00C3, 0xC3 }, // Ã
    { 0x00C8, 0xC8 }, // È
    { 0x00C9, 0xC9 }, // É
    { 0x00CA, 0xCA }, // Ê
    { 0x00CC, 0xCC }, // Ì
    { 0x00CD, 0xCD }, // Í
    { 0x00D2, 0xD2 }, // Ò
    { 0x00D3, 0xD3 }, // Ó
    { 0x00D4, 0xD4 }, // Ô
    { 0x00D5, 0xA0 }, // Õ
    { 0x00D9, 0xD9 }, // Ù
    { 0x00DA, 0xDA }, // Ú
    { 0x00DD, 0xDD }, // Ý
    { 0x00E0, 0xE0 }, // à
    { 0x00E1, 0xE1 }, // á
    { 0x00E2, 0xE2 }, // â
    { 0x00E3, 0xE3 }, // ã
    { 0x00E8, 0xE8 }, // è
    { 0x00E9, 0xE9 }, // é
    { 0x00EA, 0xEA }, // ê
    { 0x00EC, 0xEC }, // ì
    { 0x00ED, 0xED }, // í
    { 0x00F2, 0xF2 }, // ò
    { 0x00F3, 0xF3 }, // ó
    { 0x00F4, 0xF4 }, // ô
    { 0x00F5, 0xF5 }, // õ
    { 0x00F9, 0xF9 }, // ù
    { 0x00FA, 0xFA }, // ú
    { 0x00FD, 0xFD }, // ý
    { 0x0102, 0xC5 }, // Ă
    { 0x0103, 0xE5 }, // ă
    { 0x0110, 0xD0 }, // Đ
    { 0x0111, 0xF0 }, // đ
    { 0x0128, 0xCE }, // Ĩ
    { 0x0129, 0xEE }, // ĩ
    { 0x0168, 0x9D }, // Ũ
    { 0x0169, 0xFB }, // ũ
    { 0x01A0, 0xB4 }, // Ơ
    { 0x01A1, 0xBD }, // ơ
    { 0x01AF, 0xBF }, // Ư
    { 0x01B0, 0xDF }, // ư
    { 0x1EA0, 0x80 }, // Ạ
    { 0x1EA1, 0xD5 }, // ạ
    { 0x1EA2, 0xC4 }, // Ả
    { 0x1EA3, 0xE4 }, // ả
    { 0x1EA4, 0x84 }, // Ấ
    { 0x1EA5, 0xA4 }, // ấ
    { 0x1EA6, 0x85 }, // Ầ
    { 0x1EA7, 0xA5 }, // ầ
    { 0x1EA8, 0x86 }, // Ẩ
    { 0x1EA9, 0xA6 }, // ẩ
    { 0x1EAA, 0x06 }, // Ẫ
    { 0x1EAB, 0xE7 }, // ẫ
    { 0x1EAC, 0x87 }, // Ậ
    { 0x1EAD, 0xA7 }, // ậ
    { 0x1EAE, 0x81 }, // Ắ
    { 0x1EAF, 0xA1 }, // ắ
    { 0x1EB0, 0x82 }, // Ằ
    { 0x1EB1, 0xA2 }, // ằ
    { 0x1EB2, 0x02 }, // Ẳ
    { 0x1EB3, 0xC6 }, // ẳ //
    { 0x1EB4, 0x05 }, // Ẵ
    { 0x1EB5, 0xC7 }, // ẵ //
    { 0x1EB6, 0x83 }, // Ặ
    { 0x1EB7, 0xA3 }, // ặ
    { 0x1EB8, 0x89 }, // Ẹ
    { 0x1EB9, 0xA9 }, // ẹ
    { 0x1EBA, 0xCB }, // Ẻ
    { 0x1EBB, 0xEB }, // ẻ
    { 0x1EBC, 0x88 }, // Ẽ
    { 0x1EBD, 0xA8 }, // ẽ
    { 0x1EBE, 0x8A }, // Ế
    { 0x1EBF, 0xAA }, // ế
    { 0x1EC0, 0x8B }, // Ề
    { 0x1EC1, 0xAB }, // ề
    { 0x1EC2, 0x8C }, // Ể
    { 0x1EC3, 0xAC }, // ể
    { 0x1EC4, 0x8D }, // Ễ
    { 0x1EC5, 0xAD }, // ễ
    { 0x1EC6, 0x8E }, // Ệ
    { 0x1EC7, 0xAE }, // ệ
    { 0x1EC8, 0x9B }, // Ỉ
    { 0x1EC9, 0xEF }, // ỉ
    { 0x1ECA, 0x98 }, // Ị
    { 0x1ECB, 0xB8 }, // ị
    { 0x1ECC, 0x9A }, // Ọ
    { 0x1ECD, 0xF7 }, // ọ
    { 0x1ECE, 0x99 }, // Ỏ
    { 0x1ECF, 0xF6 }, // ỏ
    { 0x1ED0, 0x8F }, // Ố
    { 0x1ED1, 0xAF }, // ố
    { 0x1ED2, 0x90 }, // Ồ
    { 0x1ED3, 0xB0 }, // ồ
    { 0x1ED4, 0x91 }, // Ổ
    { 0x1ED5, 0xB1 }, // ổ
    { 0x1ED6, 0x92 }, // Ỗ
    { 0x1ED7, 0xB2 }, // ỗ
    { 0x1ED8, 0x93 }, // Ộ
    { 0x1ED9, 0xB5 }, // ộ
    { 0x1EDA, 0x95 }, // Ớ
    { 0x1EDB, 0xBE }, // ớ
    { 0x1EDC, 0x96 }, // Ờ
    { 0x1EDD, 0xB6 }, // ờ
    { 0x1EDE, 0x97 }, // Ở
    { 0x1EDF, 0xB7 }, // ở
    { 0x1EE0, 0xB3 }, // Ỡ
    { 0x1EE1, 0xDE }, // ỡ
    { 0x1EE2, 0x94 }, // Ợ
    { 0x1EE3, 0xFE }, // ợ
    { 0x1EE4, 0x9E }, // Ụ
    { 0x1EE5, 0xF8 }, // ụ
    { 0x1EE6, 0x9C }, // Ủ
    { 0x1EE7, 0xFC }, // ủ
    { 0x1EE8, 0xBA }, // Ứ
    { 0x1EE9, 0xD1 }, // ứ
    { 0x1EEA, 0xBB }, // Ừ
    { 0x1EEB, 0xD7 }, // ừ
    { 0x1EEC, 0xBC }, // Ử
    { 0x1EED, 0xD8 }, // ử
    { 0x1EEE, 0xFF }, // Ữ
    { 0x1EEF, 0xE6 }, // ữ
    { 0x1EF0, 0xB9 }, // Ự
    { 0x1EF1, 0xF1 }, // ự
    { 0x1EF2, 0x9F }, // Ỳ
    { 0x1EF3, 0xCF }, // ỳ
    { 0x1EF4, 0x1E }, // Ỵ
    { 0x1EF5, 0xDC }, // ỵ
    { 0x1EF6, 0x14 }, // Ỷ
    { 0x1EF7, 0xD6 }, // ỷ
    { 0x1EF8, 0x19 }, // Ỹ
    { 0x1EF9, 0xDB }, // ỹ
}; 

static const uint8_t viscii_case_map[TCVN_CHAR_COUNT/2][2] = 
{
    { 0xC0 /*À*/ , 0xE0 /*à*/ },
    { 0xC1 /*Á*/ , 0xE1 /*á*/ },
    { 0xC2 /*Â*/ , 0xE2 /*â*/ },
    { 0xC3 /*Ã*/ , 0xE3 /*ã*/ },
    { 0xC8 /*È*/ , 0xE8 /*è*/ },
    { 0xC9 /*É*/ , 0xE9 /*é*/ },
    { 0xCA /*Ê*/ , 0xEA /*ê*/ },
    { 0xCC /*Ì*/ , 0xEC /*ì*/ },
    { 0xCD /*Í*/ , 0xED /*í*/ },
    { 0xD2 /*Ò*/ , 0xF2 /*ò*/ },
    { 0xD3 /*Ó*/ , 0xF3 /*ó*/ },
    { 0xD4 /*Ô*/ , 0xF4 /*ô*/ },
    { 0xA0 /*Õ*/ , 0xF5 /*õ*/ },
    { 0xD9 /*Ù*/ , 0xF9 /*ù*/ },
    { 0xDA /*Ú*/ , 0xFA /*ú*/ },
    { 0xDD /*Ý*/ , 0xFD /*ý*/ },
    { 0xC5 /*Ă*/ , 0xE5 /*ă*/ },
    { 0xD0 /*Đ*/ , 0xF0 /*đ*/ },
    { 0xCE /*Ĩ*/ , 0xEE /*ĩ*/ },
    { 0x9D /*Ũ*/ , 0xFB /*ũ*/ },
    { 0xB4 /*Ơ*/ , 0xBD /*ơ*/ },
    { 0xBF /*Ư*/ , 0xDF /*ư*/ },
    { 0x80 /*Ạ*/ , 0xD5 /*ạ*/ },
    { 0xC4 /*Ả*/ , 0xE4 /*ả*/ },
    { 0x84 /*Ấ*/ , 0xA4 /*ấ*/ },
    { 0x85 /*Ầ*/ , 0xA5 /*ầ*/ },
    { 0x86 /*Ẩ*/ , 0xA6 /*ẩ*/ },
    { 0x06 /*Ẫ*/ , 0xE7 /*ẫ*/ },
    { 0x87 /*Ậ*/ , 0xA7 /*ậ*/ },
    { 0x81 /*Ắ*/ , 0xA1 /*ắ*/ },
    { 0x82 /*Ằ*/ , 0xA2 /*ằ*/ },
    { 0x02 /*Ẳ*/ , 0xC6 /*ẳ*/ },
    { 0x05 /*Ẵ*/ , 0xC7 /*ẵ*/ },
    { 0x83 /*Ặ*/ , 0xA3 /*ặ*/ },
    { 0x89 /*Ẹ*/ , 0xA9 /*ẹ*/ },
    { 0xCB /*Ẻ*/ , 0xEB /*ẻ*/ },
    { 0x88 /*Ẽ*/ , 0xA8 /*ẽ*/ },
    { 0x8A /*Ế*/ , 0xAA /*ế*/ },
    { 0x8B /*Ề*/ , 0xAB /*ề*/ },
    { 0x8C /*Ể*/ , 0xAC /*ể*/ },
    { 0x8D /*Ễ*/ , 0xAD /*ễ*/ },
    { 0x8E /*Ệ*/ , 0xAE /*ệ*/ },
    { 0x9B /*Ỉ*/ , 0xEF /*ỉ*/ },
    { 0x98 /*Ị*/ , 0xB8 /*ị*/ },
    { 0x9A /*Ọ*/ , 0xF7 /*ọ*/ },
    { 0x99 /*Ỏ*/ , 0xF6 /*ỏ*/ },
    { 0x8F /*Ố*/ , 0xAF /*ố*/ },
    { 0x90 /*Ồ*/ , 0xB0 /*ồ*/ },
    { 0x91 /*Ổ*/ , 0xB1 /*ổ*/ },
    { 0x92 /*Ỗ*/ , 0xB2 /*ỗ*/ },
    { 0x93 /*Ộ*/ , 0xB5 /*ộ*/ },
    { 0x95 /*Ớ*/ , 0xBE /*ớ*/ },
    { 0x96 /*Ờ*/ , 0xB6 /*ờ*/ },
    { 0x97 /*Ở*/ , 0xB7 /*ở*/ },
    { 0xB3 /*Ỡ*/ , 0xDE /*ỡ*/ },
    { 0x94 /*Ợ*/ , 0xFE /*ợ*/ },
    { 0x9E /*Ụ*/ , 0xF8 /*ụ*/ },
    { 0x9C /*Ủ*/ , 0xFC /*ủ*/ },
    { 0xBA /*Ứ*/ , 0xD1 /*ứ*/ },
    { 0xBB /*Ừ*/ , 0xD7 /*ừ*/ },
    { 0xBC /*Ử*/ , 0xD8 /*ử*/ },
    { 0xFF /*Ữ*/ , 0xE6 /*ữ*/ },
    { 0xB9 /*Ự*/ , 0xF1 /*ự*/ },
    { 0x9F /*Ỳ*/ , 0xCF /*ỳ*/ },
    { 0x1E /*Ỵ*/ , 0xDC /*ỵ*/ },
    { 0x14 /*Ỷ*/ , 0xD6 /*ỷ*/ },
    { 0x19 /*Ỹ*/ , 0xDB /*ỹ*/ },
}; 

static inline uint8_t viscii_from_unicode(codepoint_t codepoint)
{
    int idx=0;
    for (idx = 0; idx < TCVN_CHAR_COUNT; idx++) {
        if(codepoint == unicode_viscii_map[idx][0]) {
            return unicode_viscii_map[idx][1];
        }
    }
    
    if(codepoint < 0xff) {
        return (uint8_t)codepoint; 
    }
    return '?'; 
}

// Encodes a codepoint in a UTF-16 string.
// The codepoint won't be checked for validity, that should be done beforehand.
//
// codepoint: The codepoint to be encoded.
// utf16: The UTF-16 string
// len: The length of the UTF-16 string, in UTF-16 characters
// index: The first empty index on the string.
//
// return: The number of characters written to the string.
static size_t encode_viscii(codepoint_t codepoint, viscii_t* viscii, size_t len, size_t index)
{
    // Not enough space on the string
    if (index >= len)
        return 0;

    if (codepoint <= BMP_END)
    {
        viscii[index] = viscii_from_unicode(codepoint);
        return 1;
    }

    // Not enough space on the string for two surrogates
    if (index + 1 >= len)
        return 0;

#if 0
    codepoint -= SURROGATE_CODEPOINT_OFFSET;

    utf16_t low = LOW_SURROGATE_VALUE;
    low |= codepoint & SURROGATE_CODEPOINT_MASK;

    codepoint >>= SURROGATE_CODEPOINT_BITS;

    utf16_t high = HIGH_SURROGATE_VALUE;
    high |= codepoint & SURROGATE_CODEPOINT_MASK;

    utf16[index] = high;
    utf16[index + 1] = low;
    return 2; 
#else
    viscii[index] = '?'; 
    return 1;
#endif
}

size_t utf8_to_viscii(utf8_t const* utf8, size_t utf8_len, viscii_t* viscii, size_t viscii_len)
{
    // The next codepoint that will be written in the UTF-16 string
    // or the size of the required buffer if utf16 is NULL
    size_t viscii_index = 0;

    for (size_t utf8_index = 0; utf8_index < utf8_len; utf8_index++)
    {
        codepoint_t codepoint = decode_utf8(utf8, utf8_len, &utf8_index);

        if (viscii == NULL)
            viscii_index += calculate_viscii_len(codepoint);
        else
            viscii_index += encode_viscii(codepoint, viscii, viscii_len, viscii_index);
    }

    return viscii_index;
}

void viscii_uppercase(viscii_t *viscii, size_t len)
{
    int idx = 0;
    int j = 0;  
    for(idx = 0; idx < len; idx++) {
        if(viscii[idx] >= 'a' && viscii[idx] <= 'z') {
            viscii[idx] = viscii[idx] - 'a' + 'A'; 
        }else {
            for(j = 0; j < TCVN_CHAR_COUNT/2; j++) {
                if(viscii[idx] == viscii_case_map[j][1]) {
                    viscii[idx] = viscii_case_map[j][0]; 
                    break; 
                }
            }
        }
    }
}
