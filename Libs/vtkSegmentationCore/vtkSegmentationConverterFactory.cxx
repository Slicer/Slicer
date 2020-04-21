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

  This file was originally developed by Andras Lasso, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#include "vtkSegmentationConverterFactory.h"

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkDataObject.h>

// SegmentationCore includes
#include "vtkSegmentationConverterRule.h"

//----------------------------------------------------------------------------
// The segmentation converter rule manager singleton.
// This MUST be default initialized to zero by the compiler and is
// therefore not initialized here.  The ClassInitialize and ClassFinalize methods handle this instance.
static vtkSegmentationConverterFactory* vtkSegmentationConverterFactoryInstance;

//----------------------------------------------------------------------------
// Must NOT be initialized.  Default initialization to zero is necessary.
unsigned int vtkSegmentationConverterFactoryInitialize::Count;

//----------------------------------------------------------------------------
// Implementation of vtkSegmentationConverterFactoryInitialize class.
//----------------------------------------------------------------------------
vtkSegmentationConverterFactoryInitialize::vtkSegmentationConverterFactoryInitialize()
{
  if(++Self::Count == 1)
    {
    vtkSegmentationConverterFactory::classInitialize();
    }
}

//----------------------------------------------------------------------------
vtkSegmentationConverterFactoryInitialize::~vtkSegmentationConverterFactoryInitialize()
{
  if(--Self::Count == 0)
    {
    vtkSegmentationConverterFactory::classFinalize();
    }
}

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Up the reference count so it behaves like New
vtkSegmentationConverterFactory* vtkSegmentationConverterFactory::New()
{
  vtkSegmentationConverterFactory* ret = vtkSegmentationConverterFactory::GetInstance();
  ret->Register(nullptr);
  return ret;
}

//----------------------------------------------------------------------------
// Return the single instance of the vtkSegmentationConverterFactory
vtkSegmentationConverterFactory* vtkSegmentationConverterFactory::GetInstance()
{
  if(!vtkSegmentationConverterFactoryInstance)
    {
    // Try the factory first
    vtkSegmentationConverterFactoryInstance = (vtkSegmentationConverterFactory*)vtkObjectFactory::CreateInstance("vtkSegmentationConverterFactory");
    // if the factory did not provide one, then create it here
    if(!vtkSegmentationConverterFactoryInstance)
      {
      vtkSegmentationConverterFactoryInstance = new vtkSegmentationConverterFactory;
#ifdef VTK_HAS_INITIALIZE_OBJECT_BASE
      vtkSegmentationConverterFactoryInstance->InitializeObjectBase();
#endif
      }
    }
  // return the instance
  return vtkSegmentationConverterFactoryInstance;
}

//----------------------------------------------------------------------------
vtkSegmentationConverterFactory::vtkSegmentationConverterFactory() = default;

//----------------------------------------------------------------------------
vtkSegmentationConverterFactory::~vtkSegmentationConverterFactory() = default;

//----------------------------------------------------------------------------
void vtkSegmentationConverterFactory::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkSegmentationConverterFactory::classInitialize()
{
  // Allocate the singleton
  vtkSegmentationConverterFactoryInstance = vtkSegmentationConverterFactory::GetInstance();
}

//----------------------------------------------------------------------------
void vtkSegmentationConverterFactory::classFinalize()
{
  vtkSegmentationConverterFactoryInstance->Delete();
  vtkSegmentationConverterFactoryInstance = nullptr;
}

//----------------------------------------------------------------------------
void vtkSegmentationConverterFactory::RegisterConverterRule(vtkSegmentationConverterRule* rule)
{
  if (!rule)
  {
    vtkErrorMacro("RegisterConverterRule failed: invalid input rule");
    return;
  }

  this->Rules.push_back(rule);
}

//----------------------------------------------------------------------------
void vtkSegmentationConverterFactory::UnregisterConverterRule(vtkSegmentationConverterRule* rule)
{
  for (RuleListType::iterator ruleIt = this->Rules.begin(); ruleIt != this->Rules.end(); ++ruleIt)
  {
    if (ruleIt->GetPointer() == rule)
    {
      // Found
      this->Rules.erase(ruleIt);
      return;
    }
  }
  vtkWarningMacro("UnregisterConverterRule failed: rule not found");
}

//----------------------------------------------------------------------------
void vtkSegmentationConverterFactory::CopyConverterRules(RuleListType &rules)
{
  rules.clear();
  for (RuleListType::iterator ruleIt = this->Rules.begin(); ruleIt != this->Rules.end(); ++ruleIt)
  {
    vtkSmartPointer<vtkSegmentationConverterRule> rule = vtkSmartPointer<vtkSegmentationConverterRule>::Take((*ruleIt)->Clone());
    rules.push_back(rule);
  }
}

//----------------------------------------------------------------------------
const vtkSegmentationConverterFactory::RuleListType& vtkSegmentationConverterFactory::GetConverterRules()
{
  return this->Rules;
}

//----------------------------------------------------------------------------
bool vtkSegmentationConverterFactory::DisableConverterRule(std::string sourceRepresentationName, std::string targetRepresentationName)
{
  bool result = false;
  RuleListType rulesCopy = this->Rules;
  for (RuleListType::iterator ruleIt = rulesCopy.begin(); ruleIt != rulesCopy.end(); ++ruleIt)
  {
    if ( !sourceRepresentationName.compare(ruleIt->GetPointer()->GetSourceRepresentationName())
      && !targetRepresentationName.compare(ruleIt->GetPointer()->GetTargetRepresentationName()) )
    {
      this->UnregisterConverterRule(ruleIt->GetPointer());
      result = true;
    }
  }
  return result;
}

//----------------------------------------------------------------------------
void vtkSegmentationConverterFactory::DisableRepresentation(std::string representationName)
{
  RuleListType rulesCopy = this->Rules;
  for (RuleListType::iterator ruleIt = rulesCopy.begin(); ruleIt != rulesCopy.end(); ++ruleIt)
  {
    if ( !representationName.compare(ruleIt->GetPointer()->GetSourceRepresentationName())
      || !representationName.compare(ruleIt->GetPointer()->GetTargetRepresentationName()) )
    {
      this->UnregisterConverterRule(ruleIt->GetPointer());
    }
  }
}

//----------------------------------------------------------------------------
vtkDataObject* vtkSegmentationConverterFactory::ConstructRepresentationObjectByClass(std::string className)
{
  for (RuleListType::iterator ruleIt = this->Rules.begin(); ruleIt != this->Rules.end(); ++ruleIt)
  {
    vtkDataObject* representationObject = (*ruleIt)->ConstructRepresentationObjectByClass(className);
    if (representationObject)
    {
      return representationObject;
    }
  }

  // None of the registered rules can instantiate this type
  return nullptr;
}

//----------------------------------------------------------------------------
vtkDataObject* vtkSegmentationConverterFactory::ConstructRepresentationObjectByRepresentation(std::string representationName)
{
  for (RuleListType::iterator ruleIt = this->Rules.begin(); ruleIt != this->Rules.end(); ++ruleIt)
  {
    vtkDataObject* representationObject = (*ruleIt)->ConstructRepresentationObjectByRepresentation(representationName);
    if (representationObject)
    {
      return representationObject;
    }
  }

  // None of the registered rules can instantiate this type
  return nullptr;
}
