
COMMON_SRC = hex-utils.c   \
	     bsp.c         \
	     shared-data.c \
	     bsp_if.c      \
	     breakpoint.c  \
	     irq.c         \
	     irq-rom.c     \
	     syscall.c     \
	     queue.c       \
	     sysinfo.c     \
	     debug-io.c    \
	     console-io.c  \
	     bsp_cache.c   \
	     printf.c      \
	     vprintf.c     \
	     sprintf.c     \
	     bsp_reset.c

SYSLIB_SRC = open.c         \
	     close.c        \
	     exit.c         \
	     lseek.c        \
	     print.c        \
	     read.c         \
	     write.c        \
	     sbrk.c         \
	     getpid.c       \
	     fstat.c        \
	     isatty.c       \
	     kill.c         \
	     unlink.c       \
	     raise.c        \
	     gettimeofday.c \
	     times.c
	    
THREADS_SRC = gdb-threads.c \
	      threads-syscall.c

GDBSTUB_SRC = gdb.c      \
	      gdb-data.c

NET_SRC = arp.c    \
	  bootp.c  \
	  cksum.c  \
	  enet.c   \
	  icmp.c   \
	  ip.c     \
	  net.c    \
	  pktbuf.c \
	  socket.c \
	  tcp.c    \
	  timers.c \
	  udp.c
