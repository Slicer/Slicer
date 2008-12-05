/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: itkMimxImageToHexahedronMeshFilter.txx,v $
Language:  C++
Date:      $Date: 2007/05/10 16:32:38 $
Version:   $Revision: 1.8 $

 Musculoskeletal Imaging, Modelling and Experimentation (MIMX)
 Center for Computer Aided Design
 The University of Iowa
 Iowa City, IA 52242
 http://www.ccad.uiowa.edu/mimx/
 
Copyright (c) The University of Iowa. All rights reserved.
See MIMXCopyright.txt or http://www.ccad.uiowa.edu/mimx/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even 
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef _itkImageToHexahedronMeshFilter_txx
#define _itkImageToHexahedronMeshFilter_txx


#include "itkImageToHexahedronMeshFilter.h"
#include "itkNumericTraits.h"

namespace itk
{

template<class TInputImage, class TMaskImage, class TOutputMesh>
ImageToHexahedronMeshFilter<TInputImage, TMaskImage, TOutputMesh>
::ImageToHexahedronMeshFilter( )
{
  // Modify superclass default values, can be overridden by subclasses
  this->SetNumberOfRequiredInputs(1);

  m_ImageThreshold = 0;
  m_MeshIndexOffset = 0;
  m_ComputeMeshNodeNumbering = false;
  m_UseCtProperties = true;
  m_ComputeMeshProperties = false;

  meshSource = MeshSourceType::New( );
}

/** Set the Input Image */

template<class TInputImage, class TMaskImage, class TOutputMesh>
void
ImageToHexahedronMeshFilter<TInputImage,TMaskImage,TOutputMesh>
::SetInput(const InputImageType* inputImage)
{ 
  this->ProcessObject::
                SetNthInput(0, const_cast< InputImageType * >( inputImage ) );
}
  
/** Set the Mask */

template<class TInputImage, class TMaskImage, class TOutputMesh>
void
ImageToHexahedronMeshFilter<TInputImage,TMaskImage,TOutputMesh>
::SetMaskImage(const MaskImageType* maskImage)
{ 
  this->ProcessObject::
                  SetNthInput(1, const_cast< MaskImageType * >( maskImage ) );
}


template<class TInputImage, class TMaskImage, class TOutputMesh>
const TInputImage*
ImageToHexahedronMeshFilter<TInputImage,TMaskImage,TOutputMesh>
::GetInput( ) const
{ 
  if (this->GetNumberOfInputs( ) < 1)
    {
    return 0;
    }
   
  return 
    static_cast<const InputImageType *> (this->ProcessObject::GetNthInput(0));
}


template<class TInputImage, class TMaskImage, class TOutputMesh>
const TMaskImage*
ImageToHexahedronMeshFilter<TInputImage,TMaskImage,TOutputMesh>
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

template<class TInputImage, class TMaskImage, class TOutputMesh>
void
ImageToHexahedronMeshFilter<TInputImage,TMaskImage,TOutputMesh>
::SetComputeMeshPropertiesOn( )
{
  m_ComputeMeshProperties = true;
}

/** To turn of the calculation of Material Property ( Modulus value ) */

template<class TInputImage, class TMaskImage, class TOutputMesh>
void
ImageToHexahedronMeshFilter<TInputImage,TMaskImage,TOutputMesh>
::SetComputeMeshPropertiesOff( )
{
  m_ComputeMeshProperties = false;
}

template<class TInputImage, class TMaskImage, class TOutputMesh>
void
ImageToHexahedronMeshFilter<TInputImage,TMaskImage,TOutputMesh>
::SetUseCtPropertiesOn( )
{
  m_UseCtProperties = true;
}


template<class TInputImage, class TMaskImage, class TOutputMesh>
void
ImageToHexahedronMeshFilter<TInputImage,TMaskImage,TOutputMesh>
::SetUseCtPropertiesOff( )
{
  m_UseCtProperties = false;
}


template<class TInputImage, class TMaskImage, class TOutputMesh>
void
ImageToHexahedronMeshFilter<TInputImage,TMaskImage,TOutputMesh>
::SetComputeMeshNodeNumberingOn( )
{
  m_ComputeMeshNodeNumbering = true;
}


template<class TInputImage, class TMaskImage, class TOutputMesh>
void
ImageToHexahedronMeshFilter<TInputImage,TMaskImage,TOutputMesh>
::SetComputeMeshNodeNumberingOff( )
{
  m_ComputeMeshNodeNumbering = false;
}


/** Get number of points in the mesh */

template<class TInputImage, class TMaskImage, class TOutputMesh>
unsigned long
ImageToHexahedronMeshFilter<TInputImage,TMaskImage,TOutputMesh>
::GetNumberOfPoints( )
{   
  return (this->GetOutput( )->GetNumberOfPoints( ));
}

/** Get number of cells in the mesh */

template<class TInputImage, class TMaskImage, class TOutputMesh>
unsigned long
ImageToHexahedronMeshFilter<TInputImage,TMaskImage,TOutputMesh>
::GetNumberOfCells( )
{   
  return (this->GetOutput( )->GetNumberOfCells( ));
}

/** Set the user specified cell trait function for material property */

template<class TInputImage, class TMaskImage, class TOutputMesh>
void 
ImageToHexahedronMeshFilter<TInputImage,TMaskImage,TOutputMesh>
::SetCellTraitFunction( double (*CellTraitFunction)( InputPixelType ))
{
  m_MaterialPropertyFunction = CellTraitFunction;
}

/** Calculate the Material Property ( Elastic Modulus ) value
 *  of the voxel depending on the CT value of the voxel
 */

template<class TInputImage, class TMaskImage, class TOutputMesh>
double
ImageToHexahedronMeshFilter<TInputImage,TMaskImage,TOutputMesh>
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
template<class TInputImage, class TMaskImage, class TOutputMesh>
void
ImageToHexahedronMeshFilter<TInputImage,TMaskImage,TOutputMesh>
::GenerateData( )
{  
  InputImageConstPointer inputImage = static_cast<const InputImageType * >
                                         (this->ProcessObject::GetInput(0) );
  MaskImageConstPointer maskImage = NULL;
  if ( this->GetNumberOfInputs( ) > 1)
    {
      maskImage = static_cast<const MaskImageType * >
                                          (this->ProcessObject::GetInput(1) );
    }
    
  /* Spacing used to set the locations of the points */
  InputSpacingType spacing = inputImage->GetSpacing( );
  float spacingX = spacing[0]/2.0;
  float spacingY = spacing[1]/2.0;
  float spacingZ = spacing[2]/2.0;
  
  typedef ImageRegionConstIterator<InputImageType> IteratorType;
  IteratorType it( inputImage, inputImage->GetBufferedRegion( ) );
  it.GoToBegin( );
  
  if (maskImage)
    {
    typedef ImageRegionConstIterator<MaskImageType> MaskIteratorType;
    MaskIteratorType mit( maskImage, maskImage->GetBufferedRegion( ) );
    mit.GoToBegin( );
  
    while (!it.IsAtEnd( ))
      {
      if (mit.Value( ) > 0 )
        {
        /** Add Voxel to Mesh */           
        InputIndexType imageIndex = it.GetIndex( );
        InputOriginType location;
        
  
        inputImage->TransformIndexToPhysicalPoint( imageIndex,
                                                            location );
          /** Add a hexahedron Cell */
          InputOriginType node1;
          node1[0] = static_cast<float> ( fround(static_cast<double>(location[0]-spacingX), 4));
          node1[1] = static_cast<float> ( fround(static_cast<double>(location[1]-spacingY), 4));
          node1[2] = static_cast<float> ( fround(static_cast<double>(location[2]+spacingZ), 4));
          InputOriginType node2;
          node2[0] = static_cast<float> ( fround(static_cast<double>(location[0]-spacingX), 4));
          node2[1] = static_cast<float> ( fround(static_cast<double>(location[1]+spacingY), 4));
          node2[2] = static_cast<float> ( fround(static_cast<double>(location[2]+spacingZ), 4));
          InputOriginType node3;
          node3[0] = static_cast<float> ( fround(static_cast<double>(location[0]+spacingX), 4));
          node3[1] = static_cast<float> ( fround(static_cast<double>(location[1]+spacingY), 4));
          node3[2] = static_cast<float> ( fround(static_cast<double>(location[2]+spacingZ), 4));
          InputOriginType node4;
          node4[0] = static_cast<float> ( fround(static_cast<double>(location[0]+spacingX), 4));
          node4[1] = static_cast<float> ( fround(static_cast<double>(location[1]-spacingY), 4));
          node4[2] = static_cast<float> ( fround(static_cast<double>(location[2]+spacingZ), 4));
          InputOriginType node5;
          node5[0] = static_cast<float> ( fround(static_cast<double>(location[0]-spacingX), 4));
          node5[1] = static_cast<float> ( fround(static_cast<double>(location[1]-spacingY), 4));
          node5[2] = static_cast<float> ( fround(static_cast<double>(location[2]-spacingZ), 4));
          InputOriginType node6;
          node6[0] = static_cast<float> ( fround(static_cast<double>(location[0]-spacingX), 4));
          node6[1] = static_cast<float> ( fround(static_cast<double>(location[1]+spacingY), 4));
          node6[2] = static_cast<float> ( fround(static_cast<double>(location[2]-spacingZ), 4));
          InputOriginType node7;
          node7[0] = static_cast<float> ( fround(static_cast<double>(location[0]+spacingX), 4));
          node7[1] = static_cast<float> ( fround(static_cast<double>(location[1]+spacingY), 4));
          node7[2] = static_cast<float> ( fround(static_cast<double>(location[2]-spacingZ), 4));
          InputOriginType node8;
          node8[0] = static_cast<float> ( fround(static_cast<double>(location[0]+spacingX), 4));
          node8[1] = static_cast<float> ( fround(static_cast<double>(location[1]-spacingY), 4));
          node8[2] = static_cast<float> ( fround(static_cast<double>(location[2]-spacingZ), 4));

          meshSource->AddHexahedron(
          meshSource->AddPoint( node1 ),
          meshSource->AddPoint( node2 ),
          meshSource->AddPoint( node3 ),
          meshSource->AddPoint( node4 ),
          meshSource->AddPoint( node5 ),
          meshSource->AddPoint( node6 ),
          meshSource->AddPoint( node7 ),
          meshSource->AddPoint( node8 ));


/*
          meshSource->AddHexahedron(
          meshSource->AddPoint( location[0]-spacingX, 
                            location[1]-spacingY, location[2]+spacingZ ),
          meshSource->AddPoint( location[0]-spacingX, 
                            location[1]+spacingY, location[2]+spacingZ ),
          meshSource->AddPoint( location[0]+spacingX, 
                            location[1]+spacingY, location[2]+spacingZ ),
          meshSource->AddPoint( location[0]+spacingX, 
                            location[1]-spacingY, location[2]+spacingZ ),
          meshSource->AddPoint( location[0]-spacingX, 
                            location[1]-spacingY, location[2]-spacingZ ),
          meshSource->AddPoint( location[0]-spacingX, 
                            location[1]+spacingY, location[2]-spacingZ ),
          meshSource->AddPoint( location[0]+spacingX, 
                            location[1]+spacingY, location[2]-spacingZ ),
          meshSource->AddPoint( location[0]+spacingX, 
                            location[1]-spacingY, location[2]-spacingZ));
*/
        }
      ++mit;
      ++it;
      }
    }
  else
    {
    while (!it.IsAtEnd( ))
      {
      if (it.Value( ) > this->m_ImageThreshold )
        {
        /** Add Voxel to Mesh */           
        InputIndexType imageIndex = it.GetIndex( );
        InputOriginType location;
        inputImage->TransformIndexToPhysicalPoint( imageIndex, location);

        /** Add a hexahedron Cell */
        meshSource->AddHexahedron(
          meshSource->AddPoint( location[0]-spacingX, 
                          location[1]-spacingY, location[2]+spacingZ ),
          meshSource->AddPoint( location[0]-spacingX, 
                          location[1]+spacingY, location[2]+spacingZ ),
          meshSource->AddPoint( location[0]+spacingX, 
                          location[1]+spacingY, location[2]+spacingZ ),
          meshSource->AddPoint( location[0]+spacingX, 
                          location[1]-spacingY, location[2]+spacingZ ),
          meshSource->AddPoint( location[0]-spacingX, 
                          location[1]-spacingY, location[2]-spacingZ ),
          meshSource->AddPoint( location[0]-spacingX, 
                          location[1]+spacingY, location[2]-spacingZ ),
          meshSource->AddPoint( location[0]+spacingX, 
                          location[1]+spacingY, location[2]-spacingZ ),
          meshSource->AddPoint( location[0]+spacingX, 
                          location[1]-spacingY, location[2]-spacingZ ));
        }
      ++it;
      }
    }  

  // Set the Node Numbering - Two methods for numbering exists
  //     1. Incremental Mesh numbering 
  //     2. Voxel Based numbering 

  OutputPointType pointLocation;
  InputIndexType imageIndex;
  InputImageSizeType size = inputImage->GetLargestPossibleRegion( ).GetSize();
  unsigned long numberOfPoints = meshSource->GetOutput( )
                                            ->GetNumberOfPoints( );
  for ( unsigned long i = 0; i < numberOfPoints; i++ )
    {
    meshSource->GetOutput( )->GetPoint(i, &pointLocation);
    inputImage->TransformPhysicalPointToIndex( pointLocation,
                                                imageIndex );
    unsigned long pointIndex;
    if ( m_ComputeMeshNodeNumbering )
      {
      pointIndex = imageIndex[2]*size[1]*size[0] + 
                                    imageIndex[1]*size[0] +
                                    imageIndex[0] + m_MeshIndexOffset;
      }
    else
      {
      pointIndex = (unsigned long) ( i + m_MeshIndexOffset );
      }
    meshSource->GetOutput( )->SetPointData( i, pointIndex );
    }
  
  // Set the Cell properties if requested by the user. The default
  // method for assigning the material property values is conversion
  // of image data into an elastic modulus value. It uses a standard
  // calculation appropriate for CT data. The user can also specify a
  // a user defined function for the conversion.
     
  if ( m_ComputeMeshProperties )
    {
    CellIterator cellIterator = meshSource->GetOutput( )->GetCells( )
                                                        ->Begin( );
    CellIterator cellEnd = meshSource->GetOutput( )->GetCells( )->End( );
    while(cellIterator != cellEnd)
      {
      CellType * cell = cellIterator.Value( );
      if ( cell->GetType( ) == CellType::HEXAHEDRON_CELL )      
        {
        // Iterate through the points and compute the average position
        // This position is then used to reference the image data
        
        OutputPointType meanLocation;
        meanLocation.Fill(0.0);
        for ( PointIterator pointIterator=cell->PointIdsBegin( ); 
                  pointIterator!=cell->PointIdsEnd( ); ++pointIterator )
          {
          OutputPointType pointLocation;
               meshSource->GetOutput( )->GetPoint( *pointIterator, 
                                                &pointLocation );
          meanLocation[0] += pointLocation[0];
          meanLocation[1] += pointLocation[1];
          meanLocation[2] += pointLocation[2];
          }
        meanLocation[0] /= 8.0;
        meanLocation[1] /= 8.0;
        meanLocation[2] /= 8.0;

                  InputIndexType imageIndex;
        inputImage->TransformPhysicalPointToIndex( meanLocation,
                                                    imageIndex );
        InputPixelType pixelData = inputImage->GetPixel(imageIndex);
        double matProp;
        if ( m_UseCtProperties )
          {
          matProp = CalculateMaterialProps(pixelData);
          }
        else
          {
          matProp = m_MaterialPropertyFunction(pixelData);
          }
        meshSource->GetOutput()->SetCellData( cellIterator.Index( ),matProp );
        }
      ++cellIterator;
      }
    }
  meshSource->GetOutput()->BuildCellLinks();
  this->SetOutput( meshSource->GetOutput( ) );
  
  // This indicates that the current BufferedRegion is equal to the 
  // requested region. This action prevents useless rexecutions of
  // the pipeline.

  this->GetOutput( )->SetBufferedRegion( this->GetOutput( )
                                              ->GetRequestedRegion( ) );
}

/** PrintSelf */
template<class TInputImage, class TMaskImage, class TOutputMesh>
void
ImageToHexahedronMeshFilter<TInputImage,TMaskImage,TOutputMesh>
::PrintSelf( std::ostream& os, Indent indent ) const
{
  Superclass::PrintSelf(os, indent);

  os << indent
     << "NumberOfNodes: "
     << std::endl;

  os << indent
     << "NumberOfCells: "
     << std::endl;
}

} /** end namespace itk. */

#endif
