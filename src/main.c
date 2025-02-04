/*******************************************
 *                                         *
 *            " VOID RUNNER "              *
 *                                         *
 *        a game by: Mohsen Salah          *
 *         Student ID: 403106238           *
 *                                         *
 *              2024 - 2025                *
 *                                         *
 *                                         *
 *******************************************/

#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <locale.h>
#include <ctype.h>
#include <stdbool.h>
#include <regex.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#include "ASCII_ART.h"
#include "FUNCTIONS.h"

Level level[MAX_LEVEL];
Game game;
User current_user;
Character hero;
bool ***visibility_grid;
short int ***map;
int get_input = 1;
int added_coin = 0;
int object_index = -1;
int enemy_index = -1;

int main()
{
    setlocale(LC_ALL, "");
    srand(time(NULL));

    initscr();
    curs_set(FALSE);
    keypad(stdscr, TRUE);

    if (SDL_Init(SDL_INIT_AUDIO) < 0)
    {
        fprintf(stderr, "SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        return 1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        fprintf(stderr, "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
        return 1;
    }

    load_users_from_csv("users.csv", &game.users, &game.users_num);

    initialize_visibility_grid();
    initialize_map();

    hero.health = 90, hero.coins = 0, hero.speed = 80, hero.satiety = 90, hero.health_progress = 0, hero.satiety_progress = 0, hero.last_room = -1;

    hero.food_inventory = (Object *)malloc(MAX_FOOD * sizeof(Object));
    hero.food_num = 0;

    hero.weapon_inventory = (Object *)malloc(MAX_WEAPON * sizeof(Object));
    hero.weapon_inventory[0].type = Mace;
    hero.weapon_num = 1;

    hero.spell_inventory = (Object *)malloc(MAX_SPELL * sizeof(Object));
    hero.spell_num = 0;

    if (can_change_color())
    {
        start_color();
        init_pair(1, COLOR_WHITE, COLOR_BLACK);
        init_pair(2, COLOR_BLUE, COLOR_BLACK);
        init_pair(3, COLOR_CYAN, COLOR_BLACK);
        init_pair(4, COLOR_GREEN, COLOR_BLACK);
        init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(6, COLOR_RED, COLOR_BLACK);
        init_pair(7, COLOR_YELLOW, COLOR_BLACK);

        init_color(COLOR_YELLOW + 1, 255, 170, 29); // Gold
        init_pair(8, COLOR_YELLOW + 1, COLOR_BLACK);

        init_color(COLOR_WHITE + 1, 753, 753, 753); // Silver/Metal color
        init_pair(9, COLOR_WHITE + 1, COLOR_BLACK);

        init_color(COLOR_RED + 10, 1000, 714, 757); // Light Red
        init_color(COLOR_RED + 20, 1000, 0, 0);     // Red
        init_color(COLOR_RED + 30, 545, 0, 0);      // Dark Red
        init_color(COLOR_RED + 40, 300, 0, 0);      // Darker Red

        init_pair(10, COLOR_RED + 10, COLOR_BLACK); // Light Red
        init_pair(11, COLOR_RED + 20, COLOR_BLACK); // Red
        init_pair(12, COLOR_RED + 30, COLOR_BLACK); // Dark Red
        init_pair(13, COLOR_RED + 40, COLOR_BLACK); // Darker Red
    }

    noecho();

    title_screen();

    while (1)
    {
        int choice = welcome_screen();

        if (choice == 0) // continue game
        {
            continue_game_screen();

            while (1)
            {
                int user_option_choice = user_options_menu();

                if (user_option_choice == 0) // New Game
                {
                    random_map();
                    int level_num = 0;
                    while (1)
                    {
                        if (level_num >= 0 && level_num < MAX_LEVEL)
                            level_num = run_game_level(level_num);

                        if (level_num == -2)
                        {
                            show_death_screen();
                            break;
                        }

                        if (level_num == MAX_LEVEL)
                        {
                            show_win_screen();
                            break;
                        }
                    }
                    save_user_to_csv("users.csv", &current_user);
                }

                if (user_option_choice == 1) // continue_previous_game
                {
                    random_map();
                    int level_num = 0;
                    while (1)
                    {
                        if (level_num >= 0 && level_num < MAX_LEVEL)
                            level_num = run_game_level(level_num);

                        if (level_num == -2)
                        {
                            show_death_screen();
                            break;
                        }

                        if (level_num == MAX_LEVEL)
                        {
                            show_win_screen();
                            break;
                        }
                    }
                    save_user_to_csv("users.csv", &current_user);
                }

                if (user_option_choice == 2) // View Score Board
                {
                    score_board_menu();
                }

                if (user_option_choice == 3) // My Profile
                {
                }

                if (user_option_choice == 4)
                {
                    user_settings_menu();
                    save_user_to_csv("users.csv", &current_user);
                }
            }
        }

        else if (choice == 1) // create new account
        {
            new_account_screen();
        }

        else if (choice == 2) // Play as Guest
        {
            random_map();
            int level_num = 0;
            while (1)
            {
                if (level_num >= 0 && level_num < MAX_LEVEL)
                    level_num = run_game_level(level_num);

                if (level_num == -2)
                {
                    show_death_screen();
                    break;
                }

                if (level_num == MAX_LEVEL)
                {
                    show_win_screen();
                    break;
                }
            }
        }
    }

    free_visibility_grid();
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
}

int *three_extra_rooms()
{
    int *numbers = (int *)malloc(9 * sizeof(int));
    for (int i = 0; i < 9; i++)
        numbers[i] = i;

    for (int i = 8; i > 0; i--)
    {
        int j = rand() % (i + 1);
        int temp = numbers[i];
        numbers[i] = numbers[j];
        numbers[j] = temp;
    }

    int *result = (int *)malloc(3 * sizeof(int));

    for (int i = 0; i < 3; i++)
        result[i] = numbers[i];

    return result;
}

int *extra_corridors()
{
    int *result = (int *)malloc(4 * sizeof(int));
    int r_1 = ((rand() % 2) + 1) * 3;
    r_1 += ((rand() % 2) * 2);

    result[0] = 12;

    if (r_1 == 3)
    {
        result[1] = ((rand() % 2) * 3) + 7;
        result[2] = ((rand() % 2) * 2) + 9;
        int choice[4] = {0, 1, 2, 4};
        result[3] = choice[rand() % 4];
    }

    else if (r_1 == 5)
    {
        result[1] = ((rand() % 2) * 3) + 1;
        result[2] = ((rand() % 2) * 2) + 9;
        int choice[4] = {0, 2, 7, 10};
        result[3] = choice[rand() % 4];
    }

    else if (r_1 == 6)
    {
        result[1] = (rand() % 2) * 2;
        result[2] = ((rand() % 2) * 3) + 7;
        int choice[4] = {1, 4, 9, 11};
        result[3] = choice[rand() % 4];
    }

    else if (r_1 == 8)
    {
        result[1] = (rand() % 2) * 2;
        result[2] = ((rand() % 2) * 3) + 1;
        int choice[4] = {7, 9, 10, 11};
        result[3] = choice[rand() % 4];
    }

    return result;
}

void random_map()
{
    for (int i = 0; i < MAX_LEVEL - 1; i++)
    {
        random_level(i);
        create_corridors(i);
        create_objects(i);
        create_enemies(i);
        level[i].level_num = i;
        determine_room_types(i);
        level_to_file("test.csv", i);
    }

    treasure_level();
    level[MAX_LEVEL - 1].enemy_num = 0;
    level[MAX_LEVEL - 1].object_num = 0;
    create_treasure_room_objects();
    level[MAX_LEVEL - 1].level_num = MAX_LEVEL - 1;
    level_to_file("test.csv", MAX_LEVEL - 1);
}

void random_room(int y_min, int y_max, int x_min, int x_max, Room *room)
{
    (*room).corner.y = (rand() % (y_max - y_min - MIN_ROOM_WIDTH - 3)) + (y_min + 2);
    (*room).corner.x = (rand() % (x_max - x_min - MIN_ROOM_LENGTH - 3)) + (x_min + 2);

    int max_width = y_max - (*room).corner.y - 1;
    int max_length = x_max - (*room).corner.x - 1;

    (*room).width = (rand() % (max_width - MIN_ROOM_WIDTH)) + MIN_ROOM_WIDTH;
    (*room).length = (rand() % (max_length - MIN_ROOM_LENGTH)) + MIN_ROOM_LENGTH;
}

Point middle_door(Room room)
{
    Point result;
    result.y = (rand() % (room).width) + (room).corner.y;
    result.x = (rand() % (room).length) + (room).corner.x;
    return result;
}

void break_corridors(Point door_1, Point door_2, int corridor_num, int level_num)
{
    if (corridor_num == 0 || corridor_num == 1 || corridor_num == 5 || corridor_num == 6 || corridor_num == 10 || corridor_num == 11)
    {
        level[level_num].between_doors_1[corridor_num].y = door_1.y;
        level[level_num].between_doors_2[corridor_num].y = door_2.y;
        int temp = (rand() % (door_2.x - door_1.x - 2)) + door_1.x + 1;
        level[level_num].between_doors_1[corridor_num].x = temp;
        level[level_num].between_doors_2[corridor_num].x = temp;
    }

    else
    {
        level[level_num].between_doors_1[corridor_num].x = door_1.x;
        level[level_num].between_doors_2[corridor_num].x = door_2.x;
        int temp = (rand() % (door_2.y - door_1.y - 2)) + door_1.y + 1;
        level[level_num].between_doors_1[corridor_num].y = temp;
        level[level_num].between_doors_2[corridor_num].y = temp;
    }
}

void create_corridors(int level_num)
{
    int *not_shown = extra_corridors();

    for (int i = 0; i < 12; i++)
    {
        if (i == 0)
            break_corridors(level[level_num].room[0].door[0], level[level_num].room[1].door[0], i, level_num);

        else if (i == 1)
            break_corridors(level[level_num].room[1].door[1], level[level_num].room[2].door[0], i, level_num);

        else if (i == 2)
            break_corridors(level[level_num].room[0].door[1], level[level_num].room[3].door[0], i, level_num);

        else if (i == 3)
            break_corridors(level[level_num].room[1].door[2], level[level_num].room[4].door[0], i, level_num);

        else if (i == 4)
            break_corridors(level[level_num].room[2].door[1], level[level_num].room[5].door[0], i, level_num);

        else if (i == 5)
            break_corridors(level[level_num].room[3].door[1], level[level_num].room[4].door[1], i, level_num);

        else if (i == 6)
            break_corridors(level[level_num].room[4].door[2], level[level_num].room[5].door[1], i, level_num);

        else if (i == 7)
            break_corridors(level[level_num].room[3].door[2], level[level_num].room[6].door[0], i, level_num);

        else if (i == 8)
            break_corridors(level[level_num].room[4].door[3], level[level_num].room[7].door[0], i, level_num);

        else if (i == 9)
            break_corridors(level[level_num].room[5].door[2], level[level_num].room[8].door[0], i, level_num);

        else if (i == 10)
            break_corridors(level[level_num].room[6].door[1], level[level_num].room[7].door[1], i, level_num);

        else if (i == 11)
            break_corridors(level[level_num].room[7].door[2], level[level_num].room[8].door[1], i, level_num);

        if (i == not_shown[0] || i == not_shown[1] || i == not_shown[2] || i == not_shown[3])
            level[level_num].corridor_exist[i] = 0;
        else
        {
            if (i == 0)
            {
                if (level[level_num].room[0].room_exist == 1 && level[level_num].room[1].room_exist == 1)
                    level[level_num].corridor_exist[i] = 1;
                else
                    level[level_num].corridor_exist[i] = 0;
            }

            else if (i == 1)
            {
                if (level[level_num].room[1].room_exist == 1 && level[level_num].room[2].room_exist == 1)
                    level[level_num].corridor_exist[i] = 1;
                else
                    level[level_num].corridor_exist[i] = 0;
            }

            else if (i == 2)
            {
                if (level[level_num].room[0].room_exist == 1 && level[level_num].room[3].room_exist == 1)
                    level[level_num].corridor_exist[i] = 1;
                else
                    level[level_num].corridor_exist[i] = 0;
            }

            else if (i == 3)
            {
                if (level[level_num].room[1].room_exist == 1 && level[level_num].room[4].room_exist == 1)
                    level[level_num].corridor_exist[i] = 1;
                else
                    level[level_num].corridor_exist[i] = 0;
            }

            else if (i == 4)
            {
                if (level[level_num].room[2].room_exist == 1 && level[level_num].room[5].room_exist == 1)
                    level[level_num].corridor_exist[i] = 1;
                else
                    level[level_num].corridor_exist[i] = 0;
            }

            else if (i == 5)
            {
                if (level[level_num].room[3].room_exist == 1 && level[level_num].room[4].room_exist == 1)
                    level[level_num].corridor_exist[i] = 1;
                else
                    level[level_num].corridor_exist[i] = 0;
            }

            else if (i == 6)
            {
                if (level[level_num].room[4].room_exist == 1 && level[level_num].room[5].room_exist == 1)
                    level[level_num].corridor_exist[i] = 1;
                else
                    level[level_num].corridor_exist[i] = 0;
            }

            else if (i == 7)
            {
                if (level[level_num].room[3].room_exist == 1 && level[level_num].room[6].room_exist == 1)
                    level[level_num].corridor_exist[i] = 1;
                else
                    level[level_num].corridor_exist[i] = 0;
            }

            else if (i == 8)
            {
                if (level[level_num].room[4].room_exist == 1 && level[level_num].room[7].room_exist == 1)
                    level[level_num].corridor_exist[i] = 1;
                else
                    level[level_num].corridor_exist[i] = 0;
            }

            else if (i == 9)
            {
                if (level[level_num].room[5].room_exist == 1 && level[level_num].room[8].room_exist == 1)
                    level[level_num].corridor_exist[i] = 1;
                else
                    level[level_num].corridor_exist[i] = 0;
            }

            else if (i == 10)
            {
                if (level[level_num].room[6].room_exist == 1 && level[level_num].room[7].room_exist == 1)
                    level[level_num].corridor_exist[i] = 1;
                else
                    level[level_num].corridor_exist[i] = 0;
            }

            else if (i == 11)
            {
                if (level[level_num].room[7].room_exist == 1 && level[level_num].room[8].room_exist == 1)
                    level[level_num].corridor_exist[i] = 1;
                else
                    level[level_num].corridor_exist[i] = 0;
            }
        }
    }

    if (level[level_num].room[0].room_exist == 1 && level[level_num].corridor_exist[0] == 0 && level[level_num].corridor_exist[2] == 0)
    {
        if (level[level_num].room[1].room_exist == 1)
            level[level_num].corridor_exist[0] = 1;
        else if (level[level_num].room[3].room_exist == 1)
            level[level_num].corridor_exist[2] = 1;
        else if (level[level_num].room[2].room_exist == 1)
        {
            level[level_num].corridor_exist[0] = 1;
            level[level_num].corridor_exist[1] = 1;
            level[level_num].corridor_exist[3] = 1;
        }
        else if (level[level_num].room[6].room_exist == 1)
        {
            level[level_num].corridor_exist[2] = 1;
            level[level_num].corridor_exist[7] = 1;
            level[level_num].corridor_exist[5] = 1;
        }
    }

    if (level[level_num].room[2].room_exist == 1 && level[level_num].corridor_exist[1] == 0 && level[level_num].corridor_exist[4] == 0)
    {
        if (level[level_num].room[1].room_exist == 1)
            level[level_num].corridor_exist[1] = 1;
        else if (level[level_num].room[5].room_exist == 1)
            level[level_num].corridor_exist[4] = 1;
        else if (level[level_num].room[0].room_exist == 1)
        {
            level[level_num].corridor_exist[0] = 1;
            level[level_num].corridor_exist[1] = 1;
            level[level_num].corridor_exist[3] = 1;
        }
        else if (level[level_num].room[8].room_exist == 1)
        {
            level[level_num].corridor_exist[4] = 1;
            level[level_num].corridor_exist[9] = 1;
            level[level_num].corridor_exist[6] = 1;
        }
    }

    if (level[level_num].room[6].room_exist == 1 && level[level_num].corridor_exist[7] == 0 && level[level_num].corridor_exist[10] == 0)
    {
        if (level[level_num].room[3].room_exist == 1)
            level[level_num].corridor_exist[7] = 1;
        else if (level[level_num].room[7].room_exist == 1)
            level[level_num].corridor_exist[10] = 1;
        else if (level[level_num].room[0].room_exist == 1)
        {
            level[level_num].corridor_exist[2] = 1;
            level[level_num].corridor_exist[7] = 1;
            level[level_num].corridor_exist[5] = 1;
        }
        else if (level[level_num].room[8].room_exist == 1)
        {
            level[level_num].corridor_exist[10] = 1;
            level[level_num].corridor_exist[11] = 1;
            level[level_num].corridor_exist[8] = 1;
        }
    }

    if (level[level_num].room[8].room_exist == 1 && level[level_num].corridor_exist[9] == 0 && level[level_num].corridor_exist[11] == 0)
    {
        if (level[level_num].room[5].room_exist == 1)
            level[level_num].corridor_exist[9] = 1;
        else if (level[level_num].room[7].room_exist == 1)
            level[level_num].corridor_exist[11] = 1;
        else if (level[level_num].room[2].room_exist == 1)
        {
            level[level_num].corridor_exist[4] = 1;
            level[level_num].corridor_exist[9] = 1;
            level[level_num].corridor_exist[6] = 1;
        }
        else if (level[level_num].room[6].room_exist == 1)
        {
            level[level_num].corridor_exist[10] = 1;
            level[level_num].corridor_exist[11] = 1;
            level[level_num].corridor_exist[8] = 1;
        }
    }
}

void side_door(Room *room, int door_position, int door_num)
{
    if (door_position == 0)
    {
        (*room).door[door_num].y = (*room).corner.y;
        (*room).door[door_num].x = (rand() % ((*room).length - 4)) + (*room).corner.x + 2;
    }

    else if (door_position == 1)
    {
        (*room).door[door_num].y = (rand() % ((*room).width - 4)) + (*room).corner.y + 2;
        (*room).door[door_num].x = (*room).corner.x - 1;
    }

    else if (door_position == 2)
    {
        (*room).door[door_num].y = (rand() % ((*room).width - 4)) + (*room).corner.y + 2;
        (*room).door[door_num].x = (*room).length + (*room).corner.x;
    }

    else if (door_position == 3)
    {
        (*room).door[door_num].y = (*room).width + (*room).corner.y + 1;
        (*room).door[door_num].x = (rand() % ((*room).length - 4)) + (*room).corner.x + 2;
    }
}

void generate_doors(Room *room)
{
    int room_position = (room)->room_position;

    if (room_position == 0)
    {
        (*room).door_num = 2;
        (*room).door = (Point *)malloc((*room).door_num * sizeof(Point));
        (*room).door_positions = (int *)malloc((*room).door_num * sizeof(int));
        side_door(room, 2, 0);
        side_door(room, 3, 1);
        (*room).door_positions[0] = 2;
        (*room).door_positions[1] = 3;
    }

    else if (room_position == 1)
    {
        (*room).door_num = 3;
        (*room).door = (Point *)malloc((*room).door_num * sizeof(Point));
        (*room).door_positions = (int *)malloc((*room).door_num * sizeof(int));
        side_door(room, 1, 0);
        side_door(room, 2, 1);
        side_door(room, 3, 2);
        (*room).door_positions[0] = 1;
        (*room).door_positions[1] = 2;
        (*room).door_positions[2] = 3;
    }

    else if (room_position == 2)
    {
        (*room).door_num = 2;
        (*room).door = (Point *)malloc((*room).door_num * sizeof(Point));
        (*room).door_positions = (int *)malloc((*room).door_num * sizeof(int));
        side_door(room, 1, 0);
        side_door(room, 3, 1);
        (*room).door_positions[0] = 1;
        (*room).door_positions[1] = 3;
    }

    else if (room_position == 3)
    {
        (*room).door_num = 3;
        (*room).door = (Point *)malloc((*room).door_num * sizeof(Point));
        (*room).door_positions = (int *)malloc((*room).door_num * sizeof(int));
        side_door(room, 0, 0);
        side_door(room, 2, 1);
        side_door(room, 3, 2);
        (*room).door_positions[0] = 0;
        (*room).door_positions[1] = 2;
        (*room).door_positions[2] = 3;
    }

    else if (room_position == 4)
    {
        (*room).door_num = 4;
        (*room).door = (Point *)malloc((*room).door_num * sizeof(Point));
        (*room).door_positions = (int *)malloc((*room).door_num * sizeof(int));
        side_door(room, 0, 0);
        side_door(room, 1, 1);
        side_door(room, 2, 2);
        side_door(room, 3, 3);
        (*room).door_positions[0] = 0;
        (*room).door_positions[1] = 1;
        (*room).door_positions[2] = 2;
        (*room).door_positions[3] = 3;
    }

    else if (room_position == 5)
    {
        (*room).door_num = 3;
        (*room).door = (Point *)malloc((*room).door_num * sizeof(Point));
        (*room).door_positions = (int *)malloc((*room).door_num * sizeof(int));
        side_door(room, 0, 0);
        side_door(room, 1, 1);
        side_door(room, 3, 2);
        (*room).door_positions[0] = 0;
        (*room).door_positions[1] = 1;
        (*room).door_positions[2] = 3;
    }

    else if (room_position == 6)
    {
        (*room).door_num = 2;
        (*room).door = (Point *)malloc((*room).door_num * sizeof(Point));
        (*room).door_positions = (int *)malloc((*room).door_num * sizeof(int));
        side_door(room, 0, 0);
        side_door(room, 2, 1);
        (*room).door_positions[0] = 0;
        (*room).door_positions[1] = 2;
    }

    else if (room_position == 7)
    {
        (*room).door_num = 3;
        (*room).door = (Point *)malloc((*room).door_num * sizeof(Point));
        (*room).door_positions = (int *)malloc((*room).door_num * sizeof(int));
        side_door(room, 0, 0);
        side_door(room, 1, 1);
        side_door(room, 2, 2);
        (*room).door_positions[0] = 0;
        (*room).door_positions[1] = 1;
        (*room).door_positions[2] = 2;
    }

    else if (room_position == 8)
    {
        (*room).door_num = 2;
        (*room).door = (Point *)malloc((*room).door_num * sizeof(Point));
        (*room).door_positions = (int *)malloc((*room).door_num * sizeof(int));
        side_door(room, 0, 0);
        side_door(room, 1, 1);
        (*room).door_positions[0] = 0;
        (*room).door_positions[1] = 1;
    }
}

void generate_middle_doors(Room *room)
{
    int room_position = (room)->room_position;

    if (room_position == 0)
    {
        (*room).door_num = 2;
        (*room).door = (Point *)malloc((*room).door_num * sizeof(Point));
        (*room).door_positions = (int *)malloc((*room).door_num * sizeof(int));
        (*room).door[0] = (*room).middle_door;
        (*room).door[1] = (*room).middle_door;
        (*room).door_positions[0] = 2;
        (*room).door_positions[1] = 3;
    }

    else if (room_position == 1)
    {
        (*room).door_num = 3;
        (*room).door = (Point *)malloc((*room).door_num * sizeof(Point));
        (*room).door_positions = (int *)malloc((*room).door_num * sizeof(int));
        (*room).door[0] = (*room).middle_door;
        (*room).door[1] = (*room).middle_door;
        (*room).door[2] = (*room).middle_door;
        (*room).door_positions[0] = 1;
        (*room).door_positions[1] = 2;
        (*room).door_positions[2] = 3;
    }

    else if (room_position == 2)
    {
        (*room).door_num = 2;
        (*room).door = (Point *)malloc((*room).door_num * sizeof(Point));
        (*room).door_positions = (int *)malloc((*room).door_num * sizeof(int));
        (*room).door[0] = (*room).middle_door;
        (*room).door[1] = (*room).middle_door;
        (*room).door_positions[0] = 1;
        (*room).door_positions[1] = 3;
    }

    else if (room_position == 3)
    {
        (*room).door_num = 3;
        (*room).door = (Point *)malloc((*room).door_num * sizeof(Point));
        (*room).door_positions = (int *)malloc((*room).door_num * sizeof(int));
        (*room).door[0] = (*room).middle_door;
        (*room).door[1] = (*room).middle_door;
        (*room).door[2] = (*room).middle_door;
        (*room).door_positions[0] = 0;
        (*room).door_positions[1] = 2;
        (*room).door_positions[2] = 3;
    }

    else if (room_position == 4)
    {
        (*room).door_num = 4;
        (*room).door = (Point *)malloc((*room).door_num * sizeof(Point));
        (*room).door_positions = (int *)malloc((*room).door_num * sizeof(int));
        (*room).door[0] = (*room).middle_door;
        (*room).door[1] = (*room).middle_door;
        (*room).door[2] = (*room).middle_door;
        (*room).door[3] = (*room).middle_door;
        (*room).door_positions[0] = 0;
        (*room).door_positions[1] = 1;
        (*room).door_positions[2] = 2;
        (*room).door_positions[3] = 3;
    }

    else if (room_position == 5)
    {
        (*room).door_num = 3;
        (*room).door = (Point *)malloc((*room).door_num * sizeof(Point));
        (*room).door_positions = (int *)malloc((*room).door_num * sizeof(int));
        (*room).door[0] = (*room).middle_door;
        (*room).door[1] = (*room).middle_door;
        (*room).door[2] = (*room).middle_door;
        (*room).door_positions[0] = 0;
        (*room).door_positions[1] = 1;
        (*room).door_positions[2] = 3;
    }

    else if (room_position == 6)
    {
        (*room).door_num = 2;
        (*room).door = (Point *)malloc((*room).door_num * sizeof(Point));
        (*room).door_positions = (int *)malloc((*room).door_num * sizeof(int));
        (*room).door[0] = (*room).middle_door;
        (*room).door[1] = (*room).middle_door;
        (*room).door_positions[0] = 0;
        (*room).door_positions[1] = 2;
    }

    else if (room_position == 7)
    {
        (*room).door_num = 3;
        (*room).door = (Point *)malloc((*room).door_num * sizeof(Point));
        (*room).door_positions = (int *)malloc((*room).door_num * sizeof(int));
        (*room).door[0] = (*room).middle_door;
        (*room).door[1] = (*room).middle_door;
        (*room).door[2] = (*room).middle_door;
        (*room).door_positions[0] = 0;
        (*room).door_positions[1] = 1;
        (*room).door_positions[2] = 2;
    }

    else if (room_position == 8)
    {
        (*room).door_num = 2;
        (*room).door = (Point *)malloc((*room).door_num * sizeof(Point));
        (*room).door_positions = (int *)malloc((*room).door_num * sizeof(int));
        (*room).door[0] = (*room).middle_door;
        (*room).door[1] = (*room).middle_door;
        (*room).door_positions[0] = 0;
        (*room).door_positions[1] = 1;
    }
}

void determine_room_types(int level_num)
{
    int nightmare = -1;
    while (1)
    {
        nightmare = rand() % 9;
        if (level[level_num].room[nightmare].room_exist == 1)
            break;
    }

    int enhant = -1;
    while (1)
    {
        enhant = rand() % 9;
        if (level[level_num].room[enhant].room_exist == 1 && enhant != nightmare)
            break;
    }

    for (int i = 0; i < 9; i++)
    {
        if (i == nightmare)
            level[level_num].room[i].type = Nightmare;
        else
            level[level_num].room[i].type = Normal;
    }
}

void random_level(int level_num)
{
    int perfect_y = (LINES - (UP_MARGIN + DOWN_MARGIN)) / 3;
    int y_values[12];

    y_values[0] = UP_MARGIN;
    y_values[1] = UP_MARGIN + (perfect_y);
    y_values[2] = LINES - DOWN_MARGIN - (perfect_y);
    y_values[3] = LINES - DOWN_MARGIN;

    int perfect_x = (COLS - (2 * MARGIN)) / 3;
    int x_values[12];
    x_values[0] = MARGIN;
    x_values[1] = MARGIN + (perfect_x);
    x_values[2] = COLS - MARGIN - (perfect_x);
    x_values[3] = COLS - MARGIN;

    int *extra_rooms = three_extra_rooms();
    level[level_num].room_num = 0;

    for (int i = 0; i < 9; i++)
    {
        random_room(y_values[i / 3], y_values[(i / 3) + 1], x_values[i % 3], x_values[(i % 3) + 1], &level[level_num].room[i]);
        level[level_num].room[i].room_position = i;

        if ((i == extra_rooms[0] || i == extra_rooms[1] || i == extra_rooms[2]) && i != 4)
        {
            int show_room = rand() % 4;

            if (show_room == 0)
            {
                level[level_num].room[i].room_exist = 1;
                level[level_num].room_num += 1;
                generate_doors(&level[level_num].room[i]);
            }

            else
            {
                level[level_num].room[i].room_exist = 0;
                level[level_num].room[i].middle_door = middle_door(level[level_num].room[i]);
                generate_middle_doors(&level[level_num].room[i]);
            }
        }

        else
        {
            level[level_num].room[i].room_exist = 1;
            level[level_num].room_num += 1;
            generate_doors(&level[level_num].room[i]);
        }
    }

    level[level_num].object_num = 0;

    for (int i = 0; i < 9; i++)
        level[level_num].room[i].discovered = 0;
}

void treasure_level()
{
    int level_num = MAX_LEVEL - 1;

    int y_min = (rand() % ((LINES / 2) - UP_MARGIN - 20)) + UP_MARGIN;
    int y_max = (rand() % ((LINES / 2) - DOWN_MARGIN - 20)) + (LINES / 2) + 20;

    int x_min = (rand() % ((COLS / 2) - MARGIN - 20)) + MARGIN;
    int x_max = (rand() % ((COLS / 2) - MARGIN - 20)) + (COLS / 2) + 20;

    level[level_num].room[0].corner.y = y_min;
    level[level_num].room[0].corner.x = x_min;
    level[level_num].room[0].width = y_max - y_min;
    level[level_num].room[0].length = x_max - x_min;

    level[level_num].room[0].discovered = 1;
    level[level_num].room_num = 1;
    level[level_num].discovered_room_num = 1;
    level[level_num].room[0].room_exist = 1;
    level[level_num].level_num = MAX_LEVEL - 1;

    for (int i = 0; i < 12; i++)
        level[level_num].corridor_exist[i] = 0;
}

void create_treasure_room_objects()
{
    int level_num = MAX_LEVEL - 1;
    int area = level[MAX_LEVEL - 1].room[0].width * level[MAX_LEVEL - 1].room[0].length;

    int deamon_num = (rand() % 3);
    int fire_dragon_num = (rand() % 3);
    int giant_num = (rand() % 4);
    int snake_num = (rand() % 3);
    int undead_num = (rand() % 5);

    int enemy_num = deamon_num + fire_dragon_num + giant_num + snake_num + undead_num;
    level[level_num].enemies = (Enemy *)malloc(enemy_num * sizeof(Enemy));

    int enemy_count = 0;

    for (int i = 0; i < deamon_num; i++)
    {
        level[level_num].enemies[enemy_count].location = random_location_enemy(level_num);
        level[level_num].enemies[enemy_count].type = deamon;
        level[level_num].enemies[enemy_count].visible = 1;
        level[level_num].enemies[enemy_count].location_room = find_room(level_num, level[level_num].enemies[enemy_count].location);
        map[level_num][level[level_num].enemies[enemy_count].location.y][level[level_num].enemies[enemy_count].location.x] = level[level_num].enemies[enemy_count].type + 22;
        enemy_count += 1;
    }

    for (int i = 0; i < fire_dragon_num; i++)
    {
        level[level_num].enemies[enemy_count].location = random_location_enemy(level_num);
        level[level_num].enemies[enemy_count].type = fire_monster;
        level[level_num].enemies[enemy_count].visible = 1;
        level[level_num].enemies[enemy_count].location_room = find_room(level_num, level[level_num].enemies[enemy_count].location);
        map[level_num][level[level_num].enemies[enemy_count].location.y][level[level_num].enemies[enemy_count].location.x] = level[level_num].enemies[enemy_count].type + 22;
        enemy_count += 1;
    }

    for (int i = 0; i < snake_num; i++)
    {
        level[level_num].enemies[enemy_count].location = random_location_enemy(level_num);
        level[level_num].enemies[enemy_count].type = snake;
        level[level_num].enemies[enemy_count].visible = 1;
        level[level_num].enemies[enemy_count].location_room = find_room(level_num, level[level_num].enemies[enemy_count].location);
        map[level_num][level[level_num].enemies[enemy_count].location.y][level[level_num].enemies[enemy_count].location.x] = level[level_num].enemies[enemy_count].type + 22;
        enemy_count += 1;
    }

    for (int i = 0; i < giant_num; i++)
    {
        level[level_num].enemies[enemy_count].location = random_location_enemy(level_num);
        level[level_num].enemies[enemy_count].type = giant;
        level[level_num].enemies[enemy_count].visible = 1;
        level[level_num].enemies[enemy_count].location_room = find_room(level_num, level[level_num].enemies[enemy_count].location);
        map[level_num][level[level_num].enemies[enemy_count].location.y][level[level_num].enemies[enemy_count].location.x] = level[level_num].enemies[enemy_count].type + 22;
        enemy_count += 1;
    }

    for (int i = 0; i < undead_num; i++)
    {
        level[level_num].enemies[enemy_count].location = random_location_enemy(level_num);
        level[level_num].enemies[enemy_count].type = undead;
        level[level_num].enemies[enemy_count].visible = 1;
        level[level_num].enemies[enemy_count].location_room = find_room(level_num, level[level_num].enemies[enemy_count].location);
        map[level_num][level[level_num].enemies[enemy_count].location.y][level[level_num].enemies[enemy_count].location.x] = level[level_num].enemies[enemy_count].type + 22;
        enemy_count += 1;
    }

    level[level_num].enemy_num = enemy_count;
    for (int i = 0; i < enemy_count; i++)
    {
        switch (level[level_num].enemies->type)
        {
        case (deamon):
            level[level_num].enemies[i].health = 5;
            level[level_num].enemies[i].damage = 3;
            level[level_num].enemies[i].speed = 1;
            level[level_num].enemies[i].follow = 0;
            break;
        case (fire_monster):
            level[level_num].enemies[i].health = 10;
            level[level_num].enemies[i].damage = 7;
            level[level_num].enemies[i].speed = 1;
            level[level_num].enemies[i].follow = 0;
            break;
        case (giant):
            level[level_num].enemies[i].health = 15;
            level[level_num].enemies[i].damage = 10;
            level[level_num].enemies[i].speed = 1;
            level[level_num].enemies[i].follow = 1;
            break;
        case (snake):
            level[level_num].enemies[i].health = 20;
            level[level_num].enemies[i].damage = 15;
            level[level_num].enemies[i].speed = 1;
            level[level_num].enemies[i].follow = 1;
            break;
        case (undead):
            level[level_num].enemies[i].health = 30;
            level[level_num].enemies[i].damage = 20;
            level[level_num].enemies[i].speed = 1;
            level[level_num].enemies[i].follow = 1;
            break;
        }
    }

    int normal_food_num = (rand() % 5) + 2;
    int gourmet_food_num = (rand() % 2) + 0;
    int magic_food_num = (rand() % 3) + 1;
    int coin_num = (rand() % 5) + 5;
    int black_coin_num = (rand() % 3) + 1;

    int health_spell_num = (rand() % 2) + 0;
    int speed_spell_num = (rand() % 2) + 0;
    int damage_spell_num = (rand() % 2) + 0;

    int object_num = normal_food_num + gourmet_food_num + magic_food_num + coin_num + black_coin_num + health_spell_num + speed_spell_num + damage_spell_num;
    level[level_num].objects = (Object *)malloc(object_num * sizeof(Object));

    for (int i = 0; i < normal_food_num; i++)
        spawn_object(level_num, NormalFood);

    for (int i = 0; i < gourmet_food_num; i++)
        spawn_object(level_num, GourmetFood);

    for (int i = 0; i < magic_food_num; i++)
        spawn_object(level_num, MagicFood);

    for (int i = 0; i < coin_num; i++)
        spawn_object(level_num, Coin);

    for (int i = 0; i < black_coin_num; i++)
        spawn_object(level_num, BlackCoin);

    for (int i = 0; i < health_spell_num; i++)
        spawn_object(level_num, HealthSpell);

    for (int i = 0; i < speed_spell_num; i++)
        spawn_object(level_num, SpeedSpell);

    for (int i = 0; i < damage_spell_num; i++)
        spawn_object(level_num, DamageSpell);
}

void print_level(int level_num)
{
    for (int i = 0; i < 9; i++)
    {
        print_room(level_num, i);
    }
}

void print_room(int level_num, int room_num)
{
    int x = level[level_num].room[room_num].corner.x;
    int y = level[level_num].room[room_num].corner.y;
    int length = level[level_num].room[room_num].length;
    int width = level[level_num].room[room_num].width;

    if (level[level_num].room[room_num].room_exist == 1)
    {
        for (int i = 0; i < length; i++)
        {
            attron(COLOR_PAIR(6));
            if (visibility_grid[level_num][y][x + i] == 1)
                mvprintw(y, x + i, "═");
            attroff(COLOR_PAIR(6));
            for (int j = 1; j < width + 1; j++)
            {
                // if (room_num == hero.location_room)
                // {
                attron(COLOR_PAIR(4));
                if (visibility_grid[level_num][y + j][x + i] == 1)
                    mvprintw(y + j, x + i, "·"); // middle dot - ASCII Number: 183
                attroff(COLOR_PAIR(4));
                // }
            }
            attron(COLOR_PAIR(6));
            if (visibility_grid[level_num][y + width + 1][x + i] == 1)
                mvprintw(y + width + 1, x + i, "═");
            attroff(COLOR_PAIR(6));
        }

        for (int i = 0; i < width; i++)
        {
            attron(COLOR_PAIR(6));
            if (visibility_grid[level_num][y + i + 1][x - 1] == 1)
                mvprintw(y + i + 1, x - 1, "║");
            if (visibility_grid[level_num][y + i + 1][x + length] == 1)
                mvprintw(y + i + 1, x + length, "║");
            attroff(COLOR_PAIR(6));
        }

        attron(COLOR_PAIR(6));
        if (visibility_grid[level_num][y][x - 1] == 1)
            mvprintw(y, x - 1, "╔");
        if (visibility_grid[level_num][y][x + length] == 1)
            mvprintw(y, x + length, "╗");
        if (visibility_grid[level_num][y + width + 1][x - 1] == 1)
            mvprintw(y + width + 1, x - 1, "╚");
        if (visibility_grid[level_num][y + width + 1][x + length] == 1)
            mvprintw(y + width + 1, x + length, "╝");
        attroff(COLOR_PAIR(6));
    }
}

void print_corridors(int level_num)
{
    for (int i = 0; i < 12; i++)
    {
        if (level[level_num].corridor_exist[i] == 1)
        {
            Point mid_1 = level[level_num].between_doors_1[i];
            Point mid_2 = level[level_num].between_doors_2[i];

            Point door_1, door_2;

            switch (i)
            {
            case (0):
                door_1 = level[level_num].room[0].door[0];
                door_2 = level[level_num].room[1].door[0];
                attron(COLOR_PAIR(6));
                if (level[level_num].room[0].room_exist == 1)
                {
                    if (visibility_grid[level_num][door_1.y][door_1.x] == 1)
                        mvprintw(door_1.y, door_1.x, "╠");
                }
                else
                {
                    if (visibility_grid[level_num][door_1.y][door_1.x] == 1)
                        mvprintw(door_1.y, door_1.x, "░");
                }

                if (level[level_num].room[1].room_exist == 1)
                {
                    if (visibility_grid[level_num][door_2.y][door_2.x] == 1)
                        mvprintw(door_2.y, door_2.x, "╣");
                }
                else
                {
                    if (visibility_grid[level_num][door_2.y][door_2.x] == 1)
                        mvprintw(door_2.y, door_2.x, "░");
                }
                attroff(COLOR_PAIR(6));
                break;
            case (1):
                door_1 = level[level_num].room[1].door[1];
                door_2 = level[level_num].room[2].door[0];
                attron(COLOR_PAIR(6));
                if (level[level_num].room[1].room_exist == 1)
                {
                    if (visibility_grid[level_num][door_1.y][door_1.x] == 1)
                        mvprintw(door_1.y, door_1.x, "╠");
                }
                else
                {
                    if (visibility_grid[level_num][door_1.y][door_1.x] == 1)
                        mvprintw(door_1.y, door_1.x, "░");
                }

                if (level[level_num].room[2].room_exist == 1)
                {
                    if (visibility_grid[level_num][door_2.y][door_2.x] == 1)
                        mvprintw(door_2.y, door_2.x, "╣");
                }
                else
                {
                    if (visibility_grid[level_num][door_2.y][door_2.x] == 1)
                        mvprintw(door_2.y, door_2.x, "░");
                }
                attroff(COLOR_PAIR(6));
                break;
            case (2):
                door_1 = level[level_num].room[0].door[1];
                door_2 = level[level_num].room[3].door[0];
                attron(COLOR_PAIR(6));
                if (level[level_num].room[0].room_exist == 1)
                {
                    if (visibility_grid[level_num][door_1.y][door_1.x] == 1)
                        mvprintw(door_1.y, door_1.x, "╦");
                    if (visibility_grid[level_num][door_1.y][door_1.x + 1] == 1)
                        mvprintw(door_1.y, door_1.x + 1, "╦");
                }
                else
                {
                    if (visibility_grid[level_num][door_1.y][door_1.x] == 1)
                        mvprintw(door_1.y, door_1.x, "░");
                }

                if (level[level_num].room[3].room_exist == 1)
                {
                    if (visibility_grid[level_num][door_2.y][door_2.x] == 1)
                        mvprintw(door_2.y, door_2.x, "╩");
                    if (visibility_grid[level_num][door_2.y][door_2.x + 1] == 1)
                        mvprintw(door_2.y, door_2.x + 1, "╩");
                }
                else
                {
                    if (visibility_grid[level_num][door_2.y][door_2.x] == 1)
                        mvprintw(door_2.y, door_2.x, "░");
                }
                attroff(COLOR_PAIR(6));
                break;
            case (3):
                door_1 = level[level_num].room[1].door[2];
                door_2 = level[level_num].room[4].door[0];
                attron(COLOR_PAIR(6));
                if (level[level_num].room[1].room_exist == 1)
                {
                    if (visibility_grid[level_num][door_1.y][door_1.x] == 1)
                        mvprintw(door_1.y, door_1.x, "╦");
                    if (visibility_grid[level_num][door_1.y][door_1.x + 1] == 1)
                        mvprintw(door_1.y, door_1.x + 1, "╦");
                }
                else
                {
                    if (visibility_grid[level_num][door_1.y][door_1.x] == 1)
                        mvprintw(door_1.y, door_1.x, "░");
                }

                if (level[level_num].room[4].room_exist == 1)
                {
                    if (visibility_grid[level_num][door_2.y][door_2.x] == 1)
                        mvprintw(door_2.y, door_2.x, "╩");
                    if (visibility_grid[level_num][door_2.y][door_2.x + 1] == 1)
                        mvprintw(door_2.y, door_2.x + 1, "╩");
                }
                else
                {
                    if (visibility_grid[level_num][door_2.y][door_2.x] == 1)
                        mvprintw(door_2.y, door_2.x, "░");
                }
                attroff(COLOR_PAIR(6));
                break;
            case (4):
                door_1 = level[level_num].room[2].door[1];
                door_2 = level[level_num].room[5].door[0];
                attron(COLOR_PAIR(6));
                if (level[level_num].room[2].room_exist == 1)
                {
                    if (visibility_grid[level_num][door_1.y][door_1.x] == 1)
                        mvprintw(door_1.y, door_1.x, "╦");
                    if (visibility_grid[level_num][door_1.y][door_1.x + 1] == 1)
                        mvprintw(door_1.y, door_1.x + 1, "╦");
                }
                else
                {
                    if (visibility_grid[level_num][door_1.y][door_1.x] == 1)
                        mvprintw(door_1.y, door_1.x, "░");
                }

                if (level[level_num].room[5].room_exist == 1)
                {
                    if (visibility_grid[level_num][door_2.y][door_2.x] == 1)
                        mvprintw(door_2.y, door_2.x, "╩");
                    if (visibility_grid[level_num][door_2.y][door_2.x + 1] == 1)
                        mvprintw(door_2.y, door_2.x + 1, "╩");
                }
                else
                {
                    if (visibility_grid[level_num][door_2.y][door_2.x] == 1)
                        mvprintw(door_2.y, door_2.x, "░");
                }
                attroff(COLOR_PAIR(6));
                break;
            case (5):
                door_1 = level[level_num].room[3].door[1];
                door_2 = level[level_num].room[4].door[1];
                attron(COLOR_PAIR(6));
                if (level[level_num].room[3].room_exist == 1)
                {
                    if (visibility_grid[level_num][door_1.y][door_1.x] == 1)
                        mvprintw(door_1.y, door_1.x, "╠");
                }
                else
                {
                    if (visibility_grid[level_num][door_1.y][door_1.x] == 1)
                        mvprintw(door_1.y, door_1.x, "░");
                }

                if (level[level_num].room[4].room_exist == 1)
                {
                    if (visibility_grid[level_num][door_2.y][door_2.x] == 1)
                        mvprintw(door_2.y, door_2.x, "╣");
                }
                else
                {
                    if (visibility_grid[level_num][door_2.y][door_2.x] == 1)
                        mvprintw(door_2.y, door_2.x, "░");
                }
                attroff(COLOR_PAIR(6));
                break;
            case (6):
                door_1 = level[level_num].room[4].door[2];
                door_2 = level[level_num].room[5].door[1];
                attron(COLOR_PAIR(6));
                if (level[level_num].room[4].room_exist == 1)
                {
                    if (visibility_grid[level_num][door_1.y][door_1.x] == 1)
                        mvprintw(door_1.y, door_1.x, "╠");
                }
                else
                {
                    if (visibility_grid[level_num][door_1.y][door_1.x] == 1)
                        mvprintw(door_1.y, door_1.x, "░");
                }

                if (level[level_num].room[5].room_exist == 1)
                {
                    if (visibility_grid[level_num][door_2.y][door_2.x] == 1)
                        mvprintw(door_2.y, door_2.x, "╣");
                }
                else
                {
                    if (visibility_grid[level_num][door_2.y][door_2.x] == 1)
                        mvprintw(door_2.y, door_2.x, "░");
                }
                attroff(COLOR_PAIR(6));
                break;
            case (7):
                door_1 = level[level_num].room[3].door[2];
                door_2 = level[level_num].room[6].door[0];
                attron(COLOR_PAIR(6));
                if (level[level_num].room[3].room_exist == 1)
                {
                    if (visibility_grid[level_num][door_1.y][door_1.x] == 1)
                        mvprintw(door_1.y, door_1.x, "╦");
                    if (visibility_grid[level_num][door_1.y][door_1.x + 1] == 1)
                        mvprintw(door_1.y, door_1.x + 1, "╦");
                }
                else
                {
                    if (visibility_grid[level_num][door_1.y][door_1.x] == 1)
                        mvprintw(door_1.y, door_1.x, "░");
                }

                if (level[level_num].room[6].room_exist == 1)
                {
                    if (visibility_grid[level_num][door_2.y][door_2.x] == 1)
                        mvprintw(door_2.y, door_2.x, "╩");
                    if (visibility_grid[level_num][door_2.y][door_2.x + 1] == 1)
                        mvprintw(door_2.y, door_2.x + 1, "╩");
                }
                else
                {
                    if (visibility_grid[level_num][door_2.y][door_2.x] == 1)
                        mvprintw(door_2.y, door_2.x, "░");
                }
                attroff(COLOR_PAIR(6));
                break;
            case (8):
                door_1 = level[level_num].room[4].door[3];
                door_2 = level[level_num].room[7].door[0];
                attron(COLOR_PAIR(6));
                if (level[level_num].room[4].room_exist == 1)
                {
                    if (visibility_grid[level_num][door_1.y][door_1.x] == 1)
                        mvprintw(door_1.y, door_1.x, "╦");
                    if (visibility_grid[level_num][door_1.y][door_1.x + 1] == 1)
                        mvprintw(door_1.y, door_1.x + 1, "╦");
                }
                else
                {
                    if (visibility_grid[level_num][door_1.y][door_1.x] == 1)
                        mvprintw(door_1.y, door_1.x, "░");
                }

                if (level[level_num].room[7].room_exist == 1)
                {
                    if (visibility_grid[level_num][door_2.y][door_2.x] == 1)
                        mvprintw(door_2.y, door_2.x, "╩");
                    if (visibility_grid[level_num][door_2.y][door_2.x + 1] == 1)
                        mvprintw(door_2.y, door_2.x + 1, "╩");
                }
                else
                {
                    if (visibility_grid[level_num][door_2.y][door_2.x] == 1)
                        mvprintw(door_2.y, door_2.x, "░");
                }
                attroff(COLOR_PAIR(6));
                break;
            case (9):
                door_1 = level[level_num].room[5].door[2];
                door_2 = level[level_num].room[8].door[0];
                attron(COLOR_PAIR(6));
                if (level[level_num].room[5].room_exist == 1)
                {
                    if (visibility_grid[level_num][door_1.y][door_1.x] == 1)
                        mvprintw(door_1.y, door_1.x, "╦");
                    if (visibility_grid[level_num][door_1.y][door_1.x + 1] == 1)
                        mvprintw(door_1.y, door_1.x + 1, "╦");
                }
                else
                {
                    if (visibility_grid[level_num][door_1.y][door_1.x] == 1)
                        mvprintw(door_1.y, door_1.x, "░");
                }

                if (level[level_num].room[8].room_exist == 1)
                {
                    if (visibility_grid[level_num][door_2.y][door_2.x] == 1)
                        mvprintw(door_2.y, door_2.x, "╩");
                    if (visibility_grid[level_num][door_2.y][door_2.x + 1] == 1)
                        mvprintw(door_2.y, door_2.x + 1, "╩");
                }
                else
                {
                    if (visibility_grid[level_num][door_2.y][door_2.x] == 1)
                        mvprintw(door_2.y, door_2.x, "░");
                }
                attroff(COLOR_PAIR(6));
                break;
            case (10):
                door_1 = level[level_num].room[6].door[1];
                door_2 = level[level_num].room[7].door[1];
                attron(COLOR_PAIR(6));
                if (level[level_num].room[6].room_exist == 1)
                {
                    if (visibility_grid[level_num][door_1.y][door_1.x] == 1)
                        mvprintw(door_1.y, door_1.x, "╠");
                }
                else
                {
                    if (visibility_grid[level_num][door_1.y][door_1.x] == 1)
                        mvprintw(door_1.y, door_1.x, "░");
                }

                if (level[level_num].room[7].room_exist == 1)
                {
                    if (visibility_grid[level_num][door_2.y][door_2.x] == 1)
                        mvprintw(door_2.y, door_2.x, "╣");
                }
                else
                {
                    if (visibility_grid[level_num][door_2.y][door_2.x] == 1)
                        mvprintw(door_2.y, door_2.x, "░");
                }
                attroff(COLOR_PAIR(6));
                break;
            case (11):
                door_1 = level[level_num].room[7].door[2];
                door_2 = level[level_num].room[8].door[1];
                attron(COLOR_PAIR(6));
                if (level[level_num].room[7].room_exist == 1)
                {
                    if (visibility_grid[level_num][door_1.y][door_1.x] == 1)
                        mvprintw(door_1.y, door_1.x, "╠");
                }
                else
                {
                    if (visibility_grid[level_num][door_1.y][door_1.x] == 1)
                        mvprintw(door_1.y, door_1.x, "░");
                }

                if (level[level_num].room[8].room_exist == 1)
                {
                    if (visibility_grid[level_num][door_2.y][door_2.x] == 1)
                        mvprintw(door_2.y, door_2.x, "╣");
                }
                else
                {
                    if (visibility_grid[level_num][door_2.y][door_2.x] == 1)
                        mvprintw(door_2.y, door_2.x, "░");
                }
                attroff(COLOR_PAIR(6));
                break;
            }

            if (i == 0 || i == 1 || i == 5 || i == 6 || i == 10 || i == 11)
            {
                for (int j = door_1.x + 1; j <= mid_1.x + 1; j++)
                {
                    attron(COLOR_PAIR(6));
                    if (visibility_grid[level_num][door_1.y][j] == 1)
                        mvprintw(door_1.y, j, "░");
                    attroff(COLOR_PAIR(6));
                }

                if (mid_1.y < mid_2.y)
                {
                    for (int j = mid_1.y; j <= mid_2.y; j++)
                    {
                        attron(COLOR_PAIR(6));
                        if (visibility_grid[level_num][j][mid_1.x] == 1)
                            mvprintw(j, mid_1.x, "░");
                        attroff(COLOR_PAIR(6));
                    }
                    for (int j = mid_1.y; j <= mid_2.y; j++)
                    {
                        attron(COLOR_PAIR(6));
                        if (visibility_grid[level_num][j][mid_1.x + 1] == 1)
                            mvprintw(j, mid_1.x + 1, "░");
                        attroff(COLOR_PAIR(6));
                    }
                }
                else
                {
                    for (int j = mid_2.y; j <= mid_1.y; j++)
                    {
                        attron(COLOR_PAIR(6));
                        if (visibility_grid[level_num][j][mid_2.x] == 1)
                            mvprintw(j, mid_2.x, "░");
                        attroff(COLOR_PAIR(6));
                    }
                    for (int j = mid_2.y; j <= mid_1.y; j++)
                    {
                        attron(COLOR_PAIR(6));
                        if (visibility_grid[level_num][j][mid_2.x + 1] == 1)
                            mvprintw(j, mid_2.x + 1, "░");
                        attroff(COLOR_PAIR(6));
                    }
                }

                for (int j = mid_2.x; j < door_2.x; j++)
                {
                    attron(COLOR_PAIR(6));
                    if (visibility_grid[level_num][mid_2.y][j] == 1)
                        mvprintw(mid_2.y, j, "░");
                    attroff(COLOR_PAIR(6));
                }
            }

            else
            {
                for (int j = door_1.y + 1; j <= mid_1.y; j++)
                {
                    attron(COLOR_PAIR(6));
                    if (visibility_grid[level_num][j][door_1.x] == 1)
                        mvprintw(j, door_1.x, "░");
                    attroff(COLOR_PAIR(6));
                }

                for (int j = door_1.y + 1; j <= mid_1.y; j++)
                {
                    attron(COLOR_PAIR(6));
                    if (visibility_grid[level_num][j][door_1.x + 1] == 1)
                        mvprintw(j, door_1.x + 1, "░");
                    attroff(COLOR_PAIR(6));
                }

                if (mid_1.x < mid_2.x)
                {
                    for (int j = mid_1.x; j <= mid_2.x; j++)
                    {
                        attron(COLOR_PAIR(6));
                        if (visibility_grid[level_num][mid_1.y][j] == 1)
                            mvprintw(mid_1.y, j, "░");
                        attroff(COLOR_PAIR(6));
                    }
                }
                else
                {
                    for (int j = mid_2.x; j <= mid_1.x; j++)
                    {
                        attron(COLOR_PAIR(6));
                        if (visibility_grid[level_num][mid_2.y][j] == 1)
                            mvprintw(mid_2.y, j, "░");
                        attroff(COLOR_PAIR(6));
                    }
                }

                for (int j = mid_2.y; j < door_2.y; j++)
                {
                    attron(COLOR_PAIR(6));
                    if (visibility_grid[level_num][j][mid_2.x] == 1)
                        mvprintw(j, mid_2.x, "░");
                    attroff(COLOR_PAIR(6));
                }
                for (int j = mid_2.y; j < door_2.y; j++)
                {
                    attron(COLOR_PAIR(6));
                    if (visibility_grid[level_num][j][mid_2.x + 1] == 1)
                        mvprintw(j, mid_2.x + 1, "░");
                    attroff(COLOR_PAIR(6));
                }
            }
        }
    }
}

void initialize_hero(int level_num)
{
    hero.location = random_location(level_num);
    hero.face_option = current_user.color_option;
    map[level_num][hero.location.y][hero.location.x] = 2;
    hero.currect_weapon_num = 0;
    hero.current_weapon.type = EmptyWeapon;
}

void print_hero()
{
    mvprintw(hero.location.y, hero.location.x, "%s", face[hero.face_option]);
}

void handle_move_command(int level_num, int input)
{
    Point origin, destination;
    origin = hero.location;
    destination = hero.location;

    hero.dir_y = 0;
    hero.dir_x = 0;

    switch (input)
    {
    case KEY_UP:
        hero.dir_y = -1;
        break;
    case KEY_DOWN:
        hero.dir_y = 1;
        break;
    case KEY_LEFT:
        hero.dir_x = -1;
        break;
    case KEY_RIGHT:
        hero.dir_x = 1;
        break;
    }

    switch (input)
    {
    case (KEY_UP):
    case ('w'):
    case ('W'):
        destination.y -= 1;
        break;
    case (KEY_DOWN):
    case ('x'):
    case ('X'):
        destination.y += 1;
        break;
    case (KEY_LEFT):
    case ('a'):
    case ('A'):
        destination.x -= 1;
        break;
    case (KEY_RIGHT):
    case ('d'):
    case ('D'):
        destination.x += 1;
        break;
    case (KEY_HOME):
    case ('q'):
    case ('Q'):
        destination.y -= 1;
        destination.x -= 1;
        break;
    case (KEY_PPAGE):
    case ('e'):
    case ('E'):
        destination.y -= 1;
        destination.x += 1;
        break;
    case (KEY_NPAGE):
    case ('c'):
    case ('C'):
        destination.y += 1;
        destination.x += 1;
        break;
    case (KEY_END):
    case ('z'):
    case ('Z'):
        destination.y += 1;
        destination.x -= 1;
        break;
    }

    if (valid_point(level_num, destination))
        hero.location = destination;
}

int valid_point(int level_num, Point destination)
{
    int y = destination.y;
    int x = destination.x;

    for (int i = 0; i < level[level_num].enemy_num; i++)
    {
        if ((y == level[level_num].enemies[i].location.y && x == level[level_num].enemies[i].location.x) ||
            (y == level[level_num].enemies[i].location.y && x == level[level_num].enemies[i].location.x + 1) ||
            (y == level[level_num].enemies[i].location.y && x == level[level_num].enemies[i].location.x - 1))
            return 0;
    }

    for (int i = 0; i < 9; i++)
    {
        Room temp = level[level_num].room[i];
        if (temp.room_exist == 1)
        {
            if (y > temp.corner.y && y <= temp.corner.y + temp.width && x >= temp.corner.x && x < temp.corner.x + temp.length - 1)
                return 1;
        }
    }

    for (int i = 0; i < 12; i++)
    {
        if (level[level_num].corridor_exist[i] == 1)
        {
            Point door_1, door_2, mid_1, mid_2;

            mid_1 = level[level_num].between_doors_1[i];
            mid_2 = level[level_num].between_doors_2[i];

            switch (i)
            {
            case (0):
                door_1 = level[level_num].room[0].door[0];
                door_2 = level[level_num].room[1].door[0];
                break;
            case (1):
                door_1 = level[level_num].room[1].door[1];
                door_2 = level[level_num].room[2].door[0];
                break;
            case (2):
                door_1 = level[level_num].room[0].door[1];
                door_2 = level[level_num].room[3].door[0];
                break;
            case (3):
                door_1 = level[level_num].room[1].door[2];
                door_2 = level[level_num].room[4].door[0];
                break;
            case (4):
                door_1 = level[level_num].room[2].door[1];
                door_2 = level[level_num].room[5].door[0];
                break;
            case (5):
                door_1 = level[level_num].room[3].door[1];
                door_2 = level[level_num].room[4].door[1];
                break;
            case (6):
                door_1 = level[level_num].room[4].door[2];
                door_2 = level[level_num].room[5].door[1];
                break;
            case (7):
                door_1 = level[level_num].room[3].door[2];
                door_2 = level[level_num].room[6].door[0];
                break;
            case (8):
                door_1 = level[level_num].room[4].door[3];
                door_2 = level[level_num].room[7].door[0];
                break;
            case (9):
                door_1 = level[level_num].room[5].door[2];
                door_2 = level[level_num].room[8].door[0];
                break;
            case (10):
                door_1 = level[level_num].room[6].door[1];
                door_2 = level[level_num].room[7].door[1];
                break;
            case (11):
                door_1 = level[level_num].room[7].door[2];
                door_2 = level[level_num].room[8].door[1];
                break;
            }

            if (i == 0 || i == 1 || i == 5 || i == 6 || i == 10 || i == 11)
            {
                if (y == door_1.y && x >= door_1.x - 1 && x <= mid_1.x)
                    return 1;
                if (mid_1.y < mid_2.y)
                {
                    if (x == mid_1.x && y >= mid_1.y && y <= mid_2.y)
                        return 1;
                }
                else
                {
                    if (x == mid_1.x && y >= mid_2.y && y <= mid_1.y)
                        return 1;
                }
                if (y == mid_2.y && x >= mid_2.x && x <= door_2.x)
                    return 1;
            }
            else
            {
                if (x == door_1.x && y >= door_1.y && y <= mid_1.y)
                    return 1;
                if (mid_1.x < mid_2.x)
                {
                    if (y == mid_1.y && x >= mid_1.x && x <= mid_2.x)
                        return 1;
                }
                else
                {
                    if (y == mid_1.y && x >= mid_2.x && x <= mid_1.x)
                        return 1;
                }
                if (x == mid_2.x && y >= mid_2.y && y <= door_2.y)
                    return 1;
            }
        }
    }

    return 0;
}

void make_cells_visible(int level_num)
{
    int y = hero.location.y;
    int x = hero.location.x;

    for (int i = -VISIBILITY_RADIUS + 1; i < VISIBILITY_RADIUS; i++)
    {
        for (int j = -VISIBILITY_RADIUS + 1; j < VISIBILITY_RADIUS + 1; j++)
        {
            visibility_grid[level_num][y + i][x + j] = 1;
        }
    }
}

void initialize_visibility_grid()
{
    visibility_grid = (bool ***)malloc(MAX_LEVEL * sizeof(bool **));
    for (int level = 0; level < MAX_LEVEL; level++)
    {
        visibility_grid[level] = (bool **)malloc(LINES * sizeof(bool *));
        for (int i = 0; i < LINES; i++)
        {
            visibility_grid[level][i] = (bool *)malloc(COLS * sizeof(bool));
            for (int j = 0; j < COLS; j++)
            {
                visibility_grid[level][i][j] = 0;
            }
        }
    }
}

void free_visibility_grid()
{
    for (int level = 0; level < MAX_LEVEL; level++)
    {
        for (int i = 0; i < LINES; i++)
        {
            free(visibility_grid[level][i]);
        }
        free(visibility_grid[level]);
    }
    free(visibility_grid);
}

void create_objects(int level_num)
{
    int normal_food_num = (rand() % (MAX_NORMAL_FOOD - MIN_NORMAL_FOOD + 1)) + MIN_NORMAL_FOOD;
    int gourmet_food_num = (rand() % (MAX_GOURMET_FOOD - MIN_GOURMET_FOOD + 1)) + MIN_GOURMET_FOOD;
    int magic_food_num = (rand() % (MAX_MAGIC_FOOD - MIN_MAGIC_FOOD + 1)) + MIN_MAGIC_FOOD;
    int poison_food_num = (rand() % (MAX_POISON_FOOD - MIN_POISON_FOOD + 1)) + MIN_POISON_FOOD;
    int coin_num = (rand() % (MAX_COIN - MIN_COIN + 1)) + MIN_COIN;
    int black_coin_num = (rand() % (MAX_BLACK_COIN - MIN_BLACK_COIN + 1)) + MIN_BLACK_COIN;

    int health_spell_num = (rand() % (MAX_HEALTH_SPELL - MIN_HEALTH_SPELL + 1)) + MIN_HEALTH_SPELL;
    int speed_spell_num = (rand() % (MAX_SPEED_SPELL - MIN_SPEED_SPELL + 1)) + MIN_SPEED_SPELL;
    int damage_spell_num = (rand() % (MAX_DAMAGE_SPELL - MIN_DAMAGE_SPELL + 1)) + MIN_DAMAGE_SPELL;

    level[level_num].objects = (Object *)malloc(MAX_OBJECT * (sizeof(Object)));

    int staircase_room;
    while (1)
    {
        staircase_room = rand() % 9;
        if (find_room(level_num, hero.location) != staircase_room && level[level_num].room[staircase_room].room_exist == 1)
            break;
    }

    Point location;
    location = random_location_in_room(level_num, staircase_room);

    level[level_num].objects[level[level_num].object_num].location = location;
    level[level_num].objects[level[level_num].object_num].type = Staircase;
    map[level_num][level[level_num].objects[level[level_num].object_num].location.y][level[level_num].objects[level[level_num].object_num].location.x] =
        level[level_num].objects[level[level_num].object_num].type;
    level[level_num].objects[level[level_num].object_num].location_room = find_room(level_num, level[level_num].objects[level[level_num].object_num].location);

    level[level_num].object_num += 1;

    for (int i = 0; i < normal_food_num; i++)
        spawn_object(level_num, NormalFood);

    for (int i = 0; i < gourmet_food_num; i++)
        spawn_object(level_num, GourmetFood);

    for (int i = 0; i < magic_food_num; i++)
        spawn_object(level_num, MagicFood);

    for (int i = 0; i < poison_food_num; i++)
        spawn_object(level_num, PoisonFood);

    for (int i = 0; i < coin_num; i++)
        spawn_object(level_num, Coin);

    for (int i = 0; i < black_coin_num; i++)
        spawn_object(level_num, BlackCoin);

    for (int i = 0; i < health_spell_num; i++)
        spawn_object(level_num, HealthSpell);

    for (int i = 0; i < speed_spell_num; i++)
        spawn_object(level_num, SpeedSpell);

    for (int i = 0; i < damage_spell_num; i++)
        spawn_object(level_num, DamageSpell);

    spawn_object(level_num, Dagger);
    spawn_object(level_num, MagicWand);
    spawn_object(level_num, NormalArrow);
    spawn_object(level_num, Sword);
}

void initialize_map()
{
    map = (short int ***)malloc(MAX_LEVEL * sizeof(short int **));
    for (int level = 0; level < MAX_LEVEL; level++)
    {
        map[level] = (short int **)malloc(LINES * sizeof(short int *));
        for (int i = 0; i < LINES; i++)
        {
            map[level][i] = (short *)malloc(COLS * sizeof(short int));
            for (int j = 0; j < COLS; j++)
            {
                map[level][i][j] = 0;
            }
        }
    }
}

void print_objects(int level_num)
{
    for (int i = 0; i < level[level_num].object_num; i++)
    {
        if (visibility_grid[level_num][level[level_num].objects[i].location.y][level[level_num].objects[i].location.x] == 1 &&
            visibility_grid[level_num][level[level_num].objects[i].location.y][level[level_num].objects[i].location.x + 1] == 1 &&
            level[level_num].objects[i].visible == 1)
        {
            switch (level[level_num].objects[i].type)
            {
            case (Trap):
                mvprintw(level[level_num].objects[i].location.y, level[level_num].objects[i].location.x, "💣");
                break;
            case (Staircase):
                mvprintw(level[level_num].objects[i].location.y, level[level_num].objects[i].location.x, "🚪");
                break;
            case (SecretDoor):
                mvprintw(level[level_num].objects[i].location.y, level[level_num].objects[i].location.x, "🚪");
                break;
            case (PasswordDoor):
                attron(COLOR_PAIR(9));
                mvprintw(level[level_num].objects[i].location.y, level[level_num].objects[i].location.x, "💻");
                attroff(COLOR_PAIR(9));
                break;
            case (MasterKey):
                attron(COLOR_PAIR(8));
                mvprintw(level[level_num].objects[i].location.y, level[level_num].objects[i].location.x, "🔑");
                attroff(COLOR_PAIR(8));
                break;
            case (NormalFood):
                attron(COLOR_PAIR(11));
                mvprintw(level[level_num].objects[i].location.y, level[level_num].objects[i].location.x, "🍎");
                attroff(COLOR_PAIR(11));
                break;
            case (GourmetFood):
                attron(COLOR_PAIR(11));
                mvprintw(level[level_num].objects[i].location.y, level[level_num].objects[i].location.x, "🍔");
                attroff(COLOR_PAIR(11));
                break;
            case (MagicFood):
                attron(COLOR_PAIR(12));
                mvprintw(level[level_num].objects[i].location.y, level[level_num].objects[i].location.x, "🍺");
                attroff(COLOR_PAIR(12));
                break;
            case (PoisonFood):
                attron(COLOR_PAIR(13));
                mvprintw(level[level_num].objects[i].location.y, level[level_num].objects[i].location.x, "🍎");
                attroff(COLOR_PAIR(13));
                break;
            case (Coin):
                attron(COLOR_PAIR(7));
                mvprintw(level[level_num].objects[i].location.y, level[level_num].objects[i].location.x, "⭐️");
                attroff(COLOR_PAIR(7));
                break;
            case (Mace):
                attron(COLOR_PAIR(9));
                mvprintw(level[level_num].objects[i].location.y, level[level_num].objects[i].location.x, "🪓");
                attroff(COLOR_PAIR(9));
                break;
            case (Dagger):
                attron(COLOR_PAIR(9));
                mvprintw(level[level_num].objects[i].location.y, level[level_num].objects[i].location.x, "🔪");
                attroff(COLOR_PAIR(9));
                break;
            case (MagicWand):
                mvprintw(level[level_num].objects[i].location.y, level[level_num].objects[i].location.x, "✨");
                break;
            case (NormalArrow):
                mvprintw(level[level_num].objects[i].location.y, level[level_num].objects[i].location.x, "🏹");
                break;
            case (Sword):
                attron(COLOR_PAIR(9));
                mvprintw(level[level_num].objects[i].location.y, level[level_num].objects[i].location.x, "🔫");
                attroff(COLOR_PAIR(9));
                break;
            case (HealthSpell):
                attron(COLOR_PAIR(3));
                mvprintw(level[level_num].objects[i].location.y, level[level_num].objects[i].location.x, "💊");
                attroff(COLOR_PAIR(3));
                break;
            case (SpeedSpell):
                attron(COLOR_PAIR(8));
                mvprintw(level[level_num].objects[i].location.y, level[level_num].objects[i].location.x, "💥");
                attroff(COLOR_PAIR(8));
                break;
            case (DamageSpell):
                mvprintw(level[level_num].objects[i].location.y, level[level_num].objects[i].location.x, "🔥");
                break;
            case (BlackCoin):
                mvprintw(level[level_num].objects[i].location.y, level[level_num].objects[i].location.x, "💎");
                break;
            }
        }
    }
}

Point random_location(int level_num)
{
    Point result;
    int first_room;

    while (1)
    {
        first_room = rand() % 9;
        if (level[level_num].room[first_room].room_exist == 1)
        {
            Room room = level[level_num].room[first_room];

            result.y = room.corner.y + 1;
            result.x = room.corner.x;

            result.y += (rand() % (room.width - 2) / 2) + 1;
            result.x += (rand() % ((room.length - 2) / 2) * 2) + 1;

            if (map[level_num][result.y][result.x] == 0 && map[level_num][result.y][result.x + 2] == 0)
                return result;
        }
    }
}

int handle_location(int level_num)
{
    hero.location_room = find_room(level_num, hero.location);
    level[level_num].room[hero.location_room].discovered = 1;

    level[level_num].discovered_room_num = 0;
    for (int i = 0; i < 9; i++)
    {
        if (level[level_num].room[i].discovered == 1)
            level[level_num].discovered_room_num += 1;
    }

    int y = hero.location.y;
    int x = hero.location.x;

    for (int i = 0; i < level[level_num].object_num; i++)
    {
        if (level[level_num].objects[i].location_room == hero.location_room)
            level[level_num].objects[i].visible = 1;
        else
            level[level_num].objects[i].visible = 0;
    }

    if (hero.location_room == -1)
        return -1;

    for (int i = 0; i < level[level_num].object_num; i++)
    {
        if ((level[level_num].objects[i].location.y == y && level[level_num].objects[i].location.x == x) ||
            (level[level_num].objects[i].location.y == y && level[level_num].objects[i].location.x + 1 == x) ||
            (level[level_num].objects[i].location.y == y && level[level_num].objects[i].location.x - 1 == x))
        {
            level[level_num].objects[i].visible = 0;
            object_index = i;
        }
    }

    for (int i = 0; i < level[level_num].object_num; i++)
    {
        for (int j = 0; j < level[level_num].enemy_num; j++)
        {
            y = level[level_num].enemies[j].location.y;
            x = level[level_num].enemies[j].location.x;
            if ((level[level_num].objects[i].location.y == y && level[level_num].objects[i].location.x == x) ||
                (level[level_num].objects[i].location.y == y && level[level_num].objects[i].location.x + 1 == x) ||
                (level[level_num].objects[i].location.y == y && level[level_num].objects[i].location.x - 1 == x))
            {
                level[level_num].objects[i].visible = 0;
            }
        }
    }

    y = hero.location.y;
    x = hero.location.x;

    for (int i = 0; i < level[level_num].enemy_num; i++)
    {
        if ((level[level_num].enemies[i].location.y == y && level[level_num].enemies[i].location.x == x) ||
            (level[level_num].enemies[i].location.y == y && level[level_num].enemies[i].location.x + 1 == x) ||
            (level[level_num].enemies[i].location.y == y && level[level_num].enemies[i].location.x - 1 == x))
        {
            level[level_num].enemies[i].visible = 0;
        }
    }

    if (object_index == -1)
    {
        for (int i = 0; i < level[level_num].enemy_num; i++)
        {
            if ((level[level_num].enemies[i].location.y == y && level[level_num].enemies[i].location.x - 2 == x) ||
                (level[level_num].enemies[i].location.y == y && level[level_num].enemies[i].location.x + 2 == x) ||
                (level[level_num].enemies[i].location.y - 1 == y && level[level_num].enemies[i].location.x == x) ||
                (level[level_num].enemies[i].location.y - 1 == y && level[level_num].enemies[i].location.x - 1 == x) ||
                (level[level_num].enemies[i].location.y + 1 == y && level[level_num].enemies[i].location.x == x) ||
                (level[level_num].enemies[i].location.y + 1 == y && level[level_num].enemies[i].location.x - 1 == x))
            {
                enemy_index = i;
            }
        }
    }

    if (object_index == -1 && enemy_index == -1)
        return -1;

    else if (enemy_index != -1 && object_index == -1)
    {
        if (hero.health >= level[level_num].enemies[enemy_index].damage)
            hero.health -= level[level_num].enemies[enemy_index].damage;
        else
            hero.health = 0;

        switch (level[level_num].enemies[enemy_index].type)
        {
        case (deamon):
            return 20;
            break;
        case (fire_monster):
            return 21;
            break;
        case (giant):
            return 22;
            break;
        case (snake):
            return 23;
            break;
        case (undead):
            return 24;
            break;
        }
    }

    else if (enemy_index == -1 && object_index != -1)
    {
        switch (level[level_num].objects[object_index].type)
        {
        case (Trap):
        {
            hero.health -= 20;
            int room_num;
            while (1)
            {
                room_num = rand() % 9;
                if (level[level_num].room[room_num].room_exist == 1)
                    break;
            }

            remove_object(level_num, object_index);
            break;
        }
        case (NormalFood):
        {
            return 5;
            break;
        }
        case (GourmetFood):
        {
            return 6;
            break;
        }
        case (MagicFood):
        {
            return 7;
            break;
        }
        case (PoisonFood):
        {
            return 2;
            break;
        }
        case (Coin):
        {
            if (level[level_num].room[find_room(level_num, hero.location)].type != Nightmare)
            {
                added_coin = (rand() % (MAX_COIN_POINT - MIN_COIN_POINT + 1)) + MIN_COIN_POINT;
                hero.coins += added_coin;
                current_user.total_gold += added_coin;
                remove_object(level_num, object_index);
            }
            return 3;
            break;
        }
        case (Dagger):
        {
            return 8;
            break;
        }
        case (MagicWand):
        {
            return 9;
            break;
        }
        case (NormalArrow):
        {
            return 10;
            break;
        }
        case (Sword):
        {
            return 11;
            break;
        }
        case (BlackCoin):
        {
            if (level[level_num].room[find_room(level_num, hero.location)].type != Nightmare)
            {
                added_coin = 30;
                hero.coins += added_coin;
                current_user.total_gold += added_coin;
                remove_object(level_num, object_index);
            }
            return 12;
            break;
        }
        case (HealthSpell):
        {
            return 13;
            break;
        }
        case (SpeedSpell):
        {
            return 14;
            break;
        }
        case (DamageSpell):
        {
            return 15;
            break;
        }
        case (Staircase):
        {
            return 1;
            break;
        }
        default:
        {
            return -1;
            break;
        }
        }
    }
}

void remove_object(int level_num, int object_index)
{
    int y = level[level_num].objects[object_index].location.y;
    int x = level[level_num].objects[object_index].location.x;

    for (int i = object_index; i < level[level_num].object_num - 1; i++)
    {
        level[level_num].objects[i] = level[level_num].objects[i + 1];
    }
    level[level_num].object_num -= 1;

    map[level_num][y][x] = 0;
}

void remove_enemy(int level_num, int enemy_index)
{
    int y = level[level_num].enemies[enemy_index].location.y;
    int x = level[level_num].enemies[enemy_index].location.x;

    for (int i = enemy_index; i < level[level_num].enemy_num - 1; i++)
    {
        level[level_num].enemies[i] = level[level_num].enemies[i + 1];
    }
    level[level_num].enemy_num -= 1;

    map[level_num][y][x] = 0;
}

int find_room(int level_num, Point location)
{
    for (int i = 0; i < 9; i++)
    {
        Room temp = level[level_num].room[i];
        if (temp.room_exist == 1)
        {
            if (location.y >= temp.corner.y && location.y <= temp.corner.y + temp.width + 1 &&
                location.x >= temp.corner.x - 2 && location.x <= temp.corner.x + temp.length)
                return i;
        }
    }

    return -1;
}

void draw_info_border()
{
    mvprintw(1, 3, "__ Vitals ");
    for (int i = 13; i < COLS - 4; i++)
        mvprintw(1, i, "_");
    for (int i = 0; i < 3; i++)
    {
        mvprintw(2 + i, 3, "|");
        mvprintw(2 + i, COLS - 5, "|");
    }
    for (int i = 3; i < COLS - 4; i++)
        mvprintw(5, i, "‾");
}

void print_info(int level_num)
{
    for (int i = 0; i < 5; i++)
    {
        move(1 + i, 0);
        clrtoeol();
    }
    draw_info_border();
    mvprintw(3, 8, "Level: %d", level_num + 1);
    move(3, 8 + (COLS / 7));
    printw("Health: ");
    if (hero.health > 20)
    {
        attron(COLOR_PAIR(4));
        for (int i = 0; i < hero.health / 5; i++)
            printw("█");
        attroff(COLOR_PAIR(4));
        mvprintw(3, 37 + (COLS / 7), " %d %%", hero.health);
    }
    else
    {
        attron(COLOR_PAIR(6));
        for (int i = 0; i < hero.health / 5; i++)
            printw("█");
        attroff(COLOR_PAIR(6));
        mvprintw(3, 37 + (COLS / 7), " %d %%", hero.health);
    }
    mvprintw(3, 60 + (COLS / 7), "Satiety: %d %%", hero.satiety);
    mvprintw(3, 65 + (2 * (COLS / 7)), "Stars: %d", hero.coins);
}

void draw_message_border()
{
    mvprintw(LINES - 5, 3, "__ Message ");
    for (int i = 14; i < COLS - 4; i++)
        mvprintw(LINES - 5, i, "_");
    for (int i = 0; i < 3; i++)
    {
        mvprintw(LINES - 4 + i, 3, "|");
        mvprintw(LINES - 4 + i, COLS - 5, "|");
    }
    for (int i = 3; i < COLS - 4; i++)
        mvprintw(LINES - 1, i, "‾");
}

void print_message(char *message, char *guide)
{
    draw_message_border();
    mvprintw(LINES - 3, 8, "%s ", message);
    attron(A_REVERSE);
    printw("%s", guide);
    attroff(A_REVERSE);
    refresh();
}

void spawn_object(int level_num, ObjectType type)
{
    Point location;
    location = random_location(level_num);

    level[level_num].objects[level[level_num].object_num].location = location;
    level[level_num].objects[level[level_num].object_num].type = type;
    map[level_num][level[level_num].objects[level[level_num].object_num].location.y][level[level_num].objects[level[level_num].object_num].location.x] =
        level[level_num].objects[level[level_num].object_num].type;
    level[level_num].objects[level[level_num].object_num].location_room = find_room(level_num, level[level_num].objects[level[level_num].object_num].location);

    level[level_num].object_num += 1;
}

Point random_location_in_room(int level_num, int room_num)
{
    Point result;
    int first_room;

    while (1)
    {
        first_room = rand() % 9;
        if (first_room == room_num)
        {
            if (level[level_num].room[first_room].room_exist == 1)
            {
                Room room = level[level_num].room[first_room];

                result.y = room.corner.y + 1;
                result.x = room.corner.x;

                result.y += (rand() % ((room.width - 2) / 2) * 2) + 1;
                result.x += (rand() % ((room.length - 2) / 2) * 2) + 1;

                if (map[level_num][result.y][result.x] == 0 && map[level_num][result.y][result.x + 2] == 0)
                    return result;
            }
        }
    }
}

void draw_inventory_border()
{
    for (int i = 3; i < COLS - 4; i++)
        mvprintw(LINES - 5, i, "_");
    for (int i = 0; i < 3; i++)
    {
        mvprintw(LINES - 4 + i, 3, "|");
        mvprintw(LINES - 4 + i, COLS - 5, "|");
    }
    for (int i = 3; i < COLS - 4; i++)
        mvprintw(LINES - 1, i, "‾");
}

void show_food_inventory(int level_num)
{
    int choice = 0;

    while (1)
    {
        print_info(level_num);
        for (int i = 0; i < 5; i++)
        {
            move(LINES - 5 + i, 0);
            clrtoeol();
        }
        draw_inventory_border();

        int food = 0, gourmet = 0, magic = 0;
        for (int i = 0; i < hero.food_num; i++)
        {
            switch (hero.food_inventory[i].type)
            {
            case (NormalFood):
            case (PoisonFood):
                food += 1;
                break;
            case (GourmetFood):
                gourmet += 1;
                break;
            case (MagicFood):
                magic += 1;
                break;
            }
        }

        mvprintw(LINES - 5, 5, " Food Inventory (Press '1' to close memu.) ");

        if (choice == 0)
            attron(A_REVERSE);
        mvprintw(LINES - 3, 8, "Normal Food (🍎): %d", food);
        if (choice == 0)
            attroff(A_REVERSE);

        if (choice == 1)
            attron(A_REVERSE);
        mvprintw(LINES - 3, 8 + (COLS / 4), "Gourmet Food (🍔): %d", gourmet);
        if (choice == 1)
            attroff(A_REVERSE);

        if (choice == 2)
            attron(A_REVERSE);
        mvprintw(LINES - 3, 8 + ((2 * (COLS / 4))), "Magic Food (🍺): %d", magic);
        if (choice == 2)
            attroff(A_REVERSE);

        refresh();

        int temp = getch();

        if (temp == '1')
            break;

        if (temp == KEY_RIGHT)
            choice = (choice + 1) % 3;

        else if (temp == KEY_LEFT)
            choice = (choice + 2) % 3;

        else if (temp == 10 || temp == 32)
        {
            if (choice == 0)
            {
                if (food > 0)
                {
                    int food_index;
                    for (int i = 0; i < hero.food_num; i++)
                    {
                        if (hero.food_inventory[i].type == NormalFood || hero.food_inventory[i].type == PoisonFood)
                        {
                            food_index = i;
                            break;
                        }
                    }

                    if (hero.food_inventory[food_index].type == NormalFood)
                    {
                        if (hero.satiety >= 90)
                            hero.satiety = 100;
                        else
                            hero.satiety += 10;
                    }

                    else if (hero.food_inventory[food_index].type == PoisonFood)
                    {
                        if (hero.health <= 10)
                            hero.satiety = 0;
                        else
                            hero.satiety -= 10;
                    }

                    remove_food_from_inventory(food_index);
                }
            }

            if (choice == 1)
            {
                if (gourmet > 0)
                {
                    int food_index;
                    for (int i = 0; i < hero.food_num; i++)
                    {
                        if (hero.food_inventory[i].type == GourmetFood)
                        {
                            food_index = i;
                            break;
                        }
                    }

                    if (hero.satiety >= 80)
                        hero.satiety = 100;
                    else
                        hero.satiety += 20;

                    if (hero.health >= 90)
                        hero.health = 100;
                    else
                        hero.health += 10;

                    remove_food_from_inventory(food_index);
                }
            }

            if (choice == 2)
            {
                if (food > 0)
                {
                    int food_index;
                    for (int i = 0; i < hero.food_num; i++)
                    {
                        if (hero.food_inventory[i].type == MagicFood)
                        {
                            food_index = i;
                            break;
                        }
                    }

                    if (hero.health >= 90)
                        hero.satiety = 100;
                    else
                        hero.health += 10;

                    if (hero.speed >= 80)
                        hero.health = 100;
                    else
                        hero.health += 20;

                    remove_food_from_inventory(food_index);
                }
            }
        }
    }

    for (int i = 0; i < 5; i++)
    {
        move(LINES - 5 + i, 0);
        clrtoeol();
    }
}

void remove_food_from_inventory(int food_index)
{
    int y = hero.food_inventory[food_index].location.y;
    int x = hero.food_inventory[food_index].location.x;

    for (int i = food_index; i < hero.food_num - 1; i++)
    {
        hero.food_inventory[i] = hero.food_inventory[i + 1];
    }
    hero.food_num -= 1;
}

void show_weapon_inventory()
{
    int choice = 0;

    while (1)
    {
        for (int i = 0; i < 5; i++)
        {
            move(LINES - 5 + i, 0);
            clrtoeol();
        }
        draw_inventory_border();

        int counts[5] = {0};
        for (int i = 0; i < hero.weapon_num; i++)
        {
            switch (hero.weapon_inventory[i].type)
            {
            case Mace:
                counts[0]++;
                break;
            case Dagger:
                counts[1]++;
                break;
            case MagicWand:
                counts[2]++;
                break;
            case NormalArrow:
                counts[3]++;
                break;
            case Sword:
                counts[4]++;
                break;
            }
        }

        // Display inventory
        mvprintw(LINES - 5, 5, "Weapon Inventory [Equipped: %s] (Press '2' to close)",
                 weapon_name(hero.current_weapon.type));

        char *weapons[] = {"Mace (🪓)", "Dagger (🔪)", "Magic Wand (✨)",
                           "Normal Arrow (🏹)", "Gun (🔫)"};

        for (int i = 0; i < 5; i++)
        {
            if (choice == i)
                attron(A_REVERSE);
            mvprintw(LINES - 3, 8 + (i * (COLS / 6)), "%s: %d",
                     weapons[i], counts[i]);
            if (choice == i)
                attroff(A_REVERSE);
        }
        refresh();
        int temp = getch();

        if (temp == '2')
            break;

        if (temp == KEY_RIGHT)
            choice = (choice + 1) % 5;

        else if (temp == KEY_LEFT)
            choice = (choice + 4) % 5;

        else if ((temp == 10 || temp == 32) && hero.current_weapon.type == EmptyWeapon)
        {
            if (counts[choice] > 0)
            {
                for (int i = 0; i < hero.weapon_num; i++)
                {
                    if (hero.weapon_inventory[i].type == (ObjectType)(Mace + choice))
                    {
                        equip_weapon(i);
                        return;
                    }
                }
            }
            break;
        }
    }

    for (int i = 0; i < 5; i++)
    {
        move(LINES - 5 + i, 0);
        clrtoeol();
    }
}

void remove_weapon_from_inventory(int weapon_index)
{
    int y = hero.weapon_inventory[weapon_index].location.y;
    int x = hero.weapon_inventory[weapon_index].location.x;

    for (int i = weapon_index; i < hero.weapon_num - 1; i++)
    {
        hero.weapon_inventory[i] = hero.weapon_inventory[i + 1];
    }
    hero.weapon_num -= 1;
}

void add_current_weapon_to_inventory()
{
    if (hero.current_weapon.type == EmptyWeapon)
    {
        move(LINES - 3, 0);
        clrtoeol();
        print_message("No weapon equipped!", "Press any key...");
        getch();
        return;
    }

    Object new_weapon = {
        .type = hero.current_weapon.type,
        .location = hero.location,
        .visible = 1,
        .location_room = hero.location_room};

    hero.weapon_inventory[hero.weapon_num] = new_weapon;
    hero.weapon_num += 1;

    hero.current_weapon.type = EmptyWeapon;
    move(LINES - 3, 0);
    clrtoeol();
    print_message("Weapon stored in inventory", "Press any key...");
    getch();
}

void equip_weapon(int weapon_index)
{
    if (weapon_index < 0 || weapon_index >= hero.weapon_num)
    {
        move(LINES - 3, 0);
        clrtoeol();
        print_message("Invalid weapon selection", "Press any key...");
        getch();
        return;
    }

    if (hero.current_weapon.type != EmptyWeapon)
    {
        add_current_weapon_to_inventory();
    }

    hero.current_weapon.type = hero.weapon_inventory[weapon_index].type;

    remove_weapon_from_inventory(weapon_index);
    move(LINES - 3, 0);
    clrtoeol();
    print_message("Weapon equipped!", "Press any key...");
    getch();
}

const char *weapon_name(ObjectType type)
{
    switch (type)
    {
    case Mace:
        return "Mace";
    case Dagger:
        return "Dagger";
    case MagicWand:
        return "Magic Wand";
    case NormalArrow:
        return "Arrows";
    case Sword:
        return "Sword";
    default:
        return "None";
    }
}

void show_spell_inventory(int level_num)
{
    int choice = 0;

    while (1)
    {
        print_info(level_num);
        for (int i = 0; i < 5; i++)
        {
            move(LINES - 5 + i, 0);
            clrtoeol();
        }
        draw_inventory_border();

        int health = 0, speed = 0, damage = 0;
        for (int i = 0; i < hero.spell_num; i++)
        {
            switch (hero.spell_inventory[i].type)
            {
            case (HealthSpell):
                health += 1;
                break;
            case (SpeedSpell):
                speed += 1;
                break;
            case (DamageSpell):
                damage += 1;
                break;
            }
        }
        mvprintw(LINES - 5, 5, "Spell Inventory (Press '3' to close memu.)");

        if (choice == 0)
            attron(A_REVERSE);
        mvprintw(LINES - 3, 8, "Health Spell (💊): %d", health);
        if (choice == 0)
            attroff(A_REVERSE);

        if (choice == 1)
            attron(A_REVERSE);
        mvprintw(LINES - 3, 8 + (COLS / 4), "Speed Spell (💥): %d", speed);
        if (choice == 1)
            attroff(A_REVERSE);

        if (choice == 2)
            attron(A_REVERSE);
        mvprintw(LINES - 3, 8 + ((2 * (COLS / 4))), "Damage Spell (🔥): %d", damage);
        if (choice == 2)
            attroff(A_REVERSE);

        refresh();
        int temp = getch();

        if (temp == '3')
            break;

        if (temp == KEY_RIGHT)
            choice = (choice + 1) % 3;

        else if (temp == KEY_LEFT)
            choice = (choice + 2) % 3;
    }

    for (int i = 0; i < 5; i++)
    {
        move(LINES - 5 + i, 0);
        clrtoeol();
    }
}

void remove_spell_from_inventory(int spell_index)
{
    int y = hero.spell_inventory[spell_index].location.y;
    int x = hero.spell_inventory[spell_index].location.x;

    for (int i = spell_index; i < hero.spell_num - 1; i++)
    {
        hero.spell_inventory[i] = hero.spell_inventory[i + 1];
    }
    hero.spell_num -= 1;
}

void show_current_weapon()
{
    int mace = 0, dagger = 0, wand = 0, arrow = 0, sword = 0;

    for (int i = 0; i < hero.weapon_num; i++)
    {
        switch (hero.weapon_inventory[i].type)
        {
        case Mace:
            mace += 1;
            break;
        case Dagger:
            dagger += 10;
            break;
        case MagicWand:
            wand += 8;
            break;
        case NormalArrow:
            arrow += 20;
            break;
        case Sword:
            sword += 1;
            break;
        }
    }

    switch (hero.current_weapon.type)
    {
    case Mace:
        mace += 1;
        break;
    case Dagger:
        dagger += 10;
        break;
    case MagicWand:
        wand += 8;
        break;
    case NormalArrow:
        arrow += 20;
        break;
    case Sword:
        sword += 1;
        break;
    case EmptyWeapon:
        break;
    }

    switch (hero.current_weapon.type)
    {
    case Mace:
        hero.currect_weapon_num = mace;
        break;
    case Dagger:
        hero.currect_weapon_num = dagger;
        break;
    case MagicWand:
        hero.currect_weapon_num = wand;
        break;
    case NormalArrow:
        hero.currect_weapon_num = arrow;
        break;
    case Sword:
        hero.currect_weapon_num = sword;
        break;
    case EmptyWeapon:
        hero.currect_weapon_num = 0;
        break;
    }

    while (1)
    {
        for (int i = 0; i < 5; i++)
        {
            move(LINES - 5 + i, 0);
            clrtoeol();
        }
        draw_inventory_border();

        mvprintw(LINES - 5, 5, " Current Weapon (Press 'i' to close menu.) ");

        if (hero.current_weapon.type == Mace)
        {
            mvprintw(LINES - 3, 8, "Mace (🪓): %d", hero.currect_weapon_num);
            mvprintw(LINES - 3, 8 + (((COLS / 6))), "Damage: %d", 5);
            mvprintw(LINES - 3, 8 + ((2 * (COLS / 6))), "Press 'q' to drop weapon.");
        }

        if (hero.current_weapon.type == Dagger)
        {
            mvprintw(LINES - 3, 8, "Dagger (🔪): %d", hero.currect_weapon_num);
            mvprintw(LINES - 3, 8 + (((COLS / 6))), "Damage: %d", 12);
            mvprintw(LINES - 3, 8 + ((2 * (COLS / 6))), "Press 'q' to drop weapon.");
        }

        if (hero.current_weapon.type == MagicWand)
        {
            mvprintw(LINES - 3, 8, "Magic Wand (✨): %d", hero.currect_weapon_num);
            mvprintw(LINES - 3, 8 + (((COLS / 6))), "Damage: %d", 15);
            mvprintw(LINES - 3, 8 + ((2 * (COLS / 6))), "Press 'q' to drop weapon.");
        }

        if (hero.current_weapon.type == NormalArrow)
        {
            mvprintw(LINES - 3, 8, "Normal Arrow (🏹): %d", hero.currect_weapon_num);
            mvprintw(LINES - 3, 8 + (((COLS / 6))), "Damage: %d", 5);
            mvprintw(LINES - 3, 8 + ((2 * (COLS / 6))), "Press 'q' to drop weapon.");
        }

        if (hero.current_weapon.type == Sword)
        {
            mvprintw(LINES - 3, 8, "Gun (🔫): %d", hero.currect_weapon_num);
            mvprintw(LINES - 3, 8 + (((COLS / 6))), "Damage: %d", 10);
            mvprintw(LINES - 3, 8 + ((2 * (COLS / 6))), "Press 'q' to drop weapon.");
        }

        if (hero.current_weapon.type == EmptyWeapon)
        {
            mvprintw(LINES - 3, 8, "You're not currently carrying a weapon. Check your Weapon Inventory to pick up a weapon.");
        }

        refresh();
        int temp = getch();

        if (temp == 'i' || temp == 'I')
            break;

        if (temp == 'q' || temp == 'Q')
        {
            add_current_weapon_to_inventory();
            hero.current_weapon.type = EmptyWeapon;
            hero.currect_weapon_num = 0;
        }
    }

    for (int i = 0; i < 5; i++)
    {
        move(LINES - 5 + i, 0);
        clrtoeol();
    }
}

void print_entire_map(int level_num)
{
    clear();
    print_message("Map", "Press 'q' to close map.");
    for (int i = 0; i < 9; i++)
        print_entire_room(level_num, i);

    print_entire_corridors(level_num);

    while (1)
    {
        int temp = getch();
        if (temp == 'q')
            break;
    }

    // print_objects(level_num);
}

void print_entire_room(int level_num, int room_num)
{
    int x = level[level_num].room[room_num].corner.x;
    int y = level[level_num].room[room_num].corner.y;
    int length = level[level_num].room[room_num].length;
    int width = level[level_num].room[room_num].width;

    int chance_check = rand() % 8;
    int not_printed = 0;

    if (level[level_num].room[room_num].room_exist == 1)
    {
        for (int i = 0; i < length; i++)
        {
            attron(COLOR_PAIR(6));
            if (visibility_grid[level_num][y][x + i] == 1)
                mvprintw(y, x + i, "═");
            attroff(COLOR_PAIR(6));
            for (int j = 1; j < width + 1; j++)
            {
                attron(COLOR_PAIR(4));
                if (visibility_grid[level_num][y + j][x + i] == 1)
                    mvprintw(y + j, x + i, "·"); // middle dot - ASCII Number: 183
                attroff(COLOR_PAIR(4));
            }
            attron(COLOR_PAIR(6));
            if (visibility_grid[level_num][y + width + 1][x + i] == 1)
                mvprintw(y + width + 1, x + i, "═");
            attroff(COLOR_PAIR(6));
        }

        for (int i = 0; i < width; i++)
        {
            attron(COLOR_PAIR(6));
            if (visibility_grid[level_num][y + i + 1][x - 1] == 1)
                mvprintw(y + i + 1, x - 1, "║");
            if (visibility_grid[level_num][y + i + 1][x + length] == 1)
                mvprintw(y + i + 1, x + length, "║");
            attroff(COLOR_PAIR(6));
        }

        attron(COLOR_PAIR(6));
        if (visibility_grid[level_num][y][x - 1] == 1)
            mvprintw(y, x - 1, "╔");
        if (visibility_grid[level_num][y][x + length] == 1)
            mvprintw(y, x + length, "╗");
        if (visibility_grid[level_num][y + width + 1][x - 1] == 1)
            mvprintw(y + width + 1, x - 1, "╚");
        if (visibility_grid[level_num][y + width + 1][x + length] == 1)
            mvprintw(y + width + 1, x + length, "╝");
        attroff(COLOR_PAIR(6));
    }
}

void print_entire_corridors(int level_num)
{
    for (int i = 0; i < 12; i++)
    {
        if (level[level_num].corridor_exist[i] == 1)
        {
            Point mid_1 = level[level_num].between_doors_1[i];
            Point mid_2 = level[level_num].between_doors_2[i];

            Point door_1, door_2;

            switch (i)
            {
            case (0):
                door_1 = level[level_num].room[0].door[0];
                door_2 = level[level_num].room[1].door[0];
                attron(COLOR_PAIR(6));
                if (level[level_num].room[0].room_exist == 1)
                {
                    mvprintw(door_1.y, door_1.x, "╠");
                }
                else
                {
                    mvprintw(door_1.y, door_1.x, "░");
                }

                if (level[level_num].room[1].room_exist == 1)
                {
                    mvprintw(door_2.y, door_2.x, "╣");
                }
                else
                {
                    mvprintw(door_2.y, door_2.x, "░");
                }
                attroff(COLOR_PAIR(6));
                break;
            case (1):
                door_1 = level[level_num].room[1].door[1];
                door_2 = level[level_num].room[2].door[0];
                attron(COLOR_PAIR(6));
                if (level[level_num].room[1].room_exist == 1)
                {
                    mvprintw(door_1.y, door_1.x, "╠");
                }
                else
                {
                    mvprintw(door_1.y, door_1.x, "░");
                }

                if (level[level_num].room[2].room_exist == 1)
                {
                    mvprintw(door_2.y, door_2.x, "╣");
                }
                else
                {
                    mvprintw(door_2.y, door_2.x, "░");
                }
                attroff(COLOR_PAIR(6));
                break;
            case (2):
                door_1 = level[level_num].room[0].door[1];
                door_2 = level[level_num].room[3].door[0];
                attron(COLOR_PAIR(6));
                if (level[level_num].room[0].room_exist == 1)
                {
                    mvprintw(door_1.y, door_1.x, "╦");
                    mvprintw(door_1.y, door_1.x + 1, "╦");
                }
                else
                {
                    mvprintw(door_1.y, door_1.x, "░");
                }

                if (level[level_num].room[3].room_exist == 1)
                {
                    mvprintw(door_2.y, door_2.x, "╩");
                    mvprintw(door_2.y, door_2.x + 1, "╩");
                }
                else
                {
                    mvprintw(door_2.y, door_2.x, "░");
                }
                attroff(COLOR_PAIR(6));
                break;
            case (3):
                door_1 = level[level_num].room[1].door[2];
                door_2 = level[level_num].room[4].door[0];
                attron(COLOR_PAIR(6));
                if (level[level_num].room[1].room_exist == 1)
                {
                    mvprintw(door_1.y, door_1.x, "╦");
                    mvprintw(door_1.y, door_1.x + 1, "╦");
                }
                else
                {
                    mvprintw(door_1.y, door_1.x, "░");
                }

                if (level[level_num].room[4].room_exist == 1)
                {
                    mvprintw(door_2.y, door_2.x, "╩");
                    mvprintw(door_2.y, door_2.x + 1, "╩");
                }
                else
                {
                    mvprintw(door_2.y, door_2.x, "░");
                }
                attroff(COLOR_PAIR(6));
                break;
            case (4):
                door_1 = level[level_num].room[2].door[1];
                door_2 = level[level_num].room[5].door[0];
                attron(COLOR_PAIR(6));
                if (level[level_num].room[2].room_exist == 1)
                {
                    mvprintw(door_1.y, door_1.x, "╦");
                    mvprintw(door_1.y, door_1.x + 1, "╦");
                }
                else
                {
                    mvprintw(door_1.y, door_1.x, "░");
                }

                if (level[level_num].room[5].room_exist == 1)
                {
                    mvprintw(door_2.y, door_2.x, "╩");
                    mvprintw(door_2.y, door_2.x + 1, "╩");
                }
                else
                {
                    mvprintw(door_2.y, door_2.x, "░");
                }
                attroff(COLOR_PAIR(6));
                break;
            case (5):
                door_1 = level[level_num].room[3].door[1];
                door_2 = level[level_num].room[4].door[1];
                attron(COLOR_PAIR(6));
                if (level[level_num].room[3].room_exist == 1)
                {
                    mvprintw(door_1.y, door_1.x, "╠");
                }
                else
                {
                    mvprintw(door_1.y, door_1.x, "░");
                }

                if (level[level_num].room[4].room_exist == 1)
                {
                    mvprintw(door_2.y, door_2.x, "╣");
                }
                else
                {
                    mvprintw(door_2.y, door_2.x, "░");
                }
                attroff(COLOR_PAIR(6));
                break;
            case (6):
                door_1 = level[level_num].room[4].door[2];
                door_2 = level[level_num].room[5].door[1];
                attron(COLOR_PAIR(6));
                if (level[level_num].room[4].room_exist == 1)
                {
                    mvprintw(door_1.y, door_1.x, "╠");
                }
                else
                {
                    mvprintw(door_1.y, door_1.x, "░");
                }

                if (level[level_num].room[5].room_exist == 1)
                {
                    mvprintw(door_2.y, door_2.x, "╣");
                }
                else
                {
                    mvprintw(door_2.y, door_2.x, "░");
                }
                attroff(COLOR_PAIR(6));
                break;
            case (7):
                door_1 = level[level_num].room[3].door[2];
                door_2 = level[level_num].room[6].door[0];
                attron(COLOR_PAIR(6));
                if (level[level_num].room[3].room_exist == 1)
                {
                    mvprintw(door_1.y, door_1.x, "╦");
                    mvprintw(door_1.y, door_1.x + 1, "╦");
                }
                else
                {
                    mvprintw(door_1.y, door_1.x, "░");
                }

                if (level[level_num].room[6].room_exist == 1)
                {
                    mvprintw(door_2.y, door_2.x, "╩");
                    mvprintw(door_2.y, door_2.x + 1, "╩");
                }
                else
                {
                    mvprintw(door_2.y, door_2.x, "░");
                }
                attroff(COLOR_PAIR(6));
                break;
            case (8):
                door_1 = level[level_num].room[4].door[3];
                door_2 = level[level_num].room[7].door[0];
                attron(COLOR_PAIR(6));
                if (level[level_num].room[4].room_exist == 1)
                {
                    mvprintw(door_1.y, door_1.x, "╦");
                    mvprintw(door_1.y, door_1.x + 1, "╦");
                }
                else
                {
                    mvprintw(door_1.y, door_1.x, "░");
                }

                if (level[level_num].room[7].room_exist == 1)
                {
                    mvprintw(door_2.y, door_2.x, "╩");
                    mvprintw(door_2.y, door_2.x + 1, "╩");
                }
                else
                {
                    mvprintw(door_2.y, door_2.x, "░");
                }
                attroff(COLOR_PAIR(6));
                break;
            case (9):
                door_1 = level[level_num].room[5].door[2];
                door_2 = level[level_num].room[8].door[0];
                attron(COLOR_PAIR(6));
                if (level[level_num].room[5].room_exist == 1)
                {
                    mvprintw(door_1.y, door_1.x, "╦");
                    mvprintw(door_1.y, door_1.x + 1, "╦");
                }
                else
                {
                    mvprintw(door_1.y, door_1.x, "░");
                }

                if (level[level_num].room[8].room_exist == 1)
                {
                    mvprintw(door_2.y, door_2.x, "╩");
                    mvprintw(door_2.y, door_2.x + 1, "╩");
                }
                else
                {
                    mvprintw(door_2.y, door_2.x, "░");
                }
                attroff(COLOR_PAIR(6));
                break;
            case (10):
                door_1 = level[level_num].room[6].door[1];
                door_2 = level[level_num].room[7].door[1];
                attron(COLOR_PAIR(6));
                if (level[level_num].room[6].room_exist == 1)
                {
                    mvprintw(door_1.y, door_1.x, "╠");
                }
                else
                {
                    mvprintw(door_1.y, door_1.x, "░");
                }

                if (level[level_num].room[7].room_exist == 1)
                {
                    mvprintw(door_2.y, door_2.x, "╣");
                }
                else
                {
                    mvprintw(door_2.y, door_2.x, "░");
                }
                attroff(COLOR_PAIR(6));
                break;
            case (11):
                door_1 = level[level_num].room[7].door[2];
                door_2 = level[level_num].room[8].door[1];
                attron(COLOR_PAIR(6));
                if (level[level_num].room[7].room_exist == 1)
                {
                    mvprintw(door_1.y, door_1.x, "╠");
                }
                else
                {
                    mvprintw(door_1.y, door_1.x, "░");
                }

                if (level[level_num].room[8].room_exist == 1)
                {
                    mvprintw(door_2.y, door_2.x, "╣");
                }
                else
                {
                    mvprintw(door_2.y, door_2.x, "░");
                }
                attroff(COLOR_PAIR(6));
                break;
            }

            if (i == 0 || i == 1 || i == 5 || i == 6 || i == 10 || i == 11)
            {
                for (int j = door_1.x + 1; j <= mid_1.x + 1; j++)
                {
                    attron(COLOR_PAIR(6));
                    mvprintw(door_1.y, j, "░");
                    attroff(COLOR_PAIR(6));
                }

                if (mid_1.y < mid_2.y)
                {
                    for (int j = mid_1.y; j <= mid_2.y; j++)
                    {
                        attron(COLOR_PAIR(6));
                        mvprintw(j, mid_1.x, "░");
                        attroff(COLOR_PAIR(6));
                    }
                    for (int j = mid_1.y; j <= mid_2.y; j++)
                    {
                        attron(COLOR_PAIR(6));
                        mvprintw(j, mid_1.x + 1, "░");
                        attroff(COLOR_PAIR(6));
                    }
                }
                else
                {
                    for (int j = mid_2.y; j <= mid_1.y; j++)
                    {
                        attron(COLOR_PAIR(6));
                        mvprintw(j, mid_2.x, "░");
                        attroff(COLOR_PAIR(6));
                    }
                    for (int j = mid_2.y; j <= mid_1.y; j++)
                    {
                        attron(COLOR_PAIR(6));
                        mvprintw(j, mid_2.x + 1, "░");
                        attroff(COLOR_PAIR(6));
                    }
                }

                for (int j = mid_2.x; j < door_2.x; j++)
                {
                    attron(COLOR_PAIR(6));
                    mvprintw(mid_2.y, j, "░");
                    attroff(COLOR_PAIR(6));
                }
            }

            else
            {
                for (int j = door_1.y + 1; j <= mid_1.y; j++)
                {
                    attron(COLOR_PAIR(6));
                    mvprintw(j, door_1.x, "░");
                    attroff(COLOR_PAIR(6));
                }

                for (int j = door_1.y + 1; j <= mid_1.y; j++)
                {
                    attron(COLOR_PAIR(6));
                    mvprintw(j, door_1.x + 1, "░");
                    attroff(COLOR_PAIR(6));
                }

                if (mid_1.x < mid_2.x)
                {
                    for (int j = mid_1.x; j <= mid_2.x; j++)
                    {
                        attron(COLOR_PAIR(6));
                        mvprintw(mid_1.y, j, "░");
                        attroff(COLOR_PAIR(6));
                    }
                }
                else
                {
                    for (int j = mid_2.x; j <= mid_1.x; j++)
                    {
                        attron(COLOR_PAIR(6));
                        mvprintw(mid_2.y, j, "░");
                        attroff(COLOR_PAIR(6));
                    }
                }

                for (int j = mid_2.y; j < door_2.y; j++)
                {
                    attron(COLOR_PAIR(6));
                    mvprintw(j, mid_2.x, "░");
                    attroff(COLOR_PAIR(6));
                }
                for (int j = mid_2.y; j < door_2.y; j++)
                {
                    attron(COLOR_PAIR(6));
                    mvprintw(j, mid_2.x + 1, "░");
                    attroff(COLOR_PAIR(6));
                }
            }
        }
    }
}

void room_to_file(FILE *user_map, int level_num, int room_num)
{
    fprintf(user_map, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,", level_num, room_num,
            level[level_num].room[room_num].room_exist,
            level[level_num].room[room_num].discovered,
            level[level_num].room[room_num].corner.y,
            level[level_num].room[room_num].corner.x,
            level[level_num].room[room_num].width, level[level_num].room[room_num].length,
            level[level_num].room[room_num].middle_door.y,
            level[level_num].room[room_num].middle_door.x,
            level[level_num].room[room_num].room_position,
            level[level_num].room[room_num].door_num);
    for (int i = 0; i < level[level_num].room[room_num].door_num; i++)
    {
        fprintf(user_map, "%d,", level[level_num].room[room_num].door_positions[i]);
        fprintf(user_map, "%d,%d,", level[level_num].room[room_num].door[i].y, level[level_num].room[room_num].door[i].x);
    }
    fprintf(user_map, "\n");
}

void point_to_file(FILE *user_map, Point point)
{
    fprintf(user_map, "%d,%d,", point.y, point.x);
}

void level_to_file(char *file_name, int level_num)
{
    FILE *user_map = fopen(file_name, "a");

    if (user_map == NULL)
    {
        perror("Error opening file");
    }
    else
    {
        fprintf(user_map, "%d,%d,%d,%d\n", level_num, level[level_num].room_num, level[level_num].discovered_room_num, level[level_num].object_num);
        for (int i = 0; i < 9; i++)
            room_to_file(user_map, level_num, i);
        for (int i = 0; i < 12; i++)
            point_to_file(user_map, level[level_num].between_doors_1[i]);
        fprintf(user_map, "\n");
        for (int i = 0; i < 12; i++)
            point_to_file(user_map, level[level_num].between_doors_2[i]);
        fprintf(user_map, "\n");
        for (int i = 0; i < 12; i++)
            fprintf(user_map, "%d,", level[level_num].corridor_exist[i]);
        fprintf(user_map, "\n");
        for (int i = 0; i < level[level_num].object_num; i++)
        {
            fprintf(user_map, "%d,%d,%d,%d,%d,",
                    level[level_num].objects[i].type,
                    level[level_num].objects[i].location.y,
                    level[level_num].objects[i].location.x,
                    level[level_num].objects[i].visible,
                    level[level_num].objects[i].location_room);
        }
        fprintf(user_map, "\n");
        fclose(user_map);
    }
}

void load_level_from_file(char *file_name, int level_num)
{
    FILE *user_map = fopen(file_name, "r");

    if (user_map == NULL)
    {
        perror("Error opening file");
        return;
    }

    char file_line[1024];
    int file_level_num;
    int room_num, discovered_room_num, object_num, room_exist, discovered, corner_y, corner_x, width, length, middle_door_y, middle_door_x, room_position, door_num;
    int door_positions[10], door_y[10], door_x[10];
    Point between_doors_1[12], between_doors_2[12];
    int corridor_exist[12];
    Object *file_objects;

    int level_count = find_level_num_from_file(file_name, level_num);
    for (int i = 0; i < (14 * level_count); i++)
        fgets(file_line, sizeof(file_line), user_map);

    // Search for the specific level in the file
    fgets(file_line, sizeof(file_line), user_map);
    sscanf(file_line, "%d,%d,%d,%d\n", &file_level_num, &room_num, &discovered_room_num, &object_num);

    level[level_num].room_num = room_num;
    level[level_num].discovered_room_num = discovered_room_num;
    level[level_num].object_num = object_num;

    // Read rooms
    for (int i = 0; i < 9; i++)
    {
        fgets(file_line, sizeof(file_line), user_map);
        sscanf(file_line, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,", &file_level_num, &i, &room_exist, &discovered, &corner_y, &corner_x,
               &width, &length, &middle_door_y, &middle_door_x, &room_position, &door_num);
        level[level_num].room[i].room_exist = room_exist;
        level[level_num].room[i].discovered = discovered;
        level[level_num].room[i].corner.y = corner_y;
        level[level_num].room[i].corner.x = corner_x;
        level[level_num].room[i].width = width;
        level[level_num].room[i].length = length;
        level[level_num].room[i].middle_door.y = middle_door_y;
        level[level_num].room[i].middle_door.x = middle_door_x;
        level[level_num].room[i].room_position = room_position;
        level[level_num].room[i].door_num = door_num;

        char *token = strtok(file_line, ",");
        for (int j = 0; j < 12; j++) // Skip the first 12 tokens (already parsed)
        {
            token = strtok(NULL, ",");
        }
        for (int j = 0; j < door_num; j++)
        {
            door_positions[j] = atoi(token);
            token = strtok(NULL, ",");
            door_y[j] = atoi(token);
            token = strtok(NULL, ",");
            door_x[j] = atoi(token);
            token = strtok(NULL, ",");

            level[level_num].room[i].door_positions[j] = door_positions[j];
            level[level_num].room[i].door[j].y = door_y[j];
            level[level_num].room[i].door[j].x = door_x[j];
        }
    }

    fgets(file_line, sizeof(file_line), user_map);
    sscanf(file_line, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,",
           &between_doors_1[0].y, &between_doors_1[0].x,
           &between_doors_1[1].y, &between_doors_1[1].x,
           &between_doors_1[2].y, &between_doors_1[2].x,
           &between_doors_1[3].y, &between_doors_1[3].x,
           &between_doors_1[4].y, &between_doors_1[4].x,
           &between_doors_1[5].y, &between_doors_1[5].x,
           &between_doors_1[6].y, &between_doors_1[6].x,
           &between_doors_1[7].y, &between_doors_1[7].x,
           &between_doors_1[8].y, &between_doors_1[8].x,
           &between_doors_1[9].y, &between_doors_1[9].x,
           &between_doors_1[10].y, &between_doors_1[10].x,
           &between_doors_1[11].y, &between_doors_1[11].x);
    for (int i = 0; i < 12; i++)
    {
        level[level_num].between_doors_1[i].y = between_doors_1[i].y;
        level[level_num].between_doors_1[i].x = between_doors_1[i].x;
    }

    fgets(file_line, sizeof(file_line), user_map);
    sscanf(file_line, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,",
           &between_doors_2[0].y, &between_doors_2[0].x,
           &between_doors_2[1].y, &between_doors_2[1].x,
           &between_doors_2[2].y, &between_doors_2[2].x,
           &between_doors_2[3].y, &between_doors_2[3].x,
           &between_doors_2[4].y, &between_doors_2[4].x,
           &between_doors_2[5].y, &between_doors_2[5].x,
           &between_doors_2[6].y, &between_doors_2[6].x,
           &between_doors_2[7].y, &between_doors_2[7].x,
           &between_doors_2[8].y, &between_doors_2[8].x,
           &between_doors_2[9].y, &between_doors_2[9].x,
           &between_doors_2[10].y, &between_doors_2[10].x,
           &between_doors_2[11].y, &between_doors_2[11].x);
    for (int i = 0; i < 12; i++)
    {
        level[level_num].between_doors_2[i].y = between_doors_2[i].y;
        level[level_num].between_doors_2[i].x = between_doors_2[i].x;
    }

    fgets(file_line, sizeof(file_line), user_map);
    sscanf(file_line, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,",
           &corridor_exist[0], &corridor_exist[1],
           &corridor_exist[2], &corridor_exist[3],
           &corridor_exist[4], &corridor_exist[5],
           &corridor_exist[6], &corridor_exist[7],
           &corridor_exist[8], &corridor_exist[9],
           &corridor_exist[10], &corridor_exist[11]);
    for (int i = 0; i < 12; i++)
    {
        level[level_num].corridor_exist[i] = corridor_exist[i];
    }

    file_objects = (Object *)malloc(object_num * sizeof(Object));
    level[level_num].objects = (Object *)malloc(object_num * sizeof(Object));
    fgets(file_line, sizeof(file_line), user_map);
    char *token = strtok(file_line, ",");
    for (int i = 0; i < object_num; i++)
    {
        if (token != NULL)
            level[level_num].objects[i].type = (ObjectType)atoi(token);
        token = strtok(NULL, ",");
        if (token != NULL)
            level[level_num].objects[i].location.y = atoi(token);
        token = strtok(NULL, ",");
        if (token != NULL)
            level[level_num].objects[i].location.x = atoi(token);
        token = strtok(NULL, ",");
        if (token != NULL)
            level[level_num].objects[i].visible = atoi(token);
        token = strtok(NULL, ",");
        if (token != NULL)
            level[level_num].objects[i].location_room = atoi(token);
        token = strtok(NULL, ",");
    }

    fclose(user_map);
}

int find_level_num_from_file(char *file_name, int level_num)
{
    int target_level_count = 0;
    int total_level_count = 0;

    FILE *user_map = fopen(file_name, "r");

    if (user_map == NULL)
    {
        perror("Error opening file");
        return -1;
    }

    char file_line[1024];
    int file_level_num;

    while (fgets(file_line, sizeof(file_line), user_map) != NULL)
    {
        sscanf(file_line, "%d,", &file_level_num);
        if (file_level_num == level_num)
        {
            target_level_count = total_level_count;
        }
        total_level_count += 1;
        for (int i = 0; i < 13; i++)
            fgets(file_line, sizeof(file_line), user_map);
    }

    fclose(user_map);
    return target_level_count;
}

int find_user_index(char username[26])
{
    FILE *users = fopen("users.csv", "r");

    if (users == NULL)
    {
        perror("Error opening file");
    }

    char file_line[1024];
    char file_username[256];

    int index = 0;

    if (fgets(file_line, sizeof(file_line), users) != NULL)
    {
        sscanf(file_line, "%255[^,]", file_username);
        if (strcmp(file_username, username) == 0)
        {
            fclose(users);
            return index;
        }
        index += 1;
    }

    while (fgets(file_line, sizeof(file_line), users))
    {
        sscanf(file_line, "%255[^,]", file_username);
        if (strcmp(file_username, username) == 0)
        {
            fclose(users);
            return index;
        }
        index += 1;
    }

    fclose(users);
    return (-1);
}

void title_screen()
{
    char game_start[] = "- Press any key to continue -";
    int sleep_time = 30000;

    for (int i = 0; i < 33; i++)
    {
        mvprintw((LINES / 2) - 22 + i, (COLS / 2) - 61, "%s", game_title_design_5[i]);
        refresh();
        usleep(sleep_time);
    }

    usleep(700000);

    move((LINES / 2) + 19, ((COLS - strlen(game_start)) / 2) - 1);
    printw("%s", game_start);
    refresh();

    getch();
    refresh();
}

// Welcome Screen - Manu
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

// Create New Account - Menu
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
    User *temp = malloc(sizeof(User));
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
            strcpy(temp->username, username);
            strcpy(temp->email, email);
            strcpy(temp->password, password);
            temp->difficulty = 20;
            temp->color_option = 1;
            temp->game_num = 0;
            temp->win_num = 0;
            temp->total_gold = 0;
            temp->total_score = 0;

            save_user_to_csv("users.csv", temp);
            break;
        }

        else if (choice == 3 && (key == 10 || key == 32))
            show_password = !show_password;

        else if (choice == 5 && (key == 10 || key == 32))
            strcpy(password, generate_password());

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

// Create New Account - Check Errors
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

// Create New Account - Print Errors
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

// Sign In / Continue Previous Game - Menu
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

void load_users()
{
    if (game.users != NULL)
    {
        // Free existing users if already loaded
        for (int i = 0; i < game.users_num; i++)
        {
            free(game.users[i]);
        }
        free(game.users);
    }

    // Load users from CSV
    if (load_users_from_csv("users.csv", &game.users, &game.users_num) != 0)
    {
        fprintf(stderr, "Error: Failed to load users from CSV!\n");
        exit(1);
    }
}

int username_found(const char *username)
{
    for (int i = 0; i < game.users_num; i++)
    {
        if (strcmp(game.users[i]->username, username) == 0)
        {
            return i; // Return the index of the user
        }
    }
    return -1; // User not found
}

// Function to find a user by email
int email_found(const char *email)
{
    for (int i = 0; i < game.users_num; i++)
    {
        if (strcmp(game.users[i]->email, email) == 0)
        {
            return i; // Return the index of the user
        }
    }
    return -1; // Email not found
}

// Function to check if the password is correct
bool password_correct(int user_index, const char *password)
{
    if (user_index < 0 || user_index >= game.users_num)
    {
        return false; // Invalid index
    }
    return strcmp(game.users[user_index]->password, password) == 0;
}

// Function to get user data by username
void data_from_username(const char *username, int *difficulty, int *color_option)
{
    int index = username_found(username);
    if (index != -1)
    {
        *difficulty = game.users[index]->difficulty;
        *color_option = game.users[index]->color_option;
    }
}

// Function to get user data by email
void data_from_email(const char *email, int *difficulty, int *color_option)
{
    int index = email_found(email);
    if (index != -1)
    {
        *difficulty = game.users[index]->difficulty;
        *color_option = game.users[index]->color_option;
    }
}

int find_user_index_from_username(const char *username)
{
    load_users_from_csv("users.csv", &game.users, &game.users_num);
    for (int i = 0; i < game.users_num; i++)
    {
        if (strcmp(game.users[i]->username, username) == 0)
            return i;
    }
    return -1;
}

int find_user_index_from_email(const char *email)
{
    load_users_from_csv("users.csv", &game.users, &game.users_num);
    for (int i = 0; i < game.users_num; i++)
    {
        if (strcmp(game.users[i]->email, email) == 0)
            return i;
    }
    return -1;
}

// Updated continue_game_screen function
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
        {
            curs_set(0);
        }

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
            user_index = find_user_index_from_username(username);
        else if (option == 1)
            user_index = find_user_index_from_email(email);

        if (error == 0 && choice == 3 && (key == 10 || key == 32))
        {
            User *user = game.users[user_index];

            strcpy(current_user.username, user->username);
            strcpy(current_user.email, user->email);
            strcpy(current_user.password, password);

            current_user.difficulty = user->difficulty;
            current_user.color_option = user->color_option;
            current_user.game_num = user->game_num;
            current_user.win_num = user->win_num;
            current_user.total_score = user->total_score;
            current_user.total_gold = user->total_gold;
            current_user.play_music = user->play_music;
            current_user.playlist = user->playlist;
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

// Sign In - Print Errors
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

// User Options Menu
void draw_user_options_menu()
{
    clear();
    draw_border();
    mvprintw((LINES / 2) - 8, (COLS / 2) - 18, "__ User Options ____________________");
    mvprintw((LINES / 2) - 6, (COLS / 2) - 11, "New Game");
    mvprintw((LINES / 2) - 4, (COLS / 2) - 11, "Continue Previous Game");
    mvprintw((LINES / 2) - 2, (COLS / 2) - 11, "View Score Board");
    mvprintw((LINES / 2), (COLS / 2) - 11, "My Profile");
    mvprintw((LINES / 2), (COLS / 2) + 3, "Settings");
    mvprintw((LINES / 2) + 2, (COLS / 2) - 18, "‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾");
    for (int i = 0; i < 9; i++)
    {
        mvprintw((LINES / 2) - 7 + i, (COLS / 2) - 18, "|");
        mvprintw((LINES / 2) - 7 + i, (COLS / 2) + 17, "|");
    }
}

int user_options_menu()
{
    int choice = 0;
    while (1)
    {
        draw_user_options_menu();

        if (choice < 4)
            mvprintw((LINES / 2) - 6 + (2 * choice), (COLS / 2) - 13, ">");
        else if (choice == 4)
            mvprintw((LINES / 2), (COLS / 2) + 1, ">");

        int key = getch();

        if (key == 10 || key == 32)
            return choice;

        else if (choice != 3 && key == KEY_DOWN)
            choice = (choice + 1) % 5;

        else if (choice == 3 && key == KEY_DOWN)
            choice = 0;

        else if (choice != 4 && choice != 0 && key == KEY_UP)
            choice = (choice + 4) % 5;

        else if (choice == 4 && key == KEY_UP)
            choice = 2;

        else if (choice == 0 && key == KEY_UP)
            choice = 3;

        else if ((choice == 3 || choice == 4) && (key == KEY_LEFT || key == KEY_RIGHT))
            choice = ((choice) % 2) + 3;
    }
}

// User Settings Menu
void draw_user_settings_menu()
{
    clear();
    draw_border();
    mvprintw((LINES / 2) - 8, (COLS / 2) - 28, "__ Settings ___________________________________________");
    mvprintw((LINES / 2) - 6, (COLS / 2) - 21, "Difficulty   [                    ]  (    %%)");
    mvprintw((LINES / 2) - 4, (COLS / 2) - 21, "Hero Face    Face:                   (    )");
    mvprintw((LINES / 2) - 2, (COLS / 2) - 21, "Play Music   -     -");
    mvprintw((LINES / 2), (COLS / 2) - 21, "Music Theme  Play List:");
    mvprintw((LINES / 2) + 2, (COLS / 2) - 4, "Confirm");
    mvprintw((LINES / 2) + 4, (COLS / 2) - 28, "‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾");

    for (int i = 0; i < 11; i++)
    {
        mvprintw((LINES / 2) - 7 + i, (COLS / 2) - 28, "|");
        mvprintw((LINES / 2) - 7 + i, (COLS / 2) + 26, "|");
    }

    mvprintw((LINES / 2) + 6, (COLS / 2) - 28, "__ Hint _______________________________________________");
    mvprintw((LINES / 2) + 7, (COLS / 2) - 28, "|                                                     |");
    mvprintw((LINES / 2) + 8, (COLS / 2) - 28, "|      Use left and right arrows to change your       |");
    mvprintw((LINES / 2) + 9, (COLS / 2) - 28, "|     game's difficulty or your chracter's color.     |");
    mvprintw((LINES / 2) + 10, (COLS / 2) - 28, "|                                                     |");
    mvprintw((LINES / 2) + 11, (COLS / 2) - 28, "‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾");
}

void user_settings_menu()
{
    int choice = 0;

    while (1)
    {
        clear();
        draw_user_settings_menu();

        if (choice == 0)
            mvprintw((LINES / 2) - 6, (COLS / 2) - 23, ">");
        else if (choice == 1)
            mvprintw((LINES / 2) - 4, (COLS / 2) - 23, ">");
        else if (choice == 2)
            mvprintw((LINES / 2) - 2, (COLS / 2) - 23, ">");
        else if (choice == 3)
            mvprintw((LINES / 2), (COLS / 2) - 23, ">");
        else if (choice == 4)
        {
            attron(A_REVERSE);
            mvprintw((LINES / 2) + 2, (COLS / 2) - 4, "Confirm");
            attroff(A_REVERSE);
        }

        move((LINES / 2) - 6, (COLS / 2) - 7);
        for (int i = 0; i < current_user.difficulty; i++)
            printw("#");
        for (int i = 0; i < 20 - current_user.difficulty; i++)
            printw("·");

        mvprintw((LINES / 2) - 4, (COLS / 2) - 2, "%s", face_name[current_user.color_option]);
        mvprintw((LINES / 2) - 4, (COLS / 2) + 18, "%s", face[current_user.color_option]);
        move((LINES / 2) - 2, (COLS / 2) - 6);
        if (current_user.play_music == 1)
            printw("Yes");
        if (current_user.play_music == 0)
            printw("No");
        mvprintw((LINES / 2), (COLS / 2) + 3, "%s", playlist_name[current_user.playlist]);

        if (current_user.difficulty < 2)
            mvprintw((LINES / 2) - 6, (COLS / 2) + 19, "%d", (5 * current_user.difficulty));
        else if (current_user.difficulty < 20)
            mvprintw((LINES / 2) - 6, (COLS / 2) + 18, "%d", (5 * current_user.difficulty));
        else
            mvprintw((LINES / 2) - 6, (COLS / 2) + 17, "%d", (5 * current_user.difficulty));

        int key = getch();

        if (choice == 4 && (key == 10 || key == 32))
            break;

        if (key == KEY_DOWN)
            choice = (choice + 1) % 5;
        else if (key == KEY_UP)
            choice = (choice + 4) % 5;
        else if (choice == 0 && key == KEY_LEFT)
        {
            if (current_user.difficulty > 0)
                current_user.difficulty -= 1;
        }
        else if (choice == 0 && key == KEY_RIGHT)
        {
            if (current_user.difficulty < 20)
                current_user.difficulty += 1;
        }
        else if (choice == 1 && key == KEY_LEFT)
        {
            current_user.color_option = (current_user.color_option + 1) % 10;
        }
        else if (choice == 1 && key == KEY_RIGHT)
        {
            current_user.color_option = (current_user.color_option + 9) % 10;
        }
        else if (choice == 2 && (key == KEY_RIGHT || key == KEY_LEFT))
            current_user.play_music = !current_user.play_music;
        else if (choice == 3 && key == KEY_RIGHT)
            current_user.playlist = (current_user.playlist + 1) % 3;
        else if (choice == 3 && key == KEY_LEFT)
            current_user.playlist = (current_user.playlist + 2) % 3;
    }
}

int run_game_level(int i)
{
    // load_level_from_file("test.csv", i);
    initialize_hero(i);

    while (1)
    {
        if (hero.health <= 0)
            return -2;

        get_input = 1;
        int location_result = -1;
        object_index = -1;
        enemy_index = -1;
        int attacked = 0;

        clear();
        make_cells_visible(i);
        make_enemies_visible(i);
        location_result = handle_location(i);
        print_level(i);
        if (i != MAX_LEVEL - 1)
            print_corridors(i);
        print_objects(i);
        print_enemies(i);
        print_hero();
        print_info(i);

        if (find_room(i, hero.location) != hero.last_room && find_room(i, hero.location) != -1)
        {
            if (level[i].room[find_room(i, hero.location)].type == Nightmare)
            {
                print_message("Welcome to the Nightmare Room!", "");
                if (rand() % 3 == 0)
                    golden_freddy_appear();
            }
        }
        hero.last_room = find_room(i, hero.location);

        int input;

        if (location_result == 1) // Staircase
        {
            if (level[i].discovered_room_num == level[i].room_num)
            {
                print_message("You found the staircase!", "You can use Right and Left arrows to go to other levels.");
                input = getch();

                if (input == KEY_RIGHT)
                {
                    move(LINES - 5, 0);
                    clrtoeol();
                    refresh();
                    return i + 1;
                }

                if (input == KEY_LEFT)
                {
                    move(LINES - 5, 0);
                    clrtoeol();
                    refresh();
                    return i - 1;
                }

                else
                    get_input = 0;
            }
            else
            {
                print_message("You haven't found all the rooms yet!", "");
            }
        }

        if (location_result == 2) // Poison Food
        {
            print_message("This is food.", "Press 'P' to pick it up.");
            input = getch();
            if (input == 'p' || input == 'P')
            {
                move(LINES - 5, 0);
                clrtoeol();
                refresh();
                print_message("You can press '1' to see your Food Inventory.", "");
                input = getch();
                get_input = 0;
                if (level[i].room[hero.last_room].type != Nightmare)
                    hero.food_inventory[hero.food_num] = level[i].objects[object_index];
                remove_object(i, object_index);
                move(LINES - 5, 0);
                clrtoeol();
                refresh();
                if (level[i].room[hero.last_room].type != Nightmare)
                    hero.food_num += 1;
            }
            else
            {
                move(LINES - 5, 0);
                clrtoeol();
                refresh();
                get_input = 0;
            }
        }

        else if (location_result == 3) // Coin
        {
            char message[256];
            snprintf(message, sizeof(message), "You just collected %d Points!", added_coin);
            print_message(message, "");
            input = getch();
            get_input = 0;
            move(LINES - 5, 0);
            clrtoeol();
            refresh();
        }

        else if (location_result == 12) // Black Coin
        {
            char message[256];
            snprintf(message, sizeof(message), "This is a Diamond! You just collected %d Points!", added_coin);
            print_message(message, "");
            input = getch();
            get_input = 0;
            move(LINES - 5, 0);
            clrtoeol();
            refresh();
        }

        else if (location_result == 5) // Normal Food
        {
            print_message("This is food.", "Press 'P' to pick it up.");
            input = getch();
            if (input == 'p' || input == 'P')
            {
                move(LINES - 5, 0);
                clrtoeol();
                refresh();
                print_message("You can press '1' to see your Food Inventory.", "");
                input = getch();
                get_input = 0;
                if (level[i].room[hero.last_room].type != Nightmare)
                    hero.food_inventory[hero.food_num] = level[i].objects[object_index];
                if (level[i].room[hero.last_room].type != Nightmare)
                    hero.food_inventory[hero.food_num].food_step_count = 0;
                remove_object(i, object_index);
                move(LINES - 5, 0);
                clrtoeol();
                refresh();
                if (level[i].room[hero.last_room].type != Nightmare)
                    hero.food_num += 1;
            }
            else
            {
                move(LINES - 5, 0);
                clrtoeol();
                refresh();
                get_input = 0;
            }
        }

        else if (location_result == 6) // Gourmet Food
        {
            print_message("This is gourmet food.", "Press 'P' to pick it up.");
            input = getch();
            if (input == 'p' || input == 'P')
            {
                move(LINES - 5, 0);
                clrtoeol();
                refresh();
                print_message("You can press '1' to see your Food Inventory.", "");
                input = getch();
                get_input = 0;
                if (level[i].room[hero.last_room].type != Nightmare)
                    hero.food_inventory[hero.food_num] = level[i].objects[object_index];
                if (level[i].room[hero.last_room].type != Nightmare)
                    hero.food_inventory[hero.food_num].food_step_count = 0;
                remove_object(i, object_index);
                move(LINES - 5, 0);
                clrtoeol();
                refresh();
                if (level[i].room[hero.last_room].type != Nightmare)
                    hero.food_num += 1;
            }
            else
            {
                move(LINES - 5, 0);
                clrtoeol();
                refresh();
                get_input = 0;
            }
        }

        else if (location_result == 7) // Magic Food
        {
            print_message("This is magic food.", "Press 'P' to pick it up.");
            input = getch();
            if (input == 'p' || input == 'P')
            {
                move(LINES - 5, 0);
                clrtoeol();
                refresh();
                print_message("You can press '1' to see your Food Inventory.", "");
                input = getch();
                get_input = 0;
                if (level[i].room[hero.last_room].type != Nightmare)
                    hero.food_inventory[hero.food_num] = level[i].objects[object_index];
                remove_object(i, object_index);
                move(LINES - 5, 0);
                clrtoeol();
                refresh();
                if (level[i].room[hero.last_room].type != Nightmare)
                    hero.food_num += 1;
            }
            else
            {
                move(LINES - 5, 0);
                clrtoeol();
                refresh();
                get_input = 0;
            }
        }

        else if (location_result == 8 && level[i].room[hero.last_room].type != Nightmare) // Dagger
        {
            print_message("This is a Dagger.", "Press 'P' to pick it up.");
            input = getch();
            if (input == 'p' || input == 'P')
            {
                move(LINES - 5, 0);
                clrtoeol();
                refresh();
                print_message("You can press '2' to see your Weapon Inventory.", "");
                input = getch();
                get_input = 0;
                hero.weapon_inventory[hero.weapon_num] = level[i].objects[object_index];
                remove_object(i, object_index);
                move(LINES - 5, 0);
                clrtoeol();
                refresh();
                hero.weapon_num += 1;
            }
            else
            {
                move(LINES - 5, 0);
                clrtoeol();
                refresh();
                get_input = 0;
            }
        }

        else if (location_result == 9 && level[i].room[hero.last_room].type != Nightmare) // Magic Wand
        {
            print_message("This is a Magic Wand.", "Press 'P' to pick it up.");
            input = getch();
            if (input == 'p' || input == 'P')
            {
                move(LINES - 5, 0);
                clrtoeol();
                refresh();
                print_message("You can press '2' to see your Weapon Inventory.", "");
                input = getch();
                get_input = 0;
                hero.weapon_inventory[hero.weapon_num] = level[i].objects[object_index];
                remove_object(i, object_index);
                move(LINES - 5, 0);
                clrtoeol();
                refresh();
                hero.weapon_num += 1;
            }
            else
            {
                move(LINES - 5, 0);
                clrtoeol();
                refresh();
                get_input = 0;
            }
        }

        else if (location_result == 10 && level[i].room[hero.last_room].type != Nightmare) // Normal Arrow
        {
            print_message("This is a Normal Arrow.", "Press 'P' to pick it up.");
            input = getch();
            if (input == 'p' || input == 'P')
            {
                move(LINES - 5, 0);
                clrtoeol();
                refresh();
                print_message("You can press '2' to see your Weapon Inventory.", "");
                input = getch();
                get_input = 0;
                hero.weapon_inventory[hero.weapon_num] = level[i].objects[object_index];
                remove_object(i, object_index);
                move(LINES - 5, 0);
                clrtoeol();
                refresh();
                hero.weapon_num += 1;
            }
            else
            {
                move(LINES - 5, 0);
                clrtoeol();
                refresh();
                get_input = 0;
            }
        }

        else if (location_result == 11 && level[i].room[hero.last_room].type != Nightmare) // Sword
        {
            print_message("This is a gun.", "Press 'P' to pick it up.");
            input = getch();
            if (input == 'p' || input == 'P')
            {
                move(LINES - 5, 0);
                clrtoeol();
                refresh();
                print_message("You can press '2' to see your Weapon Inventory.", "");
                input = getch();
                get_input = 0;
                hero.weapon_inventory[hero.weapon_num] = level[i].objects[object_index];
                remove_object(i, object_index);
                move(LINES - 5, 0);
                clrtoeol();
                refresh();
                hero.weapon_num += 1;
            }
            else
            {
                move(LINES - 5, 0);
                clrtoeol();
                refresh();
                get_input = 0;
            }
        }

        else if (location_result == 13 && level[i].room[hero.last_room].type != Nightmare) // Health Spell
        {
            print_message("This is a Health Spell.", "Press 'P' to pick it up.");
            input = getch();
            if (input == 'p' || input == 'P')
            {
                move(LINES - 5, 0);
                clrtoeol();
                refresh();
                print_message("You can press '3' to see your Spell Inventory.", "");
                input = getch();
                get_input = 0;
                hero.spell_inventory[hero.spell_num] = level[i].objects[object_index];
                remove_object(i, object_index);
                move(LINES - 5, 0);
                clrtoeol();
                refresh();
                hero.spell_num += 1;
            }
            else
            {
                move(LINES - 5, 0);
                clrtoeol();
                refresh();
                get_input = 0;
            }
        }

        else if (location_result == 14 && level[i].room[hero.last_room].type != Nightmare) // Speed Spell
        {
            print_message("This is a Speed Spell.", "Press 'P' to pick it up.");
            input = getch();
            if (input == 'p' || input == 'P')
            {
                move(LINES - 5, 0);
                clrtoeol();
                refresh();
                print_message("You can press '3' to see your Spell Inventory.", "");
                input = getch();
                get_input = 0;
                hero.spell_inventory[hero.spell_num] = level[i].objects[object_index];
                remove_object(i, object_index);
                move(LINES - 5, 0);
                clrtoeol();
                refresh();
                hero.spell_num += 1;
            }
            else
            {
                move(LINES - 5, 0);
                clrtoeol();
                refresh();
                get_input = 0;
            }
        }

        else if (location_result == 15 && level[i].room[hero.last_room].type != Nightmare) // Damage Spell
        {
            print_message("This is a Damage Spell.", "Press 'P' to pick it up.");
            input = getch();
            if (input == 'p' || input == 'P')
            {
                move(LINES - 5, 0);
                clrtoeol();
                refresh();
                print_message("You can press '3' to see your Spell Inventory.", "");
                input = getch();
                get_input = 0;
                hero.spell_inventory[hero.spell_num] = level[i].objects[object_index];
                remove_object(i, object_index);
                move(LINES - 5, 0);
                clrtoeol();
                refresh();
                hero.spell_num += 1;
            }
            else
            {
                move(LINES - 5, 0);
                clrtoeol();
                refresh();
                get_input = 0;
            }
        }

        if (input == 'k' || input == 'K')
        {
            switch (hero.current_weapon.type)
            {
            case Mace:
                input = attack_with_mace(i);
                get_input = 0;
                attacked = 1;
                break;
            case Dagger:
                input = attack_with_dagger(i, hero.dir_y, hero.dir_x);
                get_input = 0;
                attacked = 1;
                break;
            case MagicWand:
                input = attack_with_magic_wand(i, hero.dir_y, hero.dir_x);
                get_input = 0;
                attacked = 1;
                break;
            case NormalArrow:
                input = attack_with_normal_arrow(i, hero.dir_y, hero.dir_x);
                get_input = 0;
                attacked = 1;
                break;
            case Sword: // Gun
                input = attack_with_gun(i, hero.dir_y, hero.dir_x);
                get_input = 0;
                attacked = 1;
                break;
            case EmptyWeapon:
                print_message("No weapon equipped!", "Press any key...");
                input = getch();
                get_input = 0;
                break;
            }
        }

        if (get_input == 1)
            input = getch();

        handle_move_command(i, input);

        if (input == '1')
        {
            show_food_inventory(i);
        }

        if (input == '2')
        {
            show_weapon_inventory();
        }

        if (input == '3')
        {
            show_spell_inventory(i);
        }

        if (input == 'm' || input == 'M')
        {
            print_entire_map(i);
        }

        if (input == 'i' || input == 'I')
        {
            show_current_weapon();
        }

        if (attacked == 0)
            enemies_attack_hero(i);

        if (hero.satiety >= 50)
        {
            if (hero.health >= 90)
                hero.health = 100;
            else
                hero.health += 10;
        }

        hero.satiety_progress += 1;

        if (hero.satiety_progress == 80)
        {
            if (hero.satiety >= 5)
                hero.satiety -= 5;
            hero.satiety_progress = 0;
        }

        handle_enemies_movement(i);
        update_food_status();
    }
}

void draw_score_board_menu()
{
    clear();
    attron(COLOR_PAIR(1));
    mvprintw((LINES / 2) - 12, (COLS / 2) - 46, "__ Score Board _____________________________________________________________________________");
    mvprintw((LINES / 2) - 10, (COLS / 2) - 41, "Rank      Champion                   Score      Gold      Wins / Games      Epithet");
    for (int i = 0; i < 19; i++)
    {
        mvprintw((LINES / 2) - 11 + i, (COLS / 2) - 46, "|");
        mvprintw((LINES / 2) - 11 + i, (COLS / 2) + 45, "|");
    }

    mvprintw((LINES / 2) + 2, (COLS / 2) - 45, "------------------------------------------------------------------------------------------");
    mvprintw((LINES / 2) + 4, (COLS / 2) - 6, "Page:");
    mvprintw((LINES / 2) + 8, (COLS / 2) - 46, "‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾");
    attroff(COLOR_PAIR(1));
}

void draw_users_score_board(int page, int page_select)
{
    int total_pages = ((game.users_num + 1) / 5) + 1;
    if (page < total_pages)
    {
        for (int i = 0; i < 5; i++)
        {
            int rank = (5 * (page - 1)) + i + 1;
            if (rank == 1)
                attron(COLOR_PAIR(2));
            if (rank == 2)
                attron(COLOR_PAIR(4));
            if (rank == 3)
                attron(COLOR_PAIR(6));
            User *temp = game.users[rank - 1];
            if (strcmp(temp->username, current_user.username) == 0)
                attron(A_BOLD);
            switch (rank)
            {
            case (1):
                mvprintw((LINES / 2) - 8, (COLS / 2) - 41, "1 🏆");
                break;
            case (2):
                mvprintw((LINES / 2) - 6, (COLS / 2) - 41, "2 🥈");
                break;
            case (3):
                mvprintw((LINES / 2) - 4, (COLS / 2) - 41, "3 🥉");
                break;
            default:
                mvprintw((LINES / 2) + (2 * i) - 8, (COLS / 2) - 41, "%d", rank);
                break;
            }
            mvprintw((LINES / 2) + (2 * i) - 8, (COLS / 2) - 31, "%s", temp->username);
            mvprintw((LINES / 2) + (2 * i) - 8, (COLS / 2) - 4, "%d", temp->total_score);
            mvprintw((LINES / 2) + (2 * i) - 8, (COLS / 2) + 7, "%d", temp->total_gold);
            mvprintw((LINES / 2) + (2 * i) - 8, (COLS / 2) + 17, "%4d / %d", temp->win_num, temp->game_num);
            switch (rank)
            {
            case (1):
                mvprintw((LINES / 2) + (2 * i) - 8, (COLS / 2) + 35, "The GOAT");
                break;
            case (2):
                mvprintw((LINES / 2) + (2 * i) - 8, (COLS / 2) + 35, "LEGEND");
                break;
            case (3):
                mvprintw((LINES / 2) + (2 * i) - 8, (COLS / 2) + 35, "Master");
                break;
            }
            if (rank == 1)
                attroff(COLOR_PAIR(2));
            if (rank == 2)
                attroff(COLOR_PAIR(4));
            if (rank == 3)
                attroff(COLOR_PAIR(6));
            if (strcmp(temp->username, current_user.username) == 0)
                attroff(A_BOLD);
        }
    }
    else if (page == total_pages)
    {
        for (int i = 0; i < (game.users_num % 5); i++)
        {
            int rank = (5 * (page - 1)) + i + 1;
            if (rank == 1)
                attron(COLOR_PAIR(2));
            if (rank == 2)
                attron(COLOR_PAIR(4));
            if (rank == 3)
                attron(COLOR_PAIR(6));
            User *temp = game.users[rank - 1];
            if (strcmp(temp->username, current_user.username) == 0)
                attron(A_BOLD);
            switch (rank)
            {
            case (1):
                mvprintw((LINES / 2) - 8, (COLS / 2) - 41, "1 🏆");
                break;
            case (2):
                mvprintw((LINES / 2) - 6, (COLS / 2) - 41, "2 🥈");
                break;
            case (3):
                mvprintw((LINES / 2) - 4, (COLS / 2) - 41, "3 🥉");
                break;
            default:
                mvprintw((LINES / 2) + (2 * i) - 8, (COLS / 2) - 41, "%d", rank);
                break;
            }
            mvprintw((LINES / 2) + (2 * i) - 8, (COLS / 2) - 31, "%s", temp->username);
            mvprintw((LINES / 2) + (2 * i) - 8, (COLS / 2) - 4, "%d", temp->total_score);
            mvprintw((LINES / 2) + (2 * i) - 8, (COLS / 2) + 7, "%d", temp->total_gold);
            mvprintw((LINES / 2) + (2 * i) - 8, (COLS / 2) + 17, "%4d / %d", temp->win_num, temp->game_num);
            switch (rank)
            {
            case (1):
                mvprintw((LINES / 2) + (2 * i) - 8, (COLS / 2) + 35, "The GOAT");
                break;
            case (2):
                mvprintw((LINES / 2) + (2 * i) - 8, (COLS / 2) + 35, "LEGEND");
                break;
            case (3):
                mvprintw((LINES / 2) + (2 * i) - 8, (COLS / 2) + 35, "Master");
                break;
            }
            if (rank == 1)
                attroff(COLOR_PAIR(2));
            if (rank == 2)
                attroff(COLOR_PAIR(4));
            if (rank == 3)
                attroff(COLOR_PAIR(6));
            if (strcmp(temp->username, current_user.username) == 0)
                attroff(A_BOLD);
        }
    }

    if (page_select == 1)
    {
        attron(A_REVERSE);
        mvprintw((LINES / 2) + 4, (COLS / 2), "%d", page);
        attroff(A_REVERSE);
        mvprintw((LINES / 2) + 4, (COLS / 2) + 2, "/ %d", total_pages);
        mvprintw((LINES / 2) + 6, (COLS / 2) - 4, "Go Back");
    }

    else
    {
        mvprintw((LINES / 2) + 4, (COLS / 2), "%d", page);
        mvprintw((LINES / 2) + 4, (COLS / 2) + 2, "/ %d", total_pages);
        attron(A_REVERSE);
        mvprintw((LINES / 2) + 6, (COLS / 2) - 4, "Go Back");
        attroff(A_REVERSE);
    }
}

void score_board_menu()
{
    if (load_users_from_csv("users.csv", &game.users, &game.users_num) == 0)
    {
        int users_num = game.users_num;
        int total_pages = ((game.users_num + 1) / 5) + 1;
        int current_page = 1;
        int select_choice = 1;
        while (1)
        {
            draw_score_board_menu();
            draw_users_score_board(current_page, select_choice);
            int key = getch();
            if (select_choice == 1 && key == KEY_RIGHT)
                current_page = (current_page % total_pages) + 1;
            else if (select_choice == 1 && key == KEY_LEFT)
                current_page = ((current_page + total_pages - 2) % total_pages) + 1;
            else if (key == KEY_UP || key == KEY_DOWN)
                select_choice = (select_choice + 1) % 2;
            else if (select_choice == 0 && (key == 10 || key == 32))
                break;
        }
    }
}

// Comparator for sorting users by total_score (descending)
int compare_users(const void *a, const void *b)
{
    const User *userA = *(const User **)a;
    const User *userB = *(const User **)b;
    return (userB->total_gold - userA->total_gold);
}

// Save a single user to CSV (appends to file)
int save_user_to_csv(const char *filename, const User *user)
{
    FILE *file = fopen(filename, "a");
    if (!file)
        return -1;

    fprintf(file, "%s,%s,%s,%d,%d,%d,%d,%d,%d,%d,%d\n",
            user->username,
            user->password,
            user->email,
            user->difficulty,
            user->color_option,
            user->game_num,
            user->win_num,
            user->total_score,
            user->total_gold,
            user->play_music,
            user->playlist);

    fclose(file);
    return 0;
}

// Load users from CSV, keeping only the latest entries and sorting by score
int load_users_from_csv(const char *filename, User ***users, int *num_users)
{
    FILE *file = fopen(filename, "r");
    if (!file)
        return -1;

    char buffer[512];
    User **temp_users = NULL;
    int capacity = 10;
    int count = 0;
    temp_users = malloc(capacity * sizeof(User *));

    while (fgets(buffer, sizeof(buffer), file))
    {
        buffer[strcspn(buffer, "\n")] = '\0';

        User *new_user = malloc(sizeof(User));
        if (!new_user)
        {
            fclose(file);
            for (int i = 0; i < count; i++)
                free(temp_users[i]);
            free(temp_users);
            return -2;
        }

        int parsed = sscanf(buffer,
                            "%25[^,],%28[^,],%25[^,],%d,%d,%d,%d,%d,%d,%d,%d",
                            new_user->username,
                            new_user->password,
                            new_user->email,
                            &new_user->difficulty,
                            &new_user->color_option,
                            &new_user->game_num,
                            &new_user->win_num,
                            &new_user->total_score,
                            &new_user->total_gold,
                            &new_user->play_music,
                            &new_user->playlist);

        if (parsed != 11)
        {
            free(new_user);
            continue;
        }

        // Check for existing username
        int found = -1;
        for (int i = 0; i < count; i++)
        {
            if (strcmp(temp_users[i]->username, new_user->username) == 0)
            {
                found = i;
                break;
            }
        }

        if (found != -1)
        {
            // Replace existing entry with newer one
            free(temp_users[found]);
            temp_users[found] = new_user;
        }
        else
        {
            // Expand array if needed
            if (count >= capacity)
            {
                capacity *= 2;
                User **temp = realloc(temp_users, capacity * sizeof(User *));
                if (!temp)
                {
                    fclose(file);
                    for (int i = 0; i < count; i++)
                        free(temp_users[i]);
                    free(temp_users);
                    free(new_user);
                    return -2;
                }
                temp_users = temp;
            }
            temp_users[count++] = new_user;
        }
    }

    fclose(file);

    // Sort users by total_score descending
    qsort(temp_users, count, sizeof(User *), compare_users);

    *users = temp_users;
    *num_users = count;
    return 0;
}

char *generate_password()
{
    const char lowercase[] = "abcdefghijklmnopqrstuvwxyz";
    const char uppercase[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const char digits[] = "0123456789";
    const char all_chars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

    int length = 7 + rand() % 14;

    char *password = malloc((length + 1) * sizeof(char));
    if (password == NULL)
    {
        fprintf(stderr, "Error: Memory allocation failed!\n");
        return NULL;
    }

    // Ensure the password contains at least one lowercase, one uppercase, and one digit
    password[0] = lowercase[rand() % 26]; // Random lowercase
    password[1] = uppercase[rand() % 26]; // Random uppercase
    password[2] = digits[rand() % 10];    // Random digit

    // Fill the rest of the password with random characters
    for (int i = 3; i < length; i++)
    {
        password[i] = all_chars[rand() % 62]; // Random character from all sets
    }

    // Shuffle the password to ensure randomness
    for (int i = 0; i < length; i++)
    {
        int j = rand() % length;
        char temp = password[i];
        password[i] = password[j];
        password[j] = temp;
    }

    // Null-terminate the password
    password[length] = '\0';

    return password;
}

void golden_freddy_appear()
{
    clear();
    attron(COLOR_PAIR(7));
    for (int i = 0; i < 58; i++)
        mvprintw((LINES / 2) - 29 + i, (COLS / 2) - 65, "%s", golden_freddy[i]);
    attroff(COLOR_PAIR(7));
    refresh();
    sleep(4);
    clear();
}

void create_enemies(int level_num)
{
    int enemy_count = (rand() % 2) + level_num + 4;
    level[level_num].enemies = (Enemy *)malloc(enemy_count * sizeof(Enemy));

    for (int i = 0; i < enemy_count; i++)
    {
        level[level_num].enemies[i].location = random_location_enemy(level_num);
        level[level_num].enemies[i].type = rand() % 5;
        level[level_num].enemies[i].visible = 0;
        level[level_num].enemies[i].location_room = find_room(level_num, level[level_num].enemies[i].location);
        map[level_num][level[level_num].enemies[i].location.y][level[level_num].enemies[i].location.x] = level[level_num].enemies[i].type + 22;
        switch (level[level_num].enemies->type)
        {
        case (deamon):
            level[level_num].enemies[i].health = 5;
            level[level_num].enemies[i].damage = 3;
            level[level_num].enemies[i].speed = 1;
            level[level_num].enemies[i].follow = 0;
            break;
        case (fire_monster):
            level[level_num].enemies[i].health = 10;
            level[level_num].enemies[i].damage = 7;
            level[level_num].enemies[i].speed = 1;
            level[level_num].enemies[i].follow = 0;
            break;
        case (giant):
            level[level_num].enemies[i].health = 15;
            level[level_num].enemies[i].damage = 10;
            level[level_num].enemies[i].speed = 1;
            level[level_num].enemies[i].follow = 1;
            break;
        case (snake):
            level[level_num].enemies[i].health = 20;
            level[level_num].enemies[i].damage = 15;
            level[level_num].enemies[i].speed = 1;
            level[level_num].enemies[i].follow = 1;
            break;
        case (undead):
            level[level_num].enemies[i].health = 30;
            level[level_num].enemies[i].damage = 20;
            level[level_num].enemies[i].speed = 1;
            level[level_num].enemies[i].follow = 1;
            break;
        }
    }

    level[level_num].enemy_num = enemy_count;
}

void print_enemies(int level_num)
{
    for (int i = 0; i < level[level_num].enemy_num; i++)
    {
        if (visibility_grid[level_num][level[level_num].enemies[i].location.y][level[level_num].enemies[i].location.x] == 1 &&
            visibility_grid[level_num][level[level_num].enemies[i].location.y][level[level_num].enemies[i].location.x + 1] == 1 &&
            level[level_num].enemies[i].visible == 1)
        {
            switch (level[level_num].enemies[i].type)
            {
            case (deamon):
                mvprintw(level[level_num].enemies[i].location.y, level[level_num].enemies[i].location.x, "👹");
                break;
            case (fire_monster):
                mvprintw(level[level_num].enemies[i].location.y, level[level_num].enemies[i].location.x, "🐲");
                break;
            case (giant):
                mvprintw(level[level_num].enemies[i].location.y, level[level_num].enemies[i].location.x, "🗿");
                break;
            case (snake):
                mvprintw(level[level_num].enemies[i].location.y, level[level_num].enemies[i].location.x, "🐍");
                break;
            case (undead):
                mvprintw(level[level_num].enemies[i].location.y, level[level_num].enemies[i].location.x, "🧟");
                break;
            }
        }
    }
}

void make_enemies_visible(int level_num)
{
    int hero_room = find_room(level_num, hero.location);
    for (int i = 0; i < level[level_num].enemy_num; i++)
    {
        if (find_room(level_num, level[level_num].enemies[i].location) == hero_room && hero_room != -1)
            level[level_num].enemies[i].visible = 1;
        else
            level[level_num].enemies[i].visible = 0;
    }
}

void handle_enemies_movement(int level_num)
{
    int hero_room = find_room(level_num, hero.location);
    for (int i = 0; i < level[level_num].enemy_num; i++)
    {
        int moved = 0;
        if (level[level_num].enemies[i].visible == 1 &&
            visibility_grid[level_num][level[level_num].enemies[i].location.y][level[level_num].enemies[i].location.x] == 1 &&
            visibility_grid[level_num][level[level_num].enemies[i].location.y][level[level_num].enemies[i].location.x + 1] == 1 &&
            level[level_num].enemies[i].location_room == hero_room)
        {
            switch (level[level_num].enemies[i].type)
            {
            case (fire_monster):
                if (level[level_num].enemies[i].speed == 1 && in_range(hero.location, level[level_num].enemies[i].location, 3))
                {
                    int y = rand() % 2;
                    if (y == 1)
                    {
                        if (moved == 0 && level[level_num].enemies[i].location.y + 1 < hero.location.y)
                        {
                            Point destination;
                            destination.y = level[level_num].enemies[i].location.y + 1;
                            destination.x = level[level_num].enemies[i].location.x;
                            if (valid_point_enemy(level_num, destination) && find_room(level_num, destination) == level[level_num].enemies[i].location_room)
                            {
                                level[level_num].enemies[i].location.y += 1;
                                moved = 1;
                            }
                        }

                        if (moved == 0 && level[level_num].enemies[i].location.y - 1 > hero.location.y)
                        {
                            Point destination;
                            destination.y = level[level_num].enemies[i].location.y - 1;
                            destination.x = level[level_num].enemies[i].location.x;
                            if (valid_point_enemy(level_num, destination) && find_room(level_num, destination) == level[level_num].enemies[i].location_room)
                            {
                                level[level_num].enemies[i].location.y -= 1;
                                moved = 1;
                            }
                        }
                    }
                    else
                    {
                        if (moved == 0 && level[level_num].enemies[i].location.x + 2 < hero.location.x)
                        {
                            Point destination;
                            destination.y = level[level_num].enemies[i].location.y;
                            destination.x = level[level_num].enemies[i].location.x + 1;
                            if (valid_point_enemy(level_num, destination) && find_room(level_num, destination) == level[level_num].enemies[i].location_room)
                            {
                                level[level_num].enemies[i].location.x += 1;
                                moved = 1;
                            }
                        }

                        if (moved == 0 && level[level_num].enemies[i].location.x - 2 > hero.location.x)
                        {
                            Point destination;
                            destination.y = level[level_num].enemies[i].location.y;
                            destination.x = level[level_num].enemies[i].location.x - 1;
                            if (valid_point_enemy(level_num, destination) && find_room(level_num, destination) == level[level_num].enemies[i].location_room)
                            {
                                level[level_num].enemies[i].location.x -= 1;
                                moved = 1;
                            }
                        }
                    }
                }
                break;
            case (giant):
                if (level[level_num].enemies[i].speed == 1 && in_range(hero.location, level[level_num].enemies[i].location, 3))
                {
                    if (moved == 0 && level[level_num].enemies[i].location.y + 1 < hero.location.y)
                    {
                        Point destination;
                        destination.y = level[level_num].enemies[i].location.y + 1;
                        destination.x = level[level_num].enemies[i].location.x;
                        if (valid_point_enemy(level_num, destination) && find_room(level_num, destination) == level[level_num].enemies[i].location_room)
                        {
                            level[level_num].enemies[i].location.y += 1;
                            moved = 1;
                        }
                    }

                    if (moved == 0 && level[level_num].enemies[i].location.y - 1 > hero.location.y)
                    {
                        Point destination;
                        destination.y = level[level_num].enemies[i].location.y - 1;
                        destination.x = level[level_num].enemies[i].location.x;
                        if (valid_point_enemy(level_num, destination) && find_room(level_num, destination) == level[level_num].enemies[i].location_room)
                        {
                            level[level_num].enemies[i].location.y -= 1;
                            moved = 1;
                        }
                    }

                    moved = 0;
                    if (moved == 0 && level[level_num].enemies[i].location.x + 2 < hero.location.x)
                    {
                        Point destination;
                        destination.y = level[level_num].enemies[i].location.y;
                        destination.x = level[level_num].enemies[i].location.x + 1;
                        if (valid_point_enemy(level_num, destination) && find_room(level_num, destination) == level[level_num].enemies[i].location_room)
                        {
                            level[level_num].enemies[i].location.x += 1;
                            moved = 1;
                        }
                    }

                    if (moved == 0 && level[level_num].enemies[i].location.x - 2 > hero.location.x)
                    {
                        Point destination;
                        destination.y = level[level_num].enemies[i].location.y;
                        destination.x = level[level_num].enemies[i].location.x - 1;
                        if (valid_point_enemy(level_num, destination) && find_room(level_num, destination) == level[level_num].enemies[i].location_room)
                        {
                            level[level_num].enemies[i].location.x -= 1;
                            moved = 1;
                        }
                    }
                }
                break;
            case (snake):
                if (moved == 0 && level[level_num].enemies[i].location.y + 1 < hero.location.y)
                {
                    Point destination;
                    destination.y = level[level_num].enemies[i].location.y + 1;
                    destination.x = level[level_num].enemies[i].location.x;
                    if (valid_point_enemy(level_num, destination) && find_room(level_num, destination) == level[level_num].enemies[i].location_room)
                    {
                        level[level_num].enemies[i].location.y += 1;
                        moved = 1;
                    }
                }

                if (moved == 0 && level[level_num].enemies[i].location.y - 1 > hero.location.y)
                {
                    Point destination;
                    destination.y = level[level_num].enemies[i].location.y - 1;
                    destination.x = level[level_num].enemies[i].location.x;
                    if (valid_point_enemy(level_num, destination) && find_room(level_num, destination) == level[level_num].enemies[i].location_room)
                    {
                        level[level_num].enemies[i].location.y -= 1;
                        moved = 1;
                    }
                }

                moved = 0;
                if (moved == 0 && level[level_num].enemies[i].location.x + 2 < hero.location.x)
                {
                    Point destination;
                    destination.y = level[level_num].enemies[i].location.y;
                    destination.x = level[level_num].enemies[i].location.x + 1;
                    if (valid_point_enemy(level_num, destination) && find_room(level_num, destination) == level[level_num].enemies[i].location_room)
                    {
                        level[level_num].enemies[i].location.x += 1;
                        moved = 1;
                    }
                }

                if (moved == 0 && level[level_num].enemies[i].location.x - 2 > hero.location.x)
                {
                    Point destination;
                    destination.y = level[level_num].enemies[i].location.y;
                    destination.x = level[level_num].enemies[i].location.x - 1;
                    if (valid_point_enemy(level_num, destination) && find_room(level_num, destination) == level[level_num].enemies[i].location_room)
                    {
                        level[level_num].enemies[i].location.x -= 1;
                        moved = 1;
                    }
                }
                break;
            case (undead):
                if (level[level_num].enemies[i].speed == 1)
                {
                    if (moved == 0 && level[level_num].enemies[i].location.y + 2 < hero.location.y)
                    {
                        Point destination;
                        destination.y = level[level_num].enemies[i].location.y + 2;
                        destination.x = level[level_num].enemies[i].location.x;
                        if (valid_point_enemy(level_num, destination) && find_room(level_num, destination) == level[level_num].enemies[i].location_room)
                        {
                            level[level_num].enemies[i].location.y += 2;
                            moved = 1;
                        }
                    }

                    if (moved == 0 && level[level_num].enemies[i].location.y - 2 > hero.location.y)
                    {
                        Point destination;
                        destination.y = level[level_num].enemies[i].location.y - 2;
                        destination.x = level[level_num].enemies[i].location.x;
                        if (valid_point_enemy(level_num, destination) && find_room(level_num, destination) == level[level_num].enemies[i].location_room)
                        {
                            level[level_num].enemies[i].location.y -= 2;
                            moved = 1;
                        }
                    }

                    if (moved == 0 && level[level_num].enemies[i].location.y + 1 < hero.location.y)
                    {
                        Point destination;
                        destination.y = level[level_num].enemies[i].location.y + 1;
                        destination.x = level[level_num].enemies[i].location.x;
                        if (valid_point_enemy(level_num, destination) && find_room(level_num, destination) == level[level_num].enemies[i].location_room)
                        {
                            level[level_num].enemies[i].location.y += 1;
                            moved = 1;
                        }
                    }

                    if (moved == 0 && level[level_num].enemies[i].location.y - 1 > hero.location.y)
                    {
                        Point destination;
                        destination.y = level[level_num].enemies[i].location.y - 1;
                        destination.x = level[level_num].enemies[i].location.x;
                        if (valid_point_enemy(level_num, destination) && find_room(level_num, destination) == level[level_num].enemies[i].location_room)
                        {
                            level[level_num].enemies[i].location.y -= 1;
                            moved = 1;
                        }
                    }

                    moved = 0;
                    if (moved == 0 && level[level_num].enemies[i].location.x + 3 < hero.location.x)
                    {
                        Point destination;
                        destination.y = level[level_num].enemies[i].location.y;
                        destination.x = level[level_num].enemies[i].location.x + 1;
                        if (valid_point_enemy(level_num, destination) && find_room(level_num, destination) == level[level_num].enemies[i].location_room)
                        {
                            level[level_num].enemies[i].location.x += 2;
                            moved = 1;
                        }
                    }

                    if (moved == 0 && level[level_num].enemies[i].location.x - 3 > hero.location.x)
                    {
                        Point destination;
                        destination.y = level[level_num].enemies[i].location.y;
                        destination.x = level[level_num].enemies[i].location.x - 2;
                        if (valid_point_enemy(level_num, destination) && find_room(level_num, destination) == level[level_num].enemies[i].location_room)
                        {
                            level[level_num].enemies[i].location.x -= 2;
                            moved = 1;
                        }
                    }

                    if (moved == 0 && level[level_num].enemies[i].location.x + 2 < hero.location.x)
                    {
                        Point destination;
                        destination.y = level[level_num].enemies[i].location.y;
                        destination.x = level[level_num].enemies[i].location.x + 1;
                        if (valid_point_enemy(level_num, destination) && find_room(level_num, destination) == level[level_num].enemies[i].location_room)
                        {
                            level[level_num].enemies[i].location.x += 1;
                            moved = 1;
                        }
                    }

                    if (moved == 0 && level[level_num].enemies[i].location.x - 2 > hero.location.x)
                    {
                        Point destination;
                        destination.y = level[level_num].enemies[i].location.y;
                        destination.x = level[level_num].enemies[i].location.x - 1;
                        if (valid_point_enemy(level_num, destination) && find_room(level_num, destination) == level[level_num].enemies[i].location_room)
                        {
                            level[level_num].enemies[i].location.x -= 1;
                            moved = 1;
                        }
                    }
                }
                break;
            }
        }

        level[level_num].enemies[i].speed = !level[level_num].enemies[i].speed;
    }
}

int in_range(Point p1, Point p2, int distance)
{
    int dx = p1.x - p2.x;
    if (dx < 0)
        dx *= -1;

    int dy = p1.y - p2.y;
    if (dy < 0)
        dy *= -1;

    if (dx + dy <= distance)
        return 1;
    return 0;
}

Point random_location_enemy(int level_num)
{
    Point result;
    int first_room;

    while (1)
    {
        first_room = rand() % 9;
        if (level[level_num].room[first_room].room_exist == 1)
        {
            Room room = level[level_num].room[first_room];

            result.y = room.corner.y + 1;
            result.x = room.corner.x;

            result.y += (rand() % (room.width - 2)) + 1;
            result.x += (rand() % ((room.length - 6) / 2) * 2) + 2;

            if (map[level_num][result.y][result.x] == 0 && map[level_num][result.y][result.x + 2] == 0)
                return result;
        }
    }
}

int valid_point_enemy(int level_num, Point destination)
{
    int y = destination.y;
    int x = destination.x;

    for (int i = 0; i < level[level_num].object_num; i++)
    {
        if ((y == level[level_num].objects[i].location.y && x == level[level_num].objects[i].location.x) ||
            (y == level[level_num].objects[i].location.y && x == level[level_num].objects[i].location.x - 1) ||
            (y == level[level_num].objects[i].location.y && x == level[level_num].objects[i].location.x + 1))
            return 0;
    }

    if ((y == hero.location.y && x == hero.location.x) ||
        (y == hero.location.y && x == hero.location.x - 1) ||
        (y == hero.location.y && x == hero.location.x + 1))
        return 0;

    for (int i = 0; i < 9; i++)
    {
        Room temp = level[level_num].room[i];
        if (temp.room_exist == 1)
        {
            if (y > temp.corner.y + 1 && y < temp.corner.y + temp.width && x > temp.corner.x + 1 && x < temp.corner.x + temp.length - 3)
                return 1;
        }
    }

    for (int i = 0; i < 12; i++)
    {
        if (level[level_num].corridor_exist[i] == 1)
        {
            Point door_1, door_2, mid_1, mid_2;

            mid_1 = level[level_num].between_doors_1[i];
            mid_2 = level[level_num].between_doors_2[i];

            switch (i)
            {
            case (0):
                door_1 = level[level_num].room[0].door[0];
                door_2 = level[level_num].room[1].door[0];
                break;
            case (1):
                door_1 = level[level_num].room[1].door[1];
                door_2 = level[level_num].room[2].door[0];
                break;
            case (2):
                door_1 = level[level_num].room[0].door[1];
                door_2 = level[level_num].room[3].door[0];
                break;
            case (3):
                door_1 = level[level_num].room[1].door[2];
                door_2 = level[level_num].room[4].door[0];
                break;
            case (4):
                door_1 = level[level_num].room[2].door[1];
                door_2 = level[level_num].room[5].door[0];
                break;
            case (5):
                door_1 = level[level_num].room[3].door[1];
                door_2 = level[level_num].room[4].door[1];
                break;
            case (6):
                door_1 = level[level_num].room[4].door[2];
                door_2 = level[level_num].room[5].door[1];
                break;
            case (7):
                door_1 = level[level_num].room[3].door[2];
                door_2 = level[level_num].room[6].door[0];
                break;
            case (8):
                door_1 = level[level_num].room[4].door[3];
                door_2 = level[level_num].room[7].door[0];
                break;
            case (9):
                door_1 = level[level_num].room[5].door[2];
                door_2 = level[level_num].room[8].door[0];
                break;
            case (10):
                door_1 = level[level_num].room[6].door[1];
                door_2 = level[level_num].room[7].door[1];
                break;
            case (11):
                door_1 = level[level_num].room[7].door[2];
                door_2 = level[level_num].room[8].door[1];
                break;
            }

            if (i == 0 || i == 1 || i == 5 || i == 6 || i == 10 || i == 11)
            {
                if (y == door_1.y && x > door_1.x - 1 && x <= mid_1.x)
                    return 1;
                if (mid_1.y < mid_2.y)
                {
                    if (x == mid_1.x && y >= mid_1.y && y <= mid_2.y)
                        return 1;
                }
                else
                {
                    if (x == mid_1.x && y >= mid_2.y && y <= mid_1.y)
                        return 1;
                }
                if (y == mid_2.y && x >= mid_2.x && x < door_2.x)
                    return 1;
            }
            else
            {
                if (x == door_1.x && y > door_1.y && y <= mid_1.y)
                    return 1;
                if (mid_1.x < mid_2.x)
                {
                    if (y == mid_1.y && x >= mid_1.x && x <= mid_2.x)
                        return 1;
                }
                else
                {
                    if (y == mid_1.y && x >= mid_2.x && x <= mid_1.x)
                        return 1;
                }
                if (x == mid_2.x && y >= mid_2.y && y < door_2.y)
                    return 1;
            }
        }
    }

    return 0;
}

const char *enemy_type_name(EnemyType type)
{
    switch (type)
    {
    case deamon:
        return "Deamon";
    case fire_monster:
        return "Fire Monster";
    case giant:
        return "Giant";
    case snake:
        return "Snake";
    case undead:
        return "Undead";
    default:
        return "Unknown Enemy";
    }
}

int attack_with_mace(int level_num)
{
    int result;
    int got_input = 0;

    int damage = 5;
    char message[100];

    for (int dy = -1; dy <= 1; dy++)
    {
        for (int dx = -2; dx <= 2; dx++)
        {
            if (dx == 0 && dy == 0)
                continue; // Skip the hero's position
            Point target = {hero.location.y + dy, hero.location.x + dx};
            for (int i = 0; i < level[level_num].enemy_num; i++)
            {
                Enemy *e = &level[level_num].enemies[i];
                if (e->location.y == target.y && e->location.x == target.x && e->health > 0)
                {
                    e->health -= damage;

                    // Print damage message
                    char msg[100];
                    snprintf(msg, sizeof(msg), "Hit %s with mace! (-%d HP)", enemy_type_name(e->type), damage);
                    print_message(msg, "");
                    result = getch();
                    got_input = 1;

                    // Print death message if enemy dies
                    if (e->health <= 0)
                    {
                        char death_msg[100];
                        snprintf(death_msg, sizeof(death_msg), "Congrats! %s was defeated!", enemy_type_name(e->type));
                        print_message(death_msg, "");
                        remove_enemy(level_num, i);
                        result = getch();
                        got_input = 1;
                    }
                }
            }
        }
    }

    if (got_input == 0)
        result = getch();

    return result;
}

int attack_with_dagger(int level_num, int direction_y, int direction_x)
{
    int result;
    int got_input = 0;

    int damage = 12;
    Point dagger_pos = hero.location;
    for (int i = 0; i < 5; i++)
    { // Dagger range is 5 units
        dagger_pos.y += direction_y;
        dagger_pos.x += direction_x;
        for (int j = 0; j < level[level_num].enemy_num; j++)
        {
            Enemy *e = &level[level_num].enemies[j];
            if (e->location.y == dagger_pos.y && e->location.x == dagger_pos.x && e->health > 0)
            {
                e->health -= damage;

                // Print damage message
                char msg[100];
                snprintf(msg, sizeof(msg), "Hit %s with dagger! (-%d HP)", enemy_type_name(e->type), damage);
                print_message(msg, "");
                result = getch();
                got_input = 1;

                // Print death message if enemy dies
                if (e->health <= 0)
                {
                    char death_msg[100];
                    snprintf(death_msg, sizeof(death_msg), "Congrats! %s was defeated!", enemy_type_name(e->type));
                    print_message(death_msg, "");
                    remove_enemy(level_num, i);
                    result = getch();
                    got_input = 1;
                }

                if (got_input == 0)
                    result = getch();
                return result; // Dagger stops after hitting an enemy
            }
        }
    }

    if (got_input == 0)
        result = getch();

    return result;
}

int attack_with_magic_wand(int level_num, int direction_y, int direction_x)
{
    int result;
    int got_input = 0;

    int damage = 15;
    Point wand_pos = hero.location;
    for (int i = 0; i < 10; i++)
    { // Wand range is 10 units
        wand_pos.y += direction_y;
        wand_pos.x += direction_x;
        for (int j = 0; j < level[level_num].enemy_num; j++)
        {
            Enemy *e = &level[level_num].enemies[j];
            if (e->location.y == wand_pos.y && e->location.x == wand_pos.x && e->health > 0)
            {
                e->health -= damage;
                e->stunned = 1; // Stun the enemy

                // Print damage and stun message
                char msg[100];
                snprintf(msg, sizeof(msg), "Hit %s with magic wand! (-%d HP, stunned)", enemy_type_name(e->type), damage);
                print_message(msg, "");
                result = getch();
                got_input = 1;

                // Print death message if enemy dies
                if (e->health <= 0)
                {
                    char death_msg[100];
                    snprintf(death_msg, sizeof(death_msg), "Congrats! %s was defeated!", enemy_type_name(e->type));
                    print_message(death_msg, "");
                    remove_enemy(level_num, i);
                    result = getch();
                    got_input = 1;
                }

                if (got_input == 0)
                    result = getch();
                return result;
            }
        }
    }

    if (got_input == 0)
        result = getch();

    return result;
}

int attack_with_normal_arrow(int level_num, int direction_y, int direction_x)
{
    int result;
    int got_input = 0;

    int damage = 5;
    Point arrow_pos = hero.location;
    for (int i = 0; i < 5; i++)
    { // Arrow range is 5 units
        arrow_pos.y += direction_y;
        arrow_pos.x += direction_x;
        for (int j = 0; j < level[level_num].enemy_num; j++)
        {
            Enemy *e = &level[level_num].enemies[j];
            if (e->location.y == arrow_pos.y && e->location.x == arrow_pos.x && e->health > 0)
            {
                e->health -= damage;

                // Print damage message
                char msg[100];
                snprintf(msg, sizeof(msg), "Hit %s with arrow! (-%d HP)", enemy_type_name(e->type), damage);
                print_message(msg, "");
                result = getch();
                got_input = 1;

                // Print death message if enemy dies
                if (e->health <= 0)
                {
                    char death_msg[100];
                    snprintf(death_msg, sizeof(death_msg), "Congrats! %s was defeated!", enemy_type_name(e->type));
                    print_message(death_msg, "");
                    remove_enemy(level_num, i);
                    result = getch();
                    got_input = 1;
                }

                if (got_input == 0)
                    result = getch();
                return result;
            }
        }
    }

    if (got_input == 0)
        result = getch();

    return result;
}

int attack_with_gun(int level_num, int direction_y, int direction_x)
{
    int result;
    int got_input = 0;

    int damage = 10;
    Point gun_pos = hero.location;
    for (int i = 0; i < 10; i++)
    {
        gun_pos.y += direction_y;
        gun_pos.x += direction_x;
        for (int j = 0; j < level[level_num].enemy_num; j++)
        {
            Enemy *e = &level[level_num].enemies[j];
            if (e->location.y == gun_pos.y && e->location.x == gun_pos.x && e->health > 0)
            {
                e->health -= damage;
                char msg[100];
                snprintf(msg, sizeof(msg), "Shot %s with gun! (-%d HP)", enemy_type_name(e->type), damage);
                print_message(msg, "");
                result = getch();
                got_input = 1;

                if (e->health <= 0)
                {
                    char death_msg[100];
                    snprintf(death_msg, sizeof(death_msg), "Congrats! %s was eliminated!", enemy_type_name(e->type));
                    print_message(death_msg, "");
                    remove_enemy(level_num, i);
                    result = getch();
                    got_input = 1;
                }
            }
        }
    }

    if (got_input == 0)
        result = getch();

    return result;
}

void enemies_attack_hero(int level_num)
{
    for (int i = 0; i < level[level_num].enemy_num; i++)
    {
        if (abs(level[level_num].enemies[i].location.y - hero.location.y) <= 1 &&
            abs(level[level_num].enemies[i].location.x - hero.location.x) <= 1)
        {
            if (hero.health > level[level_num].enemies[i].damage)
                hero.health -= level[level_num].enemies[i].damage;
            else
                hero.health = 0;
            char death_msg[100];
            snprintf(death_msg, sizeof(death_msg), "Enemy hit you for %d damage!", level[level_num].enemies[i].damage);
            print_message(death_msg, "Press any key...");
            getch();
        }
    }
}

void update_food_status()
{
    for (int i = 0; i < hero.food_num; i++)
    {
        if (hero.food_inventory[i].type == GourmetFood)
            hero.food_inventory[i].food_step_count += 1;
        if (hero.food_inventory[i].type == NormalFood)
            hero.food_inventory[i].food_step_count += 1;
    }

    for (int i = 0; i < hero.food_num; i++)
    {
        if (hero.food_inventory[i].type == GourmetFood && hero.food_inventory[i].food_step_count == 30)
        {
            hero.food_inventory[i].type = NormalFood;
            hero.food_inventory[i].food_step_count = 0;
        }
        if (hero.food_inventory[i].type == NormalFood && hero.food_inventory[i].food_step_count == 30)
        {
            hero.food_inventory[i].type = PoisonFood;
            hero.food_inventory[i].food_step_count = 0;
        }
    }
}

void show_death_screen()
{
    clear();
    // for (int i = 0; i < 33; i++)
    //     mvprintw(LINES - 33 + i, (COLS / 2) - 31, "%s", tombstone[i]);
    mvprintw(LINES - 40, ((COLS / 2) - 26), "Sorry, you Lost. I know you can do better next time!");
    mvprintw(LINES - 38, ((COLS / 2) - 9), "- Press any key -");
    getch();
}

void show_win_screen()
{
    clear();
    // for (int i = 0; i < 34; i++)
    //     mvprintw(LINES - 34 + i, (COLS / 2) - 33, "%s", tombstone[i]);
    mvprintw(LINES - 40, ((COLS / 2) - 23), "Congrats, you won. You are officially amazing!");
    mvprintw(LINES - 38, ((COLS / 2) - 9), "- Press any key -");
    getch();
}
