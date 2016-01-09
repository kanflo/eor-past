/* Parameter storage example
 *
 * This sample code is in the public domain.
 */

#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <espressif/esp_common.h>
#include <past.h>
#include <string.h>
#include <stdlib.h>
#include <FreeRTOS.h>
#include <task.h>
#include <stdin_uart_interrupt.h>

// Max number of arguments to commands
#define MAX_ARGC  (10)

// Helpers
#define delay_ms(t) vTaskDelay((t) / portTICK_RATE_MS)
#define systime_ms() (xTaskGetTickCount()*portTICK_RATE_MS)

static void cmd_help(uint32_t argc, char *argv[])
{
    printf("pastmon help\n\n");
    printf("commands are typed line by line, the colon character is used to separate arguments\n\n");
    printf(" format                  - format past area\n");
    printf(" valid                   - check if past area is valid\n");
    printf(" read:<unit id>          - read past unit\n");
    printf(" write:<unit id>:<data>  - write data to past unit\n");
    printf(" erase:<unit id>         - erase past unit\n");
    printf(" size                    - print size of past\n");
    printf(" dump                    - dump past area\n");
    printf(" quit                    - quit pastmon\n");
}

static void cmd_past_format(uint32_t argc, char *argv[])
{
    if (past_format()) {
        printf("OK\n");
    } else {
        printf("Failed to format past\n");
    }
}

static void cmd_past_read(uint32_t argc, char *argv[])
{
    if (argc == 2) {
        uint32_t id = atoi(argv[1]);
        static char data[20]; // Be careful with stack usage
        if (past_read_unit(id, (void*) data, sizeof(data))) {
            printf("OK: %s\n", data);
        } else {
            printf("Failed to read unit %u\n", id);
        }
    } else {
        printf("Error: wrong number of arguments.\n");
    }
}

static void cmd_past_write(uint32_t argc, char *argv[])
{
    if (argc == 3) {
        uint32_t id = atoi(argv[1]);
        char *data = argv[2];
        if (past_write_unit(id, (void*) data, strlen(data)+1)) {
            printf("OK\n");
        } else {
            printf("Failed to write unit %u\n", id);
        }
    } else {
        printf("Error: wrong number of arguments.\n");
    }
}

static void cmd_past_erase(uint32_t argc, char *argv[])
{
    if (argc == 2) {
        uint32_t id = atoi(argv[1]);
        if (past_erase_unit(id)) {
            printf("OK\n");
        } else {
            printf("Failed to erase unit %u\n", id);
        }
    } else {
        printf("Error: wrong number of arguments.\n");
    }
}

static void handle_command(char *cmd)
{
    char *argv[MAX_ARGC];
    int argc = 1;
    char *temp, *rover;
    memset((void*) argv, 0, sizeof(argv));
    argv[0] = cmd;
    rover = cmd;
    // Split string "<command>:<argument 1>:<argument 2>: ... :<argument N>"
    // into argv, argc style
    while(argc < MAX_ARGC && (temp = strstr(rover, ":"))) {
        rover = &(temp[1]);
        argv[argc++] = rover;
        *temp = 0;
    }

#if 0
    printf("argc : %d\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("%d : '%s'\n", i, argv[i]);
    }
#endif

    if (strlen(argv[0]) > 0) {
        if (strcmp(argv[0], "help") == 0) cmd_help(argc, argv);
        else if (strcmp(argv[0], "format") == 0) cmd_past_format(argc, argv);
        else if (strcmp(argv[0], "valid") == 0) printf("Past is %s\n", past_valid() ? "valid" : "invalid! Please format.");
        else if (strcmp(argv[0], "read") == 0) cmd_past_read(argc, argv);
        else if (strcmp(argv[0], "write") == 0) cmd_past_write(argc, argv);
        else if (strcmp(argv[0], "erase") == 0) cmd_past_erase(argc, argv);
        else if (strcmp(argv[0], "size") == 0) printf("Past occupies %u bytes\n", past_size());
        else if (strcmp(argv[0], "dump") == 0) past_dump();
        else printf("Unknown command %s, try 'help'\n", argv[0]);
    }
}

// The program will wait for 1s on boot for a colon character on the uart.
// If found, the past monitor will be invoked.
static bool enter_pastmon(void)
{
    printf("                                                                           \n");
    printf("pastmon? Hit ':' within one second...\n");
    uint32_t start_time = systime_ms();
    while(systime_ms() - start_time < 1000) {
        if (uart0_num_char() > 0) {
            char ch;
            if (read(0, (void*)&ch, 1)) { // 0 is stdin
                if (ch == ':') {
                    return true;
                }
            }
        }
        delay_ms(50);
    }

    return false;
}

static void pastmon()
{
    if (enter_pastmon()) {
        char ch;
        char cmd[81];
        int i = 0;
        printf("\n\n\nWelcome to pastmon. Type 'help\\n' for, well, help\n");
        while(1) {
            if (read(0, (void*)&ch, 1)) { // 0 is stdin
                if (ch != '\n') {
                    if (i < sizeof(cmd)) cmd[i++] = ch;
                } else {
                    cmd[i] = 0;
                    i = 0;
                    if (strcmp(cmd, "quit") == 0)
                        return;
                    else
                        handle_command((char*) cmd);
                }
            }
        }
    }
}


void user_init(void)
{
    sdk_uart_div_modify(0, UART_CLK_FREQ / 115200);
    delay_ms(1000); // Neded for uart to "stabilize", ie less junk characters

    pastmon();

    printf("Normal program boot...\n");
    while(1) {
        delay_ms(1000);
    }
}
