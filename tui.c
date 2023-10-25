/*
 * Race for the Galaxy AI
 *
 * Copyright (C) 2009-2011 Keldon Jones
 *
 * Source file modified by B. Nordli, August 2014.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "rftg.h"
#include "tui.h"


/* 
* Discard cards, partially inspired by ChatGPT.
*/
// Constants for better clarity
#define MIN_SELECTION 1

void display_cards(game *g, int list[], int num, const char *message) {
    card *c_ptr;
    printf("%s\n", message);
    for (int i = 0; i < num; i++) {
        c_ptr = &g->deck[list[i]];
        printf("%d. %s\n", i + 1, c_ptr->d_ptr->name);
    }
}

// Dummy function to display card details
void display_card_info(game *g, int card_index) {
    card *c_ptr = &g->deck[card_index];
    design *d_ptr = c_ptr->d_ptr;

    // Display details about the card
    printf("---- Details about %s ----\n", d_ptr->name);
    
    // Display card type
    if (d_ptr->type == TYPE_WORLD)
        printf("Type: World\n");
    else if (d_ptr->type == TYPE_DEVELOPMENT)
        printf("Type: Development\n");
    else
        printf("Type: Unknown\n");
    
    printf("Cost: %d\n", d_ptr->cost);
    printf("VP: %d\n", d_ptr->vp);

    // Display card powers
    for (int i = 0; i < d_ptr->num_power; i++) {
        char *power_name = get_card_power_name(card_index, i);
        printf("Power %d: %s\n", i + 1, power_name);
    }
    
    printf("----------------------------\n\n");
}

int get_card_choice(game *g, int list[], int num, const char *prompt) {
    char action[10];
    int selected_card;

    while (1) {
        printf("%s (or 'i' followed by number for info, e.g., i2, 'q' to quit, 'h' for help, 'r' to redisplay list): ", prompt);
        scanf("%s", action);

        // Info command
        if (action[0] == 'i' || strncmp(action, "info", 4) == 0) {
            if (sscanf(action + 1, "%d", &selected_card) == 1 || sscanf(action + 4, "%d", &selected_card) == 1) {
                if (selected_card >= 1 && selected_card <= num) {
                    display_card_info(g, list[selected_card - 1]);
                } else {
                    printf("Invalid card number. Please try again.\n");
                }
            } else {
                printf("Invalid format. Please try again.\n");
            }

        // Quit command
        } else if (action[0] == 'q' || strncmp(action, "quit", 4) == 0) {
            printf("Quitting...\n");
            exit(0);

        // Help command
        } else if (action[0] == 'h' || strncmp(action, "help", 4) == 0) {
            printf("Help: Enter a card number to choose, 'i' followed by number for info, 'q' to quit, 'r' to redisplay list.\n");

        // Redisplay command
        } else if (action[0] == 'r' || strncmp(action, "redisplay", 9) == 0) {
            display_cards(g, list, num, prompt);

        // Card selection
        } else if (sscanf(action, "%d", &selected_card) == 1) {
            if (selected_card >= 1 && selected_card <= num) {
                return selected_card;
            } else {
                printf("Invalid selection. Please try again.\n");
            }

        // Invalid command
        } else {
            printf("Invalid input. Please try again or enter 'h' for help.\n");
        }
    }
}



void gui_choose_discard(game *g, int who, int list[], int *num, int discard) {
    int discard_count = 0;
    int temp_list[*num];  // Temporary list to hold indices of cards not yet discarded

    // Initially, temp_list is a copy of the original list
    for (int i = 0; i < *num; i++) {
        temp_list[i] = list[i];
    }

    display_cards(g, temp_list, *num - discard_count, "You need to discard. Here are your options:");

    while (discard_count < discard) {
        int selected_card = get_card_choice(g, temp_list, *num - discard_count, "Enter card number to discard");
        
        // Add the selected card to the list of discarded cards
        list[discard_count] = temp_list[selected_card - 1];
        
        // Remove the discarded card from temp_list by shifting all subsequent cards
        for (int i = selected_card - 1; i < *num - discard_count - 1; i++) {
            temp_list[i] = temp_list[i + 1];
        }
        
        discard_count++;

        if (discard_count < discard) {
            display_cards(g, temp_list, *num - discard_count, "Remaining options:");
        }
    }

    // Update num to reflect the number of cards discarded.
    *num = discard_count;
}

