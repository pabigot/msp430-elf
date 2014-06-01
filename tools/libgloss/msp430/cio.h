#ifndef CIO_BUF_SIZE
#define CIO_BUF_SIZE 64
#endif

typedef struct {
  unsigned char length[2];
  unsigned char parms[9];
  unsigned char buf[CIO_BUF_SIZE];
} __CIOBUF__TYPE__;

extern __CIOBUF__TYPE__ __CIOBUF__;

extern void _libgloss_cio_hook (void);

#define CIO_OPEN    (0xF0)
#define CIO_CLOSE   (0xF1)
#define CIO_READ    (0xF2)
#define CIO_WRITE   (0xF3)
#define CIO_LSEEK   (0xF4)
#define CIO_UNLINK  (0xF5)
#define CIO_GETENV  (0xF6)
#define CIO_RENAME  (0xF7)
#define CIO_GETTIME (0xF8)
#define CIO_GETCLK  (0xF9)
#define CIO_SYNC    (0xFF)

