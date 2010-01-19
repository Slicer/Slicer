/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/


#include "qSlicerWidget.h"

// SlicerLogic includes
#include "vtkSlicerApplicationLogic.h"

// VTK includes
#include "vtkSmartPointer.h"

// QT includes
#include <QScrollArea>
#include <QVBoxLayout>
#include <QPointer>
#include <QDebug>

//-----------------------------------------------------------------------------
class qSlicerWidgetPrivate: public qCTKPrivate<qSlicerWidget>
{
public:
  QPointer<QWidget>                          ParentContainer;
  vtkSmartPointer<vtkMRMLScene>              MRMLScene;
};

//-----------------------------------------------------------------------------
qSlicerWidget::qSlicerWidget(QWidget * _parent, Qt::WindowFlags f)
  :Superclass(_parent, f)
{
  QCTK_INIT_PRIVATE(qSlicerWidget);
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
bool qSlicerWidget::isParentContainerScrollArea()
{
  return (qobject_cast<QScrollArea*>(qctk_d()->ParentContainer)!=0);
}

//-----------------------------------------------------------------------------
QScrollArea* qSlicerWidget::getScrollAreaParentContainer()
{
  return qobject_cast<QScrollArea*>(qctk_d()->ParentContainer);
}

//-----------------------------------------------------------------------------
void qSlicerWidget::setScrollAreaAsParentContainer(bool enable)
{
  QCTK_D(qSlicerWidget);
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
  QCTK_D(qSlicerWidget);
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
     this->Superclass::setVisible(visible);
     }
}

//-----------------------------------------------------------------------------
void qSlicerWidget::setMRMLScene(vtkMRMLScene* scene)
{
  if (scene == qctk_d()->MRMLScene)
    {
    return ;
    }
  qctk_d()->MRMLScene = scene;
  emit mrmlSceneChanged(scene);
}

//-----------------------------------------------------------------------------
QCTK_GET_CXX(qSlicerWidget, vtkMRMLScene*, mrmlScene, MRMLScene);

//-----------------------------------------------------------------------------
//QCTK_SET_CXX(qSlicerWidget, vtkSlicerApplicationLogic*, setAppLogic, AppLogic);
//QCTK_GET_CXX(qSlicerWidget, vtkSlicerApplicationLogic*, appLogic, AppLogic);
