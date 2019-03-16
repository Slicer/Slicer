/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qMRMLVolumeInfoWidget_h
#define __qMRMLVolumeInfoWidget_h


// Qt includes
#include <QListWidget>

// CTK includes
#include <ctkVTKObject.h>

// qMRML includes
#include "qMRMLWidget.h"
#include "qMRMLWidgetsExport.h"

class qMRMLVolumeInfoWidgetPrivate;
class vtkMRMLNode;
class vtkMRMLVolumeNode;

class QMRML_WIDGETS_EXPORT qMRMLVolumeInfoWidget : public qMRMLWidget
{
  Q_OBJECT
  QVTK_OBJECT
  Q_PROPERTY(bool dataTypeEditable READ isDataTypeEditable WRITE setDataTypeEditable)
public:
  typedef qMRMLWidget Superclass;

  qMRMLVolumeInfoWidget(QWidget *parent=nullptr);
  ~qMRMLVolumeInfoWidget() override;

  vtkMRMLVolumeNode* volumeNode()const;
  // Depends on the dimension, spacing and origin of the volume
  bool isCentered()const;

  // Disabled by default
  bool isDataTypeEditable()const;

public slots:
  /// Utility function to be connected with generic signals
  void setVolumeNode(vtkMRMLNode *node);
  /// Set the volume node to display
  void setVolumeNode(vtkMRMLVolumeNode *node);
  void setDataTypeEditable(bool enable);

  void setImageSpacing(double*);
  void setImageOrigin(double*);
  void center();
  void setScanOrder(int);
  void setNumberOfScalars(int);
  void setScalarType(int);
  void setWindowLevelFromPreset(QListWidgetItem*);

protected slots:
  void updateWidgetFromMRML();

protected:
  QScopedPointer<qMRMLVolumeInfoWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLVolumeInfoWidget);
  Q_DISABLE_COPY(qMRMLVolumeInfoWidget);
};

#endif
