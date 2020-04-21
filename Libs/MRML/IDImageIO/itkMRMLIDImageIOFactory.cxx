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
#include "itkMRMLIDImageIOFactory.h"
#include "itkVersion.h"


namespace itk
{
MRMLIDImageIOFactory::MRMLIDImageIOFactory()
{
  this->RegisterOverride("itkImageIOBase",
                         "itkMRMLIDImageIO",
                         "ImageIO to communicate directly with a MRML scene.",
                         true,
                         CreateObjectFunction<MRMLIDImageIO>::New());
}

MRMLIDImageIOFactory::~MRMLIDImageIOFactory() = default;

const char* MRMLIDImageIOFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char*
MRMLIDImageIOFactory::GetDescription() const
{
  return "ImageIOFactory that imports/exports data to a MRML node.";
}

} // end namespace itk
