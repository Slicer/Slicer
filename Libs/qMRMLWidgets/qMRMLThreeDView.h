/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qMRMLThreeDView_h
#define __qMRMLThreeDView_h

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKRenderView.h>

#include "qMRMLWidgetsExport.h"

class qMRMLThreeDViewPrivate;
class vtkMRMLScene;
class vtkMRMLViewNode;

class QMRML_WIDGETS_EXPORT qMRMLThreeDView : public ctkVTKRenderView
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef ctkVTKRenderView Superclass;
  
  /// Constructors
  explicit qMRMLThreeDView(QWidget* parent = 0);
  virtual ~qMRMLThreeDView();

  /// Register DisplayableManagers
  /// \a scriptedDisplayableManagerDirectory is the based directory from which
  /// scripted DisplayableManager should be sourced from.
  void registerDisplayableManagers(const QString& scriptedDisplayableManagerDirectory);

  /// If set to True, scripted DisplayableManager located in the directory specified
  /// when calling registerDisplayableManagers() will be ignored.
  void setIgnoreScriptedDisplayableManagers(bool value);

  /// Get the 3D View node observed by view.
  vtkMRMLViewNode* mrmlViewNode()const;

  /// Returns the interactor style of the view
  vtkInteractorObserver* interactorStyle()const;

public slots:

  /// Set the MRML \a scene that should be listened for events
  void setMRMLScene(vtkMRMLScene* newScene);

  /// Set the current \a viewNode to observe
  void setMRMLViewNode(vtkMRMLViewNode* newViewNode);

  /// Look from a given axis, need a mrml view node to be set
  void lookFromViewAxis(const ctkAxesWidget::Axis& axis);

  /// Reimplemented to hide items to not take into
  /// account when computing the boundaries
  virtual void resetFocalPoint();

protected:
  QScopedPointer<qMRMLThreeDViewPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLThreeDView);
  Q_DISABLE_COPY(qMRMLThreeDView);
};

#endif
