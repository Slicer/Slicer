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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qMRMLSliceInformationWidget_p_h
#define __qMRMLSliceInformationWidget_p_h

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

/// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// qMRML includes
#include "qMRMLSliceInformationWidget.h"
#include "ui_qMRMLSliceInformationWidget.h"

// MRMLLogic includes
#include <vtkMRMLSliceLogic.h>

/// VTK includes
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>
#include <vtkImageData.h>

class QAction;
class ctkVTKSliceView;
class vtkMRMLSliceNode;
class vtkObject;

//-----------------------------------------------------------------------------
class qMRMLSliceInformationWidgetPrivate: public QObject,
                                   public Ui_qMRMLSliceInformationWidget
{
  Q_OBJECT
  QVTK_OBJECT
  Q_DECLARE_PUBLIC(qMRMLSliceInformationWidget);
protected:
  qMRMLSliceInformationWidget* const q_ptr;
public:
  qMRMLSliceInformationWidgetPrivate(qMRMLSliceInformationWidget& object);
  ~qMRMLSliceInformationWidgetPrivate() override;

  void setupUi(qMRMLWidget* widget);

public slots:
  /// Update widget state using the associated MRML slice node
  void updateWidgetFromMRMLSliceNode();



public:
  vtkWeakPointer<vtkMRMLSliceNode>    MRMLSliceNode;
  QButtonGroup*                       SliceSpacingModeGroup;

};

#endif
