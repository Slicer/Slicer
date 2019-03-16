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

#ifndef __qMRMLColorTableComboBox_h
#define __qMRMLColorTableComboBox_h

// CTK includes
#include <ctkPimpl.h>

// qMRML includes
#include "qMRMLNodeComboBox.h"
#include "qMRMLWidgetsExport.h"

class qMRMLColorTableComboBoxPrivate;

class QMRML_WIDGETS_EXPORT qMRMLColorTableComboBox : public qMRMLNodeComboBox
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef qMRMLNodeComboBox Superclass;

  /// Construct an empty qMRMLColorTableComboBox with a null scene,
  /// no nodeType, where the hidden nodes are not forced on display.
  explicit qMRMLColorTableComboBox(QWidget* parent = nullptr);
  ~qMRMLColorTableComboBox() override;
  void setMRMLScene(vtkMRMLScene* scene) override;

protected:
  QAbstractItemModel* createSceneModel();

protected:
  QScopedPointer<qMRMLColorTableComboBoxPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLColorTableComboBox);
  Q_DISABLE_COPY(qMRMLColorTableComboBox);
};

#endif
