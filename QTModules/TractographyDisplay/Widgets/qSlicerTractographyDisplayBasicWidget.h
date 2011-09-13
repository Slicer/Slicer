#ifndef __qSlicerTractographyDisplayBasicWidget_h
#define __qSlicerTractographyDisplayBasicWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkVTKObject.h>

// SlicerQt includes
#include <qSlicerWidget.h>

// qMRML includes
#include "qSlicerTractographyDisplayModuleWidgetsExport.h"

class qSlicerTractographyDisplayBasicWidgetPrivate;
class vtkMRMLNode;
class vtkMRMLFiberBundleNode;

class Q_SLICER_MODULE_TRACTOGRAPHYDISPLAY_WIDGETS_EXPORT qSlicerTractographyDisplayBasicWidget
 : public qSlicerWidget
{
  Q_OBJECT
  QVTK_OBJECT
public:
  typedef qSlicerWidget Superclass;
  qSlicerTractographyDisplayBasicWidget(QWidget *parent=0);
  virtual ~qSlicerTractographyDisplayBasicWidget();

  vtkMRMLFiberBundleNode* fiberBundleNode()const;

public slots:
  void setFiberBundleNode(vtkMRMLNode *);
  void setFiberBundleNode(vtkMRMLFiberBundleNode*);
  void setLineVisibility(int);
  void setTubeVisibility(int);
  void setTubeSliceIntersectVisibility(int);
  void setGlyphVisibility(int);

protected slots:
  void updateWidgetFromMRML();

protected:
  QScopedPointer<qSlicerTractographyDisplayBasicWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerTractographyDisplayBasicWidget);
  Q_DISABLE_COPY(qSlicerTractographyDisplayBasicWidget);
  int updating;
};

#endif
