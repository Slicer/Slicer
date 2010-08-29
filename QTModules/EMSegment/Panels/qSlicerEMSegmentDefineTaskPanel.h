#ifndef __qSlicerEMSegmentDefineTaskPanel_h
#define __qSlicerEMSegmentDefineTaskPanel_h 

// CTK includes
#include <ctkPimpl.h>

// EMSegment includes
#include "qSlicerEMSegmentWidget.h"

#include "qSlicerEMSegmentModuleExport.h"

class qSlicerEMSegmentDefineTaskPanelPrivate;
class vtkMRMLNode;

class Q_SLICER_QTMODULES_EMSEGMENT_EXPORT qSlicerEMSegmentDefineTaskPanel :
    public qSlicerEMSegmentWidget
{ 
  Q_OBJECT

public:

  typedef qSlicerEMSegmentWidget Superclass;
  qSlicerEMSegmentDefineTaskPanel(QWidget *newParent=0);

protected slots:
  void selectTask(vtkMRMLNode* mrmlNode);

private:
  CTK_DECLARE_PRIVATE(qSlicerEMSegmentDefineTaskPanel);
};

#endif
