/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerMeasurementsModuleWidget.h"
#include "ui_qSlicerMeasurementsModuleWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Measurements
class qSlicerMeasurementsModuleWidgetPrivate: public Ui_qSlicerMeasurementsModuleWidget
{
public:
};

//-----------------------------------------------------------------------------
CTK_CONSTRUCTOR_1_ARG_CPP(qSlicerMeasurementsModuleWidget, QWidget*);

//-----------------------------------------------------------------------------
qSlicerMeasurementsModuleWidget::~qSlicerMeasurementsModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerMeasurementsModuleWidget::setup()
{
  Q_D(qSlicerMeasurementsModuleWidget);
  d->setupUi(this);
}
