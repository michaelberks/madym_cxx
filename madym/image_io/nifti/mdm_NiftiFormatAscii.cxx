/**
 *  @file    mdm_NiftiFormatAscii.cxx
 *  @brief   Implementation of class for Analyze image format reading and writing
 *  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
 */

#include "mdm_NiftiFormat.h"
#include <madym/mdm_platform_defs.h>

DISABLE_WARNING_PUSH
DISABLE_WARNING_DEPRECATED
/*----------------------------------------------------------------------*/
/*! duplicate the given string (alloc length+1)
 *
 * \return allocated pointer (or NULL on failure)
*//*--------------------------------------------------------------------*/
char *nifti_strdup(const char *str)
{
  char *dup;

  if (!str) return NULL;       /* allow calls passing NULL */

  dup = (char *)malloc(strlen(str) + 1);

  /* check for failure */
  if (dup) strcpy(dup, str);
  else      fprintf(stderr, "** nifti_strdup: failed to alloc  bytes\n");

  return dup;
}

/*------------------------------------------------------------------------*/
/* Un-escape a C string in place -- that is, convert XML escape sequences
   back into their characters.  (This can be done in place since the
   replacement is always smaller than the input.)  Escapes recognized are:
     -  &lt;   ->  <
     -  &gt;   ->  >
     -  &quot; ->  "
     -  &apos; ->  '
     -  &amp;  ->  &
   Also replace CR LF pair (Microsoft), or CR alone (Macintosh) with
   LF (Unix), per the XML standard.
   Return value is number of replacements made (if you care).
--------------------------------------------------------------------------*/

#undef  CR
#undef  LF
#define CR 0x0D
#define LF 0x0A

int unescape_string(char *str)
{
  int ii, jj, nn, ll;

  if (str == NULL) return 0;                /* no string? */
  ll = (int)strlen(str); if (ll == 0) return 0;

  /* scan for escapes: &something; */

  for (ii = jj = nn = 0; ii < ll; ii++, jj++) { /* scan at ii; results go in at jj */

    if (str[ii] == '&') {  /* start of escape? */

      if (ii + 3 < ll        &&   /* &lt; */
        str[ii + 1] == 'l' &&
        str[ii + 2] == 't' &&
        str[ii + 3] == ';') {
        str[jj] = '<'; ii += 3; nn++;
      }

      else if (ii + 3 < ll        &&   /* &gt; */
        str[ii + 1] == 'g' &&
        str[ii + 2] == 't' &&
        str[ii + 3] == ';') {
        str[jj] = '>'; ii += 3; nn++;
      }

      else if (ii + 5 < ll        &&   /* &quot; */
        str[ii + 1] == 'q' &&
        str[ii + 2] == 'u' &&
        str[ii + 3] == 'o' &&
        str[ii + 4] == 't' &&
        str[ii + 5] == ';') {
        str[jj] = '"'; ii += 5; nn++;
      }

      else if (ii + 5 < ll        &&   /* &apos; */
        str[ii + 1] == 'a' &&
        str[ii + 2] == 'p' &&
        str[ii + 3] == 'o' &&
        str[ii + 4] == 's' &&
        str[ii + 5] == ';') {
        str[jj] = '\''; ii += 5; nn++;
      }

      else if (ii + 4 < ll        &&  /* &amp; */
        str[ii + 1] == 'a' &&
        str[ii + 2] == 'm' &&
        str[ii + 3] == 'p' &&
        str[ii + 4] == ';') {
        str[jj] = '&'; ii += 4; nn++;
      }

      /* although the comments above don't mention it,
         we also look for XML style numeric escapes
         of the forms &#32; (decimal) and &#xfd; (hex) */

      else if (ii + 3 < ll        &&
        str[ii + 1] == '#' &&
        isdigit((int)str[ii + 2])) {   /* &#dec; */

        unsigned int val = '?'; int kk = ii + 3;
        while (kk < ll && kk != ';') kk++;
        sscanf(str + ii + 2, "%u", &val);
        str[jj] = (char)val; ii = kk; nn++;
      }

      else if (ii + 4 < ll        &&
        str[ii + 1] == '#' &&
        str[ii + 2] == 'x' &&
        isxdigit((int)str[ii + 3])) {   /* &#hex; */

        unsigned int val = '?'; int kk = ii + 4;
        while (kk < ll && kk != ';') kk++;
        sscanf(str + ii + 3, "%x", &val);
        str[jj] = (char)val; ii = kk; nn++;
      }

      /* didn't start a recognized escape, so just copy as normal */

      else if (jj < ii) { str[jj] = str[ii]; }

    }
    else if (str[ii] == CR) {  /* is a carriage return */

      if (str[ii + 1] == LF) { str[jj] = LF; ii++; nn++; }  /* CR LF */
      else { str[jj] = LF; ; nn++; }  /* CR only */

    }
    else { /* is a normal character, just copy to output */

      if (jj < ii) { str[jj] = str[ii]; }
    }

    /* at this point, ii=index of last character used up in scan
                      jj=index of last character written to (jj <= ii) */
  }

  if (jj < ll) str[jj] = '\0'; /* end string properly */

  return nn;
}
DISABLE_WARNING_POP

/*------------------------------------------------------------------------*/
/* Quotize (and escapize) one string, returning a new string.
   Approximately speaking, this is the inverse of unescape_string().
   The result should be free()-ed when you are done with it.
--------------------------------------------------------------------------*/

char *escapize_string(const char * str)
{
  int ii, jj, lstr, lout;
  char *out;

  if (str == NULL || (lstr = (int)strlen(str)) == 0) {      /* 0 length */
    out = nifti_strdup("''"); return out;                /* string?? */
  }

  lout = 4;                      /* initialize length of output */
  for (ii = 0; ii < lstr; ii++) { /* count characters for output */
    switch (str[ii]) {
    case '&':  lout += 5; break;  /* replace '&' with "&amp;" */

    case '<':
    case '>':  lout += 4; break;  /* replace '<' with "&lt;" */

    case '"':
    case '\'': lout += 6; break;  /* replace '"' with "&quot;" */

    case CR:
    case LF:   lout += 6; break;  /* replace CR with "&#x0d;"
                                               LF with "&#x0a;" */

    default: lout++; break;      /* copy all other chars */
    }
  }
  out = (char *)calloc(1, lout);     /* allocate output string */
  if (!out) {
    fprintf(stderr, "** NIFTI escapize_string: failed to alloc %d bytes\n",
      lout);
    return NULL;
  }
  out[0] = '\'';                    /* opening quote mark */
  for (ii = 0, jj = 1; ii < lstr; ii++) {
    switch (str[ii]) {
    default: out[jj++] = str[ii]; break;  /* normal characters */

    case '&':  memcpy(out + jj, "&amp;", 5); jj += 5; break;

    case '<':  memcpy(out + jj, "&lt;", 4); jj += 4; break;
    case '>':  memcpy(out + jj, "&gt;", 4); jj += 4; break;

    case '"': memcpy(out + jj, "&quot;", 6); jj += 6; break;

    case '\'': memcpy(out + jj, "&apos;", 6); jj += 6; break;

    case CR:   memcpy(out + jj, "&#x0d;", 6); jj += 6; break;
    case LF:   memcpy(out + jj, "&#x0a;", 6); jj += 6; break;
    }
  }
  out[jj++] = '\'';  /* closing quote mark */
  out[jj] = '\0';  /* terminate the string */
  return out;
}

//**********************************************************************
//Private 
//**********************************************************************
//
//

/*----------------------------------------------------------------------
 * has_ascii_header  - see if the NIFTI header is an ASCII format
 *
 * If the file starts with the ASCII string "<nifti_image", then
 * process the dataset as a type-3 .nia file.
 *
 * return:  -1 on error, 1 if true, or 0 if false
 *
 * NOTE: this is NOT part of the NIFTI-1 standard
 *----------------------------------------------------------------------*/
int mdm_NiftiFormat::has_ascii_header(znzFile fp)
{
  char  buf[16];
  int   nread;

  if (znz_isnull(fp)) return 0;

  nread = (int)znzread(buf, 1, 12, fp);
  buf[12] = '\0';

  if (nread < 12) return -1;

  znzrewind(fp);  /* move back to the beginning, and check */

  if (strcmp(buf, "<nifti_image") == 0) return 1;

  return 0;
}


/*----------------------------------------------------------------------*/
/*! nifti_read_ascii_image  - process as a type-3 .nia image file

   return NULL on failure

   NOTE: this is NOT part of the NIFTI-1 standard
*//*--------------------------------------------------------------------*/
mdm_NiftiFormat::nifti_image mdm_NiftiFormat::nifti_read_ascii_image(znzFile fp, const std::string &fname, int flen,
  int read_data)
{
  nifti_image nim;
  int           slen, txt_size, remain, rv = 0;
  char        * sbuf;

  if (nifti_is_gzfile(fname))
    throw mdm_exception(__func__, "compression not supported for file type NIFTI_FTYPE::ASCII");

  
  slen = flen;  /* slen will be our buffer length */
  if (slen <= 0) 
    slen = (int)nifti_get_filesize(fname);

  /*if (g_opts.debug > 1)
    fprintf(stderr, "-d %s: have ASCII NIFTI file of size %d\n", fname, slen);*/

  if (slen > 65530) slen = 65530;
  sbuf = (char *)calloc(sizeof(char), slen + 1);
  if (!sbuf)
    throw mdm_exception(__func__, "failed to alloc " + std::to_string(65530) + " bytes for sbuf");
    
  znzread(sbuf, 1, slen, fp);
  nim = nifti_image_from_ascii(sbuf, &txt_size); free(sbuf);
  
  nim.nifti_type = NIFTI_FTYPE::ASCII;

  /* compute remaining space for extensions */
  remain = flen - txt_size - (int)nifti_get_volsize(nim);
  if (remain > 4) {
    /* read extensions (reposition file pointer, first) */
    znzseek(fp, txt_size, SEEK_SET);
    (void)nifti_read_extensions(nim, fp, (int64_t)remain);
  }

  nim.iname_offset = -1;  /* check from the end of the file */

  if (read_data) 
    rv = nifti_image_load(nim);
  else            
    nim.data = NULL;

  /* check for nifti_image_load() failure, maybe bail out */
  if (read_data && rv != 0)
    throw mdm_exception(__func__, "failed image_load, free nifti image struct");

  return nim;
}

/*----------------------------------------------------------------------*/
/*! write a nifti_image to disk in ASCII format
*//*--------------------------------------------------------------------*/
znzFile mdm_NiftiFormat::nifti_write_ascii_image(nifti_image &nim,
  const char *opts, int write_data, int leave_open)
{
  znzFile   fp;
  char    * hstr;

  hstr = nifti_image_to_ascii(nim);  /* get header in ASCII form */
  if (!hstr) { fprintf(stderr, "** failed image_to_ascii()\n"); return NULL; }

  fp = znzopen(nim.fname.c_str(), opts, nifti_is_gzfile(nim.fname));
  if (znz_isnull(fp)) {
    free(hstr);
    throw mdm_exception(__func__, nim.fname + ":failed to open for ascii write");
  }

  znzputs(hstr, fp);                                               /* header */
  nifti_write_extensions(fp, nim);                             /* extensions */

  if (write_data) { nifti_write_all_data(fp, nim); }         /* data */
  if (!leave_open) { znzclose(fp); }
  free(hstr);
  return fp;  /* returned but may be closed */
}

/*---------------------------------------------------------------------------*/
#undef  QQNUM
#undef  QNUM
#undef  QSTR

/* macro to check lhs string against "n1"; if it matches,
   interpret rhs string as a number, and put it into nim."n2" */

#define QQNUM(n1,n2,tt) if( strcmp(lhs,#n1)==0 ) nim.n2=(tt)strtod(rhs,NULL)

   /* same, but where "n1" == "n2" */

#define QNUM(nam,tt)    QQNUM(nam,nam,tt)

/* macro to check lhs string against "nam"; if it matches,
   put rhs string into nim."nam" string, with max length = "ml" */

#define QSTR(nam,ml) if( strcmp(lhs,#nam) == 0 )                           \
                       strncpy(nim.nam,rhs,ml), nim.nam[ml]='\0'

/*---------------------------------------------------------------------------*/
/*! Take an XML-ish ASCII string and create a NIFTI image header to match.

    NULL is returned if enough information isn't present in the input string.
    - The image data can later be loaded with nifti_image_load().
    - The struct returned here can be liberated with nifti_image_free().
    - Not a lot of error checking is done here to make sure that the
      input values are reasonable!
*//*-------------------------------------------------------------------------*/
DISABLE_WARNING_PUSH
DISABLE_WARNING_DEPRECATED
mdm_NiftiFormat::nifti_image mdm_NiftiFormat::nifti_image_from_ascii(const char *str, int * bytes_read)
{
  char lhs[1024], rhs[1024];
  int ii, spos, nn;
  nifti_image nim;              /* will be output */

  if (str == NULL || *str == '\0')
    throw mdm_exception(__func__, "Input string is empty");  /* bad input!? */

  /* scan for opening string */
  spos = 0;
  ii = sscanf(str + spos, "%1023s%n", lhs, &nn); spos += nn;
  if (ii == 0 || strcmp(lhs, "<nifti_image") != 0) 
    return nim;

  /* create empty image struct */
  nim.nx = nim.ny = nim.nz = nim.nt
    = nim.nu = nim.nv = nim.nw = 1;
  nim.dx = nim.dy = nim.dz = nim.dt
    = nim.du = nim.dv = nim.dw = 0;
  nim.qfac = 1.0f;

  nim.byteorder = nifti_short_order();

  /* starting at str[spos], scan for "equations" of the form
        lhs = 'rhs'
     and assign rhs values into the struct component named by lhs */

  while (1) {

    while (isspace((int)str[spos])) spos++;  /* skip whitespace */
    if (str[spos] == '\0') break;       /* end of string? */

    /* get lhs string */

    ii = sscanf(str + spos, "%1023s%n", lhs, &nn); spos += nn;
    if (ii == 0 || strcmp(lhs, "/>") == 0) break;  /* end of input? */

    /* skip whitespace and the '=' marker */

    while (isspace((int)str[spos]) || str[spos] == '=') spos++;
    if (str[spos] == '\0') break;       /* end of string? */

    /* if next character is a quote ', copy everything up to next '
       otherwise, copy everything up to next nonblank              */

    if (str[spos] == '\'') {
      ii = spos + 1;
      while (str[ii] != '\0' && str[ii] != '\'') ii++;
      nn = ii - spos - 1; if (nn > 1023) nn = 1023;
      memcpy(rhs, str + spos + 1, nn); rhs[nn] = '\0';
      spos = (str[ii] == '\'') ? ii + 1 : ii;
    }
    else {
      ii = sscanf(str + spos, "%1023s%n", rhs, &nn); spos += nn;
      if (ii == 0) break;  /* nothing found? */
    }
    unescape_string(rhs);  /* remove any XML escape sequences */

    /* Now can do the assignment, based on lhs string.
       Start with special cases that don't fit the QNUM/QSTR macros. */

    if (strcmp(lhs, "nifti_type") == 0) {
      if (strcmp(rhs, "ANALYZE-7.5") == 0)
        nim.nifti_type = NIFTI_FTYPE::ANALYZE;
      else if (strcmp(rhs, "NIFTI-1+") == 0)
        nim.nifti_type = NIFTI_FTYPE::NIFTI1_1;
      else if (strcmp(rhs, "NIFTI-1") == 0)
        nim.nifti_type = NIFTI_FTYPE::NIFTI1_2;
      else if (strcmp(rhs, "NIFTI-1A") == 0)
        nim.nifti_type = NIFTI_FTYPE::ASCII;
    }
    else if (strcmp(lhs, "header_filename") == 0) {
      nim.fname = nifti_strdup(rhs);
    }
    else if (strcmp(lhs, "image_filename") == 0) {
      nim.iname = nifti_strdup(rhs);
    }
    else if (strcmp(lhs, "sto_xyz_matrix") == 0) {
      sscanf(rhs, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
        &(nim.sto_xyz.m[0][0]), &(nim.sto_xyz.m[0][1]),
        &(nim.sto_xyz.m[0][2]), &(nim.sto_xyz.m[0][3]),
        &(nim.sto_xyz.m[1][0]), &(nim.sto_xyz.m[1][1]),
        &(nim.sto_xyz.m[1][2]), &(nim.sto_xyz.m[1][3]),
        &(nim.sto_xyz.m[2][0]), &(nim.sto_xyz.m[2][1]),
        &(nim.sto_xyz.m[2][2]), &(nim.sto_xyz.m[2][3]),
        &(nim.sto_xyz.m[3][0]), &(nim.sto_xyz.m[3][1]),
        &(nim.sto_xyz.m[3][2]), &(nim.sto_xyz.m[3][3]));
    }
    else if (strcmp(lhs, "byteorder") == 0) {
      if (strcmp(rhs, "MSB_FIRST") == 0) nim.byteorder = MSB_FIRST;
      if (strcmp(rhs, "LSB_FIRST") == 0) nim.byteorder = LSB_FIRST;
    }
    else QQNUM(image_offset, iname_offset, int);
    else QNUM(datatype, short int);
    else QNUM(ndim, int);
    else QNUM(nx, int);
  else QNUM(ny, int);
  else QNUM(nz, int);
  else QNUM(nt, int);
     else QNUM(nu, int);
     else QNUM(nv, int);
     else QNUM(nw, int);
     else QNUM(dx, float);
     else QNUM(dy, float);
     else QNUM(dz, float);
     else QNUM(dt, float);
     else QNUM(du, float);
     else QNUM(dv, float);
     else QNUM(dw, float);
     else QNUM(cal_min, float);
     else QNUM(cal_max, float);
     else QNUM(scl_slope, float);
     else QNUM(scl_inter, float);
     else QNUM(intent_code, short);
     else QNUM(intent_p1, float);
     else QNUM(intent_p2, float);
     else QNUM(intent_p3, float);
     else QSTR(intent_name, 15);
     else QNUM(toffset, float);
     else QNUM(xyz_units, int);
     else QNUM(time_units, int);
     else QSTR(descrip, 79);
     else QSTR(aux_file, 23);
     else QNUM(qform_code, int);
     else QNUM(quatern_b, float);
     else QNUM(quatern_c, float);
     else QNUM(quatern_d, float);
     else QNUM(qoffset_x, float);
     else QNUM(qoffset_y, float);
     else QNUM(qoffset_z, float);
     else QNUM(qfac, float);
     else QNUM(sform_code, int);
     else QNUM(freq_dim, int);
     else QNUM(phase_dim, int);
     else QNUM(slice_dim, int);
     else QNUM(slice_code, int);
     else QNUM(slice_start, int);
     else QNUM(slice_end, int);
     else QNUM(slice_duration, float);
     else QNUM(num_ext, int);

  } /* end of while loop */

  if (bytes_read) *bytes_read = spos + 1;         /* "process" last '\n' */

  /* do miscellaneous checking and cleanup */

  if (nim.ndim <= 0)
    throw mdm_exception(__func__, "Bad dimesnions read " + std::to_string(nim.ndim));/* bad! */

  nifti_datatype_sizes(nim.datatype, nim.nbyper, nim.swapsize);
  if (nim.nbyper == 0) 
    throw mdm_exception(__func__, "Bytes per pixel read as 0");/* bad! */

  nim.dim[0] = nim.ndim;
  nim.dim[1] = nim.nx; nim.pixdim[1] = nim.dx;
  nim.dim[2] = nim.ny; nim.pixdim[2] = nim.dy;
  nim.dim[3] = nim.nz; nim.pixdim[3] = nim.dz;
  nim.dim[4] = nim.nt; nim.pixdim[4] = nim.dt;
  nim.dim[5] = nim.nu; nim.pixdim[5] = nim.du;
  nim.dim[6] = nim.nv; nim.pixdim[6] = nim.dv;
  nim.dim[7] = nim.nw; nim.pixdim[7] = nim.dw;

  nim.nvox = (int64_t)nim.nx * nim.ny * nim.nz
    * nim.nt * nim.nu * nim.nv * nim.nw;

  if (nim.qform_code > 0)
    nim.qto_xyz = nifti_quatern_to_dmat44(
      nim.quatern_b, nim.quatern_c, nim.quatern_d,
      nim.qoffset_x, nim.qoffset_y, nim.qoffset_z,
      nim.dx, nim.dy, nim.dz,
      nim.qfac);
  else
    nim.qto_xyz = nifti_quatern_to_dmat44(
      0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
      nim.dx, nim.dy, nim.dz, 0.0);


  nim.qto_ijk = nifti_dmat44_inverse(nim.qto_xyz);

  if (nim.sform_code > 0)
    nim.sto_ijk = nifti_dmat44_inverse(nim.sto_xyz);

  return nim;
}
DISABLE_WARNING_POP

/*---------------------------------------------------------------------------*/
/*! Dump the information in a NIFTI image header to an XML-ish ASCII string
   that can later be converted back into a NIFTI header in
   nifti_image_from_ascii().

   The resulting string can be free()-ed when you are done with it.
*//*-------------------------------------------------------------------------*/
DISABLE_WARNING_PUSH
DISABLE_WARNING_DEPRECATED
char *mdm_NiftiFormat::nifti_image_to_ascii(const nifti_image &nim)
{
  char *buf, *ebuf; int nbuf;

  buf = (char *)calloc(1, 65534); /* longer than needed, to be safe */
  if (!buf) {
    fprintf(stderr, "** NIFTI NITA: failed to alloc %d bytes\n", 65534);
    return NULL;
  }

  sprintf(buf, "<nifti_image\n");   /* XML-ish opener */

  sprintf(buf + strlen(buf), "  nifti_type = '%s'\n",
    (nim.nifti_type == NIFTI_FTYPE::NIFTI1_1) ? "NIFTI-1+"
    : (nim.nifti_type == NIFTI_FTYPE::NIFTI1_2) ? "NIFTI-1"
    : (nim.nifti_type == NIFTI_FTYPE::ASCII) ? "NIFTI-1A"
    : "ANALYZE-7.5");

  /** Strings that we don't control (filenames, etc.) that might
      contain "weird" characters (like quotes) are "escaped":
      - A few special characters are replaced by XML-style escapes, using
        the function escapize_string().
      - On input, function unescape_string() reverses this process.
      - The result is that the NIFTI ASCII-format header is XML-compliant. */

  ebuf = escapize_string(nim.fname.c_str());
  sprintf(buf + strlen(buf), "  header_filename = %s\n", ebuf); free(ebuf);

  ebuf = escapize_string(nim.iname.c_str());
  sprintf(buf + strlen(buf), "  image_filename = %s\n", ebuf); free(ebuf);

  sprintf(buf + strlen(buf), "  image_offset = ' '\n");

  sprintf(buf + strlen(buf), "  ndim = ' '\n");
  sprintf(buf + strlen(buf), "  nx = ' '\n");
  if (nim.ndim > 1)
    sprintf(buf + strlen(buf), "  ny = ' '\n");
  if (nim.ndim > 2)
    sprintf(buf + strlen(buf), "  nz = ' '\n");
  if (nim.ndim > 3)
    sprintf(buf + strlen(buf), "  nt = ' '\n");
  if (nim.ndim > 4)
    sprintf(buf + strlen(buf), "  nu = ' '\n");
  if (nim.ndim > 5)
    sprintf(buf + strlen(buf), "  nv = ' '\n");
  if (nim.ndim > 6)
    sprintf(buf + strlen(buf), "  nw = ' '\n");

  sprintf(buf + strlen(buf), "  dx = '%g'\n", nim.dx);
  if (nim.ndim > 1) sprintf(buf + strlen(buf), "  dy = '%g'\n", nim.dy);
  if (nim.ndim > 2) sprintf(buf + strlen(buf), "  dz = '%g'\n", nim.dz);
  if (nim.ndim > 3) sprintf(buf + strlen(buf), "  dt = '%g'\n", nim.dt);
  if (nim.ndim > 4) sprintf(buf + strlen(buf), "  du = '%g'\n", nim.du);
  if (nim.ndim > 5) sprintf(buf + strlen(buf), "  dv = '%g'\n", nim.dv);
  if (nim.ndim > 6) sprintf(buf + strlen(buf), "  dw = '%g'\n", nim.dw);

  sprintf(buf + strlen(buf), "  datatype = '%d'\n", nim.datatype);
  sprintf(buf + strlen(buf), "  datatype_name = '%s'\n",
    nifti_datatype_string(nim.datatype));

  sprintf(buf + strlen(buf), "  nvox = ' '\n");
  sprintf(buf + strlen(buf), "  nbyper = '%d'\n", nim.nbyper);

  sprintf(buf + strlen(buf), "  byteorder = '%s'\n",
    (nim.byteorder == MSB_FIRST) ? "MSB_FIRST" : "LSB_FIRST");

  if (nim.cal_min < nim.cal_max) {
    sprintf(buf + strlen(buf), "  cal_min = '%g'\n", nim.cal_min);
    sprintf(buf + strlen(buf), "  cal_max = '%g'\n", nim.cal_max);
  }

  if (nim.scl_slope != 0.0) {
    sprintf(buf + strlen(buf), "  scl_slope = '%g'\n", nim.scl_slope);
    sprintf(buf + strlen(buf), "  scl_inter = '%g'\n", nim.scl_inter);
  }

  if (nim.intent_code > 0) {
    sprintf(buf + strlen(buf), "  intent_code = '%d'\n", nim.intent_code);
    sprintf(buf + strlen(buf), "  intent_code_name = '%s'\n",
      nifti_intent_string(nim.intent_code));
    sprintf(buf + strlen(buf), "  intent_p1 = '%g'\n", nim.intent_p1);
    sprintf(buf + strlen(buf), "  intent_p2 = '%g'\n", nim.intent_p2);
    sprintf(buf + strlen(buf), "  intent_p3 = '%g'\n", nim.intent_p3);

    if (nim.intent_name[0] != '\0') {
      ebuf = escapize_string(nim.intent_name);
      sprintf(buf + strlen(buf), "  intent_name = %s\n", ebuf);
      free(ebuf);
    }
  }

  if (nim.toffset != 0.0)
    sprintf(buf + strlen(buf), "  toffset = '%g'\n", nim.toffset);

  if (nim.xyz_units > 0)
    sprintf(buf + strlen(buf),
      "  xyz_units = '%d'\n"
      "  xyz_units_name = '%s'\n",
      nim.xyz_units, nifti_units_string(nim.xyz_units));

  if (nim.time_units > 0)
    sprintf(buf + strlen(buf),
      "  time_units = '%d'\n"
      "  time_units_name = '%s'\n",
      nim.time_units, nifti_units_string(nim.time_units));

  if (nim.freq_dim > 0)
    sprintf(buf + strlen(buf), "  freq_dim = '%d'\n", nim.freq_dim);
  if (nim.phase_dim > 0)
    sprintf(buf + strlen(buf), "  phase_dim = '%d'\n", nim.phase_dim);
  if (nim.slice_dim > 0)
    sprintf(buf + strlen(buf), "  slice_dim = '%d'\n", nim.slice_dim);
  if (nim.slice_code > 0)
    sprintf(buf + strlen(buf),
      "  slice_code = '%d'\n"
      "  slice_code_name = '%s'\n",
      nim.slice_code, nifti_slice_string(nim.slice_code));
  if (nim.slice_start >= 0 && nim.slice_end > nim.slice_start)
    sprintf(buf + strlen(buf),
      "  slice_start = ' '\n"
      "  slice_end = ' '\n");
  if (nim.slice_duration != 0.0)
    sprintf(buf + strlen(buf), "  slice_duration = '%g'\n",
      nim.slice_duration);

  if (nim.descrip[0] != '\0') {
    ebuf = escapize_string(nim.descrip);
    sprintf(buf + strlen(buf), "  descrip = %s\n", ebuf);
    free(ebuf);
  }

  if (nim.aux_file[0] != '\0') {
    ebuf = escapize_string(nim.aux_file);
    sprintf(buf + strlen(buf), "  aux_file = %s\n", ebuf);
    free(ebuf);
  }

  if (nim.qform_code > 0) {
    int i, j, k;

    sprintf(buf + strlen(buf),
      "  qform_code = '%d'\n"
      "  qform_code_name = '%s'\n"
      "  qto_xyz_matrix = '%g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g'\n",
      nim.qform_code, nifti_xform_string(nim.qform_code),
      nim.qto_xyz.m[0][0], nim.qto_xyz.m[0][1],
      nim.qto_xyz.m[0][2], nim.qto_xyz.m[0][3],
      nim.qto_xyz.m[1][0], nim.qto_xyz.m[1][1],
      nim.qto_xyz.m[1][2], nim.qto_xyz.m[1][3],
      nim.qto_xyz.m[2][0], nim.qto_xyz.m[2][1],
      nim.qto_xyz.m[2][2], nim.qto_xyz.m[2][3],
      nim.qto_xyz.m[3][0], nim.qto_xyz.m[3][1],
      nim.qto_xyz.m[3][2], nim.qto_xyz.m[3][3]);

    sprintf(buf + strlen(buf),
      "  qto_ijk_matrix = '%g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g'\n",
      nim.qto_ijk.m[0][0], nim.qto_ijk.m[0][1],
      nim.qto_ijk.m[0][2], nim.qto_ijk.m[0][3],
      nim.qto_ijk.m[1][0], nim.qto_ijk.m[1][1],
      nim.qto_ijk.m[1][2], nim.qto_ijk.m[1][3],
      nim.qto_ijk.m[2][0], nim.qto_ijk.m[2][1],
      nim.qto_ijk.m[2][2], nim.qto_ijk.m[2][3],
      nim.qto_ijk.m[3][0], nim.qto_ijk.m[3][1],
      nim.qto_ijk.m[3][2], nim.qto_ijk.m[3][3]);

    sprintf(buf + strlen(buf),
      "  quatern_b = '%g'\n"
      "  quatern_c = '%g'\n"
      "  quatern_d = '%g'\n"
      "  qoffset_x = '%g'\n"
      "  qoffset_y = '%g'\n"
      "  qoffset_z = '%g'\n"
      "  qfac = '%g'\n",
      nim.quatern_b, nim.quatern_c, nim.quatern_d,
      nim.qoffset_x, nim.qoffset_y, nim.qoffset_z, nim.qfac);

    nifti_dmat44_to_orientation(nim.qto_xyz, &i, &j, &k);
    if (i > 0 && j > 0 && k > 0)
      sprintf(buf + strlen(buf),
        "  qform_i_orientation = '%s'\n"
        "  qform_j_orientation = '%s'\n"
        "  qform_k_orientation = '%s'\n",
        nifti_orientation_string(i),
        nifti_orientation_string(j),
        nifti_orientation_string(k));
  }

  if (nim.sform_code > 0) {
    int i, j, k;

    sprintf(buf + strlen(buf),
      "  sform_code = '%d'\n"
      "  sform_code_name = '%s'\n"
      "  sto_xyz_matrix = '%g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g'\n",
      nim.sform_code, nifti_xform_string(nim.sform_code),
      nim.sto_xyz.m[0][0], nim.sto_xyz.m[0][1],
      nim.sto_xyz.m[0][2], nim.sto_xyz.m[0][3],
      nim.sto_xyz.m[1][0], nim.sto_xyz.m[1][1],
      nim.sto_xyz.m[1][2], nim.sto_xyz.m[1][3],
      nim.sto_xyz.m[2][0], nim.sto_xyz.m[2][1],
      nim.sto_xyz.m[2][2], nim.sto_xyz.m[2][3],
      nim.sto_xyz.m[3][0], nim.sto_xyz.m[3][1],
      nim.sto_xyz.m[3][2], nim.sto_xyz.m[3][3]);

    sprintf(buf + strlen(buf),
      "  sto_ijk matrix = '%g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g'\n",
      nim.sto_ijk.m[0][0], nim.sto_ijk.m[0][1],
      nim.sto_ijk.m[0][2], nim.sto_ijk.m[0][3],
      nim.sto_ijk.m[1][0], nim.sto_ijk.m[1][1],
      nim.sto_ijk.m[1][2], nim.sto_ijk.m[1][3],
      nim.sto_ijk.m[2][0], nim.sto_ijk.m[2][1],
      nim.sto_ijk.m[2][2], nim.sto_ijk.m[2][3],
      nim.sto_ijk.m[3][0], nim.sto_ijk.m[3][1],
      nim.sto_ijk.m[3][2], nim.sto_ijk.m[3][3]);

    nifti_dmat44_to_orientation(nim.sto_xyz, &i, &j, &k);
    if (i > 0 && j > 0 && k > 0)
      sprintf(buf + strlen(buf),
        "  sform_i_orientation = '%s'\n"
        "  sform_j_orientation = '%s'\n"
        "  sform_k_orientation = '%s'\n",
        nifti_orientation_string(i),
        nifti_orientation_string(j),
        nifti_orientation_string(k));
  }

  sprintf(buf + strlen(buf), "  num_ext = '%d'\n", nim.num_ext);

  sprintf(buf + strlen(buf), "/>\n");   /* XML-ish closer */

  nbuf = (int)strlen(buf);
  buf = (char *)realloc((void *)buf, nbuf + 1); /* cut back to proper length */
  if (!buf) fprintf(stderr, "** NIFTI NITA: failed to realloc %d bytes\n",
    nbuf + 1);
  return buf;
}
DISABLE_WARNING_POP

/*---------------------------------------------------------------------------*/
/*! Return a pointer to a string holding the name of a NIFTI datatype.

    \param dt NIfTI-1 datatype

    \return pointer to static string holding the datatype name

    \warning Do not free() or modify this string!
             It points to static storage.

    \sa NIFTI1_DATATYPES group in nifti1.h
*//*-------------------------------------------------------------------------*/
char const * mdm_NiftiFormat::nifti_datatype_string(int dt)
{
  switch (dt) {
  case mdm_ImageDatatypes::DT_UNKNOWN:    return "UNKNOWN";
  case mdm_ImageDatatypes::DT_BINARY:     return "BINARY";
  case mdm_ImageDatatypes::DT_INT8:       return "INT8";
  case mdm_ImageDatatypes::DT_UINT8:      return "UINT8";
  case mdm_ImageDatatypes::DT_INT16:      return "INT16";
  case mdm_ImageDatatypes::DT_UINT16:     return "UINT16";
  case mdm_ImageDatatypes::DT_INT32:      return "INT32";
  case mdm_ImageDatatypes::DT_UINT32:     return "UINT32";
  case mdm_ImageDatatypes::DT_INT64:      return "INT64";
  case mdm_ImageDatatypes::DT_UINT64:     return "UINT64";
  case mdm_ImageDatatypes::DT_FLOAT32:    return "FLOAT32";
  case mdm_ImageDatatypes::DT_FLOAT64:    return "FLOAT64";
  case mdm_ImageDatatypes::DT_FLOAT128:   return "FLOAT128";
  case mdm_ImageDatatypes::DT_COMPLEX64:  return "COMPLEX64";
  case mdm_ImageDatatypes::DT_COMPLEX128: return "COMPLEX128";
  case mdm_ImageDatatypes::DT_COMPLEX256: return "COMPLEX256";
  case mdm_ImageDatatypes::DT_RGB24:      return "RGB24";
  case mdm_ImageDatatypes::DT_RGBA32:     return "RGBA32";
  }
  return "**ILLEGAL**";
}

/*---------------------------------------------------------------------------*/
/*! Return a pointer to a string holding the name of a NIFTI units type.

    \param  uu NIfTI-1 unit code

    \return pointer to static string for the given unit type

    \warning Do not free() or modify this string!
             It points to static storage.

    \sa     NIFTI1_UNITS group in nifti1.h
*//*-------------------------------------------------------------------------*/
char const *mdm_NiftiFormat::nifti_units_string(int uu)
{
  switch (uu) {
  case NIFTI_UNITS_METER:  return "m";
  case NIFTI_UNITS_MM:     return "mm";
  case NIFTI_UNITS_MICRON: return "um";
  case NIFTI_UNITS_SEC:    return "s";
  case NIFTI_UNITS_MSEC:   return "ms";
  case NIFTI_UNITS_USEC:   return "us";
  case NIFTI_UNITS_HZ:     return "Hz";
  case NIFTI_UNITS_PPM:    return "ppm";
  case NIFTI_UNITS_RADS:   return "rad/s";
  }
  return "Unknown";
}

/*---------------------------------------------------------------------------*/
/*! Return a pointer to a string holding the name of a NIFTI transform type.

    \param  xx NIfTI-1 xform code

    \return pointer to static string describing xform code

    \warning Do not free() or modify this string!
             It points to static storage.

    \sa     NIFTI1_XFORM_CODES group in nifti1.h
*//*-------------------------------------------------------------------------*/
char const *mdm_NiftiFormat::nifti_xform_string(int xx)
{
  switch (xx) {
  case NIFTI_XFORM_SCANNER_ANAT:  return "Scanner Anat";
  case NIFTI_XFORM_ALIGNED_ANAT:  return "Aligned Anat";
  case NIFTI_XFORM_TALAIRACH:     return "Talairach";
  case NIFTI_XFORM_MNI_152:       return "MNI_152";
  }
  return "Unknown";
}

/*---------------------------------------------------------------------------*/
/*! Return a pointer to a string holding the name of a NIFTI intent type.

    \param  ii NIfTI-1 intent code

    \return pointer to static string describing code

    \warning Do not free() or modify this string!
             It points to static storage.

    \sa     NIFTI1_INTENT_CODES group in nifti1.h
*//*-------------------------------------------------------------------------*/
char const *mdm_NiftiFormat::nifti_intent_string(int ii)
{
  switch (ii) {
  case NIFTI_INTENT_CORREL:     return "Correlation statistic";
  case NIFTI_INTENT_TTEST:      return "T-statistic";
  case NIFTI_INTENT_FTEST:      return "F-statistic";
  case NIFTI_INTENT_ZSCORE:     return "Z-score";
  case NIFTI_INTENT_CHISQ:      return "Chi-squared distribution";
  case NIFTI_INTENT_BETA:       return "Beta distribution";
  case NIFTI_INTENT_BINOM:      return "Binomial distribution";
  case NIFTI_INTENT_GAMMA:      return "Gamma distribution";
  case NIFTI_INTENT_POISSON:    return "Poisson distribution";
  case NIFTI_INTENT_NORMAL:     return "Normal distribution";
  case NIFTI_INTENT_FTEST_NONC: return "F-statistic noncentral";
  case NIFTI_INTENT_CHISQ_NONC: return "Chi-squared noncentral";
  case NIFTI_INTENT_LOGISTIC:   return "Logistic distribution";
  case NIFTI_INTENT_LAPLACE:    return "Laplace distribution";
  case NIFTI_INTENT_UNIFORM:    return "Uniform distribition";
  case NIFTI_INTENT_TTEST_NONC: return "T-statistic noncentral";
  case NIFTI_INTENT_WEIBULL:    return "Weibull distribution";
  case NIFTI_INTENT_CHI:        return "Chi distribution";
  case NIFTI_INTENT_INVGAUSS:   return "Inverse Gaussian distribution";
  case NIFTI_INTENT_EXTVAL:     return "Extreme Value distribution";
  case NIFTI_INTENT_PVAL:       return "P-value";

  case NIFTI_INTENT_LOGPVAL:    return "Log P-value";
  case NIFTI_INTENT_LOG10PVAL:  return "Log10 P-value";

  case NIFTI_INTENT_ESTIMATE:   return "Estimate";
  case NIFTI_INTENT_LABEL:      return "Label index";
  case NIFTI_INTENT_NEURONAME:  return "NeuroNames index";
  case NIFTI_INTENT_GENMATRIX:  return "General matrix";
  case NIFTI_INTENT_SYMMATRIX:  return "Symmetric matrix";
  case NIFTI_INTENT_DISPVECT:   return "Displacement vector";
  case NIFTI_INTENT_VECTOR:     return "Vector";
  case NIFTI_INTENT_POINTSET:   return "Pointset";
  case NIFTI_INTENT_TRIANGLE:   return "Triangle";
  case NIFTI_INTENT_QUATERNION: return "Quaternion";

  case NIFTI_INTENT_DIMLESS:    return "Dimensionless number";
  }
  return "Unknown";
}

/*---------------------------------------------------------------------------*/
/*! Return a pointer to a string holding the name of a NIFTI slice_code.

    \param  ss NIfTI-1 slice order code

    \return pointer to static string describing code

    \warning Do not free() or modify this string!
             It points to static storage.

    \sa     NIFTI1_SLICE_ORDER group in nifti1.h
*//*-------------------------------------------------------------------------*/
char const *mdm_NiftiFormat::nifti_slice_string(int ss)
{
  switch (ss) {
  case NIFTI_SLICE_SEQ_INC:  return "sequential_increasing";
  case NIFTI_SLICE_SEQ_DEC:  return "sequential_decreasing";
  case NIFTI_SLICE_ALT_INC:  return "alternating_increasing";
  case NIFTI_SLICE_ALT_DEC:  return "alternating_decreasing";
  case NIFTI_SLICE_ALT_INC2: return "alternating_increasing_2";
  case NIFTI_SLICE_ALT_DEC2: return "alternating_decreasing_2";
  }
  return "Unknown";
}

/*---------------------------------------------------------------------------*/
/*! Return a pointer to a string holding the name of a NIFTI orientation.

    \param ii orientation code

    \return pointer to static string holding the orientation information

    \warning Do not free() or modify the return string!
             It points to static storage.

    \sa  NIFTI_L2R in nifti1_io.h
*//*-------------------------------------------------------------------------*/
char const *mdm_NiftiFormat::nifti_orientation_string(int ii)
{
  switch (ii) {
  case NIFTI_ORIENTATION::L2R: return "Left-to-Right";
  case NIFTI_ORIENTATION::R2L: return "Right-to-Left";
  case NIFTI_ORIENTATION::P2A: return "Posterior-to-Anterior";
  case NIFTI_ORIENTATION::A2P: return "Anterior-to-Posterior";
  case NIFTI_ORIENTATION::I2S: return "Inferior-to-Superior";
  case NIFTI_ORIENTATION::S2I: return "Superior-to-Inferior";
  }
  return "Unknown";
}

/*----------------------------------------------------------------------*/
/*! set the nifti_image iname_offset field, based on nifti_type

    - use nifti_ver to determine the size of the header
      (0: default, else NIFTI-version)
    - if writing to 2 files, set offset to 0
    - if writing to a single NIFTI-1 file, set the offset to
         352 + total extension size, then align to 16-byte boundary
    - if writing an ASCII header, set offset to -1
*//*--------------------------------------------------------------------*/
void mdm_NiftiFormat::nifti_set_iname_offset(nifti_image &nim, int nifti_ver)
{
  int64_t offset;
  int64_t hsize = sizeof(nifti_1_header);  /* default */

  if (nifti_ver < 0 || nifti_ver > 2) {
    //if (g_opts.debug > 0)
    //  fprintf(stderr, "** invalid nifti_ver = %d for set_iname_offset\n",
    //    nifti_ver);
    /* but stick with the default */
  }
  else if (nifti_ver == 2) {
    hsize = sizeof(nifti_2_header);
  }

  switch (nim.nifti_type) {

  default:  /* writing into 2 files */
    /* we only write files with 0 offset in the 2 file format */
    nim.iname_offset = 0;
    break;

    /* NIFTI-1 single binary file - always update */
  case NIFTI_FTYPE::NIFTI1_1:
    offset = nifti_extension_size(nim) + hsize + 4;
    /* be sure offset is aligned to a 16 byte boundary */
    if ((offset % 16) != 0)  offset = ((offset + 0xf) & ~0xf);
    if (nim.iname_offset != offset) {
      //if (g_opts.debug > 1)
      //  fprintf(stderr, "+d changing offset from   to    \n");
      nim.iname_offset = offset;
    }
    break;

    /* non-standard case: NIFTI-1 ASCII header + binary data (single file) */
  case NIFTI_FTYPE::ASCII:
    nim.iname_offset = -1;             /* compute offset from filesize */
    break;
  }
}

/*----------------------------------------------------------------------*/
/*! compute the total size of all extensions

    \return the total of all esize fields

    Note that each esize includes 4 bytes for ecode, 4 bytes for esize,
    and the bytes used for the data.  Each esize also needs to be a
    multiple of 16, so it may be greater than the sum of its 3 parts.
*//*--------------------------------------------------------------------*/
int mdm_NiftiFormat::nifti_extension_size(nifti_image &nim)
{
  int c, size = 0;

  if (nim.num_ext <= 0) return 0;

  //if (g_opts.debug > 2) fprintf(stderr, "-d ext sizes:");

  for (c = 0; c < nim.num_ext; c++) {
    size += nim.ext_list[c].esize;
    //if (g_opts.debug > 2) fprintf(stderr, "  %d", nim.ext_list[c].esize);
  }

  //if (g_opts.debug > 2) fprintf(stderr, " (total = %d)\n", size);

  return size;
}