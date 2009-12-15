/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/


#ifndef __qSlicerWidget_h
#define __qSlicerWidget_h

// qMRML includes
#include <qVTKObject.h>

// qCTK includes
#include <qCTKPimpl.h>

// QT includes
#include <QWidget>

#include "qSlicerBaseQTBaseExport.h"

class vtkSlicerApplicationLogic;
class vtkMRMLScene;
class QScrollArea;
class qSlicerWidgetPrivate;

class Q_SLICER_BASE_QTBASE_EXPORT qSlicerWidget : public QWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:

  typedef QWidget Superclass;
  qSlicerWidget(QWidget *parent=0, Qt::WindowFlags f=0);

  // Description:
  // Convenient windows to return parent widget or Null if any
  QWidget* parentWidget();

  // Description:
  // If possible, set the windowsFlags of the parent container.
  // Otherwise, set the ones of the current widget
  void setWindowFlags(Qt::WindowFlags type);

  // Description:
  // Tell if the parent container is a QScrollArea
  bool isParentContainerScrollArea();

  // Description:
  // Convenient method to Set/Get the parent container as a QScrollArea
  // Note: Method mainly used while porting the application from KwWidget to Qt
  QScrollArea* getScrollAreaParentContainer();
  void setScrollAreaAsParentContainer(bool enable);

  // Description:
  // If possible, set parent container geometry otherwise set widget geometry
  void setParentGeometry(int ax, int ay, int aw, int ah);

  // Description:
  // Return a pointer on the MRML scene
  vtkMRMLScene* mrmlScene() const;

  // Description:
  // Set/Get the application logic
  //void setAppLogic(vtkSlicerApplicationLogic* appLogic);
  //vtkSlicerApplicationLogic* appLogic() const;

public slots:

  // Description:
  // Overloaded method from QWidget
  virtual void setParentVisible(bool visible);

  // Description:
  // Set the current MRML scene to the widget
  virtual void setMRMLScene(vtkMRMLScene*);

signals:
  void mrmlSceneChanged(vtkMRMLScene*);

private:
  QCTK_DECLARE_PRIVATE(qSlicerWidget);
};

#endif
