#ifndef __qSlicerTractographyFiducialSeedingModuleWidget_h
#define __qSlicerTractographyFiducialSeedingModuleWidget_h

/// SlicerQT includes
#include "qSlicerAbstractModuleWidget.h"

/// qCTK includes
#include <qCTKPimpl.h>

#include "qSlicerTractographyFiducialSeedingModuleExport.h"

class qSlicerTractographyFiducialSeedingModuleWidgetPrivate;
class vtkMRMLNode;
class vtkMRMLTractographyFiducialSeedingNode;
class vtkMRMLFiberBundleNode;
class vtkMRMLTransformableNode;
class vtkMRMLDiffusionTensorVolumeNode;

class Q_SLICER_QTMODULES_TRACTOGRAPHYFIDUCIALSEEDING_EXPORT qSlicerTractographyFiducialSeedingModuleWidget :  public qSlicerAbstractModuleWidget
{
  Q_OBJECT
public:
  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerTractographyFiducialSeedingModuleWidget(QWidget *parent=0);

protected:
  virtual void setup();
  void updateWidgetfromMRML(vtkMRMLTractographyFiducialSeedingNode *paramNode);
  void connectNodeObservers(vtkMRMLTractographyFiducialSeedingNode* paramNode);

  vtkMRMLTractographyFiducialSeedingNode* TractographyFiducialSeedingNode;
  vtkMRMLFiberBundleNode*                 FiberBundleNode;
  vtkMRMLTransformableNode*               TransformableNode;
  vtkMRMLDiffusionTensorVolumeNode*       DiffusionTensorVolumeNode;

protected slots:
  void onParameterChanged(double value);
  void onParameterChanged(int value);
  void onParameterChanged(vtkObject* node);
  void onParameterNodeChanged(vtkMRMLNode *node);
  void onParameterNodeModified(vtkObject* node);
  void onSelectionNodeChanged(bool value);

private:
  QCTK_DECLARE_PRIVATE(qSlicerTractographyFiducialSeedingModuleWidget);
  bool processParameterChange;
};

#endif
                                     
