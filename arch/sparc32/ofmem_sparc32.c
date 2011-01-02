/*
 *	<ofmem_sparc32.c>
 *
 *	OF Memory manager
 *
 *   Copyright (C) 1999-2004 Samuel Rydh (samuel@ibrium.se)
 *   Copyright (C) 2004 Stefan Reinauer
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License
 *   as published by the Free Software Foundation
 *
 */

#include "config.h"
#include "libopenbios/bindings.h"
#include "libc/string.h"
#include "libopenbios/ofmem.h"
#include "asm/asi.h"
#include "pgtsrmmu.h"

#define OF_MALLOC_BASE		((char*)OFMEM + ALIGN_SIZE(sizeof(ofmem_t), 8))

#define MEMSIZE (512 * 1024)
static union {
	char memory[MEMSIZE];
	ofmem_t ofmem;
} s_ofmem_data;

#define OFMEM      	(&s_ofmem_data.ofmem)
#define TOP_OF_RAM 	(s_ofmem_data.memory + MEMSIZE)

translation_t **g_ofmem_translations = &s_ofmem_data.ofmem.trans;

extern uint32_t qemu_mem_size;

static inline size_t ALIGN_SIZE(size_t x, size_t a)
{
    return (x + a - 1) & ~(a-1);
}

static ucell get_heap_top( void )
{
	return (ucell)TOP_OF_RAM;
}

ofmem_t* ofmem_arch_get_private(void)
{
	return OFMEM;
}

void* ofmem_arch_get_malloc_base(void)
{
	return OF_MALLOC_BASE;
}

ucell ofmem_arch_get_heap_top(void)
{
	return get_heap_top();
}

ucell ofmem_arch_get_virt_top(void)
{
	return (ucell)TOP_OF_RAM;
}

phys_addr_t ofmem_arch_get_phys_top(void)
{
	ofmem_t *ofmem = ofmem_arch_get_private();

	return (uintptr_t)ofmem->ramsize - 0x1000000;
}

retain_t *ofmem_arch_get_retained(void)
{
	/* Not used */
	return 0;
}

int ofmem_arch_get_physaddr_cellsize(void)
{
	return 2;
}

int ofmem_arch_encode_physaddr(ucell *p, phys_addr_t value)
{
	int n = 0;

	p[n++] = value >> 32;
	p[n++] = value;

	return n;
}

int ofmem_arch_get_translation_entry_size(void)
{
	/* Return size of a single MMU package translation property entry in cells */
	return 3;
}

void ofmem_arch_create_translation_entry(ucell *transentry, translation_t *t)
{
	/* Generate translation property entry for SPARC. While there is no
	formal documentation for this, both Linux kernel and OpenSolaris sources
	expect a translation property entry to have the following layout:

		virtual address
		length
		mode
	*/

	transentry[0] = t->virt;
	transentry[1] = t->size;
	transentry[2] = t->mode;
}

/************************************************************************/
/* misc                                                                 */
/************************************************************************/

ucell ofmem_arch_default_translation_mode( phys_addr_t phys )
{
	return SRMMU_REF | SRMMU_CACHE | SRMMU_PRIV;
}

/************************************************************************/
/* init / cleanup                                                       */
/************************************************************************/

void ofmem_init( void )
{
	memset(&s_ofmem_data, 0, sizeof(s_ofmem_data));
	s_ofmem_data.ofmem.ramsize = qemu_mem_size;
}
