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

int get_card_choice(game *g, int list[], int num, const char *prompt, int allow_multiple) {
    char action[100];  // Increased size to account for multiple selections
    int selected_cards[num], selected_count = 0;  // Array to store multiple selections
    int card_index, confirm;

    while (1) {
        if (allow_multiple) {
            printf("%s (or 'i' followed by number for info, e.g., i2, 'q' to quit, 'h' for help, 'r' to redisplay list, or provide a comma-separated list for multiple selections): ", prompt);
        } else {
            printf("%s (or 'i' followed by number for info, e.g., i2, 'q' to quit, 'h' for help, 'r' to redisplay list): ", prompt);
        }
        
        scanf("%99s", action);  // Avoid buffer overflow

        // Info command
        if (action[0] == 'i' || strncmp(action, "info", 4) == 0) {
            if (sscanf(action + 1, "%d", &card_index) == 1 || sscanf(action + 4, "%d", &card_index) == 1) {
                if (card_index >= 1 && card_index <= num) {
                    display_card_info(g, list[card_index - 1]);
                }
            }
        } else if (allow_multiple && strchr(action, ',') != NULL) {  // Check for multiple selections
            char *token = strtok(action, ",");
            while (token != NULL) {
                sscanf(token, "%d", &card_index);
                if (card_index >= 1 && card_index <= num) {
                    selected_cards[selected_count++] = card_index;
                }
                token = strtok(NULL, ",");
            }

            // Display selected cards for confirmation
            printf("You've selected cards: ");
            for (int i = 0; i < selected_count; i++) {
                printf("%d ", selected_cards[i]);
            }
            printf("\\nConfirm? (1 for Yes, 0 for No): ");
            scanf("%d", &confirm);

            if (confirm == 1) {
                return selected_count;  // Return number of selected cards
            } else {
                selected_count = 0;  // Reset selection count
            }
        } else {
            sscanf(action, "%d", &card_index);
            if (card_index >= 1 && card_index <= num) {
                return card_index;  // Return the selected card index
            }
        }
        // ... (rest of the function for other commands like 'q', 'h', 'r')
    }
    return -1;  // Return -1 if the user quits without selection
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
        int selected_card = get_card_choice(g, temp_list, *num - discard_count, "Enter card number to discard", 1);
        
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

void gui_choose_action(game *g, int who, int action[2], int one) {
    int selected_action;
    int num_actions = ACT_ROUND_END + 1;  // Based on your constants, ACT_ROUND_END is the last action.
    int available_actions[num_actions];   // To store indices of actions that are available.
    int num_available_actions = 0;       // Count of available actions.

    // Check for advanced game
    if (g->advanced) {
        // Call advanced function (to be implemented later)
        // return tui_choose_action_advanced(g, who, action, one);
    }

    // Populate the available actions list and display them.
    for (int i = 0; i < num_actions; i++) {
        // Skip the ACT_SEARCH action under certain conditions
        if (i == ACT_SEARCH && (g->expanded != 3 || g->p[who].prestige_action_used)) {
            continue;
        }

        // Skip ACT_DEVELOP2 and ACT_SETTLE2
        if (i == ACT_DEVELOP2 || i == ACT_SETTLE2) {
            continue;
        }

        available_actions[num_available_actions++] = i;
        printf("%d. %s\n", num_available_actions, action_name(i));
    }

    while (1) {
        printf("Enter action number (or 'i' followed by number for info, 'q' to quit, 'h' for help, 'r' to redisplay list): ");
        char input[10];
        scanf("%s", input);

        if (input[0] == 'i') {
            if (sscanf(input + 1, "%d", &selected_action) == 1) {
                if (selected_action >= 1 && selected_action <= num_available_actions) {
                    // Display action info - this function needs to be implemented based on your game's requirements.
                    // display_action_info(g, available_actions[selected_action - 1]);
                } else {
                    printf("Invalid action number. Please try again.\n");
                }
            } else {
                printf("Invalid format. Please try again.\n");
            }
        } else if (input[0] == 'q') {
            printf("Quitting...\n");
            exit(0);
        } else if (input[0] == 'h') {
            printf("Help: Enter an action number to choose, 'i' followed by number for info, 'q' to quit, 'r' to redisplay list.\n");
        } else if (input[0] == 'r') {
            // Redisplay available actions
            for (int i = 0; i < num_available_actions; i++) {
                printf("%d. %s\n", i + 1, action_name(available_actions[i]));
            }
        } else if (sscanf(input, "%d", &selected_action) == 1) {
            if (selected_action >= 1 && selected_action <= num_available_actions) {
                action[0] = available_actions[selected_action - 1];
                action[1] = -1;
                return; // Exit the function once the action is selected.
            } else {
                printf("Invalid selection. Please try again.\n");
            }
        } else {
            printf("Invalid input. Please try again or enter 'h' for help.\n");
        }
    }
}


int tui_choose_place(game *g, int who, int list[], int num, int phase, int special) {
    int choice;

    // Display the list of cards using the display_cards function
    display_cards(g, list, num, "Choose a card to play:");

    // Get user choice
    do {
        printf("Enter the number of the card you want to play (1-%d) or 0 to pass: ", num);
        scanf("%d", &choice);

        // If the user chooses to pass
        if (choice == 0) {
            return -1;
        }

    } while (choice < 1 || choice > num); // Ensure valid choice

    return list[choice - 1]; // Return the card index from the list
}
