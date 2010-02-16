#ifndef __qSlicerTractographyFiducialSeedingModuleWidget_h
#define __qSlicerTractographyFiducialSeedingModuleWidget_h

/// SlicerQT includes
#include "qSlicerAbstractModuleWidget.h"

/// qCTK includes
#include <qCTKPimpl.h>

#include "qSlicerTractographyFiducialSeedingModuleExport.h"

class qSlicerTractographyFiducialSeedingModuleWidgetPrivate;
class vtkMRMLNode;
class vtkMRMLFiberBundleNode;
class vtkMRMLTransformableNode;
class vtkMRMLDiffusionTensorVolumeNode;
class vtkMRMLTractographyFiducialSeedingNode;

class Q_SLICER_QTMODULES_TRACTOGRAPHYFIDUCIALSEEDING_EXPORT qSlicerTractographyFiducialSeedingModuleWidget :  public qSlicerAbstractModuleWidget
{
  Q_OBJECT
public:
  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerTractographyFiducialSeedingModuleWidget(QWidget *parent=0);

  /// get current parameter node
  vtkMRMLTractographyFiducialSeedingNode* tractographyFiducialSeedingNode() {
    return this->TractographyFiducialSeedingNode;
  };

  /// get current seeding node
  vtkMRMLTransformableNode* transformableNode(); 

  /// get current DTI volume node
  vtkMRMLDiffusionTensorVolumeNode* diffusionTensorVolumeNode(); 

  /// get current fiber bundlde node
  vtkMRMLFiberBundleNode* fiberBundleNode();

public slots:

  /// 
  /// Set the current MRML scene to the widget
  virtual void setMRMLScene(vtkMRMLScene*);

  /// set current parameter node
  void setTractographyFiducialSeedingNode(vtkMRMLNode *node);

  /// set current seeding node
  void setTransformableNode(vtkMRMLNode *node);

  /// set current DTI volume node
  void setDiffusionTensorVolumeNode(vtkMRMLNode *node);

  /// set current fiber bundlde node
  void setFiberBundleNode(vtkMRMLNode *node);

  /// set stopping criteria 0-Linear Measure, 1 - FA
  void setStoppingCriteria(int value);

  /// set display mode 0-line 1-tube
  void setTrackDisplayMode(int value);

  /// set stopping curvature
  void setStoppingCurvature(double value);

  /// set stopping curvature
  void setStoppingValue(double value);

  /// set integration step length, mm
  void setIntegrationStep(double value);

  /// set minimum path length, mm
  void setMinimumPath(double value);

  /// set the size of fiducial region, mm
  void setFiducialRegion(double value);

  /// set the step of fiducial region, mm
  void setFiducialRegionStep(double value);

  /// set seed only selected fiducails
  void setSeedSelectedFiducials(int value);

  /// set enable/disable seeding
  void setEnableSeeding(int value);

  /// set max number of seeds
  void setMaxNumberSeeds(int value);

  /// update widget GUI from parameter node
  void updateWidgetfromMRML();

protected:

  virtual void setup();

  vtkMRMLTractographyFiducialSeedingNode* TractographyFiducialSeedingNode;

private:
  QCTK_DECLARE_PRIVATE(qSlicerTractographyFiducialSeedingModuleWidget);
};

#endif
                                     
