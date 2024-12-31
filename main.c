#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <locale.h>

#include "ASCII_ART.h"
#include "STRUCTS.h"

void draw_border();
void welcome_screen();
void generate_map();
void print_level(int level_num);
void print_room(int level_num, int room_num);

Level level[4];

int main()
{
    setlocale(LC_ALL, "");
    srand(time(0));

    initscr();
    curs_set(FALSE);
    keypad(stdscr, TRUE);
    noecho();

    draw_border();
    welcome_screen();
    generate_map();

    for (int i = 0; i < 4; i++)
    {
        draw_border();
        print_level(i);
        getch();
    }

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
    int sleep_time = 30000;

    for (int i = 0; i < 23; i++)
    {
        move((LINES / 2) - 15 + i, (COLS / 2) - 51);
        printw("%s", game_title_design_3[i]);
        refresh();
        usleep(sleep_time);
    }

    usleep(700000);

    move((LINES / 2) + 15, ((COLS - strlen(game_start)) / 2) - 1);
    printw("%s", game_start);
    refresh();

    getch();
    refresh();
}

void generate_map()
{
    for (int i = 0; i < 4; i++)
    {
        level[i].room_num = (rand() % (MAX_ROOM_NUM - 6 + 1)) + 6;
        for (int j = 0; j < level[i].room_num; j++)
        {
            Point temp_corner;
            float ratio;
            int temp_length, temp_width;

            while (1)
            {
                int found = 1;

                temp_corner.y = rand() % (LINES - 2) + 2;
                temp_corner.x = rand() % (COLS - 2) + 2;

                ratio = ((float)rand() / RAND_MAX) * (MAX_ROOM_RATIO - MIN_ROOM_RATIO) + MAX_ROOM_RATIO;
                temp_length = rand() % (MAX_ROOM_LENGTH - MIN_ROOM_LENGTH + 1) + MIN_ROOM_LENGTH;
                temp_width = temp_length / ratio;

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

                if (found == 1 && temp_corner.y + temp_width < LINES - MARGIN && temp_corner.x + temp_length < COLS - MARGIN &&
                    temp_corner.y > MARGIN + 1 && temp_corner.x > MARGIN + 1)
                {
                    break;
                }
            }

            level[i].room[j].upper_left_corner = temp_corner;
            level[i].room[j].length = temp_length;
            level[i].room[j].width = temp_width;
        }
    }
}

void print_level(int level_num)
{
    clear();
    draw_border();

    for (int i = 0; i < level[level_num].room_num; i++)
        print_room(level_num, i);
}

void print_room(int level_num, int room_num)
{
    int x = level[level_num].room[room_num].upper_left_corner.x;
    int y = level[level_num].room[room_num].upper_left_corner.y;
    int length = level[level_num].room[room_num].length;
    int width = level[level_num].room[room_num].width;

    for (int i = 0; i < length; i++)
    {
        mvprintw(y, x + i, "_");
        for (int j = 1; j < width + 1; j++)
        {
            mvprintw(y + j, x + i, "·"); // middle dot - ASCII Number: 183
        }
        mvprintw(y + width + 1, x + i, "‾"); // overline - UTF-16 Encoding: 0x203E
    }

    for (int i = 0; i < width; i++)
    {
        mvprintw(y + i + 1, x - 1, "|");
        mvprintw(y + i + 1, x + length, "|");
    }

    refresh();
}