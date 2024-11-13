/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported in part through NIH grant R01 HL153166.

==============================================================================*/

#ifndef __qMRMLClipNodeDisplayWidget_h
#define __qMRMLClipNodeDisplayWidget_h

// Qt includes
#include <qMRMLWidget.h>

// CTK includes
#include <ctkVTKObject.h>

// qMRML includes
#include "qMRMLWidgetsExport.h"
#include "vtkMRMLClipNode.h"

class qMRMLClipNodeDisplayWidgetPrivate;
class vtkMRMLNode;
class vtkMRMLDisplayNode;

class QMRML_WIDGETS_EXPORT qMRMLClipNodeDisplayWidget : public qMRMLWidget
{
  Q_OBJECT
  QVTK_OBJECT
public:
  qMRMLClipNodeDisplayWidget(QWidget* parent = nullptr);
  ~qMRMLClipNodeDisplayWidget() override;

  vtkMRMLDisplayNode* mrmlDisplayNode() const;

public slots:
  /// Set the clip node to represent
  void setMRMLDisplayNode(vtkMRMLDisplayNode* node);
  /// Utility function to be connected to signals/slots
  void setMRMLDisplayNode(vtkMRMLNode* node);

protected slots:
  void updateWidgetFromMRML();
  void updateMRMLFromWidget();

protected:
  QScopedPointer<qMRMLClipNodeDisplayWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLClipNodeDisplayWidget);
  Q_DISABLE_COPY(qMRMLClipNodeDisplayWidget);
};

#endif
