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

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// SegmentationCore includes
#include "vtkSegment.h"

#include "vtkSegmentationConverterFactory.h"
#include "vtkOrientedImageData.h"
#include "vtkOrientedImageDataResample.h"

// VTK includes
#include <vtkImageData.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkMath.h>
#include <vtkDataSet.h>

// STD includes
#include <algorithm>
#include <set>
#include <sstream>

//----------------------------------------------------------------------------
const double vtkSegment::SEGMENT_COLOR_VALUE_INVALID[4] = {0.5, 0.5, 0.5, 1.0};

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSegment);

//----------------------------------------------------------------------------
vtkSegment::vtkSegment()
{
  this->Name = NULL;
  this->DefaultColor[0] = SEGMENT_COLOR_VALUE_INVALID[0];
  this->DefaultColor[1] = SEGMENT_COLOR_VALUE_INVALID[1];
  this->DefaultColor[2] = SEGMENT_COLOR_VALUE_INVALID[2];
}

//----------------------------------------------------------------------------
vtkSegment::~vtkSegment()
{
  this->RemoveAllRepresentations();
  this->Representations.clear();
}

//----------------------------------------------------------------------------
void vtkSegment::PrintSelf(ostream& os, vtkIndent indent)
{
  // vtkObject's PrintSelf prints a long list of registered events, which
  // is too long and not useful, therefore we don't call vtkObject::PrintSelf
  // but print essential information on the vtkObject base.
  os << indent << "Debug: " << (this->Debug ? "On\n" : "Off\n");
  os << indent << "Modified Time: " << this->GetMTime() << "\n";

  os << indent << "Name: " << (this->Name ? this->Name : "NULL") << "\n";
  os << indent << "DefaultColor: (" << this->DefaultColor[0] << ", " << this->DefaultColor[1] << ", " << this->DefaultColor[2] << ")\n";

  RepresentationMap::iterator reprIt;
  os << indent << "Representations:\n";
  for (reprIt=this->Representations.begin(); reprIt!=this->Representations.end(); ++reprIt)
    {
    os << indent.GetNextIndent() << reprIt->first << " ";
    vtkDataObject* dataObject = reprIt->second;
    if (dataObject)
      {
      os << dataObject->GetClassName() << "\n";
      vtkImageData* imageData = vtkImageData::SafeDownCast(dataObject);
      vtkPolyData* polyData = vtkPolyData::SafeDownCast(dataObject);
      if (imageData)
        {
        vtkOrientedImageDataResample::PrintImageInformation(imageData, os, indent.GetNextIndent());
        }
      if (polyData)
        {
        os << indent.GetNextIndent().GetNextIndent() << "Number of points: " << polyData->GetNumberOfPoints() << "\n";
        os << indent.GetNextIndent().GetNextIndent() << "Number of cells: " << polyData->GetNumberOfCells() << "\n";
        }
      }
    else
      {
      os << "(none)\n";
      }
    }

  std::map<std::string,std::string>::iterator tagIt;
  os << indent << "Tags:\n";
  for (tagIt=this->Tags.begin(); tagIt!=this->Tags.end(); ++tagIt)
    {
    os << indent.GetNextIndent() << "  " << tagIt->first << ": " << tagIt->second << "\n";
    }
}

//----------------------------------------------------------------------------
void vtkSegment::ReadXMLAttributes(const char** vtkNotUsed(atts))
{
  // Note: Segment info is read by the storage node
}

//---------------------------------------------------------------------------
void vtkSegment::WriteXML(ostream& of, int nIndent)
{
  // Note: Segment info is written by the storage node, this function is not called
  vtkIndent indent(nIndent);

  of << indent << "Name=\"" << (this->Name ? this->Name : "NULL") << "\"";
  of << indent << "DefaultColor:\"(" << this->DefaultColor[0] << ", " << this->DefaultColor[1] << ", " << this->DefaultColor[2] << ")\"";

  RepresentationMap::iterator reprIt;
  of << indent << "Representations=\"";
  for (reprIt=this->Representations.begin(); reprIt!=this->Representations.end(); ++reprIt)
    {
    of << indent << "  " << reprIt->first << "\"";
    }

  std::map<std::string,std::string>::iterator tagIt;
  of << indent << "Tags=\"";
  for (tagIt=this->Tags.begin(); tagIt!=this->Tags.end(); ++tagIt)
    {
    of << tagIt->first << ":" << tagIt->second << "|";
    }
  of << "\"";
}

//----------------------------------------------------------------------------
void vtkSegment::DeepCopy(vtkSegment* source)
{
  if (!source)
    {
    vtkErrorMacro("vtkSegment::DeepCopy failed: sourceSegment is invalid")
    return;
    }

  this->DeepCopyMetadata(source);

  // Deep copy representations
  std::set<std::string> representationNamesToKeep;
  RepresentationMap::iterator reprIt;
  for (reprIt=source->Representations.begin(); reprIt!=source->Representations.end(); ++reprIt)
    {
    vtkDataObject* representationCopy =
      vtkSegmentationConverterFactory::GetInstance()->ConstructRepresentationObjectByClass( reprIt->second->GetClassName() );
    if (!representationCopy)
      {
      vtkErrorMacro("DeepCopy: Unable to construct representation type class '" << reprIt->second->GetClassName() << "'");
      continue;
      }
    representationCopy->DeepCopy(reprIt->second);
    this->AddRepresentation(reprIt->first, representationCopy);
    representationCopy->Delete(); // this representation is now owned by the segment
    representationNamesToKeep.insert(reprIt->first);
    }

  // Remove representations that are not in the source segment
  for (reprIt = this->Representations.begin(); reprIt != this->Representations.end();
    /*upon deletion the increment is done already, so don't increment here*/)
    {
    if (representationNamesToKeep.find(reprIt->first) == representationNamesToKeep.end())
      {
      // this representation should not be kept
      RepresentationMap::iterator reprItToRemove = reprIt;
      ++reprIt;
      this->Representations.erase(reprItToRemove);
      continue;
      }
    ++reprIt;
    }
}

//----------------------------------------------------------------------------
void vtkSegment::DeepCopyMetadata(vtkSegment* source)
{
  if (!source)
    {
    vtkErrorMacro("vtkSegment::DeepCopy failed: sourceSegment is invalid")
    return;
    }

  // Copy properties
  this->SetName(source->Name);
  this->SetDefaultColor(source->DefaultColor);
  this->Tags = source->Tags;
}


//---------------------------------------------------------------------------
// (Xmin, Xmax, Ymin, Ymax, Zmin, Zmax)
//---------------------------------------------------------------------------
void vtkSegment::GetBounds(double bounds[6])
{
  vtkOrientedImageData::UninitializeBounds(bounds);

  RepresentationMap::iterator reprIt;
  for (reprIt=this->Representations.begin(); reprIt!=this->Representations.end(); ++reprIt)
    {
    vtkDataSet* representationDataSet = vtkDataSet::SafeDownCast(reprIt->second);
    if (representationDataSet)
      {
      double representationBounds[6] = {0.0,0.0,0.0,0.0,0.0,0.0};
      vtkOrientedImageData::UninitializeBounds(representationBounds);
      representationDataSet->GetBounds(representationBounds);
      vtkSegment::ExtendBounds(representationBounds, bounds);
      }
    }
}

//---------------------------------------------------------------------------
vtkDataObject* vtkSegment::GetRepresentation(std::string name)
{
  // Use find function instead of operator[] not to create empty representation if it is missing
  RepresentationMap::iterator reprIt = this->Representations.find(name);
  if (reprIt != this->Representations.end())
    {
    return reprIt->second.GetPointer();
    }
  else
    {
    return NULL;
    }
}

//---------------------------------------------------------------------------
void vtkSegment::AddRepresentation(std::string name, vtkDataObject* representation)
{
  if (this->GetRepresentation(name) == representation)
    {
    return;
    }

  this->Representations[name] = representation; // Representations stores the pointer in a smart pointer, which makes sure the object is not deleted
  this->Modified();
}

//---------------------------------------------------------------------------
void vtkSegment::RemoveRepresentation(std::string name)
{
  vtkDataObject* representation = this->GetRepresentation(name);
  if (representation)
    {
    this->Representations.erase(name);
    this->Modified();
    }
}

//---------------------------------------------------------------------------
void vtkSegment::RemoveAllRepresentations(std::string exceptionRepresentationName/*=""*/)
{
  bool modified = false;
  RepresentationMap::iterator reprIt = this->Representations.begin();
  while (reprIt != this->Representations.end())
    {
    if (reprIt->first.compare(exceptionRepresentationName))
      {
      // reprIt++ is safe, as iterators remain valid after erasing from a map
      this->Representations.erase(reprIt++);
      modified = true;
      }
    else
      {
      ++reprIt;
      }
    }
  if (modified)
    {
    this->Modified();
    }
}

//---------------------------------------------------------------------------
void vtkSegment::GetContainedRepresentationNames(std::vector<std::string>& representationNames)
{
  representationNames.clear();

  RepresentationMap::iterator reprIt;
  for (reprIt=this->Representations.begin(); reprIt!=this->Representations.end(); ++reprIt)
    {
    representationNames.push_back(reprIt->first);
    }
}

//---------------------------------------------------------------------------
void vtkSegment::SetTag(std::string tag, std::string value, bool emitModified/*=false*/)
{
  this->Tags[tag] = value;
  if (emitModified)
    {
    this->Modified();
    }
}

//---------------------------------------------------------------------------
void vtkSegment::SetTag(std::string tag, int value, bool emitModified/*=false*/)
{
  std::stringstream ss;
  ss << value;
  this->SetTag(tag, ss.str(), emitModified);
}

//---------------------------------------------------------------------------
void vtkSegment::RemoveTag(std::string tag)
{
  this->Tags.erase(tag);
  this->Modified();
}

//---------------------------------------------------------------------------
bool vtkSegment::GetTag(std::string tag, std::string &value)
{
  std::map<std::string,std::string>::iterator tagIt = this->Tags.find(tag);
  if (tagIt == this->Tags.end())
    {
    return false;
    }

  value = tagIt->second;
  return true;
}

//---------------------------------------------------------------------------
bool vtkSegment::HasTag(std::string tag)
{
  std::string value;
  return this->GetTag(tag, value);
}

//---------------------------------------------------------------------------
void vtkSegment::GetTags(std::map<std::string,std::string> &tags)
{
  tags = this->Tags;
}

//---------------------------------------------------------------------------
// Global RAS in the form (Xmin, Xmax, Ymin, Ymax, Zmin, Zmax)
void vtkSegment::ExtendBounds(double partialBounds[6], double globalBounds[6])
{
  if (partialBounds[0] < globalBounds[0] )
    {
    globalBounds[0] = partialBounds[0];
    }
  if (partialBounds[2] < globalBounds[2] )
    {
    globalBounds[2] = partialBounds[2];
    }
  if (partialBounds[4] < globalBounds[4] )
    {
    globalBounds[4] = partialBounds[4];
    }

  if (partialBounds[1] > globalBounds[1] )
    {
    globalBounds[1] = partialBounds[1];
    }
  if (partialBounds[3] > globalBounds[3] )
    {
    globalBounds[3] = partialBounds[3];
    }
  if (partialBounds[5] > globalBounds[5] )
    {
    globalBounds[5] = partialBounds[5];
    }
}

//---------------------------------------------------------------------------
void vtkSegment::SetDefaultColorWithoutModifiedEvent(double color[3])
{
  this->DefaultColor[0] = color[0];
  this->DefaultColor[1] = color[1];
  this->DefaultColor[2] = color[2];
}
