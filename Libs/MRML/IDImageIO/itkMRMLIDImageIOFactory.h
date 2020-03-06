/*=========================================================================
 *
 *  Copyright NumFOCUS
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/
#ifndef itkMRMLIDImageIOFactory_h
#define itkMRMLIDImageIOFactory_h

#include "itkObjectFactoryBase.h"
#include "itkImageIOBase.h"

#include "itkMRMLIDImageIO.h"

#include "itkMRMLIDIOExport.h"

namespace itk
{
/** \class MRMLIDImageIOFactory
 * \brief Create instances of MRMLIDImageIO objects using an object factory.
 */
class MRMLIDImageIO_EXPORT MRMLIDImageIOFactory : public ObjectFactoryBase
{
public:
  /** Standard class typedefs. */
  typedef MRMLIDImageIOFactory      Self;
  typedef ObjectFactoryBase         Superclass;
  typedef SmartPointer<Self>        Pointer;
  typedef SmartPointer<const Self>  ConstPointer;

  /** Class methods used to interface with the registered factories. */
  const char* GetITKSourceVersion() const override;
  const char* GetDescription() const override;

  /** Method for class instantiation. */
  itkFactorylessNewMacro(Self);
  static MRMLIDImageIOFactory* FactoryNew() { return new MRMLIDImageIOFactory;}

  /** Run-time type information (and related methods). */
  itkTypeMacro(MRMLIDImageIOFactory, ObjectFactoryBase);

  /** Register one factory of this type  */
  static void RegisterOneFactory()
  {
    MRMLIDImageIOFactory::Pointer nrrdFactory = MRMLIDImageIOFactory::New();
    ObjectFactoryBase::RegisterFactory(nrrdFactory);
  }

protected:
  MRMLIDImageIOFactory();
  ~MRMLIDImageIOFactory() override;

private:
  MRMLIDImageIOFactory(const Self&) = delete;
  void operator=(const Self&) = delete;

};


} /// end namespace itk

#endif
