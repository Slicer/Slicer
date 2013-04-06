#ifndef __qSlicerTractographyEditorROIWidget_h
#define __qSlicerTractographyEditorROIWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkVTKObject.h>

// SlicerQt includes
#include <qSlicerWidget.h>

// qMRML includes
#include "qSlicerTractographyDisplayModuleWidgetsExport.h"

class qSlicerTractographyEditorROIWidgetPrivate;
class vtkMRMLNode;
class vtkMRMLFiberBundleNode;

class Q_SLICER_MODULE_TRACTOGRAPHYDISPLAY_WIDGETS_EXPORT qSlicerTractographyEditorROIWidget
 : public qSlicerWidget
{
  Q_OBJECT
  QVTK_OBJECT
public:
  typedef qSlicerWidget Superclass;
  qSlicerTractographyEditorROIWidget(QWidget *parent=0);
  virtual ~qSlicerTractographyEditorROIWidget();

  vtkMRMLFiberBundleNode* fiberBundleNode()const;

public slots:
  void setFiberBundleNode(vtkMRMLNode *);
  void setFiberBundleNode(vtkMRMLFiberBundleNode*);
  void setAnnotationMRMLNodeForFiberSelection(vtkMRMLNode*);
  void setAnnotationROIMRMLNodeToFiberBundleEnvelope(vtkMRMLNode*);
  void disableROISelection(bool);
  void positiveROISelection(bool);
  void negativeROISelection(bool);
  void setInteractiveROI(bool);
  void setROIVisibility(bool);
  void createNewBundleFromSelection();
  void updateBundleFromSelection();

protected slots:
  void updateWidgetFromMRML();

protected:
  QScopedPointer<qSlicerTractographyEditorROIWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerTractographyEditorROIWidget);
  Q_DISABLE_COPY(qSlicerTractographyEditorROIWidget);
  int updating;
};

#endif
