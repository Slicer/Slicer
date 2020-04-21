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

// qMRML includes
#include "qMRMLLayoutManager.h"
#include "qMRMLLayoutWidget.h"

class qMRMLLayoutWidgetPrivate
{
public:
  qMRMLLayoutManager* LayoutManager;
};

//------------------------------------------------------------------------------
// qMRMLLayoutWidget methods

// --------------------------------------------------------------------------
qMRMLLayoutWidget::qMRMLLayoutWidget(QWidget* widget)
  : Superclass(widget)
  , d_ptr(new qMRMLLayoutWidgetPrivate)
{
  this->setLayoutManager(new qMRMLLayoutManager);
}

// --------------------------------------------------------------------------
qMRMLLayoutWidget::~qMRMLLayoutWidget() = default;

//------------------------------------------------------------------------------
qMRMLLayoutManager* qMRMLLayoutWidget::layoutManager()const
{
  Q_D(const qMRMLLayoutWidget);
  return d->LayoutManager;
}

//------------------------------------------------------------------------------
void qMRMLLayoutWidget::setLayoutManager(qMRMLLayoutManager* layoutManager)
{
  Q_D(qMRMLLayoutWidget);
  d->LayoutManager = layoutManager;
  d->LayoutManager->setParent(this);
  d->LayoutManager->setViewport(this);
}

//------------------------------------------------------------------------------
void qMRMLLayoutWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qMRMLLayoutWidget);
  d->LayoutManager->setMRMLScene(scene);
}

//------------------------------------------------------------------------------
vtkMRMLScene* qMRMLLayoutWidget::mrmlScene()const
{
  Q_D(const qMRMLLayoutWidget);
  return d->LayoutManager->mrmlScene();
}

//------------------------------------------------------------------------------
int qMRMLLayoutWidget::layout()const
{
  Q_D(const qMRMLLayoutWidget);
  return d->LayoutManager->layout();
}

//------------------------------------------------------------------------------
void qMRMLLayoutWidget::setLayout(int layout)
{
  Q_D(qMRMLLayoutWidget);
  d->LayoutManager->setLayout(layout);
}
