/*==============================================================================

  Program: 3D Slicer

  Copyright(c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#ifndef __vtkMRMLTranslator_h
#define __vtkMRMLTranslator_h

// MRML includes
#include "vtkMRML.h"

// VTK includes
#include <vtkObject.h>

// STD includes
#include <string>

/// \brief Base class for localization of messages that may be displayed to users.
///
/// This base class keeps messages unchanged. Sub-classes must implement actual translation.
class VTK_MRML_EXPORT vtkMRMLTranslator : public vtkObject
{
public:
  vtkTypeMacro(vtkMRMLTranslator, vtkObject);

  /// Default translation function that returns the sourceText without any change.
  /// This method must be overridden in derived classes.
  virtual std::string Translate(const char* context,
                                const char* sourceText,
                                const char* disambiguation = nullptr,
                                int n = -1) = 0;

protected:
  vtkMRMLTranslator() = default;
  ~vtkMRMLTranslator() override = default;
  vtkMRMLTranslator(const vtkMRMLTranslator&) = delete;
  void operator=(const vtkMRMLTranslator&) = delete;
};

#endif
