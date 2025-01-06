#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdbool.h>
#include "STRUCTS.h"

int find_user_index(char username[26]);
char *data_from_username();
char *data_from_email();

void draw_border();

void title_screen();

void random_map();
void random_level(int level_num);
void random_room(int y_min, int y_max, int x_min, int x_max, Room *room);
void print_level(int level_num);
void print_room(int level_num, int room_num);
void save_user_room(char username[26], int level, int room);
void load_user_map(char username[26]);

void draw_welcome_screen_border();
int welcome_screen();

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

void draw_sign_in_screen(int option, int show_password);
void continue_game_screen();

int username_found(char username[26]);
int email_found(char email[29]);
bool password_correct(int user_index, char password[26]);

void print_user_not_found();
void print_email_not_found();
void print_wrong_password();

void draw_user_options_menu();
int user_options_menu();

void draw_user_settings_menu();
void user_settings_menu();

#endif