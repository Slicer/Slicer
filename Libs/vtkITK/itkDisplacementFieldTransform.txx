/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkDisplacementFieldTransform.txx,v $
  Language:  C++
  Date:      $Date: 2008-04-11 16:28:11 $
  Version:   $Revision: 1.40 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkDisplacementFieldTransform_txx
#define __itkDisplacementFieldTransform_txx

#include "itkDisplacementFieldTransform.h"
#include "itkContinuousIndex.h"
#include "itkImageRegionIterator.h"
#include "itkImageRegionConstIterator.h"
#include "itkIdentityTransform.h"

namespace itk
{

// Constructor with default arguments
template<class TScalarType, unsigned int NDimensions>
DisplacementFieldTransform<TScalarType, NDimensions>
::DisplacementFieldTransform():Superclass(SpaceDimension,0)
{

  m_InternalParametersBuffer = ParametersType(0);
  // Make sure the parameters pointer is not NULL after construction.
  m_InputParametersPointer = &m_InternalParametersBuffer;

  // Default grid size is zero
  SizeType size;
  IndexType index;
  RegionType region;
  size.Fill( 0 );
  index.Fill( 0 );
  region.SetSize( size );
  region.SetIndex( index );

  OriginType origin;
  origin.Fill( 0.0 );  // default origin is all zeros
  SpacingType spacing;
  spacing.Fill( 1.0 ); // default spacing is all ones
  DirectionType direction;
  direction.SetIdentity(); // default spacing is all ones

  // Initialize coeffient images
  m_CoefficientImage = ImageType::New();
  m_CoefficientImage->SetVectorLength( SpaceDimension );
  m_CoefficientImage->SetRegions( region );
  m_CoefficientImage->SetOrigin( origin.GetDataPointer() );
  m_CoefficientImage->SetSpacing( spacing.GetDataPointer() );
  m_CoefficientImage->SetDirection( direction );

  m_IndexToPoint.SetIdentity();
  m_PointToIndex.SetIdentity();

  // Initialize jacobian images
  for ( unsigned int j = 0; j < SpaceDimension; j++ )
    {
    m_JacobianImage[j] = JacobianImageType::New();
    m_JacobianImage[j]->SetVectorLength( SpaceDimension );
    m_JacobianImage[j]->SetRegions( region );
    m_JacobianImage[j]->SetOrigin( origin );
    m_JacobianImage[j]->SetSpacing( spacing );
    m_JacobianImage[j]->SetDirection( direction );
    }
  
  /** Fixed Parameters store the following information:
   *     Grid Size
   *     Grid Origin
   *     Grid Spacing
   *     Grid Direction
   *  The size of these is equal to the  NInputDimensions
   */
  this->m_FixedParameters.SetSize ( NDimensions * (NDimensions + 3) );
  this->m_FixedParameters.Fill ( 0.0 );
  for (unsigned int i=0; i<NDimensions; i++)
    {
    this->m_FixedParameters[2*NDimensions+i] = spacing[i];
    }
  for (unsigned int di=0; di<NDimensions; di++)
    {
    for (unsigned int dj=0; dj<NDimensions; dj++)
      {
      this->m_FixedParameters[3*NDimensions+(di*NDimensions+dj)] = direction[di][dj];
      }
    }
}
    

// Destructor
template<class TScalarType, unsigned int NDimensions>
DisplacementFieldTransform<TScalarType, NDimensions>
::~DisplacementFieldTransform()
{
}

// Get the number of parameters
template<class TScalarType, unsigned int NDimensions>
unsigned int
DisplacementFieldTransform<TScalarType, NDimensions>
::GetNumberOfParameters(void) const
{

  // The number of parameters equal SpaceDimension * number of
  // of pixels in the grid region.
  return ( static_cast<unsigned int>( SpaceDimension ) *
           static_cast<unsigned int>( m_CoefficientImage->GetBufferedRegion().GetNumberOfPixels() ) );

}

// Set the parameters
template<class TScalarType, unsigned int NDimensions>
void
DisplacementFieldTransform<TScalarType, NDimensions>
::SetIdentity()
{
  if( m_InputParametersPointer )
    {
    ParametersType * parameters =
      const_cast<ParametersType *>( m_InputParametersPointer );
    parameters->Fill( 0.0 );
    this->Modified();
    }
  else 
    {
    itkExceptionMacro( << "Input parameters for the DisplacementFieldTransform haven't been set ! "
       << "Set them using the SetParameters or SetImage method first." );
    }
}

// Set the parameters
template<class TScalarType, unsigned int NDimensions>
void
DisplacementFieldTransform<TScalarType, NDimensions>
::SetParameters( const ParametersType & parameters )
{

  // check if the number of parameters match the
  // expected number of parameters
  if ( parameters.Size() != this->GetNumberOfParameters() )
    {
    itkExceptionMacro(<<"Mismatched between parameters size "
                      << parameters.size() );
    }

  // Clean up buffered parameters
  m_InternalParametersBuffer = ParametersType( 0 );

  // Keep a reference to the input parameters
  m_InputParametersPointer = &parameters;

  // Wrap flat array as images of coefficients
  this->WrapAsImages();

  // Modified is always called since we just have a pointer to the
  // parameters and cannot know if the parameters have changed.
  this->Modified();
}

// Set the Fixed Parameters
template<class TScalarType, unsigned int NDimensions>
void
DisplacementFieldTransform<TScalarType, NDimensions>
::SetFixedParameters( const ParametersType & passedParameters )
{
 
  ParametersType parameters( NDimensions * (3 + NDimensions) );

  // check if the number of parameters match the
  // expected number of parameters
  if ( passedParameters.Size() == NDimensions * 3 )
    {
    parameters.Fill( 0.0 );
    for(unsigned int i=0; i<3 * NDimensions; i++)
      {
      parameters[i] = passedParameters[i];
      }
    for (unsigned int di=0; di<NDimensions; di++)
      {
      parameters[3*NDimensions+(di*NDimensions+di)] = 1;
      }
    }
  else if ( passedParameters.Size() != NDimensions * (3 + NDimensions) )
    {
    itkExceptionMacro(<< "Mismatched between parameters size "
                      << passedParameters.size() 
                      << " and number of fixed parameters "
                      << NDimensions * (3 + NDimensions) );
    }
  else
    {
    for(unsigned int i=0; i<NDimensions * (3 + NDimensions); i++)
      {
      parameters[i] = passedParameters[i];
      }
    }

  /********************************************************* 
    Fixed Parameters store the following information:
        Grid Size
        Grid Origin
        Grid Spacing
        Grid Direction
     The size of these is equal to the  NInputDimensions
  *********************************************************/
  
  /** Set the Grid Parameters */
  SizeType   gridSize;
  for (unsigned int i=0; i<NDimensions; i++)
    {
    gridSize[i] = static_cast<int> (parameters[i]);
    }
  RegionType gridRegion;
  gridRegion.SetSize( gridSize );
  
  /** Set the Origin Parameters */
  OriginType origin;
  for (unsigned int i=0; i<NDimensions; i++)
    {
    origin[i] = parameters[NDimensions+i];
    }
  
  /** Set the Spacing Parameters */
  SpacingType spacing;
  for (unsigned int i=0; i<NDimensions; i++)
    {
    spacing[i] = parameters[2*NDimensions+i];
    }

  /** Set the Direction Parameters */
  DirectionType direction;
  for (unsigned int di=0; di<NDimensions; di++)
    {
    for (unsigned int dj=0; dj<NDimensions; dj++)
      {
      direction[di][dj] = parameters[3*NDimensions+(di*NDimensions+dj)];
      }
    }
  
  m_CoefficientImage->SetRegions( gridRegion );
  m_CoefficientImage->SetSpacing( spacing.GetDataPointer() );
  m_CoefficientImage->SetDirection( direction );
  m_CoefficientImage->SetOrigin( origin.GetDataPointer() );
  
  for ( unsigned int j = 0; j < SpaceDimension; j++ )
    {
    m_JacobianImage[j]->SetRegions( gridRegion );
    m_JacobianImage[j]->SetSpacing( spacing.GetDataPointer() );
    m_JacobianImage[j]->SetDirection( direction );
    m_JacobianImage[j]->SetOrigin( origin.GetDataPointer() );
    }
  
  if (m_InputParametersPointer == &m_InternalParametersBuffer)
    {
    // Check if we need to resize the default parameter buffer.
    if ( m_InternalParametersBuffer.GetSize() != this->GetNumberOfParameters() )
      {
      m_InternalParametersBuffer.SetSize( this->GetNumberOfParameters() );
      // Fill with zeros for identity.
      m_InternalParametersBuffer.Fill( 0 );
      }
    }
  
  DirectionType scale;
  for( unsigned int i=0; i<SpaceDimension; i++)
    {
    scale[i][i] = spacing[i];
    }
  
  m_IndexToPoint = m_CoefficientImage->GetDirection() * scale;
  m_PointToIndex = m_IndexToPoint.GetInverse();

  this->Modified();
}


// Wrap flat parameters as images
template<class TScalarType, unsigned int NDimensions>
void
DisplacementFieldTransform<TScalarType, NDimensions>
::WrapAsImages()
{

  /**
   * Wrap flat parameters array into VectorImage
   * NOTE: For efficiency, parameters are not copied locally. The parameters
   * are assumed to be maintained by the caller.
   */
  InternalPixelType * dataPointer =
    const_cast<InternalPixelType *>(( m_InputParametersPointer->data_block() ));
  unsigned int numberOfParameters = this->GetNumberOfParameters();
  unsigned int numberOfPixels = numberOfParameters / SpaceDimension;

  m_CoefficientImage->GetPixelContainer()->
    SetImportPointer( dataPointer, numberOfParameters );

  /**
   * Allocate memory for Jacobian and wrap into SpaceDimension number
   * of ITK images
   */
  this->m_NonThreadsafeSharedJacobian.set_size( SpaceDimension, numberOfParameters );
  this->m_NonThreadsafeSharedJacobian.Fill( NumericTraits<JacobianInternalPixelType>::Zero );
  SizeType size;
  size.Fill( 1 );
  m_LastSupportRegion.SetSize( size );
  JacobianInternalPixelType * jacobianDataPointer = this->m_NonThreadsafeSharedJacobian.data_block();

  for ( unsigned int j = 0; j < SpaceDimension; j++ )
    {
    m_JacobianImage[j]->GetPixelContainer()->
      SetImportPointer( jacobianDataPointer, numberOfParameters );
    jacobianDataPointer += numberOfParameters;
    }
}


// Set the parameters by value
template<class TScalarType, unsigned int NDimensions>
void
DisplacementFieldTransform<TScalarType, NDimensions>
::SetParametersByValue( const ParametersType & parameters )
{

  // check if the number of parameters match the
  // expected number of parameters
  if ( parameters.Size() != this->GetNumberOfParameters() )
    {
    itkExceptionMacro(<<"Mismatched between parameters size "
                      << parameters.size() );
    }

  // copy it
  m_InternalParametersBuffer = parameters;
  m_InputParametersPointer = &m_InternalParametersBuffer;

  // wrap flat array as images of coefficients
  this->WrapAsImages();

  // Modified is always called since we just have a pointer to the
  // parameters and cannot know if the parameters have changed.
  this->Modified();

}

// Get the parameters
template<class TScalarType, unsigned int NDimensions>
const 
typename DisplacementFieldTransform<TScalarType, NDimensions>
::ParametersType &
DisplacementFieldTransform<TScalarType, NDimensions>
::GetParameters( void ) const
{
  /** NOTE: For efficiency, this class does not keep a copy of the parameters - 
   * it just keeps pointer to input parameters. 
   */
  if( !m_InputParametersPointer )
    { 
    itkExceptionMacro(<<"Parameters are not yet set. ");
    }
  return m_InternalParametersBuffer;
}


// Get the parameters
template<class TScalarType, unsigned int NDimensions>
const 
typename DisplacementFieldTransform<TScalarType, NDimensions>
::ParametersType &
DisplacementFieldTransform<TScalarType, NDimensions>
::GetFixedParameters( void ) const
{
  RegionType resRegion = m_CoefficientImage->GetBufferedRegion();
  
  for (unsigned int i=0; i<NDimensions; i++)
    {
    this->m_FixedParameters[i] = (resRegion.GetSize())[i];
    }
  for (unsigned int i=0; i<NDimensions; i++)
    {
    this->m_FixedParameters[NDimensions+i] = (m_CoefficientImage->GetOrigin())[i];
    } 
  for (unsigned int i=0; i<NDimensions; i++)
    {
    this->m_FixedParameters[2*NDimensions+i] =  (m_CoefficientImage->GetSpacing())[i];
    }
  for (unsigned int di=0; di<NDimensions; di++)
    {
    for (unsigned int dj=0; dj<NDimensions; dj++)
      {
      this->m_FixedParameters[3*NDimensions+(di*NDimensions+dj)] = (m_CoefficientImage->GetDirection())[di][dj];
      }
    }
  
  return (this->m_FixedParameters);
}


  
// Set the coefficients using input images
template<class TScalarType, unsigned int NDimensions>
void 
DisplacementFieldTransform<TScalarType, NDimensions>
::SetImage( ImagePointer image )
{
  if ( image )
    {
    m_CoefficientImage = image;

    // Clean up buffered parameters
    unsigned int numberOfParameters = this->GetNumberOfParameters();
    m_InternalParametersBuffer = ParametersType(image->GetPixelContainer()->GetImportPointer(), 
                                                numberOfParameters);
    m_InputParametersPointer = &m_InternalParametersBuffer;

    SpacingType spacing = image->GetSpacing();
    for ( unsigned int j = 0; j < SpaceDimension; j++ )
    {
    m_JacobianImage[j]->SetRegions( image->GetBufferedRegion() );
    m_JacobianImage[j]->SetSpacing( spacing.GetDataPointer() );
    m_JacobianImage[j]->SetDirection( image->GetDirection() );
    m_JacobianImage[j]->SetOrigin( image->GetOrigin().GetDataPointer() );
    }
  
    DirectionType scale;
    for( unsigned int i=0; i<SpaceDimension; i++)
      {
      scale[i][i] = spacing[i];
      }
    
    m_IndexToPoint = image->GetDirection() * scale;
    m_PointToIndex = m_IndexToPoint.GetInverse();

    this->Modified();
    }
}  

// Print self
template<class TScalarType, unsigned int NDimensions>
void
DisplacementFieldTransform<TScalarType, NDimensions>
::PrintSelf(std::ostream &os, Indent indent) const
{

  this->Superclass::PrintSelf(os, indent);

  os << indent << "CoefficientImage: " << m_CoefficientImage.GetPointer() << std::endl;

  os << indent << "InputParametersPointer: " 
     << m_InputParametersPointer << std::endl;
}

// Transform a point
template<class TScalarType, unsigned int NDimensions>
bool 
DisplacementFieldTransform<TScalarType, NDimensions>
::InsideValidRegion( 
  const ContinuousIndexType& index ) const
{
  bool inside = true;

  if ( !m_CoefficientImage->GetBufferedRegion().IsInside( index ) )
    {
    inside = false;
    }

  return inside;
}

// Transform a point
template<class TScalarType, unsigned int NDimensions>
typename DisplacementFieldTransform<TScalarType, NDimensions>
::OutputPointType
DisplacementFieldTransform<TScalarType, NDimensions>
::TransformPoint(const InputPointType &point) const 
{
  unsigned int j;
  OutputPointType outputPoint;

  InputPointType transformedPoint;
  transformedPoint = point;

  ContinuousIndexType index;
  
  this->TransformPointToContinuousIndex( point, index );
  
  // NOTE: if the support region does not lie totally within the grid
  // we assume zero displacement and return the input point
  bool inside = this->InsideValidRegion( index );
  if ( !inside )
    return transformedPoint;
  
  outputPoint.Fill( NumericTraits<ScalarType>::Zero );
  
  OutputVectorType displacement = LinearInterpolateAtIndex( index );
  for( j = 0; j < SpaceDimension; j++ )
    {
    outputPoint[j] = transformedPoint[j] + displacement[j];
    }

  return outputPoint;

}

// Compute the Jacobian in one position 
template<class TScalarType, unsigned int NDimensions>
const
typename DisplacementFieldTransform<TScalarType, NDimensions>
::JacobianType &
DisplacementFieldTransform<TScalarType, NDimensions>
::GetJacobian( const InputPointType & point )
{
   ComputeJacobianWithRespectToParameters( p, this->m_NonThreadsafeSharedJacobian );
   return this->m_NonThreadsafeSharedJacobian;
}

void
DisplacementFieldTransform<TScalarType, NDimensions>
::ComputeJacobianWithRespectToParameters(const InputPointType & p, JacobianType & jacobian) const
{
  // Can only compute Jacobian if parameters are set via
  // SetParameters or SetParametersByValue
  if( m_InputParametersPointer == NULL )
    {
    itkExceptionMacro( <<"Cannot compute Jacobian: parameters not set" );
    }

  // Zero all components of jacobian
  // NOTE: for efficiency, we only need to zero out the coefficients
  // that got fill last time this method was called.

  typedef ImageRegionIterator<JacobianImageType> IteratorType;
  IteratorType m_Iterator[ SpaceDimension ];

  JacobianPixelType p0;
  p0.SetSize( SpaceDimension );
  for ( unsigned int j = 0; j < SpaceDimension; j++ )
    {
    p0[j] = NumericTraits<JacobianInternalPixelType>::Zero;
    }

  for ( unsigned int j = 0; j < SpaceDimension; j++ )
    {
    m_Iterator[j] = IteratorType( m_JacobianImage[j], m_LastSupportRegion );
    }

  while ( ! m_Iterator[0].IsAtEnd() )
    {
    // zero out jacobian elements
    for ( unsigned int j = 0; j < SpaceDimension; j++ )
      {
      m_Iterator[j].Set( p0 );
      }

    for ( unsigned int j = 0; j < SpaceDimension; j++ )
      {
      ++( m_Iterator[j] );
      }
    }

  ContinuousIndexType index;
  this->TransformPointToContinuousIndex( point, index );

  // NOTE: if the support region does not lie totally within the grid
  // we assume zero displacement and return the input point
  if ( !this->InsideValidRegion( index ) )
    {
    jacobian = this->m_NonThreadsafeSharedJacobian;
    return;
    }

  IndexType supportIndex;
  for( unsigned int j = 0; j < SpaceDimension; j++ ) 
    {
    supportIndex[j] = (long) vcl_floor(index[j] );
    }
  m_LastSupportRegion.SetIndex( supportIndex );

  // For each dimension, copy the weight to the support region
  unsigned long counter = 0;
  for ( unsigned int j = 0; j < SpaceDimension; j++ )
    {
    m_Iterator[j] = IteratorType( m_JacobianImage[j], m_LastSupportRegion );
    }

  JacobianPixelType p[ SpaceDimension ];
  for ( unsigned int j = 0; j < SpaceDimension; j++ )
    {
    // set to zeros
    p[j] = p0; 
    }

  while ( ! m_Iterator[0].IsAtEnd() )
    {
    IndexType ind = m_Iterator[0].GetIndex();
    // copy weight to jacobian image
    double weight = 1.0;
    for ( unsigned int j = 0; j < SpaceDimension; j++ )
      {
      weight = weight * ( 1 - vcl_abs( ind[j] - index[j] ) );
      }
    for ( unsigned int j = 0; j < SpaceDimension; j++ )
      {
      p[j][j] = weight;
      m_Iterator[j].Set( p[j] );
      }

    // go to next coefficient in the support region
    ++ counter;
    for ( unsigned in j = 0; j < SpaceDimension; j++ )
      {
      ++( m_Iterator[j] );
      }
    }
  jacobian=this->m_NonThreadsafeSharedJacobian;
  return;
}

template<class TScalarType, unsigned int NDimensions>
void
DisplacementFieldTransform<TScalarType, NDimensions>
::TransformPointToContinuousIndex( const InputPointType & point, ContinuousIndexType & index ) const
{
  unsigned int j;

  Vector<double, SpaceDimension> tvector;

  OriginType origin = m_CoefficientImage->GetOrigin();
  for ( j = 0; j < SpaceDimension; j++ )
    {
    tvector[j] = point[j] - origin[j];
    }

  Vector<double, SpaceDimension> cvector;

  cvector = m_PointToIndex * tvector;

  for ( j = 0; j < SpaceDimension; j++ )
    {
    index[j] = static_cast< typename ContinuousIndexType::CoordRepType >( cvector[j] );
    }
}

/**
 * Evaluate at image index position
 */
template<class TScalarType, unsigned int NDimensions>
typename DisplacementFieldTransform<TScalarType, NDimensions>::OutputVectorType
DisplacementFieldTransform<TScalarType, NDimensions>
::LinearInterpolateAtIndex( const ContinuousIndexType& index ) const
{
  unsigned int neighbors = 1 << SpaceDimension;
  unsigned int dim;  // index over dimension

  /**
   * Compute base index = closet index below point
   * Compute distance from point to base index
   */
  signed long baseIndex[SpaceDimension];
  double distance[SpaceDimension];

  for( dim = 0; dim < SpaceDimension; dim++ )
    {
    baseIndex[dim] = (long) vcl_floor(index[dim] );
    distance[dim] = index[dim] - double( baseIndex[dim] );
    }
  
  /**
   * Interpolated value is the weight some of each of the surrounding
   * neighbors. The weight for each neighbour is the fraction overlap
   * of the neighbor pixel with respect to a pixel centered on point.
   */
  OutputVectorType output;
  output.Fill( 0.0 );

  typedef NumericTraits<TScalarType>::RealType RealType;
  RealType totalOverlap = 0.0;

  for( unsigned int counter = 0; counter < neighbors; counter++ )
    {

    double overlap = 1.0;          // fraction overlap
    unsigned int upper = counter;  // each bit indicates upper/lower neighbour
    IndexType neighIndex;

    // get neighbor index and overlap fraction
    for( dim = 0; dim < SpaceDimension; dim++ )
      {

      if ( upper & 1 )
        {
        neighIndex[dim] = baseIndex[dim] + 1;
        overlap *= distance[dim];
        }
      else
        {
        neighIndex[dim] = baseIndex[dim];
        overlap *= 1.0 - distance[dim];
        }

      upper >>= 1;

      }
    
    // get neighbor value only if overlap is not zero
    if( overlap )
      {
      const PixelType input = m_CoefficientImage->GetPixel( neighIndex );
      for(unsigned int k = 0; k < SpaceDimension; k++ )
        {
        output[k] += overlap * static_cast<double>( input[k] );
        }
      totalOverlap += overlap;
      }

    if( totalOverlap == 1.0 )
      {
      // finished
      break;
      }

    }

  return ( output );
}

} // namespace

#endif
