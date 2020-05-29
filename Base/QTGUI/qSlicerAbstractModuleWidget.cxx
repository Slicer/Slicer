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

// Slicer includes
#include "qSlicerAbstractModule.h"
#include "qSlicerAbstractModuleWidget.h"

//-----------------------------------------------------------------------------
class qSlicerAbstractModuleWidgetPrivate
{
public:
  qSlicerAbstractModuleWidgetPrivate();
  bool IsEntered;
};

//-----------------------------------------------------------------------------
qSlicerAbstractModuleWidgetPrivate::qSlicerAbstractModuleWidgetPrivate()
{
  this->IsEntered = false;
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleWidget::qSlicerAbstractModuleWidget(QWidget* parentWidget)
  : qSlicerWidget(parentWidget)
  , d_ptr(new qSlicerAbstractModuleWidgetPrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleWidget::~qSlicerAbstractModuleWidget()
{
  Q_ASSERT(this->isEntered() == false);
}

//-----------------------------------------------------------------------------
void qSlicerAbstractModuleWidget::enter()
{
  Q_D(qSlicerAbstractModuleWidget);
  Q_ASSERT(d->IsEntered == false);
  d->IsEntered = true;
}

//-----------------------------------------------------------------------------
void qSlicerAbstractModuleWidget::exit()
{
  Q_D(qSlicerAbstractModuleWidget);
  Q_ASSERT(d->IsEntered == true);
  d->IsEntered = false;
}

//-----------------------------------------------------------------------------
bool qSlicerAbstractModuleWidget::isEntered()const
{
  Q_D(const qSlicerAbstractModuleWidget);
  return d->IsEntered;
}

//-----------------------------------------------------------------------------
void qSlicerAbstractModuleWidget::setup()
{
  const qSlicerAbstractModule* m =
    qobject_cast<const qSlicerAbstractModule*>(this->module());
  if (m)
    {
    this->setObjectName(QString("%1ModuleWidget").arg(m->name()));
    this->setWindowTitle(m->title());
    this->setWindowIcon(m->icon());
    }
}

//-----------------------------------------------------------
bool qSlicerAbstractModuleWidget::setEditedNode(vtkMRMLNode* node,
                                                QString role /* = QString()*/,
                                                QString context /* = QString()*/)
{
  // this method is redefined here to make it Q_INVOKABLE
  return qSlicerAbstractModuleRepresentation::setEditedNode(node, role, context);
}

//-----------------------------------------------------------
double qSlicerAbstractModuleWidget::nodeEditable(vtkMRMLNode* node)
{
  // this method is redefined here to make it Q_INVOKABLE
  return qSlicerAbstractModuleRepresentation::nodeEditable(node);
}
