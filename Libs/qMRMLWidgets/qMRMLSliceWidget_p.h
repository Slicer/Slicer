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

#ifndef __qMRMLSliceWidget_p_h
#define __qMRMLSliceWidget_p_h

/// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// qMRML includes
#include "qMRMLSliceWidget.h"
#include "ui_qMRMLSliceWidget.h"

class vtkMRMLDisplayableManagerGroup;
class QResizeEvent;

//-----------------------------------------------------------------------------
class qMRMLSliceWidgetPrivate
  : public QObject
  , public Ui_qMRMLSliceWidget
{
  Q_OBJECT
  QVTK_OBJECT
  Q_DECLARE_PUBLIC(qMRMLSliceWidget);
protected:
  qMRMLSliceWidget* const q_ptr;
public:
  qMRMLSliceWidgetPrivate(qMRMLSliceWidget& object);
  ~qMRMLSliceWidgetPrivate();

  void init();

public slots:

  /// Handle MRML scene event
  void onSceneAboutToBeClosedEvent();
  void onSceneClosedEvent();
  void onSceneAboutToBeImportedEvent();
  void onSceneImportedEvent();
  void onSceneRestoredEvent();

  /// Set the image data to the slice view
  void setImageData(vtkImageData * imageData);

  void updateWidgetFromMRMLSliceNode();

public:
  void initDisplayableManagers();

  vtkMRMLDisplayableManagerGroup*    DisplayableManagerGroup;
  vtkMRMLSliceNode*                  MRMLSliceNode;
};

#endif
