#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <unistd.h> // For sleep

void clean_up(Mix_Chunk *chunk1, Mix_Chunk *chunk2)
{
    Mix_FreeChunk(chunk1);
    Mix_FreeChunk(chunk2);
    Mix_CloseAudio();
    SDL_Quit();
}

int main(int argc, char *argv[])
{
    const char *sound_file_1 = "music/cults-BadThings.wav";        // Relative path to the sound file
    const char *sound_file_2 = "music/TaylorSwift-Florida!!!.wav"; // Relative path to the sound file

    if (SDL_Init(SDL_INIT_AUDIO) < 0)
    {
        fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        fprintf(stderr, "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
        SDL_Quit();
        return 1;
    }

    Mix_Chunk *sound1 = Mix_LoadWAV(sound_file_1);
    Mix_Chunk *sound2 = Mix_LoadWAV(sound_file_2);

    if (sound1 == NULL)
    {
        fprintf(stderr, "Failed to load sound1! SDL_mixer Error: %s\n", Mix_GetError());
    }

    if (sound2 == NULL)
    {
        fprintf(stderr, "Failed to load sound2! SDL_mixer Error: %s\n", Mix_GetError());
    }

    if (sound1 != NULL && sound2 != NULL)
    {
        Mix_PlayChannel(-1, sound1, 0); // Play sound1 on any available channel
        Mix_PlayChannel(-1, sound2, 0); // Play sound2 on any available channel
    }

    printf("Playing sounds: %s and %s\n", sound_file_1, sound_file_2);
    printf("Press Enter to stop the sounds...\n");
    getchar();

    clean_up(sound1, sound2);
    return 0;
}