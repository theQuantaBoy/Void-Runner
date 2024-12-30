#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ASCII_ART.h"

void draw_border();
void welcome_screen();

int main()
{
    initscr();
    curs_set(FALSE);
    keypad(stdscr, TRUE);
    noecho();

    draw_border();
    welcome_screen();

    endwin();
    clear();

    return 0;
}

void draw_border()
{
    for (int i = 0; i < COLS; i++)
    {
        mvprintw(0, i, "#");
        mvprintw(LINES - 1, i, "#");
    }

    for (int i = 0; i < LINES; i++)
    {
        mvprintw(i, 0, "#");
        mvprintw(i, COLS - 1, "#");
    }

    refresh();
}

void welcome_screen()
{
    char game_start[] = "- Press any key to continue -";

    for (int i = 0; i < 23; i++)
    {
        move((LINES / 2) - 15 + i, (COLS / 2) - 47);
        printw("%s", game_title[i]);
        refresh();
        usleep(30000);
    }

    usleep(700000);

    move((LINES / 2) + 15, ((COLS - strlen(game_start)) / 2) - 1);
    printw("%s", game_start);
    refresh();

    getch();
    clear();
}