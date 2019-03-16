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

#ifndef __qMRMLSliceView_p_h
#define __qMRMLSliceView_p_h

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Slicer API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

// CTK includes
#include <ctkVTKObject.h>

// qMRML includes
#include "qMRMLSliceView.h"

// MRML includes
#include "vtkLightBoxRendererManager.h"
#include "vtkMRMLLightBoxRendererManagerProxy.h"

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

class vtkMRMLDisplayableManagerGroup;
class vtkMRMLSliceNode;
class vtkMRMLCameraNode;
class vtkObject;

//-----------------------------------------------------------------------------
class qMRMLSliceViewPrivate: public QObject
{
  Q_OBJECT
  QVTK_OBJECT
  Q_DECLARE_PUBLIC(qMRMLSliceView);
protected:
  qMRMLSliceView* const q_ptr;
public:
  qMRMLSliceViewPrivate(qMRMLSliceView& object);
  ~qMRMLSliceViewPrivate() override;

  virtual void init();

  void setMRMLScene(vtkMRMLScene* scene);

public slots:
  /// Handle MRML scene event
  void onSceneStartProcessing();
  void onSceneEndProcessing();

  void updateWidgetFromMRML();

protected:
  void initDisplayableManagers();

  vtkMRMLDisplayableManagerGroup*    DisplayableManagerGroup;
  vtkMRMLScene*                      MRMLScene;
  vtkMRMLSliceNode*                  MRMLSliceNode;
  QColor                             InactiveBoxColor;

  class vtkInternalLightBoxRendererManagerProxy;
  vtkSmartPointer<vtkInternalLightBoxRendererManagerProxy> LightBoxRendererManagerProxy;
};

//--------------------------------------------------------------------------
// qMRMLSliceWidgetPrivate::vtkInternalLightBoxRendererManagerProxy class

//---------------------------------------------------------------------------
class qMRMLSliceViewPrivate::vtkInternalLightBoxRendererManagerProxy
  : public vtkMRMLLightBoxRendererManagerProxy
{
public:
  static vtkInternalLightBoxRendererManagerProxy* New();
  vtkTypeMacro(vtkInternalLightBoxRendererManagerProxy,
                       vtkMRMLLightBoxRendererManagerProxy);


  /// Method to query the mapping from an id of a LightBox frame to
  /// the Renderer for that frame
  vtkRenderer *GetRenderer(int id) override;

  /// Method to set the real LightBoxManager
  virtual void SetLightBoxRendererManager(vtkLightBoxRendererManager *mgr);

protected:
  vtkInternalLightBoxRendererManagerProxy();
  ~vtkInternalLightBoxRendererManagerProxy() override;

private:
  vtkInternalLightBoxRendererManagerProxy(const vtkInternalLightBoxRendererManagerProxy&) = delete;
  void operator=(const vtkInternalLightBoxRendererManagerProxy&) = delete;

  vtkWeakPointer<vtkLightBoxRendererManager> LightBoxRendererManager;

};


#endif
