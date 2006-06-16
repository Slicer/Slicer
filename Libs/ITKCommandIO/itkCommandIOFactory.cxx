/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkCommandIOFactory.cxx,v $
  Language:  C++
  Date:      $Date: 2004/07/15 16:26:40 $
  Version:   $Revision: 1.1 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "itkCommandIOFactory.h"
#include "itkCreateObjectFunction.h"
#include "itkCommandIO.h"
#include "itkVersion.h"

  
namespace itk
{
CommandIOFactory::CommandIOFactory()
{
  this->RegisterOverride("itkImageIOBase",
                         "itkCommandIO",
                         "Command IO",
                         1,
                         CreateObjectFunction<CommandIO>::New());
}
  
CommandIOFactory::~CommandIOFactory()
{
}

const char* 
CommandIOFactory::GetITKSourceVersion(void) const
{
  return ITK_SOURCE_VERSION;
}

const char* 
CommandIOFactory::GetDescription() const
{
  return "Command ImageIO Factory, allows the loading of Nrrd images into insight";
}

} // end namespace itk

