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

#ifndef __qMRMLNavigationView_h
#define __qMRMLNavigationView_h

// CTK includes
#include <ctkVTKObject.h>
#include <ctkVTKThumbnailView.h>

#include "qMRMLWidgetsExport.h"

class qMRMLNavigationViewPrivate;
class vtkMRMLScene;
class vtkMRMLViewNode;

/// Specialized ctkVTKThumbnailView that recomputes its bounds when the mrml
/// scene is updated and changes its background color based on a mrml view node
/// TODO: Add utility function setThreeDView to extract automatically the
/// renderer, mrml scene and mrml view node.
class QMRML_WIDGETS_EXPORT qMRMLNavigationView : public ctkVTKThumbnailView
{
  Q_OBJECT
  QVTK_OBJECT

public:
  /// Superclass typedef
  typedef ctkVTKThumbnailView Superclass;

  /// Constructors
  explicit qMRMLNavigationView(QWidget* parent = nullptr);
  ~qMRMLNavigationView() override;

public slots:

  /// Set the MRML \a scene that should be listened for events
  void setMRMLScene(vtkMRMLScene* newScene);

  /// Set/Get \a viewNode
  void setMRMLViewNode(vtkMRMLViewNode* newViewNode);
  vtkMRMLViewNode* mrmlViewNode()const;

protected slots:
  void updateFromMRMLViewNode();
  void updateFromMRMLScene();

protected:
  QScopedPointer<qMRMLNavigationViewPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLNavigationView);
  Q_DISABLE_COPY(qMRMLNavigationView);
};

#endif
