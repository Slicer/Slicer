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

#ifndef __qSlicerCornerTextModuleWidget_h
#define __qSlicerCornerTextModuleWidget_h

// Slicer includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerCornerTextModuleExport.h"
#include "vtkMRMLAbstractLogic.h"

class qSlicerCornerTextModuleWidgetPrivate;
class vtkMRMLNode;
class vtkMRMLCornerTextLogic;

class Q_SLICER_QTMODULES_CORNERTEXT_EXPORT qSlicerCornerTextModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerCornerTextModuleWidget(QWidget *parent=0);
  virtual ~qSlicerCornerTextModuleWidget();

  /// To grab the Layout and CornerText logic (internal reasons)
  void setMRMLScene(vtkMRMLScene* scene) override;
  vtkMRMLCornerTextLogic* cornerTextLogic();

public slots:

protected:
  QScopedPointer<qSlicerCornerTextModuleWidgetPrivate> d_ptr;

  void setup() override;

private:
  Q_DECLARE_PRIVATE(qSlicerCornerTextModuleWidget);
  Q_DISABLE_COPY(qSlicerCornerTextModuleWidget);
};

#endif
