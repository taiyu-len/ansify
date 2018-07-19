#pragma once
#ifndef ANSIFY_H
#define ANSIFY_H
union ansify_colorkey {
	struct { int r, g, b; };
	int rgb[3];
};
char *ansify(const char*filename, union ansify_colorkey key, int threshold);
#endif
