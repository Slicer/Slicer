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

#ifndef __vtkSlicerTextsReader_h
#define __vtkSlicerTextsReader_h

// Slicer includes
#include "vtkMRMLFileReader.h"

#include "vtkSlicerTextsModuleLogicExport.h"

/// Reader of text files into text nodes.
class VTK_SLICER_TEXTS_MODULE_LOGIC_EXPORT vtkSlicerTextsReader : public vtkMRMLFileReader
{
public:
  static vtkSlicerTextsReader* New();
  vtkTypeMacro(vtkSlicerTextsReader, vtkMRMLFileReader);

  bool Load(vtkMRMLIOProperties* properties) override;

protected:
  vtkSlicerTextsReader();
  ~vtkSlicerTextsReader() override;

private:
  vtkSlicerTextsReader(const vtkSlicerTextsReader&) = delete;
  void operator=(const vtkSlicerTextsReader&) = delete;
};

#endif
