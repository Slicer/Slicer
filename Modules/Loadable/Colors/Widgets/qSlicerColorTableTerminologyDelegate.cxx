/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Seattle Children's Hospital d/b/a Seattle Children's Research Institute.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, EBATINCA, S.L.
  and was funded by by Murat Maga (Seattle Children's Research Institute).

==============================================================================*/

// Terminologies includes
#include "qSlicerColorTableTerminologyDelegate.h"

#include "qSlicerTerminologySelectorButton.h"
#include "qSlicerTerminologyNavigatorWidget.h"

#include "vtkSlicerTerminologiesModuleLogic.h"
#include "vtkSlicerTerminologyEntry.h"

// Colors includes
#include "qSlicerTerminologyEditorButton.h"

// Slicer includes
#include <qSlicerApplication.h>
#include <qSlicerCoreApplication.h>
#include <qSlicerModuleManager.h>
#include <qSlicerAbstractCoreModule.h>

// MRMLWidgets includes
#include "qMRMLItemDelegate.h"
#include "qMRMLColorModel.h"

// MRML includes
#include "vtkMRMLColorTableNode.h"

// VTK includes
#include <vtkSmartPointer.h>

// CTK includes
#include <ctkColorPickerButton.h>

// Qt includes
#include <QDebug>
#include <QLineEdit>
#include <QSettings>

//-----------------------------------------------------------------------------
vtkSlicerTerminologiesModuleLogic* terminologiesLogic()
{
  vtkSlicerTerminologiesModuleLogic* terminologiesLogic = vtkSlicerTerminologiesModuleLogic::SafeDownCast(
    qSlicerCoreApplication::application()->moduleLogic("Terminologies"));
  if (!terminologiesLogic)
  {
    qCritical() << Q_FUNC_INFO << ": Terminologies logic is not found";
  }
  return terminologiesLogic;
}

//-----------------------------------------------------------------------------
qSlicerColorTableTerminologyDelegate::qSlicerColorTableTerminologyDelegate(QObject* parent)
  : QStyledItemDelegate(parent)
{
}

//-----------------------------------------------------------------------------
QWidget* qSlicerColorTableTerminologyDelegate::createEditor(QWidget* parent,
                                                            const QStyleOptionViewItem& option,
                                                            const QModelIndex& index) const
{
  qSlicerTerminologyEditorButton* editorButton = new qSlicerTerminologyEditorButton(parent);
  editorButton->setProperty("changeDataOnSet", true);
  connect(editorButton, SIGNAL(terminologyChanged()), this, SLOT(commitAndClose()), Qt::QueuedConnection);
  connect(editorButton, SIGNAL(canceled()), this, SLOT(close()), Qt::QueuedConnection);

  return editorButton;
}

//-----------------------------------------------------------------------------
void qSlicerColorTableTerminologyDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
  qSlicerTerminologyEditorButton* editorButton = qobject_cast<qSlicerTerminologyEditorButton*>(editor);
  if (editorButton)
  {
    if (!editorButton->property("changeDataOnSet").toBool())
    {
      return;
    }
    vtkSlicerTerminologiesModuleLogic* logic = terminologiesLogic();
    editorButton->setProperty("changeDataOnSet", false);

    int colorIndex = index.data(qMRMLItemDelegate::ColorEntryRole).toInt();

    QVariant nodePointerVar = index.data(qMRMLItemDelegate::PointerRole);
    if (!nodePointerVar.isValid())
    {
      return;
    }
    vtkMRMLColorNode* colorNode = reinterpret_cast<vtkMRMLColorNode*>(nodePointerVar.toLongLong());
    if (colorNode == nullptr)
    {
      return;
    }

    QString terminologyString(colorNode->GetTerminologyAsString(colorIndex).c_str());

    // Convert string to VTK terminology entry. Do not check success, as an empty terminology is also a valid starting
    // point
    vtkNew<vtkSlicerTerminologyEntry> terminologyEntry;
    if (logic)
    {
      logic->DeserializeTerminologyEntry(terminologyString.toUtf8().constData(), terminologyEntry);
      // Get default color and other metadata from loaded terminologies
      // Get list of last terminology contexts selected by the user from application settings
      std::vector<std::string> preferredTerminologyNames;
      QSettings* settings = qSlicerApplication::application()->settingsDialog()->settings();
      if (settings->contains("Terminology/LastTerminologyContexts"))
      {
        QStringList lastTerminologyContextNames = settings->value("Terminology/LastTerminologyContexts").toStringList();
        for (auto& name : lastTerminologyContextNames)
        {
          preferredTerminologyNames.push_back(name.toStdString().c_str());
        }
      }
      std::vector<std::string> preferredAnatomicalContextNames; // use default order for now
      logic->UpdateEntryFromLoadedTerminologies(
        terminologyEntry, preferredTerminologyNames, preferredAnatomicalContextNames);
    }

    // Set terminology information to the button creating the editor dialog
    double rgba[4] = { 0.0, 0.0, 0.0, 1.0 };
    colorNode->GetColor(colorIndex, rgba);
    // Overwrite color and name in the color table by default if color was undefined
    bool colorAndNameAutoGenerated = !colorNode->GetColorDefined(colorIndex);
    QColor color = QColor::fromRgbF(rgba[0], rgba[1], rgba[2], rgba[3]);
    qSlicerTerminologyNavigatorWidget::TerminologyInfoBundle terminologyInfo(terminologyEntry,
                                                                             colorNode->GetColorName(colorIndex),
                                                                             colorAndNameAutoGenerated,
                                                                             color,
                                                                             colorAndNameAutoGenerated,
                                                                             color);
    editorButton->setTerminologyInfo(terminologyInfo);
    // Show dialog
    editorButton->changeTerminology();
  }
}

//-----------------------------------------------------------------------------
void qSlicerColorTableTerminologyDelegate::setModelData(QWidget* editor,
                                                        QAbstractItemModel* model,
                                                        const QModelIndex& index) const
{
  vtkSlicerTerminologiesModuleLogic* logic = terminologiesLogic();
  if (!logic)
  {
    return;
  }

  int colorIndex = index.data(qMRMLItemDelegate::ColorEntryRole).toInt();
  QVariant nodePointerVar = index.data(qMRMLItemDelegate::PointerRole);
  if (!nodePointerVar.isValid())
  {
    return;
  }
  vtkMRMLColorNode* colorNode = reinterpret_cast<vtkMRMLColorNode*>(nodePointerVar.toLongLong());
  if (colorNode == nullptr)
  {
    return;
  }

  // Get terminology (changed by the user) from the terminology button
  qSlicerTerminologyNavigatorWidget::TerminologyInfoBundle terminologyInfo;
  qSlicerTerminologyEditorButton* terminologyButton = qobject_cast<qSlicerTerminologyEditorButton*>(editor);
  terminologyButton->terminologyInfo(terminologyInfo);

  // Set color
  vtkMRMLColorTableNode* colorTableNode = vtkMRMLColorTableNode::SafeDownCast(colorNode);
  if (colorTableNode != nullptr)
  {
    colorTableNode->SetColorName(colorIndex, terminologyInfo.Name.toUtf8().constData());
    colorTableNode->SetColor(colorIndex,
                             terminologyInfo.Color.redF(),
                             terminologyInfo.Color.greenF(),
                             terminologyInfo.Color.blueF(),
                             terminologyInfo.Color.alphaF());
  }

  // Set empty terminology context name if not imported from loaded terminology context
  vtkSlicerTerminologyEntry* entry = terminologyInfo.GetTerminologyEntry();
  if (entry->GetTerminologyContextName() == nullptr)
  {
    entry->SetTerminologyContextName("");
  }

  colorNode->SetTerminologyFromString(colorIndex, logic->SerializeTerminologyEntry(entry));

  model->setData(index, qMRMLColorModel::terminologyTextForColor(colorNode, colorIndex), Qt::DisplayRole);
}

//-----------------------------------------------------------------------------
void qSlicerColorTableTerminologyDelegate::updateEditorGeometry(QWidget* editor,
                                                                const QStyleOptionViewItem& option,
                                                                const QModelIndex& /* index */) const
{
  editor->setGeometry(option.rect);
}

//------------------------------------------------------------------------------
void qSlicerColorTableTerminologyDelegate::commitSenderData()
{
  QWidget* editor = qobject_cast<QWidget*>(this->sender());
  if (!editor)
  {
    return;
  }
  emit commitData(editor);
}

//------------------------------------------------------------------------------
void qSlicerColorTableTerminologyDelegate::commitAndClose()
{
  QWidget* editor = qobject_cast<QWidget*>(this->sender());
  if (!editor)
  {
    return;
  }
  emit commitData(editor);
  emit closeEditor(editor);
}

//------------------------------------------------------------------------------
void qSlicerColorTableTerminologyDelegate::close()
{
  QWidget* editor = qobject_cast<QWidget*>(this->sender());
  if (!editor)
  {
    return;
  }
  emit closeEditor(editor);
}
