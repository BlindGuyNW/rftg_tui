# What is this?

This is a text-based version of the card game [Race for the Galaxy](https://www.riograndegames.com/games/race-for-the-galaxy/). Specifically, it is an AI designed to play the game at a high level, enforcing all the rules. This AI is similar to, though not identical with, the version included in the official digital game on Steam.

## Where did the text-based Version Come From?

This project was based on the RFTG AI released by [Keldon Jones](http://keldon.net/rftg/). Specifically, this is a fork of the [web front-end](https://github.com/nutki/RFTG_WebApp) developed by Nutki, AKA Michał Szafrański.
This text-based UI is the work of Zachary Kline, and features some code inspired by the ChatGPT OpenAI project. 

## What is the current state of the text interface?

The text interface currently implements enough of the game to play base RFTG without expansions. Some of the GUI features, such as undo and network play, are not yet supported. The two-player advanced game is not yet available, though it is hoped implementing this will be fairly easy. It is possible to play RFTG with 2 to 4 computer opponents, view card information, player status, and the like. The AI is as tough as the graphical version.

## Commands

* ?: Display help.
* Q: quit the game. Be warned this currently doesn't prompt for confirmation, though since the game autosaves by default it's easy to. restart from where you left off.
* R: re-display the current list of choices.
* H: Display the player's hand. With a number, display information about that card number from the hand list.
* I: Display information about cards in the most recent choice prompt. This is useful for situations such as explore results, and generally obtaining info about currently legal plays.
* V: Display victory point information. This breaks down all player's victory points by category, and displays a running total.
* M: Military information. This provides info on a player's military strength, including things like temporary bonuses against rebels.
* T: Tableau display. This is similar to the hand information but can be used to get info about current cards in play.
The syntax is: "T <player-number> <card-number>", for instance "t 1 2" gets info about player 2's second card. Note that with a player number alone this shows that player's tableau list. Separate the numbers with spaces.

Finally, to make choices after prompting type  numbers. Note that in some cases it is possible to press 0 to pass. This is not always indicated as well as it might be at the moment.

## Current Limitations

At the moment, the biggest limitation of the game display is the inability to get information about the victory points awarded for 6-cost developments. They are notated as "special," when viewed, but the detailed display does not explain how they are scored. An external source of information is probably required at this point if you are worried about this. Rest assured I plan to make this a little more understandable as soon as I can figure out how to do so.

## Game Rules

You can find an accessible version of the RFTG rulebook at [this page on UltraBoardGames](https://www.ultraboardgames.com/race-for-the-galaxy/game-rules.php). A PDF is also available from Rio Grande games. The text-based UI does not explain how to play, so you'll want to read up a bit on this before diving in.

I hope you enjoy this game. I am happy to answer any questions or help out however I can.