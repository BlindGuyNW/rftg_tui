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

#ifndef TUI_H
#define TUI_H

#include "rftg.h"

#define TEMP_MAX_VAL 100
// Function declarations
int get_card_choice(game *g, int who, int list[], int num, const char *prompt);
void display_cards(game *g, int list[], int num, const char *message);
	void display_card_info(game *g, int card_index);
	void tui_choose_discard(game *g, int who, int list[], int *num, int discard);
	void tui_choose_action(game *g, int who, int action[2], int one);
    int tui_choose_lucky(game *g, int who);
    int tui_choose_place(game *g, int who, int list[], int num, int phase, int special);
    void tui_choose_pay(game *g, int who, int which, int list[], int *num, int special[], int *num_special, int mil_only, int mil_bonus);
    void tui_choose_consume_hand(game *g, int who, int c_idx, int o_idx, int list[], int *num);
    void tui_choose_consume(game *g, int who, int cidx[], int oidx[], int *num,
                            int *num_special, int optional);
    void tui_choose_good(game *g, int who, int c_idx, int o_idx, int goods[],
                         int *num, int min, int max);
    void tui_choose_windfall(game *g, int who, int list[], int *num);
    void tui_choose_trade(game *g, int who, int list[], int *num, int no_bonus);
    void display_hand_card(game *g, int who, int position);
    void display_tableau(game *g, int who);
    void display_tableau_card(game *g, int who, int position);
    void display_vp(game *g);
    void display_military(game *g);
    void display_hand(game *g, int who);
    extern char *get_card_power_name(int i, int p);
    int tui_new_game_menu(options *opt);
#endif
