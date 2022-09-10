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

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, Cancer
  Care Ontario, OpenAnatomy, and Brigham and Women's Hospital through NIH grant R01MH112748.

==============================================================================*/

#ifndef __qMRMLMarkupsInteractionHandleWidget_h
#define __qMRMLMarkupsInteractionHandleWidget_h

// Qt includes
#include <QWidget>

// MarkupsWidgets includes
#include "qSlicerMarkupsModuleWidgetsExport.h"

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

#include <qMRMLWidget.h>

class vtkMRMLNode;
class vtkMRMLMarkupsDisplayNode;
class qMRMLMarkupsInteractionHandleWidgetPrivate;

class Q_SLICER_MODULE_MARKUPS_WIDGETS_EXPORT qMRMLMarkupsInteractionHandleWidget : public qMRMLWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  /// Constructors
  explicit qMRMLMarkupsInteractionHandleWidget(QWidget* parent = nullptr);
  ~qMRMLMarkupsInteractionHandleWidget() override;

  /// Returns the current MRML display node
  vtkMRMLMarkupsDisplayNode* mrmlDisplayNode() const;

public slots:
  /// Set the MRML display node
  void setMRMLDisplayNode(vtkMRMLMarkupsDisplayNode* node);

protected slots:
  /// Internal function to update the widgets based on the node/display node
  void updateWidgetFromMRML();

  /// Internal function to update the node based on the widget
  void updateMRMLFromWidget();

protected:
  QScopedPointer<qMRMLMarkupsInteractionHandleWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLMarkupsInteractionHandleWidget);
  Q_DISABLE_COPY(qMRMLMarkupsInteractionHandleWidget);
};

#endif
