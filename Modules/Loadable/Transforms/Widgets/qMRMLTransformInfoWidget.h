/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Andras Lasso and Franklin King at
  PerkLab, Queen's University and was supported through the Applied Cancer
  Research Unit program of Cancer Care Ontario with funds provided by the
  Ontario Ministry of Health and Long-Term Care.

==============================================================================*/


#ifndef __qMRMLTransformInfoWidget_h
#define __qMRMLTransformInfoWidget_h

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// Slicer includes
#include "qMRMLWidget.h"

#include "qSlicerTransformsModuleWidgetsExport.h"

class qMRMLTransformInfoWidgetPrivate;
class vtkMRMLTransformNode;
class vtkMRMLNode;

/// \ingroup Slicer_QtModules_Transforms
class Q_SLICER_MODULE_TRANSFORMS_WIDGETS_EXPORT
qMRMLTransformInfoWidget
  : public qMRMLWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef qMRMLWidget Superclass;
  qMRMLTransformInfoWidget(QWidget *parent=nullptr);
  ~qMRMLTransformInfoWidget() override;

  vtkMRMLTransformNode* mrmlTransformNode()const;

  vtkMRMLScene* mrmlScene()const;

public slots:

  /// Set the MRML node of interest
  /// Note that setting transformNode to 0 will disable the widget
  void setMRMLTransformNode(vtkMRMLTransformNode* transformNode);

  void setMRMLScene(vtkMRMLScene* scene) override;

  /// Utility function that calls setMRMLTransformNode(vtkMRMLTransformNode* transformNode)
  /// It's useful to connect to vtkMRMLNode* signals
  void setMRMLTransformNode(vtkMRMLNode* node);

  /// Process event function
  void processEvent(vtkObject* sender, void* callData, unsigned long eventId, void* clientData);

protected slots:
  void updateWidgetFromMRML();
  void updateTransformVectorDisplayFromMRML();

protected:

  void showEvent(QShowEvent *) override;

  QScopedPointer<qMRMLTransformInfoWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLTransformInfoWidget);
  Q_DISABLE_COPY(qMRMLTransformInfoWidget);
};

#endif
