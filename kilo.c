#include<unistd.h>
#include<stdio.h>
#include<termios.h>
#include<stdlib.h>
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

    


    

*/
struct  termios original_termios;
void disableRawMode(){
    tcsetattr(STDIN_FILENO,TCSAFLUSH,&original_termios);
}
void enableRawMode(){
    
    tcgetattr(STDIN_FILENO,&original_termios);
    atexit(disableRawMode);// is called at the exit of the executable not this function alone
    struct termios raw=original_termios;
    // unset the fourth bit 
    raw.c_cflag &=~(ECHO | ICANON);// turn off canonical and echo mode
    // write the values back to terminal
    tcsetattr(STDIN_FILENO,TCSAFLUSH,&raw);
    
}

int main(){
    char c;
    // we have a function to read and modify the terminal atributes
    enableRawMode();
    while (read(STDIN_FILENO,&c,1)==1 && c!='q')
    {
        /* code */
        printf("The buffer value is %c \n",c);
    }
    
    return 0;
}