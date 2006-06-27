/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkMRMLIDImageIOFactory.h,v $
  Language:  C++
  Date:      $Date: 2004/07/15 16:26:40 $
  Version:   $Revision: 1.1 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkMRMLIDImageIOFactory_h
#define __itkMRMLIDImageIOFactory_h

#include "itkObjectFactoryBase.h"
#include "itkImageIOBase.h"

#include "itkMRMLIDImageIO.h"

#include "itkMRMLIDIOWin32Header.h"

namespace itk
{
/** \class MRMLIDImageIOFactory
 * \brief Create instances of MRMLIDImageIO objects using an object factory.
 */
class MRMLIDImageIO_EXPORT MRMLIDImageIOFactory : public ObjectFactoryBase
{
public:  
  /** Standard class typedefs. */
  typedef MRMLIDImageIOFactory   Self;
  typedef ObjectFactoryBase  Superclass;
  typedef SmartPointer<Self>  Pointer;
  typedef SmartPointer<const Self>  ConstPointer;
  
  /** Class methods used to interface with the registered factories. */
  virtual const char* GetITKSourceVersion(void) const;
  virtual const char* GetDescription(void) const;
  
  /** Method for class instantiation. */
  itkFactorylessNewMacro(Self);
  static MRMLIDImageIOFactory* FactoryNew() { return new MRMLIDImageIOFactory;}

  /** Run-time type information (and related methods). */
  itkTypeMacro(MRMLIDImageIOFactory, ObjectFactoryBase);

  /** Register one factory of this type  */
  static void RegisterOneFactory(void)
  {
    MRMLIDImageIOFactory::Pointer nrrdFactory = MRMLIDImageIOFactory::New();
    ObjectFactoryBase::RegisterFactory(nrrdFactory);
  }

protected:
  MRMLIDImageIOFactory();
  ~MRMLIDImageIOFactory();

private:
  MRMLIDImageIOFactory(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

};
  
  
} // end namespace itk

#endif
