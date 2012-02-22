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
#include "qSlicerMultiVolumeRenderingModuleWidget.h"
#include "ui_qSlicerMultiVolumeRenderingModule.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_MultiVolumeRendering
class qSlicerMultiVolumeRenderingModuleWidgetPrivate: public Ui_qSlicerMultiVolumeRenderingModule
{
public:
  qSlicerMultiVolumeRenderingModuleWidgetPrivate();
};

//-----------------------------------------------------------------------------
// qSlicerMultiVolumeRenderingModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerMultiVolumeRenderingModuleWidgetPrivate::qSlicerMultiVolumeRenderingModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerMultiVolumeRenderingModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerMultiVolumeRenderingModuleWidget::qSlicerMultiVolumeRenderingModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerMultiVolumeRenderingModuleWidgetPrivate )
{
}

//-----------------------------------------------------------------------------
qSlicerMultiVolumeRenderingModuleWidget::~qSlicerMultiVolumeRenderingModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerMultiVolumeRenderingModuleWidget::setup()
{
  Q_D(qSlicerMultiVolumeRenderingModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
}

