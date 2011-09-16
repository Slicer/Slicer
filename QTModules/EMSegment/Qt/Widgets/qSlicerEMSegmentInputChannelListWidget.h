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

#ifndef __qSlicerEMSegmentInputChannelListWidget_h
#define __qSlicerEMSegmentInputChannelListWidget_h

// CTK includes
#include <ctkPimpl.h>

// EMSegment includes
#include "qSlicerEMSegmentWidget.h"

#include "qSlicerEMSegmentModuleWidgetsExport.h"

class qSlicerEMSegmentInputChannelListWidgetPrivate;
class vtkMRMLVolumeNode;

/// \ingroup Slicer_QtModules_EMSegment
class Q_SLICER_MODULE_EMSEGMENT_WIDGET_EXPORT qSlicerEMSegmentInputChannelListWidget :
    public qSlicerEMSegmentWidget
{
  Q_OBJECT

public:

  typedef qSlicerEMSegmentWidget Superclass;
  explicit qSlicerEMSegmentInputChannelListWidget(QWidget *newParent = 0);
  virtual ~qSlicerEMSegmentInputChannelListWidget();

  Q_INVOKABLE int inputChannelCount() const;

  Q_INVOKABLE QString inputChannelName(int rowId) const;

  Q_INVOKABLE vtkMRMLVolumeNode* inputChannelVolume(int rowId) const;

  Q_INVOKABLE bool identicalInputVolumes() const;

  Q_INVOKABLE bool identicalInputChannelNames() const;

public slots:

  virtual void setMRMLManager(vtkEMSegmentMRMLManager * newMRMLManager);

  void updateWidgetFromMRML();

  void updateMRMLFromWidget();

  void addInputChannel();

  void removeInputChannel();

private slots:

  void onCellChanged(int row, int column);

protected:
  QScopedPointer<qSlicerEMSegmentInputChannelListWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerEMSegmentInputChannelListWidget);
  Q_DISABLE_COPY(qSlicerEMSegmentInputChannelListWidget);

  bool updating;

};

#endif // __qSlicerEMSegmentInputChannelListWidget_h
