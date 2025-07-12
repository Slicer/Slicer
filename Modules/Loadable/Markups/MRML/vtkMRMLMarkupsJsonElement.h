/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#ifndef __vtkMRMLMarkupsJsonElement_h
#define __vtkMRMLMarkupsJsonElement_h

#include "vtkSlicerMarkupsModuleMRMLExport.h"
#include "vtkMRMLJsonElement.h"

/// \brief Represents a json object or list.
/// \deprecated This class is empty and kept only for backward compatibility.
/// The implementation has been moved to core MRML class vtkMRMLJsonElement.
class VTK_SLICER_MARKUPS_MODULE_MRML_EXPORT vtkMRMLMarkupsJsonElement : public vtkMRMLJsonElement
{
public:
  static vtkMRMLMarkupsJsonElement* New();
  vtkTypeMacro(vtkMRMLMarkupsJsonElement, vtkMRMLJsonElement);

protected:
  vtkMRMLMarkupsJsonElement() { vtkWarningMacro("This class is deprecated. Use vtkMRMLJsonElement instead."); }
  ~vtkMRMLMarkupsJsonElement() override = default;

private:
  vtkMRMLMarkupsJsonElement(const vtkMRMLMarkupsJsonElement&) = delete;
  void operator=(const vtkMRMLMarkupsJsonElement&) = delete;
};

/// \brief Reads JSON data from file.
/// \deprecated This class is empty and kept only for backward compatibility.
/// The implementation has been moved to core MRML class vtkMRMLJsonReader.
class VTK_SLICER_MARKUPS_MODULE_MRML_EXPORT vtkMRMLMarkupsJsonReader : public vtkMRMLJsonReader
{
public:
  static vtkMRMLMarkupsJsonReader* New();
  vtkTypeMacro(vtkMRMLMarkupsJsonReader, vtkMRMLJsonReader);

protected:
  vtkMRMLMarkupsJsonReader() { vtkWarningMacro("This class is deprecated. Use vtkMRMLJsonReader instead."); }
  ~vtkMRMLMarkupsJsonReader() override = default;

private:
  vtkMRMLMarkupsJsonReader(const vtkMRMLMarkupsJsonReader&) = delete;
  void operator=(const vtkMRMLMarkupsJsonReader&) = delete;
};

/// \brief Writes JSON data to file.
/// \deprecated This class is empty and kept only for backward compatibility.
/// The implementation has been moved to core MRML class vtkMRMLJsonWriter.
class VTK_SLICER_MARKUPS_MODULE_MRML_EXPORT vtkMRMLMarkupsJsonWriter : public vtkMRMLJsonWriter
{
public:
  static vtkMRMLMarkupsJsonWriter* New();
  vtkTypeMacro(vtkMRMLMarkupsJsonWriter, vtkMRMLJsonWriter);

protected:
  vtkMRMLMarkupsJsonWriter() { vtkWarningMacro("This class is deprecated. Use vtkMRMLJsonWriter instead."); }
  ~vtkMRMLMarkupsJsonWriter() override = default;

private:
  vtkMRMLMarkupsJsonWriter(const vtkMRMLMarkupsJsonWriter&) = delete;
  void operator=(const vtkMRMLMarkupsJsonWriter&) = delete;
};

#endif
