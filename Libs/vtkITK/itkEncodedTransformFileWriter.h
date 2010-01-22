/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkEncodedTransformFileWriter.h,v $
  Language:  C++
  Date:      $Date: 2007-08-10 15:43:28 $
  Version:   $Revision: 1.8 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkEncodedTransformFileWriter_h
#define __itkEncodedTransformFileWriter_h


#include "itkTransformFileWriter.h"
#include "vtkITK.h"

namespace itk
{

class VTK_ITK_EXPORT EncodedTransformFileWriter : public TransformFileWriter
{
public:

  /** SmartPointer typedef support */
  typedef EncodedTransformFileWriter    Self;
  typedef SmartPointer<Self>     Pointer;

  /** Method for creation through the object factory */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  typedef TransformFileWriter Superclass;
  itkTypeMacro(EncodedTransformFileWriter, TransformFileWriter);
  typedef Superclass::TransformType          TransformType;
  typedef Superclass::TransformPointer TransformPointer;

  /** Set/Get the input transform to write */
  void SetInput(const TransformType* transform);
  const TransformType * GetInput() {return *(m_TransformList.begin());}

  /** Add a transform to be written */
  void AddTransform(const TransformType* transform);

  /** Write out the transform */
  void Update();

protected:
  EncodedTransformFileWriter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented
   
  EncodedTransformFileWriter();
  virtual ~EncodedTransformFileWriter();

private:
  std::list<const TransformType*>  m_TransformList;
};

} /// namespace itk

#endif /// __itkEncodedTransformFileWriter_h
