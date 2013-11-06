#ifndef BASE64_H
#define BASE64_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void decodeQuantum(unsigned char *, const char *);
size_t Curl_base64_decode(const char *, unsigned char **);
size_t Curl_base64_encode(const char *, size_t, char **);

#endif
