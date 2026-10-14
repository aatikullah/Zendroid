# 🤖 Zendroid

**Zendroid** is a 2D action platformer built with the **iGraphics** library in C/C++. Guide a robot through five levels of platforms, moving slabs and fire-breathing dragons. Collect golden balls to open the portal, fight with your laser gun, and earn a place on the score board.

## Features
- **5 levels, 14 stages** with sub-levels, moving platforms and sliding stairs
- **Laser gun combat** against fireball-breathing dragons, with impact effects
- **Smooth sprite animations** for idle, walking, jumping and shooting
- **HUD** showing lives (Zeds), energy and points
- **Menus:** Main Menu, Load Game (level select), Settings, Sound, Credits, Pause and Game Over
- **High score board** saved to `scores.txt`
- **Background music and sound effects** with on/off options

## Tech Stack
| | |
|---|---|
| **Language** | C / C++ |
| **Library** | iGraphics (OpenGL / GLUT) |
| **IDE** | Visual Studio 2013 |
| **Platform** | Windows PC |
| **Window size** | 1400 × 1000 |

## Getting Started
1. Install **Visual Studio 2013** (the iGraphics library is included in this repo).
2. Clone or download the repository.
3. Open the `.sln` file via **File → Open → Project/Solution**.
4. Choose **Build → Build Solution**, then **Debug → Start Without Debugging**.

> **Note:** Keep the `Images` and `Audios` folders next to the source files and make sure `glut32.dll` is available, otherwise pictures and sounds won't load.

## How to Play

| Action | Keys |
|---|---|
| Move | `A` / `D` or `←` / `→` |
| Jump | `W` or `↑` |
| Shoot | Left Mouse Button (click or hold) |
| Main Menu | `Esc` |

Menus and the pause screen are mouse-controlled.

**Rules**
- Defeat the dragons: each one drops a golden ball. Collect them all to open the portal.
- Dragon fireballs cost you lives (Zeds), so dodge them.
- Walking drains energy, so keep moving and collect rewards.
- Blue balls give bonus points.
- Points, lives and energy carry over between stages.
- Lose all lives and it's game over. Finish the game to save your name and score.

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

## Contributors
| Name | ID |
|---|---|
| Gazi Md. Mohiman Abrar | 00725105101032 |
| Md Tausif Uddin Aishwarja | 00725105101045 |
| Md. Atik Ullah | 00725105101057 |


## Youtube Link
[Zendroid Gameplay Video](https://youtu.be/et_JY0fGsjM?si=XUhrBN8EL4GLD-Fu)

## Project Report
[Project Report: Zendroid](https://drive.google.com/file/d/10gkjWC-kxk-pxIzLS7BK-Ucsta4P6GHL/view?usp=drivesdk)
