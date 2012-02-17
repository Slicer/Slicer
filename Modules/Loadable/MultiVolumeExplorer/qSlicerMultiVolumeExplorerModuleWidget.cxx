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
#include "qSlicerMultiVolumeExplorerModuleWidget.h"
#include "ui_qSlicerMultiVolumeExplorerModule.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerMultiVolumeExplorerModuleWidgetPrivate: public Ui_qSlicerMultiVolumeExplorerModule
{
public:
  qSlicerMultiVolumeExplorerModuleWidgetPrivate();
};

//-----------------------------------------------------------------------------
// qSlicerMultiVolumeExplorerModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerMultiVolumeExplorerModuleWidgetPrivate::qSlicerMultiVolumeExplorerModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerMultiVolumeExplorerModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerMultiVolumeExplorerModuleWidget::qSlicerMultiVolumeExplorerModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerMultiVolumeExplorerModuleWidgetPrivate )
{
}

//-----------------------------------------------------------------------------
qSlicerMultiVolumeExplorerModuleWidget::~qSlicerMultiVolumeExplorerModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerMultiVolumeExplorerModuleWidget::setup()
{
  Q_D(qSlicerMultiVolumeExplorerModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
}

