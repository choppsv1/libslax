/*
 * Copyright (c) 2016-2017, Juniper Networks, Inc.
 * All rights reserved.
 * This SOFTWARE is licensed under the LICENSE provided in the
 * ../Copyright file. By downloading, installing, copying, or otherwise
 * using the SOFTWARE, you agree to be bound by the terms of that
 * LICENSE.
 *
 * Phil Shafer <phil@>, April 2016
 */

#ifndef PARROTDB_PACOMMON_H
#define PARROTDB_PACOMMON_H

/*
 * "atom" is a generic term.  Each memory allocator can (and does)
 * redefine the size and content of an "atom", but it's the smallest
 * size that can be allocated from that allocator.  It must be a
 * power-of-two in size.  Some allocators will have a fixed size,
 * while others use parameters.  All should be power-of-two based.
 *
 * "page" is a unit of higher allocation; it doesn't have to be the
 * machine's page, but should be a reasonable size.
 *
 * The page table is a table of atom numbers, so you mask off the
 * "page" bits, index into the table table for a page of atoms and
 * then mask off the "atom" bits to find the atom within that page.
 * The atom number can then be converted into a real address.
 *
 * Note that addresses cannot be tracked backward to page entries.
 *
 * Specific allocators can bend and break these rules as needed, with
 * custom definitions of the "page entry" type.
 */

#include <stdint.h>
#include <assert.h>

#include <libpsu/psucommon.h>

/*
 * NOTE WELL: There's no way to keep PA_ATOM_SHIFT up-to-date
 * automatically, but it must be log2(sizeof(pa_atom_t).
 */
#define PA_ATOM_SHIFT	2	/* log2(sizeof(pa_atom_t) */
typedef uint32_t pa_atom_t;	/* Type for atom numbers */

#define PA_NULL_ATOM	((pa_atom_t) 0)

typedef uint8_t pa_boolean_t;	/* Simple boolean */

/* Type of our trees */
#define PA_TYPE_UNKNOWN		0 /* No type (bad news) */
#define PA_TYPE_MMAP		1 /* Memory mapped segment (pa_mmap_t) */
#define PA_TYPE_FIXED		2 /* Fixed-size malloc pool (pa_fixed_t) */
#define PA_TYPE_ARB		3 /* Arbitrary-sized malloc pool (pa_arb_t) */
#define PA_TYPE_ISTR		4 /* Immutable string table (pa_istr_t) */
#define PA_TYPE_PAT		5 /* Patricia tree (pa_pat_t) */
#define PA_TYPE_OPAQUE		6 /* Opaque header (can't decode) */
#define PA_TYPE_TREE		7 /* Tree (xi_tree_t) */
#define PA_TYPE_BITMAP		8 /* Bitmap (pa_bitmap_t) */

#define PA_TYPE_MAX		9

/*
 * To distinquish between pa_mmap atoms and higher level atoms, we
 * call the former "matoms".  The types are equivalent, but the
 * shifting and meaning are different enough that I need distinct
 * type to keep them clear.
 */

typedef uint32_t pa_matom_t;	/* Identical to pa_atom_t; for mmap atoms */
#define PA_NULL_MATOM	((pa_matom_t) 0)

typedef uint32_t pa_page_t;	/* Type for page numbers */

/* An offset within the database */
typedef ptrdiff_t pa_offset_t;	/* Byte offset in the memory segment */

typedef uint8_t pa_shift_t;	/* Shift value */

#define PA_ASSERT(_a, _b) assert(_b)

static inline uint32_t
pa_roundup_shift32 (uint32_t val, pa_shift_t shift)
{
    return (val + (1 << shift) - 1) & ~((1 << shift) - 1);
}

static inline uint32_t
pa_roundup32 (uint32_t val, uint32_t rnd)
{
    return (val + rnd - 1) & ~(rnd - 1);
}

static inline uint32_t
pa_items_shift32 (uint32_t val, pa_shift_t shift)
{
    return (val + (1 << shift) - 1) >> shift;
}

static inline void *
pa_pointer (void *base, pa_atom_t atom, pa_shift_t shift)
{
    uint8_t *cp = base;

    return &cp[atom << shift];
}

static inline pa_atom_t
pa_atom (void *base, void *cur, pa_shift_t shift)
{
    ptrdiff_t delta = (uint8_t *) cur - (uint8_t *) base;

    return delta >> shift;
}

/*
 * Cheesy breakpoint for memory allocation failure
 */
void
pa_alloc_failed (const char *msg);

/*
 * Generate a warning
 */
void
pa_warning (int errnum, const char *fmt, ...);

/*
 * Allocating strings of length zero or one is a waste.  Instead,
 * we use a simple array containing each byte and a trailing NUL.
 * Then we can turn "x" in a reference into this table.  We handle
 * empty strings (len 0).  To handle PA_NULL_ATOM, we start our
 * numbering at 1, so "(pa_atom_t) 1" is the empty string, etc.
 */
#define PA_SHORT_STRINGS_MIN 1
#define PA_SHORT_STRINGS_MAX 256

extern char pa_short_strings[PA_SHORT_STRINGS_MAX * 2];

static inline pa_atom_t
pa_short_string_atom (const char *string)
{
    return PA_SHORT_STRINGS_MIN + *string;
}

static inline const char *
pa_short_string (pa_atom_t atom)
{
    return &pa_short_strings[(atom - PA_SHORT_STRINGS_MIN) << 1];
}

static inline int
pa_is_short_string (const char *string)
{
    return (&pa_short_strings[0] <= string
	    && string < &pa_short_strings[PA_SHORT_STRINGS_MAX * 2]);
}

#endif /* PARROTDB_PACOMMON_H */