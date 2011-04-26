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
#include "qSlicerExtensionTemplateModuleWidget.h"
#include "ui_qSlicerExtensionTemplateModule.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerExtensionTemplateModuleWidgetPrivate: public Ui_qSlicerExtensionTemplateModule
{
public:
  qSlicerExtensionTemplateModuleWidgetPrivate();
};

//-----------------------------------------------------------------------------
// qSlicerExtensionTemplateModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerExtensionTemplateModuleWidgetPrivate::qSlicerExtensionTemplateModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerExtensionTemplateModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerExtensionTemplateModuleWidget::qSlicerExtensionTemplateModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerExtensionTemplateModuleWidgetPrivate )
{
}

//-----------------------------------------------------------------------------
qSlicerExtensionTemplateModuleWidget::~qSlicerExtensionTemplateModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerExtensionTemplateModuleWidget::setup()
{
  Q_D(qSlicerExtensionTemplateModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
}

