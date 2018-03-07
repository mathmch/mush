#include <stdlib.h>
#include "util.h"

/* count the number of occurrences of the char in the string */
int count_occurrences(char *s, char wanted) {
    int count;
    if (s == NULL)
        return 0;
    for (count = 0; *s; s++)
        if (*s == wanted)
            count++;
    return count;
}
