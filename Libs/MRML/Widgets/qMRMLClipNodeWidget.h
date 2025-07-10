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

#ifndef __qMRMLClipNodeWidget_h
#define __qMRMLClipNodeWidget_h

// Qt includes
#include <qMRMLWidget.h>

// CTK includes
#include <ctkVTKObject.h>

// qMRML includes
#include "qMRMLWidgetsExport.h"
#include "vtkMRMLClipNode.h"

class qMRMLClipNodeWidgetPrivate;
class vtkMRMLNode;

class QMRML_WIDGETS_EXPORT qMRMLClipNodeWidget : public qMRMLWidget
{
  Q_OBJECT
    QVTK_OBJECT
public:
  qMRMLClipNodeWidget(QWidget* parent = nullptr);
  ~qMRMLClipNodeWidget() override;

  vtkMRMLClipNode* mrmlClipNode() const;

  int clipType() const;
  void setClipType(int);

  int clipState(vtkMRMLNode* node) const;
  int clipState(const char* nodeID) const;
  void setClipState(vtkMRMLNode* node, int state);
  void setClipState(const char* nodeID, int state);

public slots:
  /// Set the clip node to represent
  void setMRMLClipNode(vtkMRMLClipNode* node);
  /// Utility function to be connected to signals/slots
  void setMRMLClipNode(vtkMRMLNode* node);

protected slots:
  void updateWidgetFromMRML();
  void updateMRMLFromWidget();

  // Update clipping node widget frame from node references.
  void updateClippingNodeFrame();
  // Update clipping node references from widget.
  void updateClippingNodeFromWidget();

  void updateNodeClipType();

protected:

  // Returns true if the frame widget needs to be updated to reflect the node references.
  bool needToUpdateClippingNodeFrame() const;

protected:
  QScopedPointer<qMRMLClipNodeWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLClipNodeWidget);
  Q_DISABLE_COPY(qMRMLClipNodeWidget);
};

#endif
