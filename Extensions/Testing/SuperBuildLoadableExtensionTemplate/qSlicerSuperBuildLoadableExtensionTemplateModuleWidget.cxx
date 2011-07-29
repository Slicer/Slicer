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
#include <QDebug>

// SlicerQt includes
#include "qSlicerSuperBuildLoadableExtensionTemplateModuleWidget.h"
#include "ui_qSlicerSuperBuildLoadableExtensionTemplateModule.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerSuperBuildLoadableExtensionTemplateModuleWidgetPrivate: public Ui_qSlicerSuperBuildLoadableExtensionTemplateModule
{
public:
  qSlicerSuperBuildLoadableExtensionTemplateModuleWidgetPrivate();
};

//-----------------------------------------------------------------------------
// qSlicerSuperBuildLoadableExtensionTemplateModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerSuperBuildLoadableExtensionTemplateModuleWidgetPrivate::qSlicerSuperBuildLoadableExtensionTemplateModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerSuperBuildLoadableExtensionTemplateModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerSuperBuildLoadableExtensionTemplateModuleWidget::qSlicerSuperBuildLoadableExtensionTemplateModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerSuperBuildLoadableExtensionTemplateModuleWidgetPrivate )
{
}

//-----------------------------------------------------------------------------
qSlicerSuperBuildLoadableExtensionTemplateModuleWidget::~qSlicerSuperBuildLoadableExtensionTemplateModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerSuperBuildLoadableExtensionTemplateModuleWidget::setup()
{
  Q_D(qSlicerSuperBuildLoadableExtensionTemplateModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
}

