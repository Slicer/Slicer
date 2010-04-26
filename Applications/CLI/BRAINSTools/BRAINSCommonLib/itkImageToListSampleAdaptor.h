/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkImageToListSampleAdaptor.h,v $
  Language:  C++
  Date:      $Date: 2009-05-22 12:55:00 $
  Version:   $Revision: 1.3 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkImageToListSampleAdaptor_h
#define __itkImageToListSampleAdaptor_h

#include <typeinfo>

#include "itkImage.h"
#include "itkPixelTraits.h"
#include "itkListSample.h"
#include "itkSmartPointer.h"
#include "itkImageRegionIterator.h"
#include "itkFixedArray.h"
#include "itkMacro.h"
#include "itkMeasurementVectorTraits.h"

namespace itk { 
namespace Statistics {

/** \class ImageToListSampleAdaptor
 *  \brief This class provides ListSample interface to ITK Image
 *
 * After calling SetImage( const Image * ) method to plug in the image object,
 * users can use Sample interfaces to access Image data. The resulting data
 * are a list of measurement vectors.  
 *
 * The measurment vector type is determined from the image pixel type. This class
 * handles images with scalar, fixed array or variable length vector pixel types. 
 * 
 *
 * \sa Sample, ListSample
 */

template < class TImage >
class ITK_EXPORT ImageToListSampleAdaptor : 
    public ListSample< typename MeasurementVectorPixelTraits< typename TImage::PixelType >::MeasurementVectorType >
{
public:
  /** Standard class typedefs */
  typedef ImageToListSampleAdaptor               Self;

  typedef ListSample< typename MeasurementVectorPixelTraits<
     typename TImage::PixelType >::MeasurementVectorType >
                                                 Superclass;

  typedef SmartPointer< Self >                   Pointer;
  typedef SmartPointer<const Self>               ConstPointer;
  
  /** Run-time type information (and related methods). */
  itkTypeMacro(ImageToListSampleAdaptor, ListSample);
  
  /** Method for creation through the object factory. */
  itkNewMacro(Self);
  
  /** Image typedefs */
  typedef TImage                                          ImageType;
  typedef typename ImageType::Pointer                     ImagePointer;
  typedef typename ImageType::ConstPointer                ImageConstPointer;
  typedef typename ImageType::IndexType                   IndexType;
  typedef typename ImageType::PixelType                   PixelType;
  typedef typename ImageType::PixelContainerConstPointer  PixelContainerConstPointer;
  
  /** Image Iterator typedef support */
  typedef ImageRegionIterator< ImageType >               ImageIteratorType; 
  typedef ImageRegionConstIterator< ImageType >          ImageConstIteratorType; 
  typedef PixelTraits< typename TImage::PixelType >      PixelTraitsType;

  /** Superclass typedefs for Measurement vector, measurement, 
   * Instance Identifier, frequency, size, size element value */
  typedef typename MeasurementVectorPixelTraits<
         PixelType >::MeasurementVectorType                    MeasurementVectorType; 
  typedef typename MeasurementVectorTraitsTypes< 
             MeasurementVectorType >::ValueType                MeasurementType;
  itkSuperclassTraitMacro( AbsoluteFrequencyType )
  itkSuperclassTraitMacro( TotalAbsoluteFrequencyType )
  itkSuperclassTraitMacro( MeasurementVectorSizeType )
  itkSuperclassTraitMacro( InstanceIdentifier )

  typedef MeasurementVectorType                                ValueType;

  /** Method to set the image */
  void SetImage(const TImage* image);

  /** Method to get the image */
  const TImage* GetImage() const;

  /** returns the number of measurement vectors in this container */
  InstanceIdentifier Size() const;

  /** method to return measurement vector for a specified id */
  virtual const MeasurementVectorType & GetMeasurementVector(InstanceIdentifier id) const;

  /** method to return frequency for a specified id */
  AbsoluteFrequencyType GetFrequency(InstanceIdentifier  id) const;

  /** method to return the total frequency */
  TotalAbsoluteFrequencyType GetTotalFrequency() const;

  /** Method to set UsePixelContainer flag */
  itkSetMacro( UsePixelContainer, bool );
  
  /** Method to get UsePixelContainer flag */
  itkGetConstMacro( UsePixelContainer, bool );

  /** Convenience methods to turn on/off the UsePixelContainer flag */
  itkBooleanMacro( UsePixelContainer );

  //  PrintSelf(std::ostream& os, Indent indent) const;  

  /** \class ListSample::ConstIterator */
  class ConstIterator
    {
    friend class ImageToListSampleAdaptor;
    public:

    ConstIterator( const ImageToListSampleAdaptor * adaptor )
      {
      *this = adaptor->Begin();
      }

    ConstIterator(const ConstIterator &iter)
      {
      m_Iter = iter.m_Iter;
      m_InstanceIdentifier = iter.m_InstanceIdentifier;
      }

    ConstIterator& operator=( const ConstIterator & iter )
      {
      m_Iter = iter.m_Iter;
      m_InstanceIdentifier = iter.m_InstanceIdentifier;
      return *this;
      }

    AbsoluteFrequencyType GetFrequency() const
      {
      return 1;
      }

    const MeasurementVectorType & GetMeasurementVector() const
      {
      MeasurementVectorTraits::Assign( this->m_MeasurementVectorCache, m_Iter.Get());
      return this->m_MeasurementVectorCache;
      }

    InstanceIdentifier GetInstanceIdentifier() const
      {
      return m_InstanceIdentifier;
      }

    ConstIterator& operator++()
      {
      ++m_Iter;
      ++m_InstanceIdentifier;
      return *this;
      }

    bool operator!=(const ConstIterator &it)
      {
      return (m_Iter != it.m_Iter);
      }

    bool operator==(const ConstIterator &it)
      {
      return (m_Iter == it.m_Iter);
      }

#if !(defined(_MSC_VER) && (_MSC_VER <= 1200))
  protected:
#endif
    // This method should only be available to the ListSample class
    ConstIterator(
      ImageConstIteratorType iter,
      InstanceIdentifier iid)
      {
      m_Iter = iter;
      m_InstanceIdentifier = iid;
      }

    // This method is purposely not implemented
    ConstIterator();

  private:
    ImageConstIteratorType            m_Iter;
    mutable MeasurementVectorType     m_MeasurementVectorCache;
    InstanceIdentifier                m_InstanceIdentifier;
    };

 /** \class ImageToListSampleAdaptor::Iterator */
  class Iterator : public ConstIterator
  {

  friend class ImageToListSampleAdaptor;

  public:

    Iterator(Self * adaptor):ConstIterator(adaptor)
      {
      }

    Iterator(const Iterator &iter):ConstIterator( iter )
      {
      }

    Iterator& operator =(const Iterator & iter)
      {
      this->ConstIterator::operator=( iter );
      return *this;
      }

#if !(defined(_MSC_VER) && (_MSC_VER <= 1200))
  protected:
#endif
    // To ensure const-correctness these method must not be in the public API.
    // The are purposly not implemented, since they should never be called.
    Iterator();
    Iterator(const Self * adaptor);
    Iterator(  ImageConstIteratorType iter, InstanceIdentifier iid);
    Iterator(const ConstIterator & it);
    ConstIterator& operator=(const ConstIterator& it);
    Iterator(
      ImageIteratorType iter,
      InstanceIdentifier iid):ConstIterator( iter, iid )
      {
      }

  private:
  };


  /** returns an iterator that points to the beginning of the container */
  Iterator Begin()
    {
    ImagePointer  nonConstImage = const_cast< ImageType* >(m_Image.GetPointer());
    ImageIteratorType imageIterator( nonConstImage, nonConstImage->GetLargestPossibleRegion());  
    Iterator iter(imageIterator.Begin(), 0);
    return iter;
    }

  /** returns an iterator that points to the end of the container */
  Iterator End()
    {
    ImagePointer  nonConstImage = const_cast< ImageType* >(m_Image.GetPointer());
    ImageIteratorType imageIterator( nonConstImage, nonConstImage->GetLargestPossibleRegion());  
    Iterator iter(imageIterator.End(), m_Image->GetPixelContainer()->Size());
    return iter;
    }


  /** returns an iterator that points to the beginning of the container */
  ConstIterator Begin() const
    {
    ImageConstIteratorType imageConstIterator( m_Image, m_Image->GetLargestPossibleRegion());  
    ConstIterator iter(imageConstIterator.Begin(), 0);
    return iter;
    }

  /** returns an iterator that points to the end of the container */
  ConstIterator End() const
    {
    ImageConstIteratorType imageConstIterator( m_Image, m_Image->GetLargestPossibleRegion());  
    ConstIterator iter(imageConstIterator.End(), m_Image->GetPixelContainer()->Size());
    return iter;
    }


protected:
  ImageToListSampleAdaptor();
  virtual ~ImageToListSampleAdaptor() {}
  void PrintSelf(std::ostream& os, Indent indent) const;  

private:
  ImageToListSampleAdaptor(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

  ImageConstPointer                  m_Image;
  mutable MeasurementVectorType      m_MeasurementVectorInternal;
  bool                               m_UsePixelContainer;

  PixelContainerConstPointer m_PixelContainer;

}; // end of class ImageToListSampleAdaptor

} // end of namespace Statistics
} // end of namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkImageToListSampleAdaptor.txx"
#endif

#endif
