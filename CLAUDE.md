# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a text-based implementation of the card game "Race for the Galaxy" with an AI player. It's forked from Keldon Jones' RFTG AI and includes a terminal user interface (TUI) for playing against computer opponents.

## Build System

The project uses a Makefile for building:

- `make` or `make release` - Build optimized release version
- `make debug` - Build debug version with symbols
- `make clean` - Clean build artifacts
- `make windows` - Cross-compile for Windows using MinGW

The main executable is `rftg` and requires `cards.txt` and the `network/` directory to run.

## Architecture

### Core Game Engine
- `engine.c` - Main game logic and rule enforcement
- `rftg.h` - Central header with all game structures, constants, and function declarations
- `init.c` - Game initialization and card loading from `cards.txt`
- `ai.c` - AI decision-making logic

### User Interface
- `tui.c` - Text-based user interface implementation with full expansion 3 support
- `tui.h` - TUI function declarations
- `rftg.c` - Main program entry point and game loop

#### Expansion 3 (The Brink of War) Support
The TUI now fully supports prestige actions from expansion 3:
- **Search Action**: Search deck for specific card categories (uses prestige action, no cost)
- **Prestige Boosts**: Enhanced actions that cost 1 prestige point + prestige action
- **Prestige Display**: View prestige points and action status with 'v' command
- Both regular and advanced game modes support prestige functionality

### Game State Management
- `loadsave.c` - Save/load game functionality with autosave support
- Choice logging system for undo/redo functionality throughout gameplay

### Data Files
- `cards.txt` - Card definitions and game data (required at runtime)
- `campaign.txt` - Campaign scenarios
- `network/` - Neural network files for AI evaluation

## Key Data Structures

### Game Structure (`struct game`)
Central game state containing:
- Player information (`player p[MAX_PLAYER]`)
- Card deck (`card deck[MAX_DECK]`)
- Game settings (expansions, goals, takeovers)
- Current phase and turn information

### Card System
- `design` struct - Card templates with powers and attributes
- `card` struct - Individual card instances with state
- `power` struct - Card abilities by game phase

### AI System
- Uses neural networks for evaluation (files in `network/`)
- Implements minimax-style decision making
- Supports different difficulty levels via evaluation functions

## Command Line Options

- `-p <num>` - Number of players (2-6)
- `-e <level>` - Expansion level (0-4)
- `-n <name>` - Player name
- `-a` - Advanced 2-player game
- `-r <seed>` - Custom random seed
- `-s <file>` - Load saved game
- `-c <campaign>` - Play campaign
- `-g` / `-nog` - Enable/disable goals
- `-t` / `-not` - Enable/disable takeovers

## Development Notes

### Adding New Features
- Game rules are primarily in `engine.c`
- UI interactions go in `tui.c` 
- New card powers require updates to the power evaluation system
- The choice logging system must be maintained for undo functionality

### Testing
No automated test framework exists. Testing is done by playing games and verifying correct rule enforcement.

### Game Phases
The game follows a strict phase system (explore, develop, settle, consume, produce) with power resolution at each phase. Understanding this flow is crucial for any modifications.