#include "../inc/terminal.h"
#include "../inc/emulator.h"
#include <termios.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <errno.h>
static struct termios terminalPrev;
static struct termios terminalSettings;

int Terminal::descriptor = -1;


void Terminal::signal(int num){
    if (descriptor != -1)
        tcsetattr(descriptor, TCSANOW, &terminalPrev);

    /* exit() is not async-signal safe, but _exit() is.
     * Use the common idiom of 128 + signal number for signal exits.
     * Alternative approach is to reset the signal to default handler,
     * and immediately raise() it. */
    _exit(128 + num);
}
Terminal::Terminal(Emulator* e){
    this->e = e;
    struct sigaction act;

    /* Which standard stream is connected to our TTY? */
    if (isatty(STDERR_FILENO))
        descriptor = STDERR_FILENO;
    else
    if (isatty(STDIN_FILENO))
        descriptor = STDIN_FILENO;
    else
    if (isatty(STDOUT_FILENO))
        descriptor = STDOUT_FILENO;
    else
        return;

    /* Obtain terminal settings. */
    if (tcgetattr(descriptor, &terminalPrev) ||
        tcgetattr(descriptor, &terminalSettings))
        return;

    /* Disable buffering for terminal streams. */
    if (isatty(STDIN_FILENO))
        setvbuf(stdin, NULL, _IONBF, 0);
    if (isatty(STDOUT_FILENO))
        setvbuf(stdout, NULL, _IONBF, 0);
    if (isatty(STDERR_FILENO))
        setvbuf(stderr, NULL, _IONBF, 0);

    /* At exit() or return from main(),
     * restore the original settings. */
    if (atexit(Terminal::restore))
        return;

    /* Set new "default" handlers for typical signals,
     * so that if this process is killed by a signal,
     * the terminal settings will still be restored first. */
    sigemptyset(&act.sa_mask);
    act.sa_handler = Terminal::signal;
    act.sa_flags = 0;
    if (sigaction(SIGHUP,  &act, NULL) ||
        sigaction(SIGINT,  &act, NULL) ||
        sigaction(SIGQUIT, &act, NULL) ||
        sigaction(SIGTERM, &act, NULL) ||
#ifdef SIGXCPU
        sigaction(SIGXCPU, &act, NULL) ||
#endif
#ifdef SIGXFSZ
        sigaction(SIGXFSZ, &act, NULL) ||
#endif
#ifdef SIGIO
        sigaction(SIGIO,   &act, NULL) ||
#endif
        sigaction(SIGPIPE, &act, NULL) ||
        sigaction(SIGALRM, &act, NULL))
        return ;

  terminalSettings.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN);
  terminalSettings.c_cflag &= ~(CSIZE | PARENB);
  terminalSettings.c_cflag |= CS8;
  terminalSettings.c_cc[VMIN] = 0;
  terminalSettings.c_cc[VTIME] = 0;

    tcsetattr(descriptor, TCSANOW, &terminalSettings);

    return ;
}

void Terminal::restore(){
    if (descriptor != -1)
        tcsetattr(descriptor, TCSANOW, &terminalPrev);
}

void Terminal::readInput(){
    char c;
    if (read(STDIN_FILENO, &c, 1) == 1) {
        //cout << c << endl;
        //cout << "click!" << endl;
        e->mem[in] = 0;
        e->mem[in+1] = c;
        e->int_req |= 1 << 3;
    }
}

Terminal::~Terminal(){
    restore();
}