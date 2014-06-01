/*
 * net.h -- Network definitions for Cygnus BSP
 *
 * Copyright (c) 1998, 1999, 2000 Cygnus Support
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 */
extern unsigned long _bsp_ms_ticks(void);
#define MS_TICKS() _bsp_ms_ticks()

/* #define NET_SUPPORT_RARP  1 */
#define NET_SUPPORT_ICMP 1
#define NET_SUPPORT_UDP  1
#define NET_SUPPORT_TCP  1
/*#define NET_SUPPORT_TFTP 1*/

#ifdef __LITTLE_ENDIAN__
#define ntohl(x) \
    ((unsigned long int)((((unsigned long int)(x) & 0x000000ffU) << 24) | \
    (((unsigned long int)(x) & 0x0000ff00U) <<  8) | \
    (((unsigned long int)(x) & 0x00ff0000U) >>  8) | \
    (((unsigned long int)(x) & 0xff000000U) >> 24)))

#define ntohs(x) \
	((unsigned short int)((((unsigned short int)(x) & 0x00ff) << 8) | \
			      (((unsigned short int)(x) & 0xff00) >> 8)))

#else
#define	ntohl(x)	(x)
#define	ntohs(x)	(x)
#endif

#define	htonl(x)	ntohl(x)
#define	htons(x)	ntohs(x)

/*
 * Minimum ethernet packet length.
 */
#define ETH_MIN_PKTLEN  60
#define ETH_MAX_PKTLEN  1514

typedef unsigned char enet_addr_t[6];
typedef unsigned char ip_addr_t[4];

typedef unsigned char  octet;
typedef unsigned short word;
typedef unsigned int   dword;


/*
 * Simple timer support structure.
 */
typedef void (*tmr_handler_t)(void *user_data);

/*
 * Timer structure.
 * When expiration time is met or exceeded, the handler is
 * called and the timer struct is removed from the list.
 */
typedef struct _timer {
    struct _timer *next;        /* next timer in list */
    unsigned long delay;	/* expiration time relative to start time */
    unsigned long start;	/* when the timer was set */
    tmr_handler_t handler;      /* user procedure to call when timer 'fires' */
    void          *user_data;   /* user pointer passed to above procedure */
} timer_t;


/*
 * Ethernet header.
 */
typedef struct {
    enet_addr_t   destination;
    enet_addr_t   source;
    word          type;
#define ETH_TYPE_IP   0x800
#define ETH_TYPE_ARP  0x806
#define ETH_TYPE_RARP 0x8053
} eth_header_t;


/*
 * ARP/RARP header.
 */
typedef struct {
    word	hw_type;
#define ARP_HW_ETHER 1
    word	protocol;
    octet	hw_len;
    octet	proto_len;
    word	opcode;
#define	ARP_REQUEST	1
#define	ARP_REPLY	2
#define	RARP_REQUEST	3
#define	RARP_REPLY	4
    enet_addr_t	sender_enet;
    ip_addr_t	sender_ip;
    enet_addr_t	target_enet;
    ip_addr_t	target_ip;
} arp_header_t;


#define ARP_PKT_SIZE  (sizeof(arp_header_t) + sizeof(eth_header_t))

/*
 * Internet Protocol header.
 */
typedef struct {
#ifdef __LITTLE_ENDIAN__
    octet       hdr_len:4,
                version:4;
#else
    octet       version:4,
                hdr_len:4;
#endif
    octet       tos;
    word        length;
    word        ident;
    word        fragment;
    octet       ttl;
    octet       protocol;
#define IP_PROTO_ICMP  1
#define IP_PROTO_TCP   6
#define IP_PROTO_UDP  17
    word        checksum;
    ip_addr_t   source;
    ip_addr_t   destination;
} ip_header_t;


#define IP_PKT_SIZE (60 + sizeof(eth_header_t))


/*
 * A IP<->ethernet address mapping.
 */
typedef struct {
    ip_addr_t    ip_addr;
    enet_addr_t  enet_addr;
} ip_route_t;


/*
 * UDP header.
 */
typedef struct {
    word	src_port;
    word	dest_port;
    word	length;
    word	checksum;
} udp_header_t;


/*
 * TCP header.
 */
typedef struct {
    word	src_port;
    word	dest_port;
    dword	seqnum;
    dword	acknum;
#ifdef __LITTLE_ENDIAN__
    octet       reserved:4,
                hdr_len:4;
#else
    octet       hdr_len:4,
                reserved:4;
#endif
    octet	flags;
#define TCP_FLAG_FIN  1
#define TCP_FLAG_SYN  2
#define TCP_FLAG_RST  4
#define TCP_FLAG_PSH  8
#define TCP_FLAG_ACK 16
#define TCP_FLAG_URG 32
    word	window;
    word	checksum;
    word	urgent;
} tcp_header_t;


/*
 * ICMP header.
 */
typedef struct {
    octet	type;
#define ICMP_TYPE_ECHOREPLY   0
#define ICMP_TYPE_ECHOREQUEST 8
    octet	code;
    word	checksum;
    word	ident;
    word	seqnum;
} icmp_header_t;


/*
 * BOOTP header.
 */
typedef struct {
    octet	opcode;
#define BOOTP_OP_REQUEST 1
#define BOOTP_OP_REPLY   2
    octet	hw_type;	/* same as ARP */
    octet	hw_len;
    octet	hop_cnt;
    dword	trans_id;
    word	seconds;
    word	unused;
    ip_addr_t	client_ip;
    ip_addr_t	your_ip;
    ip_addr_t	server_ip;
    ip_addr_t	gateway_ip;
    octet	client_hw[16];
    char        server_name[64];
    char        boot_file[128];
    char        vendor_data[64];
} bootp_header_t;


/*
 * TFTP header.
 */
typedef struct {
    word	opcode;
#define TFTP_OP_RRQ  1
#define TFTP_OP_WRQ  2
#define TFTP_OP_DATA 3
#define TFTP_OP_ACK  4
#define TFTP_OP_ERR  5
    union {
	char name[2];
	word block_nr;  /* followed by 0-512 bytes data */
	word error_nr;
#define TFTP_ERR_UNDEF        0         /* Not defined, see error message (if any). */
#define TFTP_ERR_FILENOTFOUND 1         /* File not found. */
#define TFTP_ERR_ACCESS       2         /* Access violation. */
#define TFTP_ERR_DISKFULL     3         /* Disk full or allocation exceeded. */
#define TFTP_ERR_ILLEGAL      4         /* Illegal TFTP operation. */
#define TFTP_ERR_BADID        5         /* Unknown transfer ID. */
#define TFTP_ERR_FILEEXISTS   6         /* File already exists. */
#define TFTP_ERR_NOUSER       7         /* No such user. */
    } u;
} tftp_header_t;

extern ip_addr_t __tftp_server_ip;
extern ip_addr_t __tftp_gateway_ip;
extern char      __tftp_filename[128];

extern int __tftp_read(ip_route_t *r, const char *filename,
		       void *buffer, int buflen);

typedef struct _pktbuf {
    struct _pktbuf *next;
    eth_header_t   *eth_hdr;		/* pointer to ethernet header */
    union {
	ip_header_t *__iphdr;		/* pointer to IP header */
	arp_header_t *__arphdr;		/* pointer to ARP header */
    } u1;
#define ip_hdr u1.__iphdr
#define arp_hdr u1.__arphdr
    union {
	udp_header_t *__udphdr;		/* pointer to UDP header */
	tcp_header_t *__tcphdr;		/* pointer to TCP header */
	icmp_header_t *__icmphdr;	/* pointer to ICMP header */
    } u2;
#define udp_hdr u2.__udphdr
#define tcp_hdr u2.__tcphdr
#define icmp_hdr u2.__icmphdr
    word	pkt_bytes;		/* number of data bytes in buf */
    word        bufsize;                /* size of buf */
    word	*buf;
} pktbuf_t;


/* protocol handler */
typedef void (*pkt_handler_t)(pktbuf_t *pkt);


typedef struct _udp_socket {
    struct _udp_socket	*next;
    word		our_port;
    word		pad;
    void                (*handler)(struct _udp_socket *skt, char *buf, int len,
				   ip_route_t *src_route, word src_port);
} udp_socket_t;


typedef void (*udp_handler_t)(udp_socket_t *skt, char *buf, int len,
			      ip_route_t *src_route, word src_port);


typedef struct _tcp_socket {
    struct _tcp_socket *next;
    int		       state;       /* connection state */
#define _CLOSED      0
#define _LISTEN      1
#define _SYN_RCVD    2
#define _SYN_SENT    3
#define _ESTABLISHED 4
#define _CLOSE_WAIT  5
#define _LAST_ACK    6
#define _FIN_WAIT_1  7
#define _FIN_WAIT_2  8
#define _CLOSING     9
#define _TIME_WAIT  10
    ip_route_t         his_addr;    /* address of other end of connection */
    word               our_port;
    word               his_port;
    word               data_bytes;   /* number of data bytes in pkt */
    dword              ack;
    dword              seq;
    timer_t            timer;
    pktbuf_t           pkt;         /* dedicated xmit packet */
    pktbuf_t           *rxlist;     /* list of unread incoming data packets */
    char               *rxptr;      /* pointer to next byte to read */
    int                rxcnt;       /* bytes left in current read packet */
    word               ack_pending; /* true if outgoing ack is deferred while
				       awaiting incoming ack */
    char               pktbuf[ETH_MAX_PKTLEN];
} tcp_socket_t;

/*
 * Our address.
 */
extern enet_addr_t __local_enet_addr;
extern ip_addr_t   __local_ip_addr;


/*
 * Set a timer. Caller is responsible for providing the timer_t struct.
 */
extern void __timer_set(timer_t *t, unsigned long delay,
			tmr_handler_t handler, void *user_data);

/*
 * Cancel the given timer.
 */
extern void __timer_cancel(timer_t *t);

/*
 * Poll timer list for timer expirations.
 */
extern void __timer_poll(void);

/*
 * Initialize the free list.
 */
extern void __pktbuf_init(void);

/*
 * simple pktbuf allocation.
 * allocates at least nbytes for packet data including ethernet header.
 */
extern pktbuf_t *__pktbuf_alloc(int nbytes);

/*
 * return a pktbuf for reuse.
 */
extern void __pktbuf_free(pktbuf_t *pkt);


/*
 * Install handlers for ethernet packets.
 * Returns old handler.
 */
extern pkt_handler_t __eth_install_handler(int eth_type,
					   pkt_handler_t handler);

/*
 * Install procs for link driver.
 */
extern void _bsp_install_enet_driver(struct bsp_comm_procs *procs);

/*
 * Set tcp port number for debugging. If not called, a default
 * will be selected.
 */
extern void _bsp_net_set_debug_port(int portnum);

/*
 * Initialize the network stack.
 * procs is a pointer to the ethernet driver procs.
 * Returns true if successful, false if unable to init.
 */
extern int _bsp_net_init(struct bsp_comm_procs *procs);

/*
 * Non-blocking poll of ethernet link. Processes at most
 * one packet.
 */
extern void __enet_poll(void);

/*
 * Send an ethernet packet.
 */
extern void __enet_send(pktbuf_t *pkt, enet_addr_t dest, int eth_type);


/*
 * Handle incoming ARP packets.
 */
extern void __arp_handler(pktbuf_t *pkt);

/* 
 * Find the ethernet address of the machine with the given
 * ip address.
 * Return true and fills in 'eth_addr' if successful, false
 * if unsuccessful.
 */
extern int __arp_request(ip_addr_t ip_addr, enet_addr_t eth_addr);


/*
 * Do a one's complement checksum.
 * The data being checksum'd is in network byte order.
 * The returned checksum is in network byte order.
 */
extern unsigned short __sum(word *w, int len, int init_sum);

/*
 * Compute a partial checksum for the UDP/TCP pseudo header.
 */
extern int __pseudo_sum(ip_header_t *ip);

/*
 * Handle IP packets coming from the polled ethernet interface.
 */
extern void __ip_handler(pktbuf_t *pkt, enet_addr_t src_enet_addr);

/*
 * Send an IP packet.
 *
 * The IP data field should contain pkt->pkt_bytes of data.
 * pkt->[udp|tcp|icmp]_hdr points to the IP data field. Any
 * IP options are assumed to be already in place in the IP
 * options field.
 */
extern void __ip_send(pktbuf_t *pkt, int protocol, ip_route_t *dest);


/*
 * Handle incoming ICMP packets.
 */
extern void __icmp_handler(pktbuf_t *pkt, ip_route_t *r);

/*
 * Handle incoming UDP packets.
 */
extern void __udp_handler(pktbuf_t *pkt, ip_route_t *r);

/*
 * Install a handler for incoming udp packets.
 * Caller provides the udp_socket_t structure.
 * Returns zero if successful, -1 if socket is already used.
 */
extern int __udp_install_listener(udp_socket_t *s, word port,
				  udp_handler_t handler);

/*
 * Remove the handler for the given socket.
 */
extern void __udp_remove_listener(word port);

/*
 * Send a UDP packet.
 */
extern void __udp_send(char *buf, int len, ip_route_t *dest_ip,
		       word dest_port, word src_port);


/*
 * TCP poll function. Should be called as often as possible.
 */
extern void __tcp_poll(void);

/*
 * Set up a listening socket on the given port.
 * Does not block.
 */
extern int __tcp_listen(tcp_socket_t *s, word port);


/*
 * Block while waiting for all outstanding socket data to
 * be transmitted.
 */
extern void __tcp_drain(tcp_socket_t *s);


/*
 * Initiate connection close.
 */
extern void __tcp_close(tcp_socket_t *s);

/*
 * Wait until connection has fully closed.
 */
extern void __tcp_close_wait(tcp_socket_t *s);

/*
 * Read up to 'len' bytes without blocking.
 * Returns number of bytes read.
 * If connection is closed, returns -1.
 */
extern int __tcp_read(tcp_socket_t *s, char *buf, int len);

/*
 * Write up to 'len' bytes without blocking.
 * Returns number of bytes written.
 * If connection is closed, returns -1.
 */
extern int __tcp_write(tcp_socket_t *s, char *buf, int len);


/*
 * The following are a higher-level tcp socket interface.
 */

/*
 * Initialize a socket for given port.
 */
extern void __skt_init(tcp_socket_t *s, unsigned short port);

/*
 * Return true if socket connection is closed.
 */
#define __skt_is_closed(s) (((tcp_socket_t *)(s))->state == _CLOSED)

/*
 * Block while listening for an incoming connection.
 */
extern void __skt_wait_for_connect(tcp_socket_t *s);

/*
 * Read up to 'len' bytes from the given socket.
 * Returns number of bytes read.
 * Doesn't block.
 */
extern int __skt_read(tcp_socket_t *s, char *buf, int len);

/*
 * Write 'len' bytes to the given socket.
 * Returns number of bytes written.
 * May not write all data if connection closes.
 */
extern int __skt_write(tcp_socket_t *s, char *buf, int len);

