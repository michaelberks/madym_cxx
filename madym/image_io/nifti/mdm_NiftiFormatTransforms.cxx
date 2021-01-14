/**
 *  @file    mdm_NiftiFormatAscii.cxx
 *  @brief   Implementation of class for Analyze image format reading and writing
 *  @author MA Berks (c) Copyright QBI Lab, University of Manchester 2020
 */

#include "mdm_NiftiFormat.h"

/*---------------------------------------------------------------------------*/
/*! Given the quaternion parameters (etc.), compute a transformation matrix
    of doubles.

   See comments in nifti1.h for details.
     - qb,qc,qd = quaternion parameters
     - qx,qy,qz = offset parameters
     - dx,dy,dz = grid stepsizes (non-negative inputs are set to 1.0)
     - qfac     = sign of dz step (< 0 is negative; >= 0 is positive)

   <pre>
   If qx=qy=qz=0, dx=dy=dz=1, then the output is a rotation matrix.
   For qfac >= 0, the rotation is proper.
   For qfac <  0, the rotation is improper.
   </pre>

   \see "QUATERNION REPRESENTATION OF ROTATION MATRIX" in nifti1.h
   \see nifti_mat44_to_quatern, nifti_make_orthog_mat44,
       nifti_mat44_to_orientation

*//*-------------------------------------------------------------------------*/
mdm_NiftiFormat::nifti_dmat44 mdm_NiftiFormat::nifti_quatern_to_dmat44(double qb, double qc, double qd,
  double qx, double qy, double qz,
  double dx, double dy, double dz, double qfac)
{
  nifti_dmat44 R;
  double a, b = qb, c = qc, d = qd, xd, yd, zd;

  /* last row is always [ 0 0 0 1 ] */

  R.m[3][0] = R.m[3][1] = R.m[3][2] = 0.0; R.m[3][3] = 1.0;

  /* compute a parameter from b,c,d */

  a = 1.0l - (b*b + c * c + d * d);
  if (a < 1.e-7l) {                   /* special case */
    a = 1.0l / sqrt(b*b + c * c + d * d);
    b *= a; c *= a; d *= a;        /* normalize (b,c,d) vector */
    a = 0.0l;                        /* a = 0 ==> 180 degree rotation */
  }
  else {
    a = sqrt(a);                     /* angle = 2*arccos(a) */
  }

  /* load rotation matrix, including scaling factors for voxel sizes */

  xd = (dx > 0.0) ? dx : 1.0l;       /* make sure are positive */
  yd = (dy > 0.0) ? dy : 1.0l;
  zd = (dz > 0.0) ? dz : 1.0l;

  if (qfac < 0.0) zd = -zd;         /* left handedness? */

  R.m[0][0] = (a*a + b * b - c * c - d * d) * xd;
  R.m[0][1] = 2.0l * (b*c - a * d) * yd;
  R.m[0][2] = 2.0l * (b*d + a * c) * zd;
  R.m[1][0] = 2.0l * (b*c + a * d) * xd;
  R.m[1][1] = (a*a + c * c - b * b - d * d) * yd;
  R.m[1][2] = 2.0l * (c*d - a * b) * zd;
  R.m[2][0] = 2.0l * (b*d - a * c) * xd;
  R.m[2][1] = 2.0l * (c*d + a * b) * yd;
  R.m[2][2] = (a*a + d * d - c * c - b * b) * zd;

  /* load offsets */

  R.m[0][3] = qx; R.m[1][3] = qy; R.m[2][3] = qz;

  return R;
}

/*---------------------------------------------------------------------------*/
/*! Given the quaternion parameters (etc.), compute a transformation matrix.

   See comments in nifti1.h for details.
     - qb,qc,qd = quaternion parameters
     - qx,qy,qz = offset parameters
     - dx,dy,dz = grid stepsizes (non-negative inputs are set to 1.0)
     - qfac     = sign of dz step (< 0 is negative; >= 0 is positive)

   <pre>
   If qx=qy=qz=0, dx=dy=dz=1, then the output is a rotation matrix.
   For qfac >= 0, the rotation is proper.
   For qfac <  0, the rotation is improper.
   </pre>

   \see "QUATERNION REPRESENTATION OF ROTATION MATRIX" in nifti1.h
   \see nifti_mat44_to_quatern, nifti_make_orthog_mat44,
       nifti_mat44_to_orientation

*//*-------------------------------------------------------------------------*/
mdm_NiftiFormat::mat44 mdm_NiftiFormat::nifti_quatern_to_mat44(float qb, float qc, float qd,
  float qx, float qy, float qz,
  float dx, float dy, float dz, float qfac)
{
  mat44 R;
  double a, b = qb, c = qc, d = qd, xd, yd, zd;

  /* last row is always [ 0 0 0 1 ] */

  R.m[3][0] = R.m[3][1] = R.m[3][2] = 0.0f; R.m[3][3] = 1.0f;

  /* compute a parameter from b,c,d */

  a = 1.0l - (b*b + c * c + d * d);
  if (a < 1.e-7l) {                   /* special case */
    a = 1.0l / sqrt(b*b + c * c + d * d);
    b *= a; c *= a; d *= a;        /* normalize (b,c,d) vector */
    a = 0.0l;                        /* a = 0 ==> 180 degree rotation */
  }
  else {
    a = sqrt(a);                     /* angle = 2*arccos(a) */
  }

  /* load rotation matrix, including scaling factors for voxel sizes */

  xd = (dx > 0.0) ? dx : 1.0l;       /* make sure are positive */
  yd = (dy > 0.0) ? dy : 1.0l;
  zd = (dz > 0.0) ? dz : 1.0l;

  if (qfac < 0.0) zd = -zd;         /* left handedness? */

  R.m[0][0] = (float)((a*a + b * b - c * c - d * d) * xd);
  R.m[0][1] = (float)(2.0l * (b*c - a * d) * yd);
  R.m[0][2] = (float)(2.0l * (b*d + a * c) * zd);
  R.m[1][0] = (float)(2.0l * (b*c + a * d) * xd);
  R.m[1][1] = (float)((a*a + c * c - b * b - d * d) * yd);
  R.m[1][2] = (float)(2.0l * (c*d - a * b) * zd);
  R.m[2][0] = (float)(2.0l * (b*d - a * c) * xd);
  R.m[2][1] = (float)(2.0l * (c*d + a * b) * yd);
  R.m[2][2] = (float)((a*a + d * d - c * c - b * b) * zd);

  /* load offsets */

  R.m[0][3] = qx; R.m[1][3] = qy; R.m[2][3] = qz;

  return R;
}

/*----------------------------------------------------------------------*/
/*! compute the determinant of a 3x3 matrix
*//*--------------------------------------------------------------------*/
double mdm_NiftiFormat::nifti_dmat33_determ(nifti_dmat33 R)   /* determinant of 3x3 matrix */
{
  double r11, r12, r13, r21, r22, r23, r31, r32, r33;
  /*  INPUT MATRIX:  */
  r11 = R.m[0][0]; r12 = R.m[0][1]; r13 = R.m[0][2];  /* [ r11 r12 r13 ] */
  r21 = R.m[1][0]; r22 = R.m[1][1]; r23 = R.m[1][2];  /* [ r21 r22 r23 ] */
  r31 = R.m[2][0]; r32 = R.m[2][1]; r33 = R.m[2][2];  /* [ r31 r32 r33 ] */

  return (r11*r22*r33 - r11 * r32*r23 - r21 * r12*r33
    + r21 * r32*r13 + r31 * r12*r23 - r31 * r22*r13);
}

/*----------------------------------------------------------------------*/
/*! compute the determinant of a 3x3 matrix
*//*--------------------------------------------------------------------*/
float mdm_NiftiFormat::nifti_mat33_determ(mat33 R)   /* determinant of 3x3 matrix */
{
  double r11, r12, r13, r21, r22, r23, r31, r32, r33;
  /*  INPUT MATRIX:  */
  r11 = R.m[0][0]; r12 = R.m[0][1]; r13 = R.m[0][2];  /* [ r11 r12 r13 ] */
  r21 = R.m[1][0]; r22 = R.m[1][1]; r23 = R.m[1][2];  /* [ r21 r22 r23 ] */
  r31 = R.m[2][0]; r32 = R.m[2][1]; r33 = R.m[2][2];  /* [ r31 r32 r33 ] */

  return (float)(r11*r22*r33 - r11 * r32*r23 - r21 * r12*r33
    + r21 * r32*r13 + r31 * r12*r23 - r31 * r22*r13);
}

/*----------------------------------------------------------------------*/
/*! compute the max row norm of a 3x3 matrix
*//*--------------------------------------------------------------------*/
double mdm_NiftiFormat::nifti_dmat33_rownorm(nifti_dmat33 A)  /* max row norm of 3x3 matrix */
{
  double r1, r2, r3;

  r1 = fabs(A.m[0][0]) + fabs(A.m[0][1]) + fabs(A.m[0][2]);
  r2 = fabs(A.m[1][0]) + fabs(A.m[1][1]) + fabs(A.m[1][2]);
  r3 = fabs(A.m[2][0]) + fabs(A.m[2][1]) + fabs(A.m[2][2]);
  if (r1 < r2) r1 = r2;
  if (r1 < r3) r1 = r3;
  return r1;
}

/*----------------------------------------------------------------------*/
/*! compute the max row norm of a 3x3 matrix
*//*--------------------------------------------------------------------*/
float mdm_NiftiFormat::nifti_mat33_rownorm(mat33 A)  /* max row norm of 3x3 matrix */
{
  float r1, r2, r3;

  r1 = (float)(fabs(A.m[0][0]) + fabs(A.m[0][1]) + fabs(A.m[0][2]));
  r2 = (float)(fabs(A.m[1][0]) + fabs(A.m[1][1]) + fabs(A.m[1][2]));
  r3 = (float)(fabs(A.m[2][0]) + fabs(A.m[2][1]) + fabs(A.m[2][2]));
  if (r1 < r2) r1 = r2;
  if (r1 < r3) r1 = r3;
  return r1;
}

/*----------------------------------------------------------------------*/
/*! compute the max column norm of a 3x3 matrix
*//*--------------------------------------------------------------------*/
double mdm_NiftiFormat::nifti_dmat33_colnorm(nifti_dmat33 A)/* max column norm of 3x3 matrix */
{
  double r1, r2, r3;

  r1 = fabs(A.m[0][0]) + fabs(A.m[1][0]) + fabs(A.m[2][0]);
  r2 = fabs(A.m[0][1]) + fabs(A.m[1][1]) + fabs(A.m[2][1]);
  r3 = fabs(A.m[0][2]) + fabs(A.m[1][2]) + fabs(A.m[2][2]);
  if (r1 < r2) r1 = r2;
  if (r1 < r3) r1 = r3;
  return r1;
}

/*----------------------------------------------------------------------*/
/*! compute the max column norm of a 3x3 matrix
*//*--------------------------------------------------------------------*/
float mdm_NiftiFormat::nifti_mat33_colnorm(mat33 A)  /* max column norm of 3x3 matrix */
{
  float r1, r2, r3;

  r1 = (float)(fabs(A.m[0][0]) + fabs(A.m[1][0]) + fabs(A.m[2][0]));
  r2 = (float)(fabs(A.m[0][1]) + fabs(A.m[1][1]) + fabs(A.m[2][1]));
  r3 = (float)(fabs(A.m[0][2]) + fabs(A.m[1][2]) + fabs(A.m[2][2]));
  if (r1 < r2) r1 = r2;
  if (r1 < r3) r1 = r3;
  return r1;
}

/*----------------------------------------------------------------------*/
/*! multiply 2 3x3 matrices
*//*--------------------------------------------------------------------*/
mdm_NiftiFormat::nifti_dmat33 mdm_NiftiFormat::nifti_dmat33_mul(nifti_dmat33 A, nifti_dmat33 B)
/* multiply 2 3x3 matrices */
{
  nifti_dmat33 C; int i, j;
  for (i = 0; i < 3; i++)
    for (j = 0; j < 3; j++)
      C.m[i][j] = A.m[i][0] * B.m[0][j]
      + A.m[i][1] * B.m[1][j]
      + A.m[i][2] * B.m[2][j];
  return C;
}

/*----------------------------------------------------------------------*/
/*! multiply 2 3x3 matrices
*//*--------------------------------------------------------------------*/
mdm_NiftiFormat::mat33 mdm_NiftiFormat::nifti_mat33_mul(mat33 A, mat33 B)  /* multiply 2 3x3 matrices */
{
  mat33 C; int i, j;
  for (i = 0; i < 3; i++)
    for (j = 0; j < 3; j++)
      C.m[i][j] = A.m[i][0] * B.m[0][j]
      + A.m[i][1] * B.m[1][j]
      + A.m[i][2] * B.m[2][j];
  return C;
}

/*----------------------------------------------------------------------*/
/*! multiply 2 4x4 matrices
*//*--------------------------------------------------------------------*/
mdm_NiftiFormat::nifti_dmat44 mdm_NiftiFormat::nifti_dmat44_mul(nifti_dmat44 A, nifti_dmat44 B)
{
  nifti_dmat44 C; int i, j, k;
  for (i = 0; i < 4; i++)
    for (j = 0; j < 4; j++) {
      C.m[i][j] = 0.0;
      for (k = 0; k < 4; k++)
        C.m[i][j] += A.m[i][k] * B.m[k][j];
    }
  return C;
}

/*----------------------------------------------------------------------*/
/*! multiply 2 4x4 matrices
*//*--------------------------------------------------------------------*/
mdm_NiftiFormat::mat44 mdm_NiftiFormat::nifti_mat44_mul(mat44 A, mat44 B)
{
  mat44 C; int i, j, k;
  for (i = 0; i < 4; i++)
    for (j = 0; j < 4; j++) {
      C.m[i][j] = 0.0;
      for (k = 0; k < 4; k++)
        C.m[i][j] += A.m[i][k] * B.m[k][j];
    }
  return C;
}

/*----------------------------------------------------------------------*/
/*! compute the inverse of a 3x3 matrix
*//*--------------------------------------------------------------------*/
mdm_NiftiFormat::nifti_dmat33 mdm_NiftiFormat::nifti_dmat33_inverse(nifti_dmat33 R) /* inverse of 3x3 matrix */
{
  double r11, r12, r13, r21, r22, r23, r31, r32, r33, deti;
  nifti_dmat33 Q;
  /*  INPUT MATRIX:  */
  r11 = R.m[0][0]; r12 = R.m[0][1]; r13 = R.m[0][2];  /* [ r11 r12 r13 ] */
  r21 = R.m[1][0]; r22 = R.m[1][1]; r23 = R.m[1][2];  /* [ r21 r22 r23 ] */
  r31 = R.m[2][0]; r32 = R.m[2][1]; r33 = R.m[2][2];  /* [ r31 r32 r33 ] */

  deti = r11 * r22*r33 - r11 * r32*r23 - r21 * r12*r33
    + r21 * r32*r13 + r31 * r12*r23 - r31 * r22*r13;

  if (deti != 0.0l) deti = 1.0l / deti;

  Q.m[0][0] = deti * (r22*r33 - r32 * r23);
  Q.m[0][1] = deti * (-r12 * r33 + r32 * r13);
  Q.m[0][2] = deti * (r12*r23 - r22 * r13);

  Q.m[1][0] = deti * (-r21 * r33 + r31 * r23);
  Q.m[1][1] = deti * (r11*r33 - r31 * r13);
  Q.m[1][2] = deti * (-r11 * r23 + r21 * r13);

  Q.m[2][0] = deti * (r21*r32 - r31 * r22);
  Q.m[2][1] = deti * (-r11 * r32 + r31 * r12);
  Q.m[2][2] = deti * (r11*r22 - r21 * r12);

  return Q;
}

/*---------------------------------------------------------------------------*/
/*! polar decomposition of a 3x3 matrix

   This finds the closest orthogonal matrix to input A
   (in both the Frobenius and L2 norms).

   Algorithm is that from NJ Higham, SIAM J Sci Stat Comput, 7:1160-1174.
*//*-------------------------------------------------------------------------*/
mdm_NiftiFormat::nifti_dmat33 mdm_NiftiFormat::nifti_dmat33_polar(nifti_dmat33 A)
{
  nifti_dmat33 X, Y, Z;
  double alp, bet, gam, gmi, dif = 1.0;
  int k = 0;

  X = A;

  /* force matrix to be nonsingular */

  gam = nifti_dmat33_determ(X);
  while (gam == 0.0) {        /* perturb matrix */
    gam = 0.00001 * (0.001 + nifti_dmat33_rownorm(X));
    X.m[0][0] += gam; X.m[1][1] += gam; X.m[2][2] += gam;
    gam = nifti_dmat33_determ(X);
  }

  while (1) {
    Y = nifti_dmat33_inverse(X);
    if (dif > 0.3) {     /* far from convergence */
      alp = sqrt(nifti_dmat33_rownorm(X) * nifti_dmat33_colnorm(X));
      bet = sqrt(nifti_dmat33_rownorm(Y) * nifti_dmat33_colnorm(Y));
      gam = sqrt(bet / alp);
      gmi = 1.0 / gam;
    }
    else {
      gam = gmi = 1.0f;  /* close to convergence */
    }
    Z.m[0][0] = 0.5 * (gam*X.m[0][0] + gmi * Y.m[0][0]);
    Z.m[0][1] = 0.5 * (gam*X.m[0][1] + gmi * Y.m[1][0]);
    Z.m[0][2] = 0.5 * (gam*X.m[0][2] + gmi * Y.m[2][0]);
    Z.m[1][0] = 0.5 * (gam*X.m[1][0] + gmi * Y.m[0][1]);
    Z.m[1][1] = 0.5 * (gam*X.m[1][1] + gmi * Y.m[1][1]);
    Z.m[1][2] = 0.5 * (gam*X.m[1][2] + gmi * Y.m[2][1]);
    Z.m[2][0] = 0.5 * (gam*X.m[2][0] + gmi * Y.m[0][2]);
    Z.m[2][1] = 0.5 * (gam*X.m[2][1] + gmi * Y.m[1][2]);
    Z.m[2][2] = 0.5 * (gam*X.m[2][2] + gmi * Y.m[2][2]);

    dif = fabs(Z.m[0][0] - X.m[0][0]) + fabs(Z.m[0][1] - X.m[0][1])
      + fabs(Z.m[0][2] - X.m[0][2]) + fabs(Z.m[1][0] - X.m[1][0])
      + fabs(Z.m[1][1] - X.m[1][1]) + fabs(Z.m[1][2] - X.m[1][2])
      + fabs(Z.m[2][0] - X.m[2][0]) + fabs(Z.m[2][1] - X.m[2][1])
      + fabs(Z.m[2][2] - X.m[2][2]);

    k = k + 1;
    if (k > 100 || dif < 3.e-6) break;  /* convergence or exhaustion */
    X = Z;
  }

  return Z;
}

/*----------------------------------------------------------------------*/
/*! compute the inverse of a 3x3 matrix
*//*--------------------------------------------------------------------*/
mdm_NiftiFormat::mat33 mdm_NiftiFormat::nifti_mat33_inverse(mat33 R)   /* inverse of 3x3 matrix */
{
  double r11, r12, r13, r21, r22, r23, r31, r32, r33, deti;
  mat33 Q;
  /*  INPUT MATRIX:  */
  r11 = R.m[0][0]; r12 = R.m[0][1]; r13 = R.m[0][2];  /* [ r11 r12 r13 ] */
  r21 = R.m[1][0]; r22 = R.m[1][1]; r23 = R.m[1][2];  /* [ r21 r22 r23 ] */
  r31 = R.m[2][0]; r32 = R.m[2][1]; r33 = R.m[2][2];  /* [ r31 r32 r33 ] */

  deti = r11 * r22*r33 - r11 * r32*r23 - r21 * r12*r33
    + r21 * r32*r13 + r31 * r12*r23 - r31 * r22*r13;

  if (deti != 0.0l) deti = 1.0l / deti;

  Q.m[0][0] = (float)(deti*(r22*r33 - r32 * r23));
  Q.m[0][1] = (float)(deti*(-r12 * r33 + r32 * r13));
  Q.m[0][2] = (float)(deti*(r12*r23 - r22 * r13));

  Q.m[1][0] = (float)(deti*(-r21 * r33 + r31 * r23));
  Q.m[1][1] = (float)(deti*(r11*r33 - r31 * r13));
  Q.m[1][2] = (float)(deti*(-r11 * r23 + r21 * r13));

  Q.m[2][0] = (float)(deti*(r21*r32 - r31 * r22));
  Q.m[2][1] = (float)(deti*(-r11 * r32 + r31 * r12));
  Q.m[2][2] = (float)(deti*(r11*r22 - r21 * r12));

  return Q;
}

/*---------------------------------------------------------------------------*/
/*! polar decomposition of a 3x3 matrix

   This finds the closest orthogonal matrix to input A
   (in both the Frobenius and L2 norms).

   Algorithm is that from NJ Higham, SIAM J Sci Stat Comput, 7:1160-1174.
*//*-------------------------------------------------------------------------*/
mdm_NiftiFormat::mat33 mdm_NiftiFormat::nifti_mat33_polar(mat33 A)
{
  mat33 X, Y, Z;
  float alp, bet, gam, gmi, dif = 1.0f;
  int k = 0;

  X = A;

  /* force matrix to be nonsingular */

  gam = nifti_mat33_determ(X);
  while (gam == 0.0) {        /* perturb matrix */
    gam = (float)(0.00001 * (0.001 + nifti_mat33_rownorm(X)));
    X.m[0][0] += gam; X.m[1][1] += gam; X.m[2][2] += gam;
    gam = nifti_mat33_determ(X);
  }

  while (1) {
    Y = nifti_mat33_inverse(X);
    if (dif > 0.3) {     /* far from convergence */
      alp = (float)(sqrt(nifti_mat33_rownorm(X) * nifti_mat33_colnorm(X)));
      bet = (float)(sqrt(nifti_mat33_rownorm(Y) * nifti_mat33_colnorm(Y)));
      gam = (float)(sqrt(bet / alp));
      gmi = (float)(1.0 / gam);
    }
    else {
      gam = gmi = 1.0f;  /* close to convergence */
    }
    Z.m[0][0] = (float)(0.5 * (gam*X.m[0][0] + gmi * Y.m[0][0]));
    Z.m[0][1] = (float)(0.5 * (gam*X.m[0][1] + gmi * Y.m[1][0]));
    Z.m[0][2] = (float)(0.5 * (gam*X.m[0][2] + gmi * Y.m[2][0]));
    Z.m[1][0] = (float)(0.5 * (gam*X.m[1][0] + gmi * Y.m[0][1]));
    Z.m[1][1] = (float)(0.5 * (gam*X.m[1][1] + gmi * Y.m[1][1]));
    Z.m[1][2] = (float)(0.5 * (gam*X.m[1][2] + gmi * Y.m[2][1]));
    Z.m[2][0] = (float)(0.5 * (gam*X.m[2][0] + gmi * Y.m[0][2]));
    Z.m[2][1] = (float)(0.5 * (gam*X.m[2][1] + gmi * Y.m[1][2]));
    Z.m[2][2] = (float)(0.5 * (gam*X.m[2][2] + gmi * Y.m[2][2]));

    dif = (float)(fabs(Z.m[0][0] - X.m[0][0]) + fabs(Z.m[0][1] - X.m[0][1])
      + fabs(Z.m[0][2] - X.m[0][2]) + fabs(Z.m[1][0] - X.m[1][0])
      + fabs(Z.m[1][1] - X.m[1][1]) + fabs(Z.m[1][2] - X.m[1][2])
      + fabs(Z.m[2][0] - X.m[2][0]) + fabs(Z.m[2][1] - X.m[2][1])
      + fabs(Z.m[2][2] - X.m[2][2]));

    k = k + 1;
    if (k > 100 || dif < 3.e-6) break;  /* convergence or exhaustion */
    X = Z;
  }

  return Z;
}

#undef  ASSIF                                 /* assign v to *p, if possible */
#define ASSIF(p,v) if( (p)!=NULL ) *(p) = (v)

/*---------------------------------------------------------------------------*/
/*! Given the 3x4 upper corner of the matrix R, compute the quaternion
   parameters that fit it.

     - Any NULL pointer on input won't get assigned (e.g., if you don't want
       dx,dy,dz, just pass NULL in for those pointers).
     - If the 3 input matrix columns are NOT orthogonal, they will be
       orthogonalized prior to calculating the parameters, using
       the polar decomposition to find the orthogonal matrix closest
       to the column-normalized input matrix.
     - However, if the 3 input matrix columns are NOT orthogonal, then
       the matrix produced by nifti_quatern_to_dmat44 WILL have orthogonal
       columns, so it won't be the same as the matrix input here.
       This "feature" is because the NIFTI 'qform' transform is
       deliberately not fully general -- it is intended to model a volume
       with perpendicular axes.
     - If the 3 input matrix columns are not even linearly independent,
       you'll just have to take your luck, won't you?

   \see "QUATERNION REPRESENTATION OF ROTATION MATRIX" in nifti1.h

   \see nifti_quatern_to_dmat44, nifti_make_orthog_dmat44,
       nifti_dmat44_to_orientation
*//*-------------------------------------------------------------------------*/
void mdm_NiftiFormat::nifti_dmat44_to_quatern(nifti_dmat44 R,
  double *qb, double *qc, double *qd,
  double *qx, double *qy, double *qz,
  double *dx, double *dy, double *dz, double *qfac)
{
  double r11, r12, r13, r21, r22, r23, r31, r32, r33;
  double xd, yd, zd, a, b, c, d;
  nifti_dmat33 P, Q;

  /* offset outputs are read write out of input matrix  */

  ASSIF(qx, R.m[0][3]); ASSIF(qy, R.m[1][3]); ASSIF(qz, R.m[2][3]);

  /* load 3x3 matrix into local variables */

  r11 = R.m[0][0]; r12 = R.m[0][1]; r13 = R.m[0][2];
  r21 = R.m[1][0]; r22 = R.m[1][1]; r23 = R.m[1][2];
  r31 = R.m[2][0]; r32 = R.m[2][1]; r33 = R.m[2][2];

  /* compute lengths of each column; these determine grid spacings  */

  xd = sqrt(r11*r11 + r21 * r21 + r31 * r31);
  yd = sqrt(r12*r12 + r22 * r22 + r32 * r32);
  zd = sqrt(r13*r13 + r23 * r23 + r33 * r33);

  /* if a column length is zero, patch the trouble */

  if (xd == 0.0l) { r11 = 1.0l; r21 = r31 = 0.0l; xd = 1.0l; }
  if (yd == 0.0l) { r22 = 1.0l; r12 = r32 = 0.0l; yd = 1.0l; }
  if (zd == 0.0l) { r33 = 1.0l; r13 = r23 = 0.0l; zd = 1.0l; }

  /* assign the output lengths */

  ASSIF(dx, xd); ASSIF(dy, yd); ASSIF(dz, zd);

  /* normalize the columns */

  r11 /= xd; r21 /= xd; r31 /= xd;
  r12 /= yd; r22 /= yd; r32 /= yd;
  r13 /= zd; r23 /= zd; r33 /= zd;

  /* At this point, the matrix has normal columns, but we have to allow
     for the fact that the hideous user may not have given us a matrix
     with orthogonal columns.

     So, now find the orthogonal matrix closest to the current matrix.

     One reason for using the polar decomposition to get this
     orthogonal matrix, rather than just directly orthogonalizing
     the columns, is so that inputting the inverse matrix to R
     will result in the inverse orthogonal matrix at this point.
     If we just orthogonalized the columns, this wouldn't necessarily hold. */

  Q.m[0][0] = r11; Q.m[0][1] = r12; Q.m[0][2] = r13; /* load Q */
  Q.m[1][0] = r21; Q.m[1][1] = r22; Q.m[1][2] = r23;
  Q.m[2][0] = r31; Q.m[2][1] = r32; Q.m[2][2] = r33;

  P = nifti_dmat33_polar(Q);  /* P is orthog matrix closest to Q */

  r11 = P.m[0][0]; r12 = P.m[0][1]; r13 = P.m[0][2]; /* unload */
  r21 = P.m[1][0]; r22 = P.m[1][1]; r23 = P.m[1][2];
  r31 = P.m[2][0]; r32 = P.m[2][1]; r33 = P.m[2][2];

  /*                            [ r11 r12 r13 ]               */
  /* at this point, the matrix  [ r21 r22 r23 ] is orthogonal */
  /*                            [ r31 r32 r33 ]               */

  /* compute the determinant to determine if it is proper */

  zd = r11 * r22*r33 - r11 * r32*r23 - r21 * r12*r33
    + r21 * r32*r13 + r31 * r12*r23 - r31 * r22*r13;  /* should be -1 or 1 */

  if (zd > 0) {             /* proper */
    ASSIF(qfac, 1.0);
  }
  else {                  /* improper ==> flip 3rd column */
    ASSIF(qfac, -1.0);
    r13 = -r13; r23 = -r23; r33 = -r33;
  }

  /* now, compute quaternion parameters */

  a = r11 + r22 + r33 + 1.0l;

  if (a > 0.5l) {                /* simplest case */
    a = 0.5l * sqrt(a);
    b = 0.25l * (r32 - r23) / a;
    c = 0.25l * (r13 - r31) / a;
    d = 0.25l * (r21 - r12) / a;
  }
  else {                       /* trickier case */
    xd = 1.0 + r11 - (r22 + r33);  /* 4*b*b */
    yd = 1.0 + r22 - (r11 + r33);  /* 4*c*c */
    zd = 1.0 + r33 - (r11 + r22);  /* 4*d*d */
    if (xd > 1.0) {
      b = 0.5l * sqrt(xd);
      c = 0.25l* (r12 + r21) / b;
      d = 0.25l* (r13 + r31) / b;
      a = 0.25l* (r32 - r23) / b;
    }
    else if (yd > 1.0) {
      c = 0.5l * sqrt(yd);
      b = 0.25l* (r12 + r21) / c;
      d = 0.25l* (r23 + r32) / c;
      a = 0.25l* (r13 - r31) / c;
    }
    else {
      d = 0.5l * sqrt(zd);
      b = 0.25l* (r13 + r31) / d;
      c = 0.25l* (r23 + r32) / d;
      a = 0.25l* (r21 - r12) / d;
    }
    /* to be mathematically consistent, this would include a = -a */
    if (a < 0.0l) { b = -b; c = -c; d = -d; }
  }

  ASSIF(qb, b); ASSIF(qc, c); ASSIF(qd, d);
  return;
}

/*---------------------------------------------------------------------------*/
/*! Given the 3x4 upper corner of the matrix R, compute the quaternion
   parameters that fit it.

     - Any NULL pointer on input won't get assigned (e.g., if you don't want
       dx,dy,dz, just pass NULL in for those pointers).
     - If the 3 input matrix columns are NOT orthogonal, they will be
       orthogonalized prior to calculating the parameters, using
       the polar decomposition to find the orthogonal matrix closest
       to the column-normalized input matrix.
     - However, if the 3 input matrix columns are NOT orthogonal, then
       the matrix produced by nifti_quatern_to_mat44 WILL have orthogonal
       columns, so it won't be the same as the matrix input here.
       This "feature" is because the NIFTI 'qform' transform is
       deliberately not fully general -- it is intended to model a volume
       with perpendicular axes.
     - If the 3 input matrix columns are not even linearly independent,
       you'll just have to take your luck, won't you?

   \see "QUATERNION REPRESENTATION OF ROTATION MATRIX" in nifti1.h

   \see nifti_quatern_to_mat44, nifti_make_orthog_mat44,
       nifti_mat44_to_orientation
*//*-------------------------------------------------------------------------*/
void mdm_NiftiFormat::nifti_mat44_to_quatern(mat44 R,
  float *qb, float *qc, float *qd,
  float *qx, float *qy, float *qz,
  float *dx, float *dy, float *dz, float *qfac)
{
  double r11, r12, r13, r21, r22, r23, r31, r32, r33;
  double xd, yd, zd, a, b, c, d;
  mat33 P, Q;

  /* offset outputs are read write out of input matrix  */

  ASSIF(qx, R.m[0][3]); ASSIF(qy, R.m[1][3]); ASSIF(qz, R.m[2][3]);

  /* load 3x3 matrix into local variables */

  r11 = R.m[0][0]; r12 = R.m[0][1]; r13 = R.m[0][2];
  r21 = R.m[1][0]; r22 = R.m[1][1]; r23 = R.m[1][2];
  r31 = R.m[2][0]; r32 = R.m[2][1]; r33 = R.m[2][2];

  /* compute lengths of each column; these determine grid spacings  */

  xd = sqrt(r11*r11 + r21 * r21 + r31 * r31);
  yd = sqrt(r12*r12 + r22 * r22 + r32 * r32);
  zd = sqrt(r13*r13 + r23 * r23 + r33 * r33);

  /* if a column length is zero, patch the trouble */

  if (xd == 0.0l) { r11 = 1.0l; r21 = r31 = 0.0l; xd = 1.0l; }
  if (yd == 0.0l) { r22 = 1.0l; r12 = r32 = 0.0l; yd = 1.0l; }
  if (zd == 0.0l) { r33 = 1.0l; r13 = r23 = 0.0l; zd = 1.0l; }

  /* assign the output lengths */

  ASSIF(dx, (float)xd); ASSIF(dy, (float)yd); ASSIF(dz, (float)zd);

  /* normalize the columns */

  r11 /= xd; r21 /= xd; r31 /= xd;
  r12 /= yd; r22 /= yd; r32 /= yd;
  r13 /= zd; r23 /= zd; r33 /= zd;

  /* At this point, the matrix has normal columns, but we have to allow
     for the fact that the hideous user may not have given us a matrix
     with orthogonal columns.

     So, now find the orthogonal matrix closest to the current matrix.

     One reason for using the polar decomposition to get this
     orthogonal matrix, rather than just directly orthogonalizing
     the columns, is so that inputting the inverse matrix to R
     will result in the inverse orthogonal matrix at this point.
     If we just orthogonalized the columns, this wouldn't necessarily hold. */

  Q.m[0][0] = (float)r11; Q.m[0][1] = (float)r12; Q.m[0][2] = (float)r13; /* load Q */
  Q.m[1][0] = (float)r21; Q.m[1][1] = (float)r22; Q.m[1][2] = (float)r23;
  Q.m[2][0] = (float)r31; Q.m[2][1] = (float)r32; Q.m[2][2] = (float)r33;

  P = nifti_mat33_polar(Q);  /* P is orthog matrix closest to Q */

  r11 = P.m[0][0]; r12 = P.m[0][1]; r13 = P.m[0][2]; /* unload */
  r21 = P.m[1][0]; r22 = P.m[1][1]; r23 = P.m[1][2];
  r31 = P.m[2][0]; r32 = P.m[2][1]; r33 = P.m[2][2];

  /*                            [ r11 r12 r13 ]               */
  /* at this point, the matrix  [ r21 r22 r23 ] is orthogonal */
  /*                            [ r31 r32 r33 ]               */

  /* compute the determinant to determine if it is proper */

  zd = r11 * r22*r33 - r11 * r32*r23 - r21 * r12*r33
    + r21 * r32*r13 + r31 * r12*r23 - r31 * r22*r13;  /* should be -1 or 1 */

  if (zd > 0) {             /* proper */
    ASSIF(qfac, 1.0f);
  }
  else {                  /* improper ==> flip 3rd column */
    ASSIF(qfac, -1.0f);
    r13 = -r13; r23 = -r23; r33 = -r33;
  }

  /* now, compute quaternion parameters */

  a = r11 + r22 + r33 + 1.0l;

  if (a > 0.5l) {                /* simplest case */
    a = 0.5l * sqrt(a);
    b = 0.25l * (r32 - r23) / a;
    c = 0.25l * (r13 - r31) / a;
    d = 0.25l * (r21 - r12) / a;
  }
  else {                       /* trickier case */
    xd = 1.0 + r11 - (r22 + r33);  /* 4*b*b */
    yd = 1.0 + r22 - (r11 + r33);  /* 4*c*c */
    zd = 1.0 + r33 - (r11 + r22);  /* 4*d*d */
    if (xd > 1.0) {
      b = 0.5l * sqrt(xd);
      c = 0.25l* (r12 + r21) / b;
      d = 0.25l* (r13 + r31) / b;
      a = 0.25l* (r32 - r23) / b;
    }
    else if (yd > 1.0) {
      c = 0.5l * sqrt(yd);
      b = 0.25l* (r12 + r21) / c;
      d = 0.25l* (r23 + r32) / c;
      a = 0.25l* (r13 - r31) / c;
    }
    else {
      d = 0.5l * sqrt(zd);
      b = 0.25l* (r13 + r31) / d;
      c = 0.25l* (r23 + r32) / d;
      a = 0.25l* (r21 - r12) / d;
    }
    /* to be mathematically consistent, this would include a = -a */
    if (a < 0.0l) { b = -b; c = -c; d = -d; }
  }

  ASSIF(qb, (float)b); ASSIF(qc, (float)c); ASSIF(qd, (float)d);
  return;
}

/*---------------------------------------------------------------------------*/
/*! Compute the inverse of a bordered 4x4 matrix.

     <pre>
   - Some numerical code fragments were generated by Maple 8.
   - If a singular matrix is input, the output matrix will be all zero.
   - You can check for this by examining the [3][3] element, which will
     be 1.0 for the normal case and 0.0 for the bad case.

     The input matrix should have the form:
        [ r11 r12 r13 v1 ]
        [ r21 r22 r23 v2 ]
        [ r31 r32 r33 v3 ]
        [  0   0   0   1 ]
     </pre>
*//*-------------------------------------------------------------------------*/
mdm_NiftiFormat::nifti_dmat44 mdm_NiftiFormat::nifti_dmat44_inverse(nifti_dmat44 R)
{
  double r11, r12, r13, r21, r22, r23, r31, r32, r33, v1, v2, v3, deti;
  nifti_dmat44 Q;
  /*  INPUT MATRIX IS:  */
  r11 = R.m[0][0]; r12 = R.m[0][1]; r13 = R.m[0][2];  /* [ r11 r12 r13 v1 ] */
  r21 = R.m[1][0]; r22 = R.m[1][1]; r23 = R.m[1][2];  /* [ r21 r22 r23 v2 ] */
  r31 = R.m[2][0]; r32 = R.m[2][1]; r33 = R.m[2][2];  /* [ r31 r32 r33 v3 ] */
  v1 = R.m[0][3]; v2 = R.m[1][3]; v3 = R.m[2][3];  /* [  0   0   0   1 ] */

  deti = r11 * r22*r33 - r11 * r32*r23 - r21 * r12*r33
    + r21 * r32*r13 + r31 * r12*r23 - r31 * r22*r13;

  if (deti != 0.0l) deti = 1.0l / deti;

  Q.m[0][0] = deti * (r22*r33 - r32 * r23);
  Q.m[0][1] = deti * (-r12 * r33 + r32 * r13);
  Q.m[0][2] = deti * (r12*r23 - r22 * r13);
  Q.m[0][3] = deti * (-r12 * r23*v3 + r12 * v2*r33 + r22 * r13*v3
    - r22 * v1*r33 - r32 * r13*v2 + r32 * v1*r23);

  Q.m[1][0] = deti * (-r21 * r33 + r31 * r23);
  Q.m[1][1] = deti * (r11*r33 - r31 * r13);
  Q.m[1][2] = deti * (-r11 * r23 + r21 * r13);
  Q.m[1][3] = deti * (r11*r23*v3 - r11 * v2*r33 - r21 * r13*v3
    + r21 * v1*r33 + r31 * r13*v2 - r31 * v1*r23);

  Q.m[2][0] = deti * (r21*r32 - r31 * r22);
  Q.m[2][1] = deti * (-r11 * r32 + r31 * r12);
  Q.m[2][2] = deti * (r11*r22 - r21 * r12);
  Q.m[2][3] = deti * (-r11 * r22*v3 + r11 * r32*v2 + r21 * r12*v3
    - r21 * r32*v1 - r31 * r12*v2 + r31 * r22*v1);

  Q.m[3][0] = Q.m[3][1] = Q.m[3][2] = 0.0l;
  Q.m[3][3] = (deti == 0.0l) ? 0.0l : 1.0l; /* failure flag if deti == 0 */

  return Q;
}

/*---------------------------------------------------------------------------*/
/*! Compute the inverse of a bordered 4x4 matrix.

     <pre>
   - Some numerical code fragments were generated by Maple 8.
   - If a singular matrix is input, the output matrix will be all zero.
   - You can check for this by examining the [3][3] element, which will
     be 1.0 for the normal case and 0.0 for the bad case.

     The input matrix should have the form:
        [ r11 r12 r13 v1 ]
        [ r21 r22 r23 v2 ]
        [ r31 r32 r33 v3 ]
        [  0   0   0   1 ]
     </pre>
*//*-------------------------------------------------------------------------*/
mdm_NiftiFormat::mat44 mdm_NiftiFormat::nifti_mat44_inverse(mat44 R)
{
  double r11, r12, r13, r21, r22, r23, r31, r32, r33, v1, v2, v3, deti;
  mat44 Q;
  /*  INPUT MATRIX IS:  */
  r11 = R.m[0][0]; r12 = R.m[0][1]; r13 = R.m[0][2];  /* [ r11 r12 r13 v1 ] */
  r21 = R.m[1][0]; r22 = R.m[1][1]; r23 = R.m[1][2];  /* [ r21 r22 r23 v2 ] */
  r31 = R.m[2][0]; r32 = R.m[2][1]; r33 = R.m[2][2];  /* [ r31 r32 r33 v3 ] */
  v1 = R.m[0][3]; v2 = R.m[1][3]; v3 = R.m[2][3];  /* [  0   0   0   1 ] */

  deti = r11 * r22*r33 - r11 * r32*r23 - r21 * r12*r33
    + r21 * r32*r13 + r31 * r12*r23 - r31 * r22*r13;

  if (deti != 0.0l) deti = 1.0l / deti;

  Q.m[0][0] = (float)(deti*(r22*r33 - r32 * r23));
  Q.m[0][1] = (float)(deti*(-r12 * r33 + r32 * r13));
  Q.m[0][2] = (float)(deti*(r12*r23 - r22 * r13));
  Q.m[0][3] = (float)(deti*(-r12 * r23*v3 + r12 * v2*r33 + r22 * r13*v3
    - r22 * v1*r33 - r32 * r13*v2 + r32 * v1*r23));

  Q.m[1][0] = (float)(deti*(-r21 * r33 + r31 * r23));
  Q.m[1][1] = (float)(deti*(r11*r33 - r31 * r13));
  Q.m[1][2] = (float)(deti*(-r11 * r23 + r21 * r13));
  Q.m[1][3] = (float)(deti*(r11*r23*v3 - r11 * v2*r33 - r21 * r13*v3
    + r21 * v1*r33 + r31 * r13*v2 - r31 * v1*r23));

  Q.m[2][0] = (float)(deti*(r21*r32 - r31 * r22));
  Q.m[2][1] = (float)(deti*(-r11 * r32 + r31 * r12));
  Q.m[2][2] = (float)(deti*(r11*r22 - r21 * r12));
  Q.m[2][3] = (float)(deti*(-r11 * r22*v3 + r11 * r32*v2 + r21 * r12*v3
    - r21 * r32*v1 - r31 * r12*v2 + r31 * r22*v1));

  Q.m[3][0] = Q.m[3][1] = Q.m[3][2] = 0.0l;
  Q.m[3][3] = (float)((deti == 0.0l) ? 0.0l : 1.0l); /* failure flag if deti == 0 */

  return Q;
}

/*---------------------------------------------------------------------------*/
/*! Input 9 floats and make an orthgonal nifti_dmat44 out of them.

   Each row is normalized, then nifti_mat33_polar() is used to orthogonalize
   them.  If row #3 (r31,r32,r33) is input as zero, then it will be taken to
   be the cross product of rows #1 and #2.

   This function can be used to create a rotation matrix for transforming
   an oblique volume to anatomical coordinates.  For this application:
    - row #1 (r11,r12,r13) is the direction vector along the image i-axis
    - row #2 (r21,r22,r23) is the direction vector along the image j-axis
    - row #3 (r31,r32,r33) is the direction vector along the slice direction
      (if available; otherwise enter it as 0's)

   The first 2 rows can be taken from the DICOM attribute (0020,0037)
   "Image Orientation (Patient)".

   After forming the rotation matrix, the complete affine transformation from
   (i,j,k) grid indexes to (x,y,z) spatial coordinates can be computed by
   multiplying each column by the appropriate grid spacing:
    - column #1 (R.m[0][0],R.m[1][0],R.m[2][0]) by delta-x
    - column #2 (R.m[0][1],R.m[1][1],R.m[2][1]) by delta-y
    - column #3 (R.m[0][2],R.m[1][2],R.m[2][2]) by delta-z

   and by then placing the center (x,y,z) coordinates of voxel (0,0,0) into
   the column #4 (R.m[0][3],R.m[1][3],R.m[2][3]).

   \sa nifti_quatern_to_dmat44, nifti_dmat44_to_quatern,
       nifti_dmat44_to_orientation
*//*-------------------------------------------------------------------------*/
mdm_NiftiFormat::nifti_dmat44 mdm_NiftiFormat::nifti_make_orthog_dmat44(double r11, double r12, double r13,
  double r21, double r22, double r23,
  double r31, double r32, double r33)
{
  nifti_dmat44 R;
  nifti_dmat33 Q, P;
  double val;

  R.m[3][0] = R.m[3][1] = R.m[3][2] = 0.0l; R.m[3][3] = 1.0l;

  Q.m[0][0] = r11; Q.m[0][1] = r12; Q.m[0][2] = r13; /* load Q */
  Q.m[1][0] = r21; Q.m[1][1] = r22; Q.m[1][2] = r23;
  Q.m[2][0] = r31; Q.m[2][1] = r32; Q.m[2][2] = r33;

  /* normalize row 1 */

  val = Q.m[0][0] * Q.m[0][0] + Q.m[0][1] * Q.m[0][1] + Q.m[0][2] * Q.m[0][2];
  if (val > 0.0l) {
    val = 1.0l / sqrt(val);
    Q.m[0][0] *= val; Q.m[0][1] *= val; Q.m[0][2] *= val;
  }
  else {
    Q.m[0][0] = 1.0l; Q.m[0][1] = 0.0l; Q.m[0][2] = 0.0l;
  }

  /* normalize row 2 */

  val = Q.m[1][0] * Q.m[1][0] + Q.m[1][1] * Q.m[1][1] + Q.m[1][2] * Q.m[1][2];
  if (val > 0.0l) {
    val = 1.0l / sqrt(val);
    Q.m[1][0] *= val; Q.m[1][1] *= val; Q.m[1][2] *= val;
  }
  else {
    Q.m[1][0] = 0.0l; Q.m[1][1] = 1.0l; Q.m[1][2] = 0.0l;
  }

  /* normalize row 3 */

  val = Q.m[2][0] * Q.m[2][0] + Q.m[2][1] * Q.m[2][1] + Q.m[2][2] * Q.m[2][2];
  if (val > 0.0l) {
    val = 1.0l / sqrt(val);
    Q.m[2][0] *= val; Q.m[2][1] *= val; Q.m[2][2] *= val;
  }
  else {
    Q.m[2][0] = Q.m[0][1] * Q.m[1][2] - Q.m[0][2] * Q.m[1][1];  /* cross */
    Q.m[2][1] = Q.m[0][2] * Q.m[1][0] - Q.m[0][0] * Q.m[1][2];  /* product */
    Q.m[2][2] = Q.m[0][0] * Q.m[1][1] - Q.m[0][1] * Q.m[1][0];
  }

  P = nifti_dmat33_polar(Q);  /* P is orthog matrix closest to Q */

  R.m[0][0] = P.m[0][0]; R.m[0][1] = P.m[0][1]; R.m[0][2] = P.m[0][2];
  R.m[1][0] = P.m[1][0]; R.m[1][1] = P.m[1][1]; R.m[1][2] = P.m[1][2];
  R.m[2][0] = P.m[2][0]; R.m[2][1] = P.m[2][1]; R.m[2][2] = P.m[2][2];

  R.m[0][3] = R.m[1][3] = R.m[2][3] = 0.0f; return R;
}

/*---------------------------------------------------------------------------*/
/*! Input 9 floats and make an orthgonal mat44 out of them.

   Each row is normalized, then nifti_mat33_polar() is used to orthogonalize
   them.  If row #3 (r31,r32,r33) is input as zero, then it will be taken to
   be the cross product of rows #1 and #2.

   This function can be used to create a rotation matrix for transforming
   an oblique volume to anatomical coordinates.  For this application:
    - row #1 (r11,r12,r13) is the direction vector along the image i-axis
    - row #2 (r21,r22,r23) is the direction vector along the image j-axis
    - row #3 (r31,r32,r33) is the direction vector along the slice direction
      (if available; otherwise enter it as 0's)

   The first 2 rows can be taken from the DICOM attribute (0020,0037)
   "Image Orientation (Patient)".

   After forming the rotation matrix, the complete affine transformation from
   (i,j,k) grid indexes to (x,y,z) spatial coordinates can be computed by
   multiplying each column by the appropriate grid spacing:
    - column #1 (R.m[0][0],R.m[1][0],R.m[2][0]) by delta-x
    - column #2 (R.m[0][1],R.m[1][1],R.m[2][1]) by delta-y
    - column #3 (R.m[0][2],R.m[1][2],R.m[2][2]) by delta-z

   and by then placing the center (x,y,z) coordinates of voxel (0,0,0) into
   the column #4 (R.m[0][3],R.m[1][3],R.m[2][3]).

   \sa nifti_quatern_to_mat44, nifti_mat44_to_quatern,
       nifti_mat44_to_orientation
*//*-------------------------------------------------------------------------*/
mdm_NiftiFormat::mat44 mdm_NiftiFormat::nifti_make_orthog_mat44(float r11, float r12, float r13,
  float r21, float r22, float r23,
  float r31, float r32, float r33)
{
  mat44 R;
  mat33 Q, P;
  double val;

  R.m[3][0] = R.m[3][1] = R.m[3][2] = 0.0l; R.m[3][3] = 1.0l;

  Q.m[0][0] = r11; Q.m[0][1] = r12; Q.m[0][2] = r13; /* load Q */
  Q.m[1][0] = r21; Q.m[1][1] = r22; Q.m[1][2] = r23;
  Q.m[2][0] = r31; Q.m[2][1] = r32; Q.m[2][2] = r33;

  /* normalize row 1 */

  val = Q.m[0][0] * Q.m[0][0] + Q.m[0][1] * Q.m[0][1] + Q.m[0][2] * Q.m[0][2];
  if (val > 0.0l) {
    val = 1.0l / sqrt(val);
    Q.m[0][0] *= (float)val; Q.m[0][1] *= (float)val; Q.m[0][2] *= (float)val;
  }
  else {
    Q.m[0][0] = 1.0l; Q.m[0][1] = 0.0l; Q.m[0][2] = 0.0l;
  }

  /* normalize row 2 */

  val = Q.m[1][0] * Q.m[1][0] + Q.m[1][1] * Q.m[1][1] + Q.m[1][2] * Q.m[1][2];
  if (val > 0.0l) {
    val = 1.0l / sqrt(val);
    Q.m[1][0] *= (float)val; Q.m[1][1] *= (float)val; Q.m[1][2] *= (float)val;
  }
  else {
    Q.m[1][0] = 0.0l; Q.m[1][1] = 1.0l; Q.m[1][2] = 0.0l;
  }

  /* normalize row 3 */

  val = Q.m[2][0] * Q.m[2][0] + Q.m[2][1] * Q.m[2][1] + Q.m[2][2] * Q.m[2][2];
  if (val > 0.0l) {
    val = 1.0l / sqrt(val);
    Q.m[2][0] *= (float)val; Q.m[2][1] *= (float)val; Q.m[2][2] *= (float)val;
  }
  else {
    Q.m[2][0] = Q.m[0][1] * Q.m[1][2] - Q.m[0][2] * Q.m[1][1];  /* cross */
    Q.m[2][1] = Q.m[0][2] * Q.m[1][0] - Q.m[0][0] * Q.m[1][2];  /* product */
    Q.m[2][2] = Q.m[0][0] * Q.m[1][1] - Q.m[0][1] * Q.m[1][0];
  }

  P = nifti_mat33_polar(Q);  /* P is orthog matrix closest to Q */

  R.m[0][0] = P.m[0][0]; R.m[0][1] = P.m[0][1]; R.m[0][2] = P.m[0][2];
  R.m[1][0] = P.m[1][0]; R.m[1][1] = P.m[1][1]; R.m[1][2] = P.m[1][2];
  R.m[2][0] = P.m[2][0]; R.m[2][1] = P.m[2][1]; R.m[2][2] = P.m[2][2];

  R.m[0][3] = R.m[1][3] = R.m[2][3] = 0.0f; return R;
}

/*---------------------------------------------------------------------------*/
/*! compute the (closest) orientation from a 4x4 ijk->xyz tranformation matrix

   <pre>
   Input:  4x4 matrix that transforms (i,j,k) indexes to (x,y,z) coordinates,
           where +x=Right, +y=Anterior, +z=Superior.
           (Only the upper-left 3x3 corner of R is used herein.)
   Output: 3 orientation codes that correspond to the closest "standard"
           anatomical orientation of the (i,j,k) axes.
   Method: Find which permutation of (x,y,z) has the smallest angle to the
           (i,j,k) axes directions, which are the columns of the R matrix.
   Errors: The codes returned will be zero.

   For example, an axial volume might get return values of
     *icod = NIFTI_ORIENTATION::R2L   (i axis is mostly Right to Left)
     *jcod = NIFTI_ORIENTATION::P2A   (j axis is mostly Posterior to Anterior)
     *kcod = NIFTI_ORIENTATION::I2S   (k axis is mostly Inferior to Superior)
   </pre>

   \see "QUATERNION REPRESENTATION OF ROTATION MATRIX" in nifti1.h

   \see nifti_quatern_to_mat44, nifti_mat44_to_quatern,
        nifti_make_orthog_mat44
*//*-------------------------------------------------------------------------*/
void mdm_NiftiFormat::nifti_dmat44_to_orientation(nifti_dmat44 R,
  int *icod, int *jcod, int *kcod)
{
  double xi, xj, xk, yi, yj, yk, zi, zj, zk, val, detQ, detP;
  nifti_dmat33 P, Q, M;
  int i, j, k = 0, p, q, r, ibest, jbest, kbest, pbest, qbest, rbest;
  double vbest;

  if (icod == NULL || jcod == NULL || kcod == NULL) return; /* bad */

  *icod = *jcod = *kcod = 0; /* error returns, if sh*t happens */

  /* load column vectors for each (i,j,k) direction from matrix */

  /*-- i axis --*/ /*-- j axis --*/ /*-- k axis --*/

  xi = R.m[0][0]; xj = R.m[0][1]; xk = R.m[0][2];
  yi = R.m[1][0]; yj = R.m[1][1]; yk = R.m[1][2];
  zi = R.m[2][0]; zj = R.m[2][1]; zk = R.m[2][2];

  /* normalize column vectors to get unit vectors along each ijk-axis */

  /* normalize i axis */

  val = sqrt(xi*xi + yi * yi + zi * zi);
  if (val == 0.0) return;                 /* stupid input */
  xi /= val; yi /= val; zi /= val;

  /* normalize j axis */

  val = sqrt(xj*xj + yj * yj + zj * zj);
  if (val == 0.0) return;                 /* stupid input */
  xj /= val; yj /= val; zj /= val;

  /* orthogonalize j axis to i axis, if needed */

  val = xi * xj + yi * yj + zi * zj;    /* dot product between i and j */
  if (fabs(val) > 1.e-4) {
    xj -= val * xi; yj -= val * yi; zj -= val * zi;
    val = sqrt(xj*xj + yj * yj + zj * zj);  /* must renormalize */
    if (val == 0.0) return;              /* j was parallel to i? */
    xj /= val; yj /= val; zj /= val;
  }

  /* normalize k axis; if it is zero, make it the cross product i x j */

  val = sqrt(xk*xk + yk * yk + zk * zk);
  if (val == 0.0) { xk = yi * zj - zi * yj; yk = zi * xj - zj * xi; zk = xi * yj - yi * xj; }
  else { xk /= val; yk /= val; zk /= val; }

  /* orthogonalize k to i */

  val = xi * xk + yi * yk + zi * zk;    /* dot product between i and k */
  if (fabs(val) > 1.e-4) {
    xk -= val * xi; yk -= val * yi; zk -= val * zi;
    val = sqrt(xk*xk + yk * yk + zk * zk);
    if (val == 0.0) return;      /* bad */
    xk /= val; yk /= val; zk /= val;
  }

  /* orthogonalize k to j */

  val = xj * xk + yj * yk + zj * zk;    /* dot product between j and k */
  if (fabs(val) > 1.e-4) {
    xk -= val * xj; yk -= val * yj; zk -= val * zj;
    val = sqrt(xk*xk + yk * yk + zk * zk);
    if (val == 0.0) return;      /* bad */
    xk /= val; yk /= val; zk /= val;
  }

  Q.m[0][0] = xi; Q.m[0][1] = xj; Q.m[0][2] = xk;
  Q.m[1][0] = yi; Q.m[1][1] = yj; Q.m[1][2] = yk;
  Q.m[2][0] = zi; Q.m[2][1] = zj; Q.m[2][2] = zk;

  /* at this point, Q is the rotation matrix from (i,j,k) to (x,y,z) axes */

  detQ = nifti_dmat33_determ(Q);
  if (detQ == 0.0) return; /* shouldn't happen unless user is a DUFIS */

  /* Build and test all possible +1/-1 coordinate permutation matrices P;
     then find the P such that the rotation matrix M=PQ is closest to the
     identity, in the sense of M having the smallest total rotation angle. */

     /* Despite the formidable looking 6 nested loops, there are
        only 3*3*3*2*2*2 = 216 passes, which will run very quickly. */

  vbest = -666.0; ibest = pbest = qbest = rbest = 1; jbest = 2; kbest = 3;
  for (i = 1; i <= 3; i++) {     /* i = column number to use for row #1 */
    for (j = 1; j <= 3; j++) {    /* j = column number to use for row #2 */
      if (i == j) continue;
      for (k = 1; k <= 3; k++) {  /* k = column number to use for row #3 */
        if (i == k || j == k) continue;
        P.m[0][0] = P.m[0][1] = P.m[0][2] =
          P.m[1][0] = P.m[1][1] = P.m[1][2] =
          P.m[2][0] = P.m[2][1] = P.m[2][2] = 0.0;
        for (p = -1; p <= 1; p += 2) {    /* p,q,r are -1 or +1      */
          for (q = -1; q <= 1; q += 2) {   /* and go into rows #1,2,3 */
            for (r = -1; r <= 1; r += 2) {
              P.m[0][i - 1] = p; P.m[1][j - 1] = q; P.m[2][k - 1] = r;
              detP = nifti_dmat33_determ(P);          /* sign of permutation */
              if (detP * detQ <= 0.0) continue;  /* doesn't match sign of Q */
              M = nifti_dmat33_mul(P, Q);

              /* angle of M rotation = 2.0*acos(0.5*sqrt(1.0+trace(M)))       */
              /* we want largest trace(M) == smallest angle == M nearest to I */

              val = M.m[0][0] + M.m[1][1] + M.m[2][2]; /* trace */
              if (val > vbest) {
                vbest = val;
                ibest = i; jbest = j; kbest = k;
                pbest = p; qbest = q; rbest = r;
              }
            }
          }
        }
      }
    }
  }

  /* At this point ibest is 1 or 2 or 3; pbest is -1 or +1; etc.

     The matrix P that corresponds is the best permutation approximation
     to Q-inverse; that is, P (approximately) takes (x,y,z) coordinates
     to the (i,j,k) axes.

     For example, the first row of P (which contains pbest in column ibest)
     determines the way the i axis points relative to the anatomical
     (x,y,z) axes.  If ibest is 2, then the i axis is along the y axis,
     which is direction P2A (if pbest > 0) or A2P (if pbest < 0).

     So, using ibest and pbest, we can assign the output code for
     the i axis.  Mutatis mutandis for the j and k axes, of course. */

  switch (ibest*pbest) {
  case  1: i = NIFTI_ORIENTATION::L2R; break;
  case -1: i = NIFTI_ORIENTATION::R2L; break;
  case  2: i = NIFTI_ORIENTATION::P2A; break;
  case -2: i = NIFTI_ORIENTATION::A2P; break;
  case  3: i = NIFTI_ORIENTATION::I2S; break;
  case -3: i = NIFTI_ORIENTATION::S2I; break;
  }

  switch (jbest*qbest) {
  case  1: j = NIFTI_ORIENTATION::L2R; break;
  case -1: j = NIFTI_ORIENTATION::R2L; break;
  case  2: j = NIFTI_ORIENTATION::P2A; break;
  case -2: j = NIFTI_ORIENTATION::A2P; break;
  case  3: j = NIFTI_ORIENTATION::I2S; break;
  case -3: j = NIFTI_ORIENTATION::S2I; break;
  }

  switch (kbest*rbest) {
  case  1: k = NIFTI_ORIENTATION::L2R; break;
  case -1: k = NIFTI_ORIENTATION::R2L; break;
  case  2: k = NIFTI_ORIENTATION::P2A; break;
  case -2: k = NIFTI_ORIENTATION::A2P; break;
  case  3: k = NIFTI_ORIENTATION::I2S; break;
  case -3: k = NIFTI_ORIENTATION::S2I; break;
  }

  *icod = i; *jcod = j; *kcod = k; return;
}

/*---------------------------------------------------------------------------*/
/*! compute the (closest) orientation from a 4x4 ijk->xyz tranformation matrix

   <pre>
   Input:  4x4 matrix that transforms (i,j,k) indexes to (x,y,z) coordinates,
           where +x=Right, +y=Anterior, +z=Superior.
           (Only the upper-left 3x3 corner of R is used herein.)
   Output: 3 orientation codes that correspond to the closest "standard"
           anatomical orientation of the (i,j,k) axes.
   Method: Find which permutation of (x,y,z) has the smallest angle to the
           (i,j,k) axes directions, which are the columns of the R matrix.
   Errors: The codes returned will be zero.

   For example, an axial volume might get return values of
     *icod = NIFTI_ORIENTATION::R2L   (i axis is mostly Right to Left)
     *jcod = NIFTI_ORIENTATION::P2A   (j axis is mostly Posterior to Anterior)
     *kcod = NIFTI_ORIENTATION::I2S   (k axis is mostly Inferior to Superior)
   </pre>

   \see "QUATERNION REPRESENTATION OF ROTATION MATRIX" in nifti1.h

   \see nifti_quatern_to_mat44, nifti_mat44_to_quatern,
        nifti_make_orthog_mat44
*//*-------------------------------------------------------------------------*/
void mdm_NiftiFormat::nifti_mat44_to_orientation(mat44 R, int *icod, int *jcod, int *kcod)
{
  float xi, xj, xk, yi, yj, yk, zi, zj, zk, val, detQ, detP;
  mat33 P, Q, M;
  int i, j, k = 0, p, q, r, ibest, jbest, kbest, pbest, qbest, rbest;
  float vbest;

  if (icod == NULL || jcod == NULL || kcod == NULL) return; /* bad */

  *icod = *jcod = *kcod = 0; /* error returns, if sh*t happens */

  /* load column vectors for each (i,j,k) direction from matrix */

  /*-- i axis --*/ /*-- j axis --*/ /*-- k axis --*/

  xi = R.m[0][0]; xj = R.m[0][1]; xk = R.m[0][2];
  yi = R.m[1][0]; yj = R.m[1][1]; yk = R.m[1][2];
  zi = R.m[2][0]; zj = R.m[2][1]; zk = R.m[2][2];

  /* normalize column vectors to get unit vectors along each ijk-axis */

  /* normalize i axis */

  val = (float)sqrt(xi*xi + yi * yi + zi * zi);
  if (val == 0.0) return;                 /* stupid input */
  xi /= val; yi /= val; zi /= val;

  /* normalize j axis */

  val = (float)sqrt(xj*xj + yj * yj + zj * zj);
  if (val == 0.0) return;                 /* stupid input */
  xj /= val; yj /= val; zj /= val;

  /* orthogonalize j axis to i axis, if needed */

  val = xi * xj + yi * yj + zi * zj;    /* dot product between i and j */
  if (fabs(val) > 1.e-4) {
    xj -= val * xi; yj -= val * yi; zj -= val * zi;
    val = (float)sqrt(xj*xj + yj * yj + zj * zj);  /* must renormalize */
    if (val == 0.0) return;              /* j was parallel to i? */
    xj /= val; yj /= val; zj /= val;
  }

  /* normalize k axis; if it is zero, make it the cross product i x j */

  val = (float)sqrt(xk*xk + yk * yk + zk * zk);
  if (val == 0.0) { xk = yi * zj - zi * yj; yk = zi * xj - zj * xi; zk = xi * yj - yi * xj; }
  else { xk /= val; yk /= val; zk /= val; }

  /* orthogonalize k to i */

  val = xi * xk + yi * yk + zi * zk;    /* dot product between i and k */
  if (fabs(val) > 1.e-4) {
    xk -= val * xi; yk -= val * yi; zk -= val * zi;
    val = (float)sqrt(xk*xk + yk * yk + zk * zk);
    if (val == 0.0) return;      /* bad */
    xk /= val; yk /= val; zk /= val;
  }

  /* orthogonalize k to j */

  val = xj * xk + yj * yk + zj * zk;    /* dot product between j and k */
  if (fabs(val) > 1.e-4) {
    xk -= val * xj; yk -= val * yj; zk -= val * zj;
    val = (float)sqrt(xk*xk + yk * yk + zk * zk);
    if (val == 0.0) return;      /* bad */
    xk /= val; yk /= val; zk /= val;
  }

  Q.m[0][0] = xi; Q.m[0][1] = xj; Q.m[0][2] = xk;
  Q.m[1][0] = yi; Q.m[1][1] = yj; Q.m[1][2] = yk;
  Q.m[2][0] = zi; Q.m[2][1] = zj; Q.m[2][2] = zk;

  /* at this point, Q is the rotation matrix from the (i,j,k) to (x,y,z) axes */

  detQ = nifti_mat33_determ(Q);
  if (detQ == 0.0) return; /* shouldn't happen unless user is a DUFIS */

  /* Build and test all possible +1/-1 coordinate permutation matrices P;
     then find the P such that the rotation matrix M=PQ is closest to the
     identity, in the sense of M having the smallest total rotation angle. */

     /* Despite the formidable looking 6 nested loops, there are
        only 3*3*3*2*2*2 = 216 passes, which will run very quickly. */

  vbest = -666.0f; ibest = pbest = qbest = rbest = 1; jbest = 2; kbest = 3;
  for (i = 1; i <= 3; i++) {     /* i = column number to use for row #1 */
    for (j = 1; j <= 3; j++) {    /* j = column number to use for row #2 */
      if (i == j) continue;
      for (k = 1; k <= 3; k++) {  /* k = column number to use for row #3 */
        if (i == k || j == k) continue;
        P.m[0][0] = P.m[0][1] = P.m[0][2] =
          P.m[1][0] = P.m[1][1] = P.m[1][2] =
          P.m[2][0] = P.m[2][1] = P.m[2][2] = 0.0f;
        for (p = -1; p <= 1; p += 2) {    /* p,q,r are -1 or +1      */
          for (q = -1; q <= 1; q += 2) {   /* and go into rows #1,2,3 */
            for (r = -1; r <= 1; r += 2) {
              P.m[0][i - 1] = (float)p;
              P.m[1][j - 1] = (float)q;
              P.m[2][k - 1] = (float)r;
              detP = nifti_mat33_determ(P);           /* sign of permutation */
              if (detP * detQ <= 0.0) continue;  /* doesn't match sign of Q */
              M = nifti_mat33_mul(P, Q);

              /* angle of M rotation = 2.0*acos(0.5*sqrt(1.0+trace(M)))       */
              /* we want largest trace(M) == smallest angle == M nearest to I */

              val = M.m[0][0] + M.m[1][1] + M.m[2][2]; /* trace */
              if (val > vbest) {
                vbest = val;
                ibest = i; jbest = j; kbest = k;
                pbest = p; qbest = q; rbest = r;
              }
            }
          }
        }
      }
    }
  }

  /* At this point ibest is 1 or 2 or 3; pbest is -1 or +1; etc.

     The matrix P that corresponds is the best permutation approximation
     to Q-inverse; that is, P (approximately) takes (x,y,z) coordinates
     to the (i,j,k) axes.

     For example, the first row of P (which contains pbest in column ibest)
     determines the way the i axis points relative to the anatomical
     (x,y,z) axes.  If ibest is 2, then the i axis is along the y axis,
     which is direction P2A (if pbest > 0) or A2P (if pbest < 0).

     So, using ibest and pbest, we can assign the output code for
     the i axis.  Mutatis mutandis for the j and k axes, of course. */

  switch (ibest*pbest) {
  case  1: i = NIFTI_ORIENTATION::L2R; break;
  case -1: i = NIFTI_ORIENTATION::R2L; break;
  case  2: i = NIFTI_ORIENTATION::P2A; break;
  case -2: i = NIFTI_ORIENTATION::A2P; break;
  case  3: i = NIFTI_ORIENTATION::I2S; break;
  case -3: i = NIFTI_ORIENTATION::S2I; break;
  }

  switch (jbest*qbest) {
  case  1: j = NIFTI_ORIENTATION::L2R; break;
  case -1: j = NIFTI_ORIENTATION::R2L; break;
  case  2: j = NIFTI_ORIENTATION::P2A; break;
  case -2: j = NIFTI_ORIENTATION::A2P; break;
  case  3: j = NIFTI_ORIENTATION::I2S; break;
  case -3: j = NIFTI_ORIENTATION::S2I; break;
  }

  switch (kbest*rbest) {
  case  1: k = NIFTI_ORIENTATION::L2R; break;
  case -1: k = NIFTI_ORIENTATION::R2L; break;
  case  2: k = NIFTI_ORIENTATION::P2A; break;
  case -2: k = NIFTI_ORIENTATION::A2P; break;
  case  3: k = NIFTI_ORIENTATION::I2S; break;
  case -3: k = NIFTI_ORIENTATION::S2I; break;
  }

  *icod = i; *jcod = j; *kcod = k; return;
}
