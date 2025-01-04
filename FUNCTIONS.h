#ifndef FUNCTIONS_H
#define FUNCTIONS_H

void draw_border();

void title_screen();

void generate_map();
void print_level(int level_num);
void print_room(int level_num, int room_num);

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

#endif