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

#ifndef __vtkSegment_h
#define __vtkSegment_h

// VTK includes
#include <vtkObject.h>
#include <vtkSmartPointer.h>
#include <vtkDataObject.h>

// STD includes
#include <vector>
#include <map>

// Segmentation includes
#include "vtkSegmentationCoreConfigure.h"

/// \ingroup SegmentationCore
class vtkSegmentationCore_EXPORT vtkSegment : public vtkObject
{
  typedef std::map<std::string, vtkSmartPointer<vtkDataObject> > RepresentationMap;

public:
  static const double SEGMENT_COLOR_VALUE_INVALID[4];

  static vtkSegment* New();
  vtkTypeMacro(vtkSegment, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// Set attributes from name/value pairs
  virtual void ReadXMLAttributes(const char** atts);

  /// Write this object's information to a MRML file in XML format.
  void WriteXML(ostream& of, int nIndent);

  /// Deep copy one segment into another
  virtual void DeepCopy(vtkSegment* aSegment);

  /// Get bounding box in global RAS in the form (xmin,xmax, ymin,ymax, zmin,zmax).
  virtual void GetBounds(double bounds[6]);

  /// Returns true if the node (default behavior) or the internal data are modified
  /// since read/written.
  /// Note: The MTime of the internal data is used to know if it has been modified.
  /// So if you invoke one of the data modified events without calling Modified() on the
  /// internal data, GetModifiedSinceRead() won't return true.
  /// \sa vtkMRMLStorableNode::GetModifiedSinceRead()
  bool GetModifiedSinceRead(const vtkTimeStamp& storedTime);

  /// Utility function to get extended bounds
  /// \param partialBounds New bounds with which the globalBounds will be extended if necessary
  /// \param globalBounds Global bounds to be extended with partialBounds
  static void ExtendBounds(double partialBounds[6], double globalBounds[6]);

  /// Get representation of a given type. This class is not responsible for conversion, only storage!
  /// \param name Representation name
  /// \return The specified representation object, NULL if not present
  vtkDataObject* GetRepresentation(std::string name);

  /// Add representation
  void AddRepresentation(std::string type, vtkDataObject* representation);

  /// Remove representation of given type
  void RemoveRepresentation(std::string name);

  /// Remove all representations except one if specified. Fires only one Modified event
  /// \param exceptionRepresentationName Exception name that will not be removed (e.g. invalidate non-master representations), empty by default
  void RemoveAllRepresentations(std::string exceptionRepresentationName="");

  /// Set/add tag
  /// \param emitModified Flag determining whether modified event is emitted. As usually tags are
  ///   used to store metadata that do not trigger display or other changes (so default is false),
  ///   modified event is not necessary. For the cases it is, the flag can be set to true.
  void SetTag(std::string tag, std::string value, bool emitModified=false);
  /// Set/add integer tag
  void SetTag(std::string tag, int value, bool emitModified=false);

  /// Remove tag
  void RemoveTag(std::string tag);

  /// Get tag
  /// \param tag Name of requested tag
  /// \param value Output argument for the value of the tag if found
  /// \return True if tag is found, false otherwise
  bool GetTag(std::string tag, std::string &value);
  /// Determine if a tag is present
  bool HasTag(std::string tag);
  /// Get tags
  void GetTags(std::map<std::string,std::string> &tags);

  /// Get representation names present in this segment in an output string vector
  void GetContainedRepresentationNames(std::vector<std::string>& representationNames);

public:
  vtkGetStringMacro(Name);
  vtkSetStringMacro(Name);

  vtkGetVector3Macro(DefaultColor, double);
  vtkSetVector3Macro(DefaultColor, double);

protected:
  vtkSegment();
  ~vtkSegment();
  void operator=(const vtkSegment&);

protected:
  /// Stored representations. Map from type string to data object
  RepresentationMap Representations;

  /// Name (e.g. segment label in DICOM Segmentation Object)
  /// This is the default identifier of the segment within segmentation, so needs to be unique within a segmentation
  char* Name;

  /// Default color
  /// Called default because this is only used initially indicating the original color of the segment,
  /// but then copies are made for display and this member variable has no effect on it afterwards.
  double DefaultColor[3];

  /// Tags (for grouping and selection)
  std::map<std::string,std::string> Tags;
};

#endif // __vtkSegment_h
