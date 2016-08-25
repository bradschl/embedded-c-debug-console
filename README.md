# Embedded C Debug Console
This is a minimalistic library for adding a debug console to an embedded project. It is written to be compatible with C99 and C11, and can be easily wrapped in C++.

This library provides a console for that receives and parses user input. Arbitrary commands can be registered with it to handle line entries, with a familiar argc/argv style callback system.

## Installation
Install with [clib](https://github.com/clibs/clib):

```
$ clib install bradschl/embedded-c-debug-console
```

## Examples
```C
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
console_putc(void * console_hint, char c)
{
    (void) console_hint;
    UART_1_PutChar(c);
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
        ecdc_alloc_console(NULL, console_getc, console_putc, 100, 10);

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

Compiled, this library is only a few kilobytes. Runtime memory footprint is very small, and is dependent on the settings passed in when allocating the console and the number of registered commands.

## License
MIT license for all files.
