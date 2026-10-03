#ifndef GAME_OBJECTS_H
#define GAME_OBJECTS_H

struct Button {
	int x1, y1, x2, y2;
};

static bool isInside(Button b, int mx, int my) {
	return (mx >= b.x1 && mx <= b.x2 && my >= b.y1 && my <= b.y2);
}

struct Platform {
	int x1, y1, x2, y2;
};
struct Dragon {
	int x, y;
	int width, height;
	int animFrame;
	int animTimer;
};

#endif