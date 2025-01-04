#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <locale.h>
#include <ctype.h>
#include <stdbool.h>
#include <regex.h>

#include "ASCII_ART.h"
#include "STRUCTS.h"

void draw_border();
void title_screen();
int welcome_screen();
void draw_welcome_screen_border();
void draw_new_account_screen();
void new_account_screen();
void save_user_data();
bool username_unique();
bool email_unique();
bool email_valid();
bool password_long_enough();
bool password_valid();
void print_user_taken();
void print_email_taken();
void print_wrong_email_format();
void print_password_short();
void print_wrong_password_format();
void generate_map();
void print_level(int level_num);
void print_room(int level_num, int room_num);
void draw_sign_in_screen(int option, int show_password);
void continue_game_screen();
bool password_correct(int user_index, char password[26]);
void print_user_not_found();
void print_email_not_found();
void print_wrong_password();
int username_found(char username[26]);
int email_found(char email[29]);

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

    int choice = welcome_screen();

    if (choice == 1)
        new_account_screen();
    else if (choice == 0)
        continue_game_screen();

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
    int choice = 0, error = 0;

    while (1)
    {
        noecho();
        draw_new_account_screen(show_password);

        if (error == 1)
            print_user_taken();
        else if (error == 2)
            print_email_taken();
        else if (error == 3 && strlen(email) > 0)
            print_wrong_email_format();
        else if (error == 4)
            print_password_short();
        else if (error == 5)
            print_wrong_password_format();

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
        else if (key == KEY_BACKSPACE || key == 127)
        {
            if (choice == 0 && strlen(username) > 0)
                username[strlen(username) - 1] = '\0';
            else if (choice == 1 && strlen(email) > 0)
                email[strlen(email) - 1] = '\0';
            else if (choice == 2 && strlen(password) > 0)
                password[strlen(password) - 1] = '\0';
        }

        if (!username_unique(username))
            error = 1;
        else if (!email_unique(email))
            error = 2;
        else if (strlen(email) > 0 && !email_valid(email))
            error = 3;
        else if (strlen(password) > 0 && !password_long_enough(password))
            error = 4;
        else if (strlen(password) > 0 && !password_valid(password))
            error = 5;
        else
            error = 0;

        if (error == 0 && choice == 4 && (key == 10 || key == 32))
        {
            strcpy(current_user.username, username);
            strcpy(current_user.email, email);
            strcpy(current_user.password, password);

            save_user_data(username, email, password);

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

void save_user_data(char username[26], char email[29], char password[26])
{
    FILE *users = fopen("users.csv", "a");
    fprintf(users, "%s,%s,%s\n", username, email, password);
    fclose(users);
}

bool username_unique(char username[26]) // Error 1
{
    FILE *users = fopen("users.csv", "r");

    if (users == NULL)
    {
        perror("Error opening file");
        return false;
    }

    char file_line[1024];
    char file_username[256];

    if (fgets(file_line, sizeof(file_line), users) != NULL)
    {
        sscanf(file_line, "%255[^,]", file_username);
        if (strcmp(file_username, username) == 0)
        {
            fclose(users);
            return false;
        }
    }

    while (fgets(file_line, sizeof(file_line), users))
    {
        sscanf(file_line, "%255[^,]", file_username);
        if (strcmp(file_username, username) == 0)
        {
            fclose(users);
            return false;
        }
    }

    fclose(users);
    return true;
}

bool email_unique(char email[29]) // Error 2
{
    FILE *users = fopen("users.csv", "r");

    if (users == NULL)
    {
        perror("Error opening file");
        return false;
    }

    char file_line[1024];
    char file_email[256];

    if (fgets(file_line, sizeof(file_line), users) != NULL)
    {
        sscanf(file_line, "%255[^,]", file_email);
        mvprintw(5, 5, "%s\n", file_email);
        if (strcmp(file_email, email) == 0)
        {
            fclose(users);
            return false;
        }
    }

    while (fgets(file_line, sizeof(file_line), users))
    {
        char trash[256];
        sscanf(file_line, "%255[^,],%255[^,],", trash, file_email);
        if (strcmp(file_email, email) == 0)
        {
            fclose(users);
            return false;
        }
    }

    fclose(users);
    return true;
}

bool email_valid(char email[29]) // Error 3
{
    regex_t regex;
    int ret;

    const char *pattern = "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$";

    ret = regcomp(&regex, pattern, REG_EXTENDED);
    ret = regexec(&regex, email, 0, NULL, 0);

    regfree(&regex);

    if (ret == 0)
        return true;

    else if (ret == REG_NOMATCH)
        return false;
}

bool password_long_enough(char password[26]) // Error 4
{
    if (strlen(password) < 7)
        return false;
    return true;
}

bool password_valid(char password[26]) // Error 5
{
    int upper = 0, lower = 0, digit = 0;
    for (int i = 0; i < strlen(password); i++)
    {
        if (password[i] >= 'A' && password[i] <= 'Z')
            upper = 1;
        else if (password[i] >= 'a' && password[i] <= 'z')
            lower = 1;
        else if (password[i] >= '0' && password[i] <= '9')
            digit = 1;
    }

    if (upper && lower && digit)
        return true;
    return false;
}

void print_user_taken() // Error 1
{
    mvprintw((LINES / 2) + 5, (COLS / 2) - 24, "__ Error _______________________________________");
    mvprintw((LINES / 2) + 6, (COLS / 2) - 24, "|                                              |");
    mvprintw((LINES / 2) + 7, (COLS / 2) - 24, "|      Usernmae already taken. Try again!      |");
    mvprintw((LINES / 2) + 8, (COLS / 2) - 24, "|                                              |");
    mvprintw((LINES / 2) + 9, (COLS / 2) - 24, "‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾");
}

void print_email_taken() // Error 2
{
    mvprintw((LINES / 2) + 5, (COLS / 2) - 24, "__ Error _______________________________________");
    mvprintw((LINES / 2) + 6, (COLS / 2) - 24, "|                                              |");
    mvprintw((LINES / 2) + 7, (COLS / 2) - 24, "|       Email already taken. Try again!        |");
    mvprintw((LINES / 2) + 8, (COLS / 2) - 24, "|                                              |");
    mvprintw((LINES / 2) + 9, (COLS / 2) - 24, "‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾");
}

void print_wrong_email_format() // Error 3
{
    mvprintw((LINES / 2) + 5, (COLS / 2) - 24, "__ Error _______________________________________");
    mvprintw((LINES / 2) + 6, (COLS / 2) - 24, "|                                              |");
    mvprintw((LINES / 2) + 7, (COLS / 2) - 24, "|      Email format is wrong. Try again!       |");
    mvprintw((LINES / 2) + 8, (COLS / 2) - 24, "|                                              |");
    mvprintw((LINES / 2) + 9, (COLS / 2) - 24, "‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾");
}

void print_password_short() // Error 4
{
    mvprintw((LINES / 2) + 5, (COLS / 2) - 24, "__ Error _______________________________________");
    mvprintw((LINES / 2) + 6, (COLS / 2) - 24, "|                                              |");
    mvprintw((LINES / 2) + 7, (COLS / 2) - 24, "|      Password is too short. Try again!       |");
    mvprintw((LINES / 2) + 8, (COLS / 2) - 24, "|                                              |");
    mvprintw((LINES / 2) + 9, (COLS / 2) - 24, "‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾");
}

void print_wrong_password_format() // Error 5
{
    mvprintw((LINES / 2) + 5, (COLS / 2) - 24, "__ Error _______________________________________");
    mvprintw((LINES / 2) + 6, (COLS / 2) - 24, "|                                              |");
    mvprintw((LINES / 2) + 7, (COLS / 2) - 24, "|      Password must contain at least one      |");
    mvprintw((LINES / 2) + 8, (COLS / 2) - 24, "|    lowercase letter, one uppercase letter,   |");
    mvprintw((LINES / 2) + 9, (COLS / 2) - 24, "|           and one digit. Try again!          |");
    mvprintw((LINES / 2) + 10, (COLS / 2) - 24, "|                                              |");
    mvprintw((LINES / 2) + 11, (COLS / 2) - 24, "‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾");
}

void draw_sign_in_screen(int option, int show_password)
{
    clear();
    draw_border();

    if (option == 0)
    {
        mvprintw((LINES / 2) - 8, (COLS / 2) - 24, "__ Continue Game _______________________________");
        mvprintw((LINES / 2) - 6, (COLS / 2) - 17, "Username: ");
        mvprintw((LINES / 2) - 4, (COLS / 2) - 17, "Password: ");
        mvprintw((LINES / 2) - 2, (COLS / 2) - 17, "[%c] Show Password", show_password ? 'X' : ' ');
        mvprintw((LINES / 2) - 2, (COLS / 2) + 7, "Sign In");
        mvprintw((LINES / 2), (COLS / 2) - 10, "Continue with Email");
        mvprintw((LINES / 2) + 2, (COLS / 2) - 24, "‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾");

        for (int i = 0; i < 9; i++)
        {
            mvprintw((LINES / 2) - 7 + i, (COLS / 2) - 24, "|");
            mvprintw((LINES / 2) - 7 + i, (COLS / 2) + 23, "|");
        }
    }

    else if (option == 1)
    {
        mvprintw((LINES / 2) - 8, (COLS / 2) - 24, "__ Continue Game _______________________________");
        mvprintw((LINES / 2) - 6, (COLS / 2) - 17, "Email: ");
        mvprintw((LINES / 2) - 4, (COLS / 2) - 17, "Password: ");
        mvprintw((LINES / 2) - 2, (COLS / 2) - 17, "[%c] Show Password", show_password ? 'X' : ' ');
        mvprintw((LINES / 2) - 2, (COLS / 2) + 7, "Sign In");
        mvprintw((LINES / 2), (COLS / 2) - 11, "Continue with Username");
        mvprintw((LINES / 2) + 2, (COLS / 2) - 24, "‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾");

        for (int i = 0; i < 9; i++)
        {
            mvprintw((LINES / 2) - 7 + i, (COLS / 2) - 24, "|");
            mvprintw((LINES / 2) - 7 + i, (COLS / 2) + 23, "|");
        }
    }
}

void continue_game_screen()
{
    char username[26] = {};
    char email[29] = {};
    char password[26] = {};

    int show_password = 0;
    int choice = 0, error = 0, user_index = -2;
    int option = 0; // 0: username - 1: email

    while (1)
    {
        noecho();
        draw_sign_in_screen(option, show_password);

        if (option == 0 && user_index == -1)
            error = 1;
        else if (option == 1 && user_index == -1)
            error = 2;
        else if (strlen(password) > 0 && !password_correct(user_index, password))
            error = 3;
        else
            error = 0;

        if (error == 1)
            print_user_not_found();
        else if (error == 2)
            print_email_not_found();
        else if (error == 3 && strlen(password) > 0)
            print_wrong_password();

        if (option == 0)
            mvprintw((LINES / 2) - 6, (COLS / 2) - 7, "%s", username);
        else if (option == 1)
            mvprintw((LINES / 2) - 6, (COLS / 2) - 10, "%s", email);

        if (show_password == 0)
        {
            move((LINES / 2) - 4, (COLS / 2) - 7);
            for (int i = 0; i < strlen(password); i++)
                printw("*");
        }
        else
        {
            move((LINES / 2) - 4, (COLS / 2) - 7);
            for (int i = 0; i < strlen(password); i++)
                printw("%c", password[i]);
        }

        if (choice == 2)
        {
            attron(A_REVERSE);
            mvprintw((LINES / 2) - 2, (COLS / 2) - 17, "[");
            printw("%c", show_password ? 'X' : ' ');
            printw("] Show Password");
            attroff(A_REVERSE);
        }
        else if (choice == 3)
        {
            attron(A_REVERSE);
            mvprintw((LINES / 2) - 2, (COLS / 2) + 7, "Sign In");
            attroff(A_REVERSE);
        }
        else if (choice == 4)
        {
            if (option == 0)
            {
                attron(A_REVERSE);
                mvprintw((LINES / 2), (COLS / 2) - 10, "Continue with Email");
                attroff(A_REVERSE);
            }

            else if (option == 1)
            {
                attron(A_REVERSE);
                mvprintw((LINES / 2), (COLS / 2) - 11, "Continue with Username");
                attroff(A_REVERSE);
            }
        }

        if (choice < 2)
        {
            echo();
            curs_set(1);
            if (choice == 0 && option == 0)
                move((LINES / 2) - 6, (COLS / 2) - 7 + strlen(username));
            else if (choice == 0 && option == 1)
                move((LINES / 2) - 6, (COLS / 2) - 10 + strlen(email));
            else if (choice == 1)
                move((LINES / 2) - 4, (COLS / 2) - 7 + strlen(password));
        }
        else
            curs_set(0);

        int key = getch();

        if (choice == 0 && option == 0 && strlen(username) < sizeof(username) - 1 && isprint(key))
        {
            username[strlen(username)] = key;
        }
        else if (choice == 0 && option == 1 && strlen(email) < sizeof(email) - 1 && isprint(key))
        {
            email[strlen(email)] = key;
        }
        else if (choice == 1 && strlen(password) < sizeof(password) - 1 && isprint(key))
        {
            password[strlen(password)] = key;
        }
        else if ((choice == 0 || choice == 1) && (key == KEY_BACKSPACE || key == 127))
        {
            if (choice == 0 && option == 0 && strlen(username) > 0)
                username[strlen(username) - 1] = '\0';
            else if (choice == 0 && option == 1 && strlen(email) > 0)
                email[strlen(email) - 1] = '\0';
            else if (choice == 1 && strlen(password) > 0)
                password[strlen(password) - 1] = '\0';
        }

        if (option == 0)
            user_index = username_found(username);
        else if (option == 1)
            user_index = email_found(email);

        if (error == 0 && choice == 3 && (key == 10 || key == 32))
        {
            break;
        }

        else if (choice == 4 && (key == 10 || key == 32))
            option = !option;

        else if (choice == 2 && (key == 10 || key == 32))
            show_password = !show_password;

        else if (key == KEY_DOWN)
        {
            if (choice == 2 || choice == 3)
                choice = 4;
            else
                choice = (choice + 1) % 5;
        }

        else if (key == KEY_UP)
        {
            if (choice == 4)
                choice = 2;
            else
                choice = (choice + 4) % 5;
        }

        else if ((choice == 2 || choice == 3) && (key == KEY_LEFT || key == KEY_RIGHT))
            choice = ((choice + 1) % 2) + 2;

        else if ((choice == 0 || choice == 1) && (key == 10 || key == 32))
            choice += 1;
    }
}

int username_found(char username[26]) // Error 1
{
    FILE *users = fopen("users.csv", "r");

    if (users == NULL)
    {
        perror("Error opening file");
        return -1;
    }

    int user_index = 0;

    char file_line[1024];
    char file_username[256];

    if (fgets(file_line, sizeof(file_line), users) != NULL)
    {
        sscanf(file_line, "%255[^,]", file_username);
        if (strcmp(file_username, username) == 0)
        {
            fclose(users);
            return user_index;
        }

        user_index += 1;
    }

    while (fgets(file_line, sizeof(file_line), users))
    {
        sscanf(file_line, "%255[^,]", file_username);
        if (strcmp(file_username, username) == 0)
        {
            fclose(users);
            return user_index;
        }
        user_index += 1;
    }

    fclose(users);
    return -1; // user not found
}

int email_found(char email[29]) // Error 2
{
    FILE *users = fopen("users.csv", "r");

    if (users == NULL)
    {
        perror("Error opening file");
        return -1;
    }

    int user_index = 0;
    char file_line[1024];
    char file_username[256], file_email[256];

    while (fgets(file_line, sizeof(file_line), users))
    {
        sscanf(file_line, "%255[^,],%255[^,]", file_username, file_email);
        if (strcmp(file_email, email) == 0)
        {
            fclose(users);
            return user_index;
        }
        user_index += 1;
    }

    fclose(users);
    return -1; // email not found
}

bool password_correct(int user_index, char password[26]) // Error 3
{
    FILE *users = fopen("users.csv", "r");

    if (users == NULL)
    {
        perror("Error opening file");
        return false;
    }

    char trash[1024];
    char temp_username[256], temp_email[256];
    char file_line[1024];
    char file_password[256];

    for (int i = 0; i <= user_index; i++)
    {
        if (fgets(file_line, sizeof(file_line), users) == NULL)
        {
            fclose(users);
            return false;
        }
    }

    sscanf(file_line, "%255[^,],%255[^,],%255[^\n]", temp_username, temp_email, file_password);

    if (strcmp(password, file_password) == 0)
    {
        fclose(users);
        return true;
    }

    fclose(users);
    return false;
}

void print_user_not_found() // Error 1
{
    mvprintw((LINES / 2) + 3, (COLS / 2) - 24, "__ Error _______________________________________");
    mvprintw((LINES / 2) + 4, (COLS / 2) - 24, "|                                              |");
    mvprintw((LINES / 2) + 5, (COLS / 2) - 24, "|        Usernmae not found. Try again!        |");
    mvprintw((LINES / 2) + 6, (COLS / 2) - 24, "|                                              |");
    mvprintw((LINES / 2) + 7, (COLS / 2) - 24, "‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾");
}

void print_email_not_found() // Error 2
{
    mvprintw((LINES / 2) + 3, (COLS / 2) - 24, "__ Error _______________________________________");
    mvprintw((LINES / 2) + 4, (COLS / 2) - 24, "|                                              |");
    mvprintw((LINES / 2) + 5, (COLS / 2) - 24, "|         Email not found. Try again!          |");
    mvprintw((LINES / 2) + 6, (COLS / 2) - 24, "|                                              |");
    mvprintw((LINES / 2) + 7, (COLS / 2) - 24, "‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾");
}

void print_wrong_password() // Error 3
{
    mvprintw((LINES / 2) + 3, (COLS / 2) - 24, "__ Error _______________________________________");
    mvprintw((LINES / 2) + 4, (COLS / 2) - 24, "|                                              |");
    mvprintw((LINES / 2) + 5, (COLS / 2) - 24, "|          Wrong Password. Try again!          |");
    mvprintw((LINES / 2) + 6, (COLS / 2) - 24, "|                                              |");
    mvprintw((LINES / 2) + 7, (COLS / 2) - 24, "‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾");
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