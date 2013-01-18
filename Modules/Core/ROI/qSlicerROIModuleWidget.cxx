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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// SlicerQt includes
#include "qSlicerROIModuleWidget.h"
#include "ui_qSlicerROIModuleWidget.h"

//-----------------------------------------------------------------------------
class qSlicerROIModuleWidgetPrivate: public Ui_qSlicerROIModuleWidget
{
public:
};

//-----------------------------------------------------------------------------
qSlicerROIModuleWidget::qSlicerROIModuleWidget(QWidget* parentWidget)
  :qSlicerAbstractModuleWidget(parentWidget)
  , d_ptr(new qSlicerROIModuleWidgetPrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerROIModuleWidget::~qSlicerROIModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerROIModuleWidget::setup()
{
  Q_D(qSlicerROIModuleWidget);
  d->setupUi(this);
}
