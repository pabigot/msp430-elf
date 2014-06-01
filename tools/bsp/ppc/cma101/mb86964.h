/* 
 *  Header file for very simple ethernet driver, based on Fujitsu MB86964
 *  on a Cogent Computer Systems CMA101 motherboard.
 *
 */

typedef volatile unsigned char  reg8;
typedef volatile unsigned short reg16;
typedef volatile unsigned int   reg32;

/*
 *  MB86964 layout for Cogent CMA101 Motherboard.
 */
struct mb86964_regs {
    reg8    tx_status;	/* Transmit Status */
#define TX_DONE     0x80
#define NET_BSY     0x40
#define TX_PKT_RCD  0x20
#define CR_LOST     0x10
#define JABBER      0x08
#define COLLISION   0x04
#define COLLISIONS  0x02

    char    _pad1[7];
    reg8    rx_status;	/* Receive Status */
#define RX_PKT      0x80
#define BUS_RD_ERR  0x40
#define DMA_EOP     0x20
#define RMT_CNTRL   0x10
#define SHORT_PKT   0x08
#define ALIGN_ERR   0x04
#define CRC_ERR     0x02
#define BUF_OVRFLO  0x01

    char    _pad2[7];
    reg8    tx_ien;	/* Transmit Interrupt Enable */
    char    _pad3[7];
    reg8    rx_ien;	/* Receiver Interrupt Enable */
    char    _pad4[7];
    reg8    tx_mode;	/* Transmit Mode */
    char    _pad5[7];
    reg8    rx_mode;	/* Receive Mode */
#define RX_BUF_EMPTY      0x40
#define ACCEPT_BAD_PKTS   0x20
#define RX_SHORT_ADDR     0x10
#define ACCEPT_SHORT_PKTS 0x08
#define RMT_CNTRL_EN      0x04

    char    _pad6[7];
    reg8    config0;    /* Configuration 0 */
    char    _pad7[7];
    reg8    config1;    /* Configuration 1 */
    char    _pad8[7];

    union {
	struct {
	    reg8    node_0;
	    char    _pad1[7];
	    reg8    node_1;
	    char    _pad2[7];
	    reg8    node_2;
	    char    _pad3[7];
	    reg8    node_3;
	    char    _pad4[7];
	    reg8    node_4;
	    char    _pad5[7];
	    reg8    node_5;
	    char    _pad6[7];
	    reg8    tdr_0;
	    char    _pad7[7];
	    reg8    tdr_1;
	    char    _pad8[7];
	} rbs0;
	struct {
	    reg8    hash_0;
	    char    _pad1[7];
	    reg8    hash_1;
	    char    _pad2[7];
	    reg8    hash_2;
	    char    _pad3[7];
	    reg8    hash_3;
	    char    _pad4[7];
	    reg8    hash_4;
	    char    _pad5[7];
	    reg8    hash_5;
	    char    _pad6[7];
	    reg8    hash_6;
	    char    _pad7[7];
	    reg8    hash_7;
	    char    _pad8[7];
	} rbs1;
	struct {
	    reg8    buffer;
	    char    _pad1[7];
	    reg8    unused;	/* buffer MSB */
	    char    _pad2[7];
	    reg8    tx_start;	/* tx start, packet count */
	    char    _pad3[7];
	    reg8    collision;
	    char    _pad4[7];
	    reg8    dma_enable;
	    char    _pad5[7];
	    reg8    dma_burst;
	    char    _pad6[7];
	    reg8    xcvr_ien;
	    char    _pad7[7];
	    reg8    xcvr_status;
	    char    _pad8[7];
	} rbs2;
    } u;
};


struct mb86964_regs *mb_regs = (struct mb86964_regs *)0x0ea00007;

