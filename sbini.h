/**
* sbini -- a general purpose ini file reader/writer
* This file is released into the public domain
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

int sbini_load (const char *file_path, sbini_t *ini);
int sbini_save (sbini_t *ini, const char *file_path);
void sbini_free (sbini_t *ini);

int sbini_get_int (sbini_t *ini, const char *group, const char *key);
double sbini_get_float (sbini_t *ini, const char *group, const char *key);
const char *sbini_get_string (sbini_t *ini, const char *group, const char *key);
int sbini_get_boolean (sbini_t *ini, const char *group, const char *key);

#ifdef __cplusplus
}
#endif

#endif // _SBINI_H_
