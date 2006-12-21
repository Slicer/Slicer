/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   ITKCommandIO
  Module:    $HeadURL$
  Date:      $Date$
  Version:   $Revision$

==========================================================================*/

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
class CommandIO_EXPORT CommandIOFactory : public ObjectFactoryBase
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
  static CommandIOFactory* FactoryNew() { return new CommandIOFactory;}

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
