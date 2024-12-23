/*==============================================================================

  Program: 3D Slicer

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// MRMLLogic includes
#include "vtkMRMLCornerTextLogic.h"

// MRML includes
#include "vtkMRMLScene.h"
#include "vtkMRMLSliceNode.h"
#include "vtkMRMLTextNode.h"
#include "vtkXMLDataParser.h"
#include "vtkMRMLAbstractAnnotationPropertyValueProvider.h"

// VTK includes
#include <string>
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <cassert>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLCornerTextLogic);

//----------------------------------------------------------------------------
vtkMRMLCornerTextLogic::vtkMRMLCornerTextLogic()
{
}

//----------------------------------------------------------------------------
vtkMRMLCornerTextLogic::~vtkMRMLCornerTextLogic()
{
}

//----------------------------------------------------------------------------
void vtkMRMLCornerTextLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkMRMLCornerTextLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkMRMLCornerTextLogic::RegisterNodes()
{
  assert(this->GetMRMLScene() != 0);
}

//---------------------------------------------------------------------------
void vtkMRMLCornerTextLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}

//---------------------------------------------------------------------------
void vtkMRMLCornerTextLogic
::OnMRMLSceneNodeAdded(vtkMRMLNode* vtkNotUsed(node))
{
}

//---------------------------------------------------------------------------
void vtkMRMLCornerTextLogic
::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
}

//----------------------------------------------------------------------------
vtkXMLDataElement *
vtkMRMLCornerTextLogic::ParseTextNode(vtkMRMLTextNode *textNode)
{
  if (!textNode)
  {
    return nullptr;
  }

  std::istringstream iss(textNode->GetText(), std::istringstream::in);
  vtkNew<vtkXMLDataParser> parser;
  parser->SetStream(&iss);
  parser->Parse();

  vtkXMLDataElement *root = parser->GetRootElement();
  if (root == nullptr)
  {
    vtkErrorWithObjectMacro(parser, "vtkMRMLCornerTextLogic::ParseTextNode: failed to parse layout description");
    return nullptr;
  }

  // if we don't register, then the root element will be destroyed when the
  // parser gets out of scope
  root->Register(nullptr);
  return root;
}

//---------------------------------------------------------------------------
bool vtkMRMLCornerTextLogic::RegisterPropertyValueProvider(const std::string &pluginName,
                               vtkMRMLAbstractAnnotationPropertyValueProvider *pluginProvider)
{
  if (registeredProviders.find(pluginName) == registeredProviders.end())
  {
    registeredProviders[pluginName] = pluginProvider;
    return true;
  }
  else
  {
    vtkWarningWithObjectMacro(
        pluginProvider, "Provider for " << pluginName << " already provided.");
  }
  return false;
}

//---------------------------------------------------------------------------
vtkMRMLTextNode *vtkMRMLCornerTextLogic::GetCornerAnnotations(vtkMRMLScene *mrmlScene,
                                               const int viewArrangement,
                                               const std::string& viewName)
{
  vtkMRMLTextNode *textNode;
  const std::string baseName = "CornerAnnotationsSingleton",
                    viewArrStr = "Layout" + std::to_string(viewArrangement);

  // check if a layout specific and view specific text node exists
  if ((textNode = vtkMRMLTextNode::SafeDownCast(mrmlScene->GetSingletonNode(
           (viewArrStr + viewName + baseName).c_str(), "vtkMRMLTextNode"))))
    return textNode;

  // check if a layout specific text node exists
  if ((textNode = vtkMRMLTextNode::SafeDownCast(mrmlScene->GetSingletonNode(
           (viewArrStr + baseName).c_str(), "vtkMRMLTextNode"))))
    return textNode;

  // if not, then CornerAnnotationsSingleton should always exist as it is
  // created with the scene.
  if (!(textNode = vtkMRMLTextNode::SafeDownCast(
            mrmlScene->GetSingletonNode(baseName.c_str(), "vtkMRMLTextNode"))))
    vtkErrorWithObjectMacro(mrmlScene,
                            "vtkMRMLCornerTextLogic::GetCornerAnnotations: "
                            "failed to get text node from scene");

  return textNode;
}

//---------------------------------------------------------------------------
std::array<std::string, 8>
vtkMRMLCornerTextLogic::GenerateAnnotations(vtkMRMLSliceNode *sliceNode,
                                              vtkMRMLTextNode *textNode,
                                              bool printWarnings)
{
  std::array<std::string, 8> cornerAnnotations{};
  if (!this->SliceViewAnnotationsEnabled)
  {
    return cornerAnnotations;
  }

  if (!sliceNode || !textNode)
  {
    if (printWarnings)
    {
      vtkErrorWithObjectMacro(sliceNode, "Invalid input nodes.");
    }
    return cornerAnnotations;
  }

  if (textNode->GetText().size() <= 0) return cornerAnnotations;

  // Parse <annotations> element

  vtkXMLDataElement* annotations = this->ParseTextNode(textNode);
  if (!annotations)
  {
    if (printWarnings)
    {
      vtkErrorWithObjectMacro(textNode, "Could not find <annotations> tag.");
    }
    return cornerAnnotations;
  }

  // Parse <corner>/<edge> elements within <annotations>

  if (annotations->GetNumberOfNestedElements() == 0)
  {
    if (printWarnings)
    {
      vtkErrorWithObjectMacro(textNode,
                              "<annotations> tag had no nested elements.");
    }
    return cornerAnnotations;
  }

  for (int idx = 0; idx < annotations->GetNumberOfNestedElements(); ++idx)
  {
    vtkXMLDataElement* cornerOrEdge = annotations->GetNestedElement(idx);

    const std::string tagName = std::string(cornerOrEdge->GetName());
    if (std::string(cornerOrEdge->GetName()) != "corner" &&
        std::string(cornerOrEdge->GetName()) != "edge")
    {
      if (printWarnings)
      {
        vtkErrorWithObjectMacro(
            textNode,
            "<annotations> tag must be nested with <corner> or <edge> tags.");
      }
      return cornerAnnotations;
    }
    if (!cornerOrEdge->GetAttribute("position"))
    {
      if (printWarnings)
      {
        vtkErrorWithObjectMacro(
            textNode,
            "<corner> and <edge> tags must specify a position attribute (e.g. "
            "<corner position=\"bottom-left\"> or <edge "
            "position=\"bottom\">).");
      }
      return cornerAnnotations;
    }

    std::string position = cornerOrEdge->GetAttribute("position");

    if (!this->positionMap.count(position))
    {
      if (printWarnings)
      {
        vtkErrorWithObjectMacro(
            textNode, "position attribute of <corner>/<edge> tag invalid (e.g. "
                      "<corner position=\"bottom-left\"> or <edge "
                      "position=\"bottom\">).");
      }
      return cornerAnnotations;
    }

    TextLocation location = this->positionMap.at(position);
    int numProperties = cornerOrEdge->GetNumberOfNestedElements();

    if (numProperties == 0)
    {
      if (printWarnings)
      {
        vtkWarningWithObjectMacro(textNode,
                                  "<" + std::string(cornerOrEdge->GetName()) +
                                      " position=" + position +
                                      "> had no nested elements.");
      }
        return cornerAnnotations;
    }

    // parse each line within <corner> or <edge>

    std::string text = "";

    for (int propertyIdx = 0; propertyIdx < numProperties; ++propertyIdx)
    {
      vtkXMLDataElement *property = cornerOrEdge->GetNestedElement(propertyIdx);
      if (std::string(property->GetName()) != "property")
      {
        if (printWarnings)
        {
          vtkErrorWithObjectMacro(
              textNode, "<corner>/<edge> tags must only be nested with "
                        "self-closing <property /> tags.");
        }
        break;
      }

      std::string propertyName = "", propertyValue = "";
      vtkMRMLAbstractAnnotationPropertyValueProvider::XMLTagAttributes
          attributes;

      // Get the value for "name" in the tag <property "name"=...> (required)

      if (property->GetAttribute("name"))
      {
        propertyName = property->GetAttribute("name");
      }
      else
      {
        if (printWarnings)
        {
          vtkErrorWithObjectMacro(textNode,
                                  "<" + std::string(cornerOrEdge->GetName()) +
                                      " position=" + position +
                                      "> has a property with a missing name.");
        }
        break;
      }

      // Get the attribute values out of the XML tag

      for (int attributeIdx = 0;
           attributeIdx < property->GetNumberOfAttributes(); ++attributeIdx)
      {
        attributes.insert({property->GetAttributeName(attributeIdx),
                           property->GetAttributeValue(attributeIdx)});
      }

      // we have to check if the name is registered by a plugin
      for (const auto [plugin, provider] : registeredProviders)
      {
        if (provider->CanProvideValueForPropertyName(propertyName))
        {
          propertyValue = provider->GetValueForPropertyName(
              propertyName, attributes, sliceNode);
        }
      }

      const bool valueNotProvided = propertyName != "" && propertyValue == "";

      if (valueNotProvided)
      {
        if (printWarnings)
        {
          vtkErrorWithObjectMacro(textNode,
                                  "<" + std::string(cornerOrEdge->GetName()) +
                                      " position=" + position +
                                      "> had no property value for " +
                                      propertyName + ".");
        }
      }
      else
      {
        // Only append the text if our selected display amount permits
        if (vtkMRMLAbstractAnnotationPropertyValueProvider::
                GetDisplayLevelValueAsInteger(attributes) >=
            this->DisplayStrictness)
        {
          text += propertyValue + '\n';
        }
      }
    }

    // once each property has been parsed, the annotation for that position
    // is fully specified

    cornerAnnotations[location] += text;
  }

  return cornerAnnotations;
}
