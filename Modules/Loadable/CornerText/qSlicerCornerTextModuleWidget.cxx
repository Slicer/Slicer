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

// Qt includes
#include <QDebug>

// Slicer includes
#include "qSlicerCornerTextModuleWidget.h"
#include "ui_qSlicerCornerTextModuleWidget.h"
#include "vtkMRMLLayoutLogic.h"

#include <qSlicerApplication.h>
#include <qSlicerLayoutManager.h>
#include <vtkSlicerApplicationLogic.h>

//-----------------------------------------------------------------------------
class qSlicerCornerTextModuleWidgetPrivate: public Ui_qSlicerCornerTextModuleWidget
{
public:
  qSlicerCornerTextModuleWidgetPrivate();
};

//-----------------------------------------------------------------------------
// qSlicerCornerTextModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerCornerTextModuleWidgetPrivate::qSlicerCornerTextModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerCornerTextModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerCornerTextModuleWidget::qSlicerCornerTextModuleWidget(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerCornerTextModuleWidgetPrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerCornerTextModuleWidget::~qSlicerCornerTextModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerCornerTextModuleWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qSlicerCornerTextModuleWidget);

  this->Superclass::setMRMLScene(scene);

  d->Settings->setMRMLScene(scene);
}

//-----------------------------------------------------------------------------
vtkMRMLCornerTextLogic* qSlicerCornerTextModuleWidget::cornerTextLogic()
{
 return vtkMRMLCornerTextLogic::SafeDownCast(this->logic());
}

//-----------------------------------------------------------------------------
void qSlicerCornerTextModuleWidget::setup()
{
  Q_D(qSlicerCornerTextModuleWidget);
  d->setupUi(this);

  this->Superclass::setup();

  d->Settings->setLayoutLogic(
        qSlicerApplication::application()->layoutManager()->layoutLogic());

  d->Settings->setAndObserveCornerTextLogic(
        qSlicerApplication::application()->applicationLogic()->GetCornerTextLogic());
}
