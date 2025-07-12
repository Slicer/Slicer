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
#include <QPointer>

#include "qSlicerWidget.h"

// Slicer includes
#include <qSlicerApplication.h>

// MRML includes
#include <vtkSlicerApplicationLogic.h>

// VTK includes

//-----------------------------------------------------------------------------
class qSlicerWidgetPrivate
{
  Q_DECLARE_PUBLIC(qSlicerWidget);

protected:
  qSlicerWidget* const q_ptr;

public:
  qSlicerWidgetPrivate(qSlicerWidget& object);

  QPointer<QWidget> ParentContainer;
};

//-----------------------------------------------------------------------------
qSlicerWidgetPrivate::qSlicerWidgetPrivate(qSlicerWidget& object)
  : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
qSlicerWidget::qSlicerWidget(QWidget* _parent, Qt::WindowFlags f)
  : QWidget(_parent, f)
  , d_ptr(new qSlicerWidgetPrivate(*this))
{
}

//-----------------------------------------------------------------------------
qSlicerWidget::~qSlicerWidget() = default;

//-----------------------------------------------------------------------------

void qSlicerWidget::setMRMLScene(vtkMRMLScene* scene)
{
  bool emitSignal = this->mrmlScene() != scene;
  this->qSlicerObject::setMRMLScene(scene);
  if (emitSignal)
  {
    emit mrmlSceneChanged(scene);
  }
}

//-----------------------------------------------------------------------------
vtkSlicerApplicationLogic* qSlicerWidget::appLogic() const
{
  Q_D(const qSlicerWidget);
  if (!qSlicerApplication::application())
  {
    return nullptr;
  }
  return qSlicerApplication::application()->applicationLogic();
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerWidget::moduleLogic(const QString& moduleName) const
{
  Q_D(const qSlicerWidget);
  vtkSlicerApplicationLogic* applicationLogic = this->appLogic();
  if (!applicationLogic)
  {
    return nullptr;
  }
  return applicationLogic->GetModuleLogic(moduleName.toUtf8());
}
