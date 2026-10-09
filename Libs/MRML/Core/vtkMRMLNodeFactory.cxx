/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#include "vtkMRMLNodeFactory.h"

// MRML includes
#include "vtkMRMLBSplineTransformNode.h"
#include "vtkMRMLCameraNode.h"
#include "vtkMRMLClipModelsNode.h"
#include "vtkMRMLClipNode.h"
#include "vtkMRMLColorTableNode.h"
#include "vtkMRMLColorTableStorageNode.h"
#include "vtkMRMLCrosshairNode.h"
#include "vtkMRMLDiffusionTensorDisplayPropertiesNode.h"
#include "vtkMRMLDiffusionWeightedVolumeDisplayNode.h"
#include "vtkMRMLDiffusionWeightedVolumeNode.h"
#include "vtkMRMLDisplayableHierarchyNode.h"
#include "vtkMRMLFolderDisplayNode.h"
#include "vtkMRMLGridTransformNode.h"
#include "vtkMRMLHierarchyNode.h"
#include "vtkMRMLHierarchyStorageNode.h"
#include "vtkMRMLInteractionNode.h"
#include "vtkMRMLLabelMapVolumeDisplayNode.h"
#include "vtkMRMLLabelMapVolumeNode.h"
#include "vtkMRMLLayoutNode.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLLinearTransformSequenceStorageNode.h"
#include "vtkMRMLMarkupsAngleNode.h"
#include "vtkMRMLMarkupsClosedCurveNode.h"
#include "vtkMRMLMarkupsCurveNode.h"
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLMarkupsFiducialDisplayNode.h"
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkMRMLMarkupsFiducialStorageNode.h"
#include "vtkMRMLMarkupsLineNode.h"
#include "vtkMRMLMarkupsPlaneDisplayNode.h"
#include "vtkMRMLMarkupsPlaneNode.h"
#include "vtkMRMLMarkupsROIDisplayNode.h"
#include "vtkMRMLMarkupsROINode.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLModelHierarchyNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLModelStorageNode.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLPlotChartNode.h"
#include "vtkMRMLPlotSeriesNode.h"
#include "vtkMRMLPlotViewNode.h"
#include "vtkMRMLProceduralColorNode.h"
#include "vtkMRMLProceduralColorStorageNode.h"
#include "vtkMRMLROIListNode.h"
#include "vtkMRMLROINode.h"
#include "vtkMRMLScalarVolumeDisplayNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLScriptedModuleNode.h"
#include "vtkMRMLSegmentationDisplayNode.h"
#include "vtkMRMLSegmentationNode.h"
#include "vtkMRMLSegmentationStorageNode.h"
#include "vtkMRMLSelectionNode.h"
#include "vtkMRMLSequenceNode.h"
#include "vtkMRMLSequenceStorageNode.h"
#include "vtkMRMLSliceCompositeNode.h"
#include "vtkMRMLSliceDisplayNode.h"
#include "vtkMRMLSliceNode.h"
#include "vtkMRMLSnapshotClipNode.h"
#include "vtkMRMLStorageNode.h"
#include "vtkMRMLSubjectHierarchyNode.h"
#include "vtkMRMLTableNode.h"
#include "vtkMRMLTableStorageNode.h"
#include "vtkMRMLTableViewNode.h"
#include "vtkMRMLTextNode.h"
#include "vtkMRMLTextStorageNode.h"
#include "vtkMRMLTransformDisplayNode.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLTransformSequenceStorageNode.h"
#include "vtkMRMLTransformStorageNode.h"
#include "vtkMRMLVectorVolumeDisplayNode.h"
#include "vtkMRMLViewNode.h"
#include "vtkMRMLVolumeArchetypeStorageNode.h"
#include "vtkMRMLVolumeSequenceStorageNode.h"

#ifdef MRML_USE_vtkTeem
# include "vtkMRMLDiffusionTensorVolumeDisplayNode.h"
# include "vtkMRMLDiffusionTensorVolumeNode.h"
# include "vtkMRMLDiffusionTensorVolumeSliceDisplayNode.h"
# include "vtkMRMLNRRDStorageNode.h"
# include "vtkMRMLStreamingVolumeNode.h"
# include "vtkMRMLVectorVolumeNode.h"
#endif

// VTK includes
#include <vtkDebugLeaks.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

// STD includes
#include <cstring>

//------------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLNodeFactory);

//------------------------------------------------------------------------------
vtkMRMLNodeFactory::vtkMRMLNodeFactory() = default;

//------------------------------------------------------------------------------
vtkMRMLNodeFactory::~vtkMRMLNodeFactory()
{
  for (vtkMRMLNode* node : this->RegisteredNodeClasses)
  {
    node->Delete();
  }
  this->RegisteredNodeClasses.clear();
  this->RegisteredNodeTags.clear();
}

//------------------------------------------------------------------------------
void vtkMRMLNodeFactory::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Registered node classes:\n";
  for (vtkMRMLNode* node : this->RegisteredNodeClasses)
  {
    os << indent.GetNextIndent() << "Class name = " << node->GetClassName() << endl;
    if (node->IsA("vtkMRMLStorageNode"))
    {
      vtkMRMLStorageNode* snode = vtkMRMLStorageNode::SafeDownCast(node);
      const char* exts = snode->GetDefaultWriteFileExtension();
      os << indent.GetNextIndent().GetNextIndent() << "Default write extension = " << (exts != nullptr ? exts : "NULL") << endl;
    }
  }

  os << indent << "Registered abstract node classes:\n";
  for (const auto& nodeClassNameTypeDisplayName : this->RegisteredAbstractNodeClassTypeDisplayNames)
  {
    os << indent.GetNextIndent() << nodeClassNameTypeDisplayName.first << ": " << nodeClassNameTypeDisplayName.second << endl;
  }
}

//------------------------------------------------------------------------------
void vtkMRMLNodeFactory::RegisterCoreNodeClasses()
{
  // A prototype instance of each class is created and registered.
  // The factory keeps a reference to the prototype, the smart pointer releases its reference immediately.
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLBSplineTransformNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLCameraNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLClipModelsNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLClipNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLColorTableNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLColorTableStorageNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLCrosshairNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLDiffusionTensorDisplayPropertiesNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLDiffusionWeightedVolumeDisplayNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLDiffusionWeightedVolumeNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLDisplayableHierarchyNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLFolderDisplayNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLGridTransformNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLHierarchyNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLHierarchyStorageNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLInteractionNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLLabelMapVolumeDisplayNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLLabelMapVolumeNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLLayoutNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLLinearTransformNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLLinearTransformSequenceStorageNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLMarkupsAngleNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLMarkupsClosedCurveNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLMarkupsCurveNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLMarkupsDisplayNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLMarkupsFiducialDisplayNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLMarkupsFiducialNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLMarkupsFiducialStorageNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLMarkupsLineNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLMarkupsPlaneDisplayNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLMarkupsPlaneNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLMarkupsROIDisplayNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLMarkupsROINode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLModelDisplayNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLModelHierarchyNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLModelNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLModelStorageNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLPlotChartNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLPlotSeriesNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLPlotViewNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLProceduralColorNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLProceduralColorStorageNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLROIListNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLROINode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLScalarVolumeDisplayNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLScalarVolumeNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLScriptedModuleNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLSegmentationDisplayNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLSegmentationNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLSegmentationStorageNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLSelectionNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLSequenceNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLSequenceStorageNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLSliceCompositeNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLSliceDisplayNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLSliceNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLSnapshotClipNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLSubjectHierarchyNode>::New()); // Increments next subject hierarchy item ID
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLTableNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLTableStorageNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLTableViewNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLTextNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLTextStorageNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLTransformDisplayNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLTransformNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLTransformStorageNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLVectorVolumeDisplayNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLViewNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLVolumeArchetypeStorageNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLVolumeSequenceStorageNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLTransformSequenceStorageNode>::New());

#ifdef MRML_USE_vtkTeem
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLDiffusionTensorVolumeDisplayNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLDiffusionTensorVolumeNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLDiffusionTensorVolumeSliceDisplayNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLNRRDStorageNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLStreamingVolumeNode>::New());
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLVectorVolumeNode>::New());
#endif

  this->RegisterAbstractNodeClass("vtkMRMLMarkupsNode", "Markup");
  this->RegisterAbstractNodeClass("vtkMRMLVolumeNode", "Volume");
}

//------------------------------------------------------------------------------
void vtkMRMLNodeFactory::RegisterNodeClass(vtkMRMLNode* node)
{
  if (!node)
  {
    vtkErrorMacro("RegisterNodeClass: cannot register a null node");
    return;
  }
  this->RegisterNodeClass(node, node->GetNodeTagName());
}

//------------------------------------------------------------------------------
void vtkMRMLNodeFactory::RegisterNodeClass(vtkMRMLNode* node, const char* tagName)
{
  if (!node)
  {
    vtkErrorMacro("RegisterNodeClass: cannot register a null node");
    return;
  }
  if (!tagName)
  {
    tagName = node->GetNodeTagName();
  }
  if (!tagName)
  {
    vtkErrorMacro(<< __FUNCTION__ << ": cannot register a null tag name for node class " << (node->GetClassName() ? node->GetClassName() : "null"));
    return;
  }
  std::string xmlTag(tagName);
  // Replace the previously registered node if any.
  // By doing so we make sure there is no more than 1 node matching a given
  // XML tag. It allows plugins to MRML to override default behavior when
  // instantiating nodes via XML tags.
  for (size_t i = 0; i < this->RegisteredNodeTags.size(); ++i)
  {
    if (this->RegisteredNodeTags[i] == xmlTag)
    {
      if (this->RegisteredNodeClasses[i] == node)
      {
        // already registered
        return;
      }
      const char* previousClassName = this->RegisteredNodeClasses[i]->GetClassName();
      const char* newClassName = node->GetClassName();
      vtkWarningMacro("Tag " << tagName << " has already been registered, unregistering previous node class "  //
                             << (previousClassName ? previousClassName : "(no class name)") << " to register " //
                             << (newClassName ? newClassName : "(no class name)"));
      this->RegisteredNodeClasses[i]->Delete();
      this->RegisteredNodeClasses.erase(this->RegisteredNodeClasses.begin() + i);
      this->RegisteredNodeTags.erase(this->RegisteredNodeTags.begin() + i);
      // there is maximum one matching tag in the list, no need to search any further
      break;
    }
  }

  node->Register(this);
  this->RegisteredNodeClasses.push_back(node);
  this->RegisteredNodeTags.push_back(xmlTag);
  this->InvokeEvent(vtkMRMLNodeFactory::NodeClassRegisteredEvent);
}

//------------------------------------------------------------------------------
void vtkMRMLNodeFactory::RegisterAbstractNodeClass(const std::string& className, const std::string& typeDisplayName)
{
  auto classNameTypeDisplayNameIt = this->RegisteredAbstractNodeClassTypeDisplayNames.find(className);
  if (classNameTypeDisplayNameIt != this->RegisteredAbstractNodeClassTypeDisplayNames.end())
  {
    // class already registered
    if (classNameTypeDisplayNameIt->second == typeDisplayName)
    {
      // no change
      return;
    }
  }
  this->RegisteredAbstractNodeClassTypeDisplayNames[className] = typeDisplayName;
  this->InvokeEvent(vtkMRMLNodeFactory::NodeClassRegisteredEvent);
}

//------------------------------------------------------------------------------
void vtkMRMLNodeFactory::CopyRegisteredNodeClasses(vtkMRMLNodeFactory* sourceFactory)
{
  if (!sourceFactory || sourceFactory == this)
  {
    return;
  }
  for (const auto& classNameTypeDisplayName : sourceFactory->RegisteredAbstractNodeClassTypeDisplayNames)
  {
    if (this->RegisteredAbstractNodeClassTypeDisplayNames.find(classNameTypeDisplayName.first) == this->RegisteredAbstractNodeClassTypeDisplayNames.end())
    {
      this->RegisterAbstractNodeClass(classNameTypeDisplayName.first, classNameTypeDisplayName.second);
    }
  }
  for (size_t i = 0; i < sourceFactory->RegisteredNodeClasses.size(); i++)
  {
    // The registered node is a prototype that is only used for creating new node instances
    // (and for querying class name, tag, etc.), therefore the same prototype can be shared between factories.
    if (!this->GetClassNameByTag(sourceFactory->RegisteredNodeTags[i].c_str()))
    {
      this->RegisterNodeClass(sourceFactory->RegisteredNodeClasses[i], sourceFactory->RegisteredNodeTags[i].c_str());
    }
  }
}

//------------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLNodeFactory::CreateNodeByClass(const char* className)
{
  if (className == nullptr)
  {
    vtkErrorMacro("CreateNodeByClass: className is NULL");
    return nullptr;
  }
  for (vtkMRMLNode* registeredNode : this->RegisteredNodeClasses)
  {
    if (!strcmp(registeredNode->GetClassName(), className))
    {
      return registeredNode->CreateNodeInstance();
    }
  }
  // non-registered nodes can have a registered factory
  vtkObject* ret = vtkObjectFactory::CreateInstance(className);
  if (!ret)
  {
#ifndef VTK_HAS_INITIALIZE_OBJECT_BASE
# ifdef VTK_DEBUG_LEAKS
    vtkDebugLeaks::DestructClass(className);
# endif
#endif
    return nullptr;
  }
  vtkMRMLNode* node = vtkMRMLNode::SafeDownCast(ret);
  if (!node)
  {
    // not a MRML node
    ret->Delete();
  }
  return node;
}

//------------------------------------------------------------------------------
const char* vtkMRMLNodeFactory::GetClassNameByTag(const char* tagName)
{
  if (tagName == nullptr)
  {
    vtkErrorMacro("GetClassNameByTag: tagname is null");
    return nullptr;
  }
  for (size_t i = 0; i < this->RegisteredNodeTags.size(); i++)
  {
    if (!strcmp(this->RegisteredNodeTags[i].c_str(), tagName))
    {
      return this->RegisteredNodeClasses[i]->GetClassName();
    }
  }
  return nullptr;
}

//------------------------------------------------------------------------------
const char* vtkMRMLNodeFactory::GetTagByClassName(const char* className)
{
  if (!className)
  {
    vtkErrorMacro("GetTagByClassName: className is null");
    return nullptr;
  }
  for (vtkMRMLNode* registeredNode : this->RegisteredNodeClasses)
  {
    if (!strcmp(registeredNode->GetClassName(), className))
    {
      return registeredNode->GetNodeTagName();
    }
  }
  return nullptr;
}

//------------------------------------------------------------------------------
std::string vtkMRMLNodeFactory::GetTypeDisplayNameByClassName(const std::string& className)
{
  for (vtkMRMLNode* registeredNode : this->RegisteredNodeClasses)
  {
    if (className.compare(registeredNode->GetClassName()) == 0)
    {
      return registeredNode->GetTypeDisplayName();
    }
  }
  auto classNameTypeDisplayNameIt = this->RegisteredAbstractNodeClassTypeDisplayNames.find(className);
  if (classNameTypeDisplayNameIt != this->RegisteredAbstractNodeClassTypeDisplayNames.end())
  {
    return classNameTypeDisplayNameIt->second;
  }
  return "";
}

//------------------------------------------------------------------------------
int vtkMRMLNodeFactory::GetNumberOfRegisteredNodeClasses()
{
  return static_cast<int>(this->RegisteredNodeClasses.size());
}

//------------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLNodeFactory::GetNthRegisteredNodeClass(int n)
{
  if (n < 0 || n >= this->GetNumberOfRegisteredNodeClasses())
  {
    vtkErrorMacro("GetNthRegisteredNodeClass: index " << n << " out of bounds 0 - " << this->GetNumberOfRegisteredNodeClasses());
    return nullptr;
  }
  return this->RegisteredNodeClasses[n];
}

//------------------------------------------------------------------------------
bool vtkMRMLNodeFactory::IsNodeClassRegistered(const std::string& className)
{
  for (vtkMRMLNode* registeredNode : this->RegisteredNodeClasses)
  {
    if (className == registeredNode->GetClassName())
    {
      return true;
    }
  }
  return false;
}

//------------------------------------------------------------------------------
int vtkMRMLNodeFactory::GetNumberOfRegisteredAbstractNodeClasses()
{
  return static_cast<int>(this->RegisteredAbstractNodeClassTypeDisplayNames.size());
}

//------------------------------------------------------------------------------
std::string vtkMRMLNodeFactory::GetNthRegisteredAbstractNodeClassName(int n)
{
  if (n < 0 || n >= this->GetNumberOfRegisteredAbstractNodeClasses())
  {
    vtkErrorMacro("GetNthRegisteredAbstractNodeClassName: index " << n << " out of bounds 0 - " << this->GetNumberOfRegisteredAbstractNodeClasses());
    return "";
  }
  auto classNameTypeDisplayNameIt = this->RegisteredAbstractNodeClassTypeDisplayNames.begin();
  std::advance(classNameTypeDisplayNameIt, n);
  return classNameTypeDisplayNameIt->first;
}

//------------------------------------------------------------------------------
std::string vtkMRMLNodeFactory::GetNthRegisteredAbstractNodeTypeDisplayName(int n)
{
  if (n < 0 || n >= this->GetNumberOfRegisteredAbstractNodeClasses())
  {
    vtkErrorMacro("GetNthRegisteredAbstractNodeTypeDisplayName: index " << n << " out of bounds 0 - " << this->GetNumberOfRegisteredAbstractNodeClasses());
    return "";
  }
  auto classNameTypeDisplayNameIt = this->RegisteredAbstractNodeClassTypeDisplayNames.begin();
  std::advance(classNameTypeDisplayNameIt, n);
  return classNameTypeDisplayNameIt->second;
}
