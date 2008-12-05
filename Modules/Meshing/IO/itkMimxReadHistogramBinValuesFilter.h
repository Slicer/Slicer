/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: itkMimxReadHistogramBinValuesFilter.h,v $
Language:  C++
Date:      $Date: 2007/05/10 16:32:38 $
Version:   $Revision: 1.3 $

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

#ifndef __itkReadHistogramBinValuesFilter_h
#define __itkReadHistogramBinValuesFilter_h

#include "vnl/vnl_matrix_fixed.h"
#include "itkArray.h"
#include "itkExceptionObject.h"
#include <string>
#include <sstream> 
#include <exception> 

namespace itk
{

/**
 * \class ReadHistogramBinValuesFilter
 * This class reads the modulus values from a user specified file
 * and populates the bins array.
 */

template <class TTraitType>
class ReadHistogramBinValuesFilter : public LightProcessObject
{
public:
  /** Standard "Self" typedef. */
  typedef ReadHistogramBinValuesFilter         Self;

  typedef SmartPointer<Self>  Pointer;
  typedef SmartPointer<const Self>  ConstPointer;

  /** Method for creation through the object factory */
  itkNewMacro(Self);

  typedef double                 TraitType;
  typedef itk::Array<TraitType>  ArrayType;

  /* SetInput and GetOutput Macros */ 
  itkSetStringMacro( BinFileName );
  itkGetStringMacro( BinFileName );

  itkSetMacro( BinLowerBound, float );
  itkSetMacro( BinUpperBound, float );

  void Update( );
  int GetNumberOfBins( );
  ArrayType GetHistogramBins( );

protected:

  ReadHistogramBinValuesFilter( );
  virtual ~ReadHistogramBinValuesFilter( );

private:
  ReadHistogramBinValuesFilter(const Self&); //purposely not implemented
  void operator = ( const Self& ); //purposely not implemented
  
  std::string            m_BinFileName;
  int                           m_NumberOfBins;
  TraitType              m_BinLowerBound;;
  TraitType              m_BinUpperBound;
  ArrayType              m_binValue;
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkReadHistogramBinValuesFilter.txx"
#endif

#endif
