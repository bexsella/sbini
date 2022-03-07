/**
* sbini -- a general purpose ini file reader/writer
* This file is released into the public domain
*
* Written by Saxon Bell
*/

#ifndef _SBINI_H_
#define _SBINI_H_

#ifdef __cplusplus
extern "C" {
#endif

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

typedef struct sbini_t
{
  sbini_group_t *head;
  int group_count;
} sbini_t;

sbini_t *sbini_new (void);
sbini_t *sbini_load (const char *file_path);
int sbini_save (sbini_t *ini, const char *file_path);
void sbini_free (sbini_t *ini);

int sbini_get_int (const sbini_t *ini, const char *group_name, const char *key, int *value);
int sbini_get_float (const sbini_t *ini, const char *group_name, const char *key, float *value);
int sbini_get_string (const sbini_t *ini, const char *group_name, const char *key, char *value, const size_t buffer_length);
int sbini_get_boolean (const sbini_t *ini, const char *group_name, const char *key, int *value);

int sbini_set_float (sbini_t *ini, const char *group_name, const char *key, const float value);
int sbini_set_boolean (sbini_t *ini, const char *group_name, const char *key, const int value);
int sbini_set_int (sbini_t *ini, const char *group_name, const char *key, const int value);
int sbini_set_string (sbini_t *ini, const char *group_name, const char *key, const char *value);

#ifdef __cplusplus
}
#endif

#endif // _SBINI_H_

#ifdef SBINI_IMPLEMENTATION
#define _CRT_SECURE_NO_WARNINGS

/** USEFUL MACROS **/
#define SBINI_IS_WHITESPACE(c) (c == ' ' || c == '\t')
#define SBINI_SKIP_WHITESPACE(cp) while (cp && SBINI_IS_WHITESPACE(*cp)) cp++
#define SBINI_SEEK_CHAR(cp,c) while (cp && *cp != c) cp++
#define SBINI_SEEK_CHAR_AND_WHITESPACE(cp,wsp,c) while (cp && *cp != c) { \
                                                  if (!wsp && SBINI_IS_WHITESPACE(*cp)) { \
                                                    wsp = cp; \
                                                  } else if (wsp && !SBINI_IS_WHITESPACE(*cp)) { \
                                                    wsp = NULL; \
                                                  } \
                                                  cp++; \
                                                }

#define SBINI_BOOLEAN_STRING(b) (b ? "true":"false")

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(SBINI_MALLOC) && defined(SBINI_FREE)
#elif !defined(SBINI_MALLOC) && !defined(SBINI_FREE)
#else
#error "Must define none or both of SBINI_MALLOC and SBINI_FREE"
#endif

#if !defined (SBINI_MALLOC) && !defined(SBINI_FREE)
#define SBINI_MALLOC(size) malloc(size)
#define SBINI_FREE(ptr) free(ptr)
#endif

#ifdef _MSC_VER
#define strncasecmp _strnicmp
#endif

/** INTERNAL FUNCTION DECLERATIONS **/
sbini_t *sbini__internal_create_ini (void);
sbini_group_t *sbini__internal_add_group (sbini_t *ini, const char *name);
sbini_group_t *sbini__internal_find_group (const sbini_t *ini, const char *name);
sbini_group_t *sbini__internal_find_or_add_group (sbini_t *ini, const char *name);
sbini_item_t *sbini__internal_add_item (sbini_group_t *group, const char *key, const char *value, const int string_val);
sbini_item_t *sbini__internal_find_item (const sbini_t *ini, const char *group, const char *key);
sbini_item_t *sbini__internal_find_item_in_group (const sbini_group_t *group, const char *key);
sbini_item_t *sbini__internal_modify_or_add_item (sbini_group_t *group, const char *key, const char *value, const int str_value);

/*********************************************************************/
/******************** PUBLIC FUNCTION DEFINITIONS ********************/
/*********************************************************************/

sbini_t *sbini_new (void) {
  return sbini__internal_create_ini();
}

sbini_t *sbini_load (const char *file_path) {
  FILE *fp;
  char line_buffer[SBINI_MAX_LINE_LENGTH];
  char *line, *ws, *str, *item_val;
  int string_val;
  int return_val = 0;
  sbini_t *ini;
  sbini_group_t *current_group = NULL;
  sbini_item_t *current_item = NULL;

  fp = fopen(file_path, "r");

  if (!fp) {
    return NULL;
  }

  ini = sbini__internal_create_ini();

  if (!ini) {
    fclose(fp);
    return NULL;
  }

  while (fgets(line_buffer, SBINI_MAX_LINE_LENGTH, fp) != NULL) {
    line = &line_buffer[0];

    SBINI_SKIP_WHITESPACE(line);

    if (*line != '#' && *line != ';' && *line != '\n') {
      if (*line == '[') {
        line++;
        str = line;
        SBINI_SEEK_CHAR(line, ']');
        *line = '\0';

        // if duplicate, append to previous, otherwise create new group
        current_group = sbini__internal_find_group(ini, str);

        if (current_group == NULL) {
          current_group = sbini__internal_add_group(ini, str);
        }

        if (!current_group) {
          return_val = -1;
          goto error_exit;
        }
      } else {
        /* READ KEY */

        SBINI_SKIP_WHITESPACE(line);
        str = line;

        ws = NULL;
        SBINI_SEEK_CHAR_AND_WHITESPACE(line, ws, '=');
        
        *line = '\0';

        if (ws) {
          *ws = '\0';
        }

        line++;

        /* READ VALUE */
        string_val = 0;

        SBINI_SKIP_WHITESPACE(line);

        if (*line == '\"') {
          string_val = 1;
          line++;
          item_val = line;

          SBINI_SEEK_CHAR(line, '\"');

          *line = '\0';
        } else {
          item_val = line;
          
          ws = NULL;
          SBINI_SEEK_CHAR_AND_WHITESPACE(line, ws, '\n');

          *line = '\0';
          
          if (ws) {
            *ws = '\0';
          }
        }

        if (!current_group) {
          return_val = -2;
          goto error_exit;
        }

        current_item = sbini__internal_add_item(current_group, str, item_val, string_val);

        if (!current_item) {
          return_val = -3;
          goto error_exit;
        }
      }
    }
  }

error_exit:;
  fclose(fp);

  if (return_val < 0) {
    sbini_free(ini);
    ini = NULL;
  }

  return ini;
}

/* GETTERS */

int sbini_get_int (const sbini_t *ini, const char *group_name, const char *key, int *value) {
  sbini_item_t *item;

  item = sbini__internal_find_item(ini, group_name, key);

  if (!item) {
    return -1;
  }

  *value = atoi(item->value);
  return 0;
}

int sbini_get_float (const sbini_t *ini, const char *group_name, const char *key, float *value) {
  sbini_item_t *item;

  item = sbini__internal_find_item(ini, group_name, key);

  if (!item) {
    return -1;
  }

  *value = (float) atof(item->value);
  return 0;
}

int sbini_get_string (const sbini_t *ini, const char *group_name, const char *key, char *buffer, const size_t buffer_length) {
  sbini_item_t *item;

  item = sbini__internal_find_item(ini, group_name, key);

  if (!item) {
    return -1;
  }

  strncpy(buffer, item->value, buffer_length);
  return 0;
}

int sbini_get_boolean (const sbini_t *ini, const char *group_name, const char *key, int *value) {
  sbini_item_t *item;

  item = sbini__internal_find_item(ini, group_name, key);

  if (!item) {
    return 0;
  }

  *value = (strncasecmp(item->value, "true", 4) == 0);
  return 1;
}

/* SETTERS */

int sbini_set_boolean (sbini_t *ini, const char *group_name, const char *key, const int value) {
  sbini_group_t *group;
  sbini_item_t *item;
  char str_value[SBINI_MAX_KEY_VALUE_LENGTH];

  group = sbini__internal_find_or_add_group(ini, group_name);

  if (!group) {
    return -1;
  }

  snprintf(str_value, SBINI_MAX_KEY_VALUE_LENGTH, "%s", SBINI_BOOLEAN_STRING(value));

  item = sbini__internal_modify_or_add_item(group, key, str_value, 0);

  if (!item) {
    return -2;
  }

  return 0;
}

int sbini_set_int (sbini_t *ini, const char *group_name, const char *key, const int value) {
  sbini_group_t *group;
  sbini_item_t *item;
  char str_value[SBINI_MAX_KEY_VALUE_LENGTH];

  group = sbini__internal_find_or_add_group(ini, group_name);

  if (!group) {
    return -1;
  }

  snprintf(str_value, SBINI_MAX_KEY_VALUE_LENGTH, "%d", value);

  item = sbini__internal_modify_or_add_item(group, key, str_value, 0);

  if (!item) {
    return -2;
  }

  return 0;
}


int sbini_set_float (sbini_t *ini, const char *group_name, const char *key, const float value) {
  sbini_group_t *group;
  sbini_item_t *item;
  char str_value[SBINI_MAX_KEY_VALUE_LENGTH];

  group = sbini__internal_find_or_add_group(ini, group_name);

  if (!group) {
    return -1;
  }

  snprintf(str_value, SBINI_MAX_KEY_VALUE_LENGTH, "%f",value);

  item = sbini__internal_modify_or_add_item(group, key, str_value, 0);

  if (!item) {
    return -2;
  }

  return 0;
}

int sbini_set_string (sbini_t *ini, const char *group_name, const char *key, const char *value) {
  sbini_group_t *group;
  sbini_item_t *item;

  group = sbini__internal_find_or_add_group(ini, group_name);

  if (!group) {
    return -1;
  }

  item = sbini__internal_modify_or_add_item(group, key, value, 1);

  if (!item) {
    return -2;
  }

  return 0; 
}

int sbini_save (sbini_t *ini, const char *file_path) {
  FILE *fp;
  sbini_group_t *group;
  sbini_item_t *item;

  fp = fopen(file_path, "w");

  if (!fp) {
    return -1;
  }

  group = ini->head;

  while (group) {
    fprintf(fp, "[%s]\n", group->name);

    item = group->head;

    while (item) {
      if (item->string_val) {
        fprintf(fp, "%s = \"%s\"\n", item->key, item->value);
      } else {
        fprintf(fp, "%s = %s\n", item->key, item->value);
      }
      item = item->next;
    }

    group = group->next;

    if (group) {
      fprintf(fp, "\n");
    }
  }

  fclose(fp);

  return 0;
}

void sbini_free (sbini_t *ini) {
  sbini_group_t *group, *tmp_group;
  sbini_item_t *item, *tmp_item;

  group = ini->head;

  while (group) {
    item = group->head;

    while (item) {
      tmp_item = item;
      item = item->next;
      SBINI_FREE(tmp_item);
    }

    tmp_group = group;
    group = group->next;
    SBINI_FREE(tmp_group);
  }
}

/*********************************************************************/
/******************* INTERNAL FUNCTION DEFINITIONS *******************/
/*********************************************************************/

sbini_t *sbini__internal_create_ini (void) {
  sbini_t *ini;

  ini = (sbini_t*)SBINI_MALLOC(sizeof(sbini_t));

  if (!ini) {
    return NULL;
  }

  ini->head = NULL;
  ini->group_count = 0;

  return ini;
}

sbini_group_t *sbini__internal_add_group (sbini_t *ini, const char *name) {
  sbini_group_t *group, *search;

  group = (sbini_group_t*)SBINI_MALLOC(sizeof(sbini_group_t));

  if (!group) {
    return NULL;
  }

  strncpy(group->name, name, SBINI_MAX_KEY_VALUE_LENGTH);

  group->next = NULL;
  group->head = NULL;

  if (!ini->head) {
    ini->head = group;
  } else {
    search = ini->head;

    while (search->next) {
      search = search->next;
    }

    search->next = group;
  }

  ini->group_count++;

  return group;
}

sbini_group_t *sbini__internal_find_group (const sbini_t *ini, const char *name) {
  sbini_group_t *group;

  group = ini->head;

  while (group) {
    if (strncmp(group->name, name, SBINI_MAX_KEY_VALUE_LENGTH) == 0) {
      return group;
    }

    group = group->next;
  }

  return NULL;
}

sbini_group_t *sbini__internal_find_or_add_group (sbini_t *ini, const char *name) {
  sbini_group_t *group;

  group = sbini__internal_find_group(ini, name);

  if (!group) {
    group = sbini__internal_add_group(ini, name);

    if (!group) {
      return NULL;
    }
  }

  return group;
}

sbini_item_t *sbini__internal_add_item (sbini_group_t *group, const char *key, const char *value, const int string_val) {
  sbini_item_t *item, *search;

  item = (sbini_item_t*)SBINI_MALLOC(sizeof(sbini_item_t));

  if (!item) {
    return NULL;
  }

  strncpy(item->key, key, SBINI_MAX_KEY_VALUE_LENGTH);
  strncpy(item->value, value, SBINI_MAX_KEY_VALUE_LENGTH);

  item->next = NULL;

  item->string_val = string_val;

  if (group->head == NULL) {
    group->head = item;
  } else {
    search = group->head;

    while (search->next) {
      search = search->next;
    }

    search->next = item;
  }

  group->item_count++;

  return item;
}

sbini_item_t *sbini__internal_find_item (const sbini_t *ini, const char *group_name, const char *key) {
  sbini_item_t *item;
  sbini_group_t *group;

  group = ini->head;

  while (group) {
    if (strncmp(group->name, group_name, SBINI_MAX_KEY_VALUE_LENGTH) == 0) {
      item = group->head;

      while (item) {
        if (strncmp(item->key, key, SBINI_MAX_KEY_VALUE_LENGTH) == 0) {
          return item;
        }

        item = item->next;
      }
      return NULL; // not in group, return
    }

    group = group->next;
  }

  return NULL; // no such group;
}

sbini_item_t *sbini__internal_find_item_in_group (const sbini_group_t *group, const char *key) {
  sbini_item_t *item;

  item = group->head;

  while (item) {
    if (strncmp(item->key, key, SBINI_MAX_KEY_VALUE_LENGTH) == 0) {
      return item;
    }

    item = item->next;
  }

  return NULL;
}

sbini_item_t *sbini__internal_modify_or_add_item (sbini_group_t *group, const char *key, const char *value, const int str_value) {
  sbini_item_t *item;

  item = sbini__internal_find_item_in_group(group, key);

  if (!item) {
    item = sbini__internal_add_item(group, key, value, str_value);

    if (!item) {
      return NULL;
    }
  } else {
    if (str_value) {
      snprintf(item->value, SBINI_MAX_KEY_VALUE_LENGTH, "\"%s\"", value);
    } else {
      snprintf(item->value, SBINI_MAX_KEY_VALUE_LENGTH, "%s", value);
    }
  }

  return item;
}

#endif // SBINI_IMPLEMENTATION

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
