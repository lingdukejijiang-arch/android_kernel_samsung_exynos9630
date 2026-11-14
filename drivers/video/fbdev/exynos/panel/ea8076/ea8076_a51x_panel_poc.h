/*
 * drivers/video/fbdev/exynos/panel/ea8076/ea8076_a51x_panel_poc.h
 *
 * POC (Panel burn-in Compensation flash) driver data for EA8076_A51X panel.
 *
 * Translated 1:1 from the A51 4G (dpu20) standalone driver
 * ea8076_a51_poc.c/h, which drives the same EA8076 DDI.
 * Command bytes are identical; only the container format differs
 * (exynos9630 unified panel framework: seqtbl/maptbl).
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __EA8076_A51X_PANEL_POC_H__
#define __EA8076_A51X_PANEL_POC_H__

#include "../panel.h"
#include "../panel_poc.h"

#define A51X_POC_IMG_ADDR	(0)
#define A51X_POC_IMG_SIZE	(327192)
#define EA8076_POC_WRITE_MAX_SIZE	(256)

/* ===================================================================================== */
/* ============================== [EA8076 MAPPING TABLE] =============================== */
/* ===================================================================================== */
/*
 * EA8076 write-address register (0xFF, page 0x11) is 16-byte granular:
 *   byte0 = (waddr >> 4) & 0xFF, byte1 = (waddr >> 12) & 0xFF
 * The generic copy_poc_wr_addr_maptbl() emits 24-bit big-endian byte
 * addresses, which does NOT match this DDI, hence the panel-specific
 * copy functions below. OR1 variant sets bit0 of byte0 (write-pulse).
 */
enum {
	EA8076_POC_WR_ADDR_MAPTBL = 0,
	EA8076_POC_WR_ADDR_OR1_MAPTBL,
	EA8076_POC_WR_DATA_MAPTBL,
	MAX_EA8076_POC_MAPTBL,
};

static void ea8076_copy_poc_wr_addr_maptbl(struct maptbl *tbl, u8 *dst)
{
	struct panel_poc_device *poc_dev;
	struct panel_poc_info *poc_info;
	u32 waddr;

	if (!tbl || !dst)
		return;

	poc_dev = (struct panel_poc_device *)tbl->pdata;
	if (unlikely(!poc_dev))
		return;

	poc_info = &poc_dev->poc_info;
	waddr = poc_info->waddr;

	dst[0] = (waddr >> 4) & 0xFF;
	dst[1] = (waddr >> 12) & 0xFF;
}

static void ea8076_copy_poc_wr_addr_or1_maptbl(struct maptbl *tbl, u8 *dst)
{
	ea8076_copy_poc_wr_addr_maptbl(tbl, dst);
	dst[0] |= 0x01;
}

static struct maptbl ea8076_a51x_poc_maptbl[] = {
	[EA8076_POC_WR_ADDR_MAPTBL] = DEFINE_0D_MAPTBL(ea8076_a51x_poc_wr_addr_table, init_common_table, NULL, ea8076_copy_poc_wr_addr_maptbl),
	[EA8076_POC_WR_ADDR_OR1_MAPTBL] = DEFINE_0D_MAPTBL(ea8076_a51x_poc_wr_addr_or1_table, init_common_table, NULL, ea8076_copy_poc_wr_addr_or1_maptbl),
	[EA8076_POC_WR_DATA_MAPTBL] = DEFINE_0D_MAPTBL(ea8076_a51x_poc_wr_data_table, init_common_table, NULL, copy_poc_wr_data_maptbl),
};

/* ===================================================================================== */
/* ============================== [EA8076 COMMAND TABLE] =============================== */
/* ===================================================================================== */
static u8 EA8076_POC_KEY1_ENABLE[] = { 0xF0, 0x5A, 0x5A };
static u8 EA8076_POC_KEY1_DISABLE[] = { 0xF0, 0xA5, 0xA5 };
static u8 EA8076_POC_KEY2_ENABLE[] = { 0xF1, 0xF1, 0xA2 };
static u8 EA8076_POC_KEY2_DISABLE[] = { 0xF1, 0xA5, 0xA5 };
static u8 EA8076_POC_KEY3_ENABLE[] = { 0xFC, 0x5A, 0x5A };
static u8 EA8076_POC_KEY3_DISABLE[] = { 0xFC, 0xA5, 0xA5 };
/* page switch to 0x11 before touching the 0xFF address register */
static u8 EA8076_POC_WRITE_GP[] = { 0xB0, 0x11 };
/* 0xCF data register: 1 reg byte + 256 data bytes */
static u8 EA8076_POC_WR_DAT[EA8076_POC_WRITE_MAX_SIZE + 1] = { 0xCF };
/* 0xFF address register: reg byte + 2 addr bytes (maptbl) */
static u8 EA8076_POC_WR_ADDR[3] = { 0xFF, 0x00, 0x00 };
static u8 EA8076_POC_WR_ADDR_OR1[3] = { 0xFF, 0x00, 0x00 };

static DEFINE_STATIC_PACKET(ea8076_poc_key1_enable_pkt, DSI_PKT_TYPE_WR, EA8076_POC_KEY1_ENABLE, 0);
static DEFINE_STATIC_PACKET(ea8076_poc_key1_disable_pkt, DSI_PKT_TYPE_WR, EA8076_POC_KEY1_DISABLE, 0);
static DEFINE_STATIC_PACKET(ea8076_poc_key2_enable_pkt, DSI_PKT_TYPE_WR, EA8076_POC_KEY2_ENABLE, 0);
static DEFINE_STATIC_PACKET(ea8076_poc_key2_disable_pkt, DSI_PKT_TYPE_WR, EA8076_POC_KEY2_DISABLE, 0);
static DEFINE_STATIC_PACKET(ea8076_poc_key3_enable_pkt, DSI_PKT_TYPE_WR, EA8076_POC_KEY3_ENABLE, 0);
static DEFINE_STATIC_PACKET(ea8076_poc_key3_disable_pkt, DSI_PKT_TYPE_WR, EA8076_POC_KEY3_DISABLE, 0);
static DEFINE_STATIC_PACKET(ea8076_poc_write_gp_pkt, DSI_PKT_TYPE_WR, EA8076_POC_WRITE_GP, 0);

static DEFINE_PKTUI(ea8076_poc_wr_dat, &ea8076_a51x_poc_maptbl[EA8076_POC_WR_DATA_MAPTBL], 1);
static DEFINE_VARIABLE_PACKET(ea8076_poc_wr_dat_pkt, DSI_PKT_TYPE_WR, EA8076_POC_WR_DAT, 0);
static DEFINE_PKTUI(ea8076_poc_wr_addr, &ea8076_a51x_poc_maptbl[EA8076_POC_WR_ADDR_MAPTBL], 1);
static DEFINE_VARIABLE_PACKET(ea8076_poc_wr_addr_pkt, DSI_PKT_TYPE_WR, EA8076_POC_WR_ADDR, 0);
static DEFINE_PKTUI(ea8076_poc_wr_addr_or1, &ea8076_a51x_poc_maptbl[EA8076_POC_WR_ADDR_OR1_MAPTBL], 1);
static DEFINE_VARIABLE_PACKET(ea8076_poc_wr_addr_or1_pkt, DSI_PKT_TYPE_WR, EA8076_POC_WR_ADDR_OR1, 0);

static DEFINE_PANEL_MDELAY(ea8076_poc_wait_10ms, 10);
static DEFINE_PANEL_MDELAY(ea8076_poc_wait_33ms, 33);

/* DDI flash unlock/lock + full-chip erase command tables (see
 * ea8076_a51_poc_tables.h, translated from the 4G dpu20 driver). */
#include "ea8076_a51_poc_tables.h"

/* standalone seqinfos so the big generated cmdtbls can be nested
 * (panel_do_seqtbl() handles CMD_TYPE_SEQ nodes recursively) */
static struct seqinfo ea8076_a51x_poc_unlock_seq =
	SEQINFO_INIT("poc-unlock-seq", ea8076_a51x_poc_unlock_cmdtbl);
static struct seqinfo ea8076_a51x_poc_lock_seq =
	SEQINFO_INIT("poc-lock-seq", ea8076_a51x_poc_lock_cmdtbl);
static struct seqinfo ea8076_a51x_poc_erase_all_seq =
	SEQINFO_INIT("poc-erase-all-seq", ea8076_a51x_poc_erase_all_cmdtbl);

/* ---- enter: keys on + flash unlock, 1:1 from dpu20 enter ---- */
static void *ea8076_a51x_poc_wr_enter_cmdtbl[] = {
	&DLYINFO(ea8076_poc_wait_33ms),
	&PKTINFO(ea8076_poc_key1_enable_pkt),
	&PKTINFO(ea8076_poc_key2_enable_pkt),
	&PKTINFO(ea8076_poc_key3_enable_pkt),
	&SEQINFO(ea8076_a51x_poc_unlock_seq),
	&DLYINFO(ea8076_poc_wait_33ms),
};

/*
 * Per 256-byte chunk, 1:1 from dpu20 POC_WRITE_DAT_SEQ:
 *   CF + 256B data, B0:11 -> FF:addr|1, 10ms, B0:11 -> FF:addr
 */
static void *ea8076_a51x_poc_wr_dat_cmdtbl[] = {
	&PKTINFO(ea8076_poc_wr_dat_pkt),
	&PKTINFO(ea8076_poc_write_gp_pkt),
	&PKTINFO(ea8076_poc_wr_addr_or1_pkt),
	&DLYINFO(ea8076_poc_wait_10ms),
	&PKTINFO(ea8076_poc_write_gp_pkt),
	&PKTINFO(ea8076_poc_wr_addr_pkt),
};

/* ---- exit: flash lock + keys off, 1:1 from dpu20 exit ---- */
static void *ea8076_a51x_poc_wr_exit_cmdtbl[] = {
	&SEQINFO(ea8076_a51x_poc_lock_seq),
	&DLYINFO(ea8076_poc_wait_33ms),
	&PKTINFO(ea8076_poc_key1_disable_pkt),
	&PKTINFO(ea8076_poc_key2_disable_pkt),
	&PKTINFO(ea8076_poc_key3_disable_pkt),
};

/*
 * The EA8076 erase tables ignore the address and erase the whole DDI
 * flash; each ERASE_TABLE_xx is one bank. Run all five banks per
 * invocation (idempotent) so any block-size loop of the framework
 * results in a complete erase.
 */
static void *ea8076_a51x_poc_er_64k_cmdtbl[] = {
	&SEQINFO(ea8076_a51x_poc_erase_all_seq),
};

static struct seqinfo ea8076_a51x_poc_seqtbl[MAX_POC_SEQ] = {
	/* poc erase */
	[POC_ERASE_ENTER_SEQ] = SEQINFO_INIT("poc-erase-enter-seq", ea8076_a51x_poc_wr_enter_cmdtbl),
	[POC_ERASE_64K_SEQ] = SEQINFO_INIT("poc-erase-64k-seq", ea8076_a51x_poc_er_64k_cmdtbl),
	[POC_ERASE_32K_SEQ] = SEQINFO_INIT("poc-erase-32k-seq", ea8076_a51x_poc_er_64k_cmdtbl),
	[POC_ERASE_4K_SEQ] = SEQINFO_INIT("poc-erase-4k-seq", ea8076_a51x_poc_er_64k_cmdtbl),
	[POC_ERASE_EXIT_SEQ] = SEQINFO_INIT("poc-erase-exit-seq", ea8076_a51x_poc_wr_exit_cmdtbl),
	/* poc write */
	[POC_WRITE_ENTER_SEQ] = SEQINFO_INIT("poc-wr-enter-seq", ea8076_a51x_poc_wr_enter_cmdtbl),
	[POC_WRITE_DAT_SEQ] = SEQINFO_INIT("poc-wr-dat-seq", ea8076_a51x_poc_wr_dat_cmdtbl),
	[POC_WRITE_EXIT_SEQ] = SEQINFO_INIT("poc-wr-exit-seq", ea8076_a51x_poc_wr_exit_cmdtbl),
	/* EA8076 DDI: flash read not supported by the 4G driver either */
};

static struct poc_partition ea8076_a51x_poc_partition[] = {
	[POC_IMG_PARTITION] = {
		.name = "image",
		.addr = A51X_POC_IMG_ADDR,
		.size = A51X_POC_IMG_SIZE,
		.need_preload = false
	},
};

static struct panel_poc_data ea8076_a51x_poc_data = {
	.version = 2,
	.seqtbl = ea8076_a51x_poc_seqtbl,
	.nr_seqtbl = ARRAY_SIZE(ea8076_a51x_poc_seqtbl),
	.maptbl = ea8076_a51x_poc_maptbl,
	.nr_maptbl = ARRAY_SIZE(ea8076_a51x_poc_maptbl),
	.partition = ea8076_a51x_poc_partition,
	.nr_partition = ARRAY_SIZE(ea8076_a51x_poc_partition),
	.wdata_len = EA8076_POC_WRITE_MAX_SIZE,
	.rdata_len = 0, /* read unsupported on this DDI */
};
#endif /* __EA8076_A51X_PANEL_POC_H__ */
