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

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// Segmentations includes
#include "vtkMRMLSegmentationDisplayNode.h"
#include "vtkMRMLSegmentationNode.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLColorTableNode.h>

// SegmentationCore includes
#include "vtkSegmentation.h"
#include "vtkOrientedImageData.h"
#include "vtkTopologicalHierarchy.h"
#include "vtkSegmentationConverterFactory.h"

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkVersion.h>
#include <vtkLookupTable.h>
#include <vtkVector.h>

// STD includes
#include <algorithm>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLSegmentationDisplayNode);

//----------------------------------------------------------------------------
vtkMRMLSegmentationDisplayNode::vtkMRMLSegmentationDisplayNode()
  : PreferredDisplayRepresentationName2D(NULL)
  , PreferredDisplayRepresentationName3D(NULL)
  , NumberOfAddedSegments(0)
{
  this->SliceIntersectionVisibility = true;

  this->SegmentationDisplayProperties.clear();
}

//----------------------------------------------------------------------------
vtkMRMLSegmentationDisplayNode::~vtkMRMLSegmentationDisplayNode()
{
  this->SetPreferredDisplayRepresentationName2D(NULL);
  this->SetPreferredDisplayRepresentationName3D(NULL);
  this->SegmentationDisplayProperties.clear();
}

//----------------------------------------------------------------------------
void vtkMRMLSegmentationDisplayNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  vtkIndent indent(nIndent);

  of << indent << " PreferredDisplayRepresentationName2D=\""
    << (this->PreferredDisplayRepresentationName2D ? this->PreferredDisplayRepresentationName2D : "NULL") << "\"";
  of << indent << " PreferredDisplayRepresentationName3D=\""
    << (this->PreferredDisplayRepresentationName3D ? this->PreferredDisplayRepresentationName3D : "NULL") << "\"";

  of << indent << " SegmentationDisplayProperties=\"";
  for (SegmentDisplayPropertiesMap::iterator propIt = this->SegmentationDisplayProperties.begin();
    propIt != this->SegmentationDisplayProperties.end(); ++propIt)
    {
    of << vtkMRMLNode::URLEncodeString(propIt->first.c_str())
      << " ColorR:" << propIt->second.Color[0]
      << " ColorG:" << propIt->second.Color[1]
      << " ColorB:" << propIt->second.Color[2]
      << " Visible:" << (propIt->second.Visible ? "true" : "false")
      << " Visible3D:" << (propIt->second.Visible3D ? "true" : "false")
      << " Visible2DFill:" << (propIt->second.Visible2DFill ? "true" : "false")
      << " Visible2DOutline:" << (propIt->second.Visible2DOutline ? "true" : "false")
      << " Opacity3D:" << propIt->second.Opacity3D
      << " Opacity2DFill:" << propIt->second.Opacity2DFill
      << " Opacity2DOutline:" << propIt->second.Opacity2DOutline << "|";
    }
  of << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLSegmentationDisplayNode::ReadXMLAttributes(const char** atts)
{
  // Read all MRML node attributes from two arrays of names and values
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  // Read all MRML node attributes from two arrays of names and values
  const char* attName = NULL;
  const char* attValue = NULL;

  while (*atts != NULL)
    {
    attName = *(atts++);
    attValue = *(atts++);

    if (!strcmp(attName, "PreferredDisplayRepresentationName2D"))
      {
      this->SetPreferredDisplayRepresentationName2D(attValue);
      }
    else if (!strcmp(attName, "PreferredDisplayRepresentationName3D"))
      {
      this->SetPreferredDisplayRepresentationName3D(attValue);
      }
    else if (!strcmp(attName, "SegmentationDisplayProperties"))
      {
      // attValue: "Segment_0 ColorR:0.2 ColorG:0.501961 ... Opacity2DOutline:1|Segment_1 ColorR:1 ColorG:0.8...|Segment_2 ColorR:1 ColorG:1...|"
      std::stringstream segmentsDisplayProperties(attValue); // properties of all segments
      std::string segmentDisplayPropertiesString; // properties of a single segment
      while (std::getline(segmentsDisplayProperties, segmentDisplayPropertiesString, '|'))
        {
        // segmentDisplayPropertiesString: "Segment_0 ColorR:0.2 ColorG:0.501961 ... Opacity2DOutline:1"
        std::stringstream segmentDisplayProperties(segmentDisplayPropertiesString);
        std::string id;
        segmentDisplayProperties >> id;
        SegmentDisplayProperties props;
        std::string segmentDisplayPropertyString; // properties of a single segment
        while (std::getline(segmentDisplayProperties, segmentDisplayPropertyString, ' '))
          {
          // segmentDisplayPropertyString: "ColorR:0.2"
          if (segmentDisplayPropertyString.empty())
            {
            // multiple spaces between properties, just get the next item
            continue;
            }
          size_t colonIndex = segmentDisplayPropertyString.find(':');
          if (colonIndex == std::string::npos)
            {
            // no colon found, not a valid property, ignore it
            vtkErrorMacro("Invalid property found in attribue of "<<(this->ID?this->ID:"(unknown)")<<" node: "<<segmentDisplayPropertyString);
            continue;
            }
          std::string propertyName = segmentDisplayPropertyString.substr(0, colonIndex); // "ColorR"
          std::stringstream propertyValue(segmentDisplayPropertyString.substr(colonIndex + 1)); // "0.2"
          if (propertyName=="ColorR") { propertyValue >> props.Color[0]; }
          else if (propertyName=="ColorG") { propertyValue >> props.Color[1]; }
          else if (propertyName=="ColorB") { propertyValue >> props.Color[2]; }
          else if (propertyName=="Opacity3D") { propertyValue >> props.Opacity3D; }
          else if (propertyName=="Opacity2DFill") { propertyValue >> props.Opacity2DFill; }
          else if (propertyName=="Opacity2DOutline") { propertyValue >> props.Opacity2DOutline; }
          else
            {
            // boolean values
            std::string booleanValueString;
            propertyValue >> booleanValueString;
            bool booleanValue = booleanValueString.compare("true") ? false : true;
            if (propertyName=="Visible") { props.Visible = booleanValue; }
            else if (propertyName=="Visible3D") { props.Visible3D = booleanValue; }
            else if (propertyName=="Visible2DFill") { props.Visible2DFill = booleanValue; }
            else if (propertyName=="Visible2DOutline") { props.Visible2DOutline = booleanValue; }
            }
          }
        this->SetSegmentDisplayProperties(vtkMRMLNode::URLDecodeString(id.c_str()), props);
        }
      }
    }

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLSegmentationDisplayNode::Copy(vtkMRMLNode *anode)
{
  bool wasModifying = this->StartModify();
  this->Superclass::Copy(anode);

  vtkMRMLSegmentationDisplayNode *node = vtkMRMLSegmentationDisplayNode::SafeDownCast(anode);
  if (node)
    {
    this->SetPreferredDisplayRepresentationName2D(node->GetPreferredDisplayRepresentationName2D());
    this->SetPreferredDisplayRepresentationName3D(node->GetPreferredDisplayRepresentationName3D());
    this->SegmentationDisplayProperties = node->SegmentationDisplayProperties;
    }

  this->EndModify(wasModifying);
}

//----------------------------------------------------------------------------
void vtkMRMLSegmentationDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << indent << " PreferredDisplayRepresentationName2D:   " << (this->PreferredDisplayRepresentationName2D ? this->PreferredDisplayRepresentationName2D : "NULL") << "\n";
  os << indent << " PreferredDisplayRepresentationName3D:   " << (this->PreferredDisplayRepresentationName3D ? this->PreferredDisplayRepresentationName3D : "NULL") << "\n";

  os << indent << " SegmentationDisplayProperties:\n";
  for (SegmentDisplayPropertiesMap::iterator propIt = this->SegmentationDisplayProperties.begin();
    propIt != this->SegmentationDisplayProperties.end(); ++propIt)
    {
    os << indent << "   SegmentID=" << propIt->first << ", Color=("
       << propIt->second.Color[0] << "," << propIt->second.Color[1] << "," << propIt->second.Color[2]
       << "), Visible=" << (propIt->second.Visible ? "true" : "false")
       << ", Visible3D=" << (propIt->second.Visible3D ? "true" : "false") << ", Visible2DFill=" << (propIt->second.Visible2DFill ? "true" : "false") << ", Visible2DOutline=" << (propIt->second.Visible2DOutline ? "true" : "false")
       << ", Opacity3D=" << propIt->second.Opacity3D << ", Opacity2DFill=" << propIt->second.Opacity2DFill << ", Opacity2DOutline=" << propIt->second.Opacity2DOutline << "\n";
    }
}

//---------------------------------------------------------------------------
vtkMRMLColorTableNode* vtkMRMLSegmentationDisplayNode::CreateColorTableNode(const char* segmentationNodeName)
{
  if (!this->Scene)
    {
    vtkErrorMacro("CreateColorTableNode: Invalid MRML scene!");
    return NULL;
    }

  vtkSmartPointer<vtkMRMLColorTableNode> segmentationColorTableNode = vtkSmartPointer<vtkMRMLColorTableNode>::New();
  std::string segmentationColorTableNodeName = std::string(segmentationNodeName ? segmentationNodeName : "Segmentation") + GetColorTableNodeNamePostfix();
  segmentationColorTableNodeName = this->Scene->GenerateUniqueName(segmentationColorTableNodeName);
  segmentationColorTableNode->SetName(segmentationColorTableNodeName.c_str());
  segmentationColorTableNode->HideFromEditorsOff();
  segmentationColorTableNode->SetTypeToUser();
  segmentationColorTableNode->NamesInitialisedOn();
  segmentationColorTableNode->SetAttribute("Category", this->GetNodeTagName());
  this->Scene->AddNode(segmentationColorTableNode);

  // Initialize color table
  segmentationColorTableNode->SetNumberOfColors(1);
  segmentationColorTableNode->GetLookupTable()->SetTableRange(0,0);
  segmentationColorTableNode->AddColor(this->GetSegmentationColorNameBackground(), 0.0, 0.0, 0.0, 0.0); // Black background

  // Set reference to color table node
  this->SetAndObserveColorNodeID(segmentationColorTableNode->GetID());

  return segmentationColorTableNode;
}

//---------------------------------------------------------------------------
bool vtkMRMLSegmentationDisplayNode::SetSegmentColorTableEntry(std::string segmentId, double r, double g, double b)
{
  // Set color in color table (for merged labelmap)
  vtkMRMLColorTableNode* colorTableNode = vtkMRMLColorTableNode::SafeDownCast(this->GetColorNode());
  if (!colorTableNode)
    {
    if (this->Scene && !this->Scene->IsImporting())
      {
      vtkErrorMacro("SetSegmentColorTableEntry: No color table node associated with segmentation. Maybe CreateColorTableNode was not called?");
      }
    return false;
    }

  // Look up segment color in color table node (-1 if not found)
  int colorIndex = colorTableNode->GetColorIndexByName(segmentId.c_str());
  if (colorIndex < 0)
    {
    if (!this->Scene->IsImporting())
      {
      // It is possible that when the segmentation is loaded, the color table node is already loaded but its storage node is not
      vtkWarningMacro("SetSegmentColorTableEntry: No color table entry found for segment " << segmentId);
      }
    return false;
    }
  // Do not support opacity in color table. If advanced display is needed then use the displayable manager
  colorTableNode->SetColor(colorIndex, r, g, b, 1.0);
  return true;
}

//---------------------------------------------------------------------------
bool vtkMRMLSegmentationDisplayNode::GetSegmentDisplayProperties(std::string segmentId, SegmentDisplayProperties &properties)
{
  SegmentDisplayPropertiesMap::iterator propsIt = this->SegmentationDisplayProperties.find(segmentId);
  if (propsIt == this->SegmentationDisplayProperties.end())
    {
    vtkWarningMacro("vtkMRMLSegmentationDisplayNode::GetSegmentDisplayProperties: no display properties are found for segment ID="<<segmentId<<", return default");
    SegmentDisplayProperties defaultProperties;
    properties = defaultProperties;
    return false;
    }
  properties = propsIt->second;
  return true;
}

//---------------------------------------------------------------------------
bool vtkMRMLSegmentationDisplayNode::GetSegmentDisplayPropertiesDefined(std::string segmentId)
{
  SegmentDisplayPropertiesMap::iterator propsIt = this->SegmentationDisplayProperties.find(segmentId);
  return (propsIt != this->SegmentationDisplayProperties.end());
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationDisplayNode::SetSegmentDisplayProperties(std::string segmentId, SegmentDisplayProperties &properties)
{
  SegmentDisplayPropertiesMap::iterator propsIt = this->SegmentationDisplayProperties.find(segmentId);
  bool modified = false;
  if (propsIt == this->SegmentationDisplayProperties.end())
    {
    // If not found then add
    SegmentDisplayProperties newPropertiesEntry;
    newPropertiesEntry.Color[0] = properties.Color[0];
    newPropertiesEntry.Color[1] = properties.Color[1];
    newPropertiesEntry.Color[2] = properties.Color[2];
    newPropertiesEntry.Visible = properties.Visible;
    newPropertiesEntry.Visible3D = properties.Visible3D;
    newPropertiesEntry.Visible2DFill = properties.Visible2DFill;
    newPropertiesEntry.Visible2DOutline = properties.Visible2DOutline;
    newPropertiesEntry.Opacity3D = properties.Opacity3D;
    newPropertiesEntry.Opacity2DFill = properties.Opacity2DFill;
    newPropertiesEntry.Opacity2DOutline = properties.Opacity2DOutline;
    this->SegmentationDisplayProperties[segmentId] = newPropertiesEntry;
    modified = true;
    }
  else
    {
    // If found then replace values
    for (int i=0; i<3; i++)
      {
      if (propsIt->second.Color[i] != properties.Color[i])
        {
        propsIt->second.Color[i] = properties.Color[i];
        modified = true;
        }
      }
    if (propsIt->second.Visible != properties.Visible)
      {
      propsIt->second.Visible = properties.Visible;
      modified = true;
      }
    if (propsIt->second.Visible3D != properties.Visible3D)
      {
      propsIt->second.Visible3D = properties.Visible3D;
      modified = true;
      }
    if (propsIt->second.Visible2DFill != properties.Visible2DFill)
      {
      propsIt->second.Visible2DFill = properties.Visible2DFill;
      modified = true;
      }
    if (propsIt->second.Visible2DOutline != properties.Visible2DOutline)
      {
      propsIt->second.Visible2DOutline = properties.Visible2DOutline;
      modified = true;
      }
    if (propsIt->second.Opacity3D != properties.Opacity3D)
      {
      propsIt->second.Opacity3D = properties.Opacity3D;
      modified = true;
      }
    if (propsIt->second.Opacity2DFill != properties.Opacity2DFill)
      {
      propsIt->second.Opacity2DFill = properties.Opacity2DFill;
      modified = true;
      }
    if (propsIt->second.Opacity2DOutline != properties.Opacity2DOutline)
      {
      propsIt->second.Opacity2DOutline = properties.Opacity2DOutline;
      modified = true;
      }
    }

  // Set color in color table too
  this->SetSegmentColorTableEntry(segmentId, properties.Color[0], properties.Color[1], properties.Color[2]);

  if (modified)
    {
    this->Modified();
    }
}

//---------------------------------------------------------------------------
vtkVector3d vtkMRMLSegmentationDisplayNode::GetSegmentColor(std::string segmentID)
{
  SegmentDisplayPropertiesMap::iterator propsIt = this->SegmentationDisplayProperties.find(segmentID);
  if (propsIt == this->SegmentationDisplayProperties.end())
    {
    vtkErrorMacro("GetSegmentColor: No display properties found for segment with ID " << segmentID);
    vtkVector3d color(vtkSegment::SEGMENT_COLOR_VALUE_INVALID[0], vtkSegment::SEGMENT_COLOR_VALUE_INVALID[1], vtkSegment::SEGMENT_COLOR_VALUE_INVALID[2]);
    return color;
    }

  vtkVector3d color(propsIt->second.Color[0], propsIt->second.Color[1], propsIt->second.Color[2]);
  return color;
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationDisplayNode::SetSegmentColor(std::string segmentID, double r, double g, double b)
{
  // Set color in display properties
  SegmentDisplayProperties properties;
  this->GetSegmentDisplayProperties(segmentID, properties);
  properties.Color[0] = r;
  properties.Color[1] = g;
  properties.Color[2] = b;
  this->SetSegmentDisplayProperties(segmentID, properties);
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationDisplayNode::SetSegmentColor(std::string segmentID, vtkVector3d color)
{
  SegmentDisplayProperties properties;
  this->GetSegmentDisplayProperties(segmentID, properties);
  properties.Color[0] = color.GetX();
  properties.Color[1] = color.GetY();
  properties.Color[2] = color.GetZ();
  this->SetSegmentDisplayProperties(segmentID, properties);
}

//---------------------------------------------------------------------------
bool vtkMRMLSegmentationDisplayNode::GetSegmentVisibility(std::string segmentID)
{
  SegmentDisplayPropertiesMap::iterator propsIt = this->SegmentationDisplayProperties.find(segmentID);
  if (propsIt == this->SegmentationDisplayProperties.end())
    {
    vtkErrorMacro("GetSegmentVisibility: No display properties found for segment with ID " << segmentID);
    return false;
    }
  return propsIt->second.Visible;
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationDisplayNode::SetSegmentVisibility(std::string segmentID, bool visible)
{
  SegmentDisplayProperties properties;
  this->GetSegmentDisplayProperties(segmentID, properties);
  properties.Visible = visible;
  this->SetSegmentDisplayProperties(segmentID, properties);
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationDisplayNode::SetAllSegmentsVisibility(bool visible)
{
  std::vector<std::string> segmentIDs;
  this->GetAllSegmentIDs(segmentIDs, false);
  for (std::vector<std::string>::iterator segmentIDIt = segmentIDs.begin(); segmentIDIt != segmentIDs.end(); ++segmentIDIt)
    {
    this->SetSegmentVisibility(*segmentIDIt, visible);
    }
}

//---------------------------------------------------------------------------
bool vtkMRMLSegmentationDisplayNode::GetSegmentVisibility3D(std::string segmentID)
{
  SegmentDisplayPropertiesMap::iterator propsIt = this->SegmentationDisplayProperties.find(segmentID);
  if (propsIt == this->SegmentationDisplayProperties.end())
    {
    vtkErrorMacro("GetSegmentVisibility3D: No display properties found for segment with ID " << segmentID);
    return false;
    }
  return propsIt->second.Visible3D;
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationDisplayNode::SetSegmentVisibility3D(std::string segmentID, bool visible)
{
  SegmentDisplayProperties properties;
  this->GetSegmentDisplayProperties(segmentID, properties);
  properties.Visible3D = visible;
  this->SetSegmentDisplayProperties(segmentID, properties);
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationDisplayNode::SetAllSegmentsVisibility3D(bool visible, bool changeVisibleSegmentsOnly /* = false */)
{
  std::vector<std::string> segmentIDs;
  this->GetAllSegmentIDs(segmentIDs, changeVisibleSegmentsOnly);
  for (std::vector<std::string>::iterator segmentIDIt = segmentIDs.begin(); segmentIDIt != segmentIDs.end(); ++segmentIDIt)
    {
    this->SetSegmentVisibility3D(*segmentIDIt, visible);
    }
}

//---------------------------------------------------------------------------
bool vtkMRMLSegmentationDisplayNode::GetSegmentVisibility2DFill(std::string segmentID)
{
  SegmentDisplayPropertiesMap::iterator propsIt = this->SegmentationDisplayProperties.find(segmentID);
  if (propsIt == this->SegmentationDisplayProperties.end())
    {
    vtkErrorMacro("GetSegmentVisibility2DFill: No display properties found for segment with ID " << segmentID);
    return false;
    }
  return propsIt->second.Visible2DFill;
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationDisplayNode::SetSegmentVisibility2DFill(std::string segmentID, bool visible)
{
  SegmentDisplayProperties properties;
  this->GetSegmentDisplayProperties(segmentID, properties);
  properties.Visible2DFill = visible;
  this->SetSegmentDisplayProperties(segmentID, properties);
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationDisplayNode::SetAllSegmentsVisibility2DFill(bool visible, bool changeVisibleSegmentsOnly /* = false */)
{
  std::vector<std::string> segmentIDs;
  this->GetAllSegmentIDs(segmentIDs, changeVisibleSegmentsOnly);
  for (std::vector<std::string>::iterator segmentIDIt = segmentIDs.begin(); segmentIDIt != segmentIDs.end(); ++segmentIDIt)
    {
    this->SetSegmentVisibility2DFill(*segmentIDIt, visible);
    }
}

//---------------------------------------------------------------------------
bool vtkMRMLSegmentationDisplayNode::GetSegmentVisibility2DOutline(std::string segmentID)
{
  SegmentDisplayPropertiesMap::iterator propsIt = this->SegmentationDisplayProperties.find(segmentID);
  if (propsIt == this->SegmentationDisplayProperties.end())
    {
    vtkErrorMacro("GetSegmentVisibility2DOutline: No display properties found for segment with ID " << segmentID);
    return false;
    }
  return propsIt->second.Visible2DOutline;
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationDisplayNode::SetSegmentVisibility2DOutline(std::string segmentID, bool visible)
{
  SegmentDisplayProperties properties;
  this->GetSegmentDisplayProperties(segmentID, properties);
  properties.Visible2DOutline = visible;
  this->SetSegmentDisplayProperties(segmentID, properties);
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationDisplayNode::SetAllSegmentsVisibility2DOutline(bool visible, bool changeVisibleSegmentsOnly /* = false */)
{
  std::vector<std::string> segmentIDs;
  this->GetAllSegmentIDs(segmentIDs, changeVisibleSegmentsOnly);
  for (std::vector<std::string>::iterator segmentIDIt = segmentIDs.begin(); segmentIDIt != segmentIDs.end(); ++segmentIDIt)
    {
    this->SetSegmentVisibility2DOutline(*segmentIDIt, visible);
    }
}

//---------------------------------------------------------------------------
double vtkMRMLSegmentationDisplayNode::GetSegmentOpacity3D(std::string segmentID)
{
  SegmentDisplayPropertiesMap::iterator propsIt = this->SegmentationDisplayProperties.find(segmentID);
  if (propsIt == this->SegmentationDisplayProperties.end())
    {
    vtkErrorMacro("GetSegmentOpacity3D: No display properties found for segment with ID " << segmentID);
    return 0.0;
    }
  return propsIt->second.Opacity3D;
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationDisplayNode::SetSegmentOpacity3D(std::string segmentID, double opacity)
{
  SegmentDisplayProperties properties;
  this->GetSegmentDisplayProperties(segmentID, properties);
  properties.Opacity3D = opacity;
  this->SetSegmentDisplayProperties(segmentID, properties);
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationDisplayNode::SetAllSegmentsOpacity3D(double opacity, bool changeVisibleSegmentsOnly /* = false */)
{
  std::vector<std::string> segmentIDs;
  this->GetAllSegmentIDs(segmentIDs, changeVisibleSegmentsOnly);
  for (std::vector<std::string>::iterator segmentIDIt = segmentIDs.begin(); segmentIDIt != segmentIDs.end(); ++segmentIDIt)
    {
    this->SetSegmentOpacity3D(*segmentIDIt, opacity);
    }
}

//---------------------------------------------------------------------------
double vtkMRMLSegmentationDisplayNode::GetSegmentOpacity2DFill(std::string segmentID)
{
  SegmentDisplayPropertiesMap::iterator propsIt = this->SegmentationDisplayProperties.find(segmentID);
  if (propsIt == this->SegmentationDisplayProperties.end())
    {
    vtkWarningMacro("GetSegmentOpacity2DFill: No display properties found for segment with ID " << segmentID);
    SegmentDisplayProperties defaultProperties;
    return defaultProperties.Opacity2DFill;
    }
  return propsIt->second.Opacity2DFill;
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationDisplayNode::SetSegmentOpacity2DFill(std::string segmentID, double opacity)
{
  SegmentDisplayProperties properties;
  this->GetSegmentDisplayProperties(segmentID, properties);
  properties.Opacity2DFill = opacity;
  this->SetSegmentDisplayProperties(segmentID, properties);
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationDisplayNode::SetAllSegmentsOpacity2DFill(double opacity, bool changeVisibleSegmentsOnly /* = false */)
{
  std::vector<std::string> segmentIDs;
  this->GetAllSegmentIDs(segmentIDs, changeVisibleSegmentsOnly);
  for (std::vector<std::string>::iterator segmentIDIt = segmentIDs.begin(); segmentIDIt != segmentIDs.end(); ++segmentIDIt)
    {
    this->SetSegmentOpacity2DFill(*segmentIDIt, opacity);
    }
}

//---------------------------------------------------------------------------
double vtkMRMLSegmentationDisplayNode::GetSegmentOpacity2DOutline(std::string segmentID)
{
  SegmentDisplayPropertiesMap::iterator propsIt = this->SegmentationDisplayProperties.find(segmentID);
  if (propsIt == this->SegmentationDisplayProperties.end())
    {
    vtkErrorMacro("GetSegmentOpacity2DOutline: No display properties found for segment with ID " << segmentID);
    SegmentDisplayProperties defaultProperties;
    return defaultProperties.Opacity2DOutline;
    }
  return propsIt->second.Opacity2DOutline;
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationDisplayNode::SetSegmentOpacity2DOutline(std::string segmentID, double opacity)
{
  SegmentDisplayProperties properties;
  this->GetSegmentDisplayProperties(segmentID, properties);
  properties.Opacity2DOutline = opacity;
  this->SetSegmentDisplayProperties(segmentID, properties);
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationDisplayNode::SetAllSegmentsOpacity2DOutline(double opacity, bool changeVisibleSegmentsOnly /* = false */)
{
  std::vector<std::string> segmentIDs;
  this->GetAllSegmentIDs(segmentIDs, changeVisibleSegmentsOnly);
  for (std::vector<std::string>::iterator segmentIDIt = segmentIDs.begin(); segmentIDIt != segmentIDs.end(); ++segmentIDIt)
    {
    this->SetSegmentOpacity2DOutline(*segmentIDIt, opacity);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationDisplayNode::SetSegmentOpacity(std::string segmentID, double opacity)
{
  SegmentDisplayProperties properties;
  this->GetSegmentDisplayProperties(segmentID, properties);
  properties.Opacity3D = opacity;
  properties.Opacity2DFill = opacity;
  properties.Opacity2DOutline = opacity;
  this->SetSegmentDisplayProperties(segmentID, properties);
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationDisplayNode::SetAllSegmentsOpacity(double opacity, bool changeVisibleSegmentsOnly /* = false */)
{
  std::vector<std::string> segmentIDs;
  this->GetAllSegmentIDs(segmentIDs, changeVisibleSegmentsOnly);
  for (std::vector<std::string>::iterator segmentIDIt = segmentIDs.begin(); segmentIDIt != segmentIDs.end(); ++segmentIDIt)
    {
    this->SetSegmentOpacity(*segmentIDIt, opacity);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationDisplayNode::RemoveSegmentDisplayProperties(std::string segmentId)
{
  SegmentDisplayPropertiesMap::iterator propsIt = this->SegmentationDisplayProperties.find(segmentId);
  if (propsIt != this->SegmentationDisplayProperties.end())
    {
    this->SegmentationDisplayProperties.erase(propsIt);
    }

  this->Modified();
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationDisplayNode::ClearSegmentDisplayProperties()
{
  this->SegmentationDisplayProperties.clear();
  this->NumberOfAddedSegments = 0;
  this->Modified();
}

//---------------------------------------------------------------------------
bool vtkMRMLSegmentationDisplayNode::CalculateAutoOpacitiesForSegments()
{
  // Get segmentation node
  vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(this->GetDisplayableNode());
  if (!segmentationNode)
    {
    vtkErrorMacro("CalculateAutoOpacitiesForSegments: No segmentation node associated to this display node!");
    return false;
    }

  // Make sure the requested representation exists
  vtkSegmentation* segmentation = segmentationNode->GetSegmentation();
  if (!segmentation->CreateRepresentation(this->PreferredDisplayRepresentationName3D))
    {
    return false;
    }

  // Get displayed 3D representation (always poly data)
  std::string displayedPolyDataRepresentationName = this->GetDisplayRepresentationName3D();

  // Assemble segment poly datas into a collection that can be fed to topological hierarchy algorithm
  vtkSmartPointer<vtkPolyDataCollection> segmentPolyDataCollection = vtkSmartPointer<vtkPolyDataCollection>::New();
  for (SegmentDisplayPropertiesMap::iterator propIt = this->SegmentationDisplayProperties.begin();
    propIt != this->SegmentationDisplayProperties.end(); ++propIt)
    {
    // Get segment
    vtkSegment* currentSegment = segmentation->GetSegment(propIt->first);
    if (!currentSegment)
      {
      vtkErrorMacro("CalculateAutoOpacitiesForSegments: Mismatch in display properties and segments!");
      continue;
      }

    // Get poly data from segment
    vtkPolyData* currentPolyData = vtkPolyData::SafeDownCast(
      currentSegment->GetRepresentation(displayedPolyDataRepresentationName.c_str()) );
    if (!currentPolyData)
      {
      continue;
      }

    segmentPolyDataCollection->AddItem(currentPolyData);
    }

  // Set opacities according to topological hierarchy levels
  vtkSmartPointer<vtkTopologicalHierarchy> topologicalHierarchy = vtkSmartPointer<vtkTopologicalHierarchy>::New();
  topologicalHierarchy->SetInputPolyDataCollection(segmentPolyDataCollection);
  topologicalHierarchy->Update();
  vtkIntArray* levels = topologicalHierarchy->GetOutputLevels();

  // Determine number of levels
  int numberOfLevels = 0;
  for (int i=0; i<levels->GetNumberOfTuples(); ++i)
    {
    if (levels->GetValue(i) > numberOfLevels)
      {
      numberOfLevels = levels->GetValue(i);
      }
    }
  // Sanity check
  if (static_cast<vtkIdType>(this->SegmentationDisplayProperties.size()) != levels->GetNumberOfTuples())
    {
    vtkErrorMacro("CalculateAutoOpacitiesForSegments: Number of poly data colors does not match number of segment display properties!");
    return false;
    }

  // Set opacities into lookup table
  int idx = 0;
  SegmentDisplayPropertiesMap::iterator propIt;
  for (idx=0, propIt=this->SegmentationDisplayProperties.begin(); idx < levels->GetNumberOfTuples(); ++idx, ++propIt)
    {
    int level = levels->GetValue(idx);

    // The opacity level is set evenly distributed between 0 and 1 (excluding 0)
    // according to the topological hierarchy level of the segment
    double opacity = 1.0 - ((double)level) / (numberOfLevels+1);
    propIt->second.Opacity3D = opacity;
    }

  this->Modified();
  return true;
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationDisplayNode::GenerateSegmentColor(double color[3])
{
  if (!this->Scene)
    {
    vtkErrorMacro("GenerateSegmentColor: Invalid MRML scene!");
    return;
    }

  // Get default labels color table
  vtkMRMLColorTableNode* labelsColorNode = vtkMRMLColorTableNode::SafeDownCast(
    this->Scene->GetNodeByID("vtkMRMLColorTableNodeLabels") );
  if (!labelsColorNode)
    {
    // Generate random color if default color table is not available (such as in logic tests)
    color[0] = rand() * 1.0 / RAND_MAX;
    color[1] = rand() * 1.0 / RAND_MAX;
    color[2] = rand() * 1.0 / RAND_MAX;
    return;
    }

  // Get color corresponding to the number of added segments (which is incremented in
  // vtkMRMLSegmentationNode::AddSegmentDisplayProperties every time a new segment display
  // properties entry is added
  double currentColor[4] = {0.0, 0.0, 0.0, 0.0};
  labelsColorNode->GetColor(this->NumberOfAddedSegments, currentColor);
  color[0] = currentColor[0];
  color[1] = currentColor[1];
  color[2] = currentColor[2];
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationDisplayNode::GenerateSegmentColor(double &r, double &g, double &b)
{
  double color[3] = {0.0, 0.0, 0.0};
  this->GenerateSegmentColor(color);
  r = color[0];
  g = color[1];
  b = color[2];
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationDisplayNode::IncrementNumberOfAddedSegments()
{
  this->NumberOfAddedSegments += 1;
}

//---------------------------------------------------------------------------
std::string vtkMRMLSegmentationDisplayNode::GetDisplayRepresentationName3D()
{
  // Get segmentation node
  vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(this->GetDisplayableNode());
  if (!segmentationNode)
    {
    return "";
    }
  // If segmentation is empty then we cannot show poly data
  vtkSegmentation* segmentation = segmentationNode->GetSegmentation();
  if (!segmentation || segmentation->GetNumberOfSegments() == 0)
    {
    return "";
    }

  // Assume the first segment contains the same name of representations as all segments (this should be the case by design)
  vtkSegment* firstSegment = segmentation->GetSegments().begin()->second;

  // If preferred representation is defined and exists then use that (double check it is poly data)
  if (this->PreferredDisplayRepresentationName3D)
    {
    vtkDataObject* preferredRepresentation = firstSegment->GetRepresentation(this->PreferredDisplayRepresentationName3D);
    if (vtkPolyData::SafeDownCast(preferredRepresentation))
      {
      return std::string(this->PreferredDisplayRepresentationName3D);
      }
    }

  // Otherwise if master representation is poly data then use that
  char* masterRepresentationName = segmentation->GetMasterRepresentationName();
  if (masterRepresentationName && segmentation->IsMasterRepresentationPolyData())
    {
    return std::string(masterRepresentationName);
    }

  // Otherwise return first poly data representation if any
  std::vector<std::string> containedRepresentationNames;
  segmentation->GetContainedRepresentationNames(containedRepresentationNames);
  for (std::vector<std::string>::iterator reprIt = containedRepresentationNames.begin();
    reprIt != containedRepresentationNames.end(); ++reprIt)
    {
    vtkDataObject* currentRepresentation = firstSegment->GetRepresentation(*reprIt);
    if (vtkPolyData::SafeDownCast(currentRepresentation))
      {
      return (*reprIt);
      }
    }

  // If no poly data representations are available, then return empty string
  // meaning there is no poly data representation to display
  return "";
}

//---------------------------------------------------------------------------
std::string vtkMRMLSegmentationDisplayNode::GetDisplayRepresentationName2D()
{
  // Get segmentation node
  vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(this->GetDisplayableNode());
  if (!segmentationNode)
    {
    return "";
    }
  // If segmentation is empty then return empty string
  vtkSegmentation* segmentation = segmentationNode->GetSegmentation();
  if (!segmentation || segmentation->GetNumberOfSegments() == 0)
    {
    return "";
    }

  // If preferred 2D representation exists, then return that
  if (this->PreferredDisplayRepresentationName2D)
    {
    if (segmentation->ContainsRepresentation(this->PreferredDisplayRepresentationName2D))
      {
      return std::string(this->PreferredDisplayRepresentationName2D);
      }
    }

  // Otherwise return master representation
  return std::string(segmentation->GetMasterRepresentationName());
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationDisplayNode::GetPolyDataRepresentationNames(std::set<std::string> &representationNames)
{
  representationNames.clear();

  // Note: This function used to collect existing poly data representations from the segmentation
  // It was then decided that a preferred poly data representation can be selected regardless
  // its existence, thus the list is populated based on supported poly data representations.

  // Traverse converter rules to find supported poly data representations
  vtkSegmentationConverterFactory::RuleListType rules = vtkSegmentationConverterFactory::GetInstance()->GetConverterRules();
  for (vtkSegmentationConverterFactory::RuleListType::iterator ruleIt = rules.begin(); ruleIt != rules.end(); ++ruleIt)
    {
    vtkSegmentationConverterRule* currentRule = (*ruleIt);

    vtkSmartPointer<vtkDataObject> sourceObject = vtkSmartPointer<vtkDataObject>::Take(
      currentRule->ConstructRepresentationObjectByRepresentation(currentRule->GetSourceRepresentationName()) );
    vtkPolyData* sourcePolyData = vtkPolyData::SafeDownCast(sourceObject);
    if (sourcePolyData)
      {
      representationNames.insert(std::string(currentRule->GetSourceRepresentationName()));
      }

    vtkSmartPointer<vtkDataObject> targetObject = vtkSmartPointer<vtkDataObject>::Take(
      currentRule->ConstructRepresentationObjectByRepresentation(currentRule->GetTargetRepresentationName()) );
    vtkPolyData* targetPolyData = vtkPolyData::SafeDownCast(targetObject);
    if (targetPolyData)
      {
      representationNames.insert(std::string(currentRule->GetTargetRepresentationName()));
      }
    }
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationDisplayNode::GetAllSegmentIDs(std::vector<std::string>& segmentIDs, bool visibleSegmentsOnly)
{
  segmentIDs.clear();
  vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(this->GetDisplayableNode());
  if (!segmentationNode)
    {
    vtkErrorMacro("vtkMRMLSegmentationDisplayNode::GetAllSegmentIDs: No segmentation node is associated to this display node");
    return;
    }
  // Make sure the requested representation exists
  vtkSegmentation* segmentation = segmentationNode->GetSegmentation();
  if (!segmentation)
    {
    vtkErrorMacro("vtkMRMLSegmentationDisplayNode::GetAllSegmentIDs: No segmentation is associated to this display node");
    return;
    }
  segmentation->GetSegmentIDs(segmentIDs);
  if (visibleSegmentsOnly)
    {
    // remove non-visible segments
    for (std::vector<std::string>::iterator segmentIDIt = segmentIDs.begin(); segmentIDIt != segmentIDs.end();)
      {
      if (!this->GetSegmentVisibility(*segmentIDIt))
        {
        segmentIDIt = segmentIDs.erase(segmentIDIt);
        }
      else
        {
        ++segmentIDIt;
        }
      }
    }
}
