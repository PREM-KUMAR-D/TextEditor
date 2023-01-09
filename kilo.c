/*
 **** includes *****
 */
#include <ctype.h>
#include <unistd.h>
#include <stdio.h>
#include <termios.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ioctl.h>
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
 **** Defines *****
 */
#define CTRL_KEYS(k) ((k)&0x1f)
// TODO - Check why we are anding with 1f

/*
 **** Data *****
 */
struct editorConfig{
    int nScreenRows;
    int nScreenColumns;
    struct termios original_termios;
};
struct editorConfig E;
/*
 **** Terminal *****
 */
void die(const char *s)
{
    // clear screen before error exit
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
    perror(s);
    exit(1);
}
void disableRawMode()
{
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.original_termios) == -1)
        die("tcsetattr");
}
void enableRawMode()
{

    if (tcgetattr(STDIN_FILENO, &E.original_termios) == -1)
        die("tcgetattr");
    atexit(disableRawMode); // is called at the exit of the executable not this function alone
    struct termios raw = E.original_termios;
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
char editorReadKey()
{

    int nRead;
    char ch;
    while ((nRead = read(STDIN_FILENO, &ch, 1)) != 1)
    {
        if (nRead == -1 && errno != EAGAIN)
            die("read");
    }
    return ch;
}
int getWindowSize(int *rows,int *cols){
    struct winsize ws;
    
    if(ioctl(STDOUT_FILENO,TIOCGWINSZ,&ws)==-1 || ws.ws_col==0)
        return -1;
    else{
        *cols=ws.ws_col;
        *rows=ws.ws_row;
        return 0;
    }
    
}
/*
    Input
*/

void editorProcessKeypress()
{
    char c = editorReadKey();
    switch (c)
    {
    case CTRL_KEYS('q'):
        write(STDOUT_FILENO, "\x1b[2J", 4);
        write(STDOUT_FILENO, "\x1b[H", 3);
        exit(0);
        break;
    }
}
/*
    Outupt
*/
void editorDrawRows(){
    int y;
    for(y=0;y<E.nScreenRows;y++){
        write(STDOUT_FILENO,"~\r\n",3);
    }
}
void editorRefreshScreen(){
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
    editorDrawRows();
    write(STDOUT_FILENO,"\x1b[H",3);
}

/*
 **** Init *****
 */
void initEditor(){
    if(getWindowSize(&E.nScreenRows,&E.nScreenColumns)==-1 ) die("getWindowSize");
}
int main()
{
    enableRawMode();
    initEditor();
    while (1)
    {
        editorRefreshScreen();
        editorProcessKeypress();
    }

    return 0;
}