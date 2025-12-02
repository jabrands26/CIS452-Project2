// Copied from Project 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>

#define MAX_BAKERS 20

//Kitchen Recources
sem_t sem_pantry; // Only 1 baker in pantry at a time
sem_t sem_fridge_slots; // 2 fridges -> 2 bakers at once
sem_t sem_mixer; // 2 mixers
sem_t sem_bowl;// 3 bowls
sem_t sem_spoon;// 5 spoons
sem_t sem_oven;// 1 oven

const char *pantry_items[] = {
    "Flour", //0
    "Sugar", //1
    "Yeast", //2
    "Baking Soda", //3
    "Salt", //4
    "Cinnamon" //5
};
const int PANTRY_COUNT = 6;



const char *fridge_items[] = {
    "Egg", //0
    "Milk", //1
    "Butter" //2
};
const int FRIDGE_COUNT = 3;

//enum-like indeces
enum PantryIndex {
    P_FLOUR = 0,
    P_SUGAR = 1,
    P_YEAST = 2,
    P_BAKING_SODA = 3,
    P_SALT = 4,
    P_CINNAMON = 5
};

enum FridgeIndex {
    F_EGG = 0,
    F_MILK = 1,
    F_BUTTER = 2
};

// All recipes
Recipe recipes[] = {
    // Cookies: Flour, Sugar, Milk, Butter
    {
        .name = "Cookies",
        .pantry_need = { P_FLOUR, P_SUGAR },
        .pantry_count = 2,
        .fridge_need = { F_MILK, F_BUTTER },
        .fridge_count = 2
    },
    // Pancakes: Flour, Sugar, Baking soda, Salt, Egg, Milk, Butter
    {
        .name = "Pancakes",
        .pantry_need = { P_FLOUR, P_SUGAR, P_BAKING_SODA, P_SALT },
        .pantry_count = 4,
        .fridge_need = { F_EGG, F_MILK, F_BUTTER },
        .fridge_count = 3
    },
    // Homemade pizza dough: Yeast, Sugar, Salt
    {
        .name = "Pizza Dough",
        .pantry_need = { P_YEAST, P_SUGAR, P_SALT },
        .pantry_count = 3,
        .fridge_need = { },
        .fridge_count = 0
    },
    // Soft Pretzels: Flour, Sugar, Salt, Yeast, Baking Soda, Egg
    {
        .name = "Soft Pretzels",
        .pantry_need = { P_FLOUR, P_SUGAR, P_SALT, P_YEAST, P_BAKING_SODA },
        .pantry_count = 5,
        .fridge_need = { F_EGG },
        .fridge_count = 1
    },
    // Cinnamon rolls: Flour, Sugar, Salt, Butter, Eggs, Cinnamon
    {
        .name = "Cinnamon Rolls",
        .pantry_need = { P_FLOUR, P_SUGAR, P_SALT, P_CINNAMON },
        .pantry_count = 4,
        .fridge_need = { F_BUTTER, F_EGG },
        .fridge_count = 2
    }
};
const int RECIPE_COUNT = 5;

