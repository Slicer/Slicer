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

#ifndef __qMRMLDisplayNodeWidget_h
#define __qMRMLDisplayNodeWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkVTKObject.h>

// qMRML includes
#include "qMRMLWidgetsExport.h"

class qMRMLDisplayNodeWidgetPrivate;
class vtkMRMLNode;
class vtkMRMLDisplayNode;

class QMRML_WIDGETS_EXPORT qMRMLDisplayNodeWidget : public QWidget
{
  Q_OBJECT
  QVTK_OBJECT
public:
  qMRMLDisplayNodeWidget(QWidget *parent=0);
  virtual ~qMRMLDisplayNodeWidget();
  
  vtkMRMLDisplayNode* mrmlDisplayNode()const;
  
  bool visibility()const;
  bool selected()const;
  bool clipping()const;
  bool sliceIntersectionVisible()const;
  int sliceIntersectionThickness()const;
  QColor color()const;
  double opacity()const;
  double ambient()const;
  double diffuse()const;
  double specular()const;
  double specularPower()const;
  bool backfaceCulling()const;

public slots:
  /// Set the volume node to display
  void setMRMLDisplayNode(vtkMRMLDisplayNode *node);
  /// Utility function to be connected with generic signals
  void setMRMLDisplayNode(vtkMRMLNode *node);
  /// Utility function to be connected with generic signals,
  /// it internally shows the 1st display node.
  void setMRMLDisplayableNode(vtkMRMLNode* displayableNode);

  void setVisibility(bool);
  void setSelected(bool);
  void setClipping(bool);
  void setSliceIntersectionVisible(bool);
  void setSliceIntersectionThickness(int);

  /// Make checkboxes visible or invisible, for use with display nodes that don't 
  /// need to control visibility, selected, clipping, slice intersection
  /// visibility, slice intersection thickness
  void setVisibilityVisible(bool);
  void setSelectedVisible(bool);
  void setClippingVisible(bool);
  void setSliceIntersectionVisibleVisible(bool);
  void setSliceIntersectionThicknessVisible(bool);

  /// Set the values on the display node
  void setColor(const QColor&);
  void setOpacity(double);
  void setAmbient(double);
  void setDiffuse(double);
  void setSpecular(double);
  void setSpecularPower(double);
  void setBackfaceCulling(bool);

protected slots:
  void updateWidgetFromMRML();

protected:
  QScopedPointer<qMRMLDisplayNodeWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLDisplayNodeWidget);
  Q_DISABLE_COPY(qMRMLDisplayNodeWidget);
};

#endif
