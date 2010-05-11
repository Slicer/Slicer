/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/


#ifndef __qSlicerWidget_h
#define __qSlicerWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

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

  /// 
  /// Convenient windows to return parent widget or Null if any
  QWidget* parentWidget();

  /// 
  /// If possible, set the windowsFlags of the parent container.
  /// Otherwise, set the ones of the current widget
  void setWindowFlags(Qt::WindowFlags type);

  /// 
  /// Tell if the parent container is a QScrollArea
  bool isParentContainerScrollArea();

  /// 
  /// Convenient method to Set/Get the parent container as a QScrollArea
  /// Note: Method mainly used while porting the application from KwWidget to Qt
  QScrollArea* getScrollAreaParentContainer();
  void setScrollAreaAsParentContainer(bool enable);

  /// 
  /// If possible, set parent container geometry otherwise set widget geometry
  void setParentGeometry(int ax, int ay, int aw, int ah);

  /// 
  /// Return a pointer on the MRML scene
  vtkMRMLScene* mrmlScene() const;

  /// 
  /// Set/Get the application logic
  //void setAppLogic(vtkSlicerApplicationLogic* appLogic);
  //vtkSlicerApplicationLogic* appLogic() const;

public slots:

  /// 
  /// \Overload
  virtual void setParentVisible(bool visible);

  /// 
  /// Set the current MRML scene to the widget
  virtual void setMRMLScene(vtkMRMLScene*);

signals:
  void mrmlSceneChanged(vtkMRMLScene*);

private:
  CTK_DECLARE_PRIVATE(qSlicerWidget);
};

#endif
