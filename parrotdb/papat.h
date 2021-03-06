/*
 * Copyright (c) 1996-2017, Juniper Networks, Inc.
 * All rights reserved.
 * This SOFTWARE is licensed under the LICENSE provided in the
 * ../Copyright file. By downloading, installing, copying, or otherwise
 * using the SOFTWARE, you agree to be bound by the terms of that
 * LICENSE.
 *
 * Phil Shafer <phil@>, April 2016
 */

#ifndef PARROTDB_PAPAT_H
#define PARROTDB_PAPAT_H

/**
 * @file pa_pat.h
 * @brief Patricia tree APIs
 *
 * This file contains the public data structures for the patricia tree
 * package.  This package is applicable to searching a non-overlapping
 * keyspace of pseudo-random data with fixed size keys.  The patricia tree
 * is not balanced, so this package is NOT appropriate for highly skewed
 * data.  The package can deal with variable length (in byte increments)
 * keys, but only when it can be guaranteed that no key in the tree is a
 * prefix of another (null-terminated strings have this property if you
 * include the '\\0' in the key).
 *
 * ....
 *
 * Generally you will not want to deal with the patricia structure
 * directly, so it's helpful to be able to be able to get back to the
 * primary structure.  This can be done with the PATNODE_TO_STRUCT() macro.
 * Using this, you can then easily define functions which completely hide
 * the patricia structure from the rest of your code.  This is STRONGLY
 * recommended.
 */

/* Wrapper for our "patricia tree node" atom */
PA_ATOM_TYPE(pa_pat_atom_t, pa_pat_atom_s, ppa_atom,
	     pa_pat_is_null, pa_pat_atom, pa_pat_atom_of, pa_pat_null_atom);

/* Wrapper for our "data node" atom */
PA_ATOM_TYPE(pa_pat_data_atom_t, pa_pat_data_atom_s, ppa_data_atom,
	     pa_pat_data_is_null, pa_pat_data_atom, pa_pat_data_atom_of,
	     pa_pat_data_null_atom);

/**
 * @brief
 * Patricia tree node.
 */
typedef struct pa_pat_node_s {
    uint16_t ppn_length;       /**< length of key, formated like bit */
    uint16_t ppn_bit;	       /**< bit number to test for patricia */
    pa_pat_atom_t ppn_left;    /**< Atom of left node of patricia tree */
    pa_pat_atom_t ppn_right;   /**< Atom of right node of patricia tree */
    pa_pat_data_atom_t ppn_data; /**< Atom of data node (in some other tree) */
} pa_pat_node_t;

/**
 * @brief
 * The maximum length of a key, in bytes.
 */
#define	PA_PAT_MAXKEY		256

/**
 * @brief
 * A macro to initialize the `length' in a patnode at compile time given
 * the length of a key.  Good for the keyword tree.  Note the length
 * must be greater than zero.
 */
#define	PA_PAT_LEN_TO_BIT(len)  ((uint16_t) ((((len) - 1) << 8) | 0xff))

/**
 * @brief
 * Patricia tree root.
 */
typedef struct pa_pat_info_s {
    pa_pat_atom_t ppi_root;	/**< root patricia node (atom) */
    uint16_t ppi_key_bytes;	/**< (maximum) key length in bytes */
} pa_pat_info_t;

struct pa_pat_s;		/* Forward declaration */
typedef const psu_byte_t *(*pa_pat_key_func_t)(struct pa_pat_s *,
					       pa_pat_data_atom_t);

typedef struct pa_pat_s {
    pa_pat_info_t *pp_infop;	/* Pointer to root info */
    pa_mmap_t *pp_mmap;		/* Underlaying mmap */
    pa_fixed_t *pp_nodes;	/* Fixed paged array of nodes */
    void *pp_data;		/* Opaque data tree */
    pa_pat_key_func_t pp_key_func; /* Find the key for a node */
} pa_pat_t;

/* Shorthand for fields */
#define pp_root pp_infop->ppi_root
#define pp_key_bytes pp_infop->ppi_key_bytes

static inline pa_fixed_atom_t
pa_pat_to_fixed (pa_pat_atom_t atom)
{
    return pa_fixed_atom(pa_pat_atom_of(atom));
}

static inline pa_pat_atom_t
pa_pat_from_fixed (pa_fixed_atom_t atom)
{
    return pa_pat_atom(pa_fixed_atom_of(atom));
}

static inline pa_pat_atom_t
pa_pat_atom_alloc (pa_pat_t *root)
{
    pa_fixed_atom_t atom = pa_fixed_alloc_atom(root->pp_nodes);
    return pa_pat_atom(pa_fixed_atom_of(atom));
}

/**
 * @brief
 * Turn an atom into a node pointer
 *
 * @param[in] root
 *     Pointer to patricia tree root
 * @param[in] atom
 *     Atom within the patricia tree
 */
static inline pa_pat_node_t *
pa_pat_node (pa_pat_t *root, pa_pat_atom_t atom)
{
    return pa_fixed_atom_addr(root->pp_nodes, pa_pat_to_fixed(atom));
}

static inline pa_pat_data_atom_t
pa_pat_node_data (pa_pat_t *root UNUSED, pa_pat_node_t *node)
{
    return node ? node->ppn_data : pa_pat_data_null_atom();
}

/**
 * @brief
 * Initializes a patricia tree root.
 *
 * @param[in] root
 *     Pointer to patricia tree root
 * @param[in] key_is_ptr
 *     Indicator that the key located at the offset is actually a
 *     pointer or not
 * @param[in] key_bytes
 *     Number of bytes in the key
 * @param[in] key_offset
 *     Offset to the key from the end of the patricia tree node structure
 *
 * @return 
 *     A pointer to the patricia tree root.
 */
pa_pat_t *
pa_pat_root_init (pa_pat_t *root, pa_pat_info_t *ppip, pa_mmap_t *pmp,
		  pa_fixed_t *nodes, void *data_store,
		  pa_pat_key_func_t key_func, uint16_t klen);

const psu_byte_t *
pa_pat_istr_key_func (pa_pat_t *pp, pa_pat_node_t *node);

/*
 * Add a node to the patricia tree.
 */
psu_boolean_t
pa_pat_add_node (pa_pat_t *root, pa_pat_atom_t atom, pa_pat_node_t *node);

/**
 * @brief
 * Deletes a patricia tree root.
 *
 * @note An assertion failure will occur if the tree is not empty when this
 *       function is called.
 *
 * @param[in] root
 *     Pointer to patricia tree root
 */
void
pa_pat_root_delete (pa_pat_t *root);

/**
 * @brief
 * Initializes a patricia tree node using the key length specified by
 * @c key_bytes.  If @c key_bytes is zero, then it falls back to the
 * length specified during root initialization (@c patricia_root_init).
 *
 * @param[in] node
 *     Pointer to patricia tree node
 * @param[in] key_bytes
 *     Length of the key, in bytes
 * @param[in] datom
 *     Atom number of node in data store
 */
void
pa_pat_node_init_length (pa_pat_node_t *node, uint16_t key_bytes,
			 pa_pat_data_atom_t datom);

/**
 * @brief
 * Adds a new node to the tree.
 *
 * @param[in] root
 *     Pointer to patricia tree root
 * @param[in] node
 *     Pointer to patricia tree node
 *
 * @return 
 *     @c TRUE if the node is successfully added; 
 *     @c FALSE if the key you are adding is the same as, or overlaps 
 *      with (variable length keys), something already in the tree.
 */
psu_boolean_t
pa_pat_add (pa_pat_t *root, pa_pat_data_atom_t datom, uint16_t key_bytes);

/**
 * @brief
 * Deletes a node from the tree.
 *
 * @param[in] root
 *     Pointer to patricia tree root
 * @param[in] node
 *     Pointer to patricia tree node
 *
 * @return 
 *     @c TRUE if the node is successfully deleted; 
 *     @c FALSE if the specified node is not in the tree.
 */
psu_boolean_t
pa_pat_delete (pa_pat_t *root, pa_pat_node_t *node);

/**
 * @brief
 * Given a node in the tree, find the node with the next numerically larger
 * key.  If the given node is NULL, the numerically smallest node in the tree
 * will be returned.
 *
 * @note Good for tree walks.
 *
 * @param[in] root
 *     Pointer to patricia tree root
 * @param[in] node
 *     Pointer to patricia tree node
 *
 * @return 
 *     @c NULL if the specified node is already the largest; 
 *     otherwise a pointer to the node with the next numerically larger key.
 */
pa_pat_node_t *
pa_pat_find_next (pa_pat_t *root, pa_pat_node_t *node);

/**
 * @brief
 * Given a node in the tree, find the node with the next numerically smaller
 * key.  If the given node is NULL, returns the numerically largest node in
 * the tree.
 *
 * @note The definitions of pa_pat_find_next() and pa_pat_find_prev() are
 *       such that
 *
 * @code
 * node == pa_pat_find_prev(root, pa_pat_find_next(root, node));
 * @endcode
 *
 * will always be @c TRUE for any node in the tree or @c NULL.
 *
 * @param[in] root
 *     Pointer to patricia tree root
 * @param[in] node
 *     Pointer to patricia tree node
 *
 * @return 
 *     @c NULL if the specified node was the smallest; 
 *      otherwise a pointer to the patricia tree node with next numerically 
 *      smaller key.
 */
pa_pat_node_t *
pa_pat_find_prev (pa_pat_t *root, pa_pat_node_t *node);

/**
 * @brief
 * Given a prefix and a prefix length in bits, find the node with the
 * numerically smallest key in the tree which includes this prefix.
 *
 * @param[in] root
 *     Pointer to patricia tree root
 * @param[in] prefix_len
 *     Length of prefix, in bits
 * @param[in] prefix
 *     Pointer to prefix
 *
 * @return 
 *     @c NULL if no node in the tree has the prefix; 
 *     otherwise a pointer to the patricia tree node with the 
 *     numerically smallest key which includes the prefix.
 */
pa_pat_node_t *
pa_pat_subtree_match (pa_pat_t *root, uint16_t prefix_len,
			const void *prefix);

/**
 * @brief
 * Given a node in the tree, and a prefix length in bits, return the next
 * numerically larger node in the tree which shares a prefix with the node
 * specified.
 *
 * @param[in] root
 *     Pointer to patricia tree root
 * @param[in] node
 *     Pointer to patricia tree node
 * @param[in] prefix_len
 *     Length of prefix, in bits
 *
 * @return 
 *     A pointer to next numerically larger patricia tree node.
 */
pa_pat_node_t *
pa_pat_subtree_next (pa_pat_t *root, pa_pat_node_t *node,
		     uint16_t prefix_len);

/**
 * @brief
 * Looks up a node having the specified key and key length in bytes.  
 *
 * @param[in] root
 *     Pointer to patricia tree root
 * @param[in] key_bytes
 *     Number of bytes in key
 * @param[in] key
 *     Pointer to key value
 *
 * @return 
 *     @c NULL if a match is not found; 
 *     otherwise a pointer to the matching patricia tree node
 */
pa_pat_node_t *
pa_pat_get (pa_pat_t *root, uint16_t key_bytes, const void *key);

/**
 * @brief
 * Given a key and key length in bytes, return a node in the tree which is at
 * least as large as the key specified.  
 *
 * The call has a parameter which
 * modifies its behaviour when an exact match is found in the tree; you can
 * either choose to have it return the exact match, if there is one, or you
 * can have it always return a node with a larger key (a la SNMP getnext).
 *
 * @param[in] root
 *     Pointer to patricia tree root
 * @param[in] key_bytes
 *     Number of bytes in key
 * @param[in] key
 *     Pointer to key value
 * @param[in] return_eq
 *     FALSE for classic getnext
 *
 * @return 
 *     A pointer to patricia tree node.
 */
pa_pat_node_t *
pa_pat_getnext (pa_pat_t *root, uint16_t key_bytes, const void *key,
		  psu_boolean_t return_eq);

/**
 * @brief
 * Determines if a patricia tree node is contained within a tree.
 *
 * @param[in] node
 *     Pointer to patricia tree node
 *
 * @return 
 *      @c TRUE if the node is in the tree; @c FALSE otherwise.
 */
psu_boolean_t
pa_pat_node_in_tree (const pa_pat_node_t *node);

/**
 * @brief
 * Given two patricia tree nodes, determine if the first has a key which is
 * numerically lesser, equal, or greater than the key of the second.
 *
 * @param[in] root
 *     Pointer to patricia tree root
 * @param[in] left
 *     Pointer to patricia tree node
 * @param[in] right
 *     Pointer to patricia tree node
 *
 * @return 
 *     Result of the comparison:
 *     @li -1 if the key of the left node is numerically lesser than the right
 *     @li 0 if the keys match
 *     @li 1 if the left key is numerically greater than the right
 */
int
pa_pat_compare_nodes (pa_pat_t *root, pa_pat_node_t *left,
		      pa_pat_node_t *right);

/*
 * utility functions for dealing with const trees -- useful for 
 * iterator functions that shouldn't be able to change the contents
 * of the tree, both as far as tree structure and as far as node content.. 
 */

/**
 * @brief
 * Constant tree form of pa_pat_get()
 *
 * @param[in] root
 *     Pointer to patricia tree root
 * @param[in] key_bytes
 *     Number of bytes in key
 * @param[in] key
 *     Pointer to key value
 *
 * @return
 *     @c NULL if a match is not found; 
 *     Otherwise a @c const pointer to the matching patricia tree node.
 *
 * @sa pa_pat_get
 */
const pa_pat_node_t *
pa_pat_cons_get (const pa_pat_t *root, const uint16_t key_bytes, 
		   const void *key);

/**
 * @brief
 * Constant tree form of pa_pat_find_next()
 *
 * @param[in] root
 *     Pointer to patricia tree root
 * @param[in] node
 *     Pointer to patricia tree node
 *
 * @return
 *     @c NULL if a match is not found; 
 *     Otherwise a @c const pointer to the next patricia tree node.
 *
 * @sa pa_pat_find_next
 */
const pa_pat_node_t *
pa_pat_cons_find_next (const pa_pat_t *root, const pa_pat_node_t *node);

/**
 * @brief
 * Constant tree form of pa_pat_find_prev()
 *
 * @param[in] root
 *     Pointer to patricia tree root
 * @param[in] node
 *     Pointer to patricia tree node
 *
 * @return
 *     @c NULL if a match is not found; 
 *     Otherwise a @c const pointer to the previous patricia tree node.
 *
 * @sa pa_pat_find_prev
 */
const pa_pat_node_t *
pa_pat_cons_find_prev (const pa_pat_t *root, const pa_pat_node_t *node);

/**
 * @brief
 * Constant tree form of pa_pat_subtree_match()
 *
 * @param[in] root
 *     Pointer to patricia tree root
 * @param[in] prefix_len
 *     Length of prefix, in bits
 * @param[in] prefix
 *     Pointer to prefix
 *
 * @return
 *     @c NULL if no node in the tree has the prefix; 
 *     Otherwise a pointer to the patricia tree node with the 
 *     numerically smallest key which includes the prefix.
 * 
 * @sa pa_pat_subtree_match
 */
const pa_pat_node_t *
pa_pat_cons_subtree_match (const pa_pat_t *root,
			   const uint16_t prefix_len, const void *prefix);

/**
 * @brief
 * Constant tree form of pa_pat_subtree_next()
 *
 * @param[in] root
 *     Pointer to patricia tree root
 * @param[in] node
 *     Pointer to patricia tree node
 * @param[in] prefix_len
 *     Length of prefix, in bits
 *
 * @return
 *     A @c const pointer to next numerically larger patricia tree node.
 * 
 * @sa pa_pat_subtree_next
 */
const pa_pat_node_t *
pa_pat_cons_subtree_next (const pa_pat_t *root, const pa_pat_node_t *node,
			  const uint16_t prefix_len);

/*
 * Inlines, for performance
 * 
 * All contents below this line are subject to change without notice.
 * Don't go poking into the implementation details here...
 */

/**
 * @brief
 * Initializes a patricia tree node using the the key length specified
 * during root initialization (@c pa_pat_root_init).
 * 
 * @param[in] node
 *     Pointer to patricia tree node
 */
static inline void
pa_pat_node_init (pa_pat_node_t *node)
{
    pa_pat_node_init_length(node, 0, pa_pat_data_null_atom());
} 

/**
 * @brief
 * Bit number when no external node
 */
#define	PA_PAT_NOBIT  (0)

/**
 * @brief
 * Obtains a pointer to the start of the key material for a patricia node.
 *
 * @param[in]  root
 *     Pointer to patricia tree root
 * @param[in] node
 *     Pointer to patricia tree node
 *
 * @return 
 *     A pointer to the start of node key.
 */
static inline const psu_byte_t *
pa_pat_key (pa_pat_t *root, pa_pat_node_t *node)
{
    return root->pp_key_func(root, node->ppn_data);
}

static inline const psu_byte_t *
pa_pat_key_atom (pa_pat_t *root, pa_pat_atom_t atom)
{
    pa_pat_node_t *node = pa_pat_node(root, atom);
    if (node == NULL)
	return NULL;

    return root->pp_key_func(root, node->ppn_data);
}

/**
 * @brief
 * Performs a bit test on a key.
 *
 * @param[in]  key
 *     Pointer to key material
 * @param[in]  bit
 *     Bit number to test
 *
 * @return 
 *     1 if the bit was set, 0 otherwise.
 */
static inline psu_boolean_t
pat_key_test (const psu_byte_t *key, uint16_t bit)
{
    return (key[bit >> 8] & (~bit & 0xff));
}

/**
 * @brief
 * Given a node, determines the key length in bytes.
 *
 * @param[in]  node
 *     Pointer to patricia tree node
 *
 * @return 
 *     The key length, in bytes.
 */
static inline uint16_t
pa_pat_length (pa_pat_node_t *node)
{
    return (node->ppn_length >> 8) + 1;
}

/**
 * @brief
 * Given the length of a key in bytes, converts it to patricia bit format.
 *
 * @param[in]  length
 *     Length of a key, in bytes
 *
 * @return 
 *     Patricia bit format or @c PA_PAT_NOBIT if length is 0.
 */
static inline uint16_t
pa_pat_length_to_bit (uint16_t length)
{
    if (length)
	return ((length - 1) << 8) | 0xff;

    return PA_PAT_NOBIT;
}

/**
 * @brief
 * Finds an exact match for the specified key and key length.
 * 
 * @param[in] root
 *     Pointer to patricia tree root
 * @param[in] key_bytes
 *     Length of the key in bytes
 * @param[in] v_key
 *     Key to match
 *
 * @return 
 *     A pointer to the pa_pat_node_t containing the matching key;
 *     @c NULL if not found
 */
static inline pa_pat_node_t *
pa_pat_get_inline (pa_pat_t *root, uint16_t key_bytes, const void *v_key)
{
    pa_pat_atom_t current;
    uint16_t bit, bit_len;
    const psu_byte_t *key = v_key;

    if (key_bytes == 0)
	abort();

    current = root->pp_root;
    if (pa_pat_is_null(current))
	return NULL;

    /*
     * Waltz down the tree.  Stop when the bits appear to go backwards.
     */
    bit = PA_PAT_NOBIT;
    bit_len = pa_pat_length_to_bit(key_bytes);

    pa_pat_node_t *node = pa_pat_node(root, current);
    while (bit < node->ppn_bit) {
	if (node == NULL)
	    return NULL;

	bit = node->ppn_bit;
	if (bit < bit_len && pat_key_test(key, bit)) {
	    current = node->ppn_right;
	} else {
	    current = node->ppn_left;
	}
	node = pa_pat_node(root, current);
    }

    /*
     * If the lengths don't match we're screwed.  Otherwise do a compare.
     */
    if (node->ppn_length != bit_len
	|| bcmp(pa_pat_key(root, node), key, key_bytes))
	return NULL;

    return node;
}

/**
 * @brief
 * Determines if a patricia tree is empty.
 *
 * @param[in]  root
 *     Pointer to patricia tree root
 *
 * @return 
 *     1 if the tree is empty, 0 otherwise.
 */
static inline psu_boolean_t
pa_pat_isempty (pa_pat_t *root)
{
    return pa_pat_is_null(root->pp_root);
}


/**
 * @brief
 * Returns the sizeof for an element in a structure.
 */
 
#define STRUCT_SIZEOF(_structure, _element) \
           (sizeof(((_structure*)0)->_element))

/**
 * @brief
 * Returns the offset of @a _elt2 from the END of @a _elt1 within @a structure.
 */

#define STRUCT_OFFSET(_structure, _elt1, _elt2)				\
           (offsetof(_structure, _elt2) - (offsetof(_structure, _elt1) + \
                                      STRUCT_SIZEOF(_structure, _elt1)))

/**
 * @brief
 * Macro to define an inline to map from a pa_pat_node_t entry back to the
 * containing data structure.
 *
 * This is just a handy way of defining the inline, which will return
 * @c NULL if the pa_pat_node_t pointer is @c NULL, or the enclosing structure
 * if not.
 *
 * The @c assert() will be removed by the compiler unless your code 
 * is broken -- this is quite useful as a way to validate that you've
 * given the right field for fieldname (a common mistake is to give
 * the KEY field instead of the NODE field).  It's harmless.
 */
#define PATNODE_TO_STRUCT(procname, structname, fieldname)		\
static inline structname * procname (pa_pat_node_t *ptr)		\
{									\
    assert(STRUCT_SIZEOF(structname, fieldname) == sizeof(pa_pat_node_t)); \
    if (ptr)								\
	return QUIET_CAST(structname *, ((u_char *) ptr) -		\
				    offsetof(structname, fieldname));	\
    return NULL;							\
}

/**
 * @brief
 * Constant version of the macro to define an inline to map from a
 * pa_pat_node_t entry back to the containing data structure.
 */
#define PATNODE_TO_CONS_STRUCT(procname, structname, fieldname)		\
static inline const structname *					\
procname (pa_pat_node_t *ptr)						\
{									\
    assert(STRUCT_SIZEOF(structname, fieldname) == sizeof(pa_pat_node_t)); \
    if (ptr) {								\
        return (const structname *) ((uchar *) ptr) -			\
            offsetof(structname, fieldname);				\
    }									\
    return NULL;							\
}


/**
 * @brief
 * Initialize a patricia root with a little more 
 * compile-time checking. 
 */

#define PA_PAT_ROOT_INIT(_rootptr, _bool_key_is_ptr, _nodestruct,	\
			   _nodeelement, _keyelt)			\
           pa_pat_root_init(_rootptr, _bool_key_is_ptr,			\
                      STRUCT_SIZEOF(_nodestruct, _keyelt),		\
                      STRUCT_OFFSET(_nodestruct, _nodeelement, _keyelt))

/**
 * @internal
 *
 * @brief
 * Look up a node having the specified fixed length key.
 *
 * The key length provided at initialization time will be used.  For trees
 * with non-fixed lengths, pa_pat_get() should be used instead, as the length
 * will need to be specified.
 *
 * @param[in] root
 *     Pointer to patricia tree root
 * @param[in] key
 *     Pointer to key value
 *
 * @return
 *     @c NULL if a match is not found;
 *     otherwise a pointer to the matchin patricia tree node
 *
 * @sa pa_pat_get
 */
static inline pa_pat_node_t *
pa_pat_lookup (pa_pat_t *root, const void *key)
{
    return pa_pat_get(root, root->pp_key_bytes, key);
}

/**
 * @internal
 *
 * @brief
 * Given a fixed length key, return a node in the tree which is at least
 * as large as the key specified.
 *
 * The key length provided at initialization time will be used.  For trees
 * with non-fixed length keys, pa_pat_getnext() should be used instead, as
 * the length of the key will need to be specified.
 *
 * @param[in] root
 *     Pointer to patricia tree root
 * @param[in] key
 *     Pointer to key value
 *
 * @return
 *     A pointer to patricia tree node.
 */
static inline pa_pat_node_t *
pa_pat_lookup_geq (pa_pat_t *root, void *key)
{
    return pa_pat_getnext(root, root->pp_key_bytes, key, TRUE);
}

/*
 * pat_makebit
 */
extern const uint8_t pa_pat_hi_bit_table[];

/**
 * @interal
 * @brief
 * Given a byte number and a bit mask, make a bit index.
 *
 * @param[in]  offset
 *     Offset byte number
 * @param[in] bit_in_byte
 *     Bit within the byte
 *
 * @return 
 *     Bit index value
 */
static inline uint16_t
pa_pat_makebit (uint16_t offset, uint8_t bit_in_byte)
{
    return ((offset & 0xff) << 8) | (~pa_pat_hi_bit_table[bit_in_byte] & 0xff);
}

/*
 * Allocate and populate a node
 */
static inline pa_pat_node_t *
pa_pat_node_alloc (pa_pat_t *root, pa_pat_data_atom_t datom,
		   uint16_t key_bytes, pa_pat_atom_t *atomp)
{
    pa_pat_node_t *node = NULL;
    pa_pat_atom_t atom = pa_pat_atom_alloc(root);
    if (!pa_pat_is_null(atom)) {
	node = pa_pat_node(root, atom);
	if (node != NULL)
	    pa_pat_node_init_length(node, key_bytes, datom);
    }

    *atomp = atom;
    return node;
}

pa_pat_t *
pa_pat_open_nodes (pa_mmap_t *pmp, const char *name, pa_fixed_t *nodes,
		   void *data_store, pa_pat_key_func_t key_func,
		   uint16_t klen);

pa_pat_t *
pa_pat_open_c (pa_mmap_t *pmp, const char *name,
	       void *data_store, pa_pat_key_func_t key_func, uint16_t klen);

pa_pat_t *
pa_pat_open (pa_mmap_t *pmp, const char *name,
	     void *data_store, pa_pat_key_func_t key_func,
	     uint16_t klen, pa_shift_t shift, uint32_t max_atoms);

void
pa_pat_close (pa_pat_t *ppp);

static inline pa_pat_data_atom_t
pa_pat_get_atom (pa_pat_t *root, uint16_t key_bytes, const void *key)
{
    pa_pat_node_t *nodep = pa_pat_get(root, key_bytes, key);
    return pa_pat_node_data(root, nodep);
}

#endif /* PARROTDB_PAPAT_H */
