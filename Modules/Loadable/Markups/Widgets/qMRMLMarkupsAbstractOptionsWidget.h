/*==============================================================================

  Copyright (c) The Intervention Centre
  Oslo University Hospital, Oslo, Norway. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Rafael Palomar (The Intervention Centre,
  Oslo University Hospital) and was supported by The Research Council of Norway
  through the ALive project (grant nr. 311393).

==============================================================================*/

#ifndef __qMRMLMarkupsAbstractOptionsWidget_h_
#define __qMRMLMarkupsAbstractOptionsWidget_h_

// Markups Widgets includes
#include "qSlicerMarkupsModuleWidgetsExport.h"

//MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLMarkupsNode.h>

// Qt includes
#include <QWidget>

//-------------------------------------------------------------------------------
class vtkMRMLMarkupsNode;
class vtkMRMLNode;

//-------------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Markups
/// \name qMRMLMarkupsAbstractOptionsWidget
/// \brief qMRMLMarkupsAbstractOptionsWidget is a base class for the
/// additional options widgets associated to some types of markups.
class Q_SLICER_MODULE_MARKUPS_WIDGETS_EXPORT qMRMLMarkupsAbstractOptionsWidget
  : public QWidget
{
  Q_OBJECT

  Q_PROPERTY(QString className READ className CONSTANT);

public:
  typedef QWidget Superclass;
  qMRMLMarkupsAbstractOptionsWidget(QWidget* parent=nullptr);
  ~qMRMLMarkupsAbstractOptionsWidget()=default;

  /// Updates the widget based on information from MRML.
  virtual void updateWidgetFromMRML() = 0;

  /// Gets the name of the additional options widget type
  virtual const QString className() const = 0;

  // Returns the associated markups node
  vtkMRMLMarkupsNode* mrmlMarkupsNode() const
    {return this->MarkupsNode.GetPointer();}

  // Returns the associated markups node
  vtkMRMLScene* mrmlScene() const
    {return this->MRMLScene.GetPointer();}

  /// Checks whether a given node can be handled by the widget. This allows
  /// using complex logics to determine whether the widget can manage a given
  /// markups node or not.
  virtual bool canManageMRMLMarkupsNode(vtkMRMLMarkupsNode *markupsNode) const = 0;

  /// Clone options widget . Override to return a new instance of the options widget
  virtual qMRMLMarkupsAbstractOptionsWidget* createInstance() const = 0;

public slots:
  /// Sets the vtkMRMLNode to operate on.
  void setMRMLMarkupsNode(vtkMRMLNode* markupsNode);
  /// Sets the vtkMRMLMarkupsNode to operate on.
  virtual void setMRMLMarkupsNode(vtkMRMLMarkupsNode* markupsNode) = 0;
  /// Sets the vtkMRMLNode to operate on.
  virtual void setMRMLScene(vtkMRMLScene* mrmlScene)
  {this->MRMLScene = mrmlScene;}

protected:
  vtkWeakPointer<vtkMRMLMarkupsNode> MarkupsNode;
  vtkWeakPointer<vtkMRMLScene> MRMLScene;

private:
  Q_DISABLE_COPY(qMRMLMarkupsAbstractOptionsWidget);
};

#endif // __qMRMLMarkupsAbstractOptionsWidget_h_
