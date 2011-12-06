#ifndef __qSlicerTractographyDisplayGlyphWidget_h
#define __qSlicerTractographyDisplayGlyphWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkVTKObject.h>

// SlicerQt includes
#include <qSlicerWidget.h>

// qMRML includes
#include "qSlicerTractographyDisplayModuleWidgetsExport.h"

class qSlicerTractographyDisplayGlyphWidgetPrivate;
class vtkMRMLNode;
class vtkMRMLFiberBundleDisplayNode;
class vtkMRMLDiffusionTensorDisplayPropertiesNode;

class Q_SLICER_MODULE_TRACTOGRAPHYDISPLAY_WIDGETS_EXPORT qSlicerTractographyDisplayGlyphWidget
 : public qSlicerWidget
{
  Q_OBJECT
  QVTK_OBJECT
public:
  typedef qSlicerWidget Superclass;
  qSlicerTractographyDisplayGlyphWidget(QWidget *parent=0);
  virtual ~qSlicerTractographyDisplayGlyphWidget();

  vtkMRMLFiberBundleDisplayNode* fiberBundleDisplayNode()const;
  vtkMRMLDiffusionTensorDisplayPropertiesNode* diffusionTensorDisplayPropertiesNode()const;

public slots:
  void setFiberBundleDisplayNode(vtkMRMLNode *);
  void setFiberBundleDisplayNode(vtkMRMLFiberBundleDisplayNode*);
  void setDiffusionTensorDisplayPropertiesNode(vtkMRMLNode*);
  void setDiffusionTensorDisplayPropertiesNode
    (vtkMRMLDiffusionTensorDisplayPropertiesNode*);

  void setGlyphScaleFactor(double);
  void setGlyphSpacing(double);
  void setGlyphType(int);
  void setGlyphEigenvector(int);
  void setTubeGlyphNumberOfSides(double);
  void setTubeGlyphRadius(double);  
  
protected slots:
  void updateWidgetFromMRMLDisplayNode();
  void updateWidgetFromMRMLDisplayPropertiesNode();

protected:
  QScopedPointer<qSlicerTractographyDisplayGlyphWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerTractographyDisplayGlyphWidget);
  Q_DISABLE_COPY(qSlicerTractographyDisplayGlyphWidget);
  int updating;
};

#endif
