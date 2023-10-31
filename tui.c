/*
 * Race for the Galaxy AI
 *
 * Copyright (C) 2009-2011 Keldon Jones
 *
 * Source file modified by B. Nordli, August 2014.
 * Text Interface by Zachary Kline, 2023.
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

// Define a struct to hold flag and its description
typedef struct {
    unsigned int flag;
    const char *description;
} FlagDescriptor;

// Create a static array of flag descriptors
static FlagDescriptor flag_descriptions[] = {
    {FLAG_MILITARY, "Military"},
    {FLAG_WINDFALL, "Windfall"},
    {FLAG_START, "Start"},
    {FLAG_START_RED, "Start Red"},
    {FLAG_START_BLUE, "Start Blue"},
    {FLAG_PROMO, "Promo"},
    {FLAG_REBEL, "Rebel"},
  {FLAG_IMPERIUM, "Imperium"},
    {FLAG_ALIEN, "Alien"},
    {FLAG_UPLIFT, "Uplift"},
    // ... add other flags as needed
};

// Function to display card flags
void display_card_flags(unsigned int flags) {
    printf("Flags: ");
    for (size_t i = 0; i < sizeof(flag_descriptions) / sizeof(flag_descriptions[0]); i++) {
        if (flags & flag_descriptions[i].flag) {
            printf("%s ", flag_descriptions[i].description);
        }
    }
    printf("\n");
}



/* 
* Discard cards, inspired by ChatGPT.
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

// Display card details
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
    if (d_ptr)
    printf("Cost: %d\n", d_ptr->cost);
    printf("VP: %d\n", d_ptr->vp);
switch (d_ptr->good_type)
{
case GOOD_ALIEN:
    printf("Good Type: Alien\n");
    break;
case GOOD_NOVELTY:
    printf("Good Type: Novelty\n");
    break;
    case GOOD_RARE:
    printf("Good Type: Rare\n");
    break;
    case GOOD_GENE:
    printf("Good Type: Genes\n");
    break;
}
display_card_flags(d_ptr->flags);

    // Display card powers
    for (int i = 0; i < d_ptr->num_power; i++) {
        char *power_name = get_card_power_name(card_index, i);
        printf("Power %d: %s\n", i + 1, power_name);
        free(power_name);
    }
    
    printf("----------------------------\n\n");
}

int get_card_choice(game *g, int who, int list[], int num, const char *prompt) {
    char action[10];
    int selected_card;

    while (1) {
        printf("%s (or '?' for help): ", prompt);
        fgets(action, sizeof(action), stdin);
        action[strcspn(action, "\n")] = 0;

        // Validate input length and check for control characters
        if (strlen(action) >= sizeof(action) - 1) {
            printf("Input too long! Please try again.\n");
            continue;
        }

        for (int i = 0; i < strlen(action); i++) {
            if (iscntrl(action[i])) {
                printf("Invalid input! Control characters are not allowed.\n");
                continue;
            }
        }

        switch (action[0]) {
            case 'i':
                if (sscanf(action + 1, "%d", &selected_card) == 1) {
                    if (selected_card >= 1 && selected_card <= num) {
                        display_card_info(g, list[selected_card - 1]);
                    } else {
                        printf("Invalid card number. Please try again.\n");
                    }
                } else {
                    printf("Invalid format. Please try again.\n");
                }
                break;

            case 'q':
                printf("Quitting...\n");
                exit(0);
                break;

            case '?':
                printf("Help: Enter a card number to choose, 'i' followed by a number for info on that card, 'q' to quit, 'r' to redisplay list, 'h' to show hand, 'h number' to get info on card number from hand, 't number' for the tableau.\n");
                break;

            case 'r':
                display_cards(g, list, num, prompt);
                break;

case 'h':
    if (sscanf(action + 1, "%d", &selected_card) == 1)
    {
        display_hand_card(g, who, selected_card - 1);
    }
    else if (action[1] == '\0')  // Ensure that only "h" is entered
    {
        display_hand(g, who);
    }
    else
    {
        printf("Invalid format. Type 'h' to view your hand or 'h [number]' to view a specific card.\n");
    }
    break;
    case 't':
            if (sscanf(action + 1, "%d", &selected_card) == 1)
            {
                display_tableau_card(g, who, selected_card - 1);
            }
            else if (action[1] == '\0')  // Ensure that only "t" is entered
            {
                display_tableau(g, who);
            }
            else
            {
                printf("Invalid format. Type 't' to view your tableau or 't [number]' to view a specific card.\n");
            }
            break;
            case 'v':
                display_vp(g);
                break;
            default:
                if (sscanf(action, "%d", &selected_card) == 1) {
                    if (selected_card >= 0 && selected_card <= num) {
                        return selected_card;
                    } else {
                        printf("Invalid selection. Please try again.\n");
                    }
                } else {
                    printf("Invalid input. Please try again or enter '?' for help.\n");
                }
                break;
        }
    }
}



void tui_choose_discard(game *g, int who, int list[], int *num, int discard) {
    int discard_count = 0;
    /* MSVC doesn't support use of variable length arrays, so... */
    #define TEMP_MAX_VAL 100
    int temp_list[TEMP_MAX_VAL];  // Temporary list to hold indices of cards not yet discarded

    // Initially, temp_list is a copy of the original list
    for (int i = 0; i < *num; i++) {
        temp_list[i] = list[i];
    }

    display_cards(g, temp_list, *num - discard_count, "You need to discard. Here are your options:");

    while (discard_count < discard) {
        int selected_card = get_card_choice(g, who, temp_list, *num - discard_count, "Enter card number to discard");
        
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

void tui_choose_action(game *g, int who, int action[2], int one) {
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
        fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = 0;

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
    choice = get_card_choice(g, who, list, num, "Enter the number of the card you want to play, or 0 to pass:");

    // If the user chooses to pass
    if (choice == 0) {
        return -1;
    }

    return list[choice - 1]; // Return the card index from the list
}

void tui_choose_pay(game *g, int who, int which, int list[], int *num,
                    int special[], int *num_special, int mil_only,
                    int mil_bonus) {
    
    // If there are specials, display an error and return
    if (*num_special > 0) {
        printf("Error: Handling of specials is not implemented yet.\n");
        return;
    }

    card *c_ptr = &g->deck[which];
    int cost = 0, military = 0, ict_mil = 0, iif_mil = 0;
    char *cost_card = NULL;
    discounts discount;

    // Compute the cost based on card type
    if (c_ptr->d_ptr->type == TYPE_DEVELOPMENT) {
        cost = devel_cost(g, who, which);
    }
    else if (c_ptr->d_ptr->type == TYPE_WORLD) {
        compute_discounts(g, who, &discount);

        if (c_ptr->d_ptr->flags & FLAG_MILITARY) {
            military_world_payment(g, who, which, mil_only, mil_bonus, &discount, &military, &cost, &cost_card);
        }
        else {
            peaceful_world_payment(g, who, which, mil_only, &discount, &cost, &ict_mil, &iif_mil);
        }
    }

    char display_message[512];
    sprintf(display_message, "Choose payment for %s (%d card%s). Here are your options:", c_ptr->d_ptr->name, cost, cost > 1 ? "s" : "");

    int temp_list[*num];
    for (int i = 0; i < *num; i++) {
        temp_list[i] = list[i];
    }

    display_cards(g, temp_list, *num, display_message);

    int total_paid = 0;
    while (total_paid < cost) {
        int selected_card = get_card_choice(g, who, temp_list, *num - total_paid, "Enter card number to use for payment");
        
        // Add the selected card to the list of paid cards
        list[total_paid] = temp_list[selected_card - 1];
        total_paid++;
        
        // Remove the selected card from temp_list by shifting all subsequent cards
        for (int i = selected_card - 1; i < *num - total_paid; i++) {
            temp_list[i] = temp_list[i + 1];
        }
        
        if (total_paid < cost) {
            sprintf(display_message, "You have paid %d out of %d. Remaining options:", total_paid, cost);
            display_cards(g, temp_list, *num - total_paid, display_message);
        }
    }

    // Set *num to total_paid
    *num = total_paid;
}
/*
* Choose consume powers to use.
*/

void tui_choose_consume(game *g, int who, int cidx[], int oidx[], int *num,
                        int *num_special, int optional) {
    int choice, i;
card *c_ptr;

    // Loop over the cards in cidx and display their powers
    for (i = 0; i < *num; i++) {
        c_ptr = &g->deck[cidx[i]];
        printf("%d: %s, %s\n",i + 1, c_ptr->d_ptr->name, get_card_power_name(cidx[i], oidx[i]));
    }

    // If optional, allow the user to not choose any power
    if (optional) {
        printf("0: Use no powers\n");
    }

    // Get the user's choice
    printf("Enter the number of the card/power to use: ");
    char choice_str[20];
    fgets(choice_str, sizeof(choice_str), stdin);
    sscanf(choice_str, "%d", &choice);

    // Validate user's choice
    while (choice < 0 || (choice > *num) || (!optional && choice == 0)) {
        printf("Invalid choice. Please enter a valid number: ");
        char choice_str[20];
    fgets(choice_str, sizeof(choice_str), stdin);
    sscanf(choice_str, "%d", &choice);
    }

    // Handle the user's choice
    if (choice == 0 && optional) {
        *num = *num_special = 0;
        return;
    } else {
        cidx[0] = cidx[choice - 1];
        oidx[0] = oidx[choice - 1];
        *num = *num_special = 1;
    }
}
/* Choose goods to consume. */
void tui_choose_good(game *g, int who, int c_idx, int o_idx, int goods[],
                     int *num, int min, int max) {
    int n = 0, selected_index, multi = -1;
    int temp_goods[*num];  // Temporary list to hold indices of goods not yet chosen

    // Initially, temp_goods is a copy of the original list
    for (int i = 0; i < *num; i++) {
        temp_goods[i] = goods[i];
    }

    /* Get pointer to card holding consume power */
    card *c_ptr = &g->deck[c_idx];

    // Display initial list of goods to choose from
    char message[1024];
    sprintf(message, "Choose good%s to consume on %s", min == 1 && max == 1 ? "" : "s", c_ptr->d_ptr->name);
    display_cards(g, temp_goods, *num, message);

    while (n < max) {
        selected_index = get_card_choice(g, who, temp_goods, *num - n, "Select a good to consume");
        if (selected_index < 0) break; // Assuming get_card_choice returns a negative value when no card is chosen.
        selected_index--;  // Adjust for 0-based indexing

        /* Check for multiple goods and remember it */
        card *selected_card = &g->deck[temp_goods[selected_index]];
        if (selected_card->num_goods > 1) {
            multi = selected_index;
        }

        // Move the chosen good to the original goods list
        goods[n] = temp_goods[selected_index];

        // Remove the chosen good from temp_goods by shifting all subsequent goods
        for (int i = selected_index; i < *num - n - 1; i++) {
            temp_goods[i] = temp_goods[i + 1];
        }

        n++;

        if (n < max && n < *num) {
            sprintf(message, "Remaining good%s to consume on %s", min - n == 1 && max - n == 1 ? "" : "s", c_ptr->d_ptr->name);
            display_cards(g, temp_goods, *num - n, message);
        }
    }

    /* If not enough goods are selected and there's a card with multiple goods, use it */
    if (multi >= 0) {
        while (n < min) {
            goods[n++] = goods[multi];
        }
    }

    /* Set number of goods chosen */
    *num = n;
}

/* Choose a windfall world to produce on. */
void tui_choose_windfall(game *g, int who, int list[], int *num) {
    int choice;

    // Display the list of cards using the display_cards function
    display_cards(g, list, *num, "Choose a windfall world to produce on:");

    // Get user choice
    choice = get_card_choice(g, who, list, *num, "Enter the number of the card you want to produce on:");

    // Set *num to 1
    *num = 1;
    list[0] = list[choice - 1]; // Return the card index from the list
}

/* Display the player's hand. */
void display_hand(game *g, int who)
{
    int x, count = 0;

    /* Display the cards in the player's hand in a numbered list */
    x = g->p[who].head[WHERE_HAND];
    printf("Cards in Hand:\n");
    while (x != -1)
    {
        count++;
        printf("%d. %s\n", count, g->deck[x].d_ptr->name);
        x = g->deck[x].next;
    }
}

/* Display specific card from hand. */
void display_hand_card(game *g, int who, int position)
{
    int x, count = 0;

    /* Navigate to the card at the specified position in the player's hand */
    x = g->p[who].head[WHERE_HAND];
    while (x != -1 && count < position)
    {
        count++;
        x = g->deck[x].next;
    }

    /* If the card exists, display its details */
    if (x != -1)
    {
        display_card_info(g, x);
    }
    else
    {
        printf("Invalid card position. Please try again.\n");
    }
}
/* Display the cards on the table. */
void display_tableau(game *g, int who)
{
    int x, count = 0;

    /* Display the cards on the table in a numbered list */
    x = g->p[who].head[WHERE_ACTIVE];
    printf("Cards on Table:\n");
    while (x != -1)
    {
        count++;
        printf("%d. %s\n", count, g->deck[x].d_ptr->name);
        x = g->deck[x].next;
    }
}
/* Display a specific card from the tableau. */
void display_tableau_card(game *g, int who, int position)
{
    int x, count = 0;

    /* Navigate to the card at the specified position in the player's tableau */
    x = g->p[who].head[WHERE_ACTIVE];
    while (x != -1 && count < position)
    {
        count++;
        x = g->deck[x].next;
    }

    /* If the card exists, display its details */
    if (x != -1)
    {
        display_card_info(g, x);
    }
    else
    {
        printf("Invalid card position. Please try again.\n");
    }
}

/* Display victory points for all players. */
void display_vp(game *g)
{
    int i;

    /* Display the victory points for each player */
    for (i = 0; i < g->num_players; i++)
    {
        printf("Player %d: %s, %d\n", i + 1, g->p[i].name, g->p[i].end_vp);
    }
}