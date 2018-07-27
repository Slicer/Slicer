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
#include "vtkSegmentation.h"

#include "vtkSegmentationConverterRule.h"
#include "vtkSegmentationConverterFactory.h"

#include "vtkOrientedImageData.h"
#include "vtkOrientedImageDataResample.h"
#include "vtkCalculateOversamplingFactor.h"

// VTK includes
#include <vtkBoundingBox.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkMath.h>
#include <vtkVersion.h>
#include <vtkCallbackCommand.h>
#include <vtkStringArray.h>
#include <vtkAbstractTransform.h>
#include <vtkMatrix4x4.h>
#include <vtkTransform.h>
#include <vtkPolyData.h>
#include <vtkTransformPolyDataFilter.h>

// STD includes
#include <sstream>
#include <algorithm>
#include <functional>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSegmentation);

//----------------------------------------------------------------------------
template<class T>
struct MapValueCompare : public std::binary_function<typename T::value_type, typename T::mapped_type, bool>
{
public:
  bool operator() (typename T::value_type &pair, typename T::mapped_type value) const
    {
    return pair.second == value;
    }
};

//----------------------------------------------------------------------------
vtkSegmentation::vtkSegmentation()
{
  this->Converter = vtkSegmentationConverter::New();

  this->SegmentCallbackCommand = vtkCallbackCommand::New();
  this->SegmentCallbackCommand->SetClientData( reinterpret_cast<void *>(this) );
  this->SegmentCallbackCommand->SetCallback( vtkSegmentation::OnSegmentModified );

  this->MasterRepresentationCallbackCommand = vtkCallbackCommand::New();
  this->MasterRepresentationCallbackCommand->SetClientData( reinterpret_cast<void *>(this) );
  this->MasterRepresentationCallbackCommand->SetCallback( vtkSegmentation::OnMasterRepresentationModified );

  this->MasterRepresentationModifiedEnabled = true;

  this->SegmentIdAutogeneratorIndex = 0;

  this->SetMasterRepresentationName(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName());
}

//----------------------------------------------------------------------------
vtkSegmentation::~vtkSegmentation()
{
  // Properly remove all segments
  this->RemoveAllSegments();

  this->Converter->Delete();

  if (this->SegmentCallbackCommand)
    {
    this->SegmentCallbackCommand->SetClientData(NULL);
    this->SegmentCallbackCommand->Delete();
    this->SegmentCallbackCommand = NULL;
    }

  if (this->MasterRepresentationCallbackCommand)
    {
    this->MasterRepresentationCallbackCommand->SetClientData(NULL);
    this->MasterRepresentationCallbackCommand->Delete();
    this->MasterRepresentationCallbackCommand = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkSegmentation::WriteXML(ostream& of, int vtkNotUsed(nIndent))
{
  of << " MasterRepresentationName=\"" << this->MasterRepresentationName << "\"";

  // Note: Segment info is not written as it is managed by the storage node instead.
}

//----------------------------------------------------------------------------
void vtkSegmentation::ReadXMLAttributes(const char** atts)
{
  const char* attName = NULL;
  const char* attValue = NULL;
  while (*atts != NULL)
    {
    attName = *(atts++);
    attValue = *(atts++);

    if (!strcmp(attName, "MasterRepresentationName"))
      {
      this->SetMasterRepresentationName(attValue);
      }
    }
}

//----------------------------------------------------------------------------
void vtkSegmentation::DeepCopy(vtkSegmentation* aSegmentation)
{
  if (!aSegmentation)
    {
    return;
    }

  this->RemoveAllSegments();

  // Copy properties
  this->SetMasterRepresentationName(aSegmentation->GetMasterRepresentationName());

  // Copy conversion parameters
  this->Converter->DeepCopy(aSegmentation->Converter);

  // Deep copy segments list
  for (std::deque< std::string >::iterator segmentIdIt = aSegmentation->SegmentIds.begin(); segmentIdIt != aSegmentation->SegmentIds.end(); ++segmentIdIt)
    {
    vtkSmartPointer<vtkSegment> segment = vtkSmartPointer<vtkSegment>::New();
    segment->DeepCopy(aSegmentation->Segments[*segmentIdIt]);
    this->AddSegment(segment);
    }
}

//----------------------------------------------------------------------------
void vtkSegmentation::CopyConversionParameters(vtkSegmentation* aSegmentation)
{
  this->Converter->DeepCopy(aSegmentation->Converter);
}

//----------------------------------------------------------------------------
void vtkSegmentation::PrintSelf(ostream& os, vtkIndent indent)
{
  // vtkObject's PrintSelf prints a long list of registered events, which
  // is too long and not useful, therefore we don't call vtkObject::PrintSelf
  // but print essential information on the vtkObject base.
  os << indent << "Debug: " << (this->Debug ? "On\n" : "Off\n");
  os << indent << "Modified Time: " << this->GetMTime() << "\n";

  os << indent << "MasterRepresentationName:  " << this->MasterRepresentationName << "\n";
  os << indent << "Number of segments:  " << this->Segments.size() << "\n";

  for (std::deque< std::string >::iterator segmentIdIt = this->SegmentIds.begin();
    segmentIdIt != this->SegmentIds.end(); ++segmentIdIt)
  {
    os << indent << "Segment: " << (*segmentIdIt) << "\n";
    vtkSegment* segment = this->Segments[*segmentIdIt];
    segment->PrintSelf(os, indent.GetNextIndent());
    }
  os << indent << "Segment converter:\n";
  this->Converter->PrintSelf(os, indent.GetNextIndent());
}

//---------------------------------------------------------------------------
// (Xmin, Xmax, Ymin, Ymax, Zmin, Zmax)
//---------------------------------------------------------------------------
void vtkSegmentation::GetBounds(double bounds[6])
{
  vtkMath::UninitializeBounds(bounds);

  if (this->Segments.empty())
    {
    return;
    }

  vtkBoundingBox boundingBox;
  for (SegmentMap::iterator it = this->Segments.begin(); it != this->Segments.end(); ++it)
    {
    double segmentBounds[6] = { 1, -1, 1, -1, 1, -1 };

    vtkSegment* segment = it->second;
    segment->GetBounds(segmentBounds);
    boundingBox.AddBounds(segmentBounds);
    }
  boundingBox.GetBounds(bounds);
}

//---------------------------------------------------------------------------
void vtkSegmentation::SetMasterRepresentationName(const std::string& representationName)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting MasterRepresentationName to " << representationName );
  if ( this->MasterRepresentationName == representationName )
    {
    // no change in representation name
    return;
    }

  // Remove observation of old master representation in all segments
  bool wasMasterRepresentationModifiedEnabled = this->SetMasterRepresentationModifiedEnabled(false);

  this->MasterRepresentationName = representationName;

  // Add observation of new master representation in all segments
  this->SetMasterRepresentationModifiedEnabled(wasMasterRepresentationModifiedEnabled);

  // Invalidate all representations other than the master.
  // These representations will be automatically converted later on demand.
  this->InvalidateNonMasterRepresentations();

  // Invoke events
  this->Modified();
  this->InvokeEvent(vtkSegmentation::MasterRepresentationModified, this);
}

//---------------------------------------------------------------------------
bool vtkSegmentation::SetMasterRepresentationModifiedEnabled(bool enabled)
{
  if (this->MasterRepresentationModifiedEnabled == enabled)
    {
    return this->MasterRepresentationModifiedEnabled;
    }
  // Add/remove observation of master representation in all segments
  for (SegmentMap::iterator segmentIt = this->Segments.begin(); segmentIt != this->Segments.end(); ++segmentIt)
    {
    vtkDataObject* masterRepresentation = segmentIt->second->GetRepresentation(this->MasterRepresentationName);
    if (masterRepresentation)
      {
      if (enabled)
        {
        if (!masterRepresentation->HasObserver(vtkCommand::ModifiedEvent, this->MasterRepresentationCallbackCommand))
          {
          masterRepresentation->AddObserver(vtkCommand::ModifiedEvent, this->MasterRepresentationCallbackCommand);
          }
        }
      else
        {
        masterRepresentation->RemoveObservers(vtkCommand::ModifiedEvent, this->MasterRepresentationCallbackCommand);
        }
      }
    }
  this->MasterRepresentationModifiedEnabled = enabled;
  return !enabled; // return old value
}

//---------------------------------------------------------------------------
std::string vtkSegmentation::GenerateUniqueSegmentID(std::string id)
{
  if (!id.empty() &&  this->Segments.find(id) == this->Segments.end())
    {
    // the provided id is already unique
    return id;
    }

  if (id.empty())
    {
    // use a non-empty default prefix if no id is provided
    id = "Segment";
    }

  // try to make it unique by attaching a postfix
  while (true)
    {
    this->SegmentIdAutogeneratorIndex++;
    if (this->SegmentIdAutogeneratorIndex < 0)
      {
      // wrapped around (almost impossible)
      this->SegmentIdAutogeneratorIndex = 0;
      break;
      }
    std::stringstream idStream;
    idStream << id << "_" << this->SegmentIdAutogeneratorIndex;
    if (this->Segments.find(idStream.str()) == this->Segments.end())
      {
      // found a unique ID
      return idStream.str();
      }
    }

  // try to make it unique by modifying prefix
  return this->GenerateUniqueSegmentID(id + "_");
}

//---------------------------------------------------------------------------
bool vtkSegmentation::AddSegment(vtkSegment* segment, std::string segmentId/*=""*/, std::string insertBeforeSegmentId/*=""*/)
{
  if (!segment)
    {
    vtkErrorMacro("AddSegment: Invalid segment!");
    return false;
    }

  // Observe segment underlying data for changes
  if (!segment->HasObserver(vtkCommand::ModifiedEvent, this->SegmentCallbackCommand))
    {
    segment->AddObserver(vtkCommand::ModifiedEvent, this->SegmentCallbackCommand);
    }

  // Get representation names contained by the added segment
  std::vector<std::string> containedRepresentationNamesInAddedSegment;
  segment->GetContainedRepresentationNames(containedRepresentationNamesInAddedSegment);

  if (containedRepresentationNamesInAddedSegment.empty())
    {
    // Add empty segment.
    // Create empty representations for all types that are present in this segmentation
    // (the representation configuration in all segments needs to match in a segmentation).
    std::vector<std::string> requiredRepresentationNames;
    if (this->Segments.empty())
      {
      // No segments, so the only representation that should be created is the master representation.
      requiredRepresentationNames.push_back(this->MasterRepresentationName);
      }
    else
      {
      vtkSegment* firstSegment = this->Segments.begin()->second;
      firstSegment->GetContainedRepresentationNames(requiredRepresentationNames);
      }

    for (std::vector<std::string>::iterator reprIt = requiredRepresentationNames.begin();
      reprIt != requiredRepresentationNames.end(); ++reprIt)
      {
      vtkSmartPointer<vtkDataObject> emptyRepresentation = vtkSmartPointer<vtkDataObject>::Take(
        vtkSegmentationConverterFactory::GetInstance()->ConstructRepresentationObjectByRepresentation(*reprIt));
      if (!emptyRepresentation)
        {
        vtkErrorMacro("AddSegment: Unable to construct empty representation type '" << (*reprIt) << "'");
        return false;
        }
      segment->AddRepresentation(*reprIt, emptyRepresentation);
      }
    }
  else
    {
    // Add non-empty segment.

    // Perform necessary conversions if needed on the added segment:
    // 1. If the segment can be added, and it does not contain the master representation,
    // then the master representation is converted using the cheapest available path.
    if (!segment->GetRepresentation(this->MasterRepresentationName))
      {
      // Collect all available paths to master representation
      vtkSegmentationConverter::ConversionPathAndCostListType allPathsToMaster;
      for (std::vector<std::string>::iterator reprIt = containedRepresentationNamesInAddedSegment.begin();
        reprIt != containedRepresentationNamesInAddedSegment.end(); ++reprIt)
        {
        vtkSegmentationConverter::ConversionPathAndCostListType pathsFromCurrentRepresentationToMaster;
        this->Converter->GetPossibleConversions((*reprIt), this->MasterRepresentationName, pathsFromCurrentRepresentationToMaster);
        // Append paths from current representation to master to all found paths to master
        allPathsToMaster.insert(allPathsToMaster.end(),
          pathsFromCurrentRepresentationToMaster.begin(), pathsFromCurrentRepresentationToMaster.end());
        }
      // Get cheapest path from any representation to master and try to convert
      vtkSegmentationConverter::ConversionPathType cheapestPath =
        vtkSegmentationConverter::GetCheapestPath(allPathsToMaster);
      if (cheapestPath.empty() || !this->ConvertSegmentUsingPath(segment, cheapestPath))
        {
        // Return if cannot convert to master representation
        vtkErrorMacro("AddSegment: Unable to create master representation!");
        return false;
        }
      }

    /// 2. Make sure that the segment contains the same types of representations that are
    /// present in the existing segments of the segmentation (because we expect all segments
    /// in a segmentation to contain the same types of representations).
    if (this->GetNumberOfSegments() > 0)
      {
      vtkSegment* firstSegment = this->Segments.begin()->second;
      std::vector<std::string> requiredRepresentationNames;
      firstSegment->GetContainedRepresentationNames(requiredRepresentationNames);

      // Convert to representations that exist in this segmentation
      for (std::vector<std::string>::iterator reprIt = requiredRepresentationNames.begin();
        reprIt != requiredRepresentationNames.end(); ++reprIt)
        {
        // If representation exists then there is nothing to do
        if (segment->GetRepresentation(*reprIt))
          {
          continue;
          }

        // Convert using the cheapest available path
        vtkSegmentationConverter::ConversionPathAndCostListType pathsToCurrentRepresentation;
        this->Converter->GetPossibleConversions(this->MasterRepresentationName, (*reprIt), pathsToCurrentRepresentation);
        vtkSegmentationConverter::ConversionPathType cheapestPath =
          vtkSegmentationConverter::GetCheapestPath(pathsToCurrentRepresentation);
        if (cheapestPath.empty())
          {
          vtkErrorMacro("AddSegment: Unable to perform conversion"); // Sanity check, it should never happen
          return false;
          }
        // Perform conversion
        this->ConvertSegmentUsingPath(segment, cheapestPath);
        }

      // Remove representations that do not exist in this segmentation
      for (std::vector<std::string>::iterator reprIt = containedRepresentationNamesInAddedSegment.begin();
        reprIt != containedRepresentationNamesInAddedSegment.end(); ++reprIt)
        {
        if (!firstSegment->GetRepresentation(*reprIt))
          {
          segment->RemoveRepresentation(*reprIt);
          }
        }
      }
    }

  // Add to list. If segmentId is empty, then segment name becomes the ID
  std::string key = segmentId;
  if (key.empty())
    {
    if (segment->GetName() == NULL)
      {
      vtkErrorMacro("AddSegment: Unable to add segment without a key; neither key is given nor segment name is defined!");
      return false;
      }
    key = segment->GetName();
    key = this->GenerateUniqueSegmentID(key);
    }
  this->Segments[key] = segment;
  if (insertBeforeSegmentId.empty())
    {
    this->SegmentIds.push_back(key);
    }
  else
    {
    std::deque< std::string >::iterator insertionPosition = std::find(this->SegmentIds.begin(), this->SegmentIds.end(), insertBeforeSegmentId);
    this->SegmentIds.insert(insertionPosition, key);
    }

  // Add observation of master representation in new segment
  vtkDataObject* masterRepresentation = segment->GetRepresentation(this->MasterRepresentationName);
  if (masterRepresentation && this->MasterRepresentationModifiedEnabled)
    {
    // Observe segment's master representation
    if (!masterRepresentation->HasObserver(vtkCommand::ModifiedEvent, this->MasterRepresentationCallbackCommand))
      {
      masterRepresentation->AddObserver(vtkCommand::ModifiedEvent, this->MasterRepresentationCallbackCommand);
      }
    }

  this->Modified();

  // Fire segment added event
  const char* segmentIdChars = key.c_str();
  this->InvokeEvent(vtkSegmentation::SegmentAdded, (void*)segmentIdChars);

  return true;
}

//---------------------------------------------------------------------------
void vtkSegmentation::RemoveSegment(std::string segmentId)
{
  SegmentMap::iterator segmentIt = this->Segments.find(segmentId);
  if (segmentIt == this->Segments.end())
    {
    vtkWarningMacro("RemoveSegment: Segment to remove cannot be found!");
    return;
    }

  // Remove segment
  this->RemoveSegment(segmentIt);
}

//---------------------------------------------------------------------------
void vtkSegmentation::RemoveSegment(vtkSegment* segment)
{
  if (!segment)
    {
    vtkErrorMacro("RemoveSegment: Invalid segment!");
    return;
    }

  SegmentMap::iterator segmentIt = std::find_if(
    this->Segments.begin(), this->Segments.end(), std::bind2nd(MapValueCompare<SegmentMap>(), segment) );
  if (segmentIt == this->Segments.end())
    {
    vtkWarningMacro("RemoveSegment: Segment to remove cannot be found!");
    return;
    }

  // Remove segment
  this->RemoveSegment(segmentIt);
}

//---------------------------------------------------------------------------
void vtkSegmentation::RemoveSegment(SegmentMap::iterator segmentIt)
{
  if (segmentIt == this->Segments.end())
    {
    return;
    }

  std::string segmentId(segmentIt->first);

  // Remove observation of segment modified event
  segmentIt->second.GetPointer()->RemoveObservers(vtkCommand::ModifiedEvent, this->SegmentCallbackCommand);
  // Remove observation of master representation of removed segment
  vtkDataObject* masterRepresentation = segmentIt->second->GetRepresentation(this->MasterRepresentationName);
  if (masterRepresentation)
    {
    masterRepresentation->RemoveObservers(vtkCommand::ModifiedEvent, this->MasterRepresentationCallbackCommand);
    }

  // Remove segment
  this->SegmentIds.erase(std::remove(this->SegmentIds.begin(), this->SegmentIds.end(), segmentId), this->SegmentIds.end());
  this->Segments.erase(segmentIt);
  if (this->Segments.empty())
    {
    this->SegmentIdAutogeneratorIndex = 0;
    }

  this->Modified();

  // Fire segment removed event
  this->InvokeEvent(vtkSegmentation::SegmentRemoved, (void*)segmentId.c_str());
}

//---------------------------------------------------------------------------
void vtkSegmentation::RemoveAllSegments()
{
  this->SegmentIds.clear();

  std::vector<std::string> segmentIds;
  this->GetSegmentIDs(segmentIds);
  for (std::vector<std::string>::iterator segmentIt = segmentIds.begin(); segmentIt != segmentIds.end(); ++segmentIt)
    {
    this->RemoveSegment(*segmentIt);
    }
  this->Segments.clear();

  this->SegmentIdAutogeneratorIndex = 0;
}

//---------------------------------------------------------------------------
void vtkSegmentation::OnSegmentModified(vtkObject* caller,
                                        unsigned long vtkNotUsed(eid),
                                        void* clientData,
                                        void* vtkNotUsed(callData))
{
  vtkSegmentation* self = reinterpret_cast<vtkSegmentation*>(clientData);
  vtkSegment* callerSegment = reinterpret_cast<vtkSegment*>(caller);
  if (!self || !callerSegment)
    {
    return;
    }

  // Invoke segment modified event, but do not invoke general modified event
  std::string segmentId = self->GetSegmentIdBySegment(callerSegment);
  if (segmentId.empty())
    {
    // Segment is modified before actually having been added to the segmentation (within AddSegment)
    return;
    }
  const char* segmentIdChars = segmentId.c_str();
  self->InvokeEvent(vtkSegmentation::SegmentModified, (void*)(segmentIdChars));
}

//---------------------------------------------------------------------------
void vtkSegmentation::OnMasterRepresentationModified(vtkObject* vtkNotUsed(caller),
                                                     unsigned long vtkNotUsed(eid),
                                                     void* clientData,
                                                     void* callData)
{
  vtkSegmentation* self = reinterpret_cast<vtkSegmentation*>(clientData);
  if (!self)
    {
    return;
    }

  // Invalidate all representations other than the master.
  // These representations will be automatically converted later on demand.
  self->InvalidateNonMasterRepresentations();

  self->InvokeEvent(vtkSegmentation::MasterRepresentationModified, callData);
}

//---------------------------------------------------------------------------
vtkSegment* vtkSegmentation::GetSegment(std::string segmentId)
{
  SegmentMap::iterator segmentIt = this->Segments.find(segmentId);
  if (segmentIt == this->Segments.end())
    {
    return NULL;
    }

  return segmentIt->second;
}

//---------------------------------------------------------------------------
int vtkSegmentation::GetNumberOfSegments() const
{
  return (int)this->SegmentIds.size();
}

//---------------------------------------------------------------------------
vtkSegment* vtkSegmentation::GetNthSegment(unsigned int index) const
{
  if (index >= this->SegmentIds.size())
    {
    return NULL;
    }
  std::string segmentId = this->SegmentIds[index];
  SegmentMap::const_iterator segmentIt = this->Segments.find(segmentId);
  if (segmentIt == this->Segments.end())
    {
    // inconsistent segment ID and segment list
    return NULL;
    }
  return segmentIt->second;
}

//---------------------------------------------------------------------------
std::string vtkSegmentation::GetNthSegmentID(unsigned int index) const
{
  if (index >= this->SegmentIds.size())
    {
    return "";
    }
  return this->SegmentIds[index];
}

//---------------------------------------------------------------------------
int vtkSegmentation::GetSegmentIndex(const std::string& segmentId)
{
  std::deque< std::string >::iterator foundIt = std::find(this->SegmentIds.begin(), this->SegmentIds.end(), segmentId);
  if (foundIt == this->SegmentIds.end())
    {
    return -1;
    }
  return foundIt - this->SegmentIds.begin();
}

//---------------------------------------------------------------------------
bool vtkSegmentation::SetSegmentIndex(const std::string& segmentId, unsigned int newIndex)
{
  if (newIndex >= this->SegmentIds.size())
    {
    vtkErrorMacro("vtkSegmentation::SetSegmentIndex failed: index " << newIndex
      << " is out of range [0," << this->SegmentIds.size()-1 << "]");
    return false;
    }
  std::deque< std::string >::iterator foundIt = std::find(this->SegmentIds.begin(), this->SegmentIds.end(), segmentId);
  if (foundIt == this->SegmentIds.end())
    {
    vtkErrorMacro("vtkSegmentation::SetSegmentIndex failed: segment " << segmentId << " not found");
    return false;
    }
  std::swap(*foundIt, this->SegmentIds[newIndex]);
  this->Modified();
  this->InvokeEvent(vtkSegmentation::SegmentsOrderModified);
  return true;
}

//---------------------------------------------------------------------------
void vtkSegmentation::ReorderSegments(std::vector<std::string> segmentIdsToMove, std::string insertBeforeSegmentId /* ="" */)
{
  if (segmentIdsToMove.empty())
    {
    return;
    }

  // Remove all segmentIdsToMove from the segment ID list
  for (std::deque< std::string >::iterator segmentIdIt = this->SegmentIds.begin(); segmentIdIt != this->SegmentIds.end();
    /*upon deletion the increment is done already, so don't increment here*/)
    {
      std::string t = *segmentIdIt;
      std::vector<std::string>::iterator foundSegmentIdToMove = std::find(segmentIdsToMove.begin(), segmentIdsToMove.end(), t);
    if (foundSegmentIdToMove != segmentIdsToMove.end())
      {
      // this segment gets a new position, so remove it from current position
      // ### Slicer 4.4: Simplify this logic when adding support for C++11 accross all supported platform/compilers
      std::deque< std::string >::iterator segmentIdItToRemove = segmentIdIt;
      ++segmentIdIt;
      this->SegmentIds.erase(segmentIdItToRemove);
      if (this->SegmentIds.empty())
        {
        // iterators are invalidated if the last element is deleted
        break;
        }
      }
    else
      {
      ++segmentIdIt;
      }
    }

  // Find insert position
  std::deque< std::string >::iterator insertPosition = this->SegmentIds.end();
  if (!insertBeforeSegmentId.empty())
    {
    insertPosition = std::find(this->SegmentIds.begin(), this->SegmentIds.end(), insertBeforeSegmentId);
    }
  bool pushBack = (insertPosition == this->SegmentIds.end());

  // Add segments at the insert position
  for (std::vector<std::string>::const_iterator segmentIdsToMoveIt = segmentIdsToMove.begin();
    segmentIdsToMoveIt != segmentIdsToMove.end(); ++segmentIdsToMoveIt)
    {
    if (this->Segments.find(*segmentIdsToMoveIt) == this->Segments.end())
      {
      // segment not found, ignore it
      continue;
      }
    if (pushBack)
      {
      this->SegmentIds.push_back(*segmentIdsToMoveIt);
      }
    else
      {
      this->SegmentIds.insert(insertPosition, *segmentIdsToMoveIt);
      }
    }
  this->Modified();
  this->InvokeEvent(vtkSegmentation::SegmentsOrderModified);
}

//---------------------------------------------------------------------------
std::string vtkSegmentation::GetSegmentIdBySegment(vtkSegment* segment)
{
  if (!segment)
    {
    vtkErrorMacro("GetSegmentIdBySegment: Invalid segment!");
    return "";
    }

  SegmentMap::iterator segmentIt = std::find_if(
    this->Segments.begin(), this->Segments.end(), std::bind2nd(MapValueCompare<SegmentMap>(), segment) );
  if (segmentIt == this->Segments.end())
    {
    vtkDebugMacro("GetSegmentIdBySegment: Segment cannot be found!");
    return "";
    }

  return segmentIt->first;
}

//---------------------------------------------------------------------------
std::string vtkSegmentation::GetSegmentIdBySegmentName(std::string name)
{
  // Make given name lowercase for case-insensitive comparison
  std::transform(name.begin(), name.end(), name.begin(), ::tolower);

  for (SegmentMap::iterator segmentIt = this->Segments.begin(); segmentIt != this->Segments.end(); ++segmentIt)
    {
    std::string currentSegmentName(segmentIt->second->GetName() ? segmentIt->second->GetName() : "");
    std::transform(currentSegmentName.begin(), currentSegmentName.end(), currentSegmentName.begin(), ::tolower);
    if (!currentSegmentName.compare(name))
      {
      return segmentIt->first;
      }
    }

  return "";
}

//---------------------------------------------------------------------------
std::vector<vtkSegment*> vtkSegmentation::GetSegmentsByTag(std::string tag, std::string value/*=""*/)
{
  std::vector<vtkSegment*> foundSegments;
  for (SegmentMap::iterator segmentIt = this->Segments.begin(); segmentIt != this->Segments.end(); ++segmentIt)
    {
    std::string tagValue;
    bool tagFound = segmentIt->second->GetTag(tag, tagValue);
    if (!tagFound)
      {
      continue;
      }

    // Add current segment to found segments if there is no requested value, or if the requested value
    // matches the tag's value in the segment
    if (value.empty() || !tagValue.compare(value))
      {
      foundSegments.push_back(segmentIt->second);
      }
    }

  return foundSegments;
}


//---------------------------------------------------------------------------
void vtkSegmentation::GetSegmentIDs(std::vector<std::string> &segmentIds)
{
  segmentIds.clear();
  for (std::deque< std::string >::iterator segmentIdIt = this->SegmentIds.begin(); segmentIdIt != this->SegmentIds.end(); ++segmentIdIt)
    {
    segmentIds.push_back(*segmentIdIt);
    }
}

//---------------------------------------------------------------------------
void vtkSegmentation::GetSegmentIDs(vtkStringArray* segmentIds)
{
  if (!segmentIds)
    {
    return;
    }
  segmentIds->Initialize();
  for (std::deque< std::string >::iterator segmentIdIt = this->SegmentIds.begin(); segmentIdIt != this->SegmentIds.end(); ++segmentIdIt)
    {
    segmentIds->InsertNextValue(segmentIdIt->c_str());
    }
}

//---------------------------------------------------------------------------
void vtkSegmentation::ApplyLinearTransform(vtkAbstractTransform* transform)
{
  // Check if input transform is indeed linear
  vtkSmartPointer<vtkTransform> linearTransform = vtkSmartPointer<vtkTransform>::New();
  if (!vtkOrientedImageDataResample::IsTransformLinear(transform, linearTransform))
    {
    vtkErrorMacro("ApplyLinearTransform: Given transform is not a linear transform!");
    return;
    }

  // Apply transform on reference image geometry conversion parameter (to preserve validity of merged labelmap)
  this->Converter->ApplyTransformOnReferenceImageGeometry(transform);

  // Apply linear transform for each segment:
  // Harden transform on master representation if poly data, apply directions if oriented image data
  for (SegmentMap::iterator it = this->Segments.begin(); it != this->Segments.end(); ++it)
    {
    vtkDataObject* currentMasterRepresentation = it->second->GetRepresentation(this->MasterRepresentationName);
    if (!currentMasterRepresentation)
      {
      vtkErrorMacro("ApplyLinearTransform: Cannot get master representation (" << this->MasterRepresentationName << ") from segment!");
      return;
      }

    vtkPolyData* currentMasterRepresentationPolyData = vtkPolyData::SafeDownCast(currentMasterRepresentation);
    vtkOrientedImageData* currentMasterRepresentationOrientedImageData = vtkOrientedImageData::SafeDownCast(currentMasterRepresentation);
    // Poly data
    if (currentMasterRepresentationPolyData)
      {
      vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
      transformFilter->SetInputData(currentMasterRepresentationPolyData);
      transformFilter->SetTransform(linearTransform);
      transformFilter->Update();
      currentMasterRepresentationPolyData->DeepCopy(transformFilter->GetOutput());
      }
    // Oriented image data
    else if (currentMasterRepresentationOrientedImageData)
      {
      vtkOrientedImageDataResample::TransformOrientedImage(currentMasterRepresentationOrientedImageData, linearTransform);
      }
    else
      {
      vtkErrorMacro("ApplyLinearTransform: Representation data type '" << currentMasterRepresentation->GetClassName() << "' not supported!");
      }
    }
}

//---------------------------------------------------------------------------
void vtkSegmentation::ApplyNonLinearTransform(vtkAbstractTransform* transform)
{
  // Check if input transform is indeed non-linear. Report warning if linear, as this function should
  // only be called with non-linear transforms.
  vtkSmartPointer<vtkTransform> linearTransform = vtkSmartPointer<vtkTransform>::New();
  if (vtkOrientedImageDataResample::IsTransformLinear(transform, linearTransform))
    {
    vtkWarningMacro("ApplyNonLinearTransform: Linear input transform is detected in function that should only handle non-linear transforms!");
    }

  // Apply transform on reference image geometry conversion parameter (to preserve validity of merged labelmap)
  this->Converter->ApplyTransformOnReferenceImageGeometry(transform);

  // Harden transform on master representation (both image data and poly data) for each segment individually
  for (SegmentMap::iterator it = this->Segments.begin(); it != this->Segments.end(); ++it)
    {
    vtkDataObject* currentMasterRepresentation = it->second->GetRepresentation(this->MasterRepresentationName);
    if (!currentMasterRepresentation)
      {
      vtkErrorMacro("ApplyNonLinearTransform: Cannot get master representation (" << this->MasterRepresentationName << ") from segment!");
      return;
      }

    vtkPolyData* currentMasterRepresentationPolyData = vtkPolyData::SafeDownCast(currentMasterRepresentation);
    vtkOrientedImageData* currentMasterRepresentationOrientedImageData = vtkOrientedImageData::SafeDownCast(currentMasterRepresentation);
    // Poly data
    if (currentMasterRepresentationPolyData)
      {
      vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
      transformFilter->SetInputData(currentMasterRepresentationPolyData);
      transformFilter->SetTransform(transform);
      transformFilter->Update();
      currentMasterRepresentationPolyData->DeepCopy(transformFilter->GetOutput());
      }
    // Oriented image data
    else if (currentMasterRepresentationOrientedImageData)
      {
      vtkOrientedImageDataResample::TransformOrientedImage(currentMasterRepresentationOrientedImageData, transform);
      }
    else
      {
      vtkErrorMacro("ApplyLinearTransform: Representation data type '" << currentMasterRepresentation->GetClassName() << "' not supported!");
      }
    }
}

//-----------------------------------------------------------------------------
bool vtkSegmentation::ConvertSegmentUsingPath(vtkSegment* segment, vtkSegmentationConverter::ConversionPathType path, bool overwriteExisting/*=false*/)
{
  // Execute each conversion step in the selected path
  vtkSegmentationConverter::ConversionPathType::iterator pathIt;
  for (pathIt = path.begin(); pathIt != path.end(); ++pathIt)
    {
    vtkSegmentationConverterRule* currentConversionRule = (*pathIt);
    if (!currentConversionRule)
      {
      vtkErrorMacro("ConvertSegmentUsingPath: Invalid converter rule!");
      return false;
      }

    // Get source representation from segment. It is expected to exist
    vtkDataObject* sourceRepresentation = segment->GetRepresentation(
      currentConversionRule->GetSourceRepresentationName() );
    if (!sourceRepresentation)
      {
      vtkErrorMacro("ConvertSegmentUsingPath: Source representation does not exist!");
      return false;
      }

    // Get target representation
    vtkSmartPointer<vtkDataObject> targetRepresentation = segment->GetRepresentation(
      currentConversionRule->GetTargetRepresentationName() );
    // If target representation exists and we do not overwrite existing representations,
    // then no conversion is necessary with this conversion rule
    if (targetRepresentation.GetPointer() && !overwriteExisting)
      {
      continue;
      }
    // Create an empty target representation if it does not exist
    if (!targetRepresentation.GetPointer())
      {
      targetRepresentation = vtkSmartPointer<vtkDataObject>::Take(
        currentConversionRule->ConstructRepresentationObjectByRepresentation(currentConversionRule->GetTargetRepresentationName()) );
      }

    // Perform conversion step
    currentConversionRule->Convert(sourceRepresentation, targetRepresentation);

    // Add representation to segment
    segment->AddRepresentation(currentConversionRule->GetTargetRepresentationName(), targetRepresentation);
    }

  return true;
}

//---------------------------------------------------------------------------
bool vtkSegmentation::CreateRepresentation(const std::string& targetRepresentationName, bool alwaysConvert/*=false*/)
{
  if (!this->Converter)
    {
    vtkErrorMacro("CreateRepresentation: Invalid converter!");
    return false;
    }

  // Simply return success if the target representation exists
  if (!alwaysConvert)
    {
    bool representationExists = true;
    for (SegmentMap::iterator segmentIt = this->Segments.begin(); segmentIt != this->Segments.end(); ++segmentIt)
      {
      if (!segmentIt->second->GetRepresentation(targetRepresentationName))
        {
        // All segments should have the same representation configuration,
        // so checking each segment is mostly a safety measure
        representationExists = false;
        break;
        }
      }
    if (representationExists)
      {
      return true;
      }
    }

  // Get conversion path with lowest cost.
  // If always convert, then only consider conversions from master, otherwise consider all available representations
  vtkSegmentationConverter::ConversionPathAndCostListType pathCosts;
  if (alwaysConvert)
    {
    this->Converter->GetPossibleConversions(this->MasterRepresentationName, targetRepresentationName, pathCosts);
    }
  else
    {
    vtkSegmentationConverter::ConversionPathAndCostListType currentPathCosts;
    std::vector<std::string> representationNames;
    this->GetContainedRepresentationNames(representationNames);
    for (std::vector<std::string>::iterator reprIt=representationNames.begin(); reprIt!=representationNames.end(); ++reprIt)
      {
      if (!reprIt->compare(targetRepresentationName))
        {
        continue; // No paths if source and target representations are the same
        }
      this->Converter->GetPossibleConversions((*reprIt), targetRepresentationName, currentPathCosts);
      for (vtkSegmentationConverter::ConversionPathAndCostListType::const_iterator pathIt = currentPathCosts.begin(); pathIt != currentPathCosts.end(); ++pathIt)
        {
        pathCosts.push_back(*pathIt);
        }
      }
    }
  // Get cheapest path from found conversion paths
  vtkSegmentationConverter::ConversionPathType cheapestPath = vtkSegmentationConverter::GetCheapestPath(pathCosts);
  if (cheapestPath.empty())
    {
    return false;
    }

  // Perform conversion on all segments (no overwrites)
  for (SegmentMap::iterator segmentIt = this->Segments.begin(); segmentIt != this->Segments.end(); ++segmentIt)
    {
    vtkDataObject* representationBefore = segmentIt->second->GetRepresentation(targetRepresentationName);
    if (!this->ConvertSegmentUsingPath(segmentIt->second, cheapestPath, alwaysConvert))
      {
      vtkErrorMacro("CreateRepresentation: Conversion failed");
      return false;
      }
    vtkDataObject* representationAfter = segmentIt->second->GetRepresentation(targetRepresentationName);
    if (representationBefore != representationAfter
      || (representationBefore != NULL && representationAfter != NULL && representationBefore->GetMTime() != representationAfter->GetMTime()) )
      {
      // representation has been modified
      const char* segmentId = segmentIt->first.c_str();
      this->InvokeEvent(vtkSegmentation::RepresentationModified, (void*)segmentId);
      }
    }

  this->InvokeEvent(vtkSegmentation::ContainedRepresentationNamesModified);
  return true;
}

//---------------------------------------------------------------------------
bool vtkSegmentation::CreateRepresentation(vtkSegmentationConverter::ConversionPathType path,
                                           vtkSegmentationConverterRule::ConversionParameterListType parameters)
{
  if (!this->Converter)
    {
    vtkErrorMacro("CreateRepresentation: Invalid converter!");
    return false;
    }
  if (path.empty())
    {
    return false;
    }

  // Set conversion parameters
  this->Converter->SetConversionParameters(parameters);

  // Perform conversion on all segments (do overwrites)
  for (SegmentMap::iterator segmentIt = this->Segments.begin(); segmentIt != this->Segments.end(); ++segmentIt)
    {
    if (!this->ConvertSegmentUsingPath(segmentIt->second, path, true))
      {
      vtkErrorMacro("CreateRepresentation: Conversion failed");
      return false;
      }
    const char* segmentId = segmentIt->first.c_str();
    this->InvokeEvent(vtkSegmentation::RepresentationModified, (void*)segmentId);
    }

  this->InvokeEvent(vtkSegmentation::ContainedRepresentationNamesModified);
  return true;
}

//---------------------------------------------------------------------------
void vtkSegmentation::RemoveRepresentation(const std::string& representationName)
{
  for (SegmentMap::iterator segmentIt = this->Segments.begin(); segmentIt != this->Segments.end(); ++segmentIt)
    {
    segmentIt->second->RemoveRepresentation(representationName);
    }

  this->InvokeEvent(vtkSegmentation::ContainedRepresentationNamesModified);
}

//---------------------------------------------------------------------------
vtkDataObject* vtkSegmentation::GetSegmentRepresentation(std::string segmentId, std::string representationName)
{
  vtkSegment* segment = this->GetSegment(segmentId);
  if (!segment)
    {
    return NULL;
    }
  return segment->GetRepresentation(representationName);
}

//---------------------------------------------------------------------------
void vtkSegmentation::InvalidateNonMasterRepresentations()
{
  // Iterate through all segments and remove all representations that are not the master representation
  for (SegmentMap::iterator segmentIt = this->Segments.begin(); segmentIt != this->Segments.end(); ++segmentIt)
    {
    segmentIt->second->RemoveAllRepresentations(this->MasterRepresentationName);
    }
}

//---------------------------------------------------------------------------
void vtkSegmentation::GetContainedRepresentationNames(std::vector<std::string>& representationNames)
{
  if (this->Segments.empty())
    {
    return;
    }

  vtkSegment* firstSegment = this->Segments.begin()->second;
  firstSegment->GetContainedRepresentationNames(representationNames);
}

//---------------------------------------------------------------------------
bool vtkSegmentation::ContainsRepresentation(std::string representationName)
{
  if (this->Segments.empty())
    {
    return false;
    }

  std::vector<std::string> containedRepresentationNames;
  this->GetContainedRepresentationNames(containedRepresentationNames);
  std::vector<std::string>::iterator reprIt = std::find(
    containedRepresentationNames.begin(), containedRepresentationNames.end(), representationName);

  return (reprIt != containedRepresentationNames.end());
}

//-----------------------------------------------------------------------------
bool vtkSegmentation::IsMasterRepresentationPolyData()
{
  if (!this->Segments.empty())
    {
    // Assume the first segment contains the same name of representations as all segments (this should be the case by design)
    vtkSegment* firstSegment = this->Segments.begin()->second;
    vtkDataObject* masterRepresentation = firstSegment->GetRepresentation(this->MasterRepresentationName);
    return vtkPolyData::SafeDownCast(masterRepresentation) != NULL;
    }
  else
    {
    // There are no segments, create an empty representation to find out what type it is
    vtkSmartPointer<vtkDataObject> masterRepresentation = vtkSmartPointer<vtkDataObject>::Take(
      vtkSegmentationConverterFactory::GetInstance()->ConstructRepresentationObjectByRepresentation(this->MasterRepresentationName));
    return vtkPolyData::SafeDownCast(masterRepresentation) != NULL;
    }
}

//-----------------------------------------------------------------------------
bool vtkSegmentation::IsMasterRepresentationImageData()
{
  if (!this->Segments.empty())
    {
    // Assume the first segment contains the same name of representations as all segments (this should be the case by design)
    vtkSegment* firstSegment = this->Segments.begin()->second;
    vtkDataObject* masterRepresentation = firstSegment->GetRepresentation(this->MasterRepresentationName);
    return vtkOrientedImageData::SafeDownCast(masterRepresentation) != NULL;
    }
  else
    {
    // There are no segments, create an empty representation to find out what type it is
    vtkSmartPointer<vtkDataObject> masterRepresentation = vtkSmartPointer<vtkDataObject>::Take(
      vtkSegmentationConverterFactory::GetInstance()->ConstructRepresentationObjectByRepresentation(this->MasterRepresentationName));
    return vtkOrientedImageData::SafeDownCast(masterRepresentation) != NULL;
    }
}

//-----------------------------------------------------------------------------
bool vtkSegmentation::CanAcceptRepresentation(std::string representationName)
{
  if (representationName.empty())
    {
    return false;
    }

  // If representation is the master representation then it can be accepted
  if (!representationName.compare(this->MasterRepresentationName))
    {
    return true;
    }

  // Otherwise if the representation can be converted to the master representation, then
  // it can be accepted, if cannot be converted then not.
  vtkSegmentationConverter::ConversionPathAndCostListType pathCosts;
  this->Converter->GetPossibleConversions(representationName, this->MasterRepresentationName, pathCosts);
  return !pathCosts.empty();
}

//-----------------------------------------------------------------------------
bool vtkSegmentation::CanAcceptSegment(vtkSegment* segment)
{
  if (!segment)
    {
    return false;
    }

  // Can accept any segment if there segmentation is empty
  if (this->Segments.size() == 0)
    {
    return true;
    }

  // Check if segmentation can accept any of the segment's representations
  std::vector<std::string> containedRepresentationNames;
  segment->GetContainedRepresentationNames(containedRepresentationNames);
  for (std::vector<std::string>::iterator reprIt = containedRepresentationNames.begin();
    reprIt != containedRepresentationNames.end(); ++reprIt)
    {
    if (this->CanAcceptRepresentation(*reprIt))
      {
      return true;
      }
    }

  // If no representation in the segment is acceptable by this segmentation then the
  // segment is unacceptable.
  return false;
}

//-----------------------------------------------------------------------------
std::string vtkSegmentation::AddEmptySegment(std::string segmentId/*=""*/, std::string segmentName/*=""*/, double* color/*=NULL*/)
{
  vtkSmartPointer<vtkSegment> segment = vtkSmartPointer<vtkSegment>::New();
  if (color)
    {
    segment->SetColor(color);
    }
  else
    {
    segment->SetColor(vtkSegment::SEGMENT_COLOR_INVALID[0], vtkSegment::SEGMENT_COLOR_INVALID[1], vtkSegment::SEGMENT_COLOR_INVALID[2]);
    }

  // Segment ID will be segment name by default
  segmentId = this->GenerateUniqueSegmentID(segmentId);
  if (!segmentName.empty())
    {
    segment->SetName(segmentName.c_str());
    }
  else
    {
    segment->SetName(segmentId.c_str());
    }

  // Add segment
  if (!this->AddSegment(segment, segmentId))
    {
    return "";
    }
  return segmentId;
}

//-----------------------------------------------------------------------------
void vtkSegmentation::GetPossibleConversions(const std::string& targetRepresentationName,
  vtkSegmentationConverter::ConversionPathAndCostListType &pathsCosts)
{
  pathsCosts.clear();
  this->Converter->GetPossibleConversions(this->MasterRepresentationName, targetRepresentationName, pathsCosts);
};

//-----------------------------------------------------------------------------
bool vtkSegmentation::CopySegmentFromSegmentation(vtkSegmentation* fromSegmentation, std::string segmentId, bool removeFromSource/*=false*/)
{
  if (!fromSegmentation || segmentId.empty())
    {
    return false;
    }

  // If segment with the same ID is present in the target (this instance), then do not copy
  std::string targetSegmentId = segmentId;
  if (this->GetSegment(segmentId))
    {
    targetSegmentId = this->GenerateUniqueSegmentID(segmentId);
    vtkWarningMacro("CopySegmentFromSegmentation: Segment with the same ID as the copied one (" << segmentId << ") already exists in the target segmentation. Generate a new unique segment ID: " << targetSegmentId);
    }

  // Get segment from source
  vtkSegment* segment = fromSegmentation->GetSegment(segmentId);
  if (!segment)
    {
    vtkErrorMacro("CopySegmentFromSegmentation: Failed to get segment!");
    return false;
    }

  // If source segmentation contains reference image geometry conversion parameter,
  // but target segmentation does not, then copy that parameter from the source segmentation
  // TODO: Do this with all parameters? (so those which have non-default values are replaced)
  std::string referenceImageGeometryParameter = this->GetConversionParameter(vtkSegmentationConverter::GetReferenceImageGeometryParameterName());
  std::string fromReferenceImageGeometryParameter = fromSegmentation->GetConversionParameter(vtkSegmentationConverter::GetReferenceImageGeometryParameterName());
  if (referenceImageGeometryParameter.empty() && !fromReferenceImageGeometryParameter.empty())
    {
    this->SetConversionParameter(vtkSegmentationConverter::GetReferenceImageGeometryParameterName(), fromReferenceImageGeometryParameter);
    }

  // If copy, then duplicate segment and add it to the target segmentation
  if (!removeFromSource)
    {
    vtkSmartPointer<vtkSegment> segmentCopy = vtkSmartPointer<vtkSegment>::New();
    segmentCopy->DeepCopy(segment);
    if (!this->AddSegment(segmentCopy, targetSegmentId))
      {
      vtkErrorMacro("CopySegmentFromSegmentation: Failed to add segment '" << targetSegmentId << "' to segmentation");
      return false;
      }
    }
  // If move, then just add segment to target and remove from source (ownership is transferred)
  else
    {
    if (!this->AddSegment(segment, targetSegmentId))
      {
      vtkErrorMacro("CopySegmentFromSegmentation: Failed to add segment '" << targetSegmentId << "' to segmentation");
      return false;
      }
    fromSegmentation->RemoveSegment(segmentId);
    }

  return true;
}

//-----------------------------------------------------------------------------
std::string vtkSegmentation::DetermineCommonLabelmapGeometry(int extentComputationMode, vtkStringArray* segmentIds)
{
  std::vector<std::string> segmentIdsVector;
  if (segmentIds)
  {
    for (int segmentIndex = 0; segmentIndex < segmentIds->GetNumberOfValues(); ++segmentIndex)
    {
      segmentIdsVector.push_back(segmentIds->GetValue(segmentIndex));
    }
  }
  return this->DetermineCommonLabelmapGeometry(extentComputationMode, segmentIdsVector);
}

//-----------------------------------------------------------------------------
void vtkSegmentation::DetermineCommonLabelmapExtent(int commonGeometryExtent[6], vtkOrientedImageData* commonGeometryImage,
  vtkStringArray* segmentIds /*=NULL*/, bool computeEffectiveExtent /*=false*/, bool addPadding /*=false*/)
{
  std::vector<std::string> segmentIdsVector;
  if (segmentIds)
  {
    for (int segmentIndex = 0; segmentIndex < segmentIds->GetNumberOfValues(); ++segmentIndex)
    {
      segmentIdsVector.push_back(segmentIds->GetValue(segmentIndex));
    }
  }
  this->DetermineCommonLabelmapExtent(commonGeometryExtent, commonGeometryImage, segmentIdsVector, computeEffectiveExtent, addPadding);
}

//-----------------------------------------------------------------------------
std::string vtkSegmentation::DetermineCommonLabelmapGeometry(int extentComputationMode, const std::vector<std::string>& segmentIDs/*=std::vector<std::string>()*/)
{
  // If segment IDs list is empty then include all segments
  std::vector<std::string> mergedSegmentIDs;
  if (segmentIDs.empty())
    {
    this->GetSegmentIDs(mergedSegmentIDs);
    }
  else
    {
    mergedSegmentIDs = segmentIDs;
    }

  // Get highest resolution reference geometry available in segments
  vtkOrientedImageData* highestResolutionLabelmap = NULL;
  double lowestSpacing[3] = {1, 1, 1}; // We'll multiply the spacings together to get the voxel size
  for (std::vector<std::string>::iterator segmentIt = mergedSegmentIDs.begin(); segmentIt != mergedSegmentIDs.end(); ++segmentIt)
    {
    vtkSegment* currentSegment = this->GetSegment(*segmentIt);
    if (!currentSegment)
      {
      vtkWarningMacro("DetermineCommonLabelmapGeometry: Segment ID " << (*segmentIt) << " not found in segmentation");
      continue;
      }
    vtkOrientedImageData* currentBinaryLabelmap = vtkOrientedImageData::SafeDownCast(
      currentSegment->GetRepresentation(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName()) );
    if (currentBinaryLabelmap->IsEmpty())
      {
      continue;
      }

    double currentSpacing[3] = {1, 1, 1};
    currentBinaryLabelmap->GetSpacing(currentSpacing);
    if (!highestResolutionLabelmap
      || currentSpacing[0] * currentSpacing[1] * currentSpacing[2] < lowestSpacing[0] * lowestSpacing[1] * lowestSpacing[2])
      {
      lowestSpacing[0] = currentSpacing[0];
      lowestSpacing[1] = currentSpacing[1];
      lowestSpacing[2] = currentSpacing[2];
      highestResolutionLabelmap = currentBinaryLabelmap;
      }
    }
  if (!highestResolutionLabelmap)
    {
    // This can occur if there are only empty segments in the segmentation
    return std::string("");
    }

  // Get reference image geometry conversion parameter
  std::string referenceGeometryString = this->GetConversionParameter(vtkSegmentationConverter::GetReferenceImageGeometryParameterName());
  if (referenceGeometryString.empty())
    {
    // Reference image geometry might be missing because segmentation was created from labelmaps.
    // Set reference image geometry from highest resolution segment labelmap
    if (!highestResolutionLabelmap)
      {
      vtkErrorMacro("DetermineCommonLabelmapGeometry: Unable to find largest extent labelmap to define reference image geometry!");
      return std::string("");
      }
    referenceGeometryString = vtkSegmentationConverter::SerializeImageGeometry(highestResolutionLabelmap);
    }

  vtkSmartPointer<vtkOrientedImageData> commonGeometryImage = vtkSmartPointer<vtkOrientedImageData>::New();
  vtkSegmentationConverter::DeserializeImageGeometry(referenceGeometryString, commonGeometryImage, false);

  if (extentComputationMode == EXTENT_UNION_OF_SEGMENTS || extentComputationMode == EXTENT_UNION_OF_EFFECTIVE_SEGMENTS
    || extentComputationMode == EXTENT_UNION_OF_SEGMENTS_PADDED || extentComputationMode == EXTENT_UNION_OF_EFFECTIVE_SEGMENTS_PADDED)
    {
    // Determine extent that contains all segments
    int commonGeometryExtent[6] = { 0, -1, 0, -1, 0, -1 };
    this->DetermineCommonLabelmapExtent(commonGeometryExtent, commonGeometryImage, mergedSegmentIDs,
      extentComputationMode == EXTENT_UNION_OF_EFFECTIVE_SEGMENTS || extentComputationMode == EXTENT_UNION_OF_EFFECTIVE_SEGMENTS_PADDED,
      extentComputationMode == EXTENT_UNION_OF_SEGMENTS_PADDED || extentComputationMode == EXTENT_UNION_OF_EFFECTIVE_SEGMENTS_PADDED);
    commonGeometryImage->SetExtent(commonGeometryExtent);
    }

  // Oversample reference image geometry to match highest resolution labelmap's spacing
  double referenceSpacing[3] = {0.0,0.0,0.0};
  commonGeometryImage->GetSpacing(referenceSpacing);
  double voxelSizeRatio = ((referenceSpacing[0]*referenceSpacing[1]*referenceSpacing[2]) / (lowestSpacing[0]*lowestSpacing[1]*lowestSpacing[2]));
  // Round oversampling to the nearest integer
  // Note: We need to round to some degree, because e.g. pow(64,1/3) is not exactly 4. It may be debated whether to round to integer or to a certain number of decimals
  double oversamplingFactor = vtkMath::Round( pow( voxelSizeRatio, 1.0/3.0 ) );
  vtkCalculateOversamplingFactor::ApplyOversamplingOnImageGeometry(commonGeometryImage, oversamplingFactor);

  // Serialize common geometry and return it
  return vtkSegmentationConverter::SerializeImageGeometry(commonGeometryImage);
}

//-----------------------------------------------------------------------------
void vtkSegmentation::DetermineCommonLabelmapExtent(int commonGeometryExtent[6], vtkOrientedImageData* commonGeometryImage,
  const std::vector<std::string>& segmentIDs/*=std::vector<std::string>()*/, bool computeEffectiveExtent /*=false*/, bool addPadding /*=false*/)
{
  // If segment IDs list is empty then include all segments
  std::vector<std::string> mergedSegmentIDs;
  if (segmentIDs.empty())
    {
    this->GetSegmentIDs(mergedSegmentIDs);
    }
  else
    {
    mergedSegmentIDs = segmentIDs;
    }

  // Determine extent that contains all segments
  commonGeometryExtent[0] = 0;
  commonGeometryExtent[1] = -1;
  commonGeometryExtent[2] = 0;
  commonGeometryExtent[3] = -1;
  commonGeometryExtent[4] = 0;
  commonGeometryExtent[5] = -1;
  for (std::vector<std::string>::iterator segmentIt = mergedSegmentIDs.begin(); segmentIt != mergedSegmentIDs.end(); ++segmentIt)
    {
    vtkSegment* currentSegment = this->GetSegment(*segmentIt);
    if (!currentSegment)
      {
      vtkWarningMacro("DetermineCommonLabelmapGeometry: Segment ID " << (*segmentIt) << " not found in segmentation");
      continue;
      }
    vtkOrientedImageData* currentBinaryLabelmap = vtkOrientedImageData::SafeDownCast(
      currentSegment->GetRepresentation(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName()));
    if (currentBinaryLabelmap==NULL || currentBinaryLabelmap->IsEmpty())
      {
      continue;
      }

    int currentBinaryLabelmapExtent[6] = { 0, -1, 0, -1, 0, -1 };
    bool validExtent = true;
    if (computeEffectiveExtent)
      {
      validExtent = vtkOrientedImageDataResample::CalculateEffectiveExtent(currentBinaryLabelmap, currentBinaryLabelmapExtent);
      }
    else
      {
      currentBinaryLabelmap->GetExtent(currentBinaryLabelmapExtent);
      }
    if (validExtent && currentBinaryLabelmapExtent[0] <= currentBinaryLabelmapExtent[1]
      && currentBinaryLabelmapExtent[2] <= currentBinaryLabelmapExtent[3]
      && currentBinaryLabelmapExtent[4] <= currentBinaryLabelmapExtent[5])
      {
      // There is a valid labelmap

      // Get transformed extents of the segment in the common labelmap geometry
      vtkNew<vtkTransform> currentBinaryLabelmapToCommonGeometryImageTransform;
      vtkOrientedImageDataResample::GetTransformBetweenOrientedImages(currentBinaryLabelmap, commonGeometryImage, currentBinaryLabelmapToCommonGeometryImageTransform.GetPointer());
      int currentBinaryLabelmapExtentInCommonGeometryImageFrame[6] = { 0, -1, 0, -1, 0, -1 };
      vtkOrientedImageDataResample::TransformExtent(currentBinaryLabelmapExtent, currentBinaryLabelmapToCommonGeometryImageTransform.GetPointer(), currentBinaryLabelmapExtentInCommonGeometryImageFrame);
      if (commonGeometryExtent[0] > commonGeometryExtent[1] || commonGeometryExtent[2] > commonGeometryExtent[3] || commonGeometryExtent[4] > commonGeometryExtent[5])
        {
        // empty commonGeometryExtent
        for (int i = 0; i < 3; i++)
          {
          commonGeometryExtent[i * 2] = currentBinaryLabelmapExtentInCommonGeometryImageFrame[i * 2];
          commonGeometryExtent[i * 2 + 1] = currentBinaryLabelmapExtentInCommonGeometryImageFrame[i * 2 + 1];
          }
        }
      else
        {
        for (int i = 0; i < 3; i++)
          {
          commonGeometryExtent[i * 2] = std::min(currentBinaryLabelmapExtentInCommonGeometryImageFrame[i * 2], commonGeometryExtent[i * 2]);
          commonGeometryExtent[i * 2 + 1] = std::max(currentBinaryLabelmapExtentInCommonGeometryImageFrame[i * 2 + 1], commonGeometryExtent[i * 2 + 1]);
          }
        }
      }
    }
  if (addPadding)
    {
    // Add single-voxel padding
    for (int i = 0; i < 3; i++)
      {
      if (commonGeometryExtent[i * 2]>commonGeometryExtent[i * 2 + 1])
        {
        // empty along this dimension, do not pad
        continue;
        }
      commonGeometryExtent[i * 2] -= 1;
      commonGeometryExtent[i * 2 + 1] += 1;
      }
    }
}

//----------------------------------------------------------------------------
bool vtkSegmentation::SetImageGeometryFromCommonLabelmapGeometry(vtkOrientedImageData* imageData, vtkStringArray* segmentIDs /*=NULL*/,
  int extentComputationMode /*=vtkSegmentation::EXTENT_UNION_OF_EFFECTIVE_SEGMENTS*/)
{
  std::string commonGeometryString = this->DetermineCommonLabelmapGeometry(extentComputationMode, segmentIDs);
  return vtkSegmentationConverter::DeserializeImageGeometry(commonGeometryString, imageData, false /* do not allocate scalars */);
}

//----------------------------------------------------------------------------
bool vtkSegmentation::ConvertSingleSegment(std::string segmentId, std::string targetRepresentationName)
{
  vtkSegment* segment = this->GetSegment(segmentId);
  if (!segment)
    {
    vtkErrorMacro("ConvertSingleSegment: Failed to find segment with ID " << segmentId);
    return false;
    }

  // Get possible conversion paths from master to the requested target representation
  vtkSegmentationConverter::ConversionPathAndCostListType pathCosts;
  this->Converter->GetPossibleConversions(this->MasterRepresentationName, targetRepresentationName, pathCosts);
  // Get cheapest path from found conversion paths
  vtkSegmentationConverter::ConversionPathType cheapestPath = vtkSegmentationConverter::GetCheapestPath(pathCosts);
  if (cheapestPath.empty())
    {
    return false;
    }

  // Perform conversion (overwrite if exists)
  if (!this->ConvertSegmentUsingPath(segment, cheapestPath, true))
    {
    vtkErrorMacro("ConvertSingleSegment: Conversion failed!");
    return false;
    }

  return true;
}

//----------------------------------------------------------------------------
std::string vtkSegmentation::SerializeAllConversionParameters()
{
  return this->Converter->SerializeAllConversionParameters();
}

//----------------------------------------------------------------------------
void vtkSegmentation::DeserializeConversionParameters(std::string conversionParametersString)
{
  this->Converter->DeserializeConversionParameters(conversionParametersString);
}
