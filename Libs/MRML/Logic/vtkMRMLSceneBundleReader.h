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

#ifndef __vtkMRMLSceneBundleReader_h
#define __vtkMRMLSceneBundleReader_h

#include "vtkMRMLFileReader.h"

/// Reader of Medical Reality Bundle (MRB) scene files.
///
/// Properties: "fileName", "clear" (bool, clear the scene before loading).
///
/// qSlicerSceneBundleReader is the Qt interface of this class, kept for backward compatibility.
class VTK_MRML_LOGIC_EXPORT vtkMRMLSceneBundleReader : public vtkMRMLFileReader
{
public:
  static vtkMRMLSceneBundleReader* New();
  vtkTypeMacro(vtkMRMLSceneBundleReader, vtkMRMLFileReader);

  bool Load(vtkMRMLIOProperties* properties) override;

protected:
  vtkMRMLSceneBundleReader();
  ~vtkMRMLSceneBundleReader() override;

private:
  vtkMRMLSceneBundleReader(const vtkMRMLSceneBundleReader&) = delete;
  void operator=(const vtkMRMLSceneBundleReader&) = delete;
};

#endif
