# Zendroid

## Game Description

**Zendroid** is a 2D action platformer created using the **iGraphics** library in C/C++ (OpenGL + GLUT). You control a robot through five levels full of platforms, moving slabs and fire-breathing dragons. Collect golden balls to open the portal, fight dragons with your laser gun, and survive to the end to earn a place on the score board.

## Features
- 5 levels, with extra sub-levels in Levels 1, 2, 3 and 5 (14 stages in total).
- Smooth sprite animations for idle, walking, jumping and shooting.
- Laser gun combat against dragons, with impact effects.
- Dragons that attack with fireball breath.
- Moving platforms and stairs that slide left, right, up and down.
- Collect golden balls to open the portal to the next stage.
- Lives (Zeds), energy bar and points shown on the HUD.
- Main menu, Load Game (level select), Settings, Sound options and Credits screens.
- High score board that saves player names and points (`scores.txt`).
- Pause menu (resume / restart / exit) and a game-over screen.
- Background music and sound effects, which can be switched on or off.

## Project Details
IDE: Visual Studio 2013

Language: C, C++

Library: iGraphics (OpenGL / GLUT)

Platform: Windows PC

Genre: 2D action platformer

Window size: 700 x 500

## How to Run the Project

Make sure you have the following installed:
- **Visual Studio 2013**
- **iGraphics Library** (included in this repository)

Steps:
- Clone or download this repository.
- Open Visual Studio 2013.
- Go to File → Open → Project/Solution.
- Select the .sln file from the cloned repository.
- Click Build → Build Solution.
- Run the program by clicking Debug → Start Without Debugging.

> **Note:** Keep the `Images` and `Audios` folders next to the source files, and make sure `glut32.dll` is available, otherwise the game will not load its pictures and sounds.

## How to Play

### **Controls**
| Action | Keys |
|---|---|
| Move Left | `A` or `←` (Left Arrow) |
| Move Right | `D` or `→` (Right Arrow) |
| Jump | `W` or `↑` (Up Arrow) |
| Attack / Shoot | Left Mouse Button (hold or click) |
| Back to Main Menu | `Esc` |

Menus and the pause screen are used with the mouse.

### **Game Rules**

- Each level has dragons that you must defeat.
- Every dragon you defeat drops a golden ball.
- Collect all the golden balls to open the portal and move to the next stage.
- Dragon fireballs take away your lives (Zeds), so dodge them.
- Walking uses up your energy, so keep moving forward and collect rewards.
- Blue balls give you extra points.
- Points, lives and energy carry over from one stage to the next.
- If you lose all your lives, the game is over and you can restart or exit.
- Finish the game, enter your name and your score is saved to the score board.

## Project Contributors

1. Gazi Md. Mohiman Abrar (ID: 00725105101032)
2. Md Tausif Uddin Aishwarja (ID: 00725105101045)
3. Md. Atik Ullah (ID: 00725105101057)


## Screenshots

### **Menu**
<img src="https://github.com/user-attachments/files/33005755/mainmenu.bmp" width="300">

### **Level 1 — Platforming and Collectibles**
<img width="734" height="525" alt="image" src="https://github.com/user-attachments/assets/9ecb31d0-c207-4299-9817-599d372a683b" />

### **Gameplay**
<img src="ADD_GAMEPLAY_SCREENSHOT_LINK_HERE" width="300">


## Youtube Link
[Zendroid Gameplay Video](https://youtu.be/et_JY0fGsjM?si=XUhrBN8EL4GLD-Fu)

## Project Report
[Project Report: Zendroid](https://drive.google.com/drive/u/1/my-drive)
