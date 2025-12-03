#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

// kitchen semaphores
typedef struct {
    sem_t pantry_sem;
    sem_t fridge_sem;
    sem_t mixer_sem;
    sem_t bowl_sem;
    sem_t spoon_sem;
    sem_t oven_sem;
} Kitchen;

Kitchen kitchen;

typedef enum {
    // Pantry
    ING_FLOUR,
    ING_SUGAR,
    ING_YEAST,
    ING_BAKING_SODA,
    ING_SALT,
    ING_CINNAMON,

    // Fridge
    ING_MILK,
    ING_BUTTER,
    ING_EGG,

    ING_COUNT  // total number of ingredients
} Ingredient;

// Track which ingredients the player has
bool hasIngredient[ING_COUNT] = { false };

// ingredient name lookup table
const char* ingredientNames[] = {
    "Flour",
    "Sugar",
    "Yeast",
    "Baking Soda",
    "Salt",
    "Cinnamon",
    "Milk",
    "Butter",
    "Egg"
};

// Recipes
typedef struct {
    const char *name;
    Ingredient *list;
    int count;
} Recipe;

Ingredient cookies[] = {
    ING_FLOUR, ING_SUGAR, ING_MILK, ING_BUTTER
};
int cookiesCount = 4;

Ingredient pancakes[] = {
    ING_FLOUR, ING_SUGAR, ING_BAKING_SODA, ING_SALT,
    ING_EGG, ING_MILK, ING_BUTTER
};
int pancakesCount = 7;

Ingredient pizzaDough[] = {
    ING_YEAST, ING_SUGAR, ING_SALT
};
int pizzaDoughCount = 3;

Ingredient softPretzels[] = {
    ING_FLOUR, ING_SUGAR, ING_SALT,
    ING_YEAST, ING_BAKING_SODA, ING_EGG
};
int softPretzelsCount = 6;

Ingredient cinnamonRolls[] = {
    ING_FLOUR, ING_SUGAR, ING_SALT,
    ING_BUTTER, ING_EGG, ING_CINNAMON
};
int cinnamonRollsCount = 6;

Recipe recipes[] = {
    {"Cookies", cookies, sizeof(cookies)/sizeof(*cookies)},
    {"Pancakes", pancakes, sizeof(pancakes)/sizeof(*pancakes)},
    {"Pizza Dough", pizza, sizeof(pizza)/sizeof(*pizza)},
    {"Soft Pretzels", pretzels, sizeof(pretzels)/sizeof(*pretzels)},
    {"Cinnamon Rolls", cinnamon_rolls, sizeof(cinnamon_rolls)/sizeof(*cinnamon_rolls)}
};

const int RECIPE_COUNT = sizeof(recipes)/sizeof(recipes[0]);

// colored output
const char *colors[] = {
    "\033[31m","\033[32m","\033[33m","\033[34m","\033[35m","\033[36m","\033[37m"
};
const int COLOR_COUNT = sizeof(colors)/sizeof(colors[0]);

pthread_mutex_t print_lock = PTHREAD_MUTEX_INITIALIZER;

// ramsied logic
int ramsied_baker = -1;
int ramsied_done = 0;

void colored_printf(const char *color, const char *fmt, ...) {
    va_list ap;
    pthread_mutex_lock(&print_lock);
    va_start(ap, fmt);
    printf("%s", color);
    vprintf(fmt, ap);
    printf("\033[0m");
    va_end(ap);
    pthread_mutex_unlock(&print_lock);
}

// code for bakers
typedef struct {
    int id;
    const char *color;
} Baker;

bool isPantry(Ingredient i) { return i <= ING_CINNAMON; }
bool isFridge(Ingredient i) { return i >= ING_MILK; }

void releaseTools(int pantry, int fridge, int bowl, int spoon, int mixer, int oven) {
    if (mixer) sem_post(&kitchen.mixer_sem);
    if (spoon) sem_post(&kitchen.spoon_sem);
    if (bowl) sem_post(&kitchen.bowl_sem);
    if (oven) sem_post(&kitchen.oven_sem);
    if (fridge) sem_post(&kitchen.fridge_sem);
    if (pantry) sem_post(&kitchen.pantry_sem);
}

void *baker_thread(void *arg) {
    Baker *b = (Baker*)arg;

    for (int r = 0; r < RECIPE_COUNT; r++) {
        Recipe *rec = &recipes[r];
        int completed = 0, attempt = 0;

        while (!completed) {
            attempt++;
            int held_pantry=0, held_fridge=0, held_bowl=0, held_spoon=0, held_mixer=0, held_oven=0;

            colored_printf(b->color, "Baker %d: Starting attempt %d for '%s'\n", b->id, attempt, rec->name);

            // Acquire pantry/fridge as needed
            for (int i = 0; i < rec->count; i++) {
                Ingredient ing = rec->list[i];
                if (isPantry(ing)) { sem_wait(&kitchen.pantry_sem); held_pantry=1; }
                if (isFridge(ing)) { sem_wait(&kitchen.fridge_sem); held_fridge=1; }

                colored_printf(b->color, "Baker %d: Taking ingredient '%s'\n", b->id, ingredientNames[ing]);

                if (isPantry(ing)) { sem_post(&kitchen.pantry_sem); held_pantry=0; }
                if (isFridge(ing)) { sem_post(&kitchen.fridge_sem); held_fridge=0; }
            }

            // Acquire mixing tools
            sem_wait(&kitchen.bowl_sem); held_bowl=1;
            sem_wait(&kitchen.spoon_sem); held_spoon=1;
            sem_wait(&kitchen.mixer_sem); held_mixer=1;

            colored_printf(b->color,"Baker %d: Mixing '%s'\n",b->id,rec->name);

            // Ramsied logic
            if (b->id == ramsied_baker && !ramsied_done) {
                colored_printf(b->color,"Baker %d: ***RAMSIED*** Restarting '%s'\n",b->id,rec->name);
                releaseTools(held_pantry,held_fridge,held_bowl,held_spoon,held_mixer,held_oven);
                held_pantry=held_fridge=held_bowl=held_spoon=held_mixer=held_oven=0;
                ramsied_done=1;
                continue;
            }

            sem_post(&kitchen.mixer_sem); held_mixer=0;
            sem_post(&kitchen.spoon_sem); held_spoon=0;
            sem_post(&kitchen.bowl_sem); held_bowl=0;

            // Bake
            sem_wait(&kitchen.oven_sem); held_oven=1;
            colored_printf(b->color,"Baker %d: Baking '%s'\n",b->id,rec->name);
            sem_post(&kitchen.oven_sem); held_oven=0;

            colored_printf(b->color,"Baker %d: Finished '%s'\n",b->id,rec->name);
            completed=1;
        }
    }

    colored_printf(b->color,"Baker %d: Completed all recipes!\n",b->id);
    return NULL;
}

int main() {
    srand(time(NULL));

    // initialize semaphores with number of available for each
    sem_init(&kitchen.pantry_sem, 0, 1);
    sem_init(&kitchen.fridge_sem, 0, 2);
    sem_init(&kitchen.mixer_sem, 0, 2);
    sem_init(&kitchen.bowl_sem, 0, 3);
    sem_init(&kitchen.spoon_sem, 0, 5);
    sem_init(&kitchen.oven_sem, 0, 1);

    int baker_count;
    printf("Enter number of bakers: ");
    if (scanf("%d",&baker_count)!=1 || baker_count<=0) return 1;

    ramsied_baker = rand()%baker_count;
    printf("Baker %d will be Ramsied once.\n",ramsied_baker);

    pthread_t threads[baker_count];
    Baker bakers[baker_count];

    for(int i=0;i<baker_count;i++){
        bakers[i].id=i;
        bakers[i].color=colors[i%COLOR_COUNT];
        pthread_create(&threads[i],NULL,baker_thread,&bakers[i]);
    }

    for(int i=0;i<baker_count;i++) pthread_join(threads[i],NULL);

    sem_destroy(&kitchen.pantry_sem);
    sem_destroy(&kitchen.fridge_sem);
    sem_destroy(&kitchen.mixer_sem);
    sem_destroy(&kitchen.bowl_sem);
    sem_destroy(&kitchen.spoon_sem);
    sem_destroy(&kitchen.oven_sem);

    printf("All bakers finished.\n");
    
    return 0;
}
=======
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
