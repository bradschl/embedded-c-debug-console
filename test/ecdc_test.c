#include <stdbool.h>
#include <stddef.h>

#include <stdio.h>
#include <errno.h>

// posix_openpt, grantpt, unlockpt, ptsname
#include <stdlib.h>
#include <fcntl.h>

// read, write
#include <unistd.h>

// nanosleep
#include <time.h>

#include "ecdc/ecdc.h"


static int
console_getc(void * console_hint)
{
    int * fd = (int *) console_hint;

    int ret = ECDC_GETC_EOF;

    char c;
    if(0 < read(*fd, &c, 1)) {
        ret = c;
        fprintf(stdout, "GETC: 0x%02X\n", ret);
    }

    return ret;
}

static void
console_puts(void * console_hint, const char * s, size_t len)
{
    if(0 == len) {
        fprintf(stdout, "WARNING: puts of 0 len\n");
    }

    fprintf(stdout, "PUTS:");
    size_t idx;
    for(idx = 0; idx < len; ++idx) {
        fprintf(stdout, " 0x%02X", (int) s[idx]);
    }
    fprintf(stdout, "\n");

    int * fd = (int *) console_hint;
    write(*fd, s, len);
}


static void
exit_cmd(void * hint, int argc, char const * argv[])
{
    (void) argc;
    (void) argv;

    bool * is_running = (bool *) hint;
    *is_running = false;
}


static void
delay_us(long us)
{
    struct timespec req;
    req.tv_sec = 0;
    req.tv_nsec = us * 100;

    (void) nanosleep(&req, NULL);
}



int
main(int argc, char const *argv[])
{
    (void) argc;
    (void) argv;


    // Create a pseudo-terminal
    int pt = posix_openpt(O_RDWR | O_NOCTTY);
    if(0 > pt) {
        fprintf(stderr, "Failed to create pseudoterminal\n");
        return errno;
    }

    if(0 != grantpt(pt)) {
        fprintf(stderr, "Failed to set permissions (grantpt)\n");
        return errno;
    }

    if(0 != unlockpt(pt)) {
        fprintf(stderr, "Failed to unlock terminal (unlockpt)\n");
        return errno;
    }

    char * pt_name = ptsname(pt);
    if(NULL == pt_name) {
        fprintf(stderr, "Failed to get pts name (ptsname)\n");
        return errno;
    } else {
        fprintf(stdout, "Opened PTS %s\n", pt_name);
        fprintf(stdout, " - Run \"screen %s\" to connect\n", pt_name);
        fprintf(stdout, " - Press Ctrl+C to quit\n");
    }


    // Create and configure the console
    struct ecdc_console * console =
        ecdc_alloc_console(&pt, console_getc, console_puts, 100, 10);

    ecdc_configure_console(console, ECDC_MODE_ANSI, ECDC_SET_LOCAL_ECHO);


    // Create a few test commands
    bool is_running = true;
    struct ecdc_command * test_cmd =
        ecdc_alloc_command(&is_running, console, "exit", exit_cmd);

    struct ecdc_command * list_command = ecdc_alloc_list_command(console, "ls");


    // Process commands until exit is typed
    while(is_running) {
        ecdc_pump_console(console);
        delay_us(250);
    }


    // Free the command and console
    ecdc_free_command(list_command);
    ecdc_free_command(test_cmd);
    ecdc_free_console(console);


    // Close the pseudo-terminal
    close(pt);
    return 0;
}

