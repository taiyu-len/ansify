/* ansify.c */
#include "ansify.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <stdlib.h>

//Characters
# define ESC "\33"
# define TOP "▀"
# define BOT "▄"
# define BLANK (0)

//Structs
struct image {
	unsigned char *data;
	int width;
	int height;
	int comp;
	int threshold;
	union ansify_colorkey key;
};
struct pixel {
	int x;
	int y;
	struct {
		int fg;
		int bg;
	} prev;
};
struct string {
	char *begin;
	char *end;
	char *capacity;
};

static struct string string_new(int initial);
static void string_put(struct string *s, char c);
static void string_append(struct string *s, const char* other);
static void string_sprintf(struct string *s, const char* format, ...);

static void write_pixel(struct string *, struct image *, struct pixel *);
static unsigned char convert_pixel(struct image const*, int x, int y);

char *ansify(const char* filename, union ansify_colorkey key, int threshold)
{
	struct image img;
	img.data = stbi_load(filename, &img.width, &img.height, &img.comp, 0);
	if (img.data == NULL) {
		return NULL;
	}
	img.key = key;
	img.threshold = threshold;

	struct string str = string_new(img.width * img.height);
	struct pixel p = {0, 0, .prev = {-30, -30}};
	for (; p.y < img.height; p.y += 2) {
		for (p.x = 0; p.x < img.width; ++p.x) {
			write_pixel(&str, &img, &p);
		}
	}
	stbi_image_free(img.data);
	return str.begin;
}

void write_pixel(struct string *string, struct image *image, struct pixel *pixel)
{
	//foreground, or upper pixel
	int fg = convert_pixel(image, pixel->x, pixel->y);
	//background or lower pixel
	int bg = pixel->y + 1 < image->height ? convert_pixel(image, pixel->x, pixel->y + 1) : BLANK;

	int newfg = fg != pixel->prev.fg && bg != fg;
	int newbg = bg != pixel->prev.bg;
	int swap  = fg == 0 && fg != bg;

	if (swap) {
		int tmp = fg;
		fg = bg;
		bg = tmp;
	}
	// write colors
	if (newfg || newbg) {
		string_append(string, "\033[");
		if (newfg) {
			string_sprintf(string, "38;5;%u%s", fg, newbg ? ";" : "");
		}
		if (newbg) {
			if (bg == BLANK) {
				string_append(string, "49");
			} else {
				string_sprintf(string, "48;5;%u", bg);
			}
		}
		string_put(string, 'm');
	}

	if (newfg) {
		pixel->prev.fg = fg;
	}
	if (newbg) {
		pixel->prev.bg = bg;
	}

	// write text
	if (fg == bg) {
		string_put(string, ' ');
	} else {
		string_append(string, swap ? BOT : TOP);
	}

	if (pixel->x + 1 >= image->width) {
		string_append(string, "\033[0m\n");
		pixel->prev.bg = 0;
		pixel->prev.fg = -10;
	}

}

unsigned char
convert_pixel(struct image const* image, int x, int y)
{
# define COMP(n) (image->data[image->comp*(x+y*image->width)+(n)])
	//get components
	int rgba[4] = {
		COMP(0),
		COMP(1),
		COMP(2),
		image->comp == 4 ? COMP(3) : 0xFF
	};
	int bw = (rgba[0] + rgba[1] + rgba[2]) / 3;
# undef COMP
	if (rgba[3] < 99) {
		return BLANK;
	}
	for (int i = 0; i < 3; ++i) {
		if (abs(rgba[i] - image->key.rgb[i]) <= image->threshold) {
			return BLANK;
		}
	}

	// compare with black/white values
	int i;
	for (i = 0; i < 3; ++i) {
		if (abs(rgba[i] - bw) >= (256/24)) {
			break;
		}
	}
	if (i == 3) {
		return 0xE8 + bw * 24 / 256;
	}

	int pixel = 16;
	pixel += 36 * (6* rgba[0] / 256);
	pixel +=  6 * (5* rgba[1] / 256);
	pixel +=      (5* rgba[2] / 256);
	return pixel;
}

struct string string_new(int initial)
{
	char *s = malloc(initial);
	return (struct string) {
		.begin = s,
		.end   = s,
		.capacity = s+initial
	};
}
static
void string_grow(struct string *s, size_t minimum)
{
	int capacity = s->capacity - s->begin;
	int size     = s->end - s->begin;
	if (capacity < minimum) {
		capacity = minimum;
	}
	s->begin = realloc(s->begin, capacity *= 2);
	s->end = s->begin + size;
	s->capacity = s->begin + capacity;
}
void string_put(struct string *s, char c)
{
	if (s->end + 1 >= s->capacity) {
		string_grow(s, 2);
	}
	*s->end = c;
	*++s->end = '\0';
}
void string_append(struct string *s, const char* other)
{
	int len = strlen(other);
	if (s->end + len >= s->capacity) {
		string_grow(s, len);
	}
	strncpy(s->end, other, len + 1);
	s->end += len;
}
void string_sprintf(struct string *s, const char* format, ...)
{
	va_list ap;
	va_start(ap, format);
	int len = vsnprintf(NULL, 0, format, ap);
	va_end(ap);

	if (len < 0) {
		return;
	}
	if (s->end + len >= s->capacity) {
		string_grow(s, len+1);
	}

	va_start(ap, format);
	len = vsnprintf(s->end, len+1, format, ap);
	va_end(ap);
	s->end += len;
}
