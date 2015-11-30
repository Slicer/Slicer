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

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// SubjectHierarchy MRML includes
#include "vtkMRMLSubjectHierarchyNode.h"
#include "vtkMRMLSubjectHierarchyConstants.h"

// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyModelsPlugin.h"
#include "qSlicerSubjectHierarchyDefaultPlugin.h"

// MRML includes
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLModelDisplayNode.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>

// Qt includes
#include <QDebug>
#include <QStandardItem>
#include <QAction>
#include <QLineEdit>

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy_Plugins
class qSlicerSubjectHierarchyModelsPluginPrivate: public QObject
{
  Q_DECLARE_PUBLIC(qSlicerSubjectHierarchyModelsPlugin);
protected:
  qSlicerSubjectHierarchyModelsPlugin* const q_ptr;
public:
  qSlicerSubjectHierarchyModelsPluginPrivate(qSlicerSubjectHierarchyModelsPlugin& object);
  ~qSlicerSubjectHierarchyModelsPluginPrivate();
  void init();
public:
  QIcon ModelIcon;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyModelsPluginPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyModelsPluginPrivate::qSlicerSubjectHierarchyModelsPluginPrivate(qSlicerSubjectHierarchyModelsPlugin& object)
: q_ptr(&object)
{
  this->ModelIcon = QIcon(":Icons/Model.png");
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyModelsPluginPrivate::~qSlicerSubjectHierarchyModelsPluginPrivate()
{
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyModelsPlugin::qSlicerSubjectHierarchyModelsPlugin(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qSlicerSubjectHierarchyModelsPluginPrivate(*this) )
{
  this->m_Name = QString("Models");

  Q_D(qSlicerSubjectHierarchyModelsPlugin);
  d->init();
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyModelsPluginPrivate::init()
{
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyModelsPlugin::~qSlicerSubjectHierarchyModelsPlugin()
{
}

//----------------------------------------------------------------------------
double qSlicerSubjectHierarchyModelsPlugin::canAddNodeToSubjectHierarchy(vtkMRMLNode* node, vtkMRMLSubjectHierarchyNode* parent/*=NULL*/)const
{
  Q_UNUSED(parent);
  if (!node)
    {
    qCritical() << "qSlicerSubjectHierarchyModelsPlugin::canAddNodeToSubjectHierarchy: Input node is NULL!";
    return 0.0;
    }
  else if (node->IsA("vtkMRMLModelNode"))
    {
    // Node is a model
    return 0.5;
    }
  return 0.0;
}

//---------------------------------------------------------------------------
double qSlicerSubjectHierarchyModelsPlugin::canOwnSubjectHierarchyNode(vtkMRMLSubjectHierarchyNode* node)const
{
  if (!node)
    {
    qCritical() << "qSlicerSubjectHierarchyModelsPlugin::canOwnSubjectHierarchyNode: Input node is NULL!";
    return 0.0;
    }

  // Model
  vtkMRMLNode* associatedNode = node->GetAssociatedNode();
  if (associatedNode && associatedNode->IsA("vtkMRMLModelNode"))
    {
    return 0.5; // There may be other plugins that can handle special models better
    }

  return 0.0;
}

//---------------------------------------------------------------------------
const QString qSlicerSubjectHierarchyModelsPlugin::roleForPlugin()const
{
  return "Model";
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyModelsPlugin::icon(vtkMRMLSubjectHierarchyNode* node)
{
  if (!node)
    {
    qCritical() << "qSlicerSubjectHierarchyModelsPlugin::icon: NULL node given!";
    return QIcon();
    }

  Q_D(qSlicerSubjectHierarchyModelsPlugin);

  if (this->canOwnSubjectHierarchyNode(node))
    {
    return d->ModelIcon;
    }

  // Node unknown by plugin
  return QIcon();
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyModelsPlugin::visibilityIcon(int visible)
{
  // Have the default plugin (which is not registered) take care of this
  return qSlicerSubjectHierarchyPluginHandler::instance()->defaultPlugin()->visibilityIcon(visible);
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyModelsPlugin::editProperties(vtkMRMLSubjectHierarchyNode* node)
{
  // Switch to models module
  qSlicerAbstractModuleWidget* moduleWidget = qSlicerSubjectHierarchyAbstractPlugin::switchToModule("Models");
  if (moduleWidget)
    {
    // Get filter search box
    QLineEdit* searchBox = moduleWidget->findChild<QLineEdit*>("ScrollToModelSearchBox");
    if (searchBox && node->GetAssociatedNode())
      {
      // Enter node name in the filter box to select it
      searchBox->setText(node->GetAssociatedNode()->GetName());
      }
    }
}

//-----------------------------------------------------------------------------
QString qSlicerSubjectHierarchyModelsPlugin::tooltip(vtkMRMLSubjectHierarchyNode* node)const
{
  if (!node)
    {
    qCritical() << "qSlicerSubjectHierarchyModelsPlugin::tooltip: Subject hierarchy node is NULL!";
    return QString("Invalid!");
    }

  // Get basic tooltip from abstract plugin
  QString tooltipString = Superclass::tooltip(node);

  vtkMRMLModelNode* modelNode =
    vtkMRMLModelNode::SafeDownCast(node->GetAssociatedNode());
  vtkPolyData* polyData = modelNode->GetPolyData();
  if (modelNode && modelNode->GetDisplayNode() && polyData)
    {
    vtkMRMLModelDisplayNode* displayNode =
      vtkMRMLModelDisplayNode::SafeDownCast(modelNode->GetDisplayNode());
    bool visible = (displayNode->GetVisibility() > 0);
    tooltipString.append( QString(" (Points: %1  Cells: %2  Visible: %3")
      .arg(polyData->GetNumberOfPoints()).arg(polyData->GetNumberOfCells())
      .arg(visible ? "YES" : "NO") );
    if (visible)
      {
        double color[3] = {0.0,0.0,0.0};
        displayNode->GetColor(color);
      tooltipString.append( QString("  Color: %4,%5,%6  Opacity: %7%")
        .arg(int(color[0]*255.0)).arg(int(color[1]*255.0)).arg(int(color[2]*255.0))
        .arg(int(displayNode->GetOpacity()*100.0)) );
      }
    tooltipString.append(QString(")"));
    }
  else
    {
    tooltipString.append(" !Invalid model!");
    }

  return tooltipString;
}

