#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ASCII_ART.h"

void welcome_screen();

int main()
{
    initscr();
    curs_set(FALSE);
    keypad(stdscr, TRUE);
    noecho();

    welcome_screen();

    getch();
    endwin();
    clear();

    return 0;
}

void welcome_screen()
{
    char game_start[] = "- Press any key to continue -";

    for (int i = 0; i < 23; i++)
    {
        move((LINES / 2) - 17 + i, (COLS / 2) - 48);
        printw("%s\n", game_title[i]);
        refresh();
        usleep(30000);
    }

    usleep(500000);

    move((LINES / 2) + 8, ((COLS - strlen(game_start)) / 2) - 1);
    printw("%s\n", game_start);
    refresh();
}