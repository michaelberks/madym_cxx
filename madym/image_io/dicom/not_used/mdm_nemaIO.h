/**
 *  @file    mdm_nemaIO.h
 *  @brief   ACR-NEMA file io functions - header
 *
 *  Original author (of this header only) GA Buonaccorsi 1 Sep 2004
 *  (c) Copyright ISBE, University of Manchester 2004
 *
 *  Last edited GAB 1 Sep 2004
 *  GAB mods:
 *  1 Sepy 2004
 *  - Created
 *
 */

#ifndef MDM_NEMAIO_HDR
#define MDM_NEMAIO_HDR

#ifndef  MDM_TINASYS_HDR
#include <tina/sys.h>      /* For Tina sys types */
#define  MDM_TINASYS_HDR
#endif

#ifndef  MDM_TINAIMAGE_HDR
#include <tina/image.h>      /* For Imrect */
#define  MDM_TINAIMAGE_HDR
#endif

void    im_endian_inplace(Imrect *imrect);
Bool    nema_hdr_TR_extract(FILE *fp, Imrect *im);
Bool    nema_hdr_flip_angle_extract(FILE *fp, Imrect *im);
Bool    nema_hdr_imagetime_R8_extract(FILE *fp, Imrect *im);
Bool    nema_hdr_voxelscale_extract(FILE *fp, Imrect *im);
Imrect *nema_read_image(const char *pathname, int file_type);

#endif /* MDM_NEMAIO_HDR */
