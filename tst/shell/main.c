#include <errno.h>
#include "nala.h"
#include "nala_mocks.h"
#include "async.h"
#include "utils.h"

static void mock_prepare_output(const char *output_p)
{
    channel_write_mock_once(strlen(output_p), strlen(output_p));
    channel_write_mock_set_buf_p_in(output_p, strlen(output_p));
}

static void mock_prepare_command(const char *input_p,
                                 const char *output_p)
{
    while (*input_p != '\0') {
        channel_read_mock_once(1, 1);
        channel_read_mock_set_buf_p_out(input_p, 1);
        input_p++;
    }

    while (*output_p != '\0') {
        channel_write_mock_once(1, 1);
        channel_write_mock_set_buf_p_in(output_p, 1);
        output_p++;
    }
}

static struct async_t async;
static struct async_shell_t shell;
static struct async_channel_t channel;

int setup(void)
{
    async_init(&async, 100);
    async_channel_init(&channel,
                       channel_open,
                       channel_close,
                       channel_read,
                       channel_write,
                       &async);

    async_shell_init(&shell, &channel, &async);
    async_process(&async);

    channel_open_mock_once();
    async_shell_start(&shell);
    async_process(&async);

    return (0);
}

int teardown(void)
{
    channel_close_mock_once();
    async_shell_stop(&shell);
    async_process(&async);

    return (0);
}

TEST(test_command_help)
{
    mock_prepare_command("help\n", "help\n");
    mock_prepare_output(
        "Cursor movement\n"
        "\n"
        "         LEFT   Go left one character.\n"
        "        RIGHT   Go right one character.\n"
        "  HOME/Ctrl+A   Go to the beginning of the line.\n"
        "   END/Ctrl+E   Go to the end of the line.\n"
        "\n"
        "Edit\n"
        "\n"
        "        Alt+D   Delete the word at the cursor.\n"
        "       Ctrl+D   Delete the chracter at the cursor.\n"
        "       Ctrl+K   Cut the line from cursor to end.\n"
        "       Ctrl+T   Swap the last two characters before the cursor "
        "(typo).\n"
        "          TAB   Tab completion for command names.\n"
        "    BACKSPACE   Delete the character before the cursor.\n"
        "\n"
        "History\n"
        "\n"
        "           UP   Previous command.\n"
        "         DOWN   Next command.\n"
        "\n"
        "Commands\n"
        "\n");
    mock_prepare_output("         help   Print this help.\n");
    mock_prepare_output("      history   List command history.\n");
    mock_prepare_output("OK\n");
    mock_prepare_output("$ ");
    async_channel_input(&channel);
    async_process(&async);
}

TEST(test_command_history)
{
    mock_prepare_command("history\n", "history\n");
    mock_prepare_output("1");
    mock_prepare_output(": ");
    mock_prepare_output("history");
    mock_prepare_output("\n");
    mock_prepare_output("OK\n");
    mock_prepare_output("$ ");
    async_channel_input(&channel);
    async_process(&async);
}
