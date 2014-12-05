/*
 * Small jpeg decoder library (Internal header)
 *
 * Copyright (c) 2006, Luc Saillard <luc@saillard.org>
 * Copyright (c) 2012 Intel Corporation.
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * - Redistributions of source code must retain the above copyright notice,
 *  this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright notice,
 *  this list of conditions and the following disclaimer in the documentation
 *  and/or other materials provided with the distribution.
 *
 * - Neither the name of the author nor the names of its contributors may be
 *  used to endorse or promote products derived from this software without
 *  specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */


#ifndef __TINYJPEG_INTERNAL_H_
#define __TINYJPEG_INTERNAL_H_

#include <setjmp.h>

#define HUFFMAN_BITS_SIZE  256

#define HUFFMAN_TABLES	   4
#define COMPONENTS	   4
#define JPEG_MAX_WIDTH	   2048
#define JPEG_MAX_HEIGHT	   2048
#define JPEG_SCAN_MAX	   4

struct huffman_table
{
  /*bits and values*/
	unsigned char bits[16];
	unsigned char values[256];
};

struct component 
{
  unsigned int Hfactor;
  unsigned int Vfactor;
  unsigned char quant_table_index;
  unsigned int cid;
};

struct jpeg_sos
{
  unsigned int nr_components;
  struct {
    unsigned int component_id;
    unsigned int dc_selector;
    unsigned int ac_selector;
  }components[4];
};

struct jdec_private_to_cl
{
  /* Public variables */
  unsigned int width, height;	/* We change this member because each frame of camera only 								      include one MJPEG image */

  /* Private variables */
  unsigned int stream_length;
  int scan_to_stream;
  int stream_to_begin;
  int stream;
  int pr_stream_begin;
  int pr_stream_end;
  int pr_stream;

  struct component component_infos[COMPONENTS];
  unsigned int nf_components;
  unsigned char Q_tables[COMPONENTS][64];		/* quantization tables, zigzag*/
  unsigned char Q_tables_valid[COMPONENTS];
  struct huffman_table HTDC[HUFFMAN_TABLES];	/* DC huffman tables   */
  unsigned char HTDC_valid[HUFFMAN_TABLES];
  struct huffman_table HTAC[HUFFMAN_TABLES];	/* AC huffman tables   */
  unsigned char HTAC_valid[HUFFMAN_TABLES];
  struct jpeg_sos cur_sos;  /* current sos values*/
  int default_huffman_table_initialized;
  int restart_interval;
};
#endif

