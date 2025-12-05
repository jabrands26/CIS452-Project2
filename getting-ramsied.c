#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <stdarg.h>
#include <stdbool.h>

//kitchen semaphores
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
    //Pantry
    ING_FLOUR,
    ING_SUGAR,
    ING_YEAST,
    ING_BAKING_SODA,
    ING_SALT,
    ING_CINNAMON,

    //Fridge
    ING_MILK,
    ING_BUTTER,
    ING_EGG,

    ING_COUNT
} Ingredient;

//ingredient name lookup table
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

bool isPantry(Ingredient i) { return i <= ING_CINNAMON; }
bool isFridge(Ingredient i) { return i >= ING_MILK; }

//Recipes
typedef struct {
    const char *name;
    Ingredient *list;
    int count;
} Recipe;

Ingredient cookies[] = {ING_FLOUR, ING_SUGAR, ING_MILK, ING_BUTTER};

Ingredient pancakes[] = {ING_FLOUR, ING_SUGAR, ING_BAKING_SODA, ING_SALT, ING_EGG, ING_MILK, ING_BUTTER};

Ingredient pizzaDough[] = {ING_YEAST, ING_SUGAR, ING_SALT};

Ingredient softPretzels[] = {ING_FLOUR, ING_SUGAR, ING_SALT, ING_YEAST, ING_BAKING_SODA, ING_EGG};

Ingredient cinnamonRolls[] = {ING_FLOUR, ING_SUGAR, ING_SALT, ING_BUTTER, ING_EGG, ING_CINNAMON};

Recipe recipes[] = {
    {"Cookies", cookies, sizeof(cookies)/sizeof(*cookies)},
    {"Pancakes", pancakes, sizeof(pancakes)/sizeof(*pancakes)},
    {"Pizza Dough", pizzaDough, sizeof(pizzaDough)/sizeof(*pizzaDough)},
    {"Soft Pretzels", softPretzels, sizeof(softPretzels)/sizeof(*softPretzels)},
    {"Cinnamon Rolls", cinnamonRolls, sizeof(cinnamonRolls)/sizeof(*cinnamonRolls)}
};

#define RECIPE_COUNT 5

//color
const char *colors[] = {
    "\033[31m","\033[32m","\033[33m","\033[34m","\033[35m","\033[36m","\033[37m"
};
const int COLOR_COUNT = sizeof(colors)/sizeof(colors[0]);
pthread_mutex_t print_lock = PTHREAD_MUTEX_INITIALIZER;

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

typedef struct {
    int id;
    const char *color;
    int order[RECIPE_COUNT];  // shuffled recipe order
} Baker;

//randomize recipe order
void shuffle(int *array, int size) {
    for (int i = size - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
}

int ramsied_baker = -1;
int ramsied_done = 0;

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
        Recipe *rec = &recipes[b->order[r]];
        int completed = 0, attempt = 0;

        while (!completed) {
            attempt++;
            int held_pantry=0, held_fridge=0, held_bowl=0, held_spoon=0, held_mixer=0, held_oven=0;

            colored_printf(b->color, "Baker %d: Starting attempt %d for '%s'\n", b->id, attempt, rec->name);

            for (int i = 0; i < rec->count; i++) {
                Ingredient ing = rec->list[i];
                if (isPantry(ing)) { sem_wait(&kitchen.pantry_sem); held_pantry=1; }
                if (isFridge(ing)) { sem_wait(&kitchen.fridge_sem); held_fridge=1; }

                colored_printf(b->color, "Baker %d: Taking ingredient '%s'\n", b->id, ingredientNames[ing]);
                usleep(1000);

                if (isPantry(ing)) { sem_post(&kitchen.pantry_sem); held_pantry=0; }
                if (isFridge(ing)) { sem_post(&kitchen.fridge_sem); held_fridge=0; }           
            }

            sem_wait(&kitchen.bowl_sem);
            colored_printf(b->color,"Baker %d: Using bowl\n",b->id);
            usleep(1000);
            sem_post(&kitchen.bowl_sem);
            sem_wait(&kitchen.spoon_sem);
            colored_printf(b->color,"Baker %d: Using spoon\n",b->id);
            usleep(1000);
            sem_post(&kitchen.spoon_sem);
            sem_wait(&kitchen.mixer_sem);
            colored_printf(b->color,"Baker %d: Using mixer\n",b->id);
            usleep(1000);
            sem_post(&kitchen.mixer_sem);
            colored_printf(b->color,"Baker %d: Mixing '%s'\n", b->id, rec->name);

            if (b->id == ramsied_baker && !ramsied_done) {
                colored_printf(b->color,"Baker %d: ***RAMSIED*** Restarting '%s'\n",b->id,rec->name);
                releaseTools(held_pantry,held_fridge,held_bowl,held_spoon,held_mixer,held_oven);
                held_pantry=held_fridge=held_bowl=held_spoon=held_mixer=held_oven=0;
                ramsied_done=1;
                continue;
            }

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

    sem_init(&kitchen.pantry_sem, 0, 1);
    sem_init(&kitchen.fridge_sem, 0, 2);
    sem_init(&kitchen.mixer_sem, 0, 2);
    sem_init(&kitchen.bowl_sem, 0, 3);
    sem_init(&kitchen.spoon_sem, 0, 5);
    sem_init(&kitchen.oven_sem, 0, 1);

    int baker_count;
    printf("Enter number of bakers: ");
    scanf("%d", &baker_count);

    ramsied_baker = rand() % baker_count;
    printf("Baker %d will be Ramsied once.\n",ramsied_baker);

    pthread_t threads[baker_count];
    Baker bakers[baker_count];

    for(int i = 0; i < baker_count; i++){
    bakers[i].id = i;
    bakers[i].color = colors[i % COLOR_COUNT];

    for(int r = 0; r < RECIPE_COUNT; r++)
        bakers[i].order[r] = r;

    shuffle(bakers[i].order, RECIPE_COUNT);

    pthread_create(&threads[i], NULL, baker_thread, &bakers[i]);
}

    for(int i = 0; i < baker_count; i++)
        pthread_join(threads[i], NULL);

    sem_destroy(&kitchen.pantry_sem);
    sem_destroy(&kitchen.fridge_sem);
    sem_destroy(&kitchen.mixer_sem);
    sem_destroy(&kitchen.bowl_sem);
    sem_destroy(&kitchen.spoon_sem);
    sem_destroy(&kitchen.oven_sem);

    printf("All bakers finished.\n");
    
    return 0;
}
