#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ASCII_ART.h"
#include "STRUCTS.h"

void draw_border();
void welcome_screen();
void generate_map();
void print_level();

int main()
{
    initscr();
    curs_set(FALSE);
    keypad(stdscr, TRUE);
    noecho();

    draw_border();
    welcome_screen();
    generate_map();

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
        move((LINES / 2) - 15 + i, (COLS / 2) - 51);
        printw("%s", game_title_design_3[i]);
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

void generate_map()
{
    Level level[4];

    for (int i = 0; i < 4; i++)
    {
        level[i].room_num = (rand() % (MAX_ROOM_NUM - 6 + 1)) + 6;
        for (int j = 0; j < level[i].room_num; j++)
        {
            Point temp_corner;
            int temp_length, temp_width;

            while (1)
            {
                int found = 1;

                temp_corner.y = rand() % (LINES - 2) + 1;
                temp_corner.x = rand() % (COLS - 2) + 1;

                temp_length = rand() % (MAX_ROOM_LENGTH - 6 + 1) + 6;
                temp_width = rand() % (MAX_ROOM_LENGTH - 6 + 1) + 6;

                for (int k = 0; k < j; k++)
                {
                    int x = level[i].room[k].upper_left_corner.x;
                    int y = level[i].room[k].upper_left_corner.y;
                    int length = level[i].room[k].length;
                    int width = level[i].room[k].width;

                    if (temp_corner.x + temp_length >= x - MIN_DISTANCE && temp_corner.x <= x + length + MIN_DISTANCE)
                    {
                        if (temp_corner.y + temp_width >= y - MIN_DISTANCE && temp_corner.y <= y + width + MIN_DISTANCE)
                        {
                            found = 0;
                            break;
                        }
                    }
                }

                if (found == 1)
                    break;
            }

            level[i].room[j].upper_left_corner = temp_corner;
            level[i].room[j].length = temp_length;
            level[i].room[j].width = temp_width;
        }
    }
}
