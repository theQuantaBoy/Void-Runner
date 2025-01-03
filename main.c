#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <locale.h>
#include <ctype.h>

#include "ASCII_ART.h"
#include "STRUCTS.h"

void draw_border();
void title_screen();
int welcome_screen();
void draw_welcome_screen_border();
void draw_new_account_screen();
void new_account_screen();
void generate_map();
void print_level(int level_num);
void print_room(int level_num, int room_num);

Level level[4];
User current_user;

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
    generate_map();

    int choice = welcome_screen();

    for (int i = 0; i < 4; i++)
    {
        draw_border();
        print_level(i);
        getch();
    }

    new_account_screen();
    // getch();

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

void title_screen()
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

void draw_welcome_screen_border()
{
    clear();
    draw_border();
    mvprintw((LINES / 2) - 5, (COLS / 2) - 18, "__ Welcome _________________________");
    mvprintw((LINES / 2) + 5, (COLS / 2) - 18, "‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾");
    for (int i = 0; i < 9; i++)
    {
        mvprintw((LINES / 2) - 4 + i, (COLS / 2) - 18, "|");
        mvprintw((LINES / 2) - 4 + i, (COLS / 2) + 17, "|");
    }
}

int welcome_screen()
{
    int choice = 0;
    while (1)
    {
        draw_welcome_screen_border();

        for (int i = 0; i < 4; i++)
        {
            if (i == choice)
                attron(A_REVERSE);
            mvprintw((LINES / 2) - 3 + (2 * i), (COLS / 2) - 9, "%s", welcome_options[i]);
            if (i == choice)
                attroff(A_REVERSE);
        }

        int key = getch();
        if (key == KEY_DOWN)
            choice = (choice + 1) % 4;
        if (key == KEY_UP)
            choice = (choice + 3) % 4;

        if (key == 10 || key == 32) // 10: Enter , 32: Space
            break;
    }

    return choice;
}

void draw_new_account_screen(int show_password)
{
    clear();
    draw_border();
    mvprintw((LINES / 2) - 8, (COLS / 2) - 24, "__ New Account _________________________________");
    mvprintw((LINES / 2) - 6, (COLS / 2) - 17, "Username: ");
    mvprintw((LINES / 2) - 4, (COLS / 2) - 17, "Email: ");
    mvprintw((LINES / 2) - 2, (COLS / 2) - 17, "Password: ");
    mvprintw((LINES / 2), (COLS / 2) - 17, "[%c] Show Password", show_password ? 'X' : ' ');
    mvprintw((LINES / 2), (COLS / 2) + 7, "Confirm");
    mvprintw((LINES / 2) + 2, (COLS / 2) - 17, "Generate random password (Beta)");
    mvprintw((LINES / 2) + 4, (COLS / 2) - 24, "‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾");
    for (int i = 0; i < 11; i++)
    {
        mvprintw((LINES / 2) - 7 + i, (COLS / 2) - 24, "|");
        mvprintw((LINES / 2) - 7 + i, (COLS / 2) + 23, "|");
    }
}

void new_account_screen()
{
    char username[26] = {};
    char email[29] = {};
    char password[26] = {};

    int show_password = 0;
    int choice = 0;
    int key;

    while (1)
    {
        noecho();
        draw_new_account_screen(show_password);

        mvprintw((LINES / 2) - 6, (COLS / 2) - 7, "%s", username);
        mvprintw((LINES / 2) - 4, (COLS / 2) - 10, "%s", email);

        if (show_password == 0)
        {
            move((LINES / 2) - 2, (COLS / 2) - 7);
            for (int i = 0; i < strlen(password); i++)
                printw("*");
        }
        else
        {
            move((LINES / 2) - 2, (COLS / 2) - 7);
            for (int i = 0; i < strlen(password); i++)
                printw("%c", password[i]);
        }

        if (choice == 3)
        {
            attron(A_REVERSE);
            mvprintw((LINES / 2), (COLS / 2) - 17, "[");
            printw("%c", show_password ? 'X' : ' ');
            printw("] Show Password");
            attroff(A_REVERSE);
        }
        else if (choice == 4)
        {
            attron(A_REVERSE);
            mvprintw((LINES / 2), (COLS / 2) + 7, "Confirm");
            attroff(A_REVERSE);
        }
        else if (choice == 5)
        {
            attron(A_REVERSE);
            mvprintw((LINES / 2) + 2, (COLS / 2) - 17, "Generate random password (Beta)");
            attroff(A_REVERSE);
        }

        if (choice < 3)
        {
            echo();
            curs_set(1);
            if (choice == 0)
                move((LINES / 2) - 6, (COLS / 2) - 7 + strlen(username));
            else if (choice == 1)
                move((LINES / 2) - 4, (COLS / 2) - 10 + strlen(email));
            else if (choice == 2)
                move((LINES / 2) - 2, (COLS / 2) - 7 + strlen(password));
        }
        else
            curs_set(0);

        int key = getch();

        if (choice == 0 && strlen(username) < sizeof(username) - 1 && isprint(key))
        {
            username[strlen(username)] = key;
        }
        else if (choice == 1 && strlen(email) < sizeof(email) - 1 && isprint(key))
        {
            email[strlen(email)] = key;
        }
        else if (choice == 2 && strlen(password) < sizeof(password) - 1 && isprint(key))
        {
            password[strlen(password)] = key;
        }
        else if (key == KEY_BACKSPACE || key == 127) // Handle backspace
        {
            if (choice == 0 && strlen(username) > 0)
                username[strlen(username) - 1] = '\0';
            else if (choice == 1 && strlen(email) > 0)
                email[strlen(email) - 1] = '\0';
            else if (choice == 2 && strlen(password) > 0)
                password[strlen(password) - 1] = '\0';
        }

        if (choice == 4 && (key == 10 || key == 32))
        {
            strcpy(current_user.username, username);
            strcpy(current_user.email, email);
            strcpy(current_user.password, password);

            break;
        }

        else if (choice == 3 && (key == 10 || key == 32))
            show_password = !show_password;

        else if (key == KEY_DOWN)
        {
            if (choice == 3 || choice == 4)
                choice = 5;
            else
                choice = (choice + 1) % 6;
        }

        else if (key == KEY_UP)
        {
            if (choice == 5)
                choice = 3;
            else
                choice = (choice + 5) % 6;
        }

        else if (choice == 3 || choice == 4 && (key == KEY_LEFT || key == KEY_RIGHT))
            choice = (choice % 2) + 3;

        else if ((choice == 0 || choice == 1 || choice == 2) && key == 10)
            choice += 1;
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