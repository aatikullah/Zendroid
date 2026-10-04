# Zendroid

## Game Description

**Zendroid** is a 2D action platformer created using the **iGraphics** library in C/C++. You control a robot through five levels full of platforms, moving slabs and fire-breathing dragons. Collect golden balls to open the portal, fight dragons with your laser gun, and survive to the end to earn a place on the score board.

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

Window size: 1400 x 1000

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

#### **Menu**
<img src="https://github.com/user-attachments/files/33005755/mainmenu.bmp" width="300" height="200">

#### **Level Selection (Load Game) Screen**
<img width="325" height="200" alt="image" src="https://github.com/user-attachments/assets/d7b22742-0a34-4dfa-934f-d1a5d6cb11d7" />

#### **Settings Menu (Sound and Score)**
<img width="325" height="200" alt="image" src="https://github.com/user-attachments/assets/9076a24a-15d2-413f-88a3-a9a4814a6b89" />

#### **Score Board**
<img width="325" height="200" alt="image" src="https://github.com/user-attachments/assets/17b492df-dc01-442a-ba49-943dff364c68" />


### **Gameplay**

#### **Level 1 — Platforming and Collectibles**
<img width="325" height="200" alt="image" src="https://github.com/user-attachments/assets/9ecb31d0-c207-4299-9817-599d372a683b" />

#### **Level 2 — Fireball-Throwing Dragons**
<img width="325" height="200" alt="image" src="https://github.com/user-attachments/assets/12496482-1435-4d1d-83ab-a001c8aaf919" />

#### **Level 3 — Sub-level 1, Ember Blocks and Fireball-Throwing Dragons**
<img width="325" height="200" alt="image" src="https://github.com/user-attachments/assets/2b846d73-7a1c-400f-bf47-61a02e664a04" />

#### **Level 4 — Moving Slabs and the Laser Gun**
<img width="325" height="200" alt="image" src="https://github.com/user-attachments/assets/f1f2e185-ac2b-47b2-8b72-b7f2821524cb" />

#### **Level 5 — Final Stage — The Boss Dragon**
<img width="325" height="200" alt="image" src="https://github.com/user-attachments/assets/e5745adf-2d72-4d92-8ebc-a9e4a71c9dd3" />

#### **Game-Over Screen**
<img width="325" height="200" alt="image" src="https://github.com/user-attachments/assets/6beaebe8-8286-4cdf-82cf-077cdbbdb7ec" />

#### **Victory Screen**
<img width="325" height="200" alt="image" src="https://github.com/user-attachments/assets/83c246c6-9274-4d44-88c6-98fbb6df19e1" />


## Youtube Link
[Zendroid Gameplay Video](https://youtu.be/et_JY0fGsjM?si=XUhrBN8EL4GLD-Fu)

## Project Report
[Project Report: Zendroid](https://drive.google.com/drive/u/1/my-drive)
