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
#include <random>
#include <sstream>
#include <vector>

//----------------------------------------------------------------------------
const double vtkMRMLSegmentationDisplayNode::SEGMENT_COLOR_NO_OVERRIDE = -1.0;

static const char* SegmentColorGeneratorSourceColorNodeReferenceRole = "segmentColorGenerator";

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLSegmentationDisplayNode);

//----------------------------------------------------------------------------
vtkMRMLSegmentationDisplayNode::vtkMRMLSegmentationDisplayNode()
{
  this->SetBackfaceCulling(0); // segment models are not necessarily closed surfaces (e.g., ribbon models)
  this->Visibility2D = 1;      // show slice intersections by default

  this->SegmentationDisplayProperties.clear();

  this->AddNodeReferenceRole(SegmentColorGeneratorSourceColorNodeReferenceRole);
}

//----------------------------------------------------------------------------
vtkMRMLSegmentationDisplayNode::~vtkMRMLSegmentationDisplayNode()
{
  this->SetPreferredDisplayRepresentationName2D(nullptr);
  this->SetPreferredDisplayRepresentationName3D(nullptr);
  this->SegmentationDisplayProperties.clear();
}

//----------------------------------------------------------------------------
void vtkMRMLSegmentationDisplayNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkMRMLWriteXMLBeginMacro(of);
  vtkMRMLWriteXMLStringMacro(PreferredDisplayRepresentationName2D, PreferredDisplayRepresentationName2D);
  vtkMRMLWriteXMLStringMacro(PreferredDisplayRepresentationName3D, PreferredDisplayRepresentationName3D);
  vtkMRMLWriteXMLBooleanMacro(Visibility2DFill, Visibility2DFill);
  vtkMRMLWriteXMLBooleanMacro(Visibility2DOutline, Visibility2DOutline);
  vtkMRMLWriteXMLFloatMacro(Opacity3D, Opacity3D);
  vtkMRMLWriteXMLFloatMacro(Opacity2DFill, Opacity2DFill);
  vtkMRMLWriteXMLFloatMacro(Opacity2DOutline, Opacity2DOutline);
  vtkMRMLWriteXMLBooleanMacro(RemoveUnusedDisplayProperties, RemoveUnusedDisplayProperties);
  vtkMRMLWriteXMLBooleanMacro(ClippingCapSurface, ClippingCapSurface);
  vtkMRMLWriteXMLFloatMacro(ClippingCapOpacity, ClippingCapOpacity);
  vtkMRMLWriteXMLBooleanMacro(ClippingOutline, ClippingOutline);

  this->UpdateSegmentList();

  of << " SegmentationDisplayProperties=\"";
  for (SegmentDisplayPropertiesMap::iterator propIt = this->SegmentationDisplayProperties.begin(); propIt != this->SegmentationDisplayProperties.end(); ++propIt)
  {
    of << vtkMRMLNode::URLEncodeString(propIt->first.c_str())                          //
       << " OverrideColorR:" << propIt->second.OverrideColor[0]                        //
       << " OverrideColorG:" << propIt->second.OverrideColor[1]                        //
       << " OverrideColorB:" << propIt->second.OverrideColor[2]                        //
       << " Visible:" << (propIt->second.Visible ? "true" : "false")                   //
       << " Visible3D:" << (propIt->second.Visible3D ? "true" : "false")               //
       << " Visible2DFill:" << (propIt->second.Visible2DFill ? "true" : "false")       //
       << " Visible2DOutline:" << (propIt->second.Visible2DOutline ? "true" : "false") //
       << " Opacity3D:" << propIt->second.Opacity3D                                    //
       << " Opacity2DFill:" << propIt->second.Opacity2DFill                            //
       << " Opacity2DOutline:" << propIt->second.Opacity2DOutline                      //
       << " Pickable:" << (propIt->second.Pickable ? "true" : "false") << "|";
  }
  of << "\"";
  vtkMRMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLSegmentationDisplayNode::ReadXMLAttributes(const char** atts)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLStringMacro(PreferredDisplayRepresentationName2D, PreferredDisplayRepresentationName2D);
  vtkMRMLReadXMLStringMacro(PreferredDisplayRepresentationName3D, PreferredDisplayRepresentationName3D);
  vtkMRMLReadXMLBooleanMacro(Visibility2DFill, Visibility2DFill);
  vtkMRMLReadXMLBooleanMacro(Visibility2DOutline, Visibility2DOutline);
  vtkMRMLReadXMLFloatMacro(Opacity3D, Opacity3D);
  vtkMRMLReadXMLFloatMacro(Opacity2DFill, Opacity2DFill);
  vtkMRMLReadXMLFloatMacro(Opacity2DOutline, Opacity2DOutline);
  vtkMRMLReadXMLBooleanMacro(RemoveUnusedDisplayProperties, RemoveUnusedDisplayProperties);
  vtkMRMLReadXMLBooleanMacro(ClippingCapSurface, ClippingCapSurface);
  vtkMRMLReadXMLFloatMacro(ClippingCapOpacity, ClippingCapOpacity);
  vtkMRMLReadXMLBooleanMacro(ClippingOutline, ClippingOutline);
  if (!strcmp(xmlReadAttName, "SegmentationDisplayProperties"))
  {
    // attValue: "Segment_0 OverrideColorR:-1 OverrideColorG:-1 ... Opacity2DOutline:1|Segment_1 OverrideColorR:1 OverrideColorG:0.8...|"
    std::stringstream segmentsDisplayProperties(xmlReadAttValue); // properties of all segments
    std::string segmentDisplayPropertiesString;                   // properties of a single segment
    while (std::getline(segmentsDisplayProperties, segmentDisplayPropertiesString, '|'))
    {
      // segmentDisplayPropertiesString: "Segment_0 OverrideColorR:0.2 OverrideColorG:0.501961 ... Opacity2DOutline:1"
      std::stringstream segmentDisplayProperties(segmentDisplayPropertiesString);
      std::string id;
      segmentDisplayProperties >> id;
      SegmentDisplayProperties props;
      std::string segmentDisplayPropertyString; // properties of a single segment
      while (std::getline(segmentDisplayProperties, segmentDisplayPropertyString, ' '))
      {
        // segmentDisplayPropertyString: "OverrideColorR:0.2"
        if (segmentDisplayPropertyString.empty())
        {
          // multiple spaces between properties, just get the next item
          continue;
        }
        size_t colonIndex = segmentDisplayPropertyString.find(':');
        if (colonIndex == std::string::npos)
        {
          // no colon found, not a valid property, ignore it
          vtkErrorMacro("Invalid property found in attribute of " << (this->ID ? this->ID : "(unknown)") << " node: " << segmentDisplayPropertyString);
          continue;
        }
        std::string propertyName = segmentDisplayPropertyString.substr(0, colonIndex);        // "OverrideColorR"
        std::stringstream propertyValue(segmentDisplayPropertyString.substr(colonIndex + 1)); // "0.2"
        // clang-format off
        if (propertyName == "OverrideColorR") { propertyValue >> props.OverrideColor[0]; }
        else if (propertyName == "OverrideColorG") { propertyValue >> props.OverrideColor[1]; }
        else if (propertyName == "OverrideColorB") { propertyValue >> props.OverrideColor[2]; }
        // clang-format on

        // for backward compatibility
        // clang-format off
        else if (propertyName == "ColorR") { propertyValue >> props.OverrideColor[0]; }
        else if (propertyName == "ColorG") { propertyValue >> props.OverrideColor[1]; }
        else if (propertyName == "ColorB") { propertyValue >> props.OverrideColor[2]; }

        else if (propertyName == "Opacity3D") { propertyValue >> props.Opacity3D; }
        else if (propertyName == "Opacity2DFill") { propertyValue >> props.Opacity2DFill; }
        else if (propertyName == "Opacity2DOutline") { propertyValue >> props.Opacity2DOutline; }
        // clang-format off
        else
        {
          // boolean values
          std::string booleanValueString;
          propertyValue >> booleanValueString;
          bool booleanValue = booleanValueString.compare("true") ? false : true;
          // clang-format off
          if (propertyName == "Visible") { props.Visible = booleanValue; }
          else if (propertyName == "Visible3D") { props.Visible3D = booleanValue; }
          else if (propertyName == "Visible2DFill") { props.Visible2DFill = booleanValue; }
          else if (propertyName == "Visible2DOutline") { props.Visible2DOutline = booleanValue; }
          // clang-format on
          else if (propertyName == "Pickable")
          {
            // Pickable property needs to be set to true if not specified
            props.Pickable = booleanValueString.compare("false") ? true : false;
          }
        }
      }
      this->SetSegmentDisplayProperties(vtkMRMLNode::URLDecodeString(id.c_str()), props);
    }
  }
  vtkMRMLReadXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLSegmentationDisplayNode::CopyContent(vtkMRMLNode* anode, bool deepCopy /*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkMRMLSegmentationDisplayNode* node = vtkMRMLSegmentationDisplayNode::SafeDownCast(anode);
  if (!node)
  {
    return;
  }

  vtkMRMLCopyBeginMacro(anode);
  vtkMRMLCopyStringMacro(PreferredDisplayRepresentationName2D);
  vtkMRMLCopyStringMacro(PreferredDisplayRepresentationName3D);
  vtkMRMLCopyBooleanMacro(Visibility2DFill);
  vtkMRMLCopyBooleanMacro(Visibility2DOutline);
  vtkMRMLCopyFloatMacro(Opacity3D);
  vtkMRMLCopyFloatMacro(Opacity2DFill);
  vtkMRMLCopyFloatMacro(Opacity2DOutline);
  vtkMRMLCopyBooleanMacro(RemoveUnusedDisplayProperties);
  vtkMRMLCopyBooleanMacro(ClippingCapSurface);
  vtkMRMLCopyFloatMacro(ClippingCapOpacity);
  vtkMRMLCopyBooleanMacro(ClippingOutline);
  if (this->SegmentationDisplayProperties != node->SegmentationDisplayProperties)
  {
    this->SegmentationDisplayProperties = node->SegmentationDisplayProperties;
    this->Modified();
  }
  // Reset segment list source to allow writing display properties to XML,
  // even if referenced segmentation node cannot be found (for example,
  // if SegmentListUpdateSource was not set to nullptr then segment display properties
  // would not be saved in scene views).
  this->SegmentListUpdateSource = nullptr;
  this->SegmentListUpdateTime = 0;
  vtkMRMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLSegmentationDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);

  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintStringMacro(PreferredDisplayRepresentationName2D);
  vtkMRMLPrintStringMacro(PreferredDisplayRepresentationName3D);
  vtkMRMLPrintBooleanMacro(Visibility2DFill);
  vtkMRMLPrintBooleanMacro(Visibility2DOutline);
  vtkMRMLPrintFloatMacro(Opacity3D);
  vtkMRMLPrintFloatMacro(Opacity2DFill);
  vtkMRMLPrintFloatMacro(Opacity2DOutline);
  vtkMRMLPrintBooleanMacro(RemoveUnusedDisplayProperties);
  vtkMRMLPrintBooleanMacro(ClippingCapSurface);
  vtkMRMLPrintFloatMacro(ClippingCapOpacity);
  vtkMRMLPrintBooleanMacro(ClippingOutline);
  this->UpdateSegmentList();
  os << indent << " SegmentationDisplayProperties:\n";
  for (SegmentDisplayPropertiesMap::iterator propIt = this->SegmentationDisplayProperties.begin(); propIt != this->SegmentationDisplayProperties.end(); ++propIt)
  {
    os << indent << "   SegmentID=" << propIt->first                                    //
       << ", OverrideColor=(" << propIt->second.OverrideColor[0]                        //
       << "," << propIt->second.OverrideColor[1]                                        //
       << "," << propIt->second.OverrideColor[2]                                        //
       << ")"                                                                           //
       << ", Visible=" << (propIt->second.Visible ? "true" : "false")                   //
       << ", Visible3D=" << (propIt->second.Visible3D ? "true" : "false")               //
       << ", Visible2DFill=" << (propIt->second.Visible2DFill ? "true" : "false")       //
       << ", Visible2DOutline=" << (propIt->second.Visible2DOutline ? "true" : "false") //
       << ", Opacity3D=" << propIt->second.Opacity3D                                    //
       << ", Opacity2DFill=" << propIt->second.Opacity2DFill                            //
       << ", Opacity2DOutline=" << propIt->second.Opacity2DOutline                      //
       << ", Pickable=" << (propIt->second.Pickable ? "true" : "false") << "\n";
  }
  vtkMRMLPrintEndMacro();
}

//---------------------------------------------------------------------------
bool vtkMRMLSegmentationDisplayNode::GetSegmentDisplayProperties(std::string segmentId, SegmentDisplayProperties& properties)
{
  this->UpdateSegmentList();

  SegmentDisplayPropertiesMap::iterator propsIt = this->SegmentationDisplayProperties.find(segmentId);
  if (propsIt == this->SegmentationDisplayProperties.end())
  {
    vtkWarningMacro("vtkMRMLSegmentationDisplayNode::GetSegmentDisplayProperties: no display properties are found for segment ID=" << segmentId << ", return default");
    SegmentDisplayProperties defaultProperties;
    properties = defaultProperties;
    return false;
  }
  properties = propsIt->second;
  return true;
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationDisplayNode::SetSegmentDisplayProperties(std::string segmentId, SegmentDisplayProperties& properties)
{
  this->UpdateSegmentList();
  SegmentDisplayPropertiesMap::iterator propsIt = this->SegmentationDisplayProperties.find(segmentId);
  bool modified = false;
  if (propsIt == this->SegmentationDisplayProperties.end())
  {
    // If not found then add
    SegmentDisplayProperties newPropertiesEntry;
    newPropertiesEntry.OverrideColor[0] = properties.OverrideColor[0];
    newPropertiesEntry.OverrideColor[1] = properties.OverrideColor[1];
    newPropertiesEntry.OverrideColor[2] = properties.OverrideColor[2];
    newPropertiesEntry.Visible = properties.Visible;
    newPropertiesEntry.Visible3D = properties.Visible3D;
    newPropertiesEntry.Visible2DFill = properties.Visible2DFill;
    newPropertiesEntry.Visible2DOutline = properties.Visible2DOutline;
    newPropertiesEntry.Opacity3D = properties.Opacity3D;
    newPropertiesEntry.Opacity2DFill = properties.Opacity2DFill;
    newPropertiesEntry.Opacity2DOutline = properties.Opacity2DOutline;
    newPropertiesEntry.Pickable = properties.Pickable;
    this->SegmentationDisplayProperties[segmentId] = newPropertiesEntry;
    modified = true;
  }
  else
  {
    // If found then replace values
    for (int i = 0; i < 3; i++)
    {
      if (propsIt->second.OverrideColor[i] != properties.OverrideColor[i])
      {
        propsIt->second.OverrideColor[i] = properties.OverrideColor[i];
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
    if (propsIt->second.Pickable != properties.Pickable)
    {
      propsIt->second.Pickable = properties.Pickable;
      modified = true;
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
  double color[3] = { vtkSegment::SEGMENT_COLOR_INVALID[0], vtkSegment::SEGMENT_COLOR_INVALID[1], vtkSegment::SEGMENT_COLOR_INVALID[2] };
  this->GetSegmentColor(segmentID, color);

  vtkVector3d colorVtk(color[0], color[1], color[2]);
  return colorVtk;
}

//---------------------------------------------------------------------------
bool vtkMRMLSegmentationDisplayNode::GetSegmentColor(std::string segmentID, double* color)
{
  if (!color)
  {
    vtkErrorMacro("GetSegmentColor: Invalid output color array");
    return false;
  }

  // Invalidate color
  color[0] = vtkSegment::SEGMENT_COLOR_INVALID[0];
  color[1] = vtkSegment::SEGMENT_COLOR_INVALID[1];
  color[2] = vtkSegment::SEGMENT_COLOR_INVALID[2];

  // Get display properties and return override color if set
  this->UpdateSegmentList();
  SegmentDisplayPropertiesMap::iterator propsIt = this->SegmentationDisplayProperties.find(segmentID);
  if (propsIt != this->SegmentationDisplayProperties.end() && propsIt->second.OverrideColor[0] >= 0.0 && propsIt->second.OverrideColor[1] >= 0.0
      && propsIt->second.OverrideColor[2] >= 0.0)
  {
    // If found and overridden, then return the override color
    color[0] = propsIt->second.OverrideColor[0];
    color[1] = propsIt->second.OverrideColor[1];
    color[2] = propsIt->second.OverrideColor[2];
    return true;
  }

  // Get segment
  vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(this->GetDisplayableNode());
  if (!segmentationNode || !segmentationNode->GetSegmentation())
  {
    vtkErrorMacro("GetSegmentColor: No valid segmentation node associated to this display node");
    return false;
  }
  vtkSegmentation* segmentation = segmentationNode->GetSegmentation();
  vtkSegment* segment = segmentation->GetSegment(segmentID);
  if (!segment)
  {
    vtkErrorMacro("GetSegmentColor: segment not found by id " << segmentID);
    return false;
  }

  // Get color from segment
  segment->GetColor(color);

  return true;
}

//---------------------------------------------------------------------------
bool vtkMRMLSegmentationDisplayNode::GetSegmentColor(std::string segmentID, double& r, double& g, double& b)
{
  double color[3] = { vtkSegment::SEGMENT_COLOR_INVALID[0], vtkSegment::SEGMENT_COLOR_INVALID[1], vtkSegment::SEGMENT_COLOR_INVALID[2] };
  if (!this->GetSegmentColor(segmentID, color))
  {
    return false;
  }

  r = color[0];
  g = color[1];
  b = color[2];
  return true;
}

//---------------------------------------------------------------------------
vtkVector3d vtkMRMLSegmentationDisplayNode::GetSegmentOverrideColor(std::string segmentID)
{
  this->UpdateSegmentList();
  SegmentDisplayPropertiesMap::iterator propsIt = this->SegmentationDisplayProperties.find(segmentID);
  if (propsIt == this->SegmentationDisplayProperties.end())
  {
    vtkErrorMacro("GetSegmentOverrideColor: No display properties found for segment with ID " << segmentID);
    vtkVector3d color(SEGMENT_COLOR_NO_OVERRIDE, SEGMENT_COLOR_NO_OVERRIDE, SEGMENT_COLOR_NO_OVERRIDE);
    return color;
  }

  vtkVector3d color(propsIt->second.OverrideColor[0], propsIt->second.OverrideColor[1], propsIt->second.OverrideColor[2]);
  return color;
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationDisplayNode::SetSegmentOverrideColor(std::string segmentID, double r, double g, double b)
{
  // Set override color in display properties
  SegmentDisplayProperties properties;
  this->GetSegmentDisplayProperties(segmentID, properties);
  properties.OverrideColor[0] = r;
  properties.OverrideColor[1] = g;
  properties.OverrideColor[2] = b;
  this->SetSegmentDisplayProperties(segmentID, properties);
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationDisplayNode::SetSegmentOverrideColor(std::string segmentID, vtkVector3d overrideColor)
{
  SegmentDisplayProperties properties;
  this->GetSegmentDisplayProperties(segmentID, properties);
  properties.OverrideColor[0] = overrideColor.GetX();
  properties.OverrideColor[1] = overrideColor.GetY();
  properties.OverrideColor[2] = overrideColor.GetZ();
  this->SetSegmentDisplayProperties(segmentID, properties);
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationDisplayNode::UnsetSegmentOverrideColor(std::string segmentID)
{
  this->SetSegmentOverrideColor(segmentID, SEGMENT_COLOR_NO_OVERRIDE, SEGMENT_COLOR_NO_OVERRIDE, SEGMENT_COLOR_NO_OVERRIDE);
}

//---------------------------------------------------------------------------
bool vtkMRMLSegmentationDisplayNode::GetSegmentVisibility(std::string segmentID)
{
  this->UpdateSegmentList();
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
  this->GetSegmentIDs(segmentIDs, false);
  for (std::vector<std::string>::iterator segmentIDIt = segmentIDs.begin(); segmentIDIt != segmentIDs.end(); ++segmentIDIt)
  {
    this->SetSegmentVisibility(*segmentIDIt, visible);
  }
}

//---------------------------------------------------------------------------
bool vtkMRMLSegmentationDisplayNode::GetSegmentVisibility3D(std::string segmentID)
{
  this->UpdateSegmentList();
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
  this->GetSegmentIDs(segmentIDs, changeVisibleSegmentsOnly);
  for (std::vector<std::string>::iterator segmentIDIt = segmentIDs.begin(); segmentIDIt != segmentIDs.end(); ++segmentIDIt)
  {
    this->SetSegmentVisibility3D(*segmentIDIt, visible);
  }
}

//---------------------------------------------------------------------------
bool vtkMRMLSegmentationDisplayNode::GetSegmentVisibility2DFill(std::string segmentID)
{
  this->UpdateSegmentList();
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
  this->GetSegmentIDs(segmentIDs, changeVisibleSegmentsOnly);
  for (std::vector<std::string>::iterator segmentIDIt = segmentIDs.begin(); segmentIDIt != segmentIDs.end(); ++segmentIDIt)
  {
    this->SetSegmentVisibility2DFill(*segmentIDIt, visible);
  }
}

//---------------------------------------------------------------------------
bool vtkMRMLSegmentationDisplayNode::GetSegmentVisibility2DOutline(std::string segmentID)
{
  this->UpdateSegmentList();
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
  this->GetSegmentIDs(segmentIDs, changeVisibleSegmentsOnly);
  for (std::vector<std::string>::iterator segmentIDIt = segmentIDs.begin(); segmentIDIt != segmentIDs.end(); ++segmentIDIt)
  {
    this->SetSegmentVisibility2DOutline(*segmentIDIt, visible);
  }
}

//---------------------------------------------------------------------------
double vtkMRMLSegmentationDisplayNode::GetSegmentOpacity3D(std::string segmentID)
{
  this->UpdateSegmentList();
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
  this->GetSegmentIDs(segmentIDs, changeVisibleSegmentsOnly);
  for (std::vector<std::string>::iterator segmentIDIt = segmentIDs.begin(); segmentIDIt != segmentIDs.end(); ++segmentIDIt)
  {
    this->SetSegmentOpacity3D(*segmentIDIt, opacity);
  }
}

//---------------------------------------------------------------------------
double vtkMRMLSegmentationDisplayNode::GetSegmentOpacity2DFill(std::string segmentID)
{
  this->UpdateSegmentList();
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
  this->GetSegmentIDs(segmentIDs, changeVisibleSegmentsOnly);
  for (std::vector<std::string>::iterator segmentIDIt = segmentIDs.begin(); segmentIDIt != segmentIDs.end(); ++segmentIDIt)
  {
    this->SetSegmentOpacity2DFill(*segmentIDIt, opacity);
  }
}

//---------------------------------------------------------------------------
double vtkMRMLSegmentationDisplayNode::GetSegmentOpacity2DOutline(std::string segmentID)
{
  this->UpdateSegmentList();
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
  this->GetSegmentIDs(segmentIDs, changeVisibleSegmentsOnly);
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
  this->GetSegmentIDs(segmentIDs, changeVisibleSegmentsOnly);
  for (std::vector<std::string>::iterator segmentIDIt = segmentIDs.begin(); segmentIDIt != segmentIDs.end(); ++segmentIDIt)
  {
    this->SetSegmentOpacity(*segmentIDIt, opacity);
  }
}

//---------------------------------------------------------------------------
bool vtkMRMLSegmentationDisplayNode::GetSegmentPickable(std::string segmentID)
{
  this->UpdateSegmentList();
  SegmentDisplayPropertiesMap::iterator propsIt = this->SegmentationDisplayProperties.find(segmentID);
  if (propsIt == this->SegmentationDisplayProperties.end())
  {
    vtkErrorMacro("GetSegmentPickable: No display properties found for segment with ID " << segmentID);
    SegmentDisplayProperties defaultProperties;
    return defaultProperties.Pickable;
  }
  return propsIt->second.Pickable;
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationDisplayNode::SetSegmentPickable(std::string segmentID, bool pickable)
{
  SegmentDisplayProperties properties;
  this->GetSegmentDisplayProperties(segmentID, properties);
  properties.Pickable = pickable;
  this->SetSegmentDisplayProperties(segmentID, properties);
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationDisplayNode::SetAllSegmentsPickable(bool pickable, bool changeVisibleSegmentsOnly /* = false*/)
{
  std::vector<std::string> segmentIDs;
  this->GetSegmentIDs(segmentIDs, changeVisibleSegmentsOnly);
  for (std::vector<std::string>::iterator segmentIDIt = segmentIDs.begin(); segmentIDIt != segmentIDs.end(); ++segmentIDIt)
  {
    this->SetSegmentPickable(*segmentIDIt, pickable);
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

  MRMLNodeModifyBlocker blocker(this);
  MRMLNodeModifyBlocker blocker2(segmentationNode);

  // Set segment color to a default if invalid (empty)
  double color[3] = { 0.0, 0.0, 0.0 };
  segment->GetColor(color);
  // Generate color if default color is the default gray
  bool generateNewColor = (color[0] == vtkSegment::SEGMENT_COLOR_INVALID[0]    //
                           && color[1] == vtkSegment::SEGMENT_COLOR_INVALID[1] //
                           && color[2] == vtkSegment::SEGMENT_COLOR_INVALID[2]);
  if (generateNewColor)
  {
    this->GenerateSegmentColor(color, ++this->NumberOfGeneratedColors);

    // Set color to segment (no override is specified by default, so segment color is used)
    segment->SetColor(color);
  }

  // Add entry in segment display properties
  vtkMRMLSegmentationDisplayNode::SegmentDisplayProperties properties;
  properties.OverrideColor[0] = SEGMENT_COLOR_NO_OVERRIDE; // Override color stays invalid. In this case the color stored in the segment is shown
  properties.OverrideColor[1] = SEGMENT_COLOR_NO_OVERRIDE;
  properties.OverrideColor[2] = SEGMENT_COLOR_NO_OVERRIDE;
  properties.Visible = true;
  properties.Visible3D = true;
  properties.Visible2DFill = true;
  properties.Visible2DOutline = true;
  properties.Opacity3D = 1.0;
  properties.Opacity2DFill = 1.0;
  properties.Opacity2DOutline = 1.0;
  properties.Pickable = true;
  this->SetSegmentDisplayProperties(segmentId, properties);
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
  this->NumberOfGeneratedColors = 0;
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
  if (!this->PreferredDisplayRepresentationName3D || !segmentation->CreateRepresentation(this->PreferredDisplayRepresentationName3D))
  {
    return false;
  }

  // Get displayed 3D representation (always poly data)
  std::string displayedPolyDataRepresentationName = this->GetDisplayRepresentationName3D();

  // Make sure the segment display properties are updated
  this->UpdateSegmentList();

  // Assemble segment polydatas into a collection that can be fed to topological hierarchy algorithm
  vtkSmartPointer<vtkPolyDataCollection> segmentPolyDataCollection = vtkSmartPointer<vtkPolyDataCollection>::New();
  for (SegmentDisplayPropertiesMap::iterator propIt = this->SegmentationDisplayProperties.begin(); propIt != this->SegmentationDisplayProperties.end(); ++propIt)
  {
    // Get segment
    vtkSegment* currentSegment = segmentation->GetSegment(propIt->first);
    if (!currentSegment)
    {
      vtkErrorMacro("CalculateAutoOpacitiesForSegments: Mismatch in display properties and segments!");
      continue;
    }

    // Get poly data from segment
    vtkPolyData* currentPolyData = vtkPolyData::SafeDownCast(currentSegment->GetRepresentation(displayedPolyDataRepresentationName.c_str()));
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
  for (int i = 0; i < levels->GetNumberOfTuples(); ++i)
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
  for (idx = 0, propIt = this->SegmentationDisplayProperties.begin(); idx < levels->GetNumberOfTuples(); ++idx, ++propIt)
  {
    int level = levels->GetValue(idx);

    // The opacity level is set evenly distributed between 0 and 1 (excluding 0)
    // according to the topological hierarchy level of the segment
    double opacity = 1.0 - ((double)level) / (numberOfLevels + 1);
    propIt->second.Opacity3D = opacity;
  }

  this->Modified();
  return true;
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationDisplayNode::SetSegmentColorGeneratorSourceColorNodeID(const char* colorNodeID)
{
  this->SetNodeReferenceID(SegmentColorGeneratorSourceColorNodeReferenceRole, colorNodeID);
}

//---------------------------------------------------------------------------
const char* vtkMRMLSegmentationDisplayNode::GetSegmentColorGeneratorSourceColorNodeID()
{
  return this->GetNodeReferenceID(SegmentColorGeneratorSourceColorNodeReferenceRole);
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationDisplayNode::GenerateSegmentColor(double color[3], int colorNumber /*=0*/)
{
  if (!this->Scene)
  {
    vtkErrorMacro("GenerateSegmentColor: Invalid MRML scene!");
    return;
  }

  vtkMRMLColorTableNode* colorTableNode = vtkMRMLColorTableNode::SafeDownCast(this->GetNodeReference(SegmentColorGeneratorSourceColorNodeReferenceRole));
  if (!colorTableNode)
  {
    // Get default generic anatomy color table if no other color table was chosen
    colorTableNode = vtkMRMLColorTableNode::SafeDownCast(this->Scene->GetNodeByID("vtkMRMLColorTableNodeFileGenericAnatomyColors.txt"));
  }

  if (colorNumber == -1                           // random color was requested
      || !colorTableNode                          // color node was not found
      || colorTableNode->GetNumberOfColors() <= 1 // color node is empty or only contains background color
  )
  {
    // Generate random color if default color table is not available (such as in logic tests)
    std::default_random_engine randomGenerator(std::random_device{}());
    color[0] = static_cast<double>(randomGenerator()) / randomGenerator.max();
    color[1] = static_cast<double>(randomGenerator()) / randomGenerator.max();
    color[2] = static_cast<double>(randomGenerator()) / randomGenerator.max();
    return;
  }

  // Default is to use NumberOfGeneratedColors
  if (colorNumber == 0)
  {
    this->NumberOfGeneratedColors++;
    colorNumber = this->NumberOfGeneratedColors;
  }

  // Keep the color index in the valid range of colors in the color table:
  // - colorNumber == 1 means the first usable color (colorNumber == 0 means automatic coloring)
  // - We skip the first color (0) because that is the background color, which is usually set to
  //   black (not a good segment color).
  colorNumber = 1 + ((colorNumber - 1) % (colorTableNode->GetNumberOfColors() - 1));

  // Get color corresponding to the number of added segments (which is incremented in
  // vtkMRMLSegmentationNode::AddSegmentDisplayProperties every time a new segment display
  // properties entry is added
  double currentColor[4] = { 0.0, 0.0, 0.0, 0.0 };
  colorTableNode->GetColor(colorNumber, currentColor);
  color[0] = currentColor[0];
  color[1] = currentColor[1];
  color[2] = currentColor[2];
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationDisplayNode::GenerateSegmentColor(double& r, double& g, double& b)
{
  double color[3] = { 0.0, 0.0, 0.0 };
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
  vtkSegment* firstSegment = segmentation->GetNthSegment(0);

  // If preferred representation is defined and exists then use that (double check it is poly data)
  if (this->PreferredDisplayRepresentationName3D)
  {
    vtkDataObject* preferredRepresentation = firstSegment->GetRepresentation(this->PreferredDisplayRepresentationName3D);
    if (vtkPolyData::SafeDownCast(preferredRepresentation))
    {
      return std::string(this->PreferredDisplayRepresentationName3D);
    }
  }

  // Otherwise if source representation is poly data then use that
  if (segmentation->IsSourceRepresentationPolyData())
  {
    return std::string(segmentation->GetSourceRepresentationName());
  }

  // Otherwise return first poly data representation if any
  std::vector<std::string> containedRepresentationNames;
  segmentation->GetContainedRepresentationNames(containedRepresentationNames);
  for (std::vector<std::string>::iterator reprIt = containedRepresentationNames.begin(); reprIt != containedRepresentationNames.end(); ++reprIt)
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

  // Otherwise return source representation
  return std::string(segmentation->GetSourceRepresentationName());
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationDisplayNode::GetPolyDataRepresentationNames(std::set<std::string>& representationNames)
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

    vtkSmartPointer<vtkDataObject> sourceObject =
      vtkSmartPointer<vtkDataObject>::Take(currentRule->ConstructRepresentationObjectByRepresentation(currentRule->GetSourceRepresentationName()));
    vtkPolyData* sourcePolyData = vtkPolyData::SafeDownCast(sourceObject);
    if (sourcePolyData)
    {
      representationNames.insert(std::string(currentRule->GetSourceRepresentationName()));
    }

    vtkSmartPointer<vtkDataObject> targetObject =
      vtkSmartPointer<vtkDataObject>::Take(currentRule->ConstructRepresentationObjectByRepresentation(currentRule->GetTargetRepresentationName()));
    vtkPolyData* targetPolyData = vtkPolyData::SafeDownCast(targetObject);
    if (targetPolyData)
    {
      representationNames.insert(std::string(currentRule->GetTargetRepresentationName()));
    }
  }
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationDisplayNode::GetVisibleSegmentIDs(std::vector<std::string>& segmentIDs)
{
  this->GetSegmentIDs(segmentIDs, true);
}

//---------------------------------------------------------------------------
std::vector<std::string> vtkMRMLSegmentationDisplayNode::GetVisibleSegmentIDs()
{
  std::vector<std::string> visibleSegmentIDs;
  this->GetVisibleSegmentIDs(visibleSegmentIDs);
  return visibleSegmentIDs;
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationDisplayNode::GetSegmentIDs(std::vector<std::string>& segmentIDs, bool visibleSegmentsOnly)
{
  segmentIDs.clear();
  vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(this->GetDisplayableNode());
  if (!segmentationNode)
  {
    vtkErrorMacro("vtkMRMLSegmentationDisplayNode::GetSegmentIDs: No segmentation node is associated to this display node");
    return;
  }
  // Make sure the requested representation exists
  vtkSegmentation* segmentation = segmentationNode->GetSegmentation();
  if (!segmentation)
  {
    vtkErrorMacro("vtkMRMLSegmentationDisplayNode::GetSegmentIDs: No segmentation is associated to this display node");
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
  if (segmentIDs == nullptr)
  {
    vtkErrorMacro("vtkMRMLSegmentationDisplayNode::GetVisibleSegmentIDs failed: invalid segmentIDs");
    return;
  }
  std::vector<std::string> segmentIDsVector;
  this->GetSegmentIDs(segmentIDsVector, true);
  segmentIDs->Reset();
  for (std::vector<std::string>::iterator it = segmentIDsVector.begin(); it != segmentIDsVector.end(); ++it)
  {
    segmentIDs->InsertNextValue(*it);
  }
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationDisplayNode::UpdateSegmentList()
{
  this->UpdateSegmentList(this->RemoveUnusedDisplayProperties);
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationDisplayNode::UpdateSegmentList(bool removeUnusedDisplayProperties)
{
  vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(this->GetDisplayableNode());
  vtkSegmentation* segmentation = segmentationNode ? segmentationNode->GetSegmentation() : nullptr;
  if (segmentation == nullptr)
  {
    // Only clear display properties if a segmentation was set before (to enable setting
    // display properties before associating with a segmentation node)
    if (this->SegmentListUpdateSource != segmentation)
    {
      this->SegmentationDisplayProperties.clear();
      this->SegmentListUpdateTime = 0;
      this->SegmentListUpdateSource = nullptr;
    }
    return;
  }
  if (this->SegmentListUpdateSource == segmentation && this->SegmentListUpdateTime >= segmentation->GetMTime())
  {
    // Already up-to-date
    return;
  }
  this->SegmentListUpdateTime = segmentation->GetMTime();
  this->SegmentListUpdateSource = segmentation;

  // Disable modified event, as we just update internal cache
  bool wasDisableModified = this->GetDisableModifiedEvent();
  this->SetDisableModifiedEvent(true);

  // Reset number of generated colors if last segment was removed
  if (segmentation->GetNumberOfSegments() == 0)
  {
    this->NumberOfGeneratedColors = 0;
  }

  // Remove unused segment display properties and colors
  if (removeUnusedDisplayProperties)
  {
    // Get list of segment IDs that we have display properties for but does not exist in
    // the segmentation anymore.
    std::vector<std::string> orphanSegmentIds;
    for (SegmentDisplayPropertiesMap::iterator it = this->SegmentationDisplayProperties.begin(); it != this->SegmentationDisplayProperties.end(); ++it)
    {
      if (segmentation->GetSegment(it->first) == nullptr)
      {
        // The segment does not exist in segmentation
        orphanSegmentIds.push_back(it->first);
      }
    }
    // Delete unused properties and color table entries
    for (std::vector<std::string>::iterator orphanSegmentIdIt = orphanSegmentIds.begin(); orphanSegmentIdIt != orphanSegmentIds.end(); ++orphanSegmentIdIt)
    {
      // Remove segment display properties
      this->RemoveSegmentDisplayProperties(*orphanSegmentIdIt);
    }
  }

  // Add missing segment display properties
  // Get segment list of segments that do not have display properties.
  std::vector<std::string> segmentIDs;
  segmentation->GetSegmentIDs(segmentIDs);
  std::vector<std::string> missingSegmentIDs;
  for (std::vector<std::string>::const_iterator segmentIdIt = segmentIDs.begin(); segmentIdIt != segmentIDs.end(); ++segmentIdIt)
  {
    if (this->SegmentationDisplayProperties.find(*segmentIdIt) == this->SegmentationDisplayProperties.end())
    {
      // The segment does not exist in segmentation
      missingSegmentIDs.push_back(*segmentIdIt);
    }
  }
  // Add missing properties
  for (std::vector<std::string>::iterator missingSegmentIdIt = missingSegmentIDs.begin(); missingSegmentIdIt != missingSegmentIDs.end(); ++missingSegmentIdIt)
  {
    this->SetSegmentDisplayPropertiesToDefault(*missingSegmentIdIt);
  }

  this->SetDisableModifiedEvent(wasDisableModified);
}
