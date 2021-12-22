#include "revert_string.h"
#include <string.h>

void RevertString(char *str)
{
    int full_len=strlen(str);
    int half_len=full_len/2;
    char tmp_char;
    for (int it=0; it<half_len; it++) {
        tmp_char=str[it];
        str[it]=str[full_len-1-it];
        str[full_len-1-it]=tmp_char;
    }
	// your code here
}

