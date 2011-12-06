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

// Qt includes

// SlicerQt includes
#include "qSlicerModuleTemplateModuleWidget.h"
#include "ui_qSlicerModuleTemplateModule.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ModuleTemplate
class qSlicerModuleTemplateModuleWidgetPrivate: public Ui_qSlicerModuleTemplateModule
{
public:
  qSlicerModuleTemplateModuleWidgetPrivate();
};

//-----------------------------------------------------------------------------
// qSlicerModuleTemplateModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerModuleTemplateModuleWidgetPrivate::qSlicerModuleTemplateModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerModuleTemplateModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerModuleTemplateModuleWidget::qSlicerModuleTemplateModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerModuleTemplateModuleWidgetPrivate )
{
}

//-----------------------------------------------------------------------------
qSlicerModuleTemplateModuleWidget::~qSlicerModuleTemplateModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerModuleTemplateModuleWidget::setup()
{
  Q_D(qSlicerModuleTemplateModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
}

