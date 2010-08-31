#ifndef __qSlicerVolumeDisplayWidget_h
#define __qSlicerVolumeDisplayWidget_h


#include <QStackedWidget>

#include "qSlicerVolumesModuleExport.h"

class vtkMRMLNode;
class qSlicerLabelMapVolumeDisplayWidget;
class qSlicerScalarVolumeDisplayWidget;

class Q_SLICER_QTMODULES_VOLUMES_EXPORT qSlicerVolumeDisplayWidget : public QStackedWidget
{
  Q_OBJECT

public:
  /// Constructors
  typedef QStackedWidget Superclass;
  explicit qSlicerVolumeDisplayWidget(QWidget* parent=0);
  virtual ~qSlicerVolumeDisplayWidget(){}


public slots:

  /// 
  /// Set the MRML node of interest
  void setMRMLVolumeNode(vtkMRMLNode* node);

private:
  qSlicerScalarVolumeDisplayWidget*   ScalarVolumeDisplayWidget;
  qSlicerLabelMapVolumeDisplayWidget* LabelMapVolumeDisplayWidget;
};

#endif
