/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   ITKCommandIO
  Module:    $HeadURL$
  Date:      $Date$
  Version:   $Revision$

==========================================================================*/
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

