/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

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

/// \ingroup Slicer_QtModules_EMSegment
class Q_SLICER_QTMODULES_EMSEGMENT_EXPORT qSlicerEMSegmentAnatomicalTreeWidget :
    public qSlicerEMSegmentWidget
{ 
  Q_OBJECT
  Q_PROPERTY(bool structureNameEditable READ structureNameEditable WRITE setStructureNameEditable)
  Q_PROPERTY(bool mrmlIDsColumnVisible READ mrmlIDsColumnVisible WRITE setMRMLIDsColumnVisible)
  Q_PROPERTY(bool displayMRMLIDsCheckBoxVisible READ isDisplayMRMLIDsCheckBoxVisible WRITE setDisplayMRMLIDsCheckBoxVisible)
  Q_PROPERTY(bool labelColumnVisible READ labelColumnVisible WRITE setLabelColumnVisible)
  Q_PROPERTY(bool classWeightColumnVisible READ classWeightColumnVisible WRITE setClassWeightColumnVisible)
  Q_PROPERTY(bool updateClassWeightColumnVisible READ updateClassWeightColumnVisible WRITE setUpdateClassWeightColumnVisible)
  Q_PROPERTY(bool atlasWeightColumnVisible READ atlasWeightColumnVisible WRITE setAtlasWeightColumnVisible)
  Q_PROPERTY(bool alphaColumnVisible READ alphaColumnVisible WRITE setAlphaColumnVisible)
  Q_PROPERTY(bool displayAlphaCheckBoxVisible READ isDisplayAlphaCheckBoxVisible WRITE setDisplayAlphaCheckBoxVisible)
  Q_PROPERTY(bool probabilityMapColumnVisible READ probabilityMapColumnVisible WRITE setProbabilityMapColumnVisible)

public:

  typedef qSlicerEMSegmentWidget Superclass;
  explicit qSlicerEMSegmentAnatomicalTreeWidget(QWidget *newParent = 0);
  virtual ~qSlicerEMSegmentAnatomicalTreeWidget();

  bool structureNameEditable() const;

  bool mrmlIDsColumnVisible() const;

  bool isDisplayMRMLIDsCheckBoxVisible() const;

  bool labelColumnVisible() const;

  bool classWeightColumnVisible() const;

  bool updateClassWeightColumnVisible() const;

  bool atlasWeightColumnVisible() const;

  bool alphaColumnVisible() const;

  bool isDisplayAlphaCheckBoxVisible() const;

  bool probabilityMapColumnVisible() const;

public slots:

  virtual void setMRMLManager(vtkEMSegmentMRMLManager * newMRMLManager);

  void updateWidgetFromMRML();

  void setStructureNameEditable(bool editable);

  void setMRMLIDsColumnVisible(bool visible);

  void setDisplayMRMLIDsCheckBoxVisible(bool visible);

  void setLabelColumnVisible(bool visible);

  void setClassWeightColumnVisible(bool visible);

  void setUpdateClassWeightColumnVisible(bool visible);

  void setAtlasWeightColumnVisible(bool visible);

  void setAlphaColumnVisible(bool visible);

  void setDisplayAlphaCheckBoxVisible(bool visible);

  void setProbabilityMapColumnVisible(bool visible);

protected slots:

  void collapseToDepthZero();

signals:

  void currentTreeNodeChanged(vtkMRMLNode* node);

//  void currentSpatialPriorVolumeNodeChanged(vtkMRMLNode* node);

//  void currentSpatialPriorVolumeNodeChanged(bool validNode);
protected:
  QScopedPointer<qSlicerEMSegmentAnatomicalTreeWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerEMSegmentAnatomicalTreeWidget);
  Q_DISABLE_COPY(qSlicerEMSegmentAnatomicalTreeWidget);
  typedef qSlicerEMSegmentAnatomicalTreeWidgetPrivate ctkPimpl;
};

#endif
