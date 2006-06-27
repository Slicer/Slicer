/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkMRMLIDImageIOFactory.cxx,v $
  Language:  C++
  Date:      $Date: 2004/07/15 16:26:40 $
  Version:   $Revision: 1.1 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "itkMRMLIDImageIOFactory.h"
#include "itkCreateObjectFunction.h"
#include "itkMRMLIDImageIO.h"
#include "itkVersion.h"

  
namespace itk
{
MRMLIDImageIOFactory::MRMLIDImageIOFactory()
{
  this->RegisterOverride("itkImageIOBase",
                         "itkMRMLIDImageIO",
                         "Command IO",
                         1,
                         CreateObjectFunction<MRMLIDImageIO>::New());
}
  
MRMLIDImageIOFactory::~MRMLIDImageIOFactory()
{
}

const char* 
MRMLIDImageIOFactory::GetITKSourceVersion(void) const
{
  return ITK_SOURCE_VERSION;
}

const char* 
MRMLIDImageIOFactory::GetDescription() const
{
  return "ImageIOFactory that imports/exports data to a MRML node.";
}

} // end namespace itk

