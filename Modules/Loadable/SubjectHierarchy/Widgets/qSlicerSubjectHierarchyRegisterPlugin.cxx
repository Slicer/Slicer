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
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyRegisterPlugin.h"

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"
#include "qSlicerApplication.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLBSplineTransformNode.h>

#ifdef Slicer_BUILD_CLI_SUPPORT
# include <vtkMRMLCommandLineModuleNode.h>
#endif

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkCollection.h>

// Qt includes
#include <QDebug>
#include <QStandardItem>
#include <QAction>
#include <QMenu>
#include <QDialog>
#include <QMainWindow>
#include <QPushButton>

// MRML widgets includes
#include "qMRMLNodeComboBox.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy_Widgets
class qSlicerSubjectHierarchyRegisterPluginPrivate: public QObject
{
  Q_DECLARE_PUBLIC(qSlicerSubjectHierarchyRegisterPlugin);
protected:
  qSlicerSubjectHierarchyRegisterPlugin* const q_ptr;
public:
  qSlicerSubjectHierarchyRegisterPluginPrivate(qSlicerSubjectHierarchyRegisterPlugin& object);
  ~qSlicerSubjectHierarchyRegisterPluginPrivate();
  void init();
public:
  QAction* RegisterThisAction;
  QAction* RegisterToAction;
  QSharedPointer<QMenu> RegistrationMethodsSubMenu;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyRegisterPluginPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyRegisterPluginPrivate::qSlicerSubjectHierarchyRegisterPluginPrivate(qSlicerSubjectHierarchyRegisterPlugin& object)
: q_ptr(&object)
{
  this->RegisterThisAction = NULL;
  this->RegisterToAction = NULL;
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyRegisterPluginPrivate::~qSlicerSubjectHierarchyRegisterPluginPrivate()
{
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyRegisterPlugin::qSlicerSubjectHierarchyRegisterPlugin(QObject* parent)
 : Superclass(parent)
 , m_RegisterFromItem(vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
 , d_ptr( new qSlicerSubjectHierarchyRegisterPluginPrivate(*this) )
{
  this->m_Name = QString("Register");

  Q_D(qSlicerSubjectHierarchyRegisterPlugin);
  d->init();
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegisterPluginPrivate::init()
{
  Q_Q(qSlicerSubjectHierarchyRegisterPlugin);

  this->RegisterThisAction = new QAction("Register this...",q);
  this->RegisterThisAction->setToolTip(tr("Select volume as moving image for registration. "
                                          "Second volume can be selected from context menu after the first one has been set."));
  QObject::connect(this->RegisterThisAction, SIGNAL(triggered()), q, SLOT(registerCurrentNodeTo()));

  this->RegisterToAction = new QAction("Register * to this using...",q);

  // Actions for the registration methods
  this->RegistrationMethodsSubMenu = QSharedPointer<QMenu>(new QMenu());
  this->RegisterToAction->setMenu(this->RegistrationMethodsSubMenu.data());

  QAction* imageBasedRigidAction = new QAction("Rigid image-based registration",q);
  QObject::connect(imageBasedRigidAction, SIGNAL(triggered()), q, SLOT(registerImageBasedRigid()));
  this->RegistrationMethodsSubMenu->addAction(imageBasedRigidAction);

  QAction* imageBasedBSplineAction = new QAction("BSpline image-based registration",q);
  QObject::connect(imageBasedBSplineAction, SIGNAL(triggered()), q, SLOT(registerImageBasedBSpline()));
  this->RegistrationMethodsSubMenu->addAction(imageBasedBSplineAction);

  QAction* interactiveLandmarkAction = new QAction("Interactive landmark registration",q);
  QObject::connect(interactiveLandmarkAction, SIGNAL(triggered()), q, SLOT(registerInteractiveLandmark()));
  this->RegistrationMethodsSubMenu->addAction(interactiveLandmarkAction);
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyRegisterPlugin::~qSlicerSubjectHierarchyRegisterPlugin()
{
}

//---------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyRegisterPlugin::itemContextMenuActions()const
{
  Q_D(const qSlicerSubjectHierarchyRegisterPlugin);

  QList<QAction*> actions;
  actions << d->RegisterThisAction << d->RegisterToAction;
  return actions;
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegisterPlugin::showContextMenuActionsForItem(
  vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemID itemID)
{
  Q_D(qSlicerSubjectHierarchyRegisterPlugin);
  this->hideAllContextMenuActions();

  if (itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    // There are no scene actions in this plugin
    return;
    }

  // Volume
  if (qSlicerSubjectHierarchyPluginHandler::instance()->pluginByName("Volumes")->canOwnSubjectHierarchyItem(itemID))
    {
    // Get current node
    vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemID currentItemID =
      qSlicerSubjectHierarchyPluginHandler::instance()->currentItem();
    if (currentItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
      {
      qCritical() << Q_FUNC_INFO << ": Invalid current node!";
      return;
      }

    // Show 'Register this' action if there is no 'from' node saved
    if (!this->m_RegisterFromItem)
      {
      d->RegisterThisAction->setVisible(true);
      }
    // Show 'Register to' action if 'from' node is saved, and another one is selected
    // (does not make sense to register a volume to itself)
    else if (currentItemID != this->m_RegisterFromItem)
      {
      d->RegisterToAction->setVisible(true);
      }
    }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegisterPlugin::registerCurrentNodeTo()
{
  Q_D(qSlicerSubjectHierarchyRegisterPlugin);

  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }

  vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemID currentItemID =
    qSlicerSubjectHierarchyPluginHandler::instance()->currentItem();
  if (currentItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid current node!";
    return;
    }

  // Save selected node as 'from' node
  this->m_RegisterFromItem = currentItemID;

  d->RegisterToAction->setText( QString("Register %1 to this using...").arg(shNode->GetItemName(currentItemID).c_str()) );
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegisterPlugin::registerImageBasedRigid()
{
#ifdef Slicer_BUILD_CLI_SUPPORT
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }
  vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemID currentItemID =
    qSlicerSubjectHierarchyPluginHandler::instance()->currentItem();
  if (currentItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid current node!";
    return;
    }
  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();
  if (!scene)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid MRML scene!";
    this->m_RegisterFromItem = NULL;
    return;
    }

  // Get volume nodes from selected subject hierarchy nodes
  vtkMRMLNode* registerFromVolumeNode = shNode->GetItemDataNode(this->m_RegisterFromItem);
  vtkMRMLNode* registerToVolumeNode = shNode->GetItemDataNode(currentItemID);
  if (!registerFromVolumeNode || !registerToVolumeNode)
    {
    qCritical() << Q_FUNC_INFO << ": Unable to get input volume nodes from the selected subject hierarchy nodes!";
    this->m_RegisterFromItem = NULL;
    return;
    }

  // Switch to Colors module and set color table as current color node
  qSlicerAbstractModuleWidget* moduleWidget = qSlicerSubjectHierarchyAbstractPlugin::switchToModule("BRAINSFit");
  if (moduleWidget)
    {
    // Create parameter set node
    vtkSmartPointer<vtkMRMLCommandLineModuleNode> registrationParameterSetNode =
      vtkSmartPointer<vtkMRMLCommandLineModuleNode>::New();
    std::string parameterSetNodeName = std::string("BRAINSRegistration_") +
      registerFromVolumeNode->GetName() + "_To_" + registerToVolumeNode->GetName();
    registrationParameterSetNode->SetName(parameterSetNodeName.c_str());
    registrationParameterSetNode->SetAttribute("CommandLineModule","General Registration (BRAINS)");
    registrationParameterSetNode->SetModuleDescription("General Registration (BRAINS)");
    scene->AddNode(registrationParameterSetNode);

    // Set registration input parameters
    registrationParameterSetNode->SetParameterAsString("movingVolume", registerFromVolumeNode->GetID()); // 'From' node is the moving volume
    registrationParameterSetNode->SetParameterAsString("fixedVolume", registerToVolumeNode->GetID()); // 'To' node is the fixed volume

    // Set output transform
    vtkSmartPointer<vtkMRMLLinearTransformNode> outputTransform =
      vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
    std::string outputTransformName = std::string("LinearTransform_") +
      registerFromVolumeNode->GetName() + "_To_" + registerToVolumeNode->GetName();
    outputTransform->SetName(outputTransformName.c_str());
    scene->AddNode(outputTransform);
    registrationParameterSetNode->SetParameterAsString("linearTransform", outputTransform->GetID());

    // Set registration type
    registrationParameterSetNode->SetParameterAsString("useRigid", "true");

    // Get parameter node selector combobox
    qMRMLNodeComboBox* parameterSetNodeSelector = moduleWidget->findChild<qMRMLNodeComboBox*>("MRMLCommandLineModuleNodeSelector");

    // Choose current data node
    if (parameterSetNodeSelector)
      {
      parameterSetNodeSelector->setCurrentNode(registrationParameterSetNode);
      }
    }

  // Reset saved 'from' node
  this->m_RegisterFromItem = NULL;

#else
  qWarning() << Q_FUNC_INFO << ": This operation cannot be performed with CLI disabled";
#endif
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegisterPlugin::registerImageBasedBSpline()
{
#ifdef Slicer_BUILD_CLI_SUPPORT
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }
  vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemID currentItemID =
    qSlicerSubjectHierarchyPluginHandler::instance()->currentItem();
  if (currentItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid current node!";
    return;
    }
  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();
  if (!scene)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid MRML scene!";
    this->m_RegisterFromItem = NULL;
    return;
    }

  // Get volume nodes from selected subject hierarchy nodes
  vtkMRMLNode* registerFromVolumeNode = shNode->GetItemDataNode(this->m_RegisterFromItem);
  vtkMRMLNode* registerToVolumeNode = shNode->GetItemDataNode(currentItemID);
  if (!registerFromVolumeNode || !registerToVolumeNode)
    {
    qCritical() << Q_FUNC_INFO << ": Unable to get input volume nodes from the selected subject hierarchy nodes!";
    this->m_RegisterFromItem = NULL;
    return;
    }

  // Switch to Colors module and set color table as current color node
  qSlicerAbstractModuleWidget* moduleWidget = qSlicerSubjectHierarchyAbstractPlugin::switchToModule("BRAINSFit");
  if (moduleWidget)
    {
    // Create parameter set node
    vtkSmartPointer<vtkMRMLCommandLineModuleNode> registrationParameterSetNode =
      vtkSmartPointer<vtkMRMLCommandLineModuleNode>::New();
    std::string parameterSetNodeName = std::string("BRAINSRegistration_") +
      registerFromVolumeNode->GetName() + "_To_" + registerToVolumeNode->GetName();
    registrationParameterSetNode->SetName(parameterSetNodeName.c_str());
    registrationParameterSetNode->SetAttribute("CommandLineModule","General Registration (BRAINS)");
    registrationParameterSetNode->SetModuleDescription("General Registration (BRAINS)");
    scene->AddNode(registrationParameterSetNode);

    // Set registration input parameters
    registrationParameterSetNode->SetParameterAsString("movingVolume", registerFromVolumeNode->GetID()); // 'From' node is the moving volume
    registrationParameterSetNode->SetParameterAsString("fixedVolume", registerToVolumeNode->GetID()); // 'To' node is the fixed volume

    // Set output transform
    vtkSmartPointer<vtkMRMLBSplineTransformNode> outputTransform =
      vtkSmartPointer<vtkMRMLBSplineTransformNode>::New();
    std::string outputTransformName = std::string("BSplineTransform_") +
      registerFromVolumeNode->GetName() + "_To_" + registerToVolumeNode->GetName();
    outputTransform->SetName(outputTransformName.c_str());
    scene->AddNode(outputTransform);
    registrationParameterSetNode->SetParameterAsString("bsplineTransform", outputTransform->GetID());

    // Set registration type
    registrationParameterSetNode->SetParameterAsString("useBSpline", "true");

    // Get parameter node selector combobox
    qMRMLNodeComboBox* parameterSetNodeSelector = moduleWidget->findChild<qMRMLNodeComboBox*>("MRMLCommandLineModuleNodeSelector");

    // Choose current data node
    if (parameterSetNodeSelector)
      {
      parameterSetNodeSelector->setCurrentNode(registrationParameterSetNode);
      }
    }

  // Reset saved 'from' node
  this->m_RegisterFromItem = NULL;

#else
  qWarning() << Q_FUNC_INFO << ": This operation cannot be performed with CLI disabled";
#endif
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegisterPlugin::registerInteractiveLandmark()
{
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }
  vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemID currentItemID =
    qSlicerSubjectHierarchyPluginHandler::instance()->currentItem();
  if (currentItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid current item!";
    return;
    }
  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();
  if (!scene)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid MRML scene!";
    this->m_RegisterFromItem = NULL;
    return;
    }

  // Get volume nodes from selected subject hierarchy nodes
  vtkMRMLNode* registerFromVolumeNode = shNode->GetItemDataNode(this->m_RegisterFromItem);
  vtkMRMLNode* registerToVolumeNode = shNode->GetItemDataNode(currentItemID);
  if (!registerFromVolumeNode || !registerToVolumeNode)
    {
    qCritical() << Q_FUNC_INFO << ": Unable to get input volume nodes from the selected subject hierarchy nodes!";
    this->m_RegisterFromItem = NULL;
    return;
    }

  // Switch to Colors module and set color table as current color node
  qSlicerAbstractModuleWidget* moduleWidget = qSlicerSubjectHierarchyAbstractPlugin::switchToModule("LandmarkRegistration");
  QApplication::processEvents(); // Make sure the volume selector dialog shows up
  if (moduleWidget)
    {
    // Get volume selector dialog
    QDialog* volumeSelectorDialog = qSlicerApplication::application()->mainWindow()->findChild<QDialog*>("LandmarkRegistrationVolumeSelect");
    if (volumeSelectorDialog)
      {
      // Get volume selector comboboxes
      QList<qMRMLNodeComboBox*> comboboxes = volumeSelectorDialog->findChildren<qMRMLNodeComboBox*>();

      // Set fixed and moving image. Unfortunately currently only the tooltip refers to the role
      foreach (qMRMLNodeComboBox* combobox, comboboxes)
        {
        if (combobox->toolTip().contains("moving", Qt::CaseInsensitive))
          {
          combobox->setCurrentNode(registerFromVolumeNode->GetID());
          }
        else if (combobox->toolTip().contains("fixed", Qt::CaseInsensitive))
          {
          combobox->setCurrentNode(registerToVolumeNode->GetID());
          }
        }

      // Click apply on the volume selector dialog
      QPushButton* applyButton = volumeSelectorDialog->findChild<QPushButton*>("VolumeDialogApply");
      if (applyButton)
        {
        applyButton->click();
        }
      }
    else
      {
      qCritical() << Q_FUNC_INFO << ": Failed to get volume selector dialog for landmark registration module!";
      }
    }

  // Reset saved 'from' node
  this->m_RegisterFromItem = NULL;
}
