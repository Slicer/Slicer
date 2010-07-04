#ifndef __qSlicerDataModuleWidget_h
#define __qSlicerDataModuleWidget_h

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"
#include "qSlicerBaseQTCoreModulesExport.h"

class vtkMRMLNode;
class qSlicerDataModuleWidgetPrivate;

class Q_SLICER_BASE_QTCOREMODULES_EXPORT qSlicerDataModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT
public:
  qSlicerDataModuleWidget(QWidget *parentWidget = 0);

public slots:
  void loadScene();
  void addScene();
  void addData();
  void addVolumes();
  void addModels();
  void addScalarOverlay();
  void addTransformation();
  void addFiducialList();
  void addColorTable();
  void addDTI();

  void setMRMLIDsVisible(bool visible);
  void setCurrentNodeName(const QString& name);

protected slots:
  void onMRMLNodeChanged(vtkMRMLNode* node);
  void onMRMLNodeModified();
  void validateNodeName();

protected:
  virtual void setup();

private:
  CTK_DECLARE_PRIVATE(qSlicerDataModuleWidget);
};

#endif
