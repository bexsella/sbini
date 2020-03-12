# sbini

A basic ini file parsing library that aims to be lightweight and easy to integrate. It relies only on the standard C library and uses some C99 functionality for safety.

## Features
* Read and/or write ini files.
* Easy integration, simply drop in one header and one source file, and you're set!

## Limitations
* No UTF8 support (ANSI only)
* Comments are limited to full line, end of line comments not supported.
* When reading subsequently saving, any comments within the ini file are lost. (Refer to Issue #4)
* Currently there isn't a way to create an ini file without first having read one. (Refer to Issue #5)
* Still a work in progress, and some baffling decisions may have been made due to lack of sleep.

## License
It's public domain under terms of the Unlicense. Refer to the bottom of the header and source files to read terms.

## Basic Usage
```C
/*
* Reading and writing a basic INI File:
*/
#include <stdio.h>
#include "sbini.h"

int main (int argc, char *argv[])
{
    sbini_t ini;

    if (sbini_load("test.ini", &ini) != 0) {
        printf("Something has gone terribly wrong\n");
        return -1;
    }

    sbini_set_boolean(&ini, "newly_added", "testkey", 1);
    sbini_set_float(&ini, "newly_added", "anotherkey", 123.1234f);
    sbini_set_int(&ini, "newly_added", "whatisthis", 4003);
    sbini_set_string(&ini, "newly_added", "bass", "Rickenbacker 4003!");

    sbini_save(&ini, "new_test.ini"); // save changes

    sbini_free(&ini);

    return 0;
}
