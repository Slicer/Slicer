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

#ifndef __qSlicerCornerTextDICOMAnnotationPropertyValueProvider_h
#define __qSlicerCornerTextDICOMAnnotationPropertyValueProvider_h

// Qt includes
#include <QObject>

// MRMLCore includes
#include <vtkMRMLAbstractAnnotationPropertyValueProvider.h>
#include <vtkMRMLApplicationLogic.h>
#include <vtkMRMLSliceNode.h>

// CornerText includes
#include "qSlicerCornerTextModuleExport.h"

class qSlicerCornerTextDICOMAnnotationPropertyValueProviderPrivate;

/// \brief The DICOM property value provider implemented for the CornerText
/// loadable module.
///
/// Implements heuristics for providing property values based on the names of volume
/// nodes. This is intended to replicate the behavior of 'DataProbe', a Scripted
/// Loadable module
///
class Q_SLICER_QTMODULES_CORNERTEXT_EXPORT qSlicerCornerTextDICOMAnnotationPropertyValueProvider
  : public vtkMRMLAbstractAnnotationPropertyValueProvider
{

public:

  static qSlicerCornerTextDICOMAnnotationPropertyValueProvider* New();

  qSlicerCornerTextDICOMAnnotationPropertyValueProvider();
  virtual ~qSlicerCornerTextDICOMAnnotationPropertyValueProvider();

  typedef vtkMRMLAbstractAnnotationPropertyValueProvider Superclass;

  bool CanProvideValueForPropertyName(const std::string& propertyName) override;
  std::string GetValueForPropertyName(const std::string& propertyName,
                                      const XMLTagAttributes& attributes,
                                      vtkMRMLSliceNode*) override;
  std::unordered_set<std::string> GetSupportedProperties() override;

protected:
  QScopedPointer<qSlicerCornerTextDICOMAnnotationPropertyValueProviderPrivate> d_ptr;

private:

  Q_DECLARE_PRIVATE(qSlicerCornerTextDICOMAnnotationPropertyValueProvider);
  Q_DISABLE_COPY(qSlicerCornerTextDICOMAnnotationPropertyValueProvider);
};

#endif
