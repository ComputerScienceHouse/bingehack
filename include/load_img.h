
/* ------------------------------------------- */
#define XIMG      0x58494D47

/* Header of GEM Image Files   */
typedef struct IMG_HEADER{
  short version;  /* Img file format version (1) */
  short length;   /* Header length in words  (8) */
  short planes;   /* Number of bit-planes    (1) */
  short pat_len;  /* length of Patterns      (2) */
  short pix_w;    /* Pixel width in 1/1000 mmm  (372)    */
  short pix_h;    /* Pixel height in 1/1000 mmm (372)    */
  short img_w;    /* Pixels per line (=(x+7)/8 Bytes)    */
  short img_h;    /* Total number of lines               */
  long  magic;    /* Contains "XIMG" if standard color   */
  short paltype;  /* palette type (0=RGB (short each)) */
  short *palette;	/* palette etc.                        */
  char *addr;     /* Address for the depacked bit-planes */
} IMG_header;

/* ------------------------------------------- */
/* error codes */
#define ERR_HEADER      1
#define ERR_ALLOC       2
#define ERR_FILE        3
#define ERR_DEPACK      4
#define ERR_COLOR       5

/* Speichert die aktuelle Farbpalette mit col Farben in palette ab */
void get_colors(int handle, short *palette, int col);

/* Setzt col Farben aus der Farbpalette palette */
void img_set_colors(int handle,short *palette, int col);

/* Konvertiert MFDB von Standard in Ger^Äteformat (0 if succeded, else error). */
int convert(MFDB *, long );

/* Transformiert das Bild in die Farbtiefe des VDI-Device */
int transform_img(MFDB *);

/* Loads & depacks IMG (0 if succeded, else error). */
/* Bitplanes are one after another in address IMG_HEADER.addr. */
int depack_img(char *, IMG_header *);

/* Erstellt ein halb so gro^Þes IMG im Standardformat!*/
void half_img(MFDB *,MFDB *);
