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

/* Common commands handling. */
typedef enum {
    CMD_CONTINUE, // Continue the loop
    CMD_QUIT,     // Quit the game
    CMD_HANDLED   // Command was handled
} CommandOutcome;

CommandOutcome handle_common_commands(game *g, char *input, int who) {
    if (strcmp(input, "q") == 0) {
        printf("Quitting...\n");
        return CMD_QUIT;
    } else if (strcmp(input, "?") == 0) {
        printf("Help: [common help info]\n");
        return CMD_HANDLED;
    } else if (input[0] == 'h') {
    int card_number;
    // Expecting a format like "h" for the hand or "h #" for a specific card in the hand
    if (sscanf(input + 1, "%d", &card_number) == 1) {
        // Display the specified card from the hand
        display_hand_card(g, who, card_number - 1); // Adjust for 0-based indexing
    } else {
        // Just "h" was entered, display the entire hand
        display_hand(g, who);
    }
    return CMD_HANDLED;
    } else if (strcmp(input, "v") == 0) {
        display_vp(g);
        return CMD_HANDLED;
    } else if (strcmp(input, "m") == 0) {
        display_military(g);
        return CMD_HANDLED;
    } else if (input[0] == 't') {
    int player_number = -1; // Default to -1 to indicate the human player
    int card_number = -1; // Default to -1 to indicate no specific card

    // Expecting a format like "t" for the human player's tableau, "t #" for an opponent's tableau,
    // or "t # #" for a specific card in a tableau
    if (input[1] == '\0') {
        // Just "t" was entered, display the human player's tableau
        display_tableau(g, who);
    } else {
        char *next_part = input + 1;
        // Parse the player number from the input
        int num_args = sscanf(next_part, "%d %d", &player_number, &card_number);

        // Adjust for 0-based indexing
        player_number -= 1;
        card_number -= 1;

        // Validate the player number
        if (num_args >= 1 && player_number >= 0 && player_number < g->num_players) {
            if (num_args == 2 && card_number >= 0) {
                // Two numbers parsed, display the specified card from the player's tableau
                display_tableau_card(g, player_number, card_number);
            } else if (num_args == 1) {
                // Only one number parsed, display the whole tableau for the player
                display_tableau(g, player_number);
            } else {
                // A card number was provided but it's invalid
                printf("Invalid card number. Please try again.\n");
            }
        } else {
            // A player number was provided but it's invalid
            printf("Invalid player number. Please try again.\n");
        }
    }
    return CMD_HANDLED;
}




    // If none of the common commands were matched, we continue processing
    return CMD_CONTINUE;
}


/* 
* Discard cards, inspired by ChatGPT.
*/

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
    if (d_ptr->vp)
    printf("VP: %d\n", d_ptr->vp);
    else {
        printf("VP: special\n");
    }
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
if (c_ptr->num_goods)
printf("Goods: %d\n", c_ptr->num_goods);
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
        if (fgets(action, sizeof(action), stdin) == NULL)
        {
            printf("Error reading input. Please try again.\n");
            continue;
            }
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
        CommandOutcome outcome = handle_common_commands(g, action, who);
        if (outcome == CMD_QUIT) {
            exit(0); // or handle quitting more gracefully if necessary
        } else if (outcome == CMD_HANDLED) {
            continue; // The command was handled, continue the loop
        }
        /* Handle specific commands below this point */
                switch (action[0]) {
                    case 'i':
                    if (sscanf(action + 1, "%d", &selected_card) == 1) {
                        if (selected_card >= 1 && selected_card <= num) {
                            display_card_info(g, list[selected_card - 1]);
                        } else {
                            printf("Invalid info command. Please try again.\n");
                        }
                    } else {
                        printf("Invalid input. Please try again or enter '?' for help.\n");
                    }
                    break;
                    case 'r':
                    display_cards(g, list, num, prompt);
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
                }
    }
}


void tui_choose_discard(game *g, int who, int list[], int *num, int discard) {
    char buf[1024];
    	/* Create prompt */
	sprintf(buf, "Choose %d card%s to discard", discard, PLURAL(discard));

    int discard_count = 0;
    /* MSVC doesn't support use of variable length arrays, so... */
    int temp_list[TEMP_MAX_VAL];  // Temporary list to hold indices of cards not yet discarded

    // Initially, temp_list is a copy of the original list
    for (int i = 0; i < *num; i++) {
        temp_list[i] = list[i];
    }

    display_cards(g, temp_list, *num - discard_count, buf);

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
    int available_actions[TEMP_MAX_VAL];   // To store indices of actions that are available.
    int num_available_actions = 0;       // Count of available actions.
printf("Choose action\n");
    // Check for advanced game
    if (g->advanced) {
        // Call advanced function (to be implemented later)
        // return tui_choose_action_advanced(g, who, action, one);
    }

    // Populate the available actions list and display them.
    for (int i = 0; i < MAX_ACTION; i++) {
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
        printf("Enter action number ('q' to quit, '?' for help, 'r' to redisplay list): ");
        char input[10];
        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            printf("Error reading input. Please try again.\n");
            continue;
            }
    input[strcspn(input, "\n")] = 0;
        CommandOutcome outcome = handle_common_commands(g, input, who);
        if (outcome == CMD_QUIT) {
            exit(0);
        } else if (outcome == CMD_HANDLED) {
            // The command was handled, re-display available actions.
            continue; // The command was handled, continue the loop
        }
            if (input[0] == 'r') {
            // Redisplay the list of available actions
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

/* Choose a number, for Gambling World etc. */
int tui_choose_lucky(game *g, int who) {
char input[10];
while (1) {
    printf("Choose a number between 1 and 7, '?' for help, 'q' to quit: ");
    if (fgets(input, sizeof(input), stdin) == NULL)
    {
        printf("Error reading input. Please try again.\n");
        continue;
        }
input[strcspn(input, "\n")] = 0;
CommandOutcome outcome = handle_common_commands(g, input, who);
if (outcome == CMD_QUIT) {
    exit(0);
} else if (outcome == CMD_HANDLED) {
continue;
    } else {
        int choice;
        if (sscanf(input, "%d", &choice) == 1) {
            if (choice >= 1 && choice <= 7) {
                return choice;
            } else {
                printf("Invalid selection. Please try again.\n");
            }
        } else {
            printf("Invalid input. Please try again or enter '?' for help.\n");
        }
    }
}
}
/* Place worlds and developments. */
int tui_choose_place(game *g, int who, int list[], int num, int phase, int special) {
    int choice;
char buf[1024];
int i, n, allow_takeover = (phase == PHASE_SETTLE);
power_where w_list[100];
power *o_ptr;
/* Create prompt */
	sprintf(buf, "Choose card to %s",
	        phase == PHASE_DEVELOP ? "develop" : "settle");

	/* Check for special card used to provide power */
	if (special != -1)
	{
		/* Append name to prompt */
		strcat(buf, " using ");
		strcat(buf, g->deck[special].d_ptr->name);

		/* XXX Check for "Rebel Sneak Attack" */
		if (!strcmp(g->deck[special].d_ptr->name, "Rebel Sneak Attack"))
		{
			/* Takeover not allowed */
			allow_takeover = 0;
		}
	}

	/* Check for settle phase and possible takeover */
	if (allow_takeover && settle_check_takeover(g, who, NULL, 1))
	{
		/* Append takeover information */
		strcat(buf, " (or pass if you want to perform a takeover)");
	}
	if (phase == PHASE_SETTLE)
	{
		/* Get settle powers */
		n = get_powers(g, who, PHASE_SETTLE, w_list);

		/* Loop over powers */
		for (i = 0; i < n; i++)
		{
			/* Get power pointer */
			o_ptr = w_list[i].o_ptr;

			/* Skip powers that aren't "flip for zero" */
			if (!(o_ptr->code & P3_FLIP_ZERO)) continue;

			/* Append flip information */
			strcat(buf, " (or pass if you want to flip a card)");

			/* Done */
			break;
		}
	}

	
    // Display the list of cards using the display_cards function
    display_cards(g, list, num, buf);

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

    card *c_ptr = &g->deck[which];
    int cost = 0, military = 0, ict_mil = 0, iif_mil = 0;
    char *cost_card = NULL;
    discounts discount;

    if (c_ptr->d_ptr->type == TYPE_DEVELOPMENT) {
        cost = devel_cost(g, who, which);
    } else if (c_ptr->d_ptr->type == TYPE_WORLD) {
        compute_discounts(g, who, &discount);

        if (c_ptr->d_ptr->flags & FLAG_MILITARY) {
            military_world_payment(g, who, which, mil_only, mil_bonus, &discount, &military, &cost, &cost_card);
        } else {
            peaceful_world_payment(g, who, which, mil_only, &discount, &cost, &ict_mil, &iif_mil);
        }
    }

    int forced_choice = compute_forced_choice(which, *num, *num_special, mil_only, mil_bonus);

    int total_regular = 0, total_special = 0;
    if (forced_choice) {
        if (forced_choice & 1) {
            total_regular = *num;
        }

        if (forced_choice >> 1) {
            total_special = *num_special;
        }
    } else {
        char display_message[512];
        sprintf(display_message, "Choose payment for %s (%d card%s). Here are your options:", c_ptr->d_ptr->name, cost, cost > 1 ? "s" : "");

        int temp_list[TEMP_MAX_VAL];
        int idx = 0;

        for (int i = 0; i < *num; i++, idx++) {
            temp_list[idx] = list[i];
        }

        for (int i = 0; i < *num_special; i++, idx++) {
            temp_list[idx] = special[i];
        }

        int combined_num = *num + *num_special;
        display_cards(g, temp_list, combined_num, display_message);

        int total_paid = 0;
        while (total_paid < cost) {
            int selected_card = get_card_choice(g, who, temp_list, combined_num - total_paid, "Enter card number to use for payment");

            if (selected_card > *num) {
                special[total_special] = temp_list[selected_card - 1];
                total_special++;
            } else {
                list[total_regular] = temp_list[selected_card - 1];
                total_regular++;
                total_paid++;
            }

            for (int i = selected_card - 1; i < combined_num - total_paid; i++) {
                temp_list[i] = temp_list[i + 1];
            }

            if (total_paid < cost) {
                sprintf(display_message, "You have paid %d out of %d. Remaining options:", total_paid, cost);
                display_cards(g, temp_list, combined_num - total_paid, display_message);
            }
        }
    }

    *num = total_regular;
    *num_special = total_special;
}
/*
* Choose cards from hand to consume.
*/
void tui_choose_consume_hand(game *g, int who, int c_idx, int o_idx, int list[], int *num)
{
	card *c_ptr;
	power *o_ptr, prestige_bonus;
	char buf[1024], *card_name;
	/* Check for prestige trade bonus power */
	if (c_idx < 0)
	{
		/* Make fake power */
		prestige_bonus.phase = PHASE_CONSUME;
		prestige_bonus.code = P4_DISCARD_HAND | P4_GET_VP;
		prestige_bonus.value = 1;
		prestige_bonus.times = 2;

		/* Use fake power */
		o_ptr = &prestige_bonus;

		/* Use fake card name */
		card_name = "Prestige Trade bonus";
	}
	else
	{
		/* Get card pointer */
		c_ptr = &g->deck[c_idx];

		/* Get power pointer */
		o_ptr = &c_ptr->d_ptr->powers[o_idx];

		/* Use card name */
		card_name = c_ptr->d_ptr->name;
	}
	/* Check for needing two cards */
	if (o_ptr->code & P4_CONSUME_TWO)
	{
		/* Create prompt */
		sprintf(buf, "Choose cards to consume on %s", card_name);
	}
	else
	{
		/* Create prompt */
		sprintf(buf, "Choose up to %d card%s to consume on %s",
		        o_ptr->times, PLURAL(o_ptr->times), card_name);
	}
/* Create a temporary list to handle the card selection */
int temp_list[TEMP_MAX_VAL];
int consume_count = 0;
for (size_t i = 0; i < *num; i++)
{
    temp_list[i] = list[i];
}
// Display cards and prompt to the user
display_cards(g, temp_list, *num, buf);

// Loop until the player has consumed the appropriate number of cards or decides to pass
while (consume_count < o_ptr->times) {
    // Get the player's choice
    int selected_card = get_card_choice(g, who, temp_list, *num, buf);
    
    // Check if the player has chosen to pass
    if (selected_card == 0) {
        printf("You have chosen to pass.\n");
        break; // Exit the loop if the player passes
    }

    // Check if the selected card is valid
    if (selected_card < 1 || selected_card > *num) {
        // Handle invalid input
        printf("Invalid choice. Please select a valid card or enter 0 to pass.\n");
        continue;
    }

    // Add the selected card to the list of consumed cards
    list[consume_count] = temp_list[selected_card - 1];
    
    // Remove the consumed card from temp_list by shifting all subsequent cards
    for (int i = selected_card - 1; i < *num - 1; i++) {
        temp_list[i] = temp_list[i + 1];
    }

    // Increment the count of consumed cards
    consume_count++;

    // If there are more cards to consume, show the remaining options
    if (consume_count < o_ptr->times) {
        display_cards(g, temp_list, *num - consume_count, "Remaining options:");
    }
}

// Update num to reflect the number of cards consumed
*num = consume_count;
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
    int temp_goods[TEMP_MAX_VAL];  // Temporary list to hold indices of goods not yet chosen

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
/* Choose a good to trade. */
void tui_choose_trade(game *g, int who, int list[], int *num, int no_bonus)
{
int choice;
char message[1024];
sprintf(message, "Choose good to trade%s", no_bonus ? " (no bonuses)" : "");
display_cards(g, list, *num, message);
choice = get_card_choice(g, who, list, *num, "Enter the number of the card you want to trade from:");
list[0] = list[choice - 1];
*num = 1;
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
    /* Display count of cards in other players' hands, but don't list them. */
    for (size_t i = 0; i < g->num_players; i++) {
 /* Skip over the human player. */
 if (!g->p[i].ai)
 continue;
 printf("%s: %d cards in hand\n", g->p[i].name, count_player_area(g, i, WHERE_HAND));
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
    printf("Cards in play for %s:\n", g->p[who].name);
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
/* Generate a detailed breakdown of victory points for a given player. */
static char *get_vp_text(game *g, int who)
{
     static char msg[1024];
    memset(msg, 0, sizeof(msg));
    player *p_ptr = &g->p[who];
    card *c_ptr;
    char text[1024] = {0};
    char bonus[1024] = {0};
    int x, t, kind, worlds, devs;

    /* Initialize counters */
    worlds = devs = 0;

    /* Start message with an empty string */
    strcpy(msg, "");

    /* VP from chips */
    if (p_ptr->vp)
    {
        sprintf(text, "VP chips: %d VP%s\n", p_ptr->vp, PLURAL(p_ptr->vp));
        strcat(msg, text);
    }

    /* VP from goals */
    if (p_ptr->goal_vp)
    {
        sprintf(text, "Goals: %d VP%s\n", p_ptr->goal_vp, PLURAL(p_ptr->goal_vp));
        strcat(msg, text);
    }

    /* VP from prestige */
    if (p_ptr->prestige)
    {
        sprintf(text, "Prestige: %d VP%s\n", p_ptr->prestige, PLURAL(p_ptr->prestige));
        strcat(msg, text);
    }

    /* Remember old kind */
    kind = g->oort_kind;

    /* Set oort kind to best scoring kind */
    g->oort_kind = g->best_oort_kind;

    /* Start at first active card */
    x = p_ptr->head[WHERE_ACTIVE];

    /* Loop over active cards */
    for ( ; x != -1; x = g->deck[x].next)
    {
        /* Get card pointer */
        c_ptr = &g->deck[x];

        /* Check for world */
        if (c_ptr->d_ptr->type == TYPE_WORLD)
        {
            /* Add VP from this world */
            worlds += c_ptr->d_ptr->vp;
        }

        /* Check for development */
        else if (c_ptr->d_ptr->type == TYPE_DEVELOPMENT)
        {
            /* Add VP from this development */
            devs += c_ptr->d_ptr->vp;
        }

        /* Check for VP bonuses */
        if (c_ptr->d_ptr->num_vp_bonus)
        {
            /* Count VPs from this card */
            t = get_score_bonus(g, who, x);

            /* Copy previous bonus (to get names in table order) */
            strcpy(text, bonus);

            /* Format text */
            sprintf(bonus, "%s: %d VP%s\n", c_ptr->d_ptr->name, t, PLURAL(t));

            /* Add to bonus string */
            strcat(bonus, text);
        }
    }

    /* Reset oort kind */
    g->oort_kind = kind;

    /* VP from worlds */
    if (worlds)
    {
        sprintf(text, "Worlds: %d VP%s\n", worlds, PLURAL(worlds));
        strcat(msg, text);
    }

    /* VP from developments */
    if (devs)
    {
        sprintf(text, "Developments: %d VP%s\n", devs, PLURAL(devs));
        strcat(msg, text);
    }

    /* Add bonus VP */
    strcat(msg, bonus);

    /* Write total */
    sprintf(text, "Total: %d VP%s\n", p_ptr->end_vp, PLURAL(p_ptr->end_vp));
    strcat(msg, text);

    /* Return message */
    return msg;
}
/* Display detailed victory points for all players. */
void display_vp(game *g)
{
    int i;
    char *vp_details;

    /* Display the victory points for each player */
    for (i = 0; i < g->num_players; i++)
    {
        /* Get detailed VP information for player i */
        vp_details = get_vp_text(g, i);
        
        /* Print player number, name, and detailed VP information */
        printf("Player %d: %s\n%s", i + 1, g->p[i].name, vp_details);
    }
}
/* Military strength. */
/* Generate a detailed breakdown of military strength for a given player. */
static char *get_military_text(mil_strength *military)
{
    static char msg[1024];
    char text[1024];
    int i;

    /* Clear text */
    strcpy(msg, "");

    /* Check for values */
    if (!military->has_data)
    {
        /* Return empty message */
        return msg;
    }

    /* Add base strength */
    sprintf(text, "Base strength: %+d\n", military->base);
    strcat(msg, text);

    /* Add temporary military */
    if (military->bonus)
    {
        /* Create text */
        sprintf(text, "Activated temporary military: %+d\n", military->bonus);
        strcat(msg, text);
    }

    /* Add rebel strength */
    if (military->rebel)
    {
        /* Create rebel text */
        sprintf(text, "Additional Rebel strength: %+d\n", military->rebel);
        strcat(msg, text);
    }

    /* Add specific strength */
    for (i = GOOD_NOVELTY; i <= GOOD_ALIEN; ++i)
    {
        /* Check for strength */
        if (military->specific[i])
        {
            /* Create text */
            sprintf(text, "Additional %s strength: %+d\n", good_printable[i], military->specific[i]);
            strcat(msg, text);
        }
    }

    /* Add defense strength */
    if (military->defense)
    {
        /* Create text */
        sprintf(text, "Additional Takeover defense: %+d\n", military->defense);
        strcat(msg, text);
    }

    /* Add attack imperium */
    if (military->attack_imperium)
    {
        /* Create text */
        sprintf(text, "Additional attack when using %s: %+d\n", military->imp_card, military->attack_imperium);
        strcat(msg, text);
    }

    /* Add maximum temporary military */
    if (military->max_bonus)
    {
        /* Create text */
        sprintf(text, "Additional potential temporary military: %+d\n", military->max_bonus);
        strcat(msg, text);
    }

    /* Check for active imperium card */
    if (military->imperium)
    {
        /* Add vulnerability text */
        strcat(msg, "IMPERIUM card played\n");
    }

    /* Check for active Rebel military world */
    if (military->military_rebel)
    {
        /* Add vulnerability text */
        strcat(msg, "REBEL Military world played\n");
    }

    /* Return text */
    return msg;
}
void display_military(game *g)
{
    int i;
    char *military_details;

    /* Display the military strength for each player */
    for (i = 0; i < g->num_players; i++)
    {
        mil_strength m; // Stack allocation of mil_strength object
        memset(&m, 0, sizeof(m)); // Initialize mil_strength object

        /* Get detailed military information for player i */
        compute_military(g, i, &m);
        military_details = get_military_text(&m);
        
        /* Print player number, name, and detailed military information */
        printf("Player %d: %s\n%s", i + 1, g->p[i].name, military_details);
    }
}
