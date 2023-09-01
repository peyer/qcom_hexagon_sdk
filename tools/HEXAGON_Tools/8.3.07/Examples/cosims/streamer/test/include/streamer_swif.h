#ifndef STREAMER_SWIF_H
#define STREAMER_SWIF_H 1

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct streamer_if_struct {
	uint32_t status_autoclear;			// 0x00
	uint32_t status;				// 0x04
	uint32_t status_mask;			// 0x08
	uint32_t event_mask;			// 0x0c
	uint32_t interrupt_mask;			// 0x10
	uint32_t unused_14;			// 0x14
	uint32_t unused_18;			// 0x18
	uint32_t unused_1c;			// 0x1c
	uint32_t rx_pixels;			// 0x20
	uint32_t rx_linewidth;			// 0x24
	uint32_t rx_lines;				// 0x28
	uint32_t unused_2c;			// 0x2c
	uint32_t rx_expected_pixels;		// 0x30
	uint32_t unused_34;			// 0x34
	uint32_t rx_expected_lines;		// 0x38
	uint32_t unused_3c;			// 0x3c
	uint32_t rx_index_avail;			// 0x40
	uint32_t rx_index_consumed;		// 0x44
	uint32_t tx_index_avail;			// 0x48
	uint32_t tx_index_consumed;		// 0x4c
	uint32_t tx_signals;			// 0x50
	uint32_t tx_settings;			// 0x54
	uint32_t unused_58;			// 0x58
	uint32_t unused_5c;			// 0x5c
	uint32_t tx_pixels;			// 0x60
	uint32_t tx_linewidth;			// 0x64
	uint32_t tx_lines;				// 0x68
	uint32_t unused_6c;			// 0x6c
	uint32_t unused_70;			// 0x70
	uint32_t unused_74;			// 0x74
	uint32_t tx_current_line;			// 0x78
	uint32_t unused_7c;			// 0x7c
	uint32_t control;				// 0x80
	uint32_t unused_84;			// 0x84
	uint32_t unused_88;			// 0x88
	uint32_t unused_8c;			// 0x8c
	uint32_t unused_90;			// 0x90
	uint32_t unused_94;			// 0x94
	uint32_t unused_98;			// 0x98
	uint32_t unused_9c;			// 0x9c
	uint32_t tx_min_start;			// 0xA0
	uint32_t tx_not_full;			// 0xA4
	uint32_t rx_timeout;			// 0xA8
	uint32_t status_mode;			// 0xAC
	uint32_t format;				// 0xB0
	uint32_t reserved_b4;			// 0xB4
	uint32_t reserved_b8;			// 0xB8
	uint32_t reserved_bc;			// 0xBC
	uint32_t reserved_c0_ff[16];		// 0xc0-ff
	uint32_t reserved_1xx_fff[1024-64];	// 0x1ff - 0xfff
	uint32_t rx_head;
	uint32_t rx_index;
	uint32_t tx_head;
	uint32_t tx_index;
} streamer_if_t;

typedef struct streamer_config_struct {
	uint32_t tx_start;
	uint32_t tx_len;
	uint32_t rx_start;
	uint32_t rx_len;
	uint32_t tx_end;
	uint32_t rx_end;
	uint32_t tx_cols;
	uint32_t rx_cols;
	uint32_t tx_rows;
	uint32_t rx_rows;
	uint32_t tx_minstart;
	uint32_t rx_timeout;
	uint32_t format;
} streamer_config_t;

//#define STREAMER_FORMAT_RX_8BIT       0x00000000
//#define STREAMER_FORMAT_RX_16BIT      0x00000004
//#define STREAMER_FORMAT_RX_MSB        0x00000000
//#define STREAMER_FORMAT_RX_LSB        0x00000008
//#define STREAMER_FORMAT_TX_8BIT       0x00000000
//#define STREAMER_FORMAT_TX_16BIT      0x00000400
//#define STREAMER_FORMAT_TX_MSB        0x00000000
//#define STREAMER_FORMAT_TX_LSB        0x00000800
//#define STREAMER_FORMAT_RX_PAD_0      0x00000000
//#define STREAMER_FORMAT_RX_PAD_1      0x00001000
//#define STREAMER_FORMAT_RX_PAD_2      0x00002000
//#define STREAMER_FORMAT_RX_PAD_3      0x00003000
//#define STREAMER_FORMAT_RX_PAD_4      0x00004000
//#define STREAMER_FORMAT_RX_PAD_5      0x00005000
//#define STREAMER_FORMAT_RX_PAD_6      0x00006000
//#define STREAMER_FORMAT_RX_PAD_7      0x00007000
//#define STREAMER_FORMAT_TX_STRIP_0    0x00000000
//#define STREAMER_FORMAT_TX_STRIP_1    0x00010000
//#define STREAMER_FORMAT_TX_STRIP_2    0x00020000
//#define STREAMER_FORMAT_TX_STRIP_3    0x00030000
//#define STREAMER_FORMAT_TX_STRIP_4    0x00040000
//#define STREAMER_FORMAT_TX_STRIP_5    0x00050000
//#define STREAMER_FORMAT_TX_STRIP_6    0x00060000
//#define STREAMER_FORMAT_TX_STRIP_7    0x00070000
//#define STREAMER_FORMAT_RXPAD_MIRX1   0x00000000
//#define STREAMER_FORMAT_RXPAD_DUPX1   0x00100000
//#define STREAMER_FORMAT_RXPAD_MIRX2   0x00200000
//#define STREAMER_FORMAT_RXPAD_DUPX2   0x00300000
//#define STREAMER_FORMAT_RXPAD_MIRX4   0x00400000
//#define STREAMER_FORMAT_RXPAD_DUPX4   0x00500000
//#define STREAMER_FORMAT_RXPAD_ZERO    0x00600000

static inline void streamer_config_init(streamer_config_t *config)
{
	memset(config,0,sizeof(*config));
}

static inline void streamer_config_tx(streamer_config_t *config, uint32_t start, uint32_t len, uint32_t cols, uint32_t rows)
{
	config->tx_start = start;
	config->tx_len = len;
	config->tx_end = start+len;
	config->tx_cols = cols;
	config->tx_rows = rows;
}

static inline void streamer_config_rx(streamer_config_t *config, uint32_t start, uint32_t len, uint32_t cols, uint32_t rows)
{
	config->rx_start = start;
	config->rx_len = len;
	config->rx_end = start+len;
	config->rx_cols = cols;
	config->rx_rows = rows;
}

static inline void streamer_init(volatile streamer_if_t *streamer, const streamer_config_t *config)
{
	streamer->tx_head = config->tx_start >> 6;
	streamer->tx_index = config->tx_len;
	streamer->rx_head = config->rx_start >> 6;
	streamer->rx_index = config->rx_len;
	streamer->tx_index_avail = streamer->tx_index_consumed = 0;
	streamer->rx_index_avail = streamer->rx_index_consumed = 0;
	streamer->format = config->format;
	streamer->rx_expected_pixels = config->rx_cols;
	streamer->rx_expected_lines = config->rx_rows;
	streamer->tx_pixels = config->tx_cols;
	streamer->tx_lines = config->tx_rows;
    streamer->tx_min_start = config->tx_cols*2;
}

#define STREAMER_CONTROL_TX_START 0x0000001
#define STREAMER_CONTROL_TX_RESET 0x0000002
#define STREAMER_CONTROL_RX_START 0x0010000
#define STREAMER_CONTROL_RX_RESET 0x0020000

static inline void streamer_tx_start(volatile streamer_if_t *streamer)
{   
	streamer->control |= STREAMER_CONTROL_TX_START;
}

static inline void streamer_tx_stop(volatile streamer_if_t *streamer)
{
	streamer->control &= ~STREAMER_CONTROL_TX_START;
}
static inline void streamer_tx_reset(volatile streamer_if_t *streamer)
{
	streamer->control |= STREAMER_CONTROL_TX_RESET;
	while (streamer->control & STREAMER_CONTROL_TX_RESET) {
		/* wait for reset to complete */
	}
}

static inline void streamer_rx_start(volatile streamer_if_t *streamer)
{
	streamer->control |= STREAMER_CONTROL_RX_START;
}

static inline void streamer_rx_stop(volatile streamer_if_t *streamer)
{
	streamer->control &= ~STREAMER_CONTROL_RX_START;
}

static inline void streamer_rx_reset(volatile streamer_if_t *streamer)
{
	streamer->control |= STREAMER_CONTROL_RX_RESET;
	while (streamer->control & STREAMER_CONTROL_RX_RESET) {
		/* wait for reset to complete */
	}
}

/* Status Register bits */
#define STREAMER_STATUS_RX_ESSOF       0x00000001
#define STREAMER_STATUS_RX_ESOF        0x00000002
#define STREAMER_STATUS_RX_SOF         0x00000004
#define STREAMER_STATUS_RX_SOL         0x00000008
#define STREAMER_STATUS_RX_EOL         0x00000010
#define STREAMER_STATUS_RX_EOF         0x00000020
#define STREAMER_STATUS_RX_UPDATE_REQ  0x00000040
#define STREAMER_STATUS_RX_SYNCH_EV    0x00000080
#define STREAMER_STATUS_RX_OVERRUN     0x00000100
#define STREAMER_STATUS_RX_BAD_PIXEL_CNT  0x00000200
#define STREAMER_STATUS_RESERVED10     0x00000400
#define STREAMER_STATUS_RX_BAD_LINE_CNT  0x00000800
#define STREAMER_STATUS_RESERVED12    0x00001000
#define STREAMER_STATUS_RESERVED13    0x00002000
#define STREAMER_STATUS_RX_BUS_ERROR  0x00004000
#define STREAMER_STATUS_RESERVED15   0x00008000
#define STREAMER_STATUS_TX_OUTPUT_NOTFULL  0x00010000
#define STREAMER_STATUS_RESERVED17   0x00020000
#define STREAMER_STATUS_TX_SOF       0x00040000
#define STREAMER_STATUS_TX_SOL       0x00080000
#define STREAMER_STATUS_TX_EOL       0x00100000
#define STREAMER_STATUS_TX_EOF       0x00200000
#define STREAMER_STATUS_RESERVED22   0x00400000
#define STREAMER_STATUS_RESERVED23   0x00800000
#define STREAMER_STATUS_TX_OVERRUN   0x01000000
#define STREAMER_STATUS_TX_UNDERRUN  0x02000000
#define STREAMER_STATUS_SW_SETABLE0  0x04000000
#define STREAMER_STATUS_SW_SETABLE1  0x08000000
#define STREAMER_STATUS_SW_SETABLE2  0x10000000
#define STREAMER_STATUS_SW_SETABLE3  0x20000000
#define STREAMER_STATUS_TX_BUS_ERROR  0x4000000
#define STREAMER_STATUS_RESERVED31   0x80000000

static inline uint32_t streamer_status(volatile streamer_if_t *streamer)
{
	return streamer->status;
}

/* Bit fields for TX SIGNALS registers */
#define TX_FLAG_ESOF      0x00000001
#define TX_FLAG_ESSOF     0x00000002
#define TX_FLAG_EOF       0x01000000
#define TX_FLAG_REGUPDATE 0x00000004
#define TX_FLAG_ERROR     0x00010000

static inline void streamer_tx_setflags(volatile streamer_if_t *streamer, uint32_t flags)
{
	streamer->tx_signals |= flags;
}

static inline void streamer_rx_wait_for_eof(volatile streamer_if_t *streamer)
{
	while (!(streamer_status(streamer) & STREAMER_STATUS_RX_EOF)) ;
}

static inline void streamer_tx_wait_for_eof(volatile streamer_if_t *streamer)
{
	while (!(streamer_status(streamer) & STREAMER_STATUS_TX_EOF)) ;
}

static inline uint32_t streamer_rx_wait_for_line(volatile streamer_if_t *streamer, uint32_t linecount)
{
	uint32_t newlines = 0;
	while ((newlines = streamer->rx_lines) <= linecount) /* wait */;
	return newlines;
}

static inline uint32_t streamer_tx_wait_for_line(volatile streamer_if_t *streamer, uint32_t linecount)
{
	uint32_t newlines = 0;
	while ((newlines = streamer->tx_current_line) <= linecount) /* wait */;
	return newlines;
}

static inline void *streamer_tx_wrap(void *ptr, const streamer_config_t *config)
{
	long iptr = (long)ptr;
	if (iptr >= config->tx_end) iptr -= config->tx_len;
	return (void *)iptr;
}

static inline void *streamer_rx_wrap(void *ptr, const streamer_config_t *config)
{
	long iptr = (long)ptr;
	if (iptr >= config->rx_end) iptr -= config->rx_len;
	return (void *)iptr;
}

static inline uint32_t streamer_tx_wrap_idx(uint32_t idx, const streamer_config_t *config)
{
	if (idx >= config->tx_len) idx -= config->tx_len;
	return idx;
}

static inline uint32_t streamer_rx_wrap_idx(uint32_t idx, const streamer_config_t *config)
{
	if (idx >= config->rx_len) idx -= config->rx_len;
	return idx;
}


static inline void streamer_rx_done(volatile streamer_if_t *streamer, uint32_t offset)
{
	streamer->rx_index_consumed = offset;
}
static inline void streamer_tx_done(volatile streamer_if_t *streamer, uint32_t offset)
{
	streamer->tx_index_avail = offset;
}

#endif
