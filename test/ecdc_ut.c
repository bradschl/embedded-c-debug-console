/**
 * Copyright (c) 2016 Bradley Kim Schleusner < bradschl@gmail.com >
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdbool.h>
#include <string.h>

#include "ecdc/ecdc.h"

#include "describe/describe.h"
#include "strdup/strdup.h"


struct simple_buf {
    char *      read_data;
    size_t      read_data_size;
    size_t      read_index;

    char *      write_data;
    size_t      write_data_size;
    size_t      write_index;
};


static struct simple_buf *
alloc_simple_buf(size_t size) {
    struct simple_buf * buf = (struct simple_buf *) malloc(sizeof(struct simple_buf));

    buf->read_data = (char *) malloc(size*sizeof(char));
    buf->read_data_size = size;
    buf->read_index = 0;

    buf->write_data = (char *) malloc(size*sizeof(char));
    buf->write_data_size = size;
    buf->write_index = 0;

    return buf;
}


static void
realloc_and_reset(struct simple_buf * buf, size_t size)
{
    buf->read_data = (char *) realloc(buf->read_data, size);
    buf->read_data_size = size;
    buf->read_index = 0;

    buf->write_data = (char *) realloc(buf->write_data, size);
    buf->write_data_size = size;
    buf->write_index = 0;
}


static void
free_simple_buf(struct simple_buf * buf)
{
    free(buf->read_data);
    free(buf->write_data);
    free(buf);
}


static void
mock_putc(void * hint, char c)
{
    if(NULL != hint) {
        struct simple_buf * buf = (struct simple_buf *) hint;
        if(buf->write_index < buf->write_data_size) {
            buf->write_data[buf->write_index] = c;
            ++buf->write_index;
        }
    }
}


static int
mock_getc(void * hint)
{
    int ret_c = ECDC_GETC_EOF;

    if(NULL != hint) {
        struct simple_buf * buf = (struct simple_buf *) hint;
        if(buf->read_index < buf->read_data_size) {
            ret_c = buf->read_data[buf->read_index];
            ++buf->read_index;
        }
    }

    return ret_c;
}



static void
mock_callback(void * hint, int argc, char const * argv[])
{
    (void) hint;
    (void) argc;
    (void) argv;
}



static int
test_alloc_1(void)
{
    describe("embedded-c-debug-console can alloc/free objects in order") {
        struct ecdc_console * console = NULL;

        it("can allocate a console") {
            console = ecdc_alloc_console(NULL, mock_getc, mock_putc, 80, 6);
            assert_not_null(console);
        }

        struct ecdc_command * cmd_1 = NULL;
        it("can allocate a command") {
            cmd_1 = ecdc_alloc_command(NULL, console, "cmd_1", mock_callback);
            assert_not_null(cmd_1);
        }

        it("can free a command") {
            ecdc_free_command(cmd_1);
        }

        it("can free a console") {
            ecdc_free_console(console);
        }
    }

    return assert_failures();
}


static int
test_alloc_2(void)
{
    describe("embedded-c-debug-console can alloc/free objects out of order") {
        struct ecdc_console * console = NULL;

        it("can allocate a console") {
            console = ecdc_alloc_console(NULL, mock_getc, mock_putc, 80, 6);
            assert_not_null(console);
        }

        struct ecdc_command * cmd_1 = NULL;
        it("can allocate a command") {
            cmd_1 = ecdc_alloc_command(NULL, console, "cmd_1", mock_callback);
            assert_not_null(cmd_1);
        }

        it("can free a console before a command") {
            ecdc_free_console(console);
        }

        it("can free a command after the console") {
            ecdc_free_command(cmd_1);
        }
    }

    return assert_failures();
}


static int
test_link_1(void)
{
    describe("embedded-c-debug-console can link multiple commands to a console") {

        struct ecdc_console * console = NULL;

        it("can allocate a console") {
            console = ecdc_alloc_console(NULL, mock_getc, mock_putc, 80, 6);
            assert_not_null(console);
        }

        struct ecdc_command * cmd_1 = NULL;
        it("can link the first command") {
            cmd_1 = ecdc_alloc_command(NULL, console, "cmd_1", mock_callback);
            assert_not_null(cmd_1);
        }

        struct ecdc_command * cmd_2 = NULL;
        it("can link the second command") {
            cmd_2 = ecdc_alloc_command(NULL, console, "cmd_2", mock_callback);
            assert_not_null(cmd_2);
        }

        it("can free/unlink the first command") {
            ecdc_free_command(cmd_1);
        }

        it("can free/unlink the second command") {
            ecdc_free_command(cmd_2);
        }

        it("can free the console") {
            ecdc_free_console(console);
        }
    }

    return assert_failures();
}


static int
test_link_2(void)
{
    describe("embedded-c-debug-console can free the console before multiple commands") {

        struct ecdc_console * console = NULL;

        it("can allocate a console") {
            console = ecdc_alloc_console(NULL, mock_getc, mock_putc, 80, 6);
            assert_not_null(console);
        }

        struct ecdc_command * cmd_1 = NULL;
        it("can link the first command") {
            cmd_1 = ecdc_alloc_command(NULL, console, "cmd_1", mock_callback);
            assert_not_null(cmd_1);
        }

        struct ecdc_command * cmd_2 = NULL;
        it("can link the second command") {
            cmd_2 = ecdc_alloc_command(NULL, console, "cmd_2", mock_callback);
            assert_not_null(cmd_2);
        }

        it("can free the console before the commands") {
            ecdc_free_console(console);
        }

        it("can free/unlink the first command") {
            ecdc_free_command(cmd_1);
        }

        it("can free/unlink the second command") {
            ecdc_free_command(cmd_2);
        }

    }

    return assert_failures();
}



static void
test_parse_1_callback(void * hint, int argc, char const * argv[])
{
    assert_not_null(hint);
    if(NULL != hint) {
        bool * was_called = (bool *) hint;
        *was_called = true;
    }

    assert_ok(argc == 4);
    if(argc == 4) {
        assert_str_equal(argv[0], "cmd_1");
        assert_str_equal(argv[1], "arg_1");
        assert_str_equal(argv[2], "arg_2");
        assert_str_equal(argv[3], "arg_3");
    }
}


static int
test_parse_1(void)
{
    describe("embedded-c-debug-console can parse an input string") {

        static const char TEST_STRING[] = "cmd_1 arg_1 arg_2     arg_3\r";
        struct simple_buf * read_buffer = alloc_simple_buf(sizeof(TEST_STRING));
        memcpy(read_buffer->read_data, TEST_STRING, sizeof(TEST_STRING));

        struct ecdc_console * console = NULL;
        it("can allocate a console") {
            console = ecdc_alloc_console(read_buffer, mock_getc, mock_putc, 80, 6);
            assert_not_null(console);
        }

        bool cmd_1_called = false;
        struct ecdc_command * cmd_1 = NULL;
        it("can allocate a command") {
            cmd_1 = ecdc_alloc_command(
                &cmd_1_called,
                console,
                "cmd_1",
                test_parse_1_callback);
            assert_not_null(cmd_1);
        }

        it("can read in strings") {
            int i;
            for(i = 0; i < 30; ++i) {
                ecdc_pump_console(console);
            }

            assert_ok(read_buffer->read_index == read_buffer->read_data_size);
        }

        it("can parse and callback a registered command handler") {
            assert_ok(cmd_1_called);
        }

        it("can free a command") {
            ecdc_free_command(cmd_1);
        }

        it("can free a console") {
            ecdc_free_console(console);
        }

        free_simple_buf(read_buffer);
    }

    return assert_failures();
}



static void
test_parse_2_cmd_1(void * hint, int argc, char const * argv[])
{
    assert_not_null(hint);
    if(NULL != hint) {
        bool * was_called = (bool *) hint;
        *was_called = true;
    }

    assert_ok(argc == 2);
    if(argc == 2) {
        assert_str_equal(argv[0], "cmd_1");
        assert_str_equal(argv[1], "arg_1");
    }
}


static void
test_parse_2_cmd_2(void * hint, int argc, char const * argv[])
{
    assert_not_null(hint);
    if(NULL != hint) {
        bool * was_called = (bool *) hint;
        *was_called = true;
    }

    assert_ok(argc == 2);
    if(argc == 2) {
        assert_str_equal(argv[0], "cmd_2");
        assert_str_equal(argv[1], "arg_2");
    }
}


static int
test_parse_2(void)
{
    describe("embedded-c-debug-console can parse multiple input strings") {

        static const char TEST_STRING_1[] = "cmd_1 arg_1\r";
        struct simple_buf * read_buffer = alloc_simple_buf(sizeof(TEST_STRING_1));
        memcpy(read_buffer->read_data, TEST_STRING_1, sizeof(TEST_STRING_1));

        struct ecdc_console * console = NULL;
        it("can allocate a console") {
            console = ecdc_alloc_console(read_buffer, mock_getc, mock_putc, 80, 6);
            assert_not_null(console);
        }

        bool cmd_1_called = false;
        struct ecdc_command * cmd_1 = NULL;
        it("can allocate a command") {
            cmd_1 = ecdc_alloc_command(
                &cmd_1_called,
                console,
                "cmd_1",
                test_parse_2_cmd_1);
            assert_not_null(cmd_1);
        }

        bool cmd_2_called = false;
        struct ecdc_command * cmd_2 = NULL;
        it("can allocate a second command") {
            cmd_2 = ecdc_alloc_command(
                &cmd_2_called,
                console,
                "cmd_2",
                test_parse_2_cmd_2);
            assert_not_null(cmd_2);
        }

        it("can read in the first string") {
            int i;
            for(i = 0; i < 30; ++i) {
                ecdc_pump_console(console);
            }

            assert_ok(read_buffer->read_index == read_buffer->read_data_size);
        }

        it("can parse the first registered command") {
            assert_ok(cmd_1_called);
        }


        static const char TEST_STRING_2[] = "cmd_2 arg_2\r";
        realloc_and_reset(read_buffer, sizeof(TEST_STRING_2));
        memcpy(read_buffer->read_data, TEST_STRING_2, sizeof(TEST_STRING_2));

        it("can read in the second string") {
            int i;
            for(i = 0; i < 30; ++i) {
                ecdc_pump_console(console);
            }

            assert_ok(read_buffer->read_index == read_buffer->read_data_size);
        }

        it("can parse the second registered command") {
            assert_ok(cmd_2_called);
        }

        it("can free the first command") {
            ecdc_free_command(cmd_1);
        }

        it("can free the second command") {
            ecdc_free_command(cmd_2);
        }

        it("can free a console") {
            ecdc_free_console(console);
        }

        free_simple_buf(read_buffer);
    }

    return assert_failures();
}



static void
test_prompt_write_cmd_1(void * hint, int argc, char const * argv[])
{
    (void) argc;
    (void) argv;

    assert_not_null(hint);
    if(NULL != hint) {
        bool * was_called = (bool *) hint;
        *was_called = true;
    }
}


static int
test_prompt_write(void)
{
    describe("embedded-c-debug-console can print a console") {

        struct simple_buf * buf = alloc_simple_buf(32);
        buf->read_index = buf->read_data_size;

        struct ecdc_console * console = NULL;
        it("can allocate a console") {
            console = ecdc_alloc_console(buf, mock_getc, mock_putc, 80, 6);
            assert_not_null(console);
        }

        bool cmd_1_called = false;
        struct ecdc_command * cmd_1 = NULL;
        it("can allocate a command") {
            cmd_1 = ecdc_alloc_command(
                &cmd_1_called,
                console,
                "cmd_1",
                test_prompt_write_cmd_1);
            assert_not_null(cmd_1);
        }

        it("will not print a prompt until the first user input") {
            int i;
            for(i = 0; i < 30; ++i) {
                ecdc_pump_console(console);
            }

            assert_ok(buf->write_index == 0);
        }

        // Write an input char
        buf->read_index = buf->read_data_size - 1;
        buf->read_data[buf->read_index] = ' ';

        it("can print a prompt") {
            int i;
            for(i = 0; i < 30; ++i) {
                ecdc_pump_console(console);
            }

            assert_ok(buf->write_index > 0);
        }

        static const char TEST_STRING_1[] = "cmd_1 arg_2\r";
        realloc_and_reset(buf, sizeof(TEST_STRING_1));
        memcpy(buf->read_data, TEST_STRING_1, sizeof(TEST_STRING_1));

        it("can print a prompt after calling a callback") {
            assert_ok(buf->write_index == 0);
            assert_ok(cmd_1_called == false);

            int i;
            for(i = 0; i < 30; ++i) {
                ecdc_pump_console(console);
            }

            assert_ok(buf->write_index > 0);
            assert_ok(cmd_1_called == true);
        }

        it("can free a console") {
            ecdc_free_console(console);
        }

        free_simple_buf(buf);
    }

    return assert_failures();
}



int
main(int argc, char const *argv[])
{
    (void) argc;
    (void) argv;

    // Make assertion failures fatal
    // assert_bail();

    return (
           test_alloc_1()
        || test_alloc_2()
        || test_link_1()
        || test_link_2()
        || test_parse_1()
        || test_parse_2()
        || test_prompt_write()
    );
}

