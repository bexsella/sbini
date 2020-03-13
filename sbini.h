/**
* sbini -- a general purpose ini file reader/writer
* This file is released into the public domain
*
* Written by Saxon Bell
*/

#ifndef _SBINI_H_
#define _SBINI_H_

#ifndef SBINI_MAX_KEY_VALUE_LENGTH
#define SBINI_MAX_KEY_VALUE_LENGTH 256
#endif

#ifndef SBINI_MAX_LINE_LENGTH
#define SBINI_MAX_LINE_LENGTH 1024
#endif

typedef struct sbini_item_t
{
  char key[SBINI_MAX_KEY_VALUE_LENGTH];
  char value[SBINI_MAX_KEY_VALUE_LENGTH];

  int string_val;

  struct sbini_item_t *next;
} sbini_item_t;

typedef struct sbini_group_t
{
  char name[SBINI_MAX_KEY_VALUE_LENGTH];
  sbini_item_t *head;
  int item_count;

  struct sbini_group_t *next;
} sbini_group_t;

typedef struct 
{
  sbini_group_t *head;
  int group_count;
} sbini_t;

#ifdef __cplusplus
extern "C" {
#endif

sbini_t *sbini_new (void);
sbini_t *sbini_load (const char *file_path);
int sbini_save (sbini_t *ini, const char *file_path);
void sbini_free (sbini_t *ini);

int sbini_get_int (sbini_t *ini, const char *group, const char *key);
float sbini_get_float (sbini_t *ini, const char *group, const char *key);
const char *sbini_get_string (sbini_t *ini, const char *group, const char *key);
int sbini_get_boolean (sbini_t *ini, const char *group, const char *key);

int sbini_set_float (sbini_t *ini, const char *group_name, const char *key, const float value);
int sbini_set_boolean (sbini_t *ini, const char *group_name, const char *key, const int value);
int sbini_set_int (sbini_t *ini, const char *group_name, const char *key, const int value);
int sbini_set_string (sbini_t *ini, const char *group_name, const char *key, const char *value);

#ifdef __cplusplus
}
#endif

#endif // _SBINI_H_

/*
* This is free and unencumbered software released into the public domain.
*
* Anyone is free to copy, modify, publish, use, compile, sell, or
* distribute this software, either in source code form or as a compiled
* binary, for any purpose, commercial or non-commercial, and by any
* means.
*
* In jurisdictions that recognize copyright laws, the author or authors
* of this software dedicate any and all copyright interest in the
* software to the public domain. We make this dedication for the benefit
* of the public at large and to the detriment of our heirs and
* successors. We intend this dedication to be an overt act of
* relinquishment in perpetuity of all present and future rights to this
* software under copyright law.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
* OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*
* For more information, please refer to <http://unlicense.org/>
*/
