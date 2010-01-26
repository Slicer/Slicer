#ifndef __qSlicerTractographyFiducialSeedingModuleWidget_h
#define __qSlicerTractographyFiducialSeedingModuleWidget_h

// SlicerQT includes
#include "qSlicerAbstractModuleWidget.h"

// qCTK includes
#include <qCTKPimpl.h>

#include "qSlicerTractographyFiducialSeedingModuleExport.h"

class qSlicerTractographyFiducialSeedingModuleWidgetPrivate;
class vtkMRMLNode;

class Q_SLICER_QTMODULES_TRACTOGRAPHYFIDUCIALSEEDING_EXPORT qSlicerTractographyFiducialSeedingModuleWidget :  public qSlicerAbstractModuleWidget
{
  Q_OBJECT
public:
  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerTractographyFiducialSeedingModuleWidget(QWidget *parent=0);

protected:
  virtual void setup();

protected slots:
  void onParameterChanged(double value);
  void onParameterChanged(int value);
  void onParameterNodeChanged(vtkMRMLNode *node);
  void onSelectionNodeChanged(bool value);

private:
  QCTK_DECLARE_PRIVATE(qSlicerTractographyFiducialSeedingModuleWidget);
};

#endif
                                     
