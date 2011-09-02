#ifndef __qSlicerTractographyFiducialSeedingModuleWidget_h
#define __qSlicerTractographyFiducialSeedingModuleWidget_h

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerTractographyFiducialSeedingModuleExport.h"

class qSlicerTractographyFiducialSeedingModuleWidgetPrivate;
class vtkMRMLNode;
class vtkMRMLFiberBundleNode;
class vtkMRMLDiffusionTensorVolumeNode;
class vtkMRMLTractographyFiducialSeedingNode;

/// \ingroup Slicer_QtModules_TractographyFiducialSeeding
class Q_SLICER_QTMODULES_TRACTOGRAPHYFIDUCIALSEEDING_EXPORT qSlicerTractographyFiducialSeedingModuleWidget :  public qSlicerAbstractModuleWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerTractographyFiducialSeedingModuleWidget(QWidget *_parent=0);
  virtual ~qSlicerTractographyFiducialSeedingModuleWidget();

  /// Get current parameter node
  vtkMRMLTractographyFiducialSeedingNode* tractographyFiducialSeedingNode() {
    return this->TractographyFiducialSeedingNode;
  };

  /// Get current seeding node
  vtkMRMLNode* seedingNode(); 

  /// Get current DTI volume node
  vtkMRMLDiffusionTensorVolumeNode* diffusionTensorVolumeNode(); 

  /// Get current fiber bundlde node
  vtkMRMLFiberBundleNode* fiberBundleNode();

  virtual void enter();

public slots:

  /// 
  /// Set the current MRML scene to the widget
  virtual void setMRMLScene(vtkMRMLScene*);

  /// Set current parameter node
  void setTractographyFiducialSeedingNode(vtkMRMLNode *node);

  /// Set current seeding node
  void setSeedingNode(vtkMRMLNode *node);

  /// Set current DTI volume node
  void setDiffusionTensorVolumeNode(vtkMRMLNode *node);

  /// Set current fiber bundlde node
  void setFiberBundleNode(vtkMRMLNode *node);

  /// Set stopping criteria 0-Linear Measure, 1 - FA
  void setStoppingCriteria(int value);

  /// Set display mode 0-line 1-tube
  void setTrackDisplayMode(int value);

  /// Set stopping curvature
  void setStoppingCurvature(double value);

  /// Set stopping curvature
  void setStoppingValue(double value);

  /// Set integration step length, mm
  void setIntegrationStep(double value);

  /// Set minimum path length, mm
  void setMinimumPath(double value);

  /// Set the size of fiducial region, mm
  void setFiducialRegion(double value);

  /// Set the step of fiducial region, mm
  void setFiducialRegionStep(double value);

  /// Set seed only selected fiducails
  void setSeedSelectedFiducials(int value);

  /// Set enable/disable seeding
  void setEnableSeeding(int value);

  /// Set max number of seeds
  void setMaxNumberSeeds(int value);

  /// Update widget GUI from parameter node
  void updateWidgetFromMRML();

protected:

  virtual void setup();

  vtkMRMLTractographyFiducialSeedingNode* TractographyFiducialSeedingNode;

  bool settingFiberBundleNode;

protected:
  QScopedPointer<qSlicerTractographyFiducialSeedingModuleWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerTractographyFiducialSeedingModuleWidget);
  Q_DISABLE_COPY(qSlicerTractographyFiducialSeedingModuleWidget);
};

#endif
                                     
