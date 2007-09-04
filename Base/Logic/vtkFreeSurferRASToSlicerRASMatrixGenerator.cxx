#include "vtkFreeSurferRASToSlicerRASMatrixGenerator.h"
#include "vtkObjectFactory.h"
#include "vtkMRMLVolumeNode.h"


vtkCxxRevisionMacro ( vtkFreeSurferRASToSlicerRASMatrixGenerator, "$Revision: 1.0 $");
vtkStandardNewMacro (vtkFreeSurferRASToSlicerRASMatrixGenerator);

//-------------------------------------------------------------------------
vtkFreeSurferRASToSlicerRASMatrixGenerator::vtkFreeSurferRASToSlicerRASMatrixGenerator()
{
  this->RAS2RASMatrix = NULL;
}

//-------------------------------------------------------------------------
vtkFreeSurferRASToSlicerRASMatrixGenerator::~vtkFreeSurferRASToSlicerRASMatrixGenerator()
{
  if ( this->RAS2RASMatrix )
    {
    this->RAS2RASMatrix->Delete();
    this->RAS2RASMatrix = NULL;
    }
  
}



//-------------------------------------------------------------------------
void vtkFreeSurferRASToSlicerRASMatrixGenerator::ComputeTkRegVox2RASMatrix ( vtkMRMLVolumeNode *VNode,
                                                                       vtkMatrix4x4 *M )
{
    double dC, dS, dR;
    double Nc, Ns, Nr;
    int dim[3];

    VNode->GetImageData()->GetDimensions(dim);
    Nc = dim[0];
    Nr = dim[1];
    Ns = dim[2];

    double *spacing = VNode->GetSpacing();
    dC = spacing[0];
    dR = spacing[1];
    dS = spacing[2];

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
void vtkFreeSurferRASToSlicerRASMatrixGenerator::ComputeOrientationMatrix ( vtkMRMLVolumeNode *vnode,
                                                                            vtkMatrix4x4 *OrientMat)
{

  vtkMatrix4x4 *IJKToRAS = vtkMatrix4x4::New();
  vnode->GetIJKToRASMatrix ( IJKToRAS );
  const char *order = vnode->ComputeScanOrderFromIJKToRAS( IJKToRAS );

  if (order == NULL) 
    {
    return;
    }

  if (!strcmp(order,"IS") ||
      !strcmp(order,"Axial IS") ||
      !strcmp(order,  "Axial"))
    {

    // rotate x +90
    double elems[] = { 1,  0,  0,  0,
                        0, 0,  1,  0, 
                        0,  -1, 0,  0,
                        0,  0,  0,  1};   
    OrientMat->DeepCopy(elems);
    }
  else if (!strcmp(order,"SI") ||
           !strcmp(order,"Axial SI"))
    {
    // rotate x -90
    double elems[] = { 1,  0,  0,  0,
                        0, 0,  -1,  0, 
                        0,  1,  0,  0,
                        0,  0,  0,  1};   
    OrientMat->DeepCopy(elems);
    }
  else if (!strcmp(order,"RL") ||
           !strcmp(order,"Sagittal RL") ||
           !strcmp(order,  "Sagittal"))
    {

    // rotate z +90
    double elems[] = {  0,  1,  0,  0,
                       -1,  0,  0,  0, 
                        0, 0,  1,  0,
                        0,  0,  0,  1};   
    OrientMat->DeepCopy(elems);
    }
  else if (!strcmp(order,"LR") ||
      !strcmp(order,"Sagittal LR") )
    {
    // rotate z -90
    double elems[] = {  0,  -1, 0,  0,
                       1,  0,  0,  0, 
                        0,  0,  1,  0,
                        0,  0,  0,  1};   
    OrientMat->DeepCopy(elems);
    }
  else if (!strcmp(order,"PA") ||
      !strcmp(order,"Coronal PA") ||
      !strcmp(order,  "Coronal"))
    {
    // identity matrix
    double elems[] = { 1,  0,  0,  0,
                        0,  1, 0,  0, 
                        0, 0,  1,  0,
                        0,  0,  0,  1};   
    OrientMat->DeepCopy(elems);
    }
  else if (!strcmp(order,"AP") ||
      !strcmp(order,"Coronal AP") )
    {
    // mirror in AP
    double elems[] = { 0,  0,  0,  0,
                        0,  -1,  0,  0, 
                        0, 0,  1,  0,
                        0,  0,  0,  1};   
    OrientMat->DeepCopy(elems);
    }

  IJKToRAS->Delete();
}


//-------------------------------------------------------------------------
void vtkFreeSurferRASToSlicerRASMatrixGenerator::ComputeRAS2RASMatrixWithOrientation( vtkMRMLVolumeNode *V1Node,
                                                        vtkMRMLVolumeNode *V2Node,
                                                        vtkMatrix4x4 *FSRegistrationMatrix )
{
  if  ( V1Node  && V2Node && FSRegistrationMatrix )
    {

    if ( this->RAS2RASMatrix == NULL )
      {
      this->RAS2RASMatrix = vtkMatrix4x4::New();
      }

  
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
    // RASv1_To_RASv2 = [Ninv]  [Sinv]  [R]  [T]  [Minv]
    //
    // I think we need orientation in FreeSurfer: nothing in the tkRegVox2RAS
    // handles scanOrder. The tkRegVox2RAS = IJKToRAS matrix for a coronal
    // volume. But for an Axial volume, these two matrices are different.
    // How do we compute the correct orientation for FreeSurfer Data?
  
    vtkMatrix4x4 *T = vtkMatrix4x4::New();
    vtkMatrix4x4 *S = vtkMatrix4x4::New();
    vtkMatrix4x4 *Sinv = vtkMatrix4x4::New();
    vtkMatrix4x4 *M = vtkMatrix4x4::New();
    vtkMatrix4x4 *Minv = vtkMatrix4x4::New();
    vtkMatrix4x4 *N = vtkMatrix4x4::New();
    vtkMatrix4x4 *Ninv = vtkMatrix4x4::New();
    vtkMatrix4x4 *O = vtkMatrix4x4::New();
    vtkMatrix4x4 *Oinv = vtkMatrix4x4::New();

    //--
    // compute FreeSurfer tkRegVox2RAS for V1 volume
    //--
    ComputeTkRegVox2RASMatrix ( V1Node, T );

    //--
    // compute FreeSurfer tkRegVox2RAS for V2 volume
    //--
    ComputeTkRegVox2RASMatrix ( V2Node, S );

    // Probably a faster way to do these things?
    vtkMatrix4x4::Invert (S, Sinv );
    V1Node->GetIJKToRASMatrix( M );
    V2Node->GetRASToIJKMatrix( N );
    this->ComputeOrientationMatrix ( V2Node, O );
    vtkMatrix4x4::Invert (O, Oinv );
    vtkMatrix4x4::Invert (M, Minv );
    vtkMatrix4x4::Invert (N, Ninv );

   //    [Ninv]  [Sinv]  [R]  [T]  [Minv]
    vtkMatrix4x4 *tmp = vtkMatrix4x4::New();
    vtkMatrix4x4::Multiply4x4 ( Ninv, Sinv, tmp );
    vtkMatrix4x4::Multiply4x4 ( tmp, Oinv, tmp );
    vtkMatrix4x4::Multiply4x4 ( tmp, FSRegistrationMatrix, tmp );
    vtkMatrix4x4::Multiply4x4 ( tmp, O, tmp );
    vtkMatrix4x4::Multiply4x4 ( tmp, T, tmp );
    vtkMatrix4x4::Multiply4x4 ( tmp, Minv, tmp );
    
    this->RAS2RASMatrix->DeepCopy(tmp);
  
    // clean up
    tmp->Delete();
    M->Delete();
    N->Delete();
    Minv->Delete();
    Ninv->Delete();
    S->Delete();
    T->Delete();
    Sinv->Delete();
    O->Delete();
    Oinv->Delete();
    }
}


//-------------------------------------------------------------------------
void vtkFreeSurferRASToSlicerRASMatrixGenerator::ComputeRAS2RASMatrix( vtkMRMLVolumeNode *V1Node,
                                                        vtkMRMLVolumeNode *V2Node,
                                                        vtkMatrix4x4 *FSRegistrationMatrix )
{
  if  ( V1Node  && V2Node && FSRegistrationMatrix )
    {

    if ( this->RAS2RASMatrix == NULL )
      {
      this->RAS2RASMatrix = vtkMatrix4x4::New();
      }

  
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
    // RASv1_To_RASv2 = [Ninv]  [Sinv]  [R]  [T]  [Minv]
    //
    // I think we need orientation in FreeSurfer: nothing in the tkRegVox2RAS
    // handles scanOrder. The tkRegVox2RAS = IJKToRAS matrix for a coronal
    // volume. But for an Axial volume, these two matrices are different.
    // How do we compute the correct orientation for FreeSurfer Data?
  
    vtkMatrix4x4 *T = vtkMatrix4x4::New();
    vtkMatrix4x4 *S = vtkMatrix4x4::New();
    vtkMatrix4x4 *Sinv = vtkMatrix4x4::New();
    vtkMatrix4x4 *M = vtkMatrix4x4::New();
    vtkMatrix4x4 *Minv = vtkMatrix4x4::New();
    vtkMatrix4x4 *N = vtkMatrix4x4::New();
    vtkMatrix4x4 *Ninv = vtkMatrix4x4::New();

    //--
    // compute FreeSurfer tkRegVox2RAS for V1 volume
    //--
    ComputeTkRegVox2RASMatrix ( V1Node, T );

    //--
    // compute FreeSurfer tkRegVox2RAS for V2 volume
    //--
    ComputeTkRegVox2RASMatrix ( V2Node, S );

    // Probably a faster way to do these things?
    vtkMatrix4x4::Invert (S, Sinv );
    V1Node->GetIJKToRASMatrix( M );
    V2Node->GetRASToIJKMatrix( N );
    vtkMatrix4x4::Invert (M, Minv );
    vtkMatrix4x4::Invert (N, Ninv );

   //    [Ninv]  [Sinv]  [R]  [T]  [Minv]
    vtkMatrix4x4 *tmp = vtkMatrix4x4::New();
    vtkMatrix4x4::Multiply4x4 ( Ninv, Sinv, tmp );
    vtkMatrix4x4::Multiply4x4 ( tmp, FSRegistrationMatrix, tmp );
    vtkMatrix4x4::Multiply4x4 ( tmp, T, tmp );
    vtkMatrix4x4::Multiply4x4 ( tmp, Minv, tmp );
    
    this->RAS2RASMatrix->DeepCopy(tmp);
  
    // clean up
    tmp->Delete();
    M->Delete();
    N->Delete();
    Minv->Delete();
    Ninv->Delete();
    S->Delete();
    T->Delete();
    Sinv->Delete();
    }
}



//-------------------------------------------------------------------------
void vtkFreeSurferRASToSlicerRASMatrixGenerator::PrintSelf(ostream& os, vtkIndent indent)
{
    Superclass::PrintSelf(os, indent);

    os << indent << "RAS2RASMatrix: " << this->RAS2RASMatrix << endl;

}
