/*
 * $Id$
 *
 * Copyright (c) 2016, Juniper Networks, Inc.
 * All rights reserved.
 * This SOFTWARE is licensed under the LICENSE provided in the
 * ../Copyright file. By downloading, installing, copying, or otherwise
 * using the SOFTWARE, you agree to be bound by the terms of that
 * LICENSE.
 *
 * Phil Shafer (phil@) June 2016
 */

#ifndef LIBSLAX_XI_IO_H
#define LIBSLAX_XI_IO_H

typedef uint8_t xi_node_type_t;	/* Type of node (XI_TYPE_*) */
typedef uint32_t xi_node_id_t;	/* Node identifier (index) */
typedef uint16_t xi_name_id_t;	/* Element name identifier (index) */
typedef uint16_t xi_ns_id_t;	/* Namespace identifier (index) */
typedef uint8_t xi_depth_t;	/* Depth in the hierarchy */
typedef off_t xi_offset_t;	/* Offset in file or buffer */
typedef uint32_t xi_parse_flags_t; /* Flags for parser */

/* Type of XML nodes */
#define XI_TYPE_NONE	0	/* Unknown type */
#define XI_TYPE_EOF	1	/* End of file */
#define XI_TYPE_FAIL	2	/* Failure mode */
#define XI_TYPE_TEXT	3	/* Text content */
#define XI_TYPE_OPEN	4	/* Open tag */
#define XI_TYPE_CLOSE	5	/* Close tag */
#define XI_TYPE_EMPTY	6	/* Empty tag */
#define XI_TYPE_PI	7	/* Processing instruction */
#define XI_TYPE_DTD	8	/* <!DTD> nonsense */
#define XI_TYPE_COMMENT	9	/* Comment */
#define XI_TYPE_ATTR	10	/* XML attribute */
#define XI_TYPE_NS	11	/* XML namespace */

/*
 * Parser source object
 *
 * Note that we return pointers directly into our buffer.
 */
typedef struct xi_parse_source_s {
    int xps_fd;			/* File being read */
    char *xps_filename;		/* Filename */
    unsigned xps_lineno;	/* Line number of input */
    unsigned xps_offset;	/* Offset in the file */
    xi_parse_flags_t xps_flags;	/* Flags for this source */
    char *xps_bufp;		/* Input buffer */
    char *xps_curp;		/* Current data point */
    unsigned xps_len;		/* Number of bytes in the input buffer */
    unsigned xps_size;		/* Size of the input buffer (max) */
    xi_node_type_t xps_last;	/* Type of last token returned */
} xi_parse_source_t;

/* Flags for ps_flags: */
#define XPSF_MMAP	(1<<0)	/* File is mmap'd */
#define XPSF_IGNOREWS	(1<<1)	/* Ignore whitespace-only mixed content */
#define XPSF_NOREAD	(1<<2)	/* Don't read() on this fd */
#define XPSF_EOFSEEN	(1<<3)	/* EOF has been seen; read should fail */
#define XPSF_READALL	(1<<4)	/* File is read completely into memory */
#define XPSF_CLOSEFD	(1<<5)	/* Close fd when cleaning up */
#define XPSF_TRIMWS	(1<<6)	/* Trim whitespace from data */

xi_parse_source_t *
xi_parse_create (int fd, xi_parse_flags_t flags);

xi_parse_source_t *
xi_parse_open (const char *path, xi_parse_flags_t flags);

void
xi_parse_destroy (xi_parse_source_t *srcp);

xi_node_type_t
xi_parse_next_token (xi_parse_source_t *srcp, char **datap, char **restp);

#endif /* LIBSLAX_XI_IO_H */
