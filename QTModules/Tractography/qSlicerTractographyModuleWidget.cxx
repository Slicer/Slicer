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
#include "qSlicerTractographyModuleWidget.h"
#include "ui_qSlicerTractographyModule.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Tractography
class qSlicerTractographyModuleWidgetPrivate: public Ui_qSlicerTractographyModule
{
public:
  qSlicerTractographyModuleWidgetPrivate();
};

//-----------------------------------------------------------------------------
// qSlicerTractographyModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerTractographyModuleWidgetPrivate::qSlicerTractographyModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerTractographyModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerTractographyModuleWidget::qSlicerTractographyModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerTractographyModuleWidgetPrivate )
{
}

//-----------------------------------------------------------------------------
qSlicerTractographyModuleWidget::~qSlicerTractographyModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerTractographyModuleWidget::setup()
{
  Q_D(qSlicerTractographyModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
}

