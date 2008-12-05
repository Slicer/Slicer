/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: itkMimxImageToVtkUnstructuredGridFilter.txx,v $
Language:  C++
Date:      $Date: 2007/05/17 17:09:46 $
Version:   $Revision: 1.2 $

 Musculoskeletal Imaging, Modelling and Experimentation (MIMX)
 Center for Computer Aided Design
 The University of Iowa
 Iowa City, IA 52242
 http://www.ccad.uiowa.edu/mimx
 
Copyright (c) Insight Software Consortium. All rights reserved.
See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even 
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef _itkMimxImageToVtkUnstructuredGridFilter_txx
#define _itkMimxImageToVtkUnstructuredGridFilter_txx


#include <itkConstNeighborhoodIterator.h>
#include <itkContinuousIndex.h>
#include <itkNeighborhood.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkUnsignedLongArray.h>
#include <vtkDoubleArray.h>
#include <vtkFieldData.h>
#include <vtkPointData.h>

#include "itkMimxImageToVtkUnstructuredGridFilter.h"


namespace itk
{

template<class TInputImage, class TMaskImage>
MimxImageToVtkUnstructuredGridFilter<TInputImage, TMaskImage>
::MimxImageToVtkUnstructuredGridFilter( )
{
  // Modify superclass default values, can be overridden by subclasses
  this->SetNumberOfRequiredInputs(1);

  m_ImageThreshold = 0;
  m_MeshIndexOffset = 0;
  m_ComputeMeshNodeNumbering = false;
  m_UseCtProperties = true;
  m_ComputeMeshProperties = false;
  m_VoxelCountThreshold = 1;
  m_Precision = 0.0001;
  m_Output = vtkUnstructuredGrid::New();

}

/** Set the Input Image */

template<class TInputImage, class TMaskImage>
void
MimxImageToVtkUnstructuredGridFilter<TInputImage,TMaskImage>
::SetInput(const InputImageType* inputImage)
{ 
  this->ProcessObject::
                SetNthInput(0, const_cast< InputImageType * >( inputImage ) );
}
  
/** Set the Mask */

template<class TInputImage, class TMaskImage>
void
MimxImageToVtkUnstructuredGridFilter<TInputImage,TMaskImage>
::SetMaskImage(const MaskImageType* maskImage)
{ 
  this->ProcessObject::
                  SetNthInput(1, const_cast< MaskImageType * >( maskImage ) );
}


template<class TInputImage, class TMaskImage>
const TInputImage*
MimxImageToVtkUnstructuredGridFilter<TInputImage,TMaskImage>
::GetInput( ) const
{ 
  if (this->GetNumberOfInputs( ) < 1)
    {
    return 0;
    }
   
  return 
    static_cast<const InputImageType *> (this->ProcessObject::GetNthInput(0));
}


template<class TInputImage, class TMaskImage>
const TMaskImage*
MimxImageToVtkUnstructuredGridFilter<TInputImage,TMaskImage>
::GetMaskImage( ) const
{ 
  if (this->GetNumberOfInputs( ) < 2)
    {
    return 0;
    }
    
  return static_cast<const MaskImageType *> 
                            (this->ProcessObject::GetNthInput(1));
}

/** To allow the calculation of Material Property ( Modulus value ) */

template<class TInputImage, class TMaskImage>
void
MimxImageToVtkUnstructuredGridFilter<TInputImage,TMaskImage>
::SetComputeMeshPropertiesOn( )
{
  m_ComputeMeshProperties = true;
}

/** To turn of the calculation of Material Property ( Modulus value ) */

template<class TInputImage, class TMaskImage>
void
MimxImageToVtkUnstructuredGridFilter<TInputImage,TMaskImage>
::SetComputeMeshPropertiesOff( )
{
  m_ComputeMeshProperties = false;
}

template<class TInputImage, class TMaskImage>
void
MimxImageToVtkUnstructuredGridFilter<TInputImage,TMaskImage>
::SetUseCtPropertiesOn( )
{
  m_UseCtProperties = true;
}


template<class TInputImage, class TMaskImage>
void
MimxImageToVtkUnstructuredGridFilter<TInputImage,TMaskImage>
::SetUseCtPropertiesOff( )
{
  m_UseCtProperties = false;
}


template<class TInputImage, class TMaskImage>
void
MimxImageToVtkUnstructuredGridFilter<TInputImage,TMaskImage>
::SetComputeMeshNodeNumberingOn( )
{
  m_ComputeMeshNodeNumbering = true;
}


template<class TInputImage, class TMaskImage>
void
MimxImageToVtkUnstructuredGridFilter<TInputImage,TMaskImage>
::SetComputeMeshNodeNumberingOff( )
{
  m_ComputeMeshNodeNumbering = false;
}


/** Set the user specified cell trait function for material property */

template<class TInputImage, class TMaskImage>
void 
MimxImageToVtkUnstructuredGridFilter<TInputImage,TMaskImage>
::SetCellTraitFunction( double (*CellTraitFunction)( InputPixelType ))
{
  m_MaterialPropertyFunction = CellTraitFunction;
}

/** Calculate the Material Property ( Elastic Modulus ) value
 *  of the voxel depending on the CT value of the voxel
 */

template<class TInputImage, class TMaskImage>
double
MimxImageToVtkUnstructuredGridFilter<TInputImage,TMaskImage>
::CalculateMaterialProps( InputPixelType ctValue )
{ 
  double voxelDensity;
  double waterDensity = 1.0;
  double strainRate = 0.01;
  double constantB = 3790.0;
  double constantC = 3.0;
  double constantD = 0.06;
  double modulus;
  
  voxelDensity = 
          ( waterDensity*static_cast<double>(ctValue) )/1000.0 + waterDensity;
  modulus = constantB * pow( strainRate, constantD ) * 
                        pow( voxelDensity, constantC );
  return modulus;
}


/* Create the Mesh */
template<class TInputImage, class TMaskImage>
void
MimxImageToVtkUnstructuredGridFilter<TInputImage,TMaskImage>
::Update( )
{  
  InputImageConstPointer inputImage = static_cast<const InputImageType * >
                                         (this->ProcessObject::GetInput(0) );
  MaskImageConstPointer maskImage = NULL;
  if ( this->GetNumberOfInputs( ) > 1)
    {
      maskImage = static_cast<const MaskImageType * >
                                          (this->ProcessObject::GetInput(1) );
    }
  
  vtkPoints *nodalPoints = vtkPoints::New();
  vtkCellArray *hexCellArray = vtkCellArray::New();
  vtkUnsignedLongArray *nodeIndexArray = vtkUnsignedLongArray::New();
  nodeIndexArray->SetName( "Node_Numbers" );
  vtkUnsignedLongArray *elementIndexArray = vtkUnsignedLongArray::New();
  elementIndexArray->SetName( "Element_Numbers" );
  vtkDoubleArray *materialPropertyArray = vtkDoubleArray::New();
  materialPropertyArray->SetName( "Material_Properties");
  m_Output->Initialize( );
  

  // support progress methods/callbacks
  unsigned long updateVisits = 0;
  unsigned long totalPixels = 0;
  unsigned long offset = m_MeshResampleSize / 2;
  unsigned long scaleFactor = m_MeshResampleSize * 
                              m_MeshResampleSize * 
                              m_MeshResampleSize;
  
  //std::cerr << "Offset: " << m_MeshResampleSize << std::endl;
  //std::cerr << "Scale Factor: " << scaleFactor << std::endl;
  
  totalPixels = inputImage->GetRequestedRegion().GetNumberOfPixels();
  updateVisits = totalPixels / 10 / scaleFactor;
  if( updateVisits < 1 ) updateVisits = 1;
  

  InputImageSizeType imageSize = inputImage->GetLargestPossibleRegion( ).GetSize();
  

  typedef typename itk::ConstNeighborhoodIterator<InputImageType> IteratorType;
  typename IteratorType::RadiusType radius;
  radius.Fill( offset );
  IteratorType it( radius, inputImage, inputImage->GetLargestPossibleRegion( ) );
  it.NeedToUseBoundaryConditionOff( );
  it.GoToBegin();


  unsigned long iterations = 0;
  

  while( !it.IsAtEnd() )
   {

   if ( !(iterations % updateVisits ) )
     {
     this->UpdateProgress((float)iterations/(float)totalPixels);
     //std::cerr << "Progress: " << (float)iterations/(float)totalPixels << std::endl;
     }

   InputIndexType imageIndex = it.GetIndex( );
   if ( ( imageIndex[0] % m_MeshResampleSize == offset ) &&
        ( imageIndex[1] % m_MeshResampleSize == offset ) &&
        ( imageIndex[2] % m_MeshResampleSize == offset ) &&
        ( imageIndex[0] < imageSize[0] - offset ) &&
        ( imageIndex[1] < imageSize[1] - offset ) &&
        ( imageIndex[2] < imageSize[2] - offset ))
     {

     /* A voxel count threshold could be applied here - 
      * Grosland did this previously to eliminate cells when 
      * only a small fraction of the underlying voxels were
      * occupied.
      */
     InputPixelType voxelData = 0;
     unsigned long numberOfRegionVoxels = 0;

     /* Fix Neighborhood Iterators */
     /*   TransformPhysicalPointToContinuousIndex  */
     if ( maskImage )
       {
       typename IteratorType::NeighborhoodType imageNeighborhood = it.GetNeighborhood( );
        
       for (unsigned long  i=0; i<imageNeighborhood.Size(); i++)
         {
         InputIndexType imageIndex = it.GetIndex( i );
         InputOriginType imagePosition;
         inputImage->TransformIndexToPhysicalPoint(imageIndex, imagePosition);
         MaskIndexType maskIndex;
         maskImage->TransformPhysicalPointToIndex(imagePosition, maskIndex);
         if ( maskImage->GetPixel( maskIndex ) > 0 )
           {
           voxelData += imageNeighborhood.GetElement( i );
           numberOfRegionVoxels++;
           }
         }
       }
     else
       {
       /* If no mask - Check that the image is 
                       above the threshold in the current region */
       typename IteratorType::NeighborhoodType imageNeighborhood = it.GetNeighborhood( );
       for (unsigned long  i=0; i<imageNeighborhood.Size(); i++)
         {
         if ( imageNeighborhood.GetElement( i ) >= m_ImageThreshold )
           {
           voxelData += imageNeighborhood.GetElement( i );
           numberOfRegionVoxels++;
           }
         }
       }
       
     /* Add a cell if it meets the Voxel Count Threshold */
     if ( numberOfRegionVoxels >= m_VoxelCountThreshold )
       {
       itk::ContinuousIndex<double, 3> tmpIndex;
       InputOriginType nodePosition;
                
       /* Set the First Nodal Position */
       InputIndexType imageIndex1;
       imageIndex1[0] = imageIndex[0] - offset;
       imageIndex1[1] = imageIndex[1] - offset;
       imageIndex1[2] = imageIndex[2] + offset;
       tmpIndex[0] = static_cast<double>(imageIndex1[0]) - 0.5; 
       tmpIndex[1] = static_cast<double>(imageIndex1[1]) - 0.5; 
       tmpIndex[2] = static_cast<double>(imageIndex1[2]) + 0.5;
       inputImage->TransformContinuousIndexToPhysicalPoint(tmpIndex, nodePosition);
       vtkFloatingPointType node1[3];
       node1[0] = nodePosition[0]; node1[1] = nodePosition[1];  node1[2] = nodePosition[2];

       /* Set the Second Nodal Position */
       InputIndexType imageIndex2;
       imageIndex2[0] = imageIndex[0] - offset;
       imageIndex2[1] = imageIndex[1] + offset;
       imageIndex2[2] = imageIndex[2] + offset;
       tmpIndex[0] = static_cast<double>(imageIndex2[0]) - 0.5; 
       tmpIndex[1] = static_cast<double>(imageIndex2[1]) + 0.5; 
       tmpIndex[2] = static_cast<double>(imageIndex2[2]) + 0.5;
       inputImage->TransformContinuousIndexToPhysicalPoint(tmpIndex, nodePosition);
       vtkFloatingPointType node2[3];
       node2[0] = nodePosition[0]; node2[1] = nodePosition[1];  node2[2] = nodePosition[2];

       /* Set the Third Nodal Position */
       InputIndexType imageIndex3;
       imageIndex3[0] = imageIndex[0] + offset;
       imageIndex3[1] = imageIndex[1] + offset;
       imageIndex3[2] = imageIndex[2] + offset;
       tmpIndex[0] = static_cast<double>(imageIndex3[0]) + 0.5; 
       tmpIndex[1] = static_cast<double>(imageIndex3[1]) + 0.5; 
       tmpIndex[2] = static_cast<double>(imageIndex3[2]) + 0.5;
       inputImage->TransformContinuousIndexToPhysicalPoint(tmpIndex, nodePosition);
       vtkFloatingPointType node3[3];
       node3[0] = nodePosition[0]; node3[1] = nodePosition[1];  node3[2] = nodePosition[2];

       /* Set the Fourth Nodal Position */
       InputIndexType imageIndex4;
       imageIndex4[0] = imageIndex[0] + offset;
       imageIndex4[1] = imageIndex[1] - offset;
       imageIndex4[2] = imageIndex[2] + offset;
       tmpIndex[0] = static_cast<double>(imageIndex4[0]) + 0.5; 
       tmpIndex[1] = static_cast<double>(imageIndex4[1]) - 0.5; 
       tmpIndex[2] = static_cast<double>(imageIndex4[2]) + 0.5;
       inputImage->TransformContinuousIndexToPhysicalPoint(tmpIndex, nodePosition);
       vtkFloatingPointType node4[3];
       node4[0] = nodePosition[0]; node4[1] = nodePosition[1];  node4[2] = nodePosition[2];
          
       /* Set the Fith Nodal Position */
       InputIndexType imageIndex5;
       imageIndex5[0] = imageIndex[0] - offset;
       imageIndex5[1] = imageIndex[1] - offset;
       imageIndex5[2] = imageIndex[2] - offset;
       tmpIndex[0] = static_cast<double>(imageIndex5[0]) - 0.5; 
       tmpIndex[1] = static_cast<double>(imageIndex5[1]) - 0.5; 
       tmpIndex[2] = static_cast<double>(imageIndex5[2]) - 0.5;
       inputImage->TransformContinuousIndexToPhysicalPoint(tmpIndex, nodePosition);
       vtkFloatingPointType node5[3];
       node5[0] = nodePosition[0]; node5[1] = nodePosition[1];  node5[2] = nodePosition[2];
   
       /* Set the Sixth Nodal Position */
       InputIndexType imageIndex6;
       imageIndex6[0] = imageIndex[0] - offset;
       imageIndex6[1] = imageIndex[1] + offset;
       imageIndex6[2] = imageIndex[2] - offset;
       tmpIndex[0] = static_cast<double>(imageIndex6[0]) - 0.5; 
       tmpIndex[1] = static_cast<double>(imageIndex6[1]) + 0.5; 
       tmpIndex[2] = static_cast<double>(imageIndex6[2]) - 0.5;
       inputImage->TransformContinuousIndexToPhysicalPoint(tmpIndex, nodePosition);
       vtkFloatingPointType node6[3];
       node6[0] = nodePosition[0]; node6[1] = nodePosition[1];  node6[2] = nodePosition[2];
          
       /* Set the Seventh Nodal Position */
       InputIndexType imageIndex7;
       imageIndex7[0] = imageIndex[0] + offset;
       imageIndex7[1] = imageIndex[1] + offset;
       imageIndex7[2] = imageIndex[2] - offset;
       tmpIndex[0] = static_cast<double>(imageIndex7[0]) + 0.5; 
       tmpIndex[1] = static_cast<double>(imageIndex7[1]) + 0.5; 
       tmpIndex[2] = static_cast<double>(imageIndex7[2]) - 0.5;
       inputImage->TransformContinuousIndexToPhysicalPoint(tmpIndex, nodePosition);
       vtkFloatingPointType node7[3];
       node7[0] = nodePosition[0]; node7[1] = nodePosition[1];  node7[2] = nodePosition[2];

       /* Set the Eighth Nodal Position */
       InputIndexType imageIndex8;
       imageIndex8[0] = imageIndex[0] + offset;
       imageIndex8[1] = imageIndex[1] - offset;
       imageIndex8[2] = imageIndex[2] - offset;
       tmpIndex[0] = static_cast<double>(imageIndex8[0]) + 0.5; 
       tmpIndex[1] = static_cast<double>(imageIndex8[1]) - 0.5; 
       tmpIndex[2] = static_cast<double>(imageIndex8[2]) - 0.5;
       inputImage->TransformContinuousIndexToPhysicalPoint(tmpIndex, nodePosition);
       vtkFloatingPointType node8[3];
       node8[0] = nodePosition[0]; node8[1] = nodePosition[1];  node8[2] = nodePosition[2];

       /* Add the Points, Cell and return Cell Index */
       vtkIdType cellPointIds[8];
       cellPointIds[0] = AddPoint(nodalPoints, node1, nodeIndexArray, imageIndex1, imageSize);
       cellPointIds[1] = AddPoint(nodalPoints, node2, nodeIndexArray, imageIndex2, imageSize);
       cellPointIds[2] = AddPoint(nodalPoints, node3, nodeIndexArray, imageIndex3, imageSize);
       cellPointIds[3] = AddPoint(nodalPoints, node4, nodeIndexArray, imageIndex4, imageSize);
       cellPointIds[4] = AddPoint(nodalPoints, node5, nodeIndexArray, imageIndex5, imageSize);
       cellPointIds[5] = AddPoint(nodalPoints, node6, nodeIndexArray, imageIndex6, imageSize);
       cellPointIds[6] = AddPoint(nodalPoints, node7, nodeIndexArray, imageIndex7, imageSize);
       cellPointIds[7] = AddPoint(nodalPoints, node8, nodeIndexArray, imageIndex8, imageSize);

       vtkIdType hexIndex = 
           hexCellArray->InsertNextCell(8, cellPointIds);
       
       // Set the Cell properties if requested by the user. The default
       // method for assigning the material property values is conversion
       // of image data into an elastic modulus value. It uses a standard
       // calculation appropriate for CT data. The user can also specify a
       // a user defined function for the conversion.
       if ( m_ComputeMeshProperties )
         {
         double matProp;
         InputPixelType meanVoxelData = voxelData / 
             static_cast<InputPixelType>(numberOfRegionVoxels);

         if ( m_UseCtProperties )
           {
           matProp = CalculateMaterialProps( meanVoxelData );
           }
         else
           {
           matProp = m_MaterialPropertyFunction( meanVoxelData );
           }
         materialPropertyArray->InsertValue( hexIndex, matProp );
         }
       elementIndexArray->InsertValue( hexIndex, nodeIndexArray->GetValue( cellPointIds[0] ) );
       }
     }
   ++it;
   //if ( maskImage ) ++maskIt;
   iterations++;
   }

   vtkFieldData *gridFields = vtkFieldData::New();
   m_Output->SetPoints( nodalPoints );
   m_Output->GetPointData( )->AddArray( nodeIndexArray );
   m_Output->SetCells( VTK_HEXAHEDRON, hexCellArray );
   if ( m_ComputeMeshProperties )
     {
     gridFields->AddArray( materialPropertyArray );
     }
   gridFields->AddArray( elementIndexArray );
   m_Output->SetFieldData( gridFields );
   m_Output->BuildLinks();
  
}


/** AddPoint - adds a point to the points if it does not 
  * already exist within the point set array 
  */ 
template<class TInputImage, class TMaskImage>
vtkIdType
MimxImageToVtkUnstructuredGridFilter<TInputImage,TMaskImage>
  ::AddPoint( vtkPoints *nodalPoints, const vtkFloatingPointType newPoint[3], 
              vtkUnsignedLongArray *nodeIndexArray, const InputIndexType imageIndex,
              const InputImageSizeType imageSize)
{
  vtkIdType numberOfPoints = nodalPoints->GetNumberOfPoints ( );
  vtkIdType index = numberOfPoints;
  
  //std::cout << "# Points: " << numberOfPoints << std::endl;
  
  for (vtkIdType id=0; id < numberOfPoints; id++)
    {
    vtkFloatingPointType currentPoint[3];
    nodalPoints->GetPoint(id, currentPoint);
    
    if ( (fabs (static_cast<double> ( currentPoint[0]-newPoint[0] ) ) < m_Precision) && 
         (fabs (static_cast<double> ( currentPoint[1]-newPoint[1] ) ) < m_Precision) &&
         (fabs (static_cast<double> ( currentPoint[2]-newPoint[2] ) ) < m_Precision) )
      {
      index = id;
      //std::cout << "found duplicate point: " << id << std::endl;
      break;
      }
    }
  
  if ( index == numberOfPoints )
    {
    //Add the Point
    nodalPoints->InsertPoint(index, newPoint);
    
    //Set the Point Data - Point Index Mapping
    // Two methods for numbering exists
    //     1. Same as point indexes
    //     2. Voxel Based numbering 
    unsigned long pointIndex;
    if ( m_ComputeMeshNodeNumbering )
      {
      pointIndex = imageIndex[2]*imageSize[1]*imageSize[0] + 
                   imageIndex[1]*imageSize[0] +
                   imageIndex[0] + m_MeshIndexOffset;
      }
    else
      {
      pointIndex = (unsigned long) ( index + m_MeshIndexOffset );
      }
    nodeIndexArray->InsertValue(index, pointIndex);
    }
  
  return index;
}


/** PrintSelf */
template<class TInputImage, class TMaskImage>
void
MimxImageToVtkUnstructuredGridFilter<TInputImage,TMaskImage>
::PrintSelf( std::ostream& os, Indent indent ) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "Compute Mesh Properties: " 
      << m_ComputeMeshProperties << std::endl;
  os << indent << "Use CT Properties: "
      << m_UseCtProperties;
  os << indent << "Compute Mesh Node Numering: "
      << m_ComputeMeshNodeNumbering;
  os << indent << "Mesh Index Offset: "
      << m_MeshIndexOffset;
  os << indent << "Mesh Resample Size: "
      << m_MeshResampleSize;
  os << indent << "Image Threshold: "
      << m_ImageThreshold; 
  os << indent << "Voxel Count Threshold: "
      << m_VoxelCountThreshold;
  os << indent << "Output Unstructured Grid: ";
  m_Output->Print( os );
}

} /** end namespace itk. */

#endif
