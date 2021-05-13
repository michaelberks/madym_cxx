/*!
 *  @file    mdm_NiftiFormat.h
 *  @brief   Class for Analyze image format reading and writing
 *  @details More info...
 *  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
 */

#ifndef MDM_NIFTIFORMAT_H
#define MDM_NIFTIFORMAT_H

#include "mdm_api.h"
#include <madym/mdm_Image3D.h>
#include <madym/image_io/analyze/mdm_AnalyzeFormat.h>
#include <madym/image_io/xtr/mdm_XtrFormat.h>
#include "znzlib.h"
#include "nifti1.h"                  /*** NIFTI-1 header specification ***/
#include "nifti2.h"                  /*** NIFTI-2 header specification ***/

 //! NIFTI image format reading and writing
	/*!
	*/
class mdm_NiftiFormat {

public:

	//!    Read Analyze format file(s) and return mdm_Image3D object
	/*!
	\param    fileName		name of file from which to read the data
	\param		loadXtr			flag, if true tries to load .xtr file too
	\return   mdm_Image3D object containing image read from disk
	*/
	MDM_API static mdm_Image3D readImage3D(const std::string& fileName,
		bool loadXtr);

	
	//!    Write mdm_Image3D to QBI extended Analyze hdr/img/xtr file set
	/*!
	\param    fileName      base name for file (gets .hdr/.img/.xtr appended)
	\param    img           mdm_Image3D holding the data to be written to file
	\param    dataTypeFlag  integer data type flag; see Data_type enum
	\param    xtrTypeFlag   integer xtr type flag; 0 for old, 1 for new
	\param		compress			flag, if true, write out compressed image (nii.gz)
	\return   bool 0 for success or 1 for failure
	*/
	MDM_API static void writeImage3D(const std::string & fileName,
		const mdm_Image3D &img,
		const mdm_ImageDatatypes::DataType dataTypeFlag, 
    const mdm_XtrFormat::XTR_type xtrTypeFlag,
		bool compress = false);

  //!    Test for existence of the file with the specified basename and all NIFTI extensions (.img, .hdr, .nii etc)
  /*!
  \param    fileName base name of files to test
  \param    warn  flag, if true triggers warning for program logger if files don't exist
  \return   bool true if files exist, false otherwise
  */
  MDM_API static bool filesExist(const std::string & fileName,
    bool warn = false);

protected:

private:

  /********************** Data structures for transforms **************************/
  struct mat44 {                   /** 4x4 matrix struct **/
    float m[4][4];
  };

  struct mat33{                   /** 3x3 matrix struct **/
    float m[3][3];
  } ;

  struct nifti_dmat44{                   /** 4x4 matrix struct (double) **/
    double m[4][4];
  };

  struct nifti_dmat33{                   /** 3x3 matrix struct (double) **/
    double m[3][3];
  };

  ///! nifti_type file code
  enum NIFTI_FTYPE {
    ANALYZE = 0, ///< old ANALYZE
    NIFTI1_1 = 1, ///< NIFTI-1, combined in single .nii file
    NIFTI1_2 = 2, ///< NIFTI-1, separate .hdr, .img files
    ASCII = 3, ///< NIFTI-ASCII format in .nia file
    NIFTI2_1 = 4, ///< NIFTI-2, combined in single .nii file
    NIFTI2_2 = 5, ///< NIFTI-2, separate .hdr, .img files
    MAX_FTYPE = 5         /* this should match the maximum code */
  };


  ///! Orientation codes that might be returned from nifti_mat44_to_orientation().*/
  enum NIFTI_ORIENTATION {
    L2R = 1, ///< Left to Right
    R2L = 2, ///< Right to Left
    P2A = 3, ///< Posterior to Anterior
    A2P = 4, ///< Anterior to Posterior
    I2S = 5, ///< Inferior to Superior 
    S2I = 6, ///< Superior to Inferior
  };

  ///! Byte order
  enum NIFTI_BYTE_ORDER {
    LSB_FIRST = 1, ///< Least significant bit first
    MSB_FIRST = 2  ///< Most significant bit first
  };

  /*...........................................................................*/

  /*! \enum analyze_75_orient_code
  *  \brief Old-style analyze75 orientation
  *         codes.
  */
  enum analyze_75_orient_code {
    a75_transverse_unflipped = 0,
    a75_coronal_unflipped = 1,
    a75_sagittal_unflipped = 2,
    a75_transverse_flipped = 3,
    a75_coronal_flipped = 4,
    a75_sagittal_flipped = 5,
    a75_orient_unknown = 6
  };

  /*! \struct nifti_image
      \brief High level data structure for open nifti datasets in the
             nifti2_io API.  Note that this structure is not part of the
             nifti2 format definition; it is used to implement one API
             for reading/writing datasets in the nifti1 or nifti2 formats.

      Field types changed for NIFTI-2 (note: ALL floats to doubles):
          nx, ny, ..., nw, dim, nvox,
          dx, dy, ..., dw, pixdim,
          scl_slope, scl_inter, cal_min, cal_max,
          slice_start, slice_end, slice_duration,
          quatern_b,c,d, qoffset_x,y,z, qfac,
          qto_xyz,ijk, sto_xyz,ijk,
          toffset, intent_p1,2,3, iname_offset
   */
  struct nifti_image {                /*!< Image storage struct **/

    int64_t ndim = 4;                /*!< last dimension greater than 1 (1..7) */
    int64_t nx = 0;                  /*!< dimensions of grid array             */
    int64_t ny = 0;                  /*!< dimensions of grid array             */
    int64_t nz = 0;                  /*!< dimensions of grid array             */
    int64_t nt = 0;                  /*!< dimensions of grid array             */
    int64_t nu = 0;                  /*!< dimensions of grid array             */
    int64_t nv = 0;                  /*!< dimensions of grid array             */
    int64_t nw = 0;                  /*!< dimensions of grid array             */
    int64_t dim[8];              /*!< dim[0]=ndim, dim[1]=nx, etc.         */
    int64_t nvox = 0;                /*!< number of voxels = nx*ny*nz*...*nw   */
    int nbyper = 0;                  /*!< bytes per voxel, matches datatype    */
    int datatype = 0;                /*!< type of data in voxels: DT_* code    */

    double dx = 1;                   /*!< grid spacings      */
    double dy = 1;                   /*!< grid spacings      */
    double dz = 1;                   /*!< grid spacings      */
    double dt = 1;                   /*!< grid spacings      */
    double du = 0;                   /*!< grid spacings      */
    double dv = 0;                   /*!< grid spacings      */
    double dw = 0;                   /*!< grid spacings      */
    double pixdim[8];            /*!< pixdim[1]=dx, etc. */

    double scl_slope = 1;            /*!< scaling parameter - slope        */
    double scl_inter = 0;            /*!< scaling parameter - intercept    */

    double cal_min = 0;              /*!< calibration parameter, minimum   */
    double cal_max = 0;              /*!< calibration parameter, maximum   */

    int qform_code = 0;              /*!< codes for (x,y,z) space meaning  */
    int sform_code = 0;              /*!< codes for (x,y,z) space meaning  */

    int freq_dim = 0;               /*!< indexes (1,2,3, or 0) for MRI    */
    int phase_dim = 0;               /*!< directions in dim[]/pixdim[]     */
    int slice_dim = 3;               /*!< directions in dim[]/pixdim[]     */

    int     slice_code = 0;         /*!< code for slice timing pattern    */

    /*! quaternion transform parameters
      [when writing a dataset, these are used for qform, NOT qto_xyz]   */
    double quatern_b = 0;
    double quatern_c = 0;
    double quatern_d = 0;
    double qoffset_x = 0;
    double qoffset_y = 0;
    double qoffset_z = 0;
    double qfac = 0;

    nifti_dmat44 qto_xyz;        /*!< qform: transform (i,j,k) to (x,y,z) */
    nifti_dmat44 qto_ijk;        /*!< qform: transform (x,y,z) to (i,j,k) */

    nifti_dmat44 sto_xyz;        /*!< sform: transform (i,j,k) to (x,y,z) */
    nifti_dmat44 sto_ijk;        /*!< sform: transform (x,y,z) to (i,j,k) */

    double toffset = 0;              /*!< time coordinate offset */

    int xyz_units = 2;              /*!< dx,dy,dz units: NIFTI_UNITS_* code  */
    int time_units = 0;              /*!< dt       units: NIFTI_UNITS_* code  */

    int nifti_type;              /*!< see NIFTI_FTYPE_* codes, below:
                                          0==ANALYZE,
                                          1==NIFTI-1     (1 file),
                                          2==NIFTI-1     (2 files),
                                          3==NIFTI-ASCII (1 file)
                                          4==NIFTI-2     (1 file),
                                          5==NIFTI-2     (2 files) */

    int    intent_code = 0;          /*!< statistic type (or something)       */
    double intent_p1 = 0;            /*!< intent parameters                   */
    double intent_p2 = 0;            /*!< intent parameters                   */
    double intent_p3 = 0;            /*!< intent parameters                   */
    char   intent_name[16];      /*!< optional description of intent data */

    char descrip[80];           /*!< optional text to describe dataset   */
    char aux_file[24];           /*!< auxiliary filename                  */

    std::string fname;                 /*!< header filename (.hdr or .nii)         */
    std::string iname;                 /*!< image filename  (.img or .nii)         */
    int64_t iname_offset;        /*!< offset into iname where data starts    */
    int   swapsize = 0;              /*!< swap unit in image data (might be 0)   */
    int   byteorder = LSB_FIRST;             /*!< byte order on disk (MSB_ or LSB_FIRST) */
    void *data;                  /*!< pointer to data: nbyper*nvox bytes     */

    int num_ext = 0;  /*!< number of extensions in ext_list       */
    nifti1_extension * ext_list; /*!< array of extension structs (with data) */
    analyze_75_orient_code analyze75_orient; /*!< for old analyze files, orient */

  };


  /*****************************************************************************/
  /*------------------ NIfTI version of ANALYZE 7.5 structure -----------------*/

  /* (based on fsliolib/dbh.h, but updated for version 7.5) */

  typedef struct {
    /* header info fields - describes the header    overlap with NIfTI */
    /*                                              ------------------ */
    int sizeof_hdr;                  /* 0 + 4        same              */
    char data_type[10];              /* 4 + 10       same              */
    char db_name[18];                /* 14 + 18      same              */
    int extents;                     /* 32 + 4       same              */
    short int session_error;         /* 36 + 2       same              */
    char regular;                    /* 38 + 1       same              */
    char hkey_un0;                   /* 39 + 1                40 bytes */

    /* image dimension fields - describes image sizes */
    short int dim[8];                /* 0 + 16       same              */
    short int unused8;               /* 16 + 2       intent_p1...      */
    short int unused9;               /* 18 + 2         ...             */
    short int unused10;              /* 20 + 2       intent_p2...      */
    short int unused11;              /* 22 + 2         ...             */
    short int unused12;              /* 24 + 2       intent_p3...      */
    short int unused13;              /* 26 + 2         ...             */
    short int unused14;              /* 28 + 2       intent_code       */
    short int datatype;              /* 30 + 2       same              */
    short int bitpix;                /* 32 + 2       same              */
    short int dim_un0;               /* 34 + 2       slice_start       */
    float pixdim[8];                 /* 36 + 32      same              */

    float vox_offset;                /* 68 + 4       same              */
    float funused1;                  /* 72 + 4       scl_slope         */
    float funused2;                  /* 76 + 4       scl_inter         */
    float funused3;                  /* 80 + 4       slice_end,        */
                                                  /* slice_code,       */
                                                  /* xyzt_units        */
    float cal_max;                   /* 84 + 4       same              */
    float cal_min;                   /* 88 + 4       same              */
    float compressed;                /* 92 + 4       slice_duration    */
    float verified;                  /* 96 + 4       toffset           */
    int glmax, glmin;                 /* 100 + 8              108 bytes */

    /* data history fields - optional */
    char descrip[80];                /* 0 + 80       same              */
    char aux_file[24];               /* 80 + 24      same              */
    char orient;                     /* 104 + 1      NO GOOD OVERLAP   */
    char originator[10];             /* 105 + 10     FROM HERE DOWN... */
    char generated[10];              /* 115 + 10                       */
    char scannum[10];                /* 125 + 10                       */
    char patient_id[10];             /* 135 + 10                       */
    char exp_date[10];               /* 145 + 10                       */
    char exp_time[10];               /* 155 + 10                       */
    char hist_un0[3];                /* 165 + 3                        */
    int views;                       /* 168 + 4                        */
    int vols_added;                  /* 172 + 4                        */
    int start_field;                 /* 176 + 4                        */
    int field_skip;                  /* 180 + 4                        */
    int omax, omin;                  /* 184 + 8                        */
    int smax, smin;                  /* 192 + 8              200 bytes */
  } nifti_analyze75;                                   /* total:  348 bytes */

  //----------------------------------------------------------------------------
  //Main read/write
  static nifti_image nifti_image_read(const std::string &hname, int read_data);

  static void nifti_image_write(nifti_image &nim);

  //----------------------------------------------------------------------------
  //Aux read/write
  static znzFile nifti_image_write_hdr_img(nifti_image &nim, int write_opts,
    const char * opts);

  static int nifti_image_load(nifti_image &nim);

  static znzFile nifti_image_load_prep(nifti_image &nim);

  static int64_t nifti_read_buffer(znzFile fp, void* dataptr, int64_t ntot,
    nifti_image &nim);

  static int nifti_write_all_data(znzFile fp, nifti_image &nim);

  static int64_t nifti_write_buffer(znzFile fp, const void *buffer, int64_t numbytes);
  

  //----------------------------------------------------------------------------
  //Read/write ASCII format
  static nifti_image nifti_read_ascii_image(znzFile fp, const std::string &fname, int flen,
    int read_data);

  static znzFile nifti_write_ascii_image(nifti_image &nim,
    const char *opts, int write_data, int leave_open);

  static char *nifti_image_to_ascii(const nifti_image &nim);

  static nifti_image nifti_image_from_ascii(const char *str, int * bytes_read);

  static int has_ascii_header(znzFile fp);

  //-----------------------------------------------------------------------------
  //Processing header and image names
  static void parseName(const std::string &fileName,
    std::string &baseName, std::string &ext, bool &gz);

  static std::string nifti_findhdrname(const std::string & fileName);
  static std::string nifti_findimgname(const std::string & fileName, int nifti_type);

  static int nifti_set_filenames(nifti_image &nim, const std::string &prefix, int check,
    int set_byte_order);

  static std::string nifti_makehdrname(const std::string &prefix, int nifti_type, int check,
    int comp);

  static std::string nifti_makeimgname(const std::string &prefix, int nifti_type, int check,
    int comp);

  static bool nifti_is_gzfile(const std::string &prefix);

  static int nifti_set_type_from_names(nifti_image &nim);

  static void nifti_set_iname_offset(nifti_image &nim, int nifti_ver);

  //------------------------------------------------------------------------
  //Converting header/image forms
  static nifti_image nifti_convert_n1hdr2nim(nifti_1_header nhdr, const std::string &fname);

  static nifti_image nifti_convert_n2hdr2nim(nifti_2_header nhdr, const std::string &fname);

  static int nifti_convert_nim2n2hdr(const nifti_image &nim, nifti_2_header &hdr);

  static int nifti_convert_nim2n1hdr(const nifti_image &nim, nifti_1_header &hdr);

  //------------------------------------------------------------------------
  //Byte swapping
  static int need_nhdr_swap(short dim0, int hdrsize);
  static void swap_nifti_header(void * hdr, int ni_ver);
  static void nifti_swap_as_nifti1(nifti_1_header * h);
  static void nifti_swap_as_nifti2(nifti_2_header * h);
  static void nifti_swap_as_analyze(nifti_analyze75 * h);

  //------------------------------------------------------------------------
  //Format/version/size checks
  static int64_t nifti_get_filesize(const std::string &pathname);

  static int nifti_header_version(const char *buf, size_t nbytes);

  static int64_t nifti_get_volsize(const nifti_image &nim);

  static int nifti_short_order(void);

  static void nifti_datatype_sizes(int datatype, int &nbyper, int &swapsize);

  static int is_valid_nifti_type(int nifti_type);
  
  //------------------------------------------------------------------------
  //NIFTI extensions
  static int nifti_read_extensions(nifti_image &nim, znzFile fp, int64_t remain);

  static int nifti_read_next_extension(nifti1_extension * nex, nifti_image &nim,
    int remain, znzFile fp);

  static int nifti_add_exten_to_list(nifti1_extension *  new_ext,
    nifti1_extension ** list, int new_length);

  static int nifti_check_extension(nifti_image &nim, int size, int code, int rem);

  static int nifti_write_extensions(znzFile fp, nifti_image &nim);

  static int valid_nifti_extensions(const nifti_image &nim);

  static int nifti_extension_size(nifti_image &nim);

  //------------------------------------------------------------------------
  //Clearing up
  static void nifti_image_free(nifti_image &nim);

  static int nifti_free_extensions(nifti_image &nim);

  //------------------------------------------------------------------------
  //String descriptors
  static char const * nifti_datatype_string(int dt);

  static char const *nifti_units_string(int uu);

  static char const *nifti_xform_string(int xx);

  static char const *nifti_intent_string(int ii);

  static char const *nifti_slice_string(int ss);

  static char const *nifti_orientation_string(int ii);

  //------------------------------------------------------------------------
  //String descriptors
  static nifti_dmat44 nifti_quatern_to_dmat44(double qb, double qc, double qd,
    double qx, double qy, double qz,
    double dx, double dy, double dz, double qfac);


  static mat44 nifti_quatern_to_mat44(float qb, float qc, float qd,
    float qx, float qy, float qz,
    float dx, float dy, float dz, float qfac);

  static double nifti_dmat33_determ(nifti_dmat33 R);

  static float nifti_mat33_determ(mat33 R);

  static double nifti_dmat33_rownorm(nifti_dmat33 A);

  static float nifti_mat33_rownorm(mat33 A);

  static double nifti_dmat33_colnorm(nifti_dmat33 A);

  static float nifti_mat33_colnorm(mat33 A);

  static nifti_dmat33 nifti_dmat33_mul(nifti_dmat33 A, nifti_dmat33 B);

  static mat33 nifti_mat33_mul(mat33 A, mat33 B);

  static nifti_dmat44 nifti_dmat44_mul(nifti_dmat44 A, nifti_dmat44 B);

  static mat44 nifti_mat44_mul(mat44 A, mat44 B);

  static nifti_dmat33 nifti_dmat33_inverse(nifti_dmat33 R);

  static nifti_dmat33 nifti_dmat33_polar(nifti_dmat33 A);

  static mat33 nifti_mat33_inverse(mat33 R);

  static mat33 nifti_mat33_polar(mat33 A);

  static void nifti_dmat44_to_quatern(nifti_dmat44 R,
    double *qb, double *qc, double *qd,
    double *qx, double *qy, double *qz,
    double *dx, double *dy, double *dz, double *qfac);

  static void nifti_mat44_to_quatern(mat44 R,
    float *qb, float *qc, float *qd,
    float *qx, float *qy, float *qz,
    float *dx, float *dy, float *dz, float *qfac);

  static nifti_dmat44 nifti_dmat44_inverse(nifti_dmat44 R);


  static mat44 nifti_mat44_inverse(mat44 R);

  static nifti_dmat44 nifti_make_orthog_dmat44(double r11, double r12, double r13,
    double r21, double r22, double r23,
    double r31, double r32, double r33);

  static mat44 nifti_make_orthog_mat44(float r11, float r12, float r13,
    float r21, float r22, float r23,
    float r31, float r32, float r33);

  static void nifti_dmat44_to_orientation(nifti_dmat44 R,
    int *icod, int *jcod, int *kcod);

  static void nifti_mat44_to_orientation(mat44 R, int *icod, int *jcod, int *kcod);

  //Copy to/from nifit_image data to Madym mdm_Image3D
  template <class T> static void fromData(const nifti_image &nii, mdm_Image3D &img);
  template <class T> static void toData(const mdm_Image3D &img, nifti_image &nii);

  //Variable constants
  static const std::string extnii;   /* modifiable, for possible uppercase */
  static const std::string exthdr;
  static const std::string extimg;
  static const std::string extnia;
  static const std::string extgz;

};


#endif /* MDM_NIFTIFORMAT_H */