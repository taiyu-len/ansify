/* ansify.c */
# include <stdio.h>
# include <getopt.h>
# include <errno.h>
# include <unistd.h>
# define STB_IMAGE_IMPLEMENTATION
# include "stb_image.h"

//Characters
# define ESC "\33"
# define TOP "▀"
# define BOT "▄"

# define BLANK 0

//Structs
struct image {
  unsigned char *data;
  int  width;
  int  height;
  int  comp;
};
struct pixel {
  int x,
      y;
  struct {
    int fg,
        bg;
  } prev;
};

//Functions
static void
print_pixel(struct image*, struct pixel *);

//color key. set to impossible value initially
static int key[3] = {-999, -999, -999};
static int thresh = 10;

int
main ( int argc, char *argv[])
{
  int c,
      delay = 0;

  while((c = getopt(argc, argv, "k:t:d:")) != -1)
    switch(c) {
      case 'k'://colorkey
        if(sscanf(optarg,"%x:%x:%x",key, key+1, key+2) < 3) {
          fprintf(stderr,"invalid  input. -k %%x:%%x:%%x\n");
          key[0] = key[1] = key[2] = -999;
          continue;
        }
      case 't'://threshhold
        thresh = strtol(optarg,NULL, 10);
        if(errno) {
          fprintf(stderr,"invalid input. -t %%d\n");
          thresh = 10;
          continue;
        }
      case 'd'://threshhold
        delay = strtol(optarg,NULL, 10);
        if(errno) {
          fprintf(stderr,"invalid input. -t %%d\n");
          thresh = 10;
          continue;
        }
      default: break;
    }

  while(argv[optind]) {
    struct pixel pixel = {0, 0, .prev = {0, 0}};
    struct image orig;

    //Load image
    if((orig.data = stbi_load(argv[optind++]
        , &orig.width, &orig.height, &orig.comp, 0)) == NULL) {
      if(errno){ perror(NULL); errno=0;}
      else       fputs("Invalid image file\n\n", stderr);
      continue;
    }
    for(; pixel.y < orig.height; pixel.y+=2, delay?usleep(delay):0)
      for(pixel.x = 0; pixel.x < orig.width; ++pixel.x)
        print_pixel(&orig, &pixel);

    stbi_image_free(orig.data);
  }
  return EXIT_SUCCESS;
}

static unsigned char
convert_pixel(struct image *image, int x, int y)
{
# define comp(n) image->data[image->comp*(x+y*image->width)+n]
  //get components
  int r = comp(0)
    , g = comp(1)
    , b = comp(2)
    , a = image->comp == 4 ? comp(3) : 0xFF,
      bw = (r + g + b)/3;

  if( a < 2 ||(abs(r-key[0]) <= thresh
            && abs(g-key[1]) <= thresh
            && abs(b-key[2]) <= thresh))
    return BLANK;
  //Check for grey scale, and calculate it better
  if( abs(r-bw) < (256/24)
      && abs(g-bw) < (256/24)
      && abs(b-bw) < (256/24))
    return 0xE8 + bw * 24 / 256;
  return 16 + 36*(r*6/256) + 6*(g*6/256) + (b*6/256);
# undef comp
}

void
print_pixel(struct image *image, struct pixel *pixel)
{
  //foreground, or upper pixel
  int fg = convert_pixel(image, pixel->x, pixel->y)
  //background or lower pixel
    , bg = pixel->y + 1 < image->height
           ? convert_pixel(image, pixel->x, pixel->y + 1)
           : BLANK
  //if there is a new fg, only bg is used if both are the same, so fg remains
  //unchanged
    , newfg = fg != pixel->prev.fg && bg != fg
  //If ther is a new bg
    , newbg = bg != pixel->prev.bg
    , swap  = fg == 0 && fg != bg;

  if(swap) {
    int tmp = fg; fg  = bg; bg  = tmp;
  }

  if(newfg || newbg) {
      printf("\033[");
      if(newfg) {
        printf("38;5;%u", fg);
      }
      if(newbg) {
        if(newfg) putchar(';');
        if(bg == BLANK)
          printf("49");
        else
          printf("48;5;%u", bg);
      }
      putchar('m');
  }
  if(newfg) pixel->prev.fg = fg;
  if(newbg) pixel->prev.bg = bg;

  //deterimine What to draw
  if(fg == bg)
    putchar(' ');
  else
    printf(swap ? BOT : TOP);

  if(pixel->x+1 >= image->width) {
    printf("\033[0m");
    if(pixel->y+2 >= image->height)
      putchar('\n');
    putchar('\n');
    pixel->prev.bg = pixel->prev.fg = 0;
  }
}


