#ifndef __qSlicerEMSegmentInputChannelListWidget_h
#define __qSlicerEMSegmentInputChannelListWidget_h

// CTK includes
#include <ctkPimpl.h>

// EMSegment includes
#include "qSlicerEMSegmentWidget.h"

#include "qSlicerEMSegmentModuleExport.h"

class qSlicerEMSegmentInputChannelListWidgetPrivate;
class vtkMRMLVolumeNode;

class Q_SLICER_QTMODULES_EMSEGMENT_EXPORT qSlicerEMSegmentInputChannelListWidget :
    public qSlicerEMSegmentWidget
{
  Q_OBJECT

public:

  typedef qSlicerEMSegmentWidget Superclass;
  explicit qSlicerEMSegmentInputChannelListWidget(QWidget *newParent = 0);

  int inputChannelCount() const;

  QString inputChannelName(int rowId) const;

  vtkMRMLVolumeNode* inputChannelVolume(int rowId) const;

  bool identicalInputVolumes() const;

public slots:

  virtual void setMRMLManager(vtkEMSegmentMRMLManager * newMRMLManager);

  void updateWidgetFromMRML();

  void updateMRMLFromWidget();

  void addInputChannel();

  void removeInputChannel();

private slots:

  void onCellChanged(int row, int column);

private:
  CTK_DECLARE_PRIVATE(qSlicerEMSegmentInputChannelListWidget);
};

#endif // __qSlicerEMSegmentInputChannelListWidget_h
