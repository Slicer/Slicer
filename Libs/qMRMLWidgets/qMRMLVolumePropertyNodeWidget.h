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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qMRMLVolumePropertyNodeWidget_h
#define __qMRMLVolumePropertyNodeWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkVTKObject.h>

// qMRMLWidget includes
#include "qMRMLWidgetsExport.h"
class qMRMLVolumePropertyNodeWidgetPrivate;

// MRML includes
class vtkMRMLNode;
class vtkMRMLVolumePropertyNode;

class QMRML_WIDGETS_EXPORT qMRMLVolumePropertyNodeWidget
  : public QWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  /// Constructors
  typedef QWidget Superclass;
  explicit qMRMLVolumePropertyNodeWidget(QWidget* parent=0);
  virtual ~qMRMLVolumePropertyNodeWidget();

public slots:
  ///
  /// Convenient function to connect with signal/slots
  void setMRMLVolumePropertyNode(vtkMRMLNode* node);
  void setMRMLVolumePropertyNode(vtkMRMLVolumePropertyNode* volumePropertyNode);

protected slots:
  void updateFromVolumePropertyNode();

protected:
  QScopedPointer<qMRMLVolumePropertyNodeWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLVolumePropertyNodeWidget);
  Q_DISABLE_COPY(qMRMLVolumePropertyNodeWidget);
};

#endif
