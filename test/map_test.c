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

Level level[MAX_LEVEL];
User current_user;
Character hero;
bool ***visibility_grid;
short int ***map;
int get_input = 1;
int added_coin = 0;
int object_index = -1;

int *three_extra_rooms() // returns an array of 3 random numbers between 0 & 8
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

int main()
{
    setlocale(LC_ALL, "");
    srand(time(NULL));

    initscr();
    curs_set(FALSE);
    keypad(stdscr, TRUE);

    initialize_visibility_grid();
    initialize_map();

    hero.health = 90, hero.coins = 0, hero.speed = 80, hero.satiety = 90, hero.health_progress = 0, hero.satiety_progress = 0;

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

    random_map();

    for (int i = 0; i < MAX_LEVEL; i++)
    {
        initialize_hero(i);
        while (1)
        {
            get_input = 1;
            int location_result = -1;
            object_index = -1;

            clear();
            clear();
            make_cells_visible(i);
            location_result = handle_location(i);
            print_level(i);
            print_corridors(i);
            print_objects(i);
            print_hero();
            print_info(i);

            int input;

            if (location_result == 1) // Staircase
            {
                if (level[i].discovered_room_num == level[i].room_num)
                {
                    print_message("You found the staircase!", "You can press SPACE to go to the next level.");
                    input = getch();
                    if (input == ' ')
                    {
                        move(2, 0);
                        clrtoeol();
                        refresh();
                        break;
                    }
                    else
                        get_input = 0;
                }
                else
                {
                    print_message("You haven't found all the rooms yet!", "");
                }
            }

            // if (location_result == 2) // Poison Food
            // {
            //     while (1)
            //     {
            //         print_message("Oops! You ate a poison food!", "Press SPACE to continue...");
            //         input = getch();
            //         if (input == ' ')
            //         {
            //             move(2, 0);
            //             clrtoeol();
            //             refresh();
            //             break;
            //         }
            //     }
            // }

            if (location_result == 2) // Poison Food
            {
                print_message("This is food.", "Press 'P' to pick it up.");
                input = getch();
                if (input == 'p' || input == 'P')
                {
                    move(2, 0);
                    clrtoeol();
                    refresh();
                    print_message("You can press '1' to see your Food Inventory.", "");
                    input = getch();
                    get_input = 0;
                    hero.food_inventory[hero.food_num] = level[i].objects[object_index];
                    remove_object(i, object_index);
                    move(2, 0);
                    clrtoeol();
                    refresh();
                    hero.food_num += 1;
                }
                else
                {
                    move(2, 0);
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
                move(2, 0);
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
                move(2, 0);
                clrtoeol();
                refresh();
            }

            else if (location_result == 5) // Normal Food
            {
                print_message("This is food.", "Press 'P' to pick it up.");
                input = getch();
                if (input == 'p' || input == 'P')
                {
                    move(2, 0);
                    clrtoeol();
                    refresh();
                    print_message("You can press '1' to see your Food Inventory.", "");
                    input = getch();
                    get_input = 0;
                    hero.food_inventory[hero.food_num] = level[i].objects[object_index];
                    remove_object(i, object_index);
                    move(2, 0);
                    clrtoeol();
                    refresh();
                    hero.food_num += 1;
                }
                else
                {
                    move(2, 0);
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
                    move(2, 0);
                    clrtoeol();
                    refresh();
                    print_message("You can press '1' to see your Food Inventory.", "");
                    input = getch();
                    get_input = 0;
                    hero.food_inventory[hero.food_num] = level[i].objects[object_index];
                    remove_object(i, object_index);
                    move(2, 0);
                    clrtoeol();
                    refresh();
                    hero.food_num += 1;
                }
                else
                {
                    move(2, 0);
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
                    move(2, 0);
                    clrtoeol();
                    refresh();
                    print_message("You can press '1' to see your Food Inventory.", "");
                    input = getch();
                    get_input = 0;
                    hero.food_inventory[hero.food_num] = level[i].objects[object_index];
                    remove_object(i, object_index);
                    move(2, 0);
                    clrtoeol();
                    refresh();
                    hero.food_num += 1;
                }
                else
                {
                    move(2, 0);
                    clrtoeol();
                    refresh();
                    get_input = 0;
                }
            }

            else if (location_result == 8) // Dagger
            {
                print_message("This is a Dagger.", "Press 'P' to pick it up.");
                input = getch();
                if (input == 'p' || input == 'P')
                {
                    move(2, 0);
                    clrtoeol();
                    refresh();
                    print_message("You can press '2' to see your Weapon Inventory.", "");
                    input = getch();
                    get_input = 0;
                    hero.weapon_inventory[hero.weapon_num] = level[i].objects[object_index];
                    remove_object(i, object_index);
                    move(2, 0);
                    clrtoeol();
                    refresh();
                    hero.weapon_num += 1;
                }
                else
                {
                    move(2, 0);
                    clrtoeol();
                    refresh();
                    get_input = 0;
                }
            }

            else if (location_result == 9) // Magic Wand
            {
                print_message("This is a Magic Wand.", "Press 'P' to pick it up.");
                input = getch();
                if (input == 'p' || input == 'P')
                {
                    move(2, 0);
                    clrtoeol();
                    refresh();
                    print_message("You can press '2' to see your Weapon Inventory.", "");
                    input = getch();
                    get_input = 0;
                    hero.weapon_inventory[hero.weapon_num] = level[i].objects[object_index];
                    remove_object(i, object_index);
                    move(2, 0);
                    clrtoeol();
                    refresh();
                    hero.weapon_num += 1;
                }
                else
                {
                    move(2, 0);
                    clrtoeol();
                    refresh();
                    get_input = 0;
                }
            }

            else if (location_result == 10) // Normal Arrow
            {
                print_message("This is a Normal Arrow.", "Press 'P' to pick it up.");
                input = getch();
                if (input == 'p' || input == 'P')
                {
                    move(2, 0);
                    clrtoeol();
                    refresh();
                    print_message("You can press '2' to see your Weapon Inventory.", "");
                    input = getch();
                    get_input = 0;
                    hero.weapon_inventory[hero.weapon_num] = level[i].objects[object_index];
                    remove_object(i, object_index);
                    move(2, 0);
                    clrtoeol();
                    refresh();
                    hero.weapon_num += 1;
                }
                else
                {
                    move(2, 0);
                    clrtoeol();
                    refresh();
                    get_input = 0;
                }
            }

            else if (location_result == 11) // Sword
            {
                print_message("This is a gun.", "Press 'P' to pick it up.");
                input = getch();
                if (input == 'p' || input == 'P')
                {
                    move(2, 0);
                    clrtoeol();
                    refresh();
                    print_message("You can press '2' to see your Weapon Inventory.", "");
                    input = getch();
                    get_input = 0;
                    hero.weapon_inventory[hero.weapon_num] = level[i].objects[object_index];
                    remove_object(i, object_index);
                    move(2, 0);
                    clrtoeol();
                    refresh();
                    hero.weapon_num += 1;
                }
                else
                {
                    move(2, 0);
                    clrtoeol();
                    refresh();
                    get_input = 0;
                }
            }

            else if (location_result == 13) // Health Spell
            {
                print_message("This is a Health Spell.", "Press 'P' to pick it up.");
                input = getch();
                if (input == 'p' || input == 'P')
                {
                    move(2, 0);
                    clrtoeol();
                    refresh();
                    print_message("You can press '3' to see your Spell Inventory.", "");
                    input = getch();
                    get_input = 0;
                    hero.spell_inventory[hero.spell_num] = level[i].objects[object_index];
                    remove_object(i, object_index);
                    move(2, 0);
                    clrtoeol();
                    refresh();
                    hero.spell_num += 1;
                }
                else
                {
                    move(2, 0);
                    clrtoeol();
                    refresh();
                    get_input = 0;
                }
            }

            else if (location_result == 14) // Speed Spell
            {
                print_message("This is a Speed Spell.", "Press 'P' to pick it up.");
                input = getch();
                if (input == 'p' || input == 'P')
                {
                    move(2, 0);
                    clrtoeol();
                    refresh();
                    print_message("You can press '3' to see your Spell Inventory.", "");
                    input = getch();
                    get_input = 0;
                    hero.spell_inventory[hero.spell_num] = level[i].objects[object_index];
                    remove_object(i, object_index);
                    move(2, 0);
                    clrtoeol();
                    refresh();
                    hero.spell_num += 1;
                }
                else
                {
                    move(2, 0);
                    clrtoeol();
                    refresh();
                    get_input = 0;
                }
            }

            else if (location_result == 15) // Damage Spell
            {
                print_message("This is a Damage Spell.", "Press 'P' to pick it up.");
                input = getch();
                if (input == 'p' || input == 'P')
                {
                    move(2, 0);
                    clrtoeol();
                    refresh();
                    print_message("You can press '3' to see your Spell Inventory.", "");
                    input = getch();
                    get_input = 0;
                    hero.spell_inventory[hero.spell_num] = level[i].objects[object_index];
                    remove_object(i, object_index);
                    move(2, 0);
                    clrtoeol();
                    refresh();
                    hero.spell_num += 1;
                }
                else
                {
                    move(2, 0);
                    clrtoeol();
                    refresh();
                    get_input = 0;
                }
            }

            if (get_input == 1)
                input = getch();
            handle_move_command(i, input);

            if (input == '1')
            {
                show_food_inventory();
            }

            if (input == '2')
            {
                show_weapon_inventory();
            }

            if (input == '3')
            {
                show_spell_inventory();
            }

            if (input == 'm' || input == 'M')
            {
                print_entire_map(i);
            }

            usleep(2000 / hero.speed);

            if (hero.satiety >= 80)
                hero.health_progress += 1;

            hero.satiety_progress += 1;

            if (hero.satiety == 100 && hero.health_progress == 10)
            {
                if (hero.health <= 95)
                    hero.health += 5;
                hero.health_progress = 0;
            }

            if (hero.satiety_progress == 30)
            {
                if (hero.satiety >= 5)
                    hero.satiety -= 5;
                hero.satiety_progress = 0;
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
    attron(COLOR_PAIR(6));
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
    attroff(COLOR_PAIR(6));
}

void random_map()
{
    for (int i = 0; i < MAX_LEVEL; i++)
    {
        random_level(i);
        create_corridors(i);
        create_objects(i);
        level[i].level_num = i;
    }
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

void print_test(int leve_num)
{
    for (int i = 0; i < 12; i++)
    {
        mvprintw(level[leve_num].between_doors_1[i].y, level[leve_num].between_doors_1[i].x, "1");
    }

    for (int i = 0; i < 12; i++)
    {
        mvprintw(level[leve_num].between_doors_2[i].y, level[leve_num].between_doors_2[i].x, "2");
    }
}

void initialize_hero(int level_num)
{
    hero.location = random_location(level_num);
    hero.face_option = rand() % 18;
    map[level_num][hero.location.y][hero.location.x] = 2;
}

void print_hero()
{
    switch (hero.face_option)
    {
    case (Dog):
        mvprintw(hero.location.y, hero.location.x, "🐶");
        break;
    case (Cat):
        mvprintw(hero.location.y, hero.location.x, "🐱");
        break;
    case (Hamster):
        mvprintw(hero.location.y, hero.location.x, "🐹");
        break;
    case (Rabbit):
        mvprintw(hero.location.y, hero.location.x, "🐰");
        break;
    case (Fox):
        mvprintw(hero.location.y, hero.location.x, "🦊");
        break;
    case (Bear):
        mvprintw(hero.location.y, hero.location.x, "🐻");
        break;
    case (Panda):
        mvprintw(hero.location.y, hero.location.x, "🐼");
        break;
    case (Tiger):
        mvprintw(hero.location.y, hero.location.x, "🐯");
        break;
    case (Lion):
        mvprintw(hero.location.y, hero.location.x, "🦁");
        break;
    case (Cow):
        mvprintw(hero.location.y, hero.location.x, "🐮");
        break;
    case (Pig):
        mvprintw(hero.location.y, hero.location.x, "🐷");
        break;
    case (Frog):
        mvprintw(hero.location.y, hero.location.x, "🐸");
        break;
    case (Wolf):
        mvprintw(hero.location.y, hero.location.x, "🐺");
        break;
    case (Unicorn):
        mvprintw(hero.location.y, hero.location.x, "🦄");
        break;
    case (Raccoon):
        mvprintw(hero.location.y, hero.location.x, "🦝");
        break;
    case (Whale):
        mvprintw(hero.location.y, hero.location.x, "🐋");
        break;
    case (Snake):
        mvprintw(hero.location.y, hero.location.x, "🐍");
        break;
    case (Monkey):
        mvprintw(hero.location.y, hero.location.x, "🐵");
        break;
    }
}

void handle_move_command(int level_num, int input)
{
    Point origin, destination;
    origin = hero.location;
    destination = hero.location;

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
                visibility_grid[level][i][j] = 1;
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

            result.y += (rand() % ((room.width - 2) / 2) * 2) + 1;
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
        // if (level[level_num].objects[i].location_room == hero.location_room)
        level[level_num].objects[i].visible = 1;
        // else
        //     level[level_num].objects[i].visible = 0;
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
            break;
        }
    }

    if (object_index == -1)
        return -1;

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
        added_coin = (rand() % (MAX_COIN_POINT - MIN_COIN_POINT + 1)) + MIN_COIN_POINT;
        hero.coins += added_coin;
        remove_object(level_num, object_index);
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
        added_coin = 30;
        hero.coins += added_coin;
        remove_object(level_num, object_index);
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

void print_info(int level_num)
{
    mvprintw(LINES - 2, (COLS / 5) - 5, "Level: %d", level_num);
    mvprintw(LINES - 2, 2 * (COLS / 5) - 5, "Health: %d", hero.health);
    mvprintw(LINES - 2, 3 * (COLS / 5) - 5, "Satiety: %d", hero.satiety);
    mvprintw(LINES - 2, 4 * (COLS / 5) - 5, "Stars: %d", hero.coins);
}

void print_message(char *message, char *guide)
{
    mvprintw(2, MARGIN + 5, "%s ", message);
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

void show_food_inventory()
{
    move(1, 0);
    clrtoeol();
    move(2, 0);
    clrtoeol();
    move(3, 0);
    clrtoeol();
    refresh();

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
    mvprintw(1, MARGIN + 5, "Food Inventory (Press '1' to close memu.)");
    mvprintw(3, MARGIN + 5, "Normal Food (🍎): %d", food);
    mvprintw(3, MARGIN + 5 + (COLS / 4), "Gourmet Food (🍔): %d", gourmet);
    mvprintw(3, MARGIN + 5 + ((2 * (COLS / 4))), "Magic Food (🍺): %d", magic);
    refresh();
    while (1)
    {
        int temp = getch();
        if (temp == '1')
            break;
    }
    move(1, 0);
    clrtoeol();
    refresh();
    move(3, 0);
    clrtoeol();
    refresh();
}

void show_weapon_inventory()
{
    move(1, 0);
    clrtoeol();
    move(2, 0);
    clrtoeol();
    move(3, 0);
    clrtoeol();
    refresh();

    int mace = 0, dagger = 0, wand = 0, arrow = 0, sword = 0;
    for (int i = 0; i < hero.weapon_num; i++)
    {
        switch (hero.weapon_inventory[i].type)
        {
        case (Mace):
            mace += 1;
            break;
        case (Dagger):
            dagger += 10;
            break;
        case (MagicWand):
            wand += 8;
            break;
        case (NormalArrow):
            arrow += 20;
            break;
        case (Sword):
            sword += 1;
            break;
        }
    }
    mvprintw(1, MARGIN + 5, "Weapon Inventory (Press '2' to close menu.)");
    mvprintw(3, MARGIN + 5, "Mace (🪓): %d", mace);
    mvprintw(3, MARGIN + 5 + (COLS / 6), "Dagger (🔪): %d", dagger);
    mvprintw(3, MARGIN + 5 + ((2 * (COLS / 6))), "Magic Wand (✨): %d", wand);
    mvprintw(3, MARGIN + 5 + ((3 * (COLS / 6))), "Normal Arrow (🏹): %d", arrow);
    mvprintw(3, MARGIN + 5 + ((4 * (COLS / 6))), "Gun (🔫): %d", sword);

    refresh();
    while (1)
    {
        int temp = getch();
        if (temp == '2')
            break;
    }
    move(1, 0);
    clrtoeol();
    refresh();
    move(3, 0);
    clrtoeol();
    refresh();
}

void show_spell_inventory()
{
    move(1, 0);
    clrtoeol();
    move(2, 0);
    clrtoeol();
    move(3, 0);
    clrtoeol();
    refresh();

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
    mvprintw(1, MARGIN + 5, "Spell Inventory (Press '3' to close memu.)");
    mvprintw(3, MARGIN + 5, "Health Spell (💊): %d", health);
    mvprintw(3, MARGIN + 5 + (COLS / 4), "Speed Spell (💥): %d", speed);
    mvprintw(3, MARGIN + 5 + ((2 * (COLS / 4))), "Damage Spell (🔥): %d", damage);
    refresh();
    while (1)
    {
        int temp = getch();
        if (temp == '3')
            break;
    }
    move(1, 0);
    clrtoeol();
    refresh();
    move(3, 0);
    clrtoeol();
    refresh();
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
