/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was developed with funding from the Children's Hospital of
  Philadelphia Frontier Program.

==============================================================================*/

#ifndef __qMRMLColorLegendGroupBox_h
#define __qMRMLColorLegendGroupBox_h

// CTK includes
#include <ctkCollapsibleGroupBox.h>
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// Colors Widgets includes
#include "qSlicerColorsModuleWidgetsExport.h"

class qMRMLColorLegendDisplayNodeWidget;
class qMRMLColorLegendGroupBoxPrivate;
class vtkMRMLDisplayNode;
class vtkMRMLNode;

/// \brief Collapsible "Color legend" section for the colors of a display node.
///
/// The legend display node is created (hidden) on first expand, so legends are only added to the scene on request.
/// Disabled while the primary display node does not color by scalars through a color table.
class Q_SLICER_MODULE_COLORS_WIDGETS_EXPORT qMRMLColorLegendGroupBox : public ctkCollapsibleGroupBox
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef ctkCollapsibleGroupBox Superclass;
  explicit qMRMLColorLegendGroupBox(QWidget* parent = nullptr);
  ~qMRMLColorLegendGroupBox() override;

  /// The display node whose colors the legend shows
  vtkMRMLDisplayNode* primaryDisplayNode() const;

  /// The widget that edits the legend
  qMRMLColorLegendDisplayNodeWidget* colorLegendDisplayNodeWidget() const;

public slots:
  void setPrimaryDisplayNode(vtkMRMLDisplayNode* displayNode);
  /// Utility function to be connected with generic signals
  void setPrimaryDisplayNode(vtkMRMLNode* node);

protected slots:
  void updateWidgetFromMRML();

protected:
  QScopedPointer<qMRMLColorLegendGroupBoxPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLColorLegendGroupBox);
  Q_DISABLE_COPY(qMRMLColorLegendGroupBox);
};

#endif
