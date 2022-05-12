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

#ifndef __qMRMLCollapsibleButton_h
#define __qMRMLCollapsibleButton_h

// CTK includes
#include <ctkCollapsibleButton.h>

// qMRMLWidget includes
#include "qMRMLWidgetsExport.h"

class qMRMLCollapsibleButtonPrivate;
class vtkMRMLScene;

/// This class is intended to improve visual appearance and convenience of ctkCollapsibleButton.
///
/// Currently the visual appearance is the same as its base class.
///
/// The mrmlSceneChanged signal can be used to simplify scene settings in Qt Designer:
/// it allows drawing one long signal/slot arrow from the top-level widget to the collapsible button
/// and a short signal/slot arrow from the collapsible button to each child widget in it.
class QMRML_WIDGETS_EXPORT qMRMLCollapsibleButton : public ctkCollapsibleButton
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef ctkCollapsibleButton Superclass;

  /// Constructors
  explicit qMRMLCollapsibleButton(QWidget* parent = nullptr);
  ~qMRMLCollapsibleButton() override;

  /// Return a pointer on the MRML scene
  vtkMRMLScene* mrmlScene() const;

public slots:
  void setMRMLScene(vtkMRMLScene* scene);

signals:
  void mrmlSceneChanged(vtkMRMLScene*);

protected:
  QScopedPointer<qMRMLCollapsibleButtonPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLCollapsibleButton);
  Q_DISABLE_COPY(qMRMLCollapsibleButton);
};

#endif
