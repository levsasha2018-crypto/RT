#include "stdio-text-protocol/stdio-text-protocol.h"

#include <stddef.h>
#include <stdio.h>

#include "pico/stdlib.h"

static char line[STDIO_TEXT_PROTOCOL_LINE_SIZE];
static uint32_t length;
static bool overflow;
static command_t command;

void stdio_text_protocol_init(void)
{
    length = 0;
    overflow = false;
    command = (command_t){0};
}

static const command_t *split_line(void)
{
    command = (command_t){ .truncated = overflow };
    char *word = line;
    while (*word != '\0') {
        while (*word == ' ') {
            word++;
        }
        if (*word == '\0') {
            break;
        }
        char *start = word;
        while (*word != '\0' && *word != ' ') {
            word++;
        }
        if (*word != '\0') {
            *word++ = '\0';
        }
        if (command.name == NULL) {
            command.name = start;
        } else if (command.argc < STDIO_TEXT_PROTOCOL_MAX_ARGS) {
            command.argv[command.argc++] = start;
        } else {
            command.truncated = true;
        }
    }
    return command.name != NULL || command.truncated ? &command : NULL;
}

const command_t *stdio_text_protocol_handle(void)
{
    const int symbol = getchar_timeout_us(0);
    if (symbol == PICO_ERROR_TIMEOUT) {
        return NULL;
    }
    if (symbol == '\r' || symbol == '\n') {
        if (length == 0) {
            return NULL;
        }
        line[length] = '\0';
        putchar('\n'); // Pico stdio emits CR LF.
        const command_t *result = split_line();
        length = 0;
        overflow = false;
        return result;
    }
    if (symbol == 0x08 || symbol == 0x7f) {
        if (length > 0) {
            length--;
            overflow = false;
            printf("\b \b");
        }
        return NULL;
    }
    if (symbol >= 0x20 && symbol <= 0x7e) {
        if (length < sizeof(line) - 1) {
            line[length++] = (char)symbol;
            putchar(symbol);
        } else {
            overflow = true;
        }
    }
    return NULL;
}
