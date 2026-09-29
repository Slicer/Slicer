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

#ifndef __vtkSlicerMarkupsWriter_h
#define __vtkSlicerMarkupsWriter_h

// Slicer includes
#include "vtkMRMLNodeWriter.h"

#include "vtkSlicerMarkupsModuleLogicExport.h"

class vtkMRMLStorableNode;

/// Writer of markups nodes. Markups can be written in json (.mrk.json) or fiducial CSV (.fcsv) format.
class VTK_SLICER_MARKUPS_MODULE_LOGIC_EXPORT vtkSlicerMarkupsWriter : public vtkMRMLNodeWriter
{
public:
  static vtkSlicerMarkupsWriter* New();
  vtkTypeMacro(vtkSlicerMarkupsWriter, vtkMRMLNodeWriter);

  /// Supported formats of json and fcsv storage nodes.
  std::vector<std::string> GetNameFiltersForObject(vtkObject* object) override;

  /// Write the node using the storage node that matches the file extension.
  bool Write(vtkMRMLIOProperties* properties) override;

protected:
  vtkSlicerMarkupsWriter();
  ~vtkSlicerMarkupsWriter() override;

  /// Make sure the storable node uses a storage node of the specified class.
  void SetStorageNodeClass(vtkMRMLStorableNode* storableNode, const std::string& storageNodeClassName);

private:
  vtkSlicerMarkupsWriter(const vtkSlicerMarkupsWriter&) = delete;
  void operator=(const vtkSlicerMarkupsWriter&) = delete;
};

#endif
