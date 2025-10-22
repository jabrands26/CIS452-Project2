// Copied from Project 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

const char *pantry_items[] = {
    "Flour",
    "Sugar",
    "Yeast",
    "Baking Soda",
    "Salt",
    "Cinnamon"
};
const int PANTRY_COUNT = 6;



const char *fridge_items[] = {
    "Egg",
    "Milk",
    "Butter"
};
const int FRIDGE_COUNT = 3;
