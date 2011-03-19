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

#ifndef __qMRMLScalarInvariantComboBox_h
#define __qMRMLScalarInvariantComboBox_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkVTKObject.h>

// qMRML includes
#include "qMRMLWidgetsExport.h"

class qMRMLScalarInvariantComboBoxPrivate;
class vtkMRMLNode;
class vtkMRMLDiffusionTensorDisplayPropertiesNode;

class QMRML_WIDGETS_EXPORT qMRMLScalarInvariantComboBox : public QWidget
{
  Q_OBJECT
  QVTK_OBJECT
  Q_PROPERTY(int scalarInvariant READ scalarInvariant WRITE setScalarInvariant DESIGNABLE false)

public:
  qMRMLScalarInvariantComboBox(QWidget *parent=0);
  virtual ~qMRMLScalarInvariantComboBox();

  vtkMRMLDiffusionTensorDisplayPropertiesNode* displayPropertiesNode()const;
  /// Returns -1 if no displayPropertiesNode is set
  int scalarInvariant()const;

public slots:
  /// Utility function to be connected with generic signals
  void setDisplayPropertiesNode(vtkMRMLNode *node);
  /// Set the volume node properties
  void setDisplayPropertiesNode(vtkMRMLDiffusionTensorDisplayPropertiesNode *node);
  /// doesn't do anything if no display properties node is set
  void setScalarInvariant(int value);

signals:
  void scalarInvariantChanged(int scalarInvariant);

protected slots:
  void updateWidgetFromMRML();
  void onCurrentScalarInvariantChanged(int index);

protected:
  QScopedPointer<qMRMLScalarInvariantComboBoxPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLScalarInvariantComboBox);
  Q_DISABLE_COPY(qMRMLScalarInvariantComboBox);
};

#endif
