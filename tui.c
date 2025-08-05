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
#include <string.h>
#include <stdlib.h>

/* External declaration for restart_loop */
extern int restart_loop;

/* Define restart loop constants */
#define RESTART_UNDO 5
#define RESTART_UNDO_ROUND 6
#define RESTART_UNDO_GAME 7

/* Special action codes for undo/redo */
#define ACT_UNDO -100
#define ACT_UNDO_ROUND -101
#define ACT_UNDO_GAME -102
#define ACT_REDO -103
#define ACT_REDO_ROUND -104
#define ACT_REDO_GAME -105

// Define a struct to hold flag and its description
typedef struct
{
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
void display_card_flags(unsigned int flags)
{
    printf("Flags: ");
    for (size_t i = 0; i < sizeof(flag_descriptions) / sizeof(flag_descriptions[0]); i++)
    {
        if (flags & flag_descriptions[i].flag)
        {
            printf("%s ", flag_descriptions[i].description);
        }
    }
    printf("\n");
}

/* Common commands handling. */
typedef enum
{
    CMD_CONTINUE, // Continue the loop
    CMD_QUIT,     // Quit the game
    CMD_HANDLED,  // Command was handled
    CMD_UNDO,     // Undo last action
    CMD_UNDO_ROUND, // Undo to previous round
    CMD_UNDO_GAME,  // Undo to beginning of game
    CMD_REDO,     // Redo last action
    CMD_REDO_ROUND, // Redo to next round
    CMD_REDO_GAME,  // Redo to end of game
    CMD_NEW_GAME,  // Start new game
    CMD_SAVE_GAME, // Save current game
    CMD_LOAD_GAME  // Load saved game
} CommandOutcome;

CommandOutcome handle_common_commands(game *g, char *input, int who)
{
    if (strcmp(input, "q") == 0)
    {
        printf("Quitting...\n");
        return CMD_QUIT;
    }
    else if (strcmp(input, "?") == 0)
    {
        printf("Help:\n \nThis is Race for the Galaxy, a text-based version of the classic card game.\nPlease see the README file for more detailed information.\n\nBasic Commands:\n\nq: Quit the game\nn: New game (with setup menu)\nsave: Save current game\nload: Load saved game\nh: Display your hand\nh #: Display a specific card from your hand\nv: Display victory points for all players\nm: Display military strength for all players\nt: Display your tableau\nt #: Display a specific player's tableau\nu: Undo last action\nur: Undo to previous round\nug: Undo to beginning of game\nr: Redo last action\nrr: Redo to next round\nrg: Redo to end of game\n\nPlease contact the developer at zkline@speedpost.net, if you have any questions or feedback.\n");
        return CMD_HANDLED;
    }
    else if (input[0] == 'h')
    {
        int card_number;
        // Expecting a format like "h" for the hand or "h #" for a specific card in the hand
        if (sscanf(input + 1, "%d", &card_number) == 1)
        {
            // Display the specified card from the hand
            display_hand_card(g, who, card_number - 1); // Adjust for 0-based indexing
        }
        else
        {
            // Just "h" was entered, display the entire hand
            display_hand(g, who);
        }
        return CMD_HANDLED;
    }
    else if (strcmp(input, "v") == 0)
    {
        display_vp(g);
        return CMD_HANDLED;
    }
    else if (strcmp(input, "m") == 0)
    {
        display_military(g);
        return CMD_HANDLED;
    }
    else if (input[0] == 't')
    {
        int player_number = -1; // Default to -1 to indicate the human player
        int card_number = -1;   // Default to -1 to indicate no specific card

        // Expecting a format like "t" for the human player's tableau, "t #" for an opponent's tableau,
        // or "t # #" for a specific card in a tableau
        if (input[1] == '\0')
        {
            // Just "t" was entered, display the human player's tableau
            display_tableau(g, who);
        }
        else
        {
            char *next_part = input + 1;
            // Parse the player number from the input
            int num_args = sscanf(next_part, "%d %d", &player_number, &card_number);

            // Adjust for 0-based indexing
            player_number -= 1;
            card_number -= 1;

            // Validate the player number
            if (num_args >= 1 && player_number >= 0 && player_number < g->num_players)
            {
                if (num_args == 2 && card_number >= 0)
                {
                    // Two numbers parsed, display the specified card from the player's tableau
                    display_tableau_card(g, player_number, card_number);
                }
                else if (num_args == 1)
                {
                    // Only one number parsed, display the whole tableau for the player
                    display_tableau(g, player_number);
                }
                else
                {
                    // A card number was provided but it's invalid
                    printf("Invalid card number. Please try again.\n");
                }
            }
            else
            {
                // A player number was provided but it's invalid
                printf("Invalid player number. Please try again.\n");
            }
        }
        return CMD_HANDLED;
    }
    else if (strcmp(input, "u") == 0)
    {
        /* Return special undo command */
        return CMD_UNDO;
    }
    else if (strcmp(input, "ur") == 0)
    {
        /* Return special undo round command */
        return CMD_UNDO_ROUND;
    }
    else if (strcmp(input, "ug") == 0)
    {
        /* Return special undo game command */
        return CMD_UNDO_GAME;
    }
    else if (strcmp(input, "r") == 0)
    {
        /* Return special redo command */
        return CMD_REDO;
    }
    else if (strcmp(input, "rr") == 0)
    {
        /* Return special redo round command */
        return CMD_REDO_ROUND;
    }
    else if (strcmp(input, "rg") == 0)
    {
        /* Return special redo game command */
        return CMD_REDO_GAME;
    }
    else if (strcmp(input, "n") == 0)
    {
        /* Return new game command */
        return CMD_NEW_GAME;
    }
    else if (strcmp(input, "save") == 0)
    {
        /* Return save game command */
        return CMD_SAVE_GAME;
    }
    else if (strcmp(input, "load") == 0)
    {
        /* Return load game command */
        return CMD_LOAD_GAME;
    }

    // If none of the common commands were matched, we continue processing
    return CMD_CONTINUE;
}

/*
 * Discard cards, inspired by ChatGPT.
 */

void display_cards(game *g, int list[], int num, const char *message)
{
    card *c_ptr;
    printf("%s\n", message);
    for (int i = 0; i < num; i++)
    {
        c_ptr = &g->deck[list[i]];
        printf("%d. %s\n", i + 1, c_ptr->d_ptr->name);
    }
}

// Display card details
void display_card_info(game *g, int card_index)
{
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
    else if (d_ptr->num_vp_bonus)
    {
        printf("VP: 0 (plus bonuses listed below)\n");
    }
    else
    {
        printf("VP: 0\n");
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

// Display military strength for military worlds
if (d_ptr->type == TYPE_WORLD && (d_ptr->flags & FLAG_MILITARY))
    printf("Military: %d\n", d_ptr->cost);

display_card_flags(d_ptr->flags);

// Display card powers
for (int i = 0; i < d_ptr->num_power; i++)
{
    char *power_name = get_card_power_name(card_index, i);
    printf("Power %d: %s\n", i + 1, power_name);
    free(power_name);
}

// Display VP bonuses if any
if (d_ptr->num_vp_bonus > 0)
{
    printf("VP Bonuses:\n");
    for (int i = 0; i < d_ptr->num_vp_bonus; i++)
    {
        vp_bonus *vp = &d_ptr->bonuses[i];
        printf("  +%d VP for ", vp->point);
        
        // Convert VP bonus type to readable string
        switch (vp->type)
        {
            case VP_NOVELTY_PRODUCTION: printf("Novelty production worlds"); break;
            case VP_RARE_PRODUCTION: printf("Rare production worlds"); break;
            case VP_GENE_PRODUCTION: printf("Gene production worlds"); break; 
            case VP_ALIEN_PRODUCTION: printf("Alien production worlds"); break;
            case VP_NOVELTY_WINDFALL: printf("Novelty windfall worlds"); break;
            case VP_RARE_WINDFALL: printf("Rare windfall worlds"); break;
            case VP_GENE_WINDFALL: printf("Gene windfall worlds"); break;
            case VP_ALIEN_WINDFALL: printf("Alien windfall worlds"); break;
            case VP_DEVEL_EXPLORE: printf("Explore developments"); break;
            case VP_WORLD_EXPLORE: printf("Explore worlds"); break;
            case VP_DEVEL_TRADE: printf("Trade developments"); break;
            case VP_WORLD_TRADE: printf("Trade worlds"); break;
            case VP_DEVEL_CONSUME: printf("Consume developments"); break;
            case VP_WORLD_CONSUME: printf("Consume worlds"); break;
            case VP_SIX_DEVEL: printf("6-cost developments"); break;
            case VP_DEVEL: printf("developments"); break;
            case VP_WORLD: printf("worlds"); break;
            case VP_NONMILITARY_WORLD: printf("non-military worlds"); break;
            case VP_REBEL_FLAG: printf("Rebel worlds"); break;
            case VP_ALIEN_FLAG: printf("Alien worlds"); break;
            case VP_TERRAFORMING_FLAG: printf("Terraforming worlds"); break;
            case VP_UPLIFT_FLAG: printf("Uplift worlds"); break;
            case VP_IMPERIUM_FLAG: printf("Imperium worlds"); break;
            case VP_MILITARY: printf("military strength"); break;
            case VP_TOTAL_MILITARY: printf("total military strength"); break;
            case VP_NEGATIVE_MILITARY: printf("negative military"); break;
            case VP_THREE_VP: printf("every 3 VP"); break;
            case VP_KIND_GOOD: printf("different kind of good"); break;
            case VP_PRESTIGE: printf("prestige"); break;
            case VP_NAME:
                if (vp->name) printf("each %s", vp->name);
                else printf("named cards");
                break;
            default: printf("special condition"); break;
        }
        printf("\n");
    }
}

printf("----------------------------\n\n");
}

int get_card_choice(game *g, int who, int list[], int num, const char *prompt)
{
    char action[10];
    int selected_card;

    while (1)
    {
        printf("%s (or '?' for help): ", prompt);
        if (fgets(action, sizeof(action), stdin) == NULL)
        {
            printf("Error reading input. Please try again.\n");
            continue;
        }
        action[strcspn(action, "\n")] = 0;

        // Validate input length and check for control characters
        if (strlen(action) >= sizeof(action) - 1)
        {
            printf("Input too long! Please try again.\n");
            continue;
        }

        for (int i = 0; i < strlen(action); i++)
        {
            if (iscntrl(action[i]))
            {
                printf("Invalid input! Control characters are not allowed.\n");
                continue;
            }
        }
        CommandOutcome outcome = handle_common_commands(g, action, who);
        if (outcome == CMD_QUIT)
        {
            exit(0); // or handle quitting more gracefully if necessary
        }
        else if (outcome == CMD_HANDLED)
        {
            continue; // The command was handled, continue the loop
        }
        /* Handle specific commands below this point */
        switch (action[0])
        {
        case 'i':
            if (sscanf(action + 1, "%d", &selected_card) == 1)
            {
                if (selected_card >= 1 && selected_card <= num)
                {
                    display_card_info(g, list[selected_card - 1]);
                }
                else
                {
                    printf("Invalid info command. Please try again.\n");
                }
            }
            else
            {
                printf("Invalid input. Please try again or enter '?' for help.\n");
            }
            break;
        case 'r':
            display_cards(g, list, num, prompt);
            break;
        default:
            if (sscanf(action, "%d", &selected_card) == 1)
            {
                if (selected_card >= 0 && selected_card <= num)
                {
                    return selected_card;
                }
                else
                {
                    printf("Invalid selection. Please try again.\n");
                }
            }
            else
            {
                printf("Invalid input. Please try again or enter '?' for help.\n");
            }
        }
    }
}

void tui_choose_discard(game *g, int who, int list[], int *num, int discard)
{
    char buf[1024];
    /* Create prompt */
    sprintf(buf, "Choose %d card%s to discard", discard, PLURAL(discard));

    int discard_count = 0;
    /* MSVC doesn't support use of variable length arrays, so... */
    int temp_list[TEMP_MAX_VAL]; // Temporary list to hold indices of cards not yet discarded

    // Initially, temp_list is a copy of the original list
    for (int i = 0; i < *num; i++)
    {
        temp_list[i] = list[i];
    }

    display_cards(g, temp_list, *num - discard_count, buf);

    while (discard_count < discard)
    {
        int selected_card = get_card_choice(g, who, temp_list, *num - discard_count, "Enter card number to discard");

        // Add the selected card to the list of discarded cards
        list[discard_count] = temp_list[selected_card - 1];

        // Remove the discarded card from temp_list by shifting all subsequent cards
        for (int i = selected_card - 1; i < *num - discard_count - 1; i++)
        {
            temp_list[i] = temp_list[i + 1];
        }

        discard_count++;

        if (discard_count < discard)
        {
            display_cards(g, temp_list, *num - discard_count, "Remaining options:");
        }
    }

    // Update num to reflect the number of cards discarded.
    *num = discard_count;
}

/*
 * Choose start world and initial hand discards
 */
void tui_choose_start(game *g, int who, int list[], int *num, int special[], int *num_special)
{
    int i, selected_world = -1, target_hand_size = 4;
    int discard_count, cards_to_discard;
    
    printf("=== GAME START: Choose Start World and Hand ===\n\n");
    
    /* Step 1: Choose start world */
    if (*num_special > 1)
    {
        printf("Available start worlds:\n");
        for (i = 0; i < *num_special; i++)
        {
            card *c_ptr = &g->deck[special[i]];
            printf("%d. %s\n", i + 1, c_ptr->d_ptr->name);
        }
        
        while (selected_world < 0)
        {
            printf("\nEnter start world number (1-%d): ", *num_special);
            char input[10];
            if (fgets(input, sizeof(input), stdin) == NULL)
            {
                printf("Error reading input. Please try again.\n");
                continue;
            }
            input[strcspn(input, "\n")] = 0;
            
            CommandOutcome outcome = handle_common_commands(g, input, who);
            if (outcome == CMD_QUIT) exit(0);
            else if (outcome == CMD_HANDLED) continue;
            
            int choice = atoi(input);
            if (choice >= 1 && choice <= *num_special)
            {
                selected_world = choice - 1;
                printf("Selected: %s\n\n", g->deck[special[selected_world]].d_ptr->name);
            }
            else
            {
                printf("Invalid choice. Please try again.\n");
            }
        }
    }
    else
    {
        /* Only one start world option */
        selected_world = 0;
        printf("Start world: %s\n\n", g->deck[special[0]].d_ptr->name);
    }
    
    /* Step 2: Choose cards to discard from hand */
    cards_to_discard = *num - target_hand_size;
    if (cards_to_discard > 0)
    {
        printf("Your starting hand (%d cards):\n", *num);
        display_cards(g, list, *num, "");
        
        printf("\nYou must discard %d card%s to get to %d cards.\n", 
               cards_to_discard, PLURAL(cards_to_discard), target_hand_size);
        
        /* Use existing discard logic */
        tui_choose_discard(g, who, list, num, cards_to_discard);
    }
    
    /* Return selected start world */
    special[0] = special[selected_world];
    *num_special = 1;
}

/*
 * Choose settle power to use
 */
void tui_choose_settle(game *g, int who, int cidx[], int oidx[], int *num, int *num_special)
{
    int i, selected_power = -1;
    
    printf("=== SETTLE PHASE: Choose Settle Power ===\n\n");
    
    /* Check if we have only one power */
    if (*num == 1)
    {
        /* Only one settle power available */
        card *c_ptr = &g->deck[cidx[0]];
        char *power_name = get_card_power_name(cidx[0], oidx[0]);
        printf("Using settle power: %s - %s\n", c_ptr->d_ptr->name, power_name);
        free(power_name);
        return;
    }
    
    /* Display available settle powers */
    printf("Available settle powers:\n");
    for (i = 0; i < *num; i++)
    {
        card *c_ptr = &g->deck[cidx[i]];
        char *power_name = get_card_power_name(cidx[i], oidx[i]);
        printf("%d. %s - %s\n", i + 1, c_ptr->d_ptr->name, power_name);
        free(power_name);
    }
    
    /* Get player's choice */
    while (selected_power < 0)
    {
        printf("\nEnter settle power number (1-%d): ", *num);
        char input[10];
        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            printf("Error reading input. Please try again.\n");
            continue;
        }
        input[strcspn(input, "\n")] = 0;
        
        CommandOutcome outcome = handle_common_commands(g, input, who);
        if (outcome == CMD_QUIT) exit(0);
        else if (outcome == CMD_HANDLED) continue;
        
        int choice = atoi(input);
        if (choice >= 1 && choice <= *num)
        {
            selected_power = choice - 1;
            card *c_ptr = &g->deck[cidx[selected_power]];
            char *power_name = get_card_power_name(cidx[selected_power], oidx[selected_power]);
            printf("Selected: %s - %s\n", c_ptr->d_ptr->name, power_name);
            free(power_name);
        }
        else
        {
            printf("Invalid choice. Please try again.\n");
        }
    }
    
    /* Return selected power (move it to first position) */
    if (selected_power != 0)
    {
        /* Swap selected power to first position */
        int temp_cidx = cidx[0];
        int temp_oidx = oidx[0];
        cidx[0] = cidx[selected_power];
        oidx[0] = oidx[selected_power];
        cidx[selected_power] = temp_cidx;
        oidx[selected_power] = temp_oidx;
    }
    
    /* Set to use only the selected power */
    *num = 1;
}

/*
 * Choose card to save for later
 */
void tui_choose_save(game *g, int who, int list[], int *num)
{
    int i, selected_card = -1;
    
    printf("=== Choose Card to Save ===\n\n");
    
    /* Check if we have only one card */
    if (*num == 1)
    {
        /* Only one card available */
        card *c_ptr = &g->deck[list[0]];
        printf("Saving card: %s\n", c_ptr->d_ptr->name);
        return;
    }
    
    /* Display available cards */
    printf("Choose card to save for later:\n");
    display_cards(g, list, *num, "");
    
    /* Get player's choice */
    while (selected_card < 0)
    {
        printf("\nEnter card number (1-%d): ", *num);
        char input[10];
        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            printf("Error reading input. Please try again.\n");
            continue;
        }
        input[strcspn(input, "\n")] = 0;
        
        CommandOutcome outcome = handle_common_commands(g, input, who);
        if (outcome == CMD_QUIT) exit(0);
        else if (outcome == CMD_HANDLED) continue;
        
        int choice = atoi(input);
        if (choice >= 1 && choice <= *num)
        {
            selected_card = choice - 1;
            card *c_ptr = &g->deck[list[selected_card]];
            printf("Selected: %s\n", c_ptr->d_ptr->name);
        }
        else
        {
            printf("Invalid choice. Please try again.\n");
        }
    }
    
    /* Return selected card (move it to first position) */
    if (selected_card != 0)
    {
        /* Swap selected card to first position */
        int temp = list[0];
        list[0] = list[selected_card];
        list[selected_card] = temp;
    }
    
    /* Set to save only the selected card */
    *num = 1;
}

/*
 * Ask player if they want to apply prestige boost to an action
 */
static int ask_prestige_boost(game *g, int who, int action)
{
    int choice;
    
    /* Check if prestige boost is possible */
    if (g->expanded != 3 || g->p[who].prestige_action_used || g->p[who].prestige <= 0)
    {
        return 0; /* No prestige boost possible */
    }
    
    /* Skip search action - it uses prestige action but no prestige points */
    if (action == ACT_SEARCH)
    {
        return 0;
    }
    
    printf("\nApply PRESTIGE BOOST to %s? (Costs 1 prestige point + your prestige action)\n", actname[action]);
    printf("You have %d prestige point%s available.\n", g->p[who].prestige, PLURAL(g->p[who].prestige));
    printf("1. No, use regular action\n");
    printf("2. Yes, use prestige-boosted action\n");
    
    while (1)
    {
        printf("Enter choice (1-2): ");
        fflush(stdout);
        
        if (scanf("%d", &choice) != 1)
        {
            /* Clear invalid input */
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            printf("Invalid input. Please enter a number.\n");
            continue;
        }
        
        if (choice == 1)
        {
            return 0; /* No prestige boost */
        }
        else if (choice == 2)
        {
            return 1; /* Apply prestige boost */
        }
        
        printf("Invalid choice. Please select 1 or 2.\n");
    }
}

/*
 * Choose actions in advanced 2-player game
 */
void tui_choose_action_advanced(game *g, int who, int action[2], int one)
{
    int i, selected_actions[2] = {-1, -1};
    int available_actions[TEMP_MAX_VAL];
    int num_available_actions = 0;
    int num_to_select = (one == 0) ? 2 : 1;
    int actions_selected = 0;
    
    printf("=== ADVANCED GAME: Choose Actions ===\n\n");
    
    /* Set prompt based on what we're choosing */
    if (one == 0)
        printf("Choose TWO actions for this round:\n");
    else if (one == 1)
        printf("Choose your FIRST action:\n");
    else if (one == 2)
        printf("Choose your SECOND action:\n");
    
    /* Populate available actions list - include advanced actions */
    for (i = 0; i < MAX_ACTION; i++)
    {
        /* Skip ACT_SEARCH under certain conditions */
        if (i == ACT_SEARCH && (g->expanded != 3 || g->p[who].prestige_action_used))
        {
            continue;
        }
        
        /* In advanced game, include ACT_DEVELOP2 and ACT_SETTLE2 */
        available_actions[num_available_actions] = i;
        num_available_actions++;
        printf("%d. %s\n", num_available_actions, actname[i]);
    }
    
    /* Get player's action choices */
    while (actions_selected < num_to_select)
    {
        if (num_to_select == 2)
        {
            if (actions_selected == 0)
                printf("\nSelect action %d of 2: ", actions_selected + 1);
            else
                printf("Select action %d of 2: ", actions_selected + 1);
        }
        else
        {
            printf("\nSelect action: ");
        }
        
        char input[10];
        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            printf("Error reading input. Please try again.\n");
            continue;
        }
        input[strcspn(input, "\n")] = 0;
        
        CommandOutcome outcome = handle_common_commands(g, input, who);
        if (outcome == CMD_QUIT) exit(0);
        else if (outcome == CMD_HANDLED) continue;
        
        if (strcmp(input, "r") == 0)
        {
            /* Redisplay action list */
            for (i = 0; i < num_available_actions; i++)
            {
                printf("%d. %s\n", i + 1, actname[available_actions[i]]);
            }
            continue;
        }
        
        int choice = atoi(input);
        
        if (choice >= 1 && choice <= num_available_actions)
        {
            int selected_action = available_actions[choice - 1];
            
            /* Check if action already selected */
            if (num_to_select == 2 && actions_selected > 0 && 
                selected_actions[0] == selected_action)
            {
                printf("You already selected %s. Choose a different action.\n", 
                       actname[selected_action]);
                continue;
            }
            
            
            /* Ask about prestige boost for this action */
            if (ask_prestige_boost(g, who, selected_action))
            {
                selected_action |= ACT_PRESTIGE;
                printf("Selected: %s (PRESTIGE BOOSTED)\n", actname[selected_action & ACT_MASK]);
            }
            else
            {
                printf("Selected: %s\n", actname[selected_action]);
            }
            
            selected_actions[actions_selected] = selected_action;
            actions_selected++;
        }
        else
        {
            printf("Invalid choice. Please try again.\n");
        }
    }
    
    /* Return selected actions */
    action[0] = selected_actions[0];
    if (num_to_select == 2)
        action[1] = selected_actions[1];
    else
        action[1] = -1;
    
    /* Apply the same coercion logic as the GUI */
    /* Check for second Develop chosen without first */
    if ((action[0] & ACT_MASK) == ACT_DEVELOP2)
        action[0] = ACT_DEVELOP | (action[0] & ACT_PRESTIGE);
    if ((action[1] & ACT_MASK) == ACT_DEVELOP2 &&
        (action[0] & ACT_MASK) != ACT_DEVELOP)
        action[1] = ACT_DEVELOP | (action[1] & ACT_PRESTIGE);
    
    /* Check for second Settle chosen without first */
    if ((action[0] & ACT_MASK) == ACT_SETTLE2)
        action[0] = ACT_SETTLE | (action[0] & ACT_PRESTIGE);
    if ((action[1] & ACT_MASK) == ACT_SETTLE2 &&
        (action[0] & ACT_MASK) != ACT_SETTLE)
        action[1] = ACT_SETTLE | (action[1] & ACT_PRESTIGE);
    
    if (num_to_select == 2)
    {
        printf("\nSelected actions: %s and %s\n", 
               actname[action[0]], actname[action[1]]);
    }
}

void tui_choose_action(game *g, int who, int action[2], int one)
{
    int selected_action;
    int available_actions[TEMP_MAX_VAL]; // To store indices of actions that are available.
    int num_available_actions = 0;       // Count of available actions.
    printf("Choose action\n");
    // Check for advanced game
    if (g->advanced)
    {
        /* Call advanced function */
        tui_choose_action_advanced(g, who, action, one);
        return;
    }

    // Populate the available actions list and display them.
    for (int i = 0; i < MAX_ACTION; i++)
    {
        // Skip the ACT_SEARCH action under certain conditions
        if (i == ACT_SEARCH && (g->expanded != 3 || g->p[who].prestige_action_used))
        {
            continue;
        }

        // Skip ACT_DEVELOP2 and ACT_SETTLE2
        if (i == ACT_DEVELOP2 || i == ACT_SETTLE2)
        {
            continue;
        }

        available_actions[num_available_actions++] = i;
        printf("%d. %s\n", num_available_actions, actname[i]);
    }

    while (1)
    {
        printf("Enter action number ('q' to quit, '?' for help, 'r' to redisplay list): ");
        char input[10];
        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            printf("Error reading input. Please try again.\n");
            continue;
        }
        input[strcspn(input, "\n")] = 0;
        CommandOutcome outcome = handle_common_commands(g, input, who);
        if (outcome == CMD_QUIT)
        {
            exit(0);
        }
        else if (outcome == CMD_UNDO)
        {
            action[0] = ACT_UNDO;
            action[1] = -1;
            return;
        }
        else if (outcome == CMD_UNDO_ROUND)
        {
            action[0] = ACT_UNDO_ROUND;
            action[1] = -1;
            return;
        }
        else if (outcome == CMD_UNDO_GAME)
        {
            action[0] = ACT_UNDO_GAME;
            action[1] = -1;
            return;
        }
        else if (outcome == CMD_REDO)
        {
            action[0] = ACT_REDO;
            action[1] = -1;
            return;
        }
        else if (outcome == CMD_REDO_ROUND)
        {
            action[0] = ACT_REDO_ROUND;
            action[1] = -1;
            return;
        }
        else if (outcome == CMD_REDO_GAME)
        {
            action[0] = ACT_REDO_GAME;
            action[1] = -1;
            return;
        }
        else if (outcome == CMD_NEW_GAME)
        {
            /* Special action code for new game */
            action[0] = -106;
            action[1] = -1;
            return;
        }
        else if (outcome == CMD_SAVE_GAME)
        {
            /* Special action code for save game */
            action[0] = -107;
            action[1] = -1;
            return;
        }
        else if (outcome == CMD_LOAD_GAME)
        {
            /* Special action code for load game */
            action[0] = -108;
            action[1] = -1;
            return;
        }
        else if (outcome == CMD_HANDLED)
        {
            // The command was handled, re-display available actions.
            continue; // The command was handled, continue the loop
        }
        if (input[0] == 'r')
        {
            // Redisplay the list of available actions
            for (int i = 0; i < num_available_actions; i++)
            {
                printf("%d. %s\n", i + 1, plain_actname[available_actions[i]]);
            }
        }
        else if (sscanf(input, "%d", &selected_action) == 1)
        {
            if (selected_action >= 1 && selected_action <= num_available_actions)
            {
                int chosen_action = available_actions[selected_action - 1];
                
                /* Ask about prestige boost for this action */
                if (ask_prestige_boost(g, who, chosen_action))
                {
                    chosen_action |= ACT_PRESTIGE;
                    printf("Action selected: %s (PRESTIGE BOOSTED)\n", actname[chosen_action & ACT_MASK]);
                }
                else
                {
                    printf("Action selected: %s\n", actname[chosen_action]);
                }
                
                action[0] = chosen_action;
                action[1] = -1;
                return; // Exit the function once the action is selected.
            }
            else
            {
                printf("Invalid selection. Please try again.\n");
            }
        }
        else
        {
            printf("Invalid input. Please try again or enter 'h' for help.\n");
        }
    }
}

/* Choose a number, for Gambling World etc. */
int tui_choose_lucky(game *g, int who)
{
    char input[10];
    while (1)
    {
        printf("Choose a number between 1 and 7, '?' for help, 'q' to quit: ");
        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            printf("Error reading input. Please try again.\n");
            continue;
        }
        input[strcspn(input, "\n")] = 0;
        CommandOutcome outcome = handle_common_commands(g, input, who);
        if (outcome == CMD_QUIT)
        {
            exit(0);
        }
        else if (outcome == CMD_HANDLED)
        {
            continue;
        }
        else
        {
            int choice;
            if (sscanf(input, "%d", &choice) == 1)
            {
                if (choice >= 1 && choice <= 7)
                {
                    return choice;
                }
                else
                {
                    printf("Invalid selection. Please try again.\n");
                }
            }
            else
            {
                printf("Invalid input. Please try again or enter '?' for help.\n");
            }
        }
    }
}
/* Place worlds and developments. */
int tui_choose_place(game *g, int who, int list[], int num, int phase, int special)
{
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
            if (!(o_ptr->code & P3_FLIP_ZERO))
                continue;

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
    if (choice == 0)
    {
        return -1;
    }

    return list[choice - 1]; // Return the card index from the list
}
void tui_choose_pay(game *g, int who, int which, int list[], int *num,
                    int special[], int *num_special, int mil_only,
                    int mil_bonus)
{

    card *c_ptr = &g->deck[which];
    int cost = 0, military = 0, ict_mil = 0, iif_mil = 0;
    char *cost_card = NULL;
    discounts discount;

    if (c_ptr->d_ptr->type == TYPE_DEVELOPMENT)
    {
        cost = devel_cost(g, who, which);
    }
    else if (c_ptr->d_ptr->type == TYPE_WORLD)
    {
        compute_discounts(g, who, &discount);

        if (c_ptr->d_ptr->flags & FLAG_MILITARY)
        {
            military_world_payment(g, who, which, mil_only, mil_bonus, &discount, &military, &cost, &cost_card);
        }
        else
        {
            peaceful_world_payment(g, who, which, mil_only, &discount, &cost, &ict_mil, &iif_mil);
        }
    }

    int forced_choice = compute_forced_choice(which, *num, *num_special, mil_only, mil_bonus);

    int total_regular = 0, total_special = 0;
    if (forced_choice)
    {
        if (forced_choice & 1)
        {
            total_regular = *num;
        }

        if (forced_choice >> 1)
        {
            total_special = *num_special;
        }
    }
    else
    {
        char display_message[512];
        sprintf(display_message, "Choose payment for %s (%d card%s). Here are your options:", c_ptr->d_ptr->name, cost, cost > 1 ? "s" : "");

        int temp_list[TEMP_MAX_VAL];
        int idx = 0;

        for (int i = 0; i < *num; i++, idx++)
        {
            temp_list[idx] = list[i];
        }

        for (int i = 0; i < *num_special; i++, idx++)
        {
            temp_list[idx] = special[i];
        }

        int combined_num = *num + *num_special;
        display_cards(g, temp_list, combined_num, display_message);

        int total_paid = 0;
        while (total_paid < cost)
        {
            int selected_card = get_card_choice(g, who, temp_list, combined_num - total_paid, "Enter card number to use for payment");

            if (selected_card > *num)
            {
                special[total_special] = temp_list[selected_card - 1];
                total_special++;
            }
            else
            {
                list[total_regular] = temp_list[selected_card - 1];
                total_regular++;
                total_paid++;
            }

            for (int i = selected_card - 1; i < combined_num - total_paid; i++)
            {
                temp_list[i] = temp_list[i + 1];
            }

            if (total_paid < cost)
            {
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
    while (consume_count < o_ptr->times)
    {
        // Get the player's choice
        int selected_card = get_card_choice(g, who, temp_list, *num, buf);

        // Check if the player has chosen to pass
        if (selected_card == 0)
        {
            printf("You have chosen to pass.\n");
            break; // Exit the loop if the player passes
        }

        // Check if the selected card is valid
        if (selected_card < 1 || selected_card > *num)
        {
            // Handle invalid input
            printf("Invalid choice. Please select a valid card or enter 0 to pass.\n");
            continue;
        }

        // Add the selected card to the list of consumed cards
        list[consume_count] = temp_list[selected_card - 1];

        // Remove the consumed card from temp_list by shifting all subsequent cards
        for (int i = selected_card - 1; i < *num - 1; i++)
        {
            temp_list[i] = temp_list[i + 1];
        }

        // Increment the count of consumed cards
        consume_count++;

        // If there are more cards to consume, show the remaining options
        if (consume_count < o_ptr->times)
        {
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
                        int *num_special, int optional)
{
    int choice, i;
    card *c_ptr;

    // Loop over the cards in cidx and display their powers
    for (i = 0; i < *num; i++)
    {
        c_ptr = &g->deck[cidx[i]];
        printf("%d: %s, %s\n", i + 1, c_ptr->d_ptr->name, get_card_power_name(cidx[i], oidx[i]));
    }

    // If optional, allow the user to not choose any power
    if (optional)
    {
        printf("0: Use no powers\n");
    }

    // Get the user's choice
    printf("Enter the number of the card/power to use: ");
    char choice_str[20];
    fgets(choice_str, sizeof(choice_str), stdin);
    sscanf(choice_str, "%d", &choice);

    // Validate user's choice
    while (choice < 0 || (choice > *num) || (!optional && choice == 0))
    {
        printf("Invalid choice. Please enter a valid number: ");
        char choice_str[20];
        fgets(choice_str, sizeof(choice_str), stdin);
        sscanf(choice_str, "%d", &choice);
    }

    // Handle the user's choice
    if (choice == 0 && optional)
    {
        *num = *num_special = 0;
        return;
    }
    else
    {
        cidx[0] = cidx[choice - 1];
        oidx[0] = oidx[choice - 1];
        *num = *num_special = 1;
    }
}
/* Choose goods to consume. */
void tui_choose_good(game *g, int who, int c_idx, int o_idx, int goods[],
                     int *num, int min, int max)
{
    int n = 0, selected_index, multi = -1;
    int temp_goods[TEMP_MAX_VAL]; // Temporary list to hold indices of goods not yet chosen

    // Initially, temp_goods is a copy of the original list
    for (int i = 0; i < *num; i++)
    {
        temp_goods[i] = goods[i];
    }

    /* Get pointer to card holding consume power */
    card *c_ptr = &g->deck[c_idx];

    // Display initial list of goods to choose from
    char message[1024];
    sprintf(message, "Choose good%s to consume on %s", min == 1 && max == 1 ? "" : "s", c_ptr->d_ptr->name);
    display_cards(g, temp_goods, *num, message);

    while (n < max)
    {
        selected_index = get_card_choice(g, who, temp_goods, *num - n, "Select a good to consume");
        if (selected_index < 0)
            break;        // Assuming get_card_choice returns a negative value when no card is chosen.
        selected_index--; // Adjust for 0-based indexing

        /* Check for multiple goods and remember it */
        card *selected_card = &g->deck[temp_goods[selected_index]];
        if (selected_card->num_goods > 1)
        {
            multi = selected_index;
        }

        // Move the chosen good to the original goods list
        goods[n] = temp_goods[selected_index];

        // Remove the chosen good from temp_goods by shifting all subsequent goods
        for (int i = selected_index; i < *num - n - 1; i++)
        {
            temp_goods[i] = temp_goods[i + 1];
        }

        n++;

        if (n < max && n < *num)
        {
            sprintf(message, "Remaining good%s to consume on %s", min - n == 1 && max - n == 1 ? "" : "s", c_ptr->d_ptr->name);
            display_cards(g, temp_goods, *num - n, message);
        }
    }

    /* If not enough goods are selected and there's a card with multiple goods, use it */
    if (multi >= 0)
    {
        while (n < min)
        {
            goods[n++] = goods[multi];
        }
    }

    /* Set number of goods chosen */
    *num = n;
}

/* Choose a windfall world to produce on. */
void tui_choose_windfall(game *g, int who, int list[], int *num)
{
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
    for (size_t i = 0; i < g->num_players; i++)
    {
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
    if (p_ptr->prestige || (g->expanded == 3))
    {
        sprintf(text, "Prestige: %d VP%s", p_ptr->prestige, PLURAL(p_ptr->prestige));
        strcat(msg, text);
        
        /* Show prestige action status in expansion 3 */
        if (g->expanded == 3)
        {
            sprintf(text, " (Prestige action: %s)\n", 
                    p_ptr->prestige_action_used ? "USED" : "Available");
            strcat(msg, text);
        }
        else
        {
            strcat(msg, "\n");
        }
    }

    /* Remember old kind */
    kind = g->oort_kind;

    /* Set oort kind to best scoring kind */
    g->oort_kind = g->best_oort_kind;

    /* Start at first active card */
    x = p_ptr->head[WHERE_ACTIVE];

    /* Loop over active cards */
    for (; x != -1; x = g->deck[x].next)
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
        mil_strength m;           // Stack allocation of mil_strength object
        memset(&m, 0, sizeof(m)); // Initialize mil_strength object

        /* Get detailed military information for player i */
        compute_military(g, i, &m);
        military_details = get_military_text(&m);

        /* Print player number, name, and detailed military information */
        printf("Player %d: %s\n%s", i + 1, g->p[i].name, military_details);
    }
}

/*
 * Display new game menu and get parameters from user.
 * Returns 1 if user wants to start a new game, 2 if loaded a game, 0 if cancelled.
 */
int tui_new_game_menu(options *opt)
{
    char input[256];
    int choice;
    int max_players;
    options temp_opt;
    
    /* Copy current options to temporary */
    memcpy(&temp_opt, opt, sizeof(options));
    
    while (1)
    {
        /* Clear screen */
        printf("\033[2J\033[H");
        
        /* Display header */
        printf("=== New Game Setup ===\n\n");
        
        /* Display current settings */
        printf("Current settings:\n");
        printf("1. Player name: %s\n", temp_opt.player_name ? temp_opt.player_name : "Human");
        printf("2. Expansion: %s\n", exp_names[temp_opt.expanded]);
        
        /* Calculate max players for current expansion */
        max_players = (temp_opt.expanded == 0) ? 4 : 
                      (temp_opt.expanded == 4) ? 5 : 6;
        
        printf("3. Number of players: %d", temp_opt.num_players);
        if (temp_opt.num_players > max_players)
        {
            printf(" (will be reduced to %d)", max_players);
        }
        printf("\n");
        
        /* Show advanced option only for 2 players */
        if (temp_opt.num_players == 2)
        {
            printf("4. Two-player advanced: %s\n", temp_opt.advanced ? "Yes" : "No");
        }
        
        /* Show goals option for expansions 1-3 */
        if (temp_opt.expanded >= 1 && temp_opt.expanded <= 3)
        {
            printf("5. Disable goals: %s\n", temp_opt.disable_goal ? "Yes" : "No");
        }
        
        /* Show takeovers option for expansions 2-3 */
        if (temp_opt.expanded >= 2 && temp_opt.expanded <= 3)
        {
            printf("6. Disable takeovers: %s\n", temp_opt.disable_takeover ? "Yes" : "No");
        }
        
        printf("7. Custom seed: ");
        if (temp_opt.customize_seed)
        {
            printf("%u\n", temp_opt.seed);
        }
        else
        {
            printf("Random\n");
        }
        
        printf("\n");
        printf("Enter number to change setting (1-7)\n");
        printf("Enter 's' to start game with these settings\n");
        printf("Enter 'l' to load saved game\n");
        printf("Enter 'c' to cancel\n");
        printf("Choice: ");
        
        /* Get user input */
        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            return 0;
        }
        input[strcspn(input, "\n")] = 0;
        
        /* Check for start game */
        if (strcmp(input, "s") == 0)
        {
            /* Validate and adjust player count */
            if (temp_opt.num_players > max_players)
            {
                temp_opt.num_players = max_players;
            }
            
            /* Copy temporary options back */
            memcpy(opt, &temp_opt, sizeof(options));
            
            /* Clear campaign */
            opt->campaign_name = "";
            
            return 1;
        }
        
        /* Check for cancel */
        if (strcmp(input, "c") == 0)
        {
            return 0;
        }
        
        /* Check for load game */
        if (strcmp(input, "l") == 0)
        {
            /* Try to load a game */
            if (tui_load_game())
            {
                /* Load successful - we need to tell the caller that we loaded a game instead of starting new */
                return 2; /* Special return code for "loaded game" */
            }
            /* Load failed or cancelled - continue showing menu */
            continue;
        }
        
        /* Parse numeric choice */
        choice = atoi(input);
        
        switch (choice)
        {
            case 1:
                /* Change player name */
                printf("Enter player name (max 50 characters): ");
                if (fgets(input, sizeof(input), stdin) != NULL)
                {
                    input[strcspn(input, "\n")] = 0;
                    if (strlen(input) > 0)
                    {
                        if (temp_opt.player_name) free(temp_opt.player_name);
                        temp_opt.player_name = strdup(input);
                    }
                }
                break;
                
            case 2:
                /* Change expansion */
                printf("\nSelect expansion:\n");
                for (int i = 0; i <= MAX_EXPANSION - 1; i++)
                {
                    printf("%d. %s\n", i + 1, exp_names[i]);
                }
                printf("Choice: ");
                if (fgets(input, sizeof(input), stdin) != NULL)
                {
                    int exp = atoi(input) - 1;
                    if (exp >= 0 && exp < MAX_EXPANSION)
                    {
                        temp_opt.expanded = exp;
                        
                        /* Reset advanced if not 2 players */
                        if (temp_opt.num_players != 2)
                        {
                            temp_opt.advanced = 0;
                        }
                        
                        /* Reset goals/takeovers if not applicable */
                        if (exp < 1 || exp > 3)
                        {
                            temp_opt.disable_goal = 0;
                        }
                        if (exp < 2 || exp > 3)
                        {
                            temp_opt.disable_takeover = 0;
                        }
                    }
                }
                break;
                
            case 3:
                /* Change number of players */
                printf("\nSelect number of players (2-%d): ", max_players);
                if (fgets(input, sizeof(input), stdin) != NULL)
                {
                    int num = atoi(input);
                    if (num >= 2 && num <= 6)
                    {
                        temp_opt.num_players = num;
                        
                        /* Reset advanced if not 2 players */
                        if (num != 2)
                        {
                            temp_opt.advanced = 0;
                        }
                    }
                }
                break;
                
            case 4:
                /* Toggle advanced (only for 2 players) */
                if (temp_opt.num_players == 2)
                {
                    temp_opt.advanced = !temp_opt.advanced;
                }
                break;
                
            case 5:
                /* Toggle disable goals (expansions 1-3) */
                if (temp_opt.expanded >= 1 && temp_opt.expanded <= 3)
                {
                    temp_opt.disable_goal = !temp_opt.disable_goal;
                }
                break;
                
            case 6:
                /* Toggle disable takeovers (expansions 2-3) */
                if (temp_opt.expanded >= 2 && temp_opt.expanded <= 3)
                {
                    temp_opt.disable_takeover = !temp_opt.disable_takeover;
                }
                break;
                
            case 7:
                /* Custom seed */
                if (temp_opt.customize_seed)
                {
                    /* Currently custom, switch to random */
                    temp_opt.customize_seed = 0;
                }
                else
                {
                    /* Currently random, get custom seed */
                    printf("Enter seed value (0-4294967295): ");
                    if (fgets(input, sizeof(input), stdin) != NULL)
                    {
                        unsigned int seed = (unsigned int)strtoul(input, NULL, 10);
                        temp_opt.seed = seed;
                        temp_opt.customize_seed = 1;
                    }
                }
                break;
        }
    }
}

/*
 * Save current game with user-specified filename.
 * Returns 1 if game was saved, 0 if cancelled.
 */
int tui_save_game(game *g, int who)
{
    char filename[256];
    char input[256];
    
    printf("Save current game\n");
    printf("Enter filename (without .rftg extension): ");
    
    if (fgets(input, sizeof(input), stdin) == NULL)
    {
        return 0;
    }
    
    /* Remove newline */
    input[strcspn(input, "\n")] = 0;
    
    /* Check for empty input */
    if (strlen(input) == 0)
    {
        printf("Save cancelled.\n");
        return 0;
    }
    
    /* Add .rftg extension if not present */
    if (strstr(input, ".rftg") == NULL)
    {
        snprintf(filename, sizeof(filename), "%s.rftg", input);
    }
    else
    {
        strncpy(filename, input, sizeof(filename) - 1);
        filename[sizeof(filename) - 1] = 0;
    }
    
    /* Save the game */
    if (save_game(g, filename, who) < 0)
    {
        printf("Error: Failed to save game to %s\n", filename);
        return 0;
    }
    
    printf("Game saved to %s\n", filename);
    return 1;
}

/*
 * Show list of save files and load selected one.
 * Returns 1 if game was loaded, 0 if cancelled.
 */
int tui_load_game(void)
{
    char line[256];
    FILE *pipe;
    int count = 0;
    char files[20][256]; /* Support up to 20 save files */
    int choice;
    
    printf("Available save files:\n");
    
    /* Use ls to find .rftg files */
    pipe = popen("ls -1 *.rftg 2>/dev/null", "r");
    if (pipe == NULL)
    {
        printf("No save files found.\n");
        return 0;
    }
    
    /* Read filenames */
    while (fgets(line, sizeof(line), pipe) != NULL && count < 20)
    {
        /* Remove newline */
        line[strcspn(line, "\n")] = 0;
        
        /* Skip autosave file */
        if (strcmp(line, "autosave.rftg") == 0)
            continue;
            
        strcpy(files[count], line);
        printf("%d. %s\n", count + 1, line);
        count++;
    }
    
    pclose(pipe);
    
    if (count == 0)
    {
        printf("No save files found.\n");
        return 0;
    }
    
    printf("Enter number to load (1-%d) or 0 to cancel: ", count);
    
    if (fgets(line, sizeof(line), stdin) == NULL)
    {
        return 0;
    }
    
    choice = atoi(line);
    
    if (choice == 0)
    {
        printf("Load cancelled.\n");
        return 0;
    }
    
    if (choice < 1 || choice > count)
    {
        printf("Invalid choice.\n");
        return 0;
    }
    
    /* We can't directly load here - we need to set up the restart mechanism */
    /* Store the filename to load in a global variable for rftg.c to use */
    extern char *load_filename;
    if (load_filename) free(load_filename);
    load_filename = strdup(files[choice - 1]);
    
    printf("Loading %s...\n", files[choice - 1]);
    
    return 1;
}

/*
 * Choose whether to discard a card for prestige.
 */
void tui_choose_discard_prestige(game *g, int who, int list[], int *num)
{
    int choice;
    char prompt[256];
    
    if (*num == 0)
    {
        /* No cards to discard */
        return;
    }
    
    sprintf(prompt, "Choose card to discard for prestige (or 0 to skip):");
    
    /* Display available cards */
    display_cards(g, list, *num, "Available cards to discard for prestige:");
    
    while (1)
    {
        printf("%s ", prompt);
        fflush(stdout);
        
        if (scanf("%d", &choice) != 1)
        {
            /* Clear invalid input */
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            printf("Invalid input. Please enter a number.\n");
            continue;
        }
        
        /* Skip discarding */
        if (choice == 0)
        {
            *num = 0;
            return;
        }
        
        /* Validate choice */
        if (choice >= 1 && choice <= *num)
        {
            /* Move selected card to front of list */
            int selected = list[choice - 1];
            list[0] = selected;
            *num = 1;
            return;
        }
        
        printf("Invalid choice. Please select 1-%d or 0 to skip.\n", *num);
    }
}

/*
 * Choose a world to takeover.
 */
int tui_choose_takeover(game *g, int who, int list[], int *num, int special[], int *num_special)
{
    int choice;
    
    if (*num == 0)
    {
        /* No takeover targets available */
        return 0;
    }
    
    /* Display available takeover targets */
    display_cards(g, list, *num, "Choose world to attempt takeover (or 0 to skip):");
    
    while (1)
    {
        printf("Enter world number to takeover (1-%d) or 0 to skip: ", *num);
        fflush(stdout);
        
        if (scanf("%d", &choice) != 1)
        {
            /* Clear invalid input */
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            printf("Invalid input. Please enter a number.\n");
            continue;
        }
        
        /* Skip takeover */
        if (choice == 0)
        {
            return 0;
        }
        
        /* Validate choice */
        if (choice >= 1 && choice <= *num)
        {
            return list[choice - 1];
        }
        
        printf("Invalid choice. Please select 1-%d or 0 to skip.\n", *num);
    }
}

/*
 * Choose which takeover to defend against.
 */
void tui_choose_defend(game *g, int who, int list[], int *num)
{
    int choice;
    
    if (*num == 0)
    {
        /* No defense options */
        return;
    }
    
    /* Display takeover attempts that can be defended */
    printf("Choose takeover to defend against (or 0 to allow all):\n");
    for (int i = 0; i < *num; i++)
    {
        printf("%d. Defend against takeover of %s\n", i + 1, g->deck[list[i]].d_ptr->name);
    }
    
    while (1)
    {
        printf("Enter choice (1-%d) or 0 to allow all takeovers: ", *num);
        fflush(stdout);
        
        if (scanf("%d", &choice) != 1)
        {
            /* Clear invalid input */
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            printf("Invalid input. Please enter a number.\n");
            continue;
        }
        
        /* Allow all takeovers */
        if (choice == 0)
        {
            *num = 0;
            return;
        }
        
        /* Validate choice */
        if (choice >= 1 && choice <= *num)
        {
            /* Move selected defense to front */
            int selected = list[choice - 1];
            list[0] = selected;
            *num = 1;
            return;
        }
        
        printf("Invalid choice. Please select 1-%d or 0 to allow all.\n", *num);
    }
}

/*
 * Choose which takeover to prevent.
 */
void tui_choose_takeover_prevent(game *g, int who, int list[], int *num, int special[])
{
    int choice;
    
    if (*num == 0)
    {
        /* No takeovers to prevent */
        return;
    }
    
    /* Display takeover attempts that can be prevented */
    printf("Choose takeover to prevent (or 0 to allow all):\n");
    for (int i = 0; i < *num; i++)
    {
        printf("%d. Prevent takeover of %s\n", i + 1, g->deck[list[i]].d_ptr->name);
    }
    
    while (1)
    {
        printf("Enter choice (1-%d) or 0 to allow all takeovers: ", *num);
        fflush(stdout);
        
        if (scanf("%d", &choice) != 1)
        {
            /* Clear invalid input */
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            printf("Invalid input. Please enter a number.\n");
            continue;
        }
        
        /* Allow all takeovers */
        if (choice == 0)
        {
            *num = 0;
            return;
        }
        
        /* Validate choice */
        if (choice >= 1 && choice <= *num)
        {
            /* Move selected prevention to front */
            int selected = list[choice - 1];
            list[0] = selected;
            *num = 1;
            return;
        }
        
        printf("Invalid choice. Please select 1-%d or 0 to allow all.\n", *num);
    }
}

/*
 * Choose whether to upgrade a world.
 */
void tui_choose_upgrade(game *g, int who, int list[], int *num, int special[], int *num_special)
{
    int choice;
    
    if (*num == 0)
    {
        /* No upgrade options */
        return;
    }
    
    /* Display worlds that can be upgraded */
    display_cards(g, list, *num, "Choose world to upgrade (or 0 to skip):");
    
    while (1)
    {
        printf("Enter world number to upgrade (1-%d) or 0 to skip: ", *num);
        fflush(stdout);
        
        if (scanf("%d", &choice) != 1)
        {
            /* Clear invalid input */
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            printf("Invalid input. Please enter a number.\n");
            continue;
        }
        
        /* Skip upgrade */
        if (choice == 0)
        {
            *num = 0;
            return;
        }
        
        /* Validate choice */
        if (choice >= 1 && choice <= *num)
        {
            /* Move selected world to front */
            int selected = list[choice - 1];
            list[0] = selected;
            *num = 1;
            return;
        }
        
        printf("Invalid choice. Please select 1-%d or 0 to skip.\n", *num);
    }
}

/*
 * Choose ante for gambling.
 */
int tui_choose_ante(game *g, int who, int list[], int num)
{
    int choice;
    
    /* Check for no cards available */
    if (num == 0)
    {
        printf("No cards available to ante.\n");
        return -1;
    }
    
    /* Display available cards */
    display_cards(g, list, num, "Choose a card to ante (0 to skip):");
    
    while (1)
    {
        fflush(stdout);
        
        if (scanf("%d", &choice) != 1)
        {
            /* Clear invalid input */
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            printf("Invalid input. Please enter a number.\n");
            continue;
        }
        
        /* Check for skip */
        if (choice == 0)
        {
            return -1;
        }
        
        /* Validate choice */
        if (choice >= 1 && choice <= num)
        {
            return list[choice - 1];
        }
        
        printf("Invalid choice. Please select 1-%d or 0 to skip: ", num);
    }
}

/*
 * Choose which cards to keep.
 */
void tui_choose_keep(game *g, int who, int list[], int *num, int min, int max)
{
    int choice, selected = 0;
    int temp_list[TEMP_MAX_VAL];
    int keep_list[TEMP_MAX_VAL];
    
    if (*num == 0)
    {
        return;
    }
    
    /* Copy original list */
    for (int i = 0; i < *num; i++)
    {
        temp_list[i] = list[i];
    }
    
    printf("Choose %d-%d cards to keep:\n", min, max);
    display_cards(g, temp_list, *num, "Available cards:");
    
    while (selected < max && selected < *num)
    {
        if (selected >= min)
        {
            printf("Selected %d cards. Enter card number to add (1-%d) or 0 to finish: ", 
                   selected, *num - selected);
        }
        else
        {
            printf("Selected %d cards (need at least %d). Enter card number to add (1-%d): ", 
                   selected, min, *num - selected);
        }
        
        fflush(stdout);
        
        if (scanf("%d", &choice) != 1)
        {
            /* Clear invalid input */
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            printf("Invalid input. Please enter a number.\n");
            continue;
        }
        
        /* Finish selection if minimum met */
        if (choice == 0 && selected >= min)
        {
            break;
        }
        
        /* Validate choice */
        if (choice >= 1 && choice <= (*num - selected))
        {
            /* Add to keep list */
            keep_list[selected] = temp_list[choice - 1];
            selected++;
            
            /* Remove from temp list */
            for (int i = choice - 1; i < *num - selected; i++)
            {
                temp_list[i] = temp_list[i + 1];
            }
            
            if (selected < max && selected < *num)
            {
                printf("Remaining options:\n");
                display_cards(g, temp_list, *num - selected, "");
            }
        }
        else
        {
            printf("Invalid choice.\n");
        }
    }
    
    /* Copy kept cards back to list */
    for (int i = 0; i < selected; i++)
    {
        list[i] = keep_list[i];
    }
    *num = selected;
}

/*
 * Choose world to produce on.
 */
void tui_choose_produce(game *g, int who, int list[], int *num)
{
    int choice;
    
    if (*num == 0)
    {
        return;
    }
    
    if (*num == 1)
    {
        /* Only one choice, automatically select it */
        return;
    }
    
    display_cards(g, list, *num, "Choose world to produce on:");
    
    while (1)
    {
        printf("Enter world number to produce on (1-%d): ", *num);
        fflush(stdout);
        
        if (scanf("%d", &choice) != 1)
        {
            /* Clear invalid input */
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            printf("Invalid input. Please enter a number.\n");
            continue;
        }
        
        /* Validate choice */
        if (choice >= 1 && choice <= *num)
        {
            /* Move selected world to front */
            int selected = list[choice - 1];
            list[0] = selected;
            *num = 1;
            return;
        }
        
        printf("Invalid choice. Please select 1-%d.\n", *num);
    }
}

/*
 * Choose cards to discard during produce phase.
 */
void tui_choose_discard_produce(game *g, int who, int list[], int *num, int discard)
{
    tui_choose_discard(g, who, list, num, discard);
}

/*
 * Choose type of good to search for.
 */
int tui_choose_search_type(game *g, int who)
{
    int choice, i;
    
    printf("Choose search category:\n");
    
    /* Display all available search categories */
    for (i = 0; i < MAX_SEARCH; i++)
    {
        printf("%d. %s\n", i + 1, search_name[i]);
    }
    
    while (1)
    {
        printf("Enter choice (1-%d): ", MAX_SEARCH);
        fflush(stdout);
        
        if (scanf("%d", &choice) != 1)
        {
            /* Clear invalid input */
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            printf("Invalid input. Please enter a number.\n");
            continue;
        }
        
        /* Validate and convert choice */
        if (choice >= 1 && choice <= MAX_SEARCH)
        {
            return choice - 1; /* Convert to 0-based index */
        }
        
        printf("Invalid choice. Please select 1-%d.\n", MAX_SEARCH);
    }
}

/*
 * Choose cards to keep after search.
 */
int tui_choose_search_keep(game *g, int who, int which, int category)
{
    card *c_ptr;
    int choice;
    
    /* Get card pointer */
    c_ptr = &g->deck[which];
    
    printf("\nCard found: %s\n", c_ptr->d_ptr->name);
    printf("Choose action:\n");
    printf("1. Keep card\n");
    printf("2. Discard (continue searching)\n");
    
    while (1)
    {
        printf("Enter choice (1-2): ");
        fflush(stdout);
        
        if (scanf("%d", &choice) != 1)
        {
            /* Clear invalid input */
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            printf("Invalid input. Please enter a number.\n");
            continue;
        }
        
        /* Validate choice */
        if (choice == 1)
        {
            return 1; /* Keep card */
        }
        else if (choice == 2)
        {
            return 0; /* Discard and continue searching */
        }
        
        printf("Invalid choice. Please select 1 or 2.\n");
    }
}

/*
 * Choose Oort Cloud kind.
 */
int tui_choose_oort_kind(game *g, int who)
{
    int choice;
    
    printf("Choose Oort Cloud kind:\n");
    printf("1. Novelty\n");
    printf("2. Rare\n");
    printf("3. Gene\n"); 
    printf("4. Alien\n");
    
    while (1)
    {
        printf("Enter choice (1-4): ");
        fflush(stdout);
        
        if (scanf("%d", &choice) != 1)
        {
            /* Clear invalid input */
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            printf("Invalid input. Please enter a number.\n");
            continue;
        }
        
        /* Validate and convert choice */
        if (choice >= 1 && choice <= 4)
        {
            return choice - 1; /* Convert to 0-based index */
        }
        
        printf("Invalid choice. Please select 1-4.\n");
    }
}
