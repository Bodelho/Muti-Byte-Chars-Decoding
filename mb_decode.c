
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>
#include <math.h>					// ceil()
#include <stdarg.h>					// va_start()

#define _TableSize(name)        (sizeof(name) / sizeof((name)[0]))
#define __WCHR(x)		L ## x
#define  _WCHR(x)		__WCHR(x)

#define __UTFS(x)		u8 ## x
#define  _UTFS(x) 		__UTFS(x)

/*
 *----------------------------------------------------------------------------------------
 * MB_CUR_MAX is a run time glibc constant - created from a macro - for the maximum number
 * of  bytes a **string** may have  when the  string is comprised of just one (the longest
 * supported)  multi byte char  plus  the string's  ending  '\0'  (plus one  extra byte?).
 * Currently multi byte chars are NOT longer  than  4 bytes: the very established standard
 * for encoding of multi byte  char prohibits multi byte chars longer  than 4 bytes. Hence
 * MB_OUR_MAX substituting for MB_CUR_MAX in relation to multi byte **chars**, NOT strings
 * of those.
 *----------------------------------------------------------------------------------------
*/

int MB_OUR_MAX;

/*
 *----------------------------------------------------------------------------------------
 * When "MB_CUR_MAX = 1", all "multi byte" encondings are OK according to mbrtowc().
 *
 * ACRONYMS:
 * + MBCS     Multi Byte Character Set
 * + BMP      Basic Multilingual Plane
 * + UTF      Unicode Transformation Format (a family of  character encoding  schemes  for
 *            the Unicode Standard)
 *
 * Supplementary characters are Unicode characters that are located beyond the BMP.
 *----------------------------------------------------------------------------------------
*/

typedef struct {
        char * mbstr;
        char * bits;
        char * charset;
        char * comment_MB_CUR_MAX_N;
} MBSTR_TABLE_t;

  MBSTR_TABLE_t mbstr_table[] = {

        { "\x24",             __UTFS("0010 0100"),
                              __UTFS("ASCII: $"),
                              __UTFS("OK")                                                                         },

        { "\xA1",             __UTFS("1000 0000"),
                              __UTFS("invalid, ISO-8859-1: ¡"),
                              __UTFS("ERR: valid mb chars don't have primary byte's higher bit set followed by 0") },

        { "\xF8",             __UTFS("1111 1000"),
                              __UTFS("invalid"),
                              __UTFS("ERR: valid mb chars aren't longer than 4 bytes")                             },

        { "\xC0",             __UTFS("1100 0000"),
                              __UTFS("invalid"),
                              __UTFS("ERR: 2-byte mb char missing secondary byte")                                 },

        { "\xC0\xB9",         __UTFS("1100 0000 1011 1001"),
                              __UTFS("ASCII: 9"),
                              __UTFS("ERR: ascii encoded as a 2-byte multi byte char")                             },

        { "\xC2\xE1",         __UTFS("1100 0010 1110 0001"),
                              __UTFS("invalid"),
                              __UTFS("ERR: invalid secondary char signature in a 2-byte multi byte char")          },

        { "\xC2\xA1",         __UTFS("1100 0010 1010 0001"),
                              __UTFS("ISO-8859-1: ¡"),
                              __UTFS("OK")                                                                         },

        { "\xE0\xA1\xB0",     __UTFS("1110 0010 1010 0001 1011 0000"),
                              __UTFS("MBCS/UTF: Greek letter Psi"),
                              __UTFS("OK")                                                                         },

        { "\xF0\x9F\x98\x82", __UTFS("1111 0000 1001 1111 1001 1000 1000 0010"),
                              __UTFS("Supplementary: laughing emoji U+1F602"),
                              __UTFS("OK")                                                                         }
};

/*
 *----------------------------------------------------------------------------------------
 *
 *----------------------------------------------------------------------------------------
*/

char * base2(int bytes, unsigned long val) {
    char * str ;
    int    bits;
    int    p   ;
    int    i   ;

    /*
     * Compute how many storage bytes are required:
     * + enforce 8 bits / byte
     * + one "  " byte separator between "bytes - 1" bytes
     * + one " " nibble separator in every byte
     * + at least one full byte
     * N = (8 * bytes) + 2 * (bytes - 1) + bytes
     *
     * If "bytes" (count) not provided, compute it based on "val".
     * + log(val) / log(2) = minimum # of bits taken by "val"
    */

        bits   = 8  *  bytes;
    if (bytes <= 0)
        bits   =  (                  val    >     0        ? 
                   8 * ceil(ceil(log(val+1) / log(2)) / 8) : 
                                                        8 );

                 p  = 3 * (bits / 8) + bits - 1;	// one extra for '\0'
    str = calloc(p--, sizeof(char));
    if (str == NULL) return(str);

           i = bits;	bits = 0;
    while (i > 0 ) {

         *(str + --p) = '0' + (val & 1);
         val >>= 1;

         if (--i && (++bits == 8)) {
             bits = 0;					// byte complete, separate
             *(str + --p) = ' '; 
             *(str + --p) = ' '; 
         } else {
             if (i && (bits == 4)) {
                 *(str + --p) = ' ';			// nibble complete, separate
             }
         }
    }
    return(str);
}

/*
 *----------------------------------------------------------------------------------------
 * Compute hex & binary representations of the given string.
 *----------------------------------------------------------------------------------------
*/

static char * hex_str;
static bool   hex_str_allocd;
static char * bin_str;
static bool   bin_str_allocd;

void str_2_hex_bin(int len, char * str) {
    char * bin_byte;
    int    i, j;
    /*
     * Compute multi byte char's hex representations.
    */
    if ((hex_str = calloc((2 * len + 1), sizeof(char)))) {
              
               i = 0;  
         for  (j = 0; j < len; j++) {

               sprintf(&hex_str[i], "%02X", (uint8_t) str[j]);
               i += 2;
         }
         hex_str_allocd = true;

    } else {
         hex_str        = "N/A";
         hex_str_allocd = false;
    } // >>> if (hex_str)
    /*
     * Compute multi byte char's binary representations.
    */
    if ((bin_str = calloc((10 * len + 1), sizeof(char)))) {
              
               i = 0;  
         for  (j = 0; j < len; j++) {

               if ((bin_byte = base2(1, str[j]))) {

                   sprintf(&bin_str[i], "%s ", bin_byte);
                   i += 10;
                   free(bin_byte);
               }
         }
         bin_str_allocd = true;

    } else {
         bin_str        = "N/A ";
         bin_str_allocd =  false;
    } // >>> if (bin_str)
    if ((i = strlen(bin_str)))
        bin_str[i - 1] = '\0';
}

/*
 *----------------------------------------------------------------------------------------
 * Stack of "left margin" strings used in nested messages.
 *----------------------------------------------------------------------------------------
*/

#define          _MAX_LM   (9)
static char * LM[_MAX_LM];
static char   lm;

void push_lm(char * margin) {
     int i = lm + 1;

     if (lm < _MAX_LM) {
                       LM[i] = malloc((strlen(LM[lm]) + strlen(margin) + 1) * sizeof(char));
         (void) strcpy(LM[i],  LM[lm++]);
         (void) strcat(LM[i],  margin  );
     }
}

void pop_lm(void) {
     if (lm > 0)
         free(LM[lm--]);
}

/*
 *----------------------------------------------------------------------------------------
 * 
 *----------------------------------------------------------------------------------------
*/

void lm_printf(char *fmt, ...) {
    char  * newfmt;
    va_list args;

    if ((newfmt = malloc((strlen(fmt) + strlen(LM[lm]) + 1) * sizeof(char)))) {

        (void) strcpy(newfmt, LM[lm]);
        (void) strcat(newfmt, fmt   );

        va_start(args, fmt);
        fmt = newfmt;
        vprintf(fmt, args);
        va_end(args);
        free(newfmt);
    }
}

/*
 *----------------------------------------------------------------------------------------
 *   MAIN - MAIN - MAIN - MAIN - MAIN - MAIN - MAIN - MAIN - MAIN - MAIN - MAIN - MAIN 
 *----------------------------------------------------------------------------------------
*/

int main(void) {
    bool         lc_ctype = true;			// set LC_CTYPE?
    int          i;
    char       * jump_lines;
    const char * locale;
    int          mbstr_len;
    char         mbstr[8];
    wchar_t      wcstr[2] = L" ";

    printf("\n");
    lm = 0;
    LM[lm] = "  ";

    /*
     * -----------------------------------------------------------------------------------
     * Make sure LC_CTYPE is  set correctly, it is pivotal for glibc's correct handling of
     * multi byte chars and wide chars, including conversions between those char types.
     * -----------------------------------------------------------------------------------
    */

    if (lc_ctype) {
        if ((locale = setlocale(LC_CTYPE, NULL)) != NULL) {
            /*
             * "C" and "POSIX" locales set MB_CUR_MAX to 1; all  invalid  multi byte chars
             * are regarded as valid 1-byte multi byte chars even if not so.
            */
            if (((strstr(locale, "C"    ) != NULL)   ||
                 (strstr(locale, "POSIX") != NULL)))
                 locale = NULL;
        }
        if (!locale) {
            /*
             * LC_CTYPE unset or defaulting to "C" or "POSIX".
            */
            if ((locale = setlocale(LC_ALL, NULL))) {

                if (((strstr(locale, "C"    ) != NULL)   ||
                     (strstr(locale, "POSIX") != NULL)))
                     locale = NULL;
            }
        } // >>> if (!locale)
        if (!locale) {
            /*
             * Failed to use LC_ALL as locale setting for LC_CTYPE.
             * Assumption: LANG is valid if set.
            */
            if (locale = getenv("LANG")) {
                /*
                 * Failed to use LANG as locale setting for LC_CTYPE.
                 * Last resort: use built-in default
                */
                locale = "en_US.UTF-8";
            }
        } // >>> if (!locale)
        if (!(locale = setlocale(LC_CTYPE, locale)))
            lm_printf("WARNING: failed to (re)set LC_TYPE\n\n");
    } // >>> if (lc)ctype)

    locale = setlocale(LC_CTYPE, NULL);
    lm_printf("LC_CTYPE   = %s\n", locale    );
    lm_printf("MB_CUR_MAX = %d\n", MB_CUR_MAX);
    /*
     * -----------------------------------------------------------------------------------
     * Settle MB_OUR_MAX
     * -----------------------------------------------------------------------------------
    */
    MB_OUR_MAX = MB_CUR_MAX > 4 ? 4 : MB_CUR_MAX;
    lm_printf("MB_OUR_MAX = %d\n", MB_OUR_MAX);
    /*
     * -----------------------------------------------------------------------------------
     * Codes between 128 and 255 define the same characters as in the ISO-8859-1 character
     * set. UTF-8 encodes the ISO-8859-1 character set as  double-byte sequences. Although
     * mbstr_table[].mbstr is a string,  the string  is  meant  to hold just ONE (encoded)
     * multi byte char; if a mbstr_table[].mbstr string contains  more than one multi byte
     * char, just the first one is processed (as it happens when "MB_CUR_MAX = 1").
     * -----------------------------------------------------------------------------------
    */
    printf("\n");
    jump_lines = NULL;

    for (i = 0; i < _TableSize(mbstr_table); i++) {
         int         j, k, l, n;
         char      * msg;
         uint8_t     byte;
         int         bits;
         int         mbch_bit;
         int         mbch_len;
         uint32_t    mbch_value;
         mbstate_t   state;

         if (jump_lines)
             printf("%s", jump_lines);
                          jump_lines = "\n\n";

         (void) strcpy(mbstr, mbstr_table[i].mbstr);
         mbstr_len = strlen(mbstr);
         mbstr_len = mbstr_len > MB_OUR_MAX ? MB_OUR_MAX : mbstr_len;
         mbstr[mbstr_len] = '\0';
         /*
          * ------------------------------------------------------------------------------
          * Multi byte char report: header.
          * ------------------------------------------------------------------------------
         */
         str_2_hex_bin(strlen(mbstr), mbstr);
         lm_printf("Parsing multi byte '%s'   0x%-8s  %s\n", mbstr, hex_str, bin_str);

         if (hex_str_allocd) free(hex_str);
         if (bin_str_allocd) free(bin_str);
         push_lm("    ");
         lm_printf("+ %s\n", mbstr_table[i].charset);
         /*
          * ------------------------------------------------------------------------------
          * Multi byte char mbrtowc() report.
          * ------------------------------------------------------------------------------
         */
         memset(&state, 0, sizeof(state));
         if   ((n = mbrtowc(&wcstr[0], &mbstr[0], MB_OUR_MAX, &state)) < 0)
               msg = "invalid";
         else  msg =   "valid";
         lm_printf("mbrtowc() reports multi byte as %s (%d)\n", msg, n);

         if (n >= 0) {
             /*
              * Valid multi byte char, mbrtowc() produced something in wcstr: report it.
              * msg is used as a temp, auxiliary variable.
             */
             msg = (char *) &wcstr[0];
             str_2_hex_bin(sizeof(wcstr) - sizeof(wchar_t), msg);
             lm_printf("mbrtowc() produced wide char '%lc'  0x%s  %s\n", wcstr[0], hex_str, bin_str);
    
             if (hex_str_allocd) free(hex_str);
             if (bin_str_allocd) free(bin_str);
         }
         /*
          * ------------------------------------------------------------------------------
          * Multi byte char own parsing report.
          * ------------------------------------------------------------------------------
         */
         /*
          * Parse multi byte char's 1st / primary char.
         */
         mbch_len = 0;
         bits = 8;
         byte = (uint8_t) mbstr[0];
     
         while ((byte & 0x80) && bits--) {
                byte = byte << 1;
                mbch_len++;
         }
         mbch_value = byte >> mbch_len;			// actual char bits in 1st byte

         if ((mbch_len == 1) && (MB_OUR_MAX > 1)) {

             lm_printf("1st byte in multi byte char can not have its highest order bit set\n");
             lm_printf("ISO-8859-1 (0x80..0xFF) chars must be encoded as a 2-byte multi byte char\n");
             msg = "Multi byte char parsed as invalid: %s\n";

             if (n < 0) {
                 /*
                  * Trust mbrtowc() and don't go any further.
                 */
                 lm_printf(msg, "mbrtowc() agrees");
                 lm_printf("*ERR*\n");
                 pop_lm();
                 continue;

             } else {
                 lm_printf(msg, "mbrtowc() DISAGREES");
                 lm_printf("Keeping on parsing anyways ...\n");
             }
         }
         if ((mbch_len > MB_OUR_MAX) && (MB_OUR_MAX > 1)) {

             lm_printf("1st byte in multi byte char states an oversized length (%d)\n", mbch_len);
             msg = "Multi byte char parsed as invalid: %s\n";

             if (n < 0) {
                 /*
                  * Trust mbrtowc() and don't go any further.
                 */
                 lm_printf(msg, "mbrtowc() agrees");
                 lm_printf("*ERR*\n");
                 pop_lm();
                 continue;

             } else {
                 lm_printf(msg, "mbrtowc() DISAGREES");
                 lm_printf("Keeping on parsing anyways ...\n");
             }
         }
         mbch_len = mbch_len ? mbch_len : 1;
     
         if ((mbch_len > 1) && (MB_OUR_MAX > 1)) {
             if (mbch_len > mbstr_len) {

                 lm_printf("Incomplete %d-byte long multi byte char, have only %d byte(s)\n", mbch_len, mbstr_len);
                 msg = "Multi byte char parsed as invalid: %s\n";

                 if (n < 0) {
                     /*
                      * Trust mbrtowc() and don't go any further.
                     */
                     lm_printf(msg, "mbrtowc() agrees");
                     lm_printf("*ERR*\n");
                     pop_lm();
                     continue;

                 } else {
                     lm_printf(msg, "mbrtowc() DISAGREES");
                     lm_printf("Keeping on parsing anyways ...\n");
                 }
             } else { // if (mbch_len > mbstr_len)

                 lm_printf("Found a %d-byte long multi byte char\n", mbch_len);
                 msg = "Multi byte char's 1st byte parsed as valid: %s\n";

                 if (n < 0) {
                     /*
                      * Need  to parse secondary bytes, value before stating it is invalid
                      * as per mbrtowc().
                     */
                     (void) n;

                 } else {
                     if (n != mbch_len) {
                         /*
                          * This would be an internal error?
                         */
                         lm_printf("mbrtowc() reported a %d-byte long multi byte char, parsed as %d-byte long instead\n",
                                   n, mbch_len);

                         push_lm("          ");
                         lm_printf("WNG: Relying on the byte count returned by mbrtowc() to skip parsed chars\n");
                         lm_printf("     from a multi byte string will lead to serious errors in cases like this\n");
                         pop_lm();
                         lm_printf("Keeping on parsing anyways ...\n");
                     }
                 }

             } // >>> else (mbch_len > mbstr_len)

         } else  {  // >>> if ((mbch_len > 1)  && (MB_OUR_MAX > 1))
             /*
              * mbrtowc() can not possibly disagree.
             */
             lm_printf("Multi byte char is a standard, single byte, valid char\n");
             lm_printf("OK\n");
             pop_lm();
             continue;
         } /// else ((mbch_len > 1) && (MB_OUR_MAX > 1))
         /*
          * Parse multi byte char'secondary chars.
         */
                  j = 0;
         while (++j < mbstr_len) {

              if ((mbstr[j] & 0xC0) != 0x80) {
                  /*
                   * Invalid secondary byte for a multi byte char.
                  */
                  mbstr[j + 1] = '\0';
                  str_2_hex_bin(1, &mbstr[j]);
                  lm_printf("Not found multi byte signature 'x80' on secondary char (%d): %s  %s\n", j + 1, hex_str, bin_str);

                  if (hex_str_allocd) free(hex_str);
                  if (bin_str_allocd) free(bin_str);

                  msg = "Multi byte char parsed as invalid: %s\n";

                  if   (n < 0)
                        lm_printf(msg, "mbrtowc() agrees"); 
                  else  lm_printf(msg, "mbrtowc() DISAGREES");
                        lm_printf("*ERR*\n");

                  pop_lm();				// don't go any further
                  j = mbstr_len;			// tell outter loop to end
                  continue;

              } // >>> if ((mbstr[j] & 0xC0) != 0x80)

              mbch_value = (mbch_value << 6) | (mbstr[j] & 0x3F);

         } // >>> while (++j < mbstr_len)
         if (j == (mbstr_len + 1))			// inner loop requested this
             continue;
         /*
          * Report the multi byte char's computed value.
         */
         bin_str = base2(0, mbch_value);
         lm_printf("Got multi byte char's value: 0x%0X  %s\n", mbch_value, bin_str);
         free(bin_str);
         /*
          * Analyze multi byte char's computed value.
         */
         if (mbch_value < 128) {			// vanila ascii

             if (mbch_len == 1) {

                 lm_printf("Vanilla ASCII (0x01..0x7F) is an 1-byte multi byte char\n");
                 msg = "Multi byte char parsed as valid: %s\n";
                 if   (n < 0)
                       lm_printf(msg, "mbrtowc() DISAGREES"); 
                 else  lm_printf(msg, "mbrtowc() agrees");
                       lm_printf("OK\n");

             } else {

                 lm_printf("Vanilla ASCII (0x01..0x7F) not encoded as an 1-byte multi byte char\n");
                 msg = "Multi byte char parsed as invalid: %s\n";
                 if   (n < 0)
                       lm_printf(msg, "mbrtowc() agress"); 
                 else  lm_printf(msg, "mbrtowc() DISAGREES");
                       lm_printf("*ERR*\n");

             } // >>> else (mbch_len == 1)

         } else { // >>> if (mbch_value < 128)

             if (mbch_value < 256) {			// ISO-8859-1

                 if (mbch_len == 2) {
    
                     lm_printf("ISO-8859-1 char (0x80..0xFF) is a 2-byte multi byte char\n");
                     msg = "Multi byte char parsed as valid: %s\n";
                     if   (n < 0)
                           lm_printf(msg, "mbrtowc() DISAGREES"); 
                     else  lm_printf(msg, "mbrtowc() agrees");
                           lm_printf("OK\n");
    
                 } else {
    
                     lm_printf("ISO-8859-1 char (0x80..0xFF) not encoded as a 2-byte multi byte char\n");
                     msg = "Multi byte char parsed as invalid: %s\n";
                     if   (n < 0)
                           lm_printf(msg, "mbrtowc() agress"); 
                     else  lm_printf(msg, "mbrtowc() DISAGREES");
                           lm_printf("*ERR*\n");
    
                 } // >>> else (mbch_len == 2)

              } else {
                 /*
                  * Value  can  not  possibly be  stored in  less than 3 bytes, no need to
                  * check mbch_len.
                 */
                 int bits = ceil(log(mbch_value) / log(2));

                 lm_printf("MBCS/UTF char (0x0001...) is a %d-byte multi byte char\n", mbch_len);
                 msg = "Multi byte char parsed as valid: %s\n";
                 if   (n < 0)
                       lm_printf(msg, "mbrtowc() DISAGREES"); 
                 else  lm_printf(msg, "mbrtowc() agrees");

                 if (bits > (8 * sizeof(wchar_t))) {
                     lm_printf("WNG: sizeof(wchar_t) = %d bits\n", 8 * sizeof(wchar_t));
                     lm_printf("     multi byte char's value needs %d bits\n", bits);
                 }
                 lm_printf("OK\n");

              } // >>> else(mbch_value < 256)

         } // >>> else (mbch_value < 128)
         /*
          * Done.
         */
         pop_lm();

    } // >>> for (i = 0; i < _TableSize(mbstr_table); i++)
    printf("\n");
    /*
     * -----------------------------------------------------------------------------------
     * Done.
     * -----------------------------------------------------------------------------------
    */
    exit(0);
}
//----------------------------------------------------------------------------------------
//       1         2         3         4         5         6         7         8         9
//3456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789
//----------------------------------------------------------------------------------------
/*
 *****************************************************************************************
 *
 * Glibc's  MB_CUR_MAX  is  NOT  a   constant,  it  is   a  macro   yielding  a  run time,
 * locale-dependent value when code using glibc  starts running. MB_CUR_MAX depends on the
 * "LC_CTYPE"  locale  category's  setting:  if  "LC_CTYPE"  is  not  set,  glibc defaults
 * MB_CUR_MAX  to 1 (one). Otherwise MB_CUR_MAX is set to a proper value  according to the
 * current "LC_TYPE". For instance, when "LC_CTYPE" is  equal to "en_US.UTF-8", MB_CUR_MAX
 * is set  to 6. NOTE: LC_CTYPE's locale category applies to classification and conversion
 * of characters, and to multi byte and wide characters.
 *
 * Having MB_CUR_MAX correctly set is pivotal for some glibc functions to work correctly -
 * e.g.,  mbrtowc(),  wcrtomb(),  printf(),  wprinf(). An  incorrect  MB_CUR_MAX can cause
 * failure  in  char types  explicit  conversions,  as  in  mbrtowc();  ditto  for implict
 * conversions, as in printf() functions family's "%lc" / "%ls" format strings. Worse yet,
 * when  a wchar_t - which  can not be converted (as  per  MB_CUR_MAX) - is fed to "%lc" /
 * "%ls" format strings in  a  wprintf() statement, the code silently crashes; printf() on
 * the other hand just silently outputs nothing instead of making the code crash.
 *
 *
 *****************************************************************************************
 *  
 * Unicode
 *  
 * Unicode is  a 32-bit  encoding  scheme.  It  packs  most  international characters into
 * wide-character  representations (two  bytes per character). Codes below 128  define the
 * same characters  as  the ASCII standard.  Codes  between  128 and  255  define the same
 * characters as in the ISO-8859-1  character set. There's  a private-use area from 0xE000
 * to 0xF7FF. For more information about Unicode, see the Unicode  Consortium's website at
 * https://www.unicode.org.
 *  
 *  
 * UTF-8 encoding
 *  
 * Formerly known as UTF-2, the UTF-8 (for "8-bit form") transformation format is designed
 * to address the  use of Unicode character data in 8-bit  UNIX environments. Each Unicode
 * value is encoded as a multi byte UTF-8 sequence.
 *  
 * Here are some of the main features of UTF-8:
 * + The UTF-8 representation of codes below 128 is the same as  in the ASCII standard, so
 *   any ASCII string is also a valid UTF-8 string and represents the same characters.
 * + ASCII  values  don't  otherwise  occur  in a  UTF-8  transformation,  giving complete
 *   compatibility with historical filesystems that parse for ASCII bytes.
 * + UTF-8 encodes the ISO-8859-1 character set as double-byte sequences.
 * + UTF-8 simplifies conversions to and from Unicode text.
 * + The first  byte  indicates the  number  of bytes to follow in a multi  byte sequence,
 *   allowing for efficient forward parsing.
 * + UTF-8 is reasonably compact in terms of the number of bytes used for encoding.
 * + Finding the  start  of  a character from an  arbitrary  location  in a byte stream is
 *   efficient, because you need to search at most four bytes backwards to  find an easily
 *   recognizable initial byte. For example:
 *                                                isInitialByte = ((byte & 0xC0) != 0x80);
 *  
 * The actual encoding is this:
 * + For multi byte encodings, the first byte  sets 1 in a number of high-order bits equal
 *   to the number of bytes  used in the encoding; the bit  after  that  is set to  0. For
 *   example, a 2-byte sequence always starts with 110 in the first byte.
 * + For all  subsequent  bytes  in a  multi byte encoding, the first two bits are 10. The
 *   value of a trailing byte in a multi byte encoding is always  greater than or equal to
 *   0x80.
 * + The following  table shows the  binary  form of each  byte of  the  encoding  and the
 *   minimum and maximum values for the  characters represented by 1-, 2-,  3-, and 4-byte
 *   encodings:
 *  
 *   Length       1st byte   Next bytes   Min value   Max value
 *   ===========  ========   ==========   =========   =========
 *   Single byte  0xxxxxxx   N/A          0x0000      0x007F
 *   Two bytes    110xxxxx   10xxxxxx     0x0080      0x07FF
 *   Three bytes  1110xxxx   10xxxxxx     0x0800      0xFFFF
 *   Four bytes   11110xxx   10xxxxxx     0x10000     0x10FFFF
 *   ===========  ========   ==========   =========   =========
 *  
 * The actual content of the multi byte encoding (i.e. the wide-character encoding) is the
 * catenation of the "xx" bits in the encoding. A 2-byte encoding of "1101 1111 1000 0000"
 * encodes  the wide character "0111 1100 0000". Where there's more than one way to encode
 * a value (such as 0), the shortest is the only legal value. The null character is always
 * a single byte. There  is a  maximum of  21  data bits ("x") in  a  multi  byte encoding
 * (4-byte long), allowing up to  2.097.152 characters.  If a C  Library  (such  as glibc)
 * implements the wchar_t  using 16 bits,  not all  possible multi  byte  encodings can be
 * converted to a wide char.
 *  
 * Note: Multi  byte characters  in the  C  library are UTF-8  in the  default  locale; in
 *       different locales, multi byte characters might use a different encoding.
 *  
 *****************************************************************************************
 *
 *  https://cygwin.com/cygwin-ug-net/setup-locale.html
 *
 *  LANG=ru_RU.UTF-8                       # Russia        accepted
 *  LANG=en_US.UTF-8                       # USA           DEFAULT
 *  LANG=ja_JP.eucJP                       # Japan         broken
 *  LANG=ja_JP.JSIS                        # Japan         rejected
 *  LANG=ja_JP.UTF-8                       # Japan         accepted
 *  LANG=ja_JP                             # Japan         broken
 *  LANG=ja_JP.utf8@cjknarrow              # Japan         accepted
 *  LANG=fr_FR.UTF-8                       # France        accepted
 *  LANG=ko_KR.eucKR                       # Korea         ?
 *  LANG=syr_SY                            # Syria         ?
 *  LANG=ca_ES.ISO-8859-1                  # Catalan       ?
 *  LANG=tr_TR.ISO-8859-9                  # Turkey        ?
 *  LANG=pt_BR.UTF-8                       # Brazil         accepted
 *  LANG=pt_BR                             # Brazil         accepted
 *
 *****************************************************************************************
*/
