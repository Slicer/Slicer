/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkCommandIOFactory.h,v $
  Language:  C++
  Date:      $Date: 2004/07/15 16:26:40 $
  Version:   $Revision: 1.1 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkCommandIOFactory_h
#define __itkCommandIOFactory_h

#include "itkObjectFactoryBase.h"
#include "itkImageIOBase.h"

#include "itkCommandIO.h"

#include "itkCommandIOWin32Header.h"

namespace itk
{
/** \class CommandIOFactory
 * \brief Create instances of CommandIO objects using an object factory.
 */
class ITKCOMMANDIO_EXPORT CommandIOFactory : public ObjectFactoryBase
{
public:  
  /** Standard class typedefs. */
  typedef CommandIOFactory   Self;
  typedef ObjectFactoryBase  Superclass;
  typedef SmartPointer<Self>  Pointer;
  typedef SmartPointer<const Self>  ConstPointer;
  
  /** Class methods used to interface with the registered factories. */
  virtual const char* GetITKSourceVersion(void) const;
  virtual const char* GetDescription(void) const;
  
  /** Method for class instantiation. */
  itkFactorylessNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(CommandIOFactory, ObjectFactoryBase);

  /** Register one factory of this type  */
  static void RegisterOneFactory(void)
  {
    CommandIOFactory::Pointer nrrdFactory = CommandIOFactory::New();
    ObjectFactoryBase::RegisterFactory(nrrdFactory);
  }

protected:
  CommandIOFactory();
  ~CommandIOFactory();

private:
  CommandIOFactory(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

};
  
  
} // end namespace itk

#endif
