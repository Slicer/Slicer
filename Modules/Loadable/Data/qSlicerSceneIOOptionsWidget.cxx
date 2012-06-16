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
#include <QFileInfo>

// CTK includes
#include <ctkFlowLayout.h>
#include <ctkUtils.h>

/// Scene includes
#include "qSlicerIOOptions_p.h"
#include "qSlicerSceneIOOptionsWidget.h"
#include "ui_qSlicerSceneIOOptionsWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Scene
class qSlicerSceneIOOptionsWidgetPrivate
  : public qSlicerIOOptionsPrivate
  , public Ui_qSlicerSceneIOOptionsWidget
{
public:
};

//-----------------------------------------------------------------------------
qSlicerSceneIOOptionsWidget::qSlicerSceneIOOptionsWidget(QWidget* parentWidget)
  : qSlicerIOOptionsWidget(new qSlicerSceneIOOptionsWidgetPrivate, parentWidget)
{
  Q_D(qSlicerSceneIOOptionsWidget);
  d->setupUi(this);

  ctkFlowLayout::replaceLayout(this);

  connect(d->ClearSceneCheckBox, SIGNAL(toggled(bool)),
          this, SLOT(updateProperties()));
  connect(d->CopyCameraCheckBox, SIGNAL(toggled(bool)),
          this, SLOT(updateProperties()));

  this->updateProperties();
}

//-----------------------------------------------------------------------------
qSlicerSceneIOOptionsWidget::~qSlicerSceneIOOptionsWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerSceneIOOptionsWidget::updateProperties()
{
  Q_D(qSlicerSceneIOOptionsWidget);

  d->Properties["clear"] = d->ClearSceneCheckBox->isChecked();
  d->Properties["copyCameras"] = d->CopyCameraCheckBox->isChecked();
}
