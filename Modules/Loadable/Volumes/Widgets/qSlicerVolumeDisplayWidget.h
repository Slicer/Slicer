#ifndef __qSlicerVolumeDisplayWidget_h
#define __qSlicerVolumeDisplayWidget_h

// Qt includes
#include <QStackedWidget>

// CTK includes
#include <ctkVTKObject.h>

// Volumes includes
#include "qSlicerVolumesModuleWidgetsExport.h"

class vtkMRMLNode;
class qSlicerVolumeDisplayWidgetPrivate;

/// \ingroup Slicer_QtModules_Volumes
class Q_SLICER_QTMODULES_VOLUMES_WIDGETS_EXPORT qSlicerVolumeDisplayWidget : public QStackedWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  /// Constructors
  typedef QStackedWidget Superclass;
  explicit qSlicerVolumeDisplayWidget(QWidget* parent=nullptr);
  ~qSlicerVolumeDisplayWidget() override;

public slots:
  /// Set the MRML node of interest
  void setMRMLVolumeNode(vtkMRMLNode* node);

protected slots:
  /// Internally use in case the current display widget should change when the
  /// volume node changes
  void updateFromMRML(vtkObject* volume);
protected:
  QScopedPointer<qSlicerVolumeDisplayWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerVolumeDisplayWidget);
  Q_DISABLE_COPY(qSlicerVolumeDisplayWidget);
};

#endif
