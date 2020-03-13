# sbini

A basic ini file parsing library that aims to be lightweight and easy to integrate. It relies only on the standard C library and uses some C99 functionality for safety.

## Features
* Read and/or write ini files.
* Easy integration, simply drop in one header and one source file, and you're set!

## Limitations
* No UTF8 support (ANSI only)
* Comments are limited to full line, end of line comments not supported.
* When reading subsequently saving, any comments within the ini file are lost. (Refer to Issue #4)
* Still a work in progress, and some baffling decisions may have been made due to lack of sleep.

## License
It's public domain under terms of the Unlicense. Refer to the bottom of the header and source files to read terms.

## Basic Usage
### test.ini
```INI
[test_group]
test1 = "this is a thing that can happen"
test2 = false
test4 = 3

[test_group_2]
test1 = 12.123123142
test2 = true
test3 = "This is a string that seems to go on for quite a while. Stay a while and listen, while I tell you about the majestic moose."
test4 = 0.0000000001
```

### test.c
```C
/*
* Reading and writing a basic INI File:
*/
#include <stdio.h>
#include "sbini.h"

int main (int argc, char *argv[])
{
    sbini_t *ini;

    ini = sbini_load("test.ini");

    if (!ini) {
        printf("Something has gone terribly wrong\n");
        return -1;
    }

    printf("String: %s\n", sbini_get_string(ini, "test_group", "test1"));
    printf("Integer: %d\n", sbini_get_int(ini, "test_group", "test4"));
    printf("Float: %f\n", sbini_get_float(ini, "test_group_2", "test1"));
    printf("Boolean: %d\n", sbini_get_boolean(ini, "test_group_2", "test2"));

    sbini_set_boolean(ini, "newly_added", "testkey", 1);
    sbini_set_float(ini, "newly_added", "anotherkey", 123.1234f);
    sbini_set_int(ini, "newly_added", "whatisthis", 4003);
    sbini_set_string(ini, "newly_added", "bass", "Rickenbacker 4003!");

    sbini_save(ini, "output.ini"); // save changes

    sbini_free(ini);

    return 0;
}
```

### output.ini
```INI
[test_group]
test1 = "this is a thing that can happen"
test2 = false
test4 = 3

[test_group_2]
test1 = 12.123123123
test2 = true
test3 = "This is a string that seems to go on for quite a while. Stay a while and listen, while I tell you about the majestic moose."
test4 = 0.0000000001

[newly_added]
testkey = true
anotherkey = 123.123398
whatisthis = 4003
bass = "Rickenbacker 4003!"
```

### Program Output:
```
String: this is a thing that can happen
Integer: 3
Float: 12.123123
Boolean: 1
```