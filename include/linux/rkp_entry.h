/*
 *  Copyright (c) 2015 Samsung Electronics Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#ifndef _RKP_ENTRY_H
#define _RKP_ENTRY_H
#include <asm/memory.h>
#include <asm/page.h>


#ifndef __ASSEMBLY__
typedef unsigned long long u64;
typedef unsigned int       u32;
typedef unsigned short     u16;
typedef unsigned char      u8;
typedef signed long long   s64;
typedef signed int         s32;
typedef signed short       s16;
typedef signed char        s8;


#define RKP_PREFIX  UL(0xc300c000)
#define RKP_CMDID(CMD_ID)  ((UL(CMD_ID) & UL(0xff)) | RKP_PREFIX)

#define RKP_EMULT_TTBR0	RKP_CMDID(0x10)
#define RKP_EMULT_SCTLR	RKP_CMDID(0x15)
#define RKP_PGD_SET	RKP_CMDID(0x21)
#define RKP_PMD_SET	RKP_CMDID(0x22)
#define RKP_PTE_SET	RKP_CMDID(0x23)
#define RKP_PGD_FREE	RKP_CMDID(0x24)
#define RKP_PGD_NEW	RKP_CMDID(0x25)
#define RKP_INIT	RKP_CMDID(0x0)
#define RKP_DEF_INIT	RKP_CMDID(0x1)


#define CFP_ROPP_INIT		RKP_CMDID(0x90)
#define CFP_ROPP_SAVE		RKP_CMDID(0x91)
#define CFP_ROPP_RELOAD		RKP_CMDID(0x92)
#define CFP_JOPP_INIT		RKP_CMDID(0x98)

#define RKP_INIT_MAGIC 0x5afe0001
#define RKP_RO_BUFFER  UL(0x800000)

#define CRED_JAR_RO "cred_jar_ro"
#define TSEC_JAR	"tsec_jar"
#define VFSMNT_JAR	"vfsmnt_cache"

#define   TIMA_DEBUG_LOG_START  0xA0600000
#define   TIMA_DEBUG_LOG_SIZE   1<<18

#define   TIMA_SEC_LOG          0x9FA00000
#define   TIMA_SEC_LOG_SIZE     0x6000 

extern u8 rkp_pgt_bitmap[];
extern u8 rkp_map_bitmap[];

typedef struct rkp_init rkp_init_t;
extern u8 rkp_started;
extern u8 rkp_def_init_done;
extern void *rkp_ro_alloc(void);
extern void rkp_ro_free(void *free_addr);
extern unsigned int is_rkp_ro_page(u64 addr);

extern unsigned long max_pfn;

struct rkp_init {
        u32 magic;
        u64 vmalloc_start;
        u64 vmalloc_end;
        u64 init_mm_pgd;
        u64 id_map_pgd;
        u64 zero_pg_addr;
        u64 rkp_pgt_bitmap;
        u64 rkp_map_bitmap;
        u32 rkp_pgt_bitmap_size;
        u64 _text;
        u64 _etext;
        u64 physmap_addr;
        u64 _srodata;
        u64 _erodata;
	u32 large_memory;
} __attribute__((packed));

/*** TODO: We need to export this so it is hard coded 
     at one place*/

#define	RKP_PHYS_OFFSET_MAX		(({ max_pfn; }) << PAGE_SHIFT)
#define RKP_PHYS_ADDR_MASK		((1ULL << 40)-1)

#define	RKP_PGT_BITMAP_LEN	0x30000
#define	TIMA_ROBUF_START	0x9FA08000
#define	TIMA_ROBUF_SIZE		0x5f8000 /* 6MB - RKP_SEC_LOG_SIZE - RKP_DASHBOARD_SIZE - KASLR_OFFSET)*/

#define RKP_RBUF_VA      (phys_to_virt(TIMA_ROBUF_START))
#define RO_PAGES  (TIMA_ROBUF_SIZE >> PAGE_SHIFT) // (TIMA_ROBUF_SIZE/PAGE_SIZE)

int rkp_call(u64 command, u64 arg0, u64 arg1, u64 arg2, u64 arg3, u64 arg4);


static inline void rkp_bm_flush_cache(u64 addr)
{
	asm volatile(
			"mov x1, %0\n"
			"dc civac, x1\n"
			:
			: "r" (addr)
			: "x1", "memory" );
}

#define rkp_is_pte_protected(va)	rkp_is_protected(__pa(va), (u64 *)rkp_pgt_bitmap, 0)
#define rkp_is_pg_protected(va)	rkp_is_protected(__pa(va), (u64 *)rkp_pgt_bitmap, 1)
#define rkp_is_pg_dbl_mapped(pa) rkp_is_protected(pa, (u64 *)rkp_map_bitmap, 0)
static inline u8 rkp_is_protected(u64 pa, u64 *base_addr, int type)
{
	u64 phys_addr = (pa & (RKP_PHYS_ADDR_MASK));
	u64 index = ((phys_addr-PHYS_OFFSET)>>PAGE_SHIFT);
	u64 *p = base_addr;
	u64 tmp = (index>>6);
	u64 rindex;
	u8 val;

	/* We are going to ignore if input address does NOT belong to DRAM area */
	if(!rkp_started || (phys_addr < PHYS_OFFSET) || (phys_addr  > RKP_PHYS_OFFSET_MAX)) {
		return 0;	
	}
	/* We don't have to check for RO buffer area, This is optimization */
	if(type && ((phys_addr >= TIMA_ROBUF_START) && (phys_addr  < (TIMA_ROBUF_START + TIMA_ROBUF_SIZE)))) {
		return 1;	
	}
	p += (tmp);
	rindex = index % 64;
	if (type)
		rkp_bm_flush_cache((u64)p);
	val = (((*p) & (1ULL<<rindex))?1:0);
	return val;
}
#endif //__ASSEMBLY__

#endif //_RKP_ENTRY_H

