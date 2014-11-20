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

// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyDefaultPlugin.h"
#include "qSlicerSubjectHierarchyPluginHandler.h"

// SubjectHierarchy MRML includes
#include "vtkMRMLSubjectHierarchyConstants.h"
#include "vtkMRMLSubjectHierarchyNode.h"

// Qt includes
#include <QDebug>
#include <QIcon>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

//----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy_Widgets
class qSlicerSubjectHierarchyDefaultPluginPrivate: public QObject
{
  Q_DECLARE_PUBLIC(qSlicerSubjectHierarchyDefaultPlugin);
protected:
  qSlicerSubjectHierarchyDefaultPlugin* const q_ptr;
public:
  qSlicerSubjectHierarchyDefaultPluginPrivate(qSlicerSubjectHierarchyDefaultPlugin& object);
  ~qSlicerSubjectHierarchyDefaultPluginPrivate();
public:
  QIcon UnknownIcon;

  QIcon VisibleIcon;
  QIcon HiddenIcon;
  QIcon PartiallyVisibleIcon;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyDefaultPluginPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyDefaultPluginPrivate::qSlicerSubjectHierarchyDefaultPluginPrivate(qSlicerSubjectHierarchyDefaultPlugin& object)
: q_ptr(&object)
{
  this->UnknownIcon = QIcon(":Icons/Unknown.png");
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyDefaultPluginPrivate::~qSlicerSubjectHierarchyDefaultPluginPrivate()
{
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyDefaultPlugin::qSlicerSubjectHierarchyDefaultPlugin(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qSlicerSubjectHierarchyDefaultPluginPrivate(*this) )
{
  this->m_Name = QString("Default");
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyDefaultPlugin::~qSlicerSubjectHierarchyDefaultPlugin()
{
}

//----------------------------------------------------------------------------
void qSlicerSubjectHierarchyDefaultPlugin::setDefaultVisibilityIcons(QIcon visibleIcon, QIcon hiddenIcon, QIcon partiallyVisibleIcon)
{
  Q_D(qSlicerSubjectHierarchyDefaultPlugin);

  d->VisibleIcon = visibleIcon;
  d->HiddenIcon = hiddenIcon;
  d->PartiallyVisibleIcon = partiallyVisibleIcon;
}

//---------------------------------------------------------------------------
double qSlicerSubjectHierarchyDefaultPlugin::canOwnSubjectHierarchyNode(vtkMRMLSubjectHierarchyNode* node)const
{
  Q_UNUSED(node);

  // The default Subject Hierarchy plugin is never selected by confidence number it returns
  return 0.0;
}

//---------------------------------------------------------------------------
const QString qSlicerSubjectHierarchyDefaultPlugin::roleForPlugin()const
{
  return "Unknown";
}

//---------------------------------------------------------------------------
const QString qSlicerSubjectHierarchyDefaultPlugin::helpText()const
{
  return QString(
    "<p style=\" margin-top:4px; margin-bottom:1px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
    "<span style=\" font-family:'sans-serif'; font-size:9pt; font-weight:600; color:#000000;\">"
    "Rename Subject hierarchy node"
    "</span>"
    "</p>"
    "<p style=\" margin-top:0px; margin-bottom:11px; margin-left:26px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
    "<span style=\" font-family:'sans-serif'; font-size:9pt; color:#000000;\">"
    "Right-click on the node and select 'Rename'"
    "</span>"
    "</p>"
    "<p style=\" margin-top:4px; margin-bottom:1px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
    "<span style=\" font-family:'sans-serif'; font-size:9pt; font-weight:600; color:#000000;\">"
    "Deform any branch using a transform (registration result)"
    "</span>"
    "</p>"
    "<p style=\" margin-top:0px; margin-bottom:11px; margin-left:26px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
    "<span style=\" font-size:8pt;\">"
    "Make sure the transform column is shown using the 'Display transforms' checkbox. "
    "To transform a branch, double click on the cell in the transform column of the row in question, and choose a transform."
    "</span>"
    "</p>");
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyDefaultPlugin::icon(vtkMRMLSubjectHierarchyNode* node)
{
  if (!node)
    {
    qCritical() << "qSlicerSubjectHierarchyDefaultPlugin::icon: NULL node given!";
    return QIcon();
    }

  Q_D(qSlicerSubjectHierarchyDefaultPlugin);

  // Unknown icon
  // This role is only used when there is no plugin to claim a node, which is an erroneous
  // scenario, as only those nodes can be added to subject hierarchy for which there is at
  // least one plugin that can claim them.
  return d->UnknownIcon;
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyDefaultPlugin::visibilityIcon(int visible)
{
  Q_D(qSlicerSubjectHierarchyDefaultPlugin);

  // Default icon is the eye icon that shows the visibility of the whole branch
  switch (visible)
    {
  case 0:
    return d->HiddenIcon;
  case 1:
    return d->VisibleIcon;
  case 2:
    return d->PartiallyVisibleIcon;
  default:
    return QIcon();
    }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyDefaultPlugin::editProperties(vtkMRMLSubjectHierarchyNode* node)
{
  Q_UNUSED(node);
  // No role, no edit properties
}
