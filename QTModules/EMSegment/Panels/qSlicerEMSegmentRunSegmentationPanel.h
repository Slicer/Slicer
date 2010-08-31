#ifndef __qSlicerEMSegmentRunSegmentationPanel_h
#define __qSlicerEMSegmentRunSegmentationPanel_h 

// CTK includes
#include <ctkPimpl.h>

// EMSegment includes
#include "qSlicerEMSegmentWidget.h"

// EMSegment/MRML includes
class vtkMRMLROINode;

#include "qSlicerEMSegmentModuleExport.h"

class qSlicerEMSegmentRunSegmentationPanelPrivate;

class Q_SLICER_QTMODULES_EMSEGMENT_EXPORT qSlicerEMSegmentRunSegmentationPanel :
    public qSlicerEMSegmentWidget
{ 
  Q_OBJECT

public:

  typedef qSlicerEMSegmentWidget Superclass;
  qSlicerEMSegmentRunSegmentationPanel(QWidget *newParent=0);

  void updateWidgetFromMRML();

  void updateMRMLFromWidget();

private:
  void setMRMLROINode(vtkMRMLROINode* node);

private slots:
  void onShowROIMapChanged(bool show);

private:
  CTK_DECLARE_PRIVATE(qSlicerEMSegmentRunSegmentationPanel);
  friend class qSlicerEMSegmentRunSegmentationStep; // for access to setMRMLROINode
};

#endif
