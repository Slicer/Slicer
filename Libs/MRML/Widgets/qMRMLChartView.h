/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: qMRMLChartView.h,v $
Date:      $Date: 2006/03/17 15:10:09 $
Version:   $Revision: 1.18 $

=========================================================================auto=*/
#ifndef __qMRMLChartView_h
#define __qMRMLChartView_h

// Qt includes
#include <QWebView>

#include "qMRMLWidgetsExport.h"

class qMRMLChartViewPrivate;

// MRML includes
class vtkMRMLChartViewNode;
class vtkMRMLColorLogic;
class vtkMRMLScene;

class QMRML_WIDGETS_EXPORT qMRMLChartView : public QWebView
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef QWebView Superclass;
  
  /// Constructors
  explicit qMRMLChartView(QWidget* parent = 0);
  virtual ~qMRMLChartView();

  /// Return a pointer on the current MRML scene
  vtkMRMLScene* mrmlScene() const;

  /// Get the ChartView node observed by view.
  vtkMRMLChartViewNode* mrmlChartViewNode()const;

  /// Set the application color logic for default node
  /// color.
  void setColorLogic(vtkMRMLColorLogic* colorLogic);

  /// Get the application color logic. 0 by default.
  vtkMRMLColorLogic* colorLogic()const;

  // Redefine the sizeHint so layouts work properly.
  virtual QSize sizeHint() const;

public slots:

  /// Set the MRML \a scene that should be listened for events
  void setMRMLScene(vtkMRMLScene* newScene);

  /// Set the current \a viewNode to observe
  void setMRMLChartViewNode(vtkMRMLChartViewNode* newChartViewNode);

signals:

  /// Signal emitted when a data point has been clicked. Returns the
  /// id of the MRMLDoubleArrayNode, the index of the point, and the values
  void dataPointClicked(const char *mrmlArrayID, int pointidx, double x, double y);

  /// When designing custom qMRMLWidget in the designer, you can connect the
  /// mrmlSceneChanged signal directly to the aggregated MRML widgets that
  /// have a setMRMLScene slot.
  void mrmlSceneChanged(vtkMRMLScene*);

protected:
  QScopedPointer<qMRMLChartViewPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLChartView);
  Q_DISABLE_COPY(qMRMLChartView);
};

#endif
