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

/// ScalarOverlay includes
#include "qSlicerIOOptions_p.h"
#include "qSlicerScalarOverlayIOOptionsWidget.h"
#include "ui_qSlicerScalarOverlayIOOptionsWidget.h"

// MRML includes
#include <vtkMRMLNode.h>

//-----------------------------------------------------------------------------
class qSlicerScalarOverlayIOOptionsWidgetPrivate
  : public qSlicerIOOptionsPrivate
  , public Ui_qSlicerScalarOverlayIOOptionsWidget
{
public:
};

//-----------------------------------------------------------------------------
qSlicerScalarOverlayIOOptionsWidget::qSlicerScalarOverlayIOOptionsWidget(QWidget* parentWidget)
  : Superclass(new qSlicerScalarOverlayIOOptionsWidgetPrivate, parentWidget)
{
  Q_D(qSlicerScalarOverlayIOOptionsWidget);
  d->setupUi(this);

  connect(d->ModelSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
          this, SLOT(updateProperties()));
}

//-----------------------------------------------------------------------------
qSlicerScalarOverlayIOOptionsWidget::~qSlicerScalarOverlayIOOptionsWidget() = default;

//-----------------------------------------------------------------------------
bool qSlicerScalarOverlayIOOptionsWidget::isValid()const
{
  Q_D(const qSlicerScalarOverlayIOOptionsWidget);
  return this->qSlicerIOOptionsWidget::isValid() &&
    d->Properties.contains("modelNodeId");
}

//-----------------------------------------------------------------------------
void qSlicerScalarOverlayIOOptionsWidget::updateProperties()
{
  Q_D(qSlicerScalarOverlayIOOptionsWidget);
  vtkMRMLNode* modelNode = d->ModelSelector->currentNode();
  if (modelNode)
    {
    d->Properties["modelNodeId"] = QString(modelNode->GetID());
    }
  else
    {
    d->Properties.remove("modelNodeId");
    }
  this->updateValid();
}
