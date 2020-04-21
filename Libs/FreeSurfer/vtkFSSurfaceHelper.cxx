/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkFSSurfaceHelper.h,v $
  Date:      $Date: 2006/05/26 19:40:14 $
  Version:   $Revision: 1.9 $

=========================================================================auto=*/
#include "vtkFSSurfaceHelper.h"

#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>

//-------------------------------------------------------------------------
//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkFSSurfaceHelper);

//-------------------------------------------------------------------------
vtkFSSurfaceHelper::vtkFSSurfaceHelper() = default;

//-------------------------------------------------------------------------
void vtkFSSurfaceHelper::ComputeTkRegVox2RASMatrix ( double* spacing,
                                                     int* dim,
                                                     vtkMatrix4x4 *M )
{
  double dC, dS, dR;
  double Nc, Ns, Nr;

  if (!M)
    {
    return;
    }
  dC = spacing[0];
  dR = spacing[1];
  dS = spacing[2];

  Nc = dim[0] * dC;
  Nr = dim[1] * dR;
  Ns = dim[2] * dS;

  M->Zero();
  M->SetElement ( 0, 0, -dC );
  M->SetElement ( 0, 3, Nc/2.0 );
  M->SetElement ( 1, 2, dS );
  M->SetElement ( 1, 3, -Ns/2.0 );
  M->SetElement ( 2, 1, -dR );
  M->SetElement ( 2, 3, Nr/2.0 );
  M->SetElement ( 3, 3, 1.0 );
}

//-------------------------------------------------------------------------
void vtkFSSurfaceHelper
::TranslateFreeSurferRegistrationMatrixIntoSlicerRASToRASMatrix(
  double* V1Spacing, int* V1Dim, vtkMatrix4x4* V1IJKToRASMatrix,
  double* V2Spacing, int* V2Dim, vtkMatrix4x4* V2RASToIJKMatrix,
  vtkMatrix4x4 *FSRegistrationMatrix, vtkMatrix4x4 *RAS2RASMatrix)
{
  if (!V1Spacing || !V1Dim || !V1IJKToRASMatrix ||
      !V2Spacing || !V2Dim || !V2RASToIJKMatrix ||
      !FSRegistrationMatrix || !RAS2RASMatrix)
    {
    return;
    }

  RAS2RASMatrix->Zero();

  //
  // Looking for RASv1_To_RASv2:
  //
  //---
  //
  // In Slicer:
  // [ IJKv1_To_IJKv2] = [ RAS_To_IJKv2 ]  [ RASv1_To_RASv2 ] [ IJK_To_RASv1 ] [i,j,k]transpose
  //
  // In FreeSurfer:
  // [ IJKv1_To_IJKv2] = [FStkRegVox_To_RASv2 ]inverse [ FSRegistrationMatrix] [FStkRegVox_To_RASv1 ] [ i,j,k] transpose
  //
  //----
  //
  // So:
  // [FStkRegVox_To_RASv2 ] inverse [ FSRegistrationMatrix] [FStkRegVox_To_RASv1 ] =
  // [ RAS_To_IJKv2 ]  [ RASv1_To_RASv2 ] [ IJKv1_2_RAS ]
  //
  //---
  //
  // Below use this shorthand:
  //
  // S = FStkRegVox_To_RASv2
  // T = FStkRegVox_To_RASv1
  // N = RAS_To_IJKv2
  // M = IJK_To_RASv1
  // R = FSRegistrationMatrix
  // [Sinv]  [R]  [T] = [N]  [RASv1_To_RASv2]  [M];
  //
  // So this is what we'll compute and use in Slicer instead
  // of the FreeSurfer register.dat matrix:
  //
  // [Ninv]  [Sinv]  [R]  [T]  [Minv]  = RASv1_To_RASv2
  //
  // I think we need orientation in FreeSurfer: nothing in the tkRegVox2RAS
  // handles scanOrder. The tkRegVox2RAS = IJKToRAS matrix for a coronal
  // volume. But for an Axial volume, these two matrices are different.
  // How do we compute the correct orientation for FreeSurfer Data?

  vtkMatrix4x4 *T = vtkMatrix4x4::New();
  vtkMatrix4x4 *S = vtkMatrix4x4::New();
  vtkMatrix4x4 *Sinv = vtkMatrix4x4::New();
  vtkMatrix4x4 *M = V1IJKToRASMatrix;
  vtkMatrix4x4 *Minv = vtkMatrix4x4::New();
  vtkMatrix4x4 *N = V2RASToIJKMatrix;
  vtkMatrix4x4 *Ninv = vtkMatrix4x4::New();

  //--
  // compute FreeSurfer tkRegVox2RAS for V1 volume
  //--
  ComputeTkRegVox2RASMatrix ( V1Spacing, V1Dim, T );

  //--
  // compute FreeSurfer tkRegVox2RAS for V2 volume
  //--
  ComputeTkRegVox2RASMatrix ( V2Spacing, V2Dim, S );

  // Probably a faster way to do these things?
  vtkMatrix4x4::Invert (S, Sinv );
  vtkMatrix4x4::Invert (M, Minv );
  vtkMatrix4x4::Invert (N, Ninv );

  //    [Ninv]  [Sinv]  [R]  [T]  [Minv]
  vtkMatrix4x4::Multiply4x4 ( T, Minv, RAS2RASMatrix );
  vtkMatrix4x4::Multiply4x4 ( FSRegistrationMatrix, RAS2RASMatrix, RAS2RASMatrix );
  vtkMatrix4x4::Multiply4x4 ( Sinv, RAS2RASMatrix, RAS2RASMatrix );
  vtkMatrix4x4::Multiply4x4 ( Ninv, RAS2RASMatrix, RAS2RASMatrix );

  // clean up
  Minv->Delete();
  Ninv->Delete();
  S->Delete();
  T->Delete();
  Sinv->Delete();
}
