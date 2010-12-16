/*=========================================================================
 *
 *  Program:   Insight Segmentation & Registration Toolkit
 *  Module:    $RCSfile: itkScalarImagePortionToHistogramGenerator.h,v $
 *  Language:  C++
 *  Date:      $Date: 2009-08-08 14:18:12 $
 *  Version:   $Revision: 1.2 $
 *
 *  Copyright (c) Insight Software Consortium. All rights reserved.
 *  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even
 *  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the above copyright notices for more information.
 *
 *  =========================================================================*/
#ifndef __itkScalarImagePortionToHistogramGenerator_h
#define __itkScalarImagePortionToHistogramGenerator_h

#include "itkImageToListSampleFilter.h"
#include "itkSampleToHistogramFilter.h"

#include "itkHistogram.h"
#include "itkObject.h"

namespace itk
{
namespace Statistics
{
/** \class ScalarImagePortionToHistogramGenerator
  *
  * \brief TODO
  */
template< class TImageType, class TMaskType >
class ScalarImagePortionToHistogramGenerator:public Object
{
public:
  /** Standard typedefs */
  typedef ScalarImagePortionToHistogramGenerator Self;
  typedef Object                                 Superclass;
  typedef SmartPointer< Self >                   Pointer;
  typedef SmartPointer< const Self >             ConstPointer;

  /** Run-time type information (and related methods). */
  itkTypeMacro(ScalarImagePortionToHistogramGenerator, Object);

  /** standard New() method support */
  itkNewMacro(Self);

  typedef TImageType                                    ImageType;
  typedef typename TImageType::PixelType                PixelType;
  typedef typename NumericTraits< PixelType >::RealType RealPixelType;

  typedef itk::Statistics::Histogram< double > HistogramType;
  typedef typename HistogramType::Pointer      HistogramPointer;
  typedef typename HistogramType::ConstPointer HistogramConstPointer;

  typedef itk::Statistics::ImageToListSampleFilter< ImageType, TMaskType > ListSampleGeneratorType;
  typedef typename ListSampleGeneratorType::Pointer                        ListSampleGeneratorPointer;
  typedef typename ListSampleGeneratorType::ListSampleType                 ListSampleType;

  typedef itk::Statistics::SampleToHistogramFilter< ListSampleType, HistogramType > GeneratorType;
  typedef typename GeneratorType::Pointer                                           GeneratorPointer;
public:

  /** Triggers the Computation of the histogram */
  void Compute(void);

  /** Connects the input image for which the histogram is going to be computed
    */
  void SetInput(const TImageType *);

  /** Connects the input image for which the histogram is going to be computed
    */
  void SetBinaryPortionImage(const TMaskType *);

  /** Return the histogram.
    * \warning This output is only valid after the Compute() method has been
    *    invoked
    * \sa Compute */
  const HistogramType * GetOutput() const;

  /** Set number of histogram bins */
  void SetNumberOfBins(unsigned int numberOfBins);

  /** Set marginal scale value to be passed to the histogram generator */
  void SetMarginalScale(double marginalScale);

  /** Set the minimum value from which the bins will be computed */
  void SetHistogramMin(RealPixelType minimumValue);

  /** Set the maximum value from which the bins will be computed */
  void SetHistogramMax(RealPixelType maximumValue);

protected:
  ScalarImagePortionToHistogramGenerator();
  virtual ~ScalarImagePortionToHistogramGenerator() {}
  void PrintSelf(std::ostream & os, Indent indent) const;

private:

  ListSampleGeneratorPointer m_ImageToListSampleGenerator;

  HistogramPointer m_Histogram;
  GeneratorPointer m_HistogramGenerator;

  ScalarImagePortionToHistogramGenerator(const Self &); // purposely not
                                                        // implemented
  void operator=(const Self &);                         // purposely not

  // implemented
};
}   // end of namespace Statistics
} // end of namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkScalarImagePortionToHistogramGenerator.txx"
#endif

#endif
