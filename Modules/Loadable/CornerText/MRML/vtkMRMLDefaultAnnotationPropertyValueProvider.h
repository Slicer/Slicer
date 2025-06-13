/*==============================================================================

  Program: 3D Slicer

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#ifndef __vtkMRMLDefaultAnnotationPropertyValueProvider_h
#define __vtkMRMLDefaultAnnotationPropertyValueProvider_h

// MRMLCore includes
#include "vtkMRMLAbstractAnnotationPropertyValueProvider.h"

// CornerText includes
#include "vtkMRMLApplicationLogic.h"
#include "vtkSlicerCornerTextModuleMRMLExport.h"

// VTK includes
#include <vtkWeakPointer.h>

/// \brief The default property value provider implemented for the CornerText
/// loadable module.
///
/// Implements heuristics for providing property values based on the names of volume
/// nodes. This is intended to replicate the behavior of 'DataProbe', a Scripted
/// Loadable module
///
class VTK_SLICER_CORNERTEXT_MODULE_MRML_EXPORT vtkMRMLDefaultAnnotationPropertyValueProvider
  : public vtkMRMLAbstractAnnotationPropertyValueProvider
{

public:

  static vtkMRMLDefaultAnnotationPropertyValueProvider* New();
  vtkTypeMacro(vtkMRMLDefaultAnnotationPropertyValueProvider, vtkMRMLAbstractAnnotationPropertyValueProvider);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  bool CanProvideValueForPropertyName(const std::string& propertyName) override;
  std::string GetValueForPropertyName(const std::string& propertyName,
                                      const XMLTagAttributes& attributes,
                                      vtkMRMLSliceNode*) override;
  std::unordered_set<std::string> GetSupportedProperties() override;

  vtkGetObjectMacro(AppLogic, vtkMRMLApplicationLogic);
  void SetAppLogic(vtkMRMLApplicationLogic* appLogic);

protected:

  vtkMRMLDefaultAnnotationPropertyValueProvider() = default;
  ~vtkMRMLDefaultAnnotationPropertyValueProvider() = default;

private:

  vtkMRMLDefaultAnnotationPropertyValueProvider(const vtkMRMLDefaultAnnotationPropertyValueProvider&) = delete;
  void operator=(const vtkMRMLDefaultAnnotationPropertyValueProvider&) = delete;

  const std::unordered_set<std::string> RegisteredProperties = {
    "VolumeName",
    "SlabReconstructionThickness",
    "SlabReconstructionType",
  };

  vtkWeakPointer<vtkMRMLApplicationLogic> AppLogic;
};

#endif
