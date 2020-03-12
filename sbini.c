/**
* sbini -- a general purpose ini file reader/writer
* This file is released into the public domain
*
* Written by Saxon Bell
*/

#include "sbini.h"

/** USEFUL MACROS **/
#define SBINI_IS_WHITESPACE(c) (c == ' ' || c == '\t')
#define SBINI_SKIP_WHITESPACE(cp) while (cp && SBINI_IS_WHITESPACE(*cp)) cp++
#define SBINI_SEEK_CHAR(cp,c) while (cp && *cp != c) cp++
#define SBINI_SEEK_CHAR_AND_WHITESPACE(cp,ws,c) while (cp && *cp != c) { \
                                                  if (!ws && SBINI_IS_WHITESPACE(*cp)) { \
                                                    ws = cp; \
                                                  } else if (ws && !SBINI_IS_WHITESPACE(*cp)) { \
                                                    ws = NULL; \
                                                  } \
                                                  cp++; \
                                                }

#define SBINI_BOOLEAN_STRING(b) b?"true":"false"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _MSC_VER
#define strncasecmp _strnicmp
#endif

/** INTERNAL FUNCTION DECLERATIONS **/
sbini_t *sbini__internal_create_ini (void);
sbini_group_t *sbini__internal_add_group (sbini_t *ini, const char *name);
sbini_group_t *sbini__internal_find_group (sbini_t *ini, const char *name);
sbini_group_t *sbini__internal_find_or_add_group (sbini_t *ini, const char *name);
sbini_item_t *sbini__internal_add_item (sbini_group_t *group, const char *key, const char *value, const int string_val);
sbini_item_t *sbini__internal_find_item (sbini_t *ini, const char *group, const char *key);
sbini_item_t *sbin__internal_find_item_in_group (sbini_group_t *group, const char *key);
sbini_item_t *sbini__internal_modify_or_add_item (sbini_group_t *group, const char *key, const char *value, const int str_value);

/** PUBLIC FUNCTION DEFINITIONS **/

int sbini_load (const char *file_path, sbini_t *ini)
{
  FILE *fp;
  char line_buffer[SBINI_MAX_LINE_LENGTH];
  char *line, *ws, *str, *item_val;
  int string_val;
  int return_val = 0;
  sbini_t *result = NULL;
  sbini_group_t *current_group = NULL;
  sbini_item_t *current_item = NULL;

  fp = fopen(file_path, "r");

  if (!fp) {
    return -1;
  }

  if (ini != NULL) {
    memset(ini, 0, sizeof(sbini_t));
    result = ini;
  } else {
    return -1;
  }

  if (!result) {
    return -2;
  }

  while (fgets(line_buffer, SBINI_MAX_LINE_LENGTH, fp) != NULL) {
    if (line_buffer[0] != '#' && line_buffer[0] != '\n') {
      line = &line_buffer[0];

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
          return_val = -3;
          break;
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
          return_val = -4;
          break;
        }

        current_item = sbini__internal_add_item(current_group, str, item_val, string_val);

        if (!current_item) {
          return_val = -5;
          break;
        }
      }
    }
  }

  ini = result;

  fclose(fp);

  return return_val;
}

/* GETTERS */

int sbini_get_int (sbini_t *ini, const char *group, const char *key)
{
  sbini_item_t *item;

  item = sbini__internal_find_item(ini, group, key);

  if (!item) {
    return 0;
  }

  return atoi(item->value);
}

float sbini_get_float (sbini_t *ini, const char *group, const char *key)
{
  sbini_item_t *item;

  item = sbini__internal_find_item(ini, group, key);

  if (!item) {
    return 0;
  }

  return (float)atof(item->value);
}

const char *sbini_get_string (sbini_t *ini, const char *group, const char *key)
{
  sbini_item_t *item;

  item = sbini__internal_find_item(ini, group, key);

  if (!item) {
    return 0;
  }

  return item->value;
}

int sbini_get_boolean (sbini_t *ini, const char *group, const char *key)
{
  sbini_item_t *item;

  item = sbini__internal_find_item(ini, group, key);

  if (!item) {
    return 0;
  }

  return (strncasecmp(item->value, "true", 4) == 0);
}

/* SETTERS */

int sbini_set_boolean (sbini_t *ini, const char *group_name, const char *key, const int value)
{
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

int sbini_set_int (sbini_t *ini, const char *group_name, const char *key, const int value)
{
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


int sbini_set_float (sbini_t *ini, const char *group_name, const char *key, const float value)
{
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

int sbini_set_string (sbini_t *ini, const char *group_name, const char *key, const char *value)
{
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

int sbini_save (sbini_t *ini, const char *file_path)
{
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

void sbini_free (sbini_t *ini)
{
  sbini_group_t *group, *tmp_group;
  sbini_item_t *item, *tmp_item;

  group = ini->head;

  while (group) {
    item = group->head;

    while (item) {
      tmp_item = item;
      item = item->next;
      free(tmp_item);
    }

    tmp_group = group;
    group = group->next;
    free(tmp_group);
  }
}

/*********************************************************************/
/******************* INTERNAL FUNCTION DEFINITIONS *******************/
/*********************************************************************/

sbini_t *sbini__internal_create_ini (void)
{
  sbini_t *ini;

  ini = (sbini_t*)malloc(sizeof(sbini_t));

  if (!ini) {
    return NULL;
  }

  ini->head = NULL;
  ini->group_count = 0;

  return ini;
}

sbini_group_t *sbini__internal_add_group (sbini_t *ini, const char *name)
{
  sbini_group_t *group, *search;

  group = (sbini_group_t*)malloc(sizeof(sbini_group_t));

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

sbini_group_t *sbini__internal_find_group (sbini_t *ini, const char *name)
{
  sbini_group_t *group;

  group = ini->head;

  while (group) {
    if (strcmp(group->name, name) == 0) {
      return group;
    }

    group = group->next;
  }

  return NULL;
}

sbini_group_t *sbini__internal_find_or_add_group (sbini_t *ini, const char *name)
{
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

sbini_item_t *sbini__internal_add_item (sbini_group_t *group, const char *key, const char *value, const int string_val)
{
  sbini_item_t *item, *search;

  item = (sbini_item_t*)malloc(sizeof(sbini_item_t));

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

sbini_item_t *sbini__internal_find_item (sbini_t *ini, const char *group_name, const char *key)
{
  sbini_item_t *item;
  sbini_group_t *group;

  group = ini->head;

  while (group) {
    if (strcmp(group->name, group_name) == 0) {
      item = group->head;

      while (item) {
        if (strcmp(item->key, key) == 0) {
          return item;
        }

        item = item->next;
        return NULL; // not in group, return
      }
    }

    group = group->next;
  }

  return NULL; // no such group;
}

sbini_item_t *sbin__internal_find_item_in_group (sbini_group_t *group, const char *key)
{
  sbini_item_t *item;

  item = group->head;

  while (item) {
    if (strcmp(item->key, key) == 0) {
      return item;
    }

    item = item->next;
  }

  return NULL;
}

sbini_item_t *sbini__internal_modify_or_add_item (sbini_group_t *group, const char *key, const char *value, const int str_value)
{
  sbini_item_t *item;

  item = sbin__internal_find_item_in_group(group, key);

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
