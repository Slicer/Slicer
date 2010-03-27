/*=========================================================================

  Program:   Diffusion Applications
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#ifndef __DiffusionTensor3DInterpolateImageFunctionReimplementation_txx
#define __DiffusionTensor3DInterpolateImageFunctionReimplementation_txx

#include "itkDiffusionTensor3DInterpolateImageFunctionReimplementation.h"


namespace itk
{


template< class TData >
DiffusionTensor3DInterpolateImageFunctionReimplementation< TData >
::DiffusionTensor3DInterpolateImageFunctionReimplementation()
{
  m_Threads = Semaphore::New() ;
  m_SeparationDone = false ;
  m_CannotSplit = false ;
  m_Lock = MutexLock::New() ;
  m_LockNewThreadDetected = MutexLock::New() ;
  m_NbThread = 0 ;
  m_CheckRegionsDone = MutexLock::New() ;
  m_ExceptionThrown = false ;
  m_AllocateInterpolatorsDone = false ;
}

//allocate the memory for the 6 images containing the 6 separated components of the DTI
//This function is run only by one single thread that blocks all the other threads if any
template< class TData >
void
DiffusionTensor3DInterpolateImageFunctionReimplementation< TData >
::AllocateImages()
{
  itkRegionType itkRegion ;
  itkRegion = this->m_InputImage->GetLargestPossibleRegion() ;
  SizeType size = itkRegion.GetSize() ;
  typename DiffusionImageType::PointType origin = this->m_InputImage->GetOrigin() ;
  typename DiffusionImageType::DirectionType direction = this->m_InputImage->GetDirection() ;
  typename DiffusionImageType::SpacingType spacing = this->m_InputImage->GetSpacing() ; 
  //Read the DTI input image size, spacing, origin and direction and create 6 scalar images with the same properties
  for( int i = 0 ; i < 6 ; i++ ) 
    {
    m_Image[ i ] = ImageType::New() ;
    m_Image[ i ]->SetRegions( size ) ;
    m_Image[ i ]->SetOrigin( origin ) ;
    m_Image[ i ]->SetDirection( direction ) ;
    m_Image[ i ]->SetSpacing( spacing ) ;
    m_Image[ i ]->Allocate() ;
    }
/*  //allocate the interpolator in the derivated class. This is only a virtual class that doesn't know the interpolator type
  AllocateInterpolator() ;
  for( int i = 0 ; i < 6 ; i++ )
    {
    m_Interpol[ i ]->SetInputImage( m_Image[ i ] ) ;
    }*/
  // Copied from itkImageSource.txx and adapted
  // split on the outermost dimension available
  m_SplitAxis = this->m_InputImage->GetImageDimension() - 1 ;
  while( size[ m_SplitAxis ] == 1)
    {
    --m_SplitAxis;
    if( m_SplitAxis < 0 )
      { // cannot split
      itkDebugMacro("  Cannot Split" ) ; 
      m_CannotSplit = true ;
      }
    }
  m_NbThread = 0 ;
  //Initialize the list of region with a first region that contains the whole image
  m_ListRegions.clear() ;
  RegionType region ;
  region.itkRegion = itkRegion ;
  region.Done = false ;
  Index< 3 > position ;
  position.Fill( 0 ) ;
  region.PositionInImage = position ;
  region.Stop = false ;
  m_ListRegions.push_back( region ) ;
  this->latestTime = Object::GetMTime() ;
  m_ExceptionThrown = false ; 
}


//Checks the region that has the most voxel not copied into the separated images and returns its ID
template< class TData >
int
DiffusionTensor3DInterpolateImageFunctionReimplementation< TData >
::RegionToDivide()
{
  int largestRegion = -1 ;//returns -1 if all the regions have been processed completely
  if( m_ListRegions.size() != 0 ) //at least one region exist
  {
    //Find largest region
    long largestNumber = 0 ;
    for( int i = 0 ; i < (signed)m_ListRegions.size() ; i++ )
    {
      if( !m_ListRegions[ i ].Done )
        {
        SizeType size = m_ListRegions[ i ].itkRegion.GetSize() ;
        Index< 3 > start = m_ListRegions[ i ].itkRegion.GetIndex() ;//Where the region starts in the image
        Index< 3 > position = m_ListRegions[ i ].PositionInImage ;//Where we are in the image
        long currentPos = position[ 0 ] - start[ 0 ] + size[ 0 ] * ( position[ 1 ] - start[ 1 ] + size[ 1 ] * ( position[ 2 ] - start[ 2 ] ) ) ;//The number of voxels copied in this region
        long currentSize = size[ 0 ] * size[ 1 ] * size[ 2 ] ;//The size of the current region
        long currentNotDone = currentSize - currentPos ;//The number of voxels that are not copied yet
        if( largestNumber < currentNotDone )
        {
          largestRegion = i ;
          largestNumber = currentNotDone ;
        }
      }
    }
  }
  return largestRegion ;
}

//Divide a region in 2 regions
template< class TData >
bool
DiffusionTensor3DInterpolateImageFunctionReimplementation< TData >
::DivideRegion( int currentThread )
{
//Compute how much has been completed in the current region
itkRegionType itkRegion = m_ListRegions[ currentThread ].itkRegion ;
SizeType oldSize = itkRegion.GetSize() ;
typename ImageType::IndexType start = itkRegion.GetIndex() ;//position in the image of the origin of the current region
int portionDone ;
Index< 3 > position ;
position = m_ListRegions[ currentThread ].PositionInImage ;//what voxel has to be copied next (its position in the image)
portionDone = position[ m_SplitAxis ] - start[ m_SplitAxis ] ;//number of line/plane already copied along the split axis
//Split region
start[ m_SplitAxis ] += portionDone ;
oldSize[ m_SplitAxis ] -= portionDone ;//size of the region that still needs to be copied
itkRegion.SetIndex( start ) ;
SizeType size ;
size = oldSize ;
size[ m_SplitAxis ] = oldSize[ m_SplitAxis ] / 2 + oldSize[ m_SplitAxis ]%2 ;//size of the first new region
itkRegion.SetSize( size ) ;
m_ListRegions[ currentThread ].itkRegion = itkRegion ;
m_ListRegions[ currentThread ].Done = false ;
m_ListRegions[ currentThread ].PositionInImage = position ;//it will continue to copy from the position where it stops (even if a line/plane had started to be copied) to avoid processing several times the same voxel
m_ListRegions[ currentThread ].Stop = false ;
if( oldSize[ m_SplitAxis ] / 2 > 0 )//if the second region is at least on voxel thick
{
  start[ m_SplitAxis ] += size[ m_SplitAxis ] ;
  size[ m_SplitAxis ] = oldSize[ m_SplitAxis ] / 2 ;//size of the second new region
  itkRegion.SetSize( size ) ;
  itkRegion.SetIndex( start ) ;
  RegionType region ;
  region.itkRegion = itkRegion ;
  region.Done = false ;
  region.PositionInImage = start ;
  region.Stop = false ;
  m_ListRegions.push_back( region ) ;// We add the new region to the list of existing regions
  return true ;
}
return false ;
}

template< class TData >
void
DiffusionTensor3DInterpolateImageFunctionReimplementation< TData >
::SeparateImages()
{
  int currentThread ;
  m_LockNewThreadDetected->Lock() ;//only one new region can be created at a time
  currentThread = m_NbThread ;//before adding one to the Nb of Thread so we do not have to substract one for the array indexes
  if( currentThread == 0 )//first thread
  {
    m_Threads->Initialize( 1 ) ;//initializing the semaphore
  }
  m_NbThread++ ;
  int regionToDivide = RegionToDivide() ;//Get the region to divide (if this is not the first thread)
  if( currentThread != 0 && regionToDivide != -1 && !m_CannotSplit )
  {
    m_ListRegions[ regionToDivide ].Stop = true ;//set a flag to true so that the thread processing the region to divide will stop
  }
  m_Threads->Down() ;//Waits for the thread processing the region to divide to divide it and call us
  m_LockNewThreadDetected->Unlock() ;//another new thread can be created
  if( m_SeparationDone || this->latestTime < Object::GetMTime() || m_ExceptionThrown )//if while waiting, the separation was done, or the input image has been changed
  {
    m_Threads->Up() ;//in case another thread is stuck because of the semaphore, we unblock it. It should go through here and exit this function
    return ;//then we just leave this function
  }
  bool isAtEnd ;//will contain the value of the iterator.IsAtEnd because the iterator is local to the do..while loop
  do
  {
  std::vector< IteratorImageType > out ;
  for( int i = 0 ; i < 6 ; i++ ) 
    {
    IteratorImageType outtemp( m_Image[ i ] , m_ListRegions[ currentThread ].itkRegion ) ;
    out.push_back( outtemp ) ;
    }
  IteratorDiffusionImageType it( this->m_InputImage , m_ListRegions[ currentThread ].itkRegion ) ;

    it.SetIndex( m_ListRegions[ currentThread ].PositionInImage ) ;
    for( int i = 0 ; i < 6 ; i++ )
      {
      out[ i ].SetIndex( m_ListRegions[ currentThread ].PositionInImage  ) ;//Set where the iterator should start the copy
      }
  //Then we continue copying the tensors into the separate images
  for( ; !it.IsAtEnd() && !m_ListRegions[ currentThread ].Stop ; ++it )//copy voxels until region is done or until a new thread interrupts
    {
    TensorDataType tensor = it.Get() ;
    for( int i = 0 ; i < 6 ; i++ )
      {
      out[ i ].Set( tensor[ i ] ) ;
      ++out[ i ] ;
      }
    }
   isAtEnd = it.IsAtEnd() ;
   m_ListRegions[ currentThread ].PositionInImage = it.GetIndex() ;//Save the position where we just stopped in the region
 if( !isAtEnd )//it means the thread got interrupted to divide the region for a new thread
 {
   //divide region
   if( DivideRegion( currentThread ) )
     {
     m_Threads->Up() ;//We let the new thread enter
     }
 }
 }while( !isAtEnd ) ;
 if( this->latestTime < Object::GetMTime() )//if the input image was modifed while we were copying it
 {
   m_ExceptionThrown = true ;
   m_Threads->Up() ;//we unlock the new thread so it can exit from this function
   itkExceptionMacro( << "Input image was changed while pre-processing it" ) ;//Throw an exception to explain the problem
 }
m_ListRegions[ currentThread ].Done = true ;
m_CheckRegionsDone->Lock() ;//Only one thread at a time checks if all the regions are done processing
if( m_SeparationDone )//If the current thread has been locked while another one was checking if the region was done processing, it just needs to exit the function if all the regions are done
{
  return ;
}
unsigned int nbRegionsDone = 0 ;
for( unsigned int i = 0 ; i < m_ListRegions.size() ; i++ )
{
  nbRegionsDone += ( m_ListRegions[ i ].Done ? 1 : 0 ) ;
}
if( nbRegionsDone == m_ListRegions.size() )
{
  m_SeparationDone = true ;//All the regions are done processing
  m_Threads->Up() ;
}
m_CheckRegionsDone->Unlock() ;
}



template< class TData >
void
DiffusionTensor3DInterpolateImageFunctionReimplementation< TData >
::SetInputImage( DiffusionImageTypePointer inputImage )
{
  DiffusionTensor3DInterpolateImageFunction< DataType >::SetInputImage( inputImage ) ;
  m_SeparationDone = false ;//The image has not been copied yet into 6 separate images
  m_AllocateInterpolatorsDone = false ;//The interpolators have not been allocated
}

template< class TData >
typename DiffusionTensor3DInterpolateImageFunctionReimplementation< TData >
::TensorDataType 
DiffusionTensor3DInterpolateImageFunctionReimplementation< TData >
::Evaluate( const PointType &point )
{
  if( this->m_InputImage.IsNotNull() )//If input image has been set
    {
    if( this->latestTime< Object::GetMTime() || !m_SeparationDone )//We need to separate the image only once until it is changed
      { 
      m_Lock->Lock();//only one thread allocates the space for the 6 separate images
      if( this->latestTime< Object::GetMTime() )
        {
        AllocateImages() ;
        }
      m_Lock->Unlock();
      while( !m_SeparationDone )//All the threads copy the image until every voxel has been copied
        {
        SeparateImages() ;
        }
      //allocate the interpolator in the derivated class. This is only a virtual class that doesn't know the interpolator type
      m_Lock->Lock();//only one thread allocates the interpolators
      if( !m_AllocateInterpolatorsDone )
      {
        m_AllocateInterpolatorsDone = true ;
        AllocateInterpolator() ;
        for( int i = 0 ; i < 6 ; i++ )
        {
          m_Interpol[ i ]->SetInputImage( m_Image[ i ] ) ;
        }
      }
      m_Lock->Unlock();
      }
     //Once the image has been separated, we can just call the scalar itkInterpolators
    if( m_Interpol[0]->IsInsideBuffer( point) )
      {
      TensorDataType pixelValue ;
      for( int i = 0 ; i < 6 ; i++ )
        {
        pixelValue[ i ] = ( DataType ) m_Interpol[ i ]->Evaluate( point ) ;
        }
      return pixelValue ;
      }
    else
      {
      return this->m_DefaultPixel ;
      }
    }
  else
    {
    itkExceptionMacro( << "No InputImage Set" ) ;
    }  
}

}//end itk namespace

#endif
