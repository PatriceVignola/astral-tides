# Astral Tides #

![Astral Tides banner](https://patricevignola.com/img/astral-tides-banner.png? "Astral Tides")

## Important Links ##
* [Visit the website](https://patricevignola.com/project/astral-tides)
* [Watch the video](https://www.youtube.com/watch?v=CusInfM1h98)
* [Download the game](https://github.com/PatriceVignola/Builds/releases/download/windows/astral-tides.zip)

## Description ##
Astral Tides is a game made in 10 weeks as a part time project for the 2016 edition of the Ubisoft Montreal Game Lab Competition. The objective of the contest was to create a game with the theme `Ocean'. Furthermore, we were required to have 2 systems and 3 mechanics that interact with each other, as well as physics, AI, checkpoints and at least 10 minutes of gameplay.

The game won Best Art Direction and the most nominations among all the games (6 out of 8 categories):
* Best Prototype
* Best Game Design
* Best Quality of the "3Cs" (Camera, Control, Character)
* Best User Experience
* Best Creativity and Integration of the Theme
* Best Art Direction

## Technology Stack ##
* Unreal Engine 4.11
* Visual Studio (C++)
* Wwise
* Perforce

<b>Important: A special patch for Unreal Engine 4.11 that fixes a bug with Visual Studio 2015 has to be applied to build this project. The executable is available at the root of the project and via AWS:
https://s3.amazonaws.com/unrealengine/qfe/QFE-FromCL-3044413.exe</b>

## Features I implemented ##
* Water Physics
* CCC (Camera, Character, Controls)
* Push And Pull Mechanics
* Game Loop
* Level Streaming
* Interactive Main Menu
* HUD Integration
* Build Automation

## Note ##
The version control software we used during the development of Astral Tides is Perforce. Therefore, the entire commit history is on the Perforce server instead of git. Also, this repository contains only the code for the game; the assets belong to the artists and will not be migrated to the git repository.
