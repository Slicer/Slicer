/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// Qt includes
#include <QDebug>

// SlicerQt includes
#include "qSlicerLoadableExtensionTemplateModuleWidget.h"
#include "ui_qSlicerLoadableExtensionTemplateModule.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerLoadableExtensionTemplateModuleWidgetPrivate: public Ui_qSlicerLoadableExtensionTemplateModule
{
public:
  qSlicerLoadableExtensionTemplateModuleWidgetPrivate();
};

//-----------------------------------------------------------------------------
// qSlicerLoadableExtensionTemplateModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerLoadableExtensionTemplateModuleWidgetPrivate::qSlicerLoadableExtensionTemplateModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerLoadableExtensionTemplateModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerLoadableExtensionTemplateModuleWidget::qSlicerLoadableExtensionTemplateModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerLoadableExtensionTemplateModuleWidgetPrivate )
{
}

//-----------------------------------------------------------------------------
qSlicerLoadableExtensionTemplateModuleWidget::~qSlicerLoadableExtensionTemplateModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerLoadableExtensionTemplateModuleWidget::setup()
{
  Q_D(qSlicerLoadableExtensionTemplateModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
}

