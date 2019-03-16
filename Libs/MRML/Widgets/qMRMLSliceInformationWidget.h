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

#ifndef __qMRMLSliceInformationWidget_h
#define __qMRMLSliceInformationWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkPimpl.h>

// qMRMLWidget includes
#include "qMRMLWidget.h"

#include "qMRMLWidgetsExport.h"

class qMRMLSliceInformationWidgetPrivate;
class vtkMRMLNode;
class vtkMRMLSliceNode;

class vtkMRMLSliceLogic;

class QMRML_WIDGETS_EXPORT qMRMLSliceInformationWidget : public qMRMLWidget
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef qMRMLWidget Superclass;

  /// Constructors
  explicit qMRMLSliceInformationWidget(QWidget* parent = nullptr);
  ~qMRMLSliceInformationWidget() override;

  /// Get \a sliceNode
  /// \sa setMRMLSliceCompositeNode();
  vtkMRMLSliceNode* mrmlSliceNode()const;

public slots:

  /// Set a new SliceNode.
  void setMRMLSliceNode(vtkMRMLNode* newNode);

  /// Set a new SliceNode.
  void setMRMLSliceNode(vtkMRMLSliceNode* newSliceNode);

  /// Set slice orientation.
  /// \note Orientation could be either "Axial, "Sagittal", "Coronal" or "Reformat".
  void setSliceOrientation(const QString& orientation);

  /// Set slice visible.
  void setSliceVisible(bool visible);

  /// Set widget visible.
  void setWidgetVisible(bool visible);

  /// Set view group
  void setViewGroup(int viewGroup);

  /// Set lightbox layout row count
  void setLightboxLayoutRows(int rowCount);

  /// Set lightbox layout column count
  void setLightboxLayoutColumns(int columnCount);

  /// Specify whether the slice spacing is automatically determined or prescribed
  /// \sa vtkMRMLSliceNode::AutomaticSliceSpacingMode vtkMRMLSliceNode::PrescribedSliceSpacingMode
  void setSliceSpacingMode(int spacingMode);

  /// Set prescribed spacing
  void setPrescribedSliceSpacing(double spacing);

protected:
  QScopedPointer<qMRMLSliceInformationWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLSliceInformationWidget);
  Q_DISABLE_COPY(qMRMLSliceInformationWidget);
};

#endif
