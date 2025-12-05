# CIS452-Project2 - Baking Project
Names: James Brands and Josh Vink

# Overview
This project simulates a shared kitchen space, where multiple bakers operate at the same time. Each baker runs in its own thread and competes for the shared resources. Semaphores are used to safely synchronize access between the bakers as they gather resources for their recipes.

# Design Features / Key Concepts
- User chooses the number of bakers (and each baker = one thread)
- Shared kitchen with limited resources controlled by the semaphores
- each baker produces all recipes required (Cookies, Pancakes, Pizza Dough, Soft Pretzels, Cinnamon Rolls)
- Bakers announce their actions in real time with different text colors
- A baker may be randomly "Ramsied" (forced to quit and restart their current recipe)

# Implementation
 - Semaphores protect each of the shared resources
 - Only one baker may enter the pantry or fridge at a time
 - Each baker must collect a bowl, a spoon, and a mixer
 - Each baker completed all recipes once.
 - Color-coded terminal outputs for each of the bakers
 - A single baker is chosen to be ramsied, causing them to release all semaphores and restart their current recipe.
 - Threads run continuously until all recipes are completed

# How to Run
1. Compile: gcc -o getting-ramsied getting-ramsied.c
2. Run: ./getting-ramsied
3. Enter the number of bakers
4. Watch the bakers create their recipes
5. Program ends once all bakers have completed their recipes.

Run using gcc in a Linux compiler (created with Ubuntu)

