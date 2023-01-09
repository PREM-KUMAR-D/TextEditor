/*
 **** includes *****
 */
#include <ctype.h>
#include <unistd.h>
#include <stdio.h>
#include <termios.h>
#include <stdlib.h>
#include <errno.h>
/*
    DOC

    By default the terminal starts in canonical mode that is why it waits for the user to press enter.

    We want to set this to raw mode instead since "But it does not work well for programs with more complex user interfaces,
     like text editors. We want to process each keypress as it comes in, so we can respond to it immediately"

    The TCSAFLUSH argument specifies when to apply the change: in this case, it waits for all pending output
    to be written to the terminal, and also discards any input that hasn’t been read.

    atexit() comes from <stdlib.h>. We use it to register our disableRawMode()
    function to be called automatically when the program exits, whether it exits by returning from main(),
    or by calling the exit() function.

    Characters that cannot be printed on the screen are known as control characters.

    Reference for pause and resume transmission https://en.wikipedia.org/wiki/Software_flow_control

    ctrl + c -- SIGINT
    ctrl + z -- SIGSTP







*/

/*
 **** Data *****
 */
struct termios original_termios;
/*
 **** Terminal *****
 */
void die(const char *s)
{
    perror(s);
    exit(1);
}
void disableRawMode()
{
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios) == -1)
        die("tcsetattr");
}
void enableRawMode()
{

    if (tcgetattr(STDIN_FILENO, &original_termios) == -1)
        die("tcgetattr");
    atexit(disableRawMode); // is called at the exit of the executable not this function alone
    struct termios raw = original_termios;
    raw.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
    // unset the fourth bit
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN); // turn off canonical and echo mode
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8); // set character size to 8 bit per byte
    // write the values back to terminal
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        die("tcsetattr");
}
/*
 **** Init *****
 */

int main()
{
    char c;
    // we have a function to read and modify the terminal atributes
    enableRawMode();
    while (1)
    {
        char c = '\0'; // set to null
        if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN)
            die("read");
        if (iscntrl(c))
            printf("%d\r\n", c);
        else
            printf("%d ('%c')\r\n", c, c);

        if (c == 'q')
            break;
    }

    return 0;
}