#ifndef __qSlicerEMSegmentAnatomicalTreeWidget_h
#define __qSlicerEMSegmentAnatomicalTreeWidget_h 

// Qt includes
#include <QModelIndex>

// CTK includes
#include <ctkPimpl.h>

// EMSegment includes
#include "qSlicerEMSegmentWidget.h"

// VTK includes
#include <vtkType.h>

#include "qSlicerEMSegmentModuleExport.h"

class qSlicerEMSegmentAnatomicalTreeWidgetPrivate;
class QStandardItem;
class vtkMRMLNode;

class Q_SLICER_QTMODULES_EMSEGMENT_EXPORT qSlicerEMSegmentAnatomicalTreeWidget :
    public qSlicerEMSegmentWidget
{ 
  Q_OBJECT
  Q_PROPERTY(bool structureNameEditable READ structureNameEditable WRITE setStructureNameEditable)
  Q_PROPERTY(bool mrmlIDsColumnVisible READ mrmlIDsColumnVisible WRITE setMRMLIDsColumnVisible)
  Q_PROPERTY(bool labelColumnVisible READ labelColumnVisible WRITE setLabelColumnVisible)
  Q_PROPERTY(bool classWeightColumnVisible READ classWeightColumnVisible WRITE setClassWeightColumnVisible)
  Q_PROPERTY(bool atlasWeightColumnVisible READ atlasWeightColumnVisible WRITE setAtlasWeightColumnVisible)
  Q_PROPERTY(bool alphaColumnVisible READ alphaColumnVisible WRITE setAlphaColumnVisible)

public:

  typedef qSlicerEMSegmentWidget Superclass;
  explicit qSlicerEMSegmentAnatomicalTreeWidget(QWidget *newParent = 0);

  virtual void setMRMLScene(vtkMRMLScene *newScene);

  bool structureNameEditable() const;

  bool mrmlIDsColumnVisible() const;

  bool labelColumnVisible() const;

  bool classWeightColumnVisible() const;

  bool updateClassWeightColumnVisible() const;

  bool atlasWeightColumnVisible() const;

  bool alphaColumnVisible() const;

public slots:

  void setup();

  void updateWidgetFromMRML();

  void setStructureNameEditable(bool editable);

  void setMRMLIDsColumnVisible(bool visible);

  void setLabelColumnVisible(bool visible);

  void setClassWeightColumnVisible(bool visible);

  void setUpdateClassWeightColumnVisible(bool visible);

  void setAtlasWeightColumnVisible(bool visible);

  void setAlphaColumnVisible(bool visible);

signals:

  void currentTreeNodeChanged(vtkMRMLNode* node);

  void currentSpatialPriorVolumeNodeChanged(vtkMRMLNode* node);

  void currentSpatialPriorVolumeNodeChanged(bool validNode);

protected slots:

  void onTreeItemChanged(QStandardItem * treeItem);

  void onTreeItemSelected(const QModelIndex & index);

  void collapseToDepthZero();

private:

  void populateTreeModel(vtkIdType treeNodeId, QStandardItem *item);

  CTK_DECLARE_PRIVATE(qSlicerEMSegmentAnatomicalTreeWidget);
  typedef qSlicerEMSegmentAnatomicalTreeWidgetPrivate ctkPimpl;
};

#endif
