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

/// Qt includes
#include <QDebug>

/// Models includes
#include "qSlicerIOOptions_p.h"
#include "qSlicerModelsIOOptionsWidget.h"
#include "ui_qSlicerModelsIOOptionsWidget.h"

// MRML includes
#include <vtkMRMLNode.h>
#include <vtkMRMLStorageNode.h>

//-----------------------------------------------------------------------------
class qSlicerModelsIOOptionsWidgetPrivate
  : public qSlicerIOOptionsPrivate
  , public Ui_qSlicerModelsIOOptionsWidget
{
public:
};

//-----------------------------------------------------------------------------
qSlicerModelsIOOptionsWidget::qSlicerModelsIOOptionsWidget(QWidget* parentWidget)
  : Superclass(new qSlicerModelsIOOptionsWidgetPrivate, parentWidget)
{
  Q_D(qSlicerModelsIOOptionsWidget);
  d->setupUi(this);

  connect(d->coordinateSystemComboBox, SIGNAL(currentIndexChanged(int)),
          this, SLOT(updateProperties()));
}

//-----------------------------------------------------------------------------
qSlicerModelsIOOptionsWidget::~qSlicerModelsIOOptionsWidget() = default;

//-----------------------------------------------------------------------------
void qSlicerModelsIOOptionsWidget::updateProperties()
{
  Q_D(qSlicerModelsIOOptionsWidget);
  d->Properties["coordinateSystem"] = vtkMRMLStorageNode::GetCoordinateSystemTypeFromString(
    d->coordinateSystemComboBox->currentText().toLatin1().constData());
}
