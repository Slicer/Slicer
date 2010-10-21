/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
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
#include <QScrollArea>
#include <QVBoxLayout>
#include <QPointer>
#include <QDebug>

#include "qSlicerWidget.h"

// VTK includes
#include "vtkSmartPointer.h"

//-----------------------------------------------------------------------------
class qSlicerWidgetPrivate
{
public:
  QPointer<QWidget>                          ParentContainer;
};

//-----------------------------------------------------------------------------
qSlicerWidget::qSlicerWidget(QWidget * _parent, Qt::WindowFlags f)
  :QWidget(_parent, f)
  , d_ptr(new qSlicerWidgetPrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerWidget::~qSlicerWidget()
{
}

//-----------------------------------------------------------------------------
QWidget* qSlicerWidget::parentWidget()
{
  if (this->isParentContainerScrollArea())
    {
    return this->getScrollAreaParentContainer();
    }
  return qobject_cast<QWidget*>(this->parent());
}

//-----------------------------------------------------------------------------
bool qSlicerWidget::isParentContainerScrollArea()const
{
  Q_D(const qSlicerWidget);
  return (qobject_cast<QScrollArea*>(d->ParentContainer)!=0);
}

//-----------------------------------------------------------------------------
QScrollArea* qSlicerWidget::getScrollAreaParentContainer()const
{
  Q_D(const qSlicerWidget);
  return qobject_cast<QScrollArea*>(d->ParentContainer);
}

//-----------------------------------------------------------------------------
void qSlicerWidget::setScrollAreaAsParentContainer(bool enable)
{
  Q_D(qSlicerWidget);
  if (enable)
    {
    if (this->getScrollAreaParentContainer())
      {
      return;
      }

    // Instantiate a scrollArea
    QScrollArea * scrollArea = new QScrollArea();
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setWidgetResizable(true);

    // Set window title
    scrollArea->setWindowTitle(this->windowTitle());

    // Add an content widget responsible for the layout
    QWidget * scrollAreaWidgetContents = new QWidget();
    this->setParent(scrollAreaWidgetContents);

    // Layout vertically and add a spacer/stretcher
    QVBoxLayout * verticalLayout = new QVBoxLayout(scrollAreaWidgetContents);
    verticalLayout->addWidget(this);
    verticalLayout->addStretch();

    // Add scrollAreaWidgetContents to the scrollArea
    scrollArea->setWidget(scrollAreaWidgetContents);

    d->ParentContainer = scrollArea;

    d->ParentContainer->setWindowFlags(this->windowFlags());
    this->QWidget::setWindowFlags(0);
    }
  else
    {
    if (!this->getScrollAreaParentContainer())
      {
      return;
      }
    this->getScrollAreaParentContainer()->takeWidget();
    this->setParent(0);
    d->ParentContainer->deleteLater();

    this->QWidget::setWindowFlags(d->ParentContainer->windowFlags());
    d->ParentContainer->setWindowFlags(0);
    }
}

//-----------------------------------------------------------------------------
void qSlicerWidget::setWindowFlags(Qt::WindowFlags type)
{
  Q_D(qSlicerWidget);
  if (d->ParentContainer)
    {
    d->ParentContainer->setWindowFlags(type);
    return;
    }
  this->setWindowFlags(type);
}

//---------------------------------------------------------------------------
void qSlicerWidget::setParentGeometry(int ax, int ay, int aw, int ah)
{
  if (this->parentWidget())
    {
    this->parentWidget()->setGeometry(QRect(ax, ay, aw, ah));
    }
  else
    {
    this->setGeometry(QRect(ax, ay, aw, ah));
    }
}

//-----------------------------------------------------------------------------
void qSlicerWidget::setParentVisible(bool visible)
{
   if (this->parentWidget())
     {
     this->parentWidget()->setVisible(visible);
     }
   else
     {
     this->QWidget::setVisible(visible);
     }
}
//-----------------------------------------------------------------------------
//CTK_SET_CPP(qSlicerWidget, vtkSlicerApplicationLogic*, setAppLogic, AppLogic);
//CTK_GET_CPP(qSlicerWidget, vtkSlicerApplicationLogic*, appLogic, AppLogic);
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
