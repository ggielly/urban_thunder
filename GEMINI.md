# Gemini Code Assistant Workspace Context

This document provides a comprehensive technical overview of the "Urban Thunder" project, a Road Rash-style motorcycle racing game for the SEGA Mega Drive/Genesis. It is intended to be used by the Gemini Code Assistant to understand the project's architecture, codebase, and development workflows.

## Project Overview

"Urban Thunder" is a pseudo-3D racing game that aims to replicate the high-speed, combat-focused gameplay of the classic "Road Rash" series on original SEGA Mega Drive hardware. The project is built using the SEGA Genesis Development Kit (SGDK) and features a custom-built engine optimized for the Motorola 68000 processor.

**Key Technical Goals:**

*   **High-Performance Pseudo-3D Engine:** Achieve a stable 60 FPS frame rate with a convincing sense of depth and speed.
*   **Advanced AI Opponents:** Implement multiple AI personalities with distinct driving styles, combat behaviors, and adaptive difficulty.
*   **Dynamic Track System:** Define complex and varied race tracks using a compact, parametric segment system.
*   **Optimized Resource Management:** Efficiently manage the Mega Drive's limited memory (64KB RAM, 64KB VRAM) and processing power.

## Core Architecture

The game engine is composed of several key systems that work together to create the racing experience.

### 1. Pseudo-3D Road Renderer

The road is rendered using a scanline-based, pseudo-3D technique. The track is divided into horizontal strips, each representing a different distance from the camera. By manipulating the width, horizontal offset, and color palette of each strip, the engine creates the illusion of a winding, undulating road.

*   **`road_engine.s`**: This assembly file contains the highly optimized, low-level code responsible for rendering the road strips directly to VRAM.
*   **`main.c`**: The main game loop in this file orchestrates the rendering process, calling the assembly functions and updating the road state each frame.

### 2. Track Segment System

Race tracks are not stored as large, pre-rendered assets. Instead, they are defined as a series of `TrackSegment` structs. Each segment defines a piece of the track with properties like curvature, hill height, length, and visual style. The engine dynamically generates the road geometry at runtime by interpolating between these segments.

*   **`src/main.c`**: The `TrackSegment` struct and the logic for processing the track data are located in this file.

### 3. AI System

The AI is a critical component of the gameplay. The system is designed to be flexible and extensible, allowing for a variety of opponent behaviors.

*   **AI Personality Types:** The `AIType` enum defines several distinct AI personalities, such as `AI_AGGRESSIVE`, `AI_DEFENSIVE`, and `AI_RUBBER_BAND`.
*   **State Machine:** Each AI rider operates on a finite state machine (`AIState`) that governs its current behavior (e.g., `AI_STATE_RACING`, `AI_STATE_ATTACKING`).
*   **`ai_riders.c` / `ai_riders.h`**: These files contain the core logic for the AI system, including the state machine, decision-making processes, and data structures for the AI riders.
*   **`ai_physics.s`**: This assembly file provides optimized physics calculations for the AI riders, ensuring that their movements are both fast and believable.

## Development Workflow

### Build System

The project uses a `Makefile` to automate the build process. The Makefile is configured to work with the SGDK and provides several useful targets:

*   **`make all`**: Compiles the project and generates a release-ready ROM file.
*   **`make debug`**: Creates a debug build with additional logging and error-checking.
*   **`make clean`**: Removes all generated files.

### Tools

The `tools/` directory contains several Python scripts used to generate game assets:

*   **`generate_simple_images.py`**: Creates simple image assets for testing and prototyping.
*   **`generate_sgdk_tiles.py`**: Converts images into a format that can be used by the SGDK.

### Debugging

The `DEBUG_GUIDE.md` file provides instructions on how to set up a debugging environment and use the available debugging tools. The game includes a built-in debug mode that can be enabled in the `Makefile`. When active, this mode displays real-time information about the game state, such as the number of active AI riders, their current states, and other performance metrics.

## How to Get Help

This `GEMINI.md` file is your primary source of information about the project. If you need to understand a specific part of the code, start by looking for the relevant section in this document. You can also use the `search_file_content` tool to find keywords and function names within the codebase.

If you are asked to modify the code, be sure to follow the existing coding style and conventions. Use the `read_file` and `read_many_files` tools to examine the surrounding code before making any changes.
