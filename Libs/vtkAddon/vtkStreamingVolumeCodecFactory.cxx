/*==============================================================================

Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
Queen's University, Kingston, ON, Canada. All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
and was supported through CANARIE's Research Software Program, and Cancer
Care Ontario.

==============================================================================*/

// vtkAddon includes
#include "vtkRawRGBVolumeCodec.h"
#include "vtkStreamingVolumeCodecFactory.h"

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkDataObject.h>

// STD includes
#include <algorithm>

//----------------------------------------------------------------------------
// The compression codec manager singleton.
// This MUST be default initialized to zero by the compiler and is
// therefore not initialized here.  The ClassInitialize and ClassFinalize methods handle this instance.
static vtkStreamingVolumeCodecFactory* vtkStreamingVolumeCodecFactoryInstance;


//----------------------------------------------------------------------------
// Must NOT be initialized.  Default initialization to zero is necessary.
unsigned int vtkStreamingVolumeCodecFactoryInitialize::Count;

//----------------------------------------------------------------------------
// Implementation of vtkStreamingVolumeCodecFactoryInitialize class.
//----------------------------------------------------------------------------
vtkStreamingVolumeCodecFactoryInitialize::vtkStreamingVolumeCodecFactoryInitialize()
{
  if(++Self::Count == 1)
    {
    vtkStreamingVolumeCodecFactory::classInitialize();
    }
}

//----------------------------------------------------------------------------
vtkStreamingVolumeCodecFactoryInitialize::~vtkStreamingVolumeCodecFactoryInitialize()
{
  if(--Self::Count == 0)
    {
    vtkStreamingVolumeCodecFactory::classFinalize();
    }
}

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// Up the reference count so it behaves like New
vtkStreamingVolumeCodecFactory* vtkStreamingVolumeCodecFactory::New()
{
  vtkStreamingVolumeCodecFactory* ret = vtkStreamingVolumeCodecFactory::GetInstance();
  ret->Register(NULL);
  return ret;
}

//----------------------------------------------------------------------------
// Return the single instance of the vtkStreamingVolumeCodecFactory
vtkStreamingVolumeCodecFactory* vtkStreamingVolumeCodecFactory::GetInstance()
{
  if(!vtkStreamingVolumeCodecFactoryInstance)
    {
    // Try the factory first
    vtkStreamingVolumeCodecFactoryInstance = (vtkStreamingVolumeCodecFactory*)vtkObjectFactory::CreateInstance("vtkStreamingVolumeCodecFactory");
    // if the factory did not provide one, then create it here
    if(!vtkStreamingVolumeCodecFactoryInstance)
      {
      vtkStreamingVolumeCodecFactoryInstance = new vtkStreamingVolumeCodecFactory;
#ifdef VTK_HAS_INITIALIZE_OBJECT_BASE
      vtkStreamingVolumeCodecFactoryInstance->InitializeObjectBase();
#endif
      }
    }
  // return the instance
  return vtkStreamingVolumeCodecFactoryInstance;
}

//----------------------------------------------------------------------------
vtkStreamingVolumeCodecFactory::vtkStreamingVolumeCodecFactory()
{
}

//----------------------------------------------------------------------------
vtkStreamingVolumeCodecFactory::~vtkStreamingVolumeCodecFactory()
{
}

//----------------------------------------------------------------------------
void vtkStreamingVolumeCodecFactory::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkStreamingVolumeCodecFactory::classInitialize()
{
  // Allocate the singleton
  vtkStreamingVolumeCodecFactoryInstance = vtkStreamingVolumeCodecFactory::GetInstance();

  vtkStreamingVolumeCodecFactoryInstance->RegisterStreamingCodec(vtkSmartPointer<vtkRawRGBVolumeCodec>::New());
}

//----------------------------------------------------------------------------
void vtkStreamingVolumeCodecFactory::classFinalize()
{
  vtkStreamingVolumeCodecFactoryInstance->Delete();
  vtkStreamingVolumeCodecFactoryInstance = 0;
}

//----------------------------------------------------------------------------
bool vtkStreamingVolumeCodecFactory::RegisterStreamingCodec(vtkSmartPointer<vtkStreamingVolumeCodec> codec)
{
  for (unsigned int i = 0; i < this->RegisteredCodecs.size(); ++i)
    {
    if (strcmp(this->RegisteredCodecs[i]->GetClassName(), codec->GetClassName())==0)
      {
      vtkWarningMacro("RegisterStreamingCodec failed: codec is already registered");
      return false;
      }
    }
  this->RegisteredCodecs.push_back(codec);
  return true;
}

//----------------------------------------------------------------------------
bool vtkStreamingVolumeCodecFactory::UnRegisterStreamingCodecByClassName(const std::string&  codecClassName)
{
  std::vector<vtkSmartPointer<vtkStreamingVolumeCodec> >::iterator codecIt;
  for (codecIt = this->RegisteredCodecs.begin(); codecIt != this->RegisteredCodecs.end(); ++codecIt)
    {
    vtkSmartPointer<vtkStreamingVolumeCodec> codec = *codecIt;
    if (strcmp(codec->GetClassName(), codecClassName.c_str())==0)
      {
      this->RegisteredCodecs.erase(codecIt);
      return true;
      }
    }
  vtkWarningMacro("UnRegisterStreamingCodecByClassName failed: codec not found");
  return false;
}

//----------------------------------------------------------------------------
 vtkStreamingVolumeCodec* vtkStreamingVolumeCodecFactory::CreateCodecByClassName(const std::string& codecClassName)
{
  std::vector<vtkSmartPointer<vtkStreamingVolumeCodec> >::iterator codecIt;
  for (codecIt = this->RegisteredCodecs.begin(); codecIt != this->RegisteredCodecs.end(); ++codecIt)
    {
    vtkSmartPointer<vtkStreamingVolumeCodec> codec = *codecIt;
    if (strcmp(codec->GetClassName(), codecClassName.c_str())==0)
      {
      return codec->CreateCodecInstance();
      }
    }
  return NULL;
}

//----------------------------------------------------------------------------
vtkStreamingVolumeCodec* vtkStreamingVolumeCodecFactory::CreateCodecByFourCC(const std::string fourCC)
{
  std::vector<vtkSmartPointer<vtkStreamingVolumeCodec> >::iterator codecIt;
  for (codecIt = this->RegisteredCodecs.begin(); codecIt != this->RegisteredCodecs.end(); ++codecIt)
    {
    vtkSmartPointer<vtkStreamingVolumeCodec> codec = *codecIt;
    if (codec->GetFourCC() == fourCC)
      {
      return codec->CreateCodecInstance();
      }
    }
  return NULL;
}

//----------------------------------------------------------------------------
const std::vector<std::string> vtkStreamingVolumeCodecFactory::GetStreamingCodecClassNames()
{
  std::vector<std::string> codecClassNames;
  std::vector<vtkSmartPointer<vtkStreamingVolumeCodec> >::iterator codecIt;
  for (codecIt = this->RegisteredCodecs.begin(); codecIt != this->RegisteredCodecs.end(); ++codecIt)
    {
    vtkSmartPointer<vtkStreamingVolumeCodec> codec = *codecIt;
    codecClassNames.push_back(codec->GetClassName());
    }
  return codecClassNames;
}

//----------------------------------------------------------------------------
std::vector<std::string> vtkStreamingVolumeCodecFactory::GetStreamingCodecFourCCs()
{
  std::vector<std::string> codecFourCCs;
  std::vector<vtkSmartPointer<vtkStreamingVolumeCodec> >::iterator codecIt;
  for (codecIt = this->RegisteredCodecs.begin(); codecIt != this->RegisteredCodecs.end(); ++codecIt)
    {
    vtkSmartPointer<vtkStreamingVolumeCodec> codec = *codecIt;
    codecFourCCs.push_back(codec->GetFourCC());
    }
  return codecFourCCs;
}
