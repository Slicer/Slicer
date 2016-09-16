/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

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
#include <vtkMRMLSegmentationNode.h>

// SegmentationCore includes
#include "vtkSegmentation.h"
#include "vtkOrientedImageData.h"
#include "vtkTopologicalHierarchy.h"
#include "vtkSegmentationConverterFactory.h"

// VTK includes
#include <vtkLookupTable.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkStringArray.h>
#include <vtkVector.h>
#include <vtkVersion.h>

// STD includes
#include <algorithm>
#include <vector>
#include <sstream>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLSegmentationDisplayNode);

//----------------------------------------------------------------------------
vtkMRMLSegmentationDisplayNode::vtkMRMLSegmentationDisplayNode()
  : PreferredDisplayRepresentationName2D(NULL)
  , PreferredDisplayRepresentationName3D(NULL)
  , NumberOfAddedSegments(0)
  , SegmentListUpdateTime(0)
  , SegmentListUpdateSource(0)
  , Visibility3D(true)
  , Visibility2DFill(true)
  , Visibility2DOutline(true)
  , Opacity3D(1.0)
  , Opacity2DFill(0.5)
  , Opacity2DOutline(1.0)
{
  this->SliceIntersectionVisibility = true;
  this->SetBackfaceCulling(0); // segment models are not necessarily closed surfaces (e.g., ribbon models)

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

  of << indent << " Visibility3D=\"" << (this->Visibility3D ? "true" : "false") << "\"";
  of << indent << " Visibility2DFill=\"" << (this->Visibility2DFill ? "true" : "false") << "\"";
  of << indent << " Visibility2DOutline=\"" << (this->Visibility2DOutline ? "true" : "false") << "\"";
  of << indent << " Opacity3D=\"" << this->Opacity3D << "\"";
  of << indent << " Opacity2DFill=\"" << this->Opacity2DFill << "\"";
  of << indent << " Opacity2DOutline=\"" << this->Opacity2DOutline << "\"";

  this->UpdateSegmentList();

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
    else if (!strcmp(attName, "Visibility3D"))
      {
      this->Visibility3D = (strcmp(attValue,"true") ? false : true);
      }
    else if (!strcmp(attName, "Visibility2DFill"))
      {
      this->Visibility2DFill = (strcmp(attValue,"true") ? false : true);
      }
    else if (!strcmp(attName, "Visibility2DOutline"))
      {
      this->Visibility2DOutline = (strcmp(attValue,"true") ? false : true);
      }
    else if (!strcmp(attName, "Opacity3D"))
      {
      this->Opacity3D = vtkVariant(attValue).ToDouble();
      }
    else if (!strcmp(attName, "Opacity2DFill"))
      {
      this->Opacity2DFill = vtkVariant(attValue).ToDouble();
      }
    else if (!strcmp(attName, "Opacity2DOutline"))
      {
      this->Opacity2DOutline = vtkVariant(attValue).ToDouble();
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
    this->Visibility3D = node->Visibility3D;
    this->Visibility2DFill = node->Visibility2DFill;
    this->Visibility2DOutline = node->Visibility2DOutline;
    this->Opacity3D = node->Opacity3D;
    this->Opacity2DFill = node->Opacity2DFill;
    this->Opacity2DOutline = node->Opacity2DOutline;
    this->SegmentationDisplayProperties = node->SegmentationDisplayProperties;
    this->SegmentListUpdateSource = node->SegmentListUpdateSource;
    this->SegmentListUpdateTime = node->SegmentListUpdateTime;
    }

  this->EndModify(wasModifying);
}

//----------------------------------------------------------------------------
void vtkMRMLSegmentationDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << indent << " PreferredDisplayRepresentationName2D:   " << (this->PreferredDisplayRepresentationName2D ? this->PreferredDisplayRepresentationName2D : "NULL") << "\n";
  os << indent << " PreferredDisplayRepresentationName3D:   " << (this->PreferredDisplayRepresentationName3D ? this->PreferredDisplayRepresentationName3D : "NULL") << "\n";

  os << indent << " Visibility3D:   " << (this->Visibility3D ? "true" : "false") << "\n";
  os << indent << " Visibility2DFill:   " << (this->Visibility2DFill ? "true" : "false") << "\n";
  os << indent << " Visibility2DOutline:   " << (this->Visibility2DOutline ? "true" : "false") << "\n";
  os << indent << " Opacity3D:   " << this->Opacity3D << "\n";
  os << indent << " Opacity2DFill:   " << this->Opacity2DFill << "\n";
  os << indent << " Opacity2DOutline:   " << this->Opacity2DOutline << "\n";

  this->UpdateSegmentList();

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
bool vtkMRMLSegmentationDisplayNode::GetSegmentDisplayProperties(std::string segmentId, SegmentDisplayProperties &properties)
{
  this->UpdateSegmentList();

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

  // Save cached value of color
  // TODO: remove this when terminology infrastructure is in place
  vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(this->GetDisplayableNode());
  if (segmentationNode)
    {
    vtkSegmentation* segmentation = segmentationNode->GetSegmentation();
    vtkSegment* segment = segmentation->GetSegment(segmentId);
    if (segment)
      {
      segment->SetDefaultColorWithoutModifiedEvent(properties.Color);
      }
    }

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
void vtkMRMLSegmentationDisplayNode::SetSegmentDisplayPropertiesToDefault(const std::string& segmentId)
{
  vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(this->GetDisplayableNode());
  if (!segmentationNode)
    {
    vtkErrorMacro("vtkMRMLSegmentationDisplayNode: No segmentation node associated to this display node");
    return;
    }
  vtkSegmentation* segmentation = segmentationNode->GetSegmentation();
  vtkSegment* segment = segmentation->GetSegment(segmentId);
  if (!segment)
    {
    vtkErrorMacro("vtkMRMLSegmentationDisplayNode: segment not found by id " << segmentId);
    return;
    }

  int wasModifyingDisplayNode = this->StartModify();

  // Get color index tag used in merged labelmap and set a consecutive one if not found
  std::string colorIndexStr("");
  if (!segment->GetTag(vtkMRMLSegmentationDisplayNode::GetColorIndexTag(), colorIndexStr))
    {
    // Set color index to be one larger than the largest one found in the segments
    // (cannot simply go by number of segments because segments can be removed and then there will be a gap in the color indices)
    int maxColorIndex = 0;
    vtkSegmentation::SegmentMap segmentMap = segmentation->GetSegments();
    for (vtkSegmentation::SegmentMap::iterator segmentIt = segmentMap.begin(); segmentIt != segmentMap.end(); ++segmentIt)
      {
      if (segmentIt->second->GetTag(vtkMRMLSegmentationDisplayNode::GetColorIndexTag(), colorIndexStr))
        {
        int colorIndex = vtkVariant(colorIndexStr).ToInt();
        if (colorIndex > maxColorIndex)
          {
          maxColorIndex = colorIndex;
          }
        }
      }

    // Set color index as tag to segment
    segment->SetTag(vtkMRMLSegmentationDisplayNode::GetColorIndexTag(), maxColorIndex+1);
    }

  // Set segment color for merged labelmap
  double defaultColor[3] = { 0.0, 0.0, 0.0 };
  segment->GetDefaultColor(defaultColor);
  // Generate color if default color is the default gray
  this->NumberOfAddedSegments += 1;
  bool generateNewDefaultColor =
    (defaultColor[0] == vtkSegment::SEGMENT_COLOR_VALUE_INVALID[0]
    && defaultColor[1] == vtkSegment::SEGMENT_COLOR_VALUE_INVALID[1]
    && defaultColor[2] == vtkSegment::SEGMENT_COLOR_VALUE_INVALID[2]);
  if (generateNewDefaultColor)
    {
    this->GenerateSegmentColor(defaultColor);
    }

  // Add entry in segment display properties
  vtkMRMLSegmentationDisplayNode::SegmentDisplayProperties properties;
  properties.Color[0] = defaultColor[0];
  properties.Color[1] = defaultColor[1];
  properties.Color[2] = defaultColor[2];
  properties.Visible = true;
  properties.Visible3D = true;
  properties.Visible2DFill = true;
  properties.Visible2DOutline = true;
  properties.Opacity3D = 1.0;
  properties.Opacity2DFill = 1.0;
  properties.Opacity2DOutline = 1.0;
  this->SetSegmentDisplayProperties(segmentId, properties);

  this->EndModify(wasModifyingDisplayNode);
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
  if (segmentation->IsMasterRepresentationPolyData())
    {
    return std::string(segmentation->GetMasterRepresentationName());
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

//---------------------------------------------------------------------------
void vtkMRMLSegmentationDisplayNode::GetVisibleSegmentIDs(vtkStringArray* segmentIDs)
{
  if (segmentIDs == NULL)
    {
    vtkErrorMacro("vtkMRMLSegmentationDisplayNode::GetVisibleSegmentIDs failed: invalid segmentIDs");
    return;
    }
  std::vector<std::string> segmentIDsVector;
  this->GetAllSegmentIDs(segmentIDsVector, true);
  segmentIDs->Reset();
  for (std::vector<std::string>::iterator it = segmentIDsVector.begin(); it != segmentIDsVector.end(); ++it)
    {
    segmentIDs->InsertNextValue(*it);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationDisplayNode::UpdateSegmentList()
{
  vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(this->GetDisplayableNode());
  if (!segmentationNode)
    {
    vtkErrorMacro("vtkMRMLSegmentationDisplayNode::UpdateSegmentList: No segmentation node associated to this display node");
    return;
    }
  vtkSegmentation* segmentation = segmentationNode->GetSegmentation();
  if (this->SegmentListUpdateSource == segmentation && this->SegmentListUpdateTime >= segmentation->GetMTime())
    {
    // already up-to-date
    return;
    }
  this->SegmentListUpdateTime = segmentation->GetMTime();
  this->SegmentListUpdateSource = segmentation;

  int wasModifyingDisplayNode = this->StartModify();

  // Remove unused segment display properties and colors
  // Get list of segment IDs that we have display properties for but does not exist in
  // the segmentation anymore.
  std::vector<std::string> orphanSegmentIds;
  for (SegmentDisplayPropertiesMap::iterator it = this->SegmentationDisplayProperties.begin();
    it != this->SegmentationDisplayProperties.end(); ++it)
    {
    if (segmentation->GetSegment(it->first) == NULL)
      {
      // the segment does not exist in segmentation
      orphanSegmentIds.push_back(it->first);
      }
    }
  // Delete unused properties and color table entries
  for (std::vector<std::string>::iterator orphanSegmentIdIt = orphanSegmentIds.begin();
    orphanSegmentIdIt != orphanSegmentIds.end(); ++orphanSegmentIdIt)
    {
    // Remove segment display properties
    this->RemoveSegmentDisplayProperties(*orphanSegmentIdIt);
    }

  // Add missing segment display properties
  // Get segment list of segments that do not have display properties.
  vtkSegmentation::SegmentMap segmentMap = segmentation->GetSegments();
  std::vector<std::string> missingSegmentIDs;
  for (vtkSegmentation::SegmentMap::iterator segmentIt = segmentMap.begin(); segmentIt != segmentMap.end(); ++segmentIt)
    {
    if (this->SegmentationDisplayProperties.find(segmentIt->first) == this->SegmentationDisplayProperties.end())
      {
      // the segment does not exist in segmentation
      missingSegmentIDs.push_back(segmentIt->first);
      }
    }
  // Add missing properties
  for (std::vector<std::string>::iterator missingSegmentIdIt = missingSegmentIDs.begin();
    missingSegmentIdIt != missingSegmentIDs.end(); ++missingSegmentIdIt)
    {
    this->SetSegmentDisplayPropertiesToDefault(*missingSegmentIdIt);
    }

  this->EndModify(wasModifyingDisplayNode);
}
