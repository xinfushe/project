#define DEBUG 0
#define SANITY_CHECK 1

#define be16_to_cpu(x) (((x)[0]<<8)|(x)[1])

#define HUFFMAN_BITS_SIZE  256

#define HUFFMAN_TABLES	   4
#define COMPONENTS	   4
#define JPEG_MAX_WIDTH	   2048
#define JPEG_MAX_HEIGHT	   2048
#define JPEG_SCAN_MAX	   4

#define cY	0
#define cCb	1
#define cCr	2

#define  DQT   0xDB /* Define Quantization Table */
#define  SOF   0xC0 /* Start of Frame (size information) */
#define  DHT   0xC4 /* Huffman Table */
#define  SOI   0xD8 /* Start of Image */
#define  SOS   0xDA /* Start of Scan */
#define  RST   0xD0 /* Reset Marker d0 -> .. */
#define  RST7  0xD7 /* Reset Marker .. -> d7 */
#define  EOI   0xD9 /* End of Image */
#define  DRI   0xDD /* Define Restart Interval */
#define  APP0  0xE0

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

/**
 * \brief Huffman table for JPEG decoding.
 *
 * This structure holds the complete Huffman tables. This is an
 * aggregation of all Huffman table (DHT) segments maintained by the
 * application. i.e. up to 2 Huffman tables are stored in there for
 * baseline profile.
 *
 * The #load_huffman_table array can be used as a hint to notify the
 * VA driver implementation about which table(s) actually changed
 * since the last submission of this buffer.
 */
typedef struct _VAHuffmanTableBufferJPEGBaseline {
    /** \brief Specifies which #huffman_table is valid. */
    unsigned char       load_huffman_table[2];
    /** \brief Huffman tables indexed by table identifier (Th). */
    struct {
        /** @name DC table (up to 12 categories) */
        /**@{*/
        /** \brief Number of Huffman codes of length i + 1 (Li). */
        unsigned char   num_dc_codes[16];
        /** \brief Value associated with each Huffman code (Vij). */
        unsigned char   dc_values[12];
        /**@}*/
        /** @name AC table (2 special codes + up to 16 * 10 codes) */
        /**@{*/
        /** \brief Number of Huffman codes of length i + 1 (Li). */
        unsigned char   num_ac_codes[16];
        /** \brief Value associated with each Huffman code (Vij). */
        unsigned char   ac_values[162];
        /** \brief Padding to 4-byte boundaries. Must be set to zero. */
        unsigned char   pad[2];
        /**@}*/
    }                   huffman_table[2];
} VAHuffmanTableBufferJPEGBaseline;

__constant VAHuffmanTableBufferJPEGBaseline default_huffman_table_param={
    huffman_table:
    {
        // lumiance component
        {
            num_dc_codes:{0,1,5,1,1,1,1,1,1,0,0,0}, // 12 bits is ok for baseline profile
            dc_values:{0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b},
            num_ac_codes:{0,2,1,3,3,2,4,3,5,5,4,4,0,0,1,125},
            ac_values:{
              0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12,
              0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
              0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08,
              0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0,
              0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16,
              0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28,
              0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
              0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
              0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
              0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
              0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
              0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
              0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
              0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
              0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,
              0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5,
              0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4,
              0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2,
              0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,
              0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
              0xf9, 0xfa
           },/*,0xonly,0xthe,0xfirst,0x162,0xbytes,0xare,0xavailable,0x*/
        },
        // chrom component
        {
            num_dc_codes:{0,3,1,1,1,1,1,1,1,1,1,0}, // 12 bits is ok for baseline profile
            dc_values:{0,1,2,3,4,5,6,7,8,9,0xa,0xb},
            num_ac_codes:{0,2,1,2,4,4,3,4,7,5,4,4,0,1,2,119},
            ac_values:{
              0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21,
              0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71,
              0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91,
              0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0,
              0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34,
              0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26,
              0x27, 0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38,
              0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
              0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
              0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
              0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
              0x79, 0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
              0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96,
              0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5,
              0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4,
              0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3,
              0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2,
              0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,
              0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
              0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
              0xf9, 0xfa
            },/*,0xonly,0xthe,0xfirst,0x162,0xbytes,0xare,0xavailable,0x*/
        },
    }
};

int build_default_huffman_tables(__global struct jdec_private_to_cl *priv)
{
    int i = 0;
    if (priv->default_huffman_table_initialized)
	return 0;

    int k;
    __global unsigned char *dst_gp;
    __constant unsigned char *src_gp;
    for (i = 0; i < 4; i++) {
        priv->HTDC_valid[i] = 1;
        dst_gp = priv->HTDC[i].bits;
        src_gp = default_huffman_table_param.huffman_table[i].num_dc_codes;
	for(k = 0; k < 16; k++)
	   *(dst_gp + k) = *(src_gp + k);
        dst_gp = priv->HTDC[i].values;
        src_gp = default_huffman_table_param.huffman_table[i].dc_values;
	for(k = 0; k < 16; k++)
	   *(dst_gp + k) = *(src_gp + k);
        //memcpy(priv->HTDC[i].bits, default_huffman_table_param.huffman_table[i].num_dc_codes, 16);
        //memcpy(priv->HTDC[i].values, default_huffman_table_param.huffman_table[i].dc_values, 16);
        priv->HTAC_valid[i] = 1;
        dst_gp = priv->HTAC[i].bits;
        src_gp = default_huffman_table_param.huffman_table[i].num_ac_codes;
	for(k = 0; k < 16; k++)
	   *(dst_gp + k) = *(src_gp + k);
        dst_gp = priv->HTAC[i].values;
        src_gp = default_huffman_table_param.huffman_table[i].ac_values;
	for(k = 0; k < 256; k++)
	   *(dst_gp + k) = *(src_gp + k);
        //memcpy(priv->HTAC[i].bits, default_huffman_table_param.huffman_table[i].num_ac_codes, 16);
        //memcpy(priv->HTAC[i].values, default_huffman_table_param.huffman_table[i].ac_values, 256);
    }
    priv->default_huffman_table_initialized = 1;
    return 0;
}

int parse_DQT(__global struct jdec_private_to_cl *priv, __global  unsigned char *stream)
{
  int qi;
  __global  unsigned char *dqt_block_end;

  dqt_block_end = stream + be16_to_cpu(stream);
  stream += 2;	/* Skip length */

  int i;
  __global unsigned char *gp;
  while (stream < dqt_block_end)
   {
     qi = *stream++;
#if SANITY_CHECK
     if (qi>>4){
	return -1;
     }
     if (qi>4){
	return -1;
     }
#endif
     gp = priv->Q_tables[qi&0x0F]; 
     for(i = 0; i< 64; i++){
       *(gp + i) =  *(stream + i);
     }
     //memcpy(priv->Q_tables[qi&0x0F], stream, 64);
     priv->Q_tables_valid[qi & 0x0f] = 1;
     stream += 64;
   }
  return 0;
}

int parse_SOF(__global struct jdec_private_to_cl *priv, __global  unsigned char *stream)
{
  int i, width, height, nr_components, cid, sampling_factor;
  unsigned char Q_table;
  __global struct component *c;

  height = be16_to_cpu(stream+3);
  width  = be16_to_cpu(stream+5);
  nr_components = stream[7];
  priv->nf_components = nr_components;
#if SANITY_CHECK
  if (stream[2] != 8){
	return -1;
  }
  if (nr_components != 3){
	return -1;
  }
  /*if (width>JPEG_MAX_WIDTH || height>JPEG_MAX_HEIGHT){}
  if (height%16){}
  if (width%16){}*/
#endif
  stream += 8;
  for (i=0; i<nr_components; i++) {
     cid = *stream++;
     sampling_factor = *stream++;
     Q_table = *stream++;
     c = &priv->component_infos[i];
     c->cid = cid;
     if (Q_table >= COMPONENTS){
	return -1;
     }
     c->Vfactor = sampling_factor&0xf;
     c->Hfactor = sampling_factor>>4;
     c->quant_table_index = Q_table;

  }
  priv->width = width;
  priv->height = height;

  return 0;
}

int parse_SOS(__global struct jdec_private_to_cl *priv, __global  unsigned char *stream, __global  unsigned char **pt_pr_stream)
{
  unsigned int i, cid, table;
  unsigned int nr_components = stream[2];

  priv->cur_sos.nr_components= nr_components;

  stream += 3;
  for (i=0;i<nr_components;i++) {
     cid = *stream++;
     table = *stream++;
     priv->cur_sos.components[i].component_id = cid;
     priv->cur_sos.components[i].dc_selector = ((table>>4)&0x0F);
     priv->cur_sos.components[i].ac_selector = (table&0x0F);
#if SANITY_CHECK
     if ((table&0xf)>=4){
	return -1;
     }
     if ((table>>4)>=4){
	return -1;
     }
     if (cid != priv->component_infos[i].cid){
	return -1;
     }
#endif
  }
  *pt_pr_stream = stream+3;
  return 0;
}

/*int tinyjpeg_parse_SOS(struct jdec_private_to_cl *priv,  unsigned char *stream)
{
    return parse_SOS(priv, stream);
}*/

int parse_DHT(__global struct jdec_private_to_cl *priv, __global  unsigned char *stream)
{
  unsigned int count, i;
  int length, index;
  unsigned char Tc, Th;

  length = be16_to_cpu(stream) - 2;
  stream += 2;	/* Skip length */

  __global unsigned char *gp;
  int k;

  while (length>0) {
     index = *stream++;

     Tc = index & 0xf0; // it is not important to <<4
     Th = index & 0x0f;
     if (Tc) {
        gp = priv->HTAC[index & 0xf].bits;
	for(k = 0; k < 16; k++)
	   *(gp + k) = *(stream + k);
        //memcpy(priv->HTAC[index & 0xf].bits, stream, 16);
     }
     else {
        gp = priv->HTDC[index & 0xf].bits;
	for(k = 0; k < 16; k++)
	   *(gp + k) = *(stream + k);
        //memcpy(priv->HTDC[index & 0xf].bits, stream, 16);
     }

     count = 0;
     for (i=0; i<16; i++) {
        count += *stream++;
     }

#if SANITY_CHECK
     if (count >= HUFFMAN_BITS_SIZE){
	return -1;
     }
     if ( (index &0xf) >= HUFFMAN_TABLES){
	return -1;
     }
#endif

     if (Tc) {
        gp = priv->HTAC[index & 0xf].values;
	for(k = 0; k < count; k++)
	   *(gp + k) = *(stream + k);
        //memcpy(priv->HTAC[index & 0xf].values, stream, count);
        priv->HTAC_valid[index & 0xf] = 1;
     }
     else {
        gp = priv->HTDC[index & 0xf].values;
	for(k = 0; k < count; k++)
	   *(gp + k) = *(stream + k);
        //memcpy(priv->HTDC[index & 0xf].values, stream, count);
        priv->HTDC_valid[index & 0xf] = 1;
     }

     length -= 1;
     length -= 16;
     length -= count;
     stream += count;
  }
  return 0;
}

int parse_DRI(__global struct jdec_private_to_cl *priv, __global  unsigned char *stream)
{
  unsigned int length;

  length = be16_to_cpu(stream);

#if SANITY_CHECK
  if (length != 4){
	return -1;
  }
#endif

  priv->restart_interval = be16_to_cpu(stream+2);

  return 0;
}

int findSOI(__global struct jdec_private_to_cl *priv, __global  unsigned char *stream, __global  unsigned char **pt_pr_stream, __global  unsigned char **pt_pr_stream_end)
{
   while (!(*stream == 0xff  && *(stream+1) == 0xd8 ) ) //searching for the start of image marker
   {
      if(stream<=*pt_pr_stream_end)
        {
           stream++;
           continue;
         }
      else
         return 0;  // No more images in the file.
   }
   *(pt_pr_stream)=stream+2;
   return 1;
}

int parse_JFIF(__global struct jdec_private_to_cl *priv, __global unsigned char *stream, __global  unsigned char **pt_pr_stream, __global  unsigned char **pt_pr_stream_begin, __global  unsigned char **pt_pr_stream_end, __global  unsigned char **pt_pr_stream_scan)
{
  int chuck_len;
  int marker;
  int sos_marker_found = 0;
  int dht_marker_found = 0;
  int dqt_marker_found = 0;
  int sof_marker_found = 0;
  __global  unsigned char *next_chunck;

  int next_image_found = findSOI(priv,stream, pt_pr_stream, pt_pr_stream_end);
  stream = *pt_pr_stream;
    while (!sos_marker_found  && stream<=*(pt_pr_stream_end))
   {
     while((*stream == 0xff))
        stream++;    
            	    
     marker = *stream++;
     chuck_len = be16_to_cpu(stream);
     next_chunck = stream + chuck_len;
     switch (marker)
      {
       case SOF:
	 if (parse_SOF(priv, stream) < 0)
	   return -1;
  	 sof_marker_found = 1;
	 break;
       case DQT:
	 if (parse_DQT(priv, stream) < 0)
	   return -1;
	 dqt_marker_found = 1;
	 break;
       case SOS:
	 if (parse_SOS(priv, stream, pt_pr_stream) < 0)
	   return -1;
	 sos_marker_found = 1;
	 break;
       case DHT:
	 if (parse_DHT(priv, stream) < 0)
	   return -1;
	 dht_marker_found = 1;
	 break;
       case DRI:
	 if (parse_DRI(priv, stream) < 0)
	   return -1;
	 break;
       default:
	 break;
      }

     stream = next_chunck;

   }


   if(next_image_found){
      if (!dht_marker_found) {
        build_default_huffman_tables(priv);
      }
      if (!dqt_marker_found) {
	return -1;
      }
   } 

#ifdef SANITY_CHECK
  if (   (priv->component_infos[cY].Hfactor < priv->component_infos[cCb].Hfactor)
      || (priv->component_infos[cY].Hfactor < priv->component_infos[cCr].Hfactor)){
	return -1;
  }
  if (   (priv->component_infos[cY].Vfactor < priv->component_infos[cCb].Vfactor)
      || (priv->component_infos[cY].Vfactor < priv->component_infos[cCr].Vfactor)){
	return -1;
  }
  if (   (priv->component_infos[cCb].Hfactor!=1) 
      || (priv->component_infos[cCr].Hfactor!=1)
      || (priv->component_infos[cCb].Vfactor!=1)
      || (priv->component_infos[cCr].Vfactor!=1)){
	return -1;
  }
#endif
  priv->stream = stream;
  priv->pr_stream = *pt_pr_stream;
  return next_image_found;
}

void tinyjpeg_parse_header(__global struct jdec_private_to_cl *priv, __global unsigned char *buf, unsigned int size, __global int *p_re)
{
  int ret;

  /* Identify the file */
  if ((buf[0] != 0xFF) || (buf[1] != SOI)){
	*p_re = -1;
	return;
  }
  
  __global unsigned char *pr_stream_begin, *pr_stream_end,*pr_stream_scan;
  __global unsigned char *pr_stream;	/* Pointer to the current stream */

  pr_stream_begin = buf;
  priv->stream_length = size;
  pr_stream_end = pr_stream_begin + priv->stream_length;

  pr_stream = pr_stream_begin;
  priv->pr_stream_begin = pr_stream_begin;
  priv->pr_stream_end = pr_stream_end;
  ret = parse_JFIF(priv, pr_stream, &pr_stream, &pr_stream_begin, &pr_stream_end, &pr_stream_scan);
  *p_re = ret;
  return; 
}

__kernel
void parse_mjpeg_header(__global uchar *stream,
		      __global struct jdec_private_to_cl* priv,
		      int image_size,
		      __global int *result
			  )
{
  tinyjpeg_parse_header(priv, stream, image_size, result);
}

__kernel
void findEOI(__global struct jdec_private_to_cl *priv)
{
  #define SLM_SIZE (64*1020)
  __local unsigned char lsm[SLM_SIZE];
  __local unsigned int *li;
  __local unsigned char *lc;
  __global unsigned int *gi;
  int begin;
  begin = priv->stream;
  int size = priv->pr_stream_end - begin;
  __global  unsigned char* out = 0;
  __local volatile int flag;
  __local volatile int address;
  flag = 0;
  priv->scan_to_stream = priv->pr_stream_end+1;
  int lid = get_local_id(0);
  int lsize = SLM_SIZE / get_local_size(0);
  int i, j;
  li = (__local unsigned int *)(lsm + lid*lsize);
  for (i = 0; i < ((size + SLM_SIZE - 1)/SLM_SIZE); i++) {
    gi = (__global unsigned int *)(begin + lid*lsize + i*SLM_SIZE);
    for (j = 0; j < lsize/4 ; j++)
      li[j] = gi[j];
    lc = lsm + lid*lsize;
    for (j = 0; j < lsize; j++, lc++) {
      if (*lc == 0xff && *(lc + 1) == 0xd9) {
        address = (int)(begin+ i*SLM_SIZE+lid*lsize+j);
        priv->scan_to_stream = address-priv->pr_stream;
        priv->stream_to_begin = priv->pr_stream - priv->pr_stream_begin;
        flag = 1;
        break;
      }
    }
    if(flag)
      return;
  }
}
