#ifndef __qSlicerEMSegmentEditRegistrationParametersPanel_h
#define __qSlicerEMSegmentEditRegistrationParametersPanel_h 

// CTK includes
#include <ctkPimpl.h>

// EMSegment includes
#include "qSlicerEMSegmentWidget.h"

#include "qSlicerEMSegmentModuleExport.h"

class qSlicerEMSegmentEditRegistrationParametersPanelPrivate;

class Q_SLICER_QTMODULES_EMSEGMENT_EXPORT qSlicerEMSegmentEditRegistrationParametersPanel :
    public qSlicerEMSegmentWidget
{ 
  Q_OBJECT

public:

  typedef qSlicerEMSegmentWidget Superclass;
  qSlicerEMSegmentEditRegistrationParametersPanel(QWidget *newParent=0);

  void updateWidgetFromMRML();

  void updateMRMLFromWidget();

private slots:

  void onCurrentAffineRegistrationIndexChanged(int index);
  void onCurrentDeformableRegistrationIndexChanged(int index);
  void onCurrentInterpolationIndexChanged(int index);

private:
  CTK_DECLARE_PRIVATE(qSlicerEMSegmentEditRegistrationParametersPanel);
};

#endif
