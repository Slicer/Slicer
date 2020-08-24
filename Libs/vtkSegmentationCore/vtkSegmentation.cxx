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
#include "vtkSegmentationHistory.h"

#include "vtkOrientedImageData.h"
#include "vtkOrientedImageDataResample.h"
#include "vtkCalculateOversamplingFactor.h"

// VTK includes
#include <vtkAbstractTransform.h>
#include <vtkBoundingBox.h>
#include <vtkCallbackCommand.h>
#include <vtkCollection.h>
#include <vtkImageThreshold.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkStringArray.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkVersion.h>

// STD includes
#include <algorithm>
#include <functional>
#include <sstream>

const int DEFAULT_LABEL_VALUE = 1;

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
  this->SegmentModifiedEnabled = true;

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
    this->SegmentCallbackCommand->SetClientData(nullptr);
    this->SegmentCallbackCommand->Delete();
    this->SegmentCallbackCommand = nullptr;
    }

  if (this->MasterRepresentationCallbackCommand)
    {
    this->MasterRepresentationCallbackCommand->SetClientData(nullptr);
    this->MasterRepresentationCallbackCommand->Delete();
    this->MasterRepresentationCallbackCommand = nullptr;
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
  const char* attName = nullptr;
  const char* attValue = nullptr;
  while (*atts != nullptr)
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
  std::map<vtkDataObject*, vtkDataObject*> copiedDataObjects;
  for (std::deque< std::string >::iterator segmentIdIt = aSegmentation->SegmentIds.begin(); segmentIdIt != aSegmentation->SegmentIds.end(); ++segmentIdIt)
    {
    vtkSmartPointer<vtkSegment> segment = vtkSmartPointer<vtkSegment>::New();
    vtkSegmentation::CopySegment(segment, aSegmentation->Segments[*segmentIdIt], nullptr, copiedDataObjects);
    this->AddSegment(segment, *segmentIdIt);
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
  this->MasterRepresentationModifiedEnabled = enabled;

  // Add/remove observation of master representation in all segments
  this->UpdateMasterRepresentationObservers();
  return !enabled; // return old value
}

//---------------------------------------------------------------------------
bool vtkSegmentation::SetSegmentModifiedEnabled(bool enabled)
{
  if (this->SegmentModifiedEnabled == enabled)
    {
    return this->SegmentModifiedEnabled;
    }
  // Add/remove observation of master representation in all segments
  for (SegmentMap::iterator segmentIt = this->Segments.begin(); segmentIt != this->Segments.end(); ++segmentIt)
    {
    if (enabled)
      {
      if (!segmentIt->second->HasObserver(vtkCommand::ModifiedEvent, this->SegmentCallbackCommand))
        {
        segmentIt->second->AddObserver(vtkCommand::ModifiedEvent, this->SegmentCallbackCommand);
        }
      }
    else
      {
      segmentIt->second->RemoveObservers(vtkCommand::ModifiedEvent, this->SegmentCallbackCommand);
      }
    }
  this->SegmentModifiedEnabled = enabled;
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
  if (this->SegmentModifiedEnabled && !segment->HasObserver(vtkCommand::ModifiedEvent, this->SegmentCallbackCommand))
    {
    segment->AddObserver(vtkCommand::ModifiedEvent, this->SegmentCallbackCommand);
    }

  bool representationsCreated = true;

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
      vtkSmartPointer<vtkDataObject> emptyRepresentation;
      if (this->GetMasterRepresentationName() == vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName())
        {
        for (std::deque<std::string>::iterator segmentIDIt = this->SegmentIds.begin(); segmentIDIt != this->SegmentIds.end(); ++segmentIDIt)
          {
          emptyRepresentation = segment->GetRepresentation(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName());
          if (emptyRepresentation)
            {
            break;
            }
          }
        }

      if (!emptyRepresentation)
        {
        emptyRepresentation = vtkSmartPointer<vtkDataObject>::Take(
          vtkSegmentationConverterFactory::GetInstance()->ConstructRepresentationObjectByRepresentation(*reprIt));
        if (!emptyRepresentation)
          {
          vtkErrorMacro("AddSegment: Unable to construct empty representation type '" << (*reprIt) << "'");
          representationsCreated = false;
          }
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
        representationsCreated = false;
        }
      }

    if (representationsCreated)
      {
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
            representationsCreated = false;
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
    }

  if (!representationsCreated)
    {
    SegmentMap::iterator segmentIt = std::find_if(
      this->Segments.begin(), this->Segments.end(), std::bind2nd(MapValueCompare<SegmentMap>(), segment));
    if (segmentIt != this->Segments.end())
      {
      this->Segments.erase(segmentIt);
      }
    return false;
    }

  // Add observation of master representation in new segment
  this->UpdateMasterRepresentationObservers();

  // Add to list. If segmentId is empty, then segment name becomes the ID
  std::string key = segmentId;
  if (key.empty())
    {
    if (segment->GetName() == nullptr)
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

  this->SeparateSegmentLabelmap(segmentId);

  // Remove segment
  this->SegmentIds.erase(std::remove(this->SegmentIds.begin(), this->SegmentIds.end(), segmentId), this->SegmentIds.end());
  this->Segments.erase(segmentIt);
  if (this->Segments.empty())
    {
    this->SegmentIdAutogeneratorIndex = 0;
    }

  this->UpdateMasterRepresentationObservers();

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
                                        unsigned long eid,
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
  if (eid == vtkCommand::ModifiedEvent)
    {
    self->InvokeEvent(vtkSegmentation::SegmentModified, (void*)(segmentIdChars));
    }

  self->UpdateMasterRepresentationObservers();
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
void vtkSegmentation::UpdateMasterRepresentationObservers()
{
  std::set<vtkSmartPointer<vtkDataObject> > newMasterRepresentations;
  // Add/remove observation of master representation in all segments
  for (SegmentMap::iterator segmentIt = this->Segments.begin(); segmentIt != this->Segments.end(); ++segmentIt)
    {
    vtkDataObject* masterRepresentation = segmentIt->second->GetRepresentation(this->MasterRepresentationName);
    if (masterRepresentation)
      {
      newMasterRepresentations.insert(masterRepresentation);
      }
    }

  // Remove observers from master representations that are no longer in any segments
  for (vtkSmartPointer<vtkDataObject> masterRepresentation : this->MasterRepresentationCache)
    {
    if (std::find(newMasterRepresentations.begin(), newMasterRepresentations.end(), masterRepresentation) == newMasterRepresentations.end())
      {
      masterRepresentation->RemoveObservers(vtkCommand::ModifiedEvent, this->MasterRepresentationCallbackCommand);
      }
    }

  // Add/remove observation of master representation in all segments
  for (vtkSmartPointer<vtkDataObject> masterRepresentation : newMasterRepresentations)
    {
    if (this->MasterRepresentationModifiedEnabled)
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
  this->MasterRepresentationCache = newMasterRepresentations;
}

//---------------------------------------------------------------------------
vtkSegment* vtkSegmentation::GetSegment(std::string segmentId)
{
  SegmentMap::iterator segmentIt = this->Segments.find(segmentId);
  if (segmentIt == this->Segments.end())
    {
    return nullptr;
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
    return nullptr;
    }
  std::string segmentId = this->SegmentIds[index];
  SegmentMap::const_iterator segmentIt = this->Segments.find(segmentId);
  if (segmentIt == this->Segments.end())
    {
    // inconsistent segment ID and segment list
    return nullptr;
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
    vtkErrorMacro("vtkSegmentation::SetSegmentIndex failed: segment not found by ID " << segmentId);
    return false;
    }
  this->SegmentIds.erase(foundIt);
  this->SegmentIds.insert(this->SegmentIds.begin() + newIndex, segmentId);
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
      // ### Slicer 4.4: Simplify this logic when adding support for C++11 across all supported platform/compilers
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

  // Apply transform on reference image geometry conversion parameter (to preserve validity of shared labelmap)
  this->Converter->ApplyTransformOnReferenceImageGeometry(transform);

  // Apply linear transform for each segment:
  // Harden transform on master representation if poly data, apply directions if oriented image data
  std::set<vtkDataObject*> transformedDataObjects;
  for (SegmentMap::iterator it = this->Segments.begin(); it != this->Segments.end(); ++it)
    {
    vtkDataObject* currentMasterRepresentation = it->second->GetRepresentation(this->MasterRepresentationName);
    if (!currentMasterRepresentation)
      {
      vtkErrorMacro("ApplyLinearTransform: Cannot get master representation (" << this->MasterRepresentationName << ") from segment!");
      return;
      }
    if (transformedDataObjects.find(currentMasterRepresentation) != transformedDataObjects.end())
      {
      continue;
      }
    transformedDataObjects.insert(currentMasterRepresentation);

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

  // Apply transform on reference image geometry conversion parameter (to preserve validity of shared labelmap)
  this->Converter->ApplyTransformOnReferenceImageGeometry(transform);

  // Harden transform on master representation (both image data and poly data) for each segment individually
  std::set<vtkDataObject*> transformedDataObjects;
  for (SegmentMap::iterator it = this->Segments.begin(); it != this->Segments.end(); ++it)
    {
    vtkDataObject* currentMasterRepresentation = it->second->GetRepresentation(this->MasterRepresentationName);
    if (!currentMasterRepresentation)
      {
      vtkErrorMacro("ApplyNonLinearTransform: Cannot get master representation (" << this->MasterRepresentationName << ") from segment!");
      return;
      }
    if (transformedDataObjects.find(currentMasterRepresentation) != transformedDataObjects.end())
      {
      continue;
      }
    transformedDataObjects.insert(currentMasterRepresentation);

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
bool vtkSegmentation::ConvertSegmentsUsingPath(std::vector<std::string> segmentIDs, vtkSegmentationConverter::ConversionPathType path, bool overwriteExisting)
{
  if (segmentIDs.empty())
    {
    return true;
    }

  // Execute each conversion step in the selected path
  vtkSegmentationConverter::ConversionPathType::iterator pathIt;
  for (pathIt = path.begin(); pathIt != path.end(); ++pathIt)
    {
    vtkSegmentationConverterRule* currentConversionRule = (*pathIt);
    if (!currentConversionRule)
      {
      vtkErrorMacro("ConvertSegmentsUsingPath: Invalid converter rule!");
      return false;
      }

    // Perform conversion step
    currentConversionRule->PreConvert(this);
    for (auto segmentID : segmentIDs)
      {
      vtkSegment* segment = this->GetSegment(segmentID);

      // Get source representation from segment. It is expected to exist
      vtkDataObject* sourceRepresentation = segment->GetRepresentation(
        currentConversionRule->GetSourceRepresentationName());
      if (!sourceRepresentation)
        {
        vtkErrorMacro("ConvertSegmentsUsingPath: Source representation does not exist!");
        return false;
        }

      // Get target representation
      vtkSmartPointer<vtkDataObject> targetRepresentation = segment->GetRepresentation(
        currentConversionRule->GetTargetRepresentationName());
      // If target representation exists and we do not overwrite existing representations,
      // then no conversion is necessary with this conversion rule
      if (targetRepresentation.GetPointer() && !overwriteExisting)
        {
        continue;
        }
      currentConversionRule->Convert(segment);
      }
    currentConversionRule->PostConvert(this);

  }

  return true;
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

    // Perform conversion step
    currentConversionRule->PreConvert(this);
    currentConversionRule->Convert(segment);
    currentConversionRule->PostConvert(this);
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
  // Delay segment modified event invocation until all segments have the new representation.
  std::deque< std::string > modifiedSegmentIds;

  bool wasSegmentModifiedEnabled = this->SetSegmentModifiedEnabled(false);
  std::map<std::string, vtkDataObject*> representationsBefore;
  for (SegmentMap::iterator segmentIt = this->Segments.begin(); segmentIt != this->Segments.end(); ++segmentIt)
    {
    representationsBefore[segmentIt->first] = segmentIt->second->GetRepresentation(targetRepresentationName);
    }

  std::vector<std::string> segmentIDs;
  this->GetSegmentIDs(segmentIDs);
  if (!this->ConvertSegmentsUsingPath(segmentIDs, cheapestPath, alwaysConvert))
    {
    vtkErrorMacro("CreateRepresentation: Conversion failed");
    return false;
    }

  for (SegmentMap::iterator segmentIt = this->Segments.begin(); segmentIt != this->Segments.end(); ++segmentIt)
    {
    vtkDataObject* representationBefore = representationsBefore[segmentIt->first];
    vtkDataObject* representationAfter = segmentIt->second->GetRepresentation(targetRepresentationName);
    if (representationBefore != representationAfter
      || (representationBefore != nullptr && representationAfter != nullptr && representationBefore->GetMTime() != representationAfter->GetMTime()) )
      {
      // representation has been modified
      modifiedSegmentIds.push_back(segmentIt->first);
      }
    }

  this->SetSegmentModifiedEnabled(wasSegmentModifiedEnabled);

  // All the updates are completed, now invoke modified events
  for (std::deque< std::string >::iterator segmentIdIt = modifiedSegmentIds.begin();
    segmentIdIt != modifiedSegmentIds.end(); ++segmentIdIt)
    {
    const char* segmentId = segmentIdIt->c_str();
    vtkSegment* segment = GetSegment(segmentId);
    if (segment)
      {
      segment->Modified();
      }
    this->InvokeEvent(vtkSegmentation::RepresentationModified, (void*)segmentId);
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
  std::vector<std::string> segmentIDs;
  this->GetSegmentIDs(segmentIDs);
  this->ConvertSegmentsUsingPath(segmentIDs, path, true);

  for (std::string segmentID : segmentIDs)
    {
    this->InvokeEvent(vtkSegmentation::RepresentationModified, (void*)segmentID.c_str());
    }

  this->InvokeEvent(vtkSegmentation::ContainedRepresentationNamesModified);
  return true;
}

//---------------------------------------------------------------------------
void vtkSegmentation::RemoveRepresentation(const std::string& representationName)
{
  // We temporarily disable modification of segments to avoid invoking events
  // when segmentation is in an inconsistent state (when segments have different
  // representations). We call Modified events after all the updates are completed.
  std::deque< vtkSegment* > modifiedSegments;
  bool wasSegmentModifiedEnabled = this->SetSegmentModifiedEnabled(false);
  for (SegmentMap::iterator segmentIt = this->Segments.begin(); segmentIt != this->Segments.end(); ++segmentIt)
    {
    if (segmentIt->second->RemoveRepresentation(representationName))
      {
      modifiedSegments.push_back(segmentIt->second);
      }
    }
  this->SetSegmentModifiedEnabled(wasSegmentModifiedEnabled);

  // All the updates are completed, now invoke modified events
  for (std::deque< vtkSegment* >::iterator segmentIt = modifiedSegments.begin(); segmentIt != modifiedSegments.end();
    ++segmentIt)
    {
    (*segmentIt)->Modified();
    }
  this->InvokeEvent(vtkSegmentation::ContainedRepresentationNamesModified);
}

//---------------------------------------------------------------------------
vtkDataObject* vtkSegmentation::GetSegmentRepresentation(std::string segmentId, std::string representationName)
{
  vtkSegment* segment = this->GetSegment(segmentId);
  if (!segment)
    {
    return nullptr;
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
  this->InvokeEvent(vtkSegmentation::ContainedRepresentationNamesModified);
}

//---------------------------------------------------------------------------
bool vtkSegmentation::IsSharedBinaryLabelmap(std::string segmentId)
{
  std::vector<std::string> sharedLabelmapSegmentIds;
    this->GetSegmentIDsSharingBinaryLabelmapRepresentation(segmentId, sharedLabelmapSegmentIds, false);
  return sharedLabelmapSegmentIds.empty();
}

//---------------------------------------------------------------------------
void vtkSegmentation::GetSegmentIDsSharingRepresentation(std::string originalSegmentId, std::string representationName,
  std::vector<std::string>& sharedSegmentIds, bool includeOriginalSegmentId/*=true*/)
{
  sharedSegmentIds.clear();

  vtkSegment* originalSegment = this->GetSegment(originalSegmentId);
  if (!originalSegment)
    {
    return;
    }

  vtkDataObject* originalBinaryLabelmap = originalSegment->GetRepresentation(representationName);
  if (!originalBinaryLabelmap)
    {
    return;
    }

  for (std::pair<std::string, vtkSegment*> segmentPair : this->Segments)
    {
    vtkSegment* currentSegment = segmentPair.second;
    if (!includeOriginalSegmentId && originalSegment == currentSegment)
      {
      continue;
      }

    vtkDataObject* binaryLabelmap = currentSegment->GetRepresentation(representationName);
    if (originalBinaryLabelmap == binaryLabelmap)
      {
      sharedSegmentIds.push_back(segmentPair.first);
      }
    }
}

//---------------------------------------------------------------------------
void vtkSegmentation::GetSegmentIDsSharingBinaryLabelmapRepresentation(std::string originalSegmentId, std::vector<std::string>& sharedSegmentIds,
  bool includeOriginalSegmentId/*=true*/)
{
  this->GetSegmentIDsSharingRepresentation(originalSegmentId, vtkSegmentationConverter::GetBinaryLabelmapRepresentationName(), sharedSegmentIds,
    includeOriginalSegmentId/*=true*/);
}

//---------------------------------------------------------------------------
void vtkSegmentation::MergeSegmentLabelmaps(std::vector<std::string> mergeSegmentIds)
{
  vtkNew<vtkOrientedImageData> sharedLabelmapRepresentation;
  this->GenerateMergedLabelmap(sharedLabelmapRepresentation, EXTENT_UNION_OF_EFFECTIVE_SEGMENTS, nullptr, mergeSegmentIds);

  int value = 0;
  for (std::string segmentId : mergeSegmentIds)
    {
    vtkSegment* segment = this->GetSegment(segmentId);
    ++value;
    segment->SetLabelValue(value);
    segment->AddRepresentation(vtkSegmentationConverter::GetBinaryLabelmapRepresentationName(), sharedLabelmapRepresentation);
    }
  sharedLabelmapRepresentation->Modified();
}

//---------------------------------------------------------------------------
bool vtkSegmentation::GenerateMergedLabelmap(
  vtkOrientedImageData* sharedImageData,
  int extentComputationMode,
  vtkOrientedImageData* sharedLabelmapGeometry/*=nullptr*/,
  const std::vector<std::string>& segmentIDs/*=std::vector<std::string>()*/,
  vtkIntArray* labelValues/*=nullptr*/)
{
  if (!sharedImageData)
    {
    vtkErrorMacro("GenerateSharedLabelmap: Invalid image data");
    return false;
    }

  if (!this->ContainsRepresentation(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName()))
    {
    vtkErrorMacro("GenerateSharedLabelmap: Segmentation does not contain binary labelmap representation");
    return false;
    }

  // If segment IDs list is empty then include all segments
  std::vector<std::string> sharedSegmentIDs;
  if (segmentIDs.empty())
    {
    this->GetSegmentIDs(sharedSegmentIDs);
    }
  else
    {
    sharedSegmentIDs = segmentIDs;
    }

  if (labelValues && labelValues->GetNumberOfValues() != sharedSegmentIDs.size())
    {
    vtkErrorMacro("GenerateSharedLabelmap: Number of label values does not equal the number of segment IDs");
    return false;
    }

  // Determine common labelmap geometry that will be used for the shared labelmap
  vtkSmartPointer<vtkMatrix4x4> sharedImageToWorldMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  vtkSmartPointer<vtkOrientedImageData> commonGeometryImage;
  if (sharedLabelmapGeometry)
    {
    // Use shared labelmap geometry if provided
    commonGeometryImage = sharedLabelmapGeometry;
    sharedLabelmapGeometry->GetImageToWorldMatrix(sharedImageToWorldMatrix);
    }
  else
    {
    commonGeometryImage = vtkSmartPointer<vtkOrientedImageData>::New();
    std::string commonGeometryString = this->DetermineCommonLabelmapGeometry(extentComputationMode, sharedSegmentIDs);
    if (commonGeometryString.empty())
      {
      // This can occur if there are only empty segments in the segmentation
      sharedImageToWorldMatrix->Identity();
      return true;
      }
    vtkSegmentationConverter::DeserializeImageGeometry(commonGeometryString, commonGeometryImage, false);
    }
  commonGeometryImage->GetImageToWorldMatrix(sharedImageToWorldMatrix);
  int referenceDimensions[3] = { 0,0,0 };
  commonGeometryImage->GetDimensions(referenceDimensions);
  int referenceExtent[6] = { 0,-1,0,-1,0,-1 };
  commonGeometryImage->GetExtent(referenceExtent);

  // Allocate image data if empty or if reference extent changed
  int imageDataExtent[6] = { 0,-1,0,-1,0,-1 };
  sharedImageData->GetExtent(imageDataExtent);
  if (sharedImageData->GetScalarType() != VTK_SHORT
    || imageDataExtent[0] != referenceExtent[0] || imageDataExtent[1] != referenceExtent[1] || imageDataExtent[2] != referenceExtent[2]
    || imageDataExtent[3] != referenceExtent[3] || imageDataExtent[4] != referenceExtent[4] || imageDataExtent[5] != referenceExtent[5])
    {
    if (sharedImageData->GetPointData()->GetScalars() && sharedImageData->GetScalarType() != VTK_SHORT)
      {
      vtkWarningMacro("GenerateSharedLabelmap: Shared image data scalar type is not short. Allocating using short.");
      }
    sharedImageData->SetExtent(referenceExtent);
    sharedImageData->AllocateScalars(VTK_SHORT, 1);
    }
  sharedImageData->SetImageToWorldMatrix(sharedImageToWorldMatrix);

  // Paint the image data background first
  short* sharedImagePtr = (short*)sharedImageData->GetScalarPointerForExtent(referenceExtent);
  if (!sharedImagePtr)
    {
    // Setting the extent may invoke this function again via ImageDataModified, in which case the pointer is nullptr
    return false;
    }

  const short backgroundColorIndex = 0;
  vtkOrientedImageDataResample::FillImage(sharedImageData, backgroundColorIndex);

  // Skip the rest if there are no segments
  if (this->GetNumberOfSegments() == 0)
    {
    return true;
    }

  // Create shared labelmap
  bool success = true;
  short segmentIndex = 0;
  for (std::vector<std::string>::iterator segmentIdIt = sharedSegmentIDs.begin(); segmentIdIt != sharedSegmentIDs.end(); ++segmentIdIt, ++segmentIndex)
    {
    std::string currentSegmentId = *segmentIdIt;
    vtkSegment* currentSegment = this->GetSegment(currentSegmentId);
    if (!currentSegment)
      {
      vtkErrorMacro("GenerateSharedLabelmap: Segment not found by ID: " << currentSegmentId);
      success = false;
      continue;
      }

    // Get binary labelmap from segment
    vtkOrientedImageData* representationBinaryLabelmap = vtkOrientedImageData::SafeDownCast(
      currentSegment->GetRepresentation(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName()));
    // If binary labelmap is empty then skip
    if (representationBinaryLabelmap->IsEmpty())
      {
      continue;
      }

    // Set oriented image data used for merging to the representation (may change later if resampling is needed)
    vtkOrientedImageData* binaryLabelmap = representationBinaryLabelmap;

    // If labelmap geometries (origin, spacing, and directions) do not match reference then resample temporarily
    vtkSmartPointer<vtkOrientedImageData> resampledBinaryLabelmap;
    if (!vtkOrientedImageDataResample::DoGeometriesMatch(commonGeometryImage, representationBinaryLabelmap))
      {
      resampledBinaryLabelmap = vtkSmartPointer<vtkOrientedImageData>::New();

      // Resample segment labelmap for merging
      if (!vtkOrientedImageDataResample::ResampleOrientedImageToReferenceGeometry(
        representationBinaryLabelmap, sharedImageToWorldMatrix, resampledBinaryLabelmap))
        {
        vtkErrorMacro("GenerateSharedLabelmap: ResampleOrientedImageToReferenceGeometry failed for segment " << currentSegmentId);
        success = false;
        continue;
        }

      // Use resampled labelmap for merging
      binaryLabelmap = resampledBinaryLabelmap;
      }

    vtkNew<vtkOrientedImageData> thresholdedLabelmap;
    vtkNew<vtkImageThreshold> threshold;
    threshold->SetInputData(binaryLabelmap);
    threshold->ThresholdBetween(currentSegment->GetLabelValue(), currentSegment->GetLabelValue());
    threshold->SetInValue(1);
    threshold->SetOutValue(0);
    threshold->Update();
    thresholdedLabelmap->ShallowCopy(threshold->GetOutput());
    thresholdedLabelmap->CopyDirections(binaryLabelmap);
    binaryLabelmap = thresholdedLabelmap;

    int labelValue = backgroundColorIndex + 1 + segmentIndex;
    if (labelValues)
      {
      labelValue = labelValues->GetValue(segmentIndex);
      }

    // Copy image data voxels into shared labelmap with the proper color index
    vtkOrientedImageDataResample::ModifyImage(
      sharedImageData,
      binaryLabelmap,
      vtkOrientedImageDataResample::OPERATION_MASKING,
      nullptr,
      0,
      labelValue);
    }

  return success;
}

//---------------------------------------------------------------------------
void vtkSegmentation::SeparateSegmentLabelmap(std::string segmentId)
{
  vtkSegment* segment = this->GetSegment(segmentId);
  if (!segment)
    {
    return;
    }

  std::vector<std::string> sharedSegmentIDs;
  this->GetSegmentIDsSharingBinaryLabelmapRepresentation(segmentId, sharedSegmentIDs, false);
  if (sharedSegmentIDs.empty())
    {
    return;
    }

  vtkOrientedImageData* labelmap = vtkOrientedImageData::SafeDownCast(
    segment->GetRepresentation(vtkSegmentationConverter::GetBinaryLabelmapRepresentationName()));
  if (labelmap)
    {
    vtkNew<vtkImageThreshold> threshold;
    threshold->SetInputData(labelmap);
    threshold->ThresholdBetween(segment->GetLabelValue(), segment->GetLabelValue());
    threshold->SetOutValue(0);
    threshold->SetInValue(DEFAULT_LABEL_VALUE);
    threshold->Update();

    vtkSmartPointer<vtkOrientedImageData> tempImage = vtkSmartPointer<vtkOrientedImageData>::New();
    tempImage->ShallowCopy(threshold->GetOutput());
    tempImage->CopyDirections(labelmap);

    segment->AddRepresentation(vtkSegmentationConverter::GetBinaryLabelmapRepresentationName(), tempImage);

    vtkNew<vtkImageThreshold> thresholdErase;
    thresholdErase->SetInputData(labelmap);
    thresholdErase->ThresholdBetween(segment->GetLabelValue(), segment->GetLabelValue());
    thresholdErase->SetInValue(0);
    thresholdErase->ReplaceOutOff();
    thresholdErase->Update();
    labelmap->ShallowCopy(thresholdErase->GetOutput());
    }
  segment->SetLabelValue(DEFAULT_LABEL_VALUE);

  this->Modified();
  this->InvokeEvent(vtkSegmentation::MasterRepresentationModified, this);
  this->InvokeEvent(vtkSegmentation::ContainedRepresentationNamesModified);
}

//---------------------------------------------------------------------------
void vtkSegmentation::ClearSegment(std::string segmentId)
{
  vtkSegment* segment = this->GetSegment(segmentId);
  if (!segment)
    {
    return;
    }

  vtkDataObject* masterRepresentation = segment->GetRepresentation(this->GetMasterRepresentationName());
  if (!masterRepresentation)
    {
    return;
    }

  std::vector<std::string> sharedSegmentIDs;
  this->GetSegmentIDsSharingBinaryLabelmapRepresentation(segmentId, sharedSegmentIDs, false);
  if (this->GetMasterRepresentationName() == vtkSegmentationConverter::GetBinaryLabelmapRepresentationName() && !sharedSegmentIDs.empty())
    {
    vtkOrientedImageData* binaryLablemap = vtkOrientedImageData::SafeDownCast(masterRepresentation);
    if (binaryLablemap)
      {
      vtkNew<vtkImageThreshold> threshold;
      threshold->SetInputData(binaryLablemap);
      threshold->ThresholdBetween(segment->GetLabelValue(), segment->GetLabelValue());
      threshold->SetInValue(0);
      threshold->ReplaceOutOff();
      threshold->Update();
      binaryLablemap->ShallowCopy(threshold->GetOutput());
      binaryLablemap->Modified();
      }
    }
  else if (masterRepresentation)
    {
    masterRepresentation->Initialize();
    masterRepresentation->Modified();
    }
}

//---------------------------------------------------------------------------
int vtkSegmentation::GetUniqueLabelValueForSharedLabelmap(std::string segmentId)
{
  std::vector<std::string> sharedLabelmapIds;
  this->GetSegmentIDsSharingBinaryLabelmapRepresentation(segmentId, sharedLabelmapIds, true);

  std::set<int> labelValues;
  for (std::string currentSegmentId : sharedLabelmapIds)
    {
    vtkSegment* segment = this->GetSegment(currentSegmentId);
    labelValues.insert(segment->GetLabelValue());
    }

  int labelValue = 1;
  while (labelValues.find(labelValue) != labelValues.end())
    {
    ++labelValue;
    }
  return labelValue;
}

//---------------------------------------------------------------------------
int vtkSegmentation::GetUniqueLabelValueForSharedLabelmap(vtkOrientedImageData* labelmap)
{
  if (!labelmap)
    {
    return DEFAULT_LABEL_VALUE;
    }

  int* extent = labelmap->GetExtent();
  if (extent[1] < extent[0] || extent[3] < extent[2] || extent[5] < extent[4])
    {
    return DEFAULT_LABEL_VALUE;
    }

  double* scalarRange = labelmap->GetScalarRange();
  int highLabel = (int)scalarRange[1];
  return highLabel + 1;
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
    return vtkPolyData::SafeDownCast(masterRepresentation) != nullptr;
    }
  else
    {
    // There are no segments, create an empty representation to find out what type it is
    vtkSmartPointer<vtkDataObject> masterRepresentation = vtkSmartPointer<vtkDataObject>::Take(
      vtkSegmentationConverterFactory::GetInstance()->ConstructRepresentationObjectByRepresentation(this->MasterRepresentationName));
    return vtkPolyData::SafeDownCast(masterRepresentation) != nullptr;
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
    return vtkOrientedImageData::SafeDownCast(masterRepresentation) != nullptr;
    }
  else
    {
    // There are no segments, create an empty representation to find out what type it is
    vtkSmartPointer<vtkDataObject> masterRepresentation = vtkSmartPointer<vtkDataObject>::Take(
      vtkSegmentationConverterFactory::GetInstance()->ConstructRepresentationObjectByRepresentation(this->MasterRepresentationName));
    return vtkOrientedImageData::SafeDownCast(masterRepresentation) != nullptr;
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
std::string vtkSegmentation::AddEmptySegment(std::string segmentId/*=""*/, std::string segmentName/*=""*/, double* color/*=nullptr*/)
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

  if (this->MasterRepresentationName == vtkSegmentationConverter::GetBinaryLabelmapRepresentationName())
    {
    std::string sharedSegmentId;
    if (this->SegmentIds.size() > 0)
      {
      // Add the empty segment to the first shared labelmap.
      // This is faster when adding a large number of empty segments, but there is likely a heuristic that would provide a better layer to merge into.
      sharedSegmentId = this->SegmentIds[0];
      }

    if (!sharedSegmentId.empty())
      {
      vtkSegment* sharedSegment = this->GetSegment(sharedSegmentId);
      vtkDataObject* dataObject = sharedSegment->GetRepresentation(vtkSegmentationConverter::GetBinaryLabelmapRepresentationName());
      int labelValue = this->GetUniqueLabelValueForSharedLabelmap(sharedSegmentId);
      segment->SetLabelValue(labelValue);
      segment->AddRepresentation(vtkSegmentationConverter::GetBinaryLabelmapRepresentationName(), dataObject);
      vtkOrientedImageData* sharedLabelmap = vtkOrientedImageData::SafeDownCast(dataObject);
      vtkOrientedImageDataResample::CastImageForValue(sharedLabelmap, labelValue);
      }
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
  vtkStringArray* segmentIds /*=nullptr*/, bool computeEffectiveExtent /*=false*/, bool addPadding /*=false*/)
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
  std::vector<std::string> sharedSegmentIDs;
  if (segmentIDs.empty())
    {
    this->GetSegmentIDs(sharedSegmentIDs);
    }
  else
    {
    sharedSegmentIDs = segmentIDs;
    }

  // Get highest resolution reference geometry available in segments
  vtkOrientedImageData* highestResolutionLabelmap = nullptr;
  double lowestSpacing[3] = {1, 1, 1}; // We'll multiply the spacings together to get the voxel size
  for (std::vector<std::string>::iterator segmentIt = sharedSegmentIDs.begin(); segmentIt != sharedSegmentIDs.end(); ++segmentIt)
    {
    vtkSegment* currentSegment = this->GetSegment(*segmentIt);
    if (!currentSegment)
      {
      vtkWarningMacro("DetermineCommonLabelmapGeometry: Segment not found by ID " << (*segmentIt));
      continue;
      }
    vtkOrientedImageData* currentBinaryLabelmap = vtkOrientedImageData::SafeDownCast(
      currentSegment->GetRepresentation(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName()) );
    if (!currentBinaryLabelmap || currentBinaryLabelmap->IsEmpty())
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

    // We were supposed to use the extent from the reference geometry string, however it did  not exist.
    // Instead, calculate extent from effective extent of segments.
    if (extentComputationMode == EXTENT_REFERENCE_GEOMETRY)
      {
      extentComputationMode = EXTENT_UNION_OF_EFFECTIVE_SEGMENTS;
      }
    }

  vtkSmartPointer<vtkOrientedImageData> commonGeometryImage = vtkSmartPointer<vtkOrientedImageData>::New();
  vtkSegmentationConverter::DeserializeImageGeometry(referenceGeometryString, commonGeometryImage, false);

  if (extentComputationMode == EXTENT_UNION_OF_SEGMENTS || extentComputationMode == EXTENT_UNION_OF_EFFECTIVE_SEGMENTS
    || extentComputationMode == EXTENT_UNION_OF_SEGMENTS_PADDED || extentComputationMode == EXTENT_UNION_OF_EFFECTIVE_SEGMENTS_PADDED)
    {
    // Determine extent that contains all segments
    int commonGeometryExtent[6] = { 0, -1, 0, -1, 0, -1 };
    this->DetermineCommonLabelmapExtent(commonGeometryExtent, commonGeometryImage, sharedSegmentIDs,
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
  std::vector<std::string> sharedSegmentIDs;
  if (segmentIDs.empty())
    {
    this->GetSegmentIDs(sharedSegmentIDs);
    }
  else
    {
    sharedSegmentIDs = segmentIDs;
    }

  // Determine extent that contains all segments
  commonGeometryExtent[0] = 0;
  commonGeometryExtent[1] = -1;
  commonGeometryExtent[2] = 0;
  commonGeometryExtent[3] = -1;
  commonGeometryExtent[4] = 0;
  commonGeometryExtent[5] = -1;
  for (std::vector<std::string>::iterator segmentIt = sharedSegmentIDs.begin(); segmentIt != sharedSegmentIDs.end(); ++segmentIt)
    {
    vtkSegment* currentSegment = this->GetSegment(*segmentIt);
    if (!currentSegment)
      {
      vtkWarningMacro("DetermineCommonLabelmapGeometry: Segment not found by ID " << (*segmentIt));
      continue;
      }
    vtkOrientedImageData* currentBinaryLabelmap = vtkOrientedImageData::SafeDownCast(
      currentSegment->GetRepresentation(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName()));
    if (currentBinaryLabelmap==nullptr || currentBinaryLabelmap->IsEmpty())
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
bool vtkSegmentation::SetImageGeometryFromCommonLabelmapGeometry(vtkOrientedImageData* imageData, vtkStringArray* segmentIDs /*=nullptr*/,
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

//----------------------------------------------------------------------------
int vtkSegmentation::GetNumberOfLayers(std::string representationName/*=""*/)
{
  if (representationName.empty())
    {
    representationName = this->MasterRepresentationName;
    }

  vtkNew<vtkCollection> layerObjects;
  this->GetLayerObjects(layerObjects, representationName);
  return layerObjects->GetNumberOfItems();
}

//----------------------------------------------------------------------------
void vtkSegmentation::GetLayerObjects(vtkCollection* layerObjects, std::string representationName/*= ""*/)
{
  if (!layerObjects)
    {
    vtkErrorMacro("GetLayerObjects: Invalid layerObjects!");
    return;
    }
  if (representationName.empty())
    {
    representationName = this->MasterRepresentationName;
    }
  layerObjects->RemoveAllItems();

  int layerCount = 0;
  std::set<vtkDataObject*> objects;
  for (std::string segmentId : this->SegmentIds)
    {
    vtkSegment* segment = this->GetSegment(segmentId);
    vtkDataObject* dataObject = segment->GetRepresentation(representationName);
    if (dataObject && objects.find(dataObject) == objects.end())
      {
      objects.insert(dataObject);
      layerObjects->AddItem(dataObject);
      ++layerCount;
      }
    }
}

//----------------------------------------------------------------------------
int vtkSegmentation::GetLayerIndex(std::string segmentId, std::string representationName/*=""*/)
{
  if (representationName.empty())
    {
    representationName = this->MasterRepresentationName;
    }

  vtkNew<vtkCollection> layerObjects;
  this->GetLayerObjects(layerObjects, representationName);

  vtkSegment* segment = this->GetSegment(segmentId);
  if (!segment)
    {
    vtkErrorMacro("GetLayerIndex: Could not find segment " << segmentId << " in segmentation");
    return -1;
    }
  vtkObject* segmentObject = segment->GetRepresentation(representationName);
  if (!segmentObject)
    {
    return -1;
    }

  for (int i = 0; i < layerObjects->GetNumberOfItems(); ++i)
    {
    if (layerObjects->GetItemAsObject(i) == segmentObject)
      {
      return i;
      }
    }

  return -1;
}

//----------------------------------------------------------------------------
vtkDataObject* vtkSegmentation::GetLayerDataObject(int layer, std::string representationName/*=""*/)
{
  if (representationName.empty())
    {
    representationName = this->MasterRepresentationName;
    }

  vtkNew<vtkCollection> layerObjects;
  this->GetLayerObjects(layerObjects, representationName);

  if (layer >= layerObjects->GetNumberOfItems())
    {
    return nullptr;
    }
  return vtkDataObject::SafeDownCast(layerObjects->GetItemAsObject(layer));
}

//----------------------------------------------------------------------------
std::vector<std::string> vtkSegmentation::GetSegmentIDsForLayer(int layer, std::string representationName/*=""*/)
{
  if (representationName.empty())
    {
    representationName = this->MasterRepresentationName;
    }

  vtkDataObject* dataObject = this->GetLayerDataObject(layer, representationName);
  return this->GetSegmentIDsForDataObject(dataObject, representationName);
}

//----------------------------------------------------------------------------
std::vector<std::string> vtkSegmentation::GetSegmentIDsForDataObject(vtkDataObject* dataObject, std::string representationName/*=""*/)
{
  if (representationName.empty())
    {
    representationName = this->MasterRepresentationName;
    }

  std::vector<std::string> segmentIds;
  for (std::string segmentID : this->SegmentIds)
    {
    vtkSegment* segment = this->GetSegment(segmentID);
    vtkDataObject* representationObject = segment->GetRepresentation(representationName);
    if (dataObject == representationObject)
      {
      segmentIds.push_back(segmentID);
      }
    }
  return segmentIds;
}

//----------------------------------------------------------------------------
void vtkSegmentation::CollapseBinaryLabelmaps(bool forceToSingleLayer/*=false*/)
{
  std::string labelmapRepresentationName = vtkSegmentationConverter::GetBinaryLabelmapRepresentationName();
  int numberOfLayers = this->GetNumberOfLayers(labelmapRepresentationName);
  if (numberOfLayers <= 1)
    {
    // No need to try to merge, the minimum number of labelmaps has been reached.
    return;
    }

  if (forceToSingleLayer)
    {
    // If the merge is forced to a single layer, segments can be overwritten.
    std::vector<std::string> segmentIds;
    this->GetSegmentIDs(segmentIds);
    this->MergeSegmentLabelmaps(segmentIds);
    return;
    }

  typedef std::pair<vtkSmartPointer<vtkOrientedImageData>, std::vector<std::string> > LayerType;
  typedef std::vector<LayerType> LayerListType;
  std::map<std::string, int> newLabelmapValues;
  LayerListType newLayers;
  for (int i = 0; i < numberOfLayers; ++i)
    {
    vtkOrientedImageData* layerLabelmap = vtkOrientedImageData::SafeDownCast(this->GetLayerDataObject(i, labelmapRepresentationName));
    std::vector<std::string> currentLayerSegmentIds = this->GetSegmentIDsForLayer(i, labelmapRepresentationName);
    if (i == 0)
      {
      vtkSmartPointer<vtkOrientedImageData> newLabelmap = vtkSmartPointer<vtkOrientedImageData>::New();
      newLabelmap->DeepCopy(layerLabelmap);
      newLayers.push_back( std::make_pair(newLabelmap, currentLayerSegmentIds));
      for (std::string currentSegmentId : currentLayerSegmentIds)
        {
        vtkSegment* segment = this->GetSegment(currentSegmentId);
        newLabelmapValues[currentSegmentId] = segment->GetLabelValue();
        }
      continue;
      }

    for (std::string currentSegmentId : currentLayerSegmentIds)
      {
      vtkSegment* currentSegment = this->GetSegment(currentSegmentId);
      vtkOrientedImageData* currentLabelmap = vtkOrientedImageData::SafeDownCast(currentSegment->GetRepresentation(labelmapRepresentationName));

      vtkSmartPointer<vtkOrientedImageData> thresholdedLabelmap = vtkSmartPointer<vtkOrientedImageData>::New();
      if (currentLabelmap)
        {
        vtkNew<vtkImageThreshold> imageThreshold;
        imageThreshold->SetInputData(currentLabelmap);
        imageThreshold->ThresholdBetween(currentSegment->GetLabelValue(), currentSegment->GetLabelValue());
        imageThreshold->SetInValue(1);
        imageThreshold->SetOutValue(0);
        imageThreshold->SetOutputScalarTypeToUnsignedChar();
        imageThreshold->Update();

        thresholdedLabelmap->ShallowCopy(imageThreshold->GetOutput());
        thresholdedLabelmap->CopyDirections(currentLabelmap);

        int effectiveExtent[6] = { 0 };
        vtkOrientedImageDataResample::CalculateEffectiveExtent(thresholdedLabelmap, effectiveExtent);

        vtkNew<vtkOrientedImageData> referenceImage;
        referenceImage->ShallowCopy(thresholdedLabelmap);
        referenceImage->SetExtent(effectiveExtent);

        vtkOrientedImageDataResample::ResampleOrientedImageToReferenceOrientedImage(thresholdedLabelmap, referenceImage, thresholdedLabelmap);
        }

      bool shared = false;
      int layerCount = 0;
      for (LayerType newLayer : newLayers)
        {
        vtkOrientedImageData* newLayerLabelmap = newLayer.first;
        bool safeToMerge = true;
        if (currentLabelmap)
          {
          safeToMerge = !vtkOrientedImageDataResample::IsLabelInMask(newLayerLabelmap, thresholdedLabelmap);
          }

        if (safeToMerge)
          {
          shared = true;
          int labelValue = this->GetUniqueLabelValueForSharedLabelmap(newLayerLabelmap);
          for (std::string layerSegmentID : newLayer.second)
            {
            // GetUniqueLabelValueForSharedLabelmap(vtkOrientedImageData) only checks the existing scalars in the labelmap.
            // If there are shared labelmaps in the new layer that do not have filled voxels in the labelmap, then the result of
            // GetUniqueLabelValueForSharedLabelmap may not be unique. Instead, we compare the label value of all of the segments in the
            // new layer to make sure the value is unique.
            int existingValue = newLabelmapValues[layerSegmentID];
            labelValue = std::max(labelValue, existingValue + 1);
            }

          if (currentLabelmap)
            {
            int extent[6] = { 0 };
            currentLabelmap->GetExtent(extent);
            // If the current labelmap is empty, we don't need to merge the image.
            if (extent[0] <= extent[1] || extent[2] <= extent[3] || extent[4] <= extent[5])
              {
              vtkOrientedImageDataResample::CastImageForValue(newLayerLabelmap, labelValue);
              vtkOrientedImageDataResample::ResampleOrientedImageToReferenceOrientedImage(thresholdedLabelmap,
                newLayerLabelmap, thresholdedLabelmap, false, true);
              vtkOrientedImageDataResample::MergeImage(newLayerLabelmap, thresholdedLabelmap, newLayerLabelmap,
                vtkOrientedImageDataResample::OPERATION_MASKING, thresholdedLabelmap->GetExtent(), 0.0, labelValue); // Add segment to new layer
              }
            }
          newLayers[layerCount].second.push_back(currentSegmentId);
          newLabelmapValues[currentSegmentId] = labelValue;
          break;
          }
        ++layerCount;
        }
      if (!shared)
        {
        newLayers.push_back(std::make_pair(thresholdedLabelmap, std::vector<std::string>({ currentSegmentId })));
        newLabelmapValues[currentSegmentId] = 1;
        }
      }
    }

  // Although the labelmaps have been collapsed, the individual segment contents should not have been modified.
  // Don't invoke a MasterRepresentation modified event, since that would invalidate the derived representations.
  bool wasMasterRepresentationModifiedEnabled = this->SetMasterRepresentationModifiedEnabled(false);
  for (LayerType newLayer : newLayers)
    {
    for (std::string segmentId : newLayer.second)
      {
      vtkSegment* segment = this->GetSegment(segmentId);
      segment->AddRepresentation(labelmapRepresentationName, newLayer.first);
      segment->SetLabelValue(newLabelmapValues[segmentId]);
      }
    newLayer.first->Modified();
    }
  this->SetMasterRepresentationModifiedEnabled(wasMasterRepresentationModifiedEnabled);
}

//---------------------------------------------------------------------------
void vtkSegmentation::CopySegment(vtkSegment* destination, vtkSegment* source, vtkSegment* baseline,
  std::map<vtkDataObject*, vtkDataObject*>& cachedRepresentations)
{
  destination->RemoveAllRepresentations();
  destination->DeepCopyMetadata(source);

  // Copy representations
  std::vector<std::string> representationNames;
  source->GetContainedRepresentationNames(representationNames);
  for (std::vector<std::string>::iterator representationNameIt = representationNames.begin();
    representationNameIt != representationNames.end(); ++representationNameIt)
    {
    vtkDataObject* sourceRepresentation = source->GetRepresentation(*representationNameIt);
    if (cachedRepresentations.find(sourceRepresentation) != cachedRepresentations.end())
      {
      // If the same object (i.e. shared labelmap) has already been cached from a previous segment, then point to that
      // object instead. No need to perform copy.
      destination->AddRepresentation(*representationNameIt, cachedRepresentations[sourceRepresentation]);
      continue;
      }

    vtkDataObject* baselineRepresentation = nullptr;
    if (baseline)
      {
      baselineRepresentation = baseline->GetRepresentation(*representationNameIt);
      }
    // Shallow-copy from baseline if it's up-to-date, otherwise deep-copy from source
    if (baselineRepresentation != nullptr
      && baselineRepresentation->GetMTime() > sourceRepresentation->GetMTime())
      {
      // we already have an up-to-date copy in the baseline, so reuse that
      destination->AddRepresentation(*representationNameIt, baselineRepresentation);
      cachedRepresentations[sourceRepresentation] = baselineRepresentation;
      }
    else
      {
      vtkDataObject* representationCopy =
        vtkSegmentationConverterFactory::GetInstance()->ConstructRepresentationObjectByClass(sourceRepresentation->GetClassName());
      if (!representationCopy)
        {
        vtkErrorWithObjectMacro(nullptr, "DeepCopy: Unable to construct representation type class '" << sourceRepresentation->GetClassName() << "'");
        continue;
        }
      representationCopy->DeepCopy(sourceRepresentation);
      destination->AddRepresentation(*representationNameIt, representationCopy);
      cachedRepresentations[sourceRepresentation] = representationCopy;
      representationCopy->Delete(); // this representation is now owned by the segment
      }
    }
}
