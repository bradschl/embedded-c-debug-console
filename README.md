# Embedded C Debug Console
This is a minimalistic library for adding a debug console to an embedded project. It is written to be compatible with C99 and C11, and can be easily wrapped in C++.

This library provides a console for that receives and parses user input. Arbitrary commands can be registered with it to handle line entries, with a familiar argc/argv style callback system.

## Installation
The source can be installed into a project automatically with [clib](https://github.com/clibs/clib):

```bash
cd <your project>
clib install bradschl/embedded-c-debug-console
```


Alternatively, just copy the source code into your project

```bash
git clone https://github.com/bradschl/embedded-c-debug-console.git
cp -r embedded-c-debug-console/src/ecdc <your project>/src
```

## Examples
```C

#include "ecdc/ecdc.h"

static int
console_getc(void * console_hint)
{
    (void) console_hint;
    int ret = ECDC_GETC_EOF;
    if(0 < UART_1_GetRxBufferSize()) {
        ret = UART_1_ReadRxData();
    }
    return ret;
}

static void
console_puts(void * console_hint, const char * s, size_t len)
{
    (void) console_hint;
    size_t idx;
    for(idx = 0; idx < len; ++idx) {
        UART_1_PutChar(s[idx]);
    }
}

static void
foo_cmd_callback(void * hint, int argc, char const * argv[])
{
    // ...
}

int
main(int argc, char const *argv[])
{
    struct ecdc_console * console =
        ecdc_alloc_console(NULL, console_getc, console_puts, 100, 10);

    ecdc_configure_console(console, ECDC_MODE_ANSI, ECDC_SET_LOCAL_ECHO);

    struct ecdc_command * foo_cmd =
        ecdc_alloc_command(NULL, console, "foo", foo_cmd_callback);

    for(;;) {
        ecdc_pump_console(console);
        // ...
    }
}
```

## API
See [ecdc.h](src/ecdc/ecdc.h) for the C API.

## Dependencies and Resources
This library uses heap when allocating structures and buffers. After initialization, additional allocations will not be made. This should be fine for an embedded target, since memory fragmentation only happens if memory is freed.

Compiled, this library is only a few kilobytes (3kB on x86_64). Runtime memory footprint is very small, and is dependent on the settings passed in when allocating the console and the number of registered commands. About 2kB of heap is a good starting point.

## License
MIT license for all files.
