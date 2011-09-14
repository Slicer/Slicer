#ifndef __qSlicerTractographyDisplayWidget_h
#define __qSlicerTractographyDisplayWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkVTKObject.h>

// SlicerQt includes
#include <qSlicerWidget.h>

// qMRML includes
#include "qSlicerTractographyDisplayModuleWidgetsExport.h"

class qSlicerTractographyDisplayWidgetPrivate;
class vtkMRMLNode;
class vtkMRMLFiberBundleDisplayNode;
class vtkMRMLDiffusionTensorDisplayPropertiesNode;

class Q_SLICER_MODULE_TRACTOGRAPHYDISPLAY_WIDGETS_EXPORT qSlicerTractographyDisplayWidget
 : public qSlicerWidget
{
  Q_OBJECT
  QVTK_OBJECT
public:
  typedef qSlicerWidget Superclass;
  qSlicerTractographyDisplayWidget(QWidget *parent=0);
  virtual ~qSlicerTractographyDisplayWidget();

  vtkMRMLFiberBundleDisplayNode* FiberBundleDisplayNode()const;
  vtkMRMLDiffusionTensorDisplayPropertiesNode* DiffusionTensorDisplayPropertiesNode()const;

public slots:
  void setFiberBundleDisplayNode(vtkMRMLNode *node);
  void setFiberBundleDisplayNode(vtkMRMLFiberBundleDisplayNode *node);

  void setVisibility(bool);
  void setColorByScalarInvariant();
  void onColorByScalarInvariantChanged(int);
  void setColorByScalarInvariantDisplayRange(double, double);
  void setColorByCellScalars();
  void setColorBySolid();
  void onColorBySolidChanged(const QColor&);
  void setColorByCellScalarsColorTable(vtkMRMLNode*);
  void setOpacity(double); 
  
  
  
protected slots:
  void updateWidgetFromMRML();

protected:
  QScopedPointer<qSlicerTractographyDisplayWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerTractographyDisplayWidget);
  Q_DISABLE_COPY(qSlicerTractographyDisplayWidget);
  int m_updating;
};

#endif
