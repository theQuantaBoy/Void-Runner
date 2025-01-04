#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <locale.h>
#include <ctype.h>
#include <stdbool.h>
#include <regex.h>
#include "FUNCTIONS.h"

int main()
{
    setlocale(LC_ALL, "");
    srand(time(0));

    initscr();
    curs_set(FALSE);
    keypad(stdscr, TRUE);
    noecho();

    draw_border();
    title_screen();

    int choice = welcome_screen();

    if (choice == 1)
        new_account_screen();
    else if (choice == 0)
        continue_game_screen();

    endwin();
    clear();

    return 0;
}
