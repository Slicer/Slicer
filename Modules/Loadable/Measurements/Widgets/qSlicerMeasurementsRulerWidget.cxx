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

// Qt includes
#include <QDebug>

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerMeasurementsRulerWidget.h"
#include "ui_qSlicerMeasurementsRulerWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Measurements
class qSlicerMeasurementsRulerWidgetPrivate : public Ui_qSlicerMeasurementsRulerWidget
{
public:
  qSlicerMeasurementsRulerWidgetPrivate()
    {
    }
};

//-----------------------------------------------------------------------------
qSlicerMeasurementsRulerWidget::qSlicerMeasurementsRulerWidget(QWidget *_parent):
Superclass(_parent), d_ptr(new qSlicerMeasurementsRulerWidgetPrivate)
{
  Q_D(qSlicerMeasurementsRulerWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerMeasurementsRulerWidget::~qSlicerMeasurementsRulerWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerMeasurementsRulerWidget::printAdditionalInfo()
{
}
