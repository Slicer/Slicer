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

// Terminologies includes
#include "qSlicerTerminologyItemDelegate.h"

#include "qSlicerTerminologySelectorButton.h"
#include "qSlicerTerminologyNavigatorWidget.h"

#include "vtkSlicerTerminologiesModuleLogic.h"
#include "vtkSlicerTerminologyEntry.h"

// Slicer includes
#include <qSlicerApplication.h>
#include <qSlicerModuleManager.h>
#include <qSlicerAbstractCoreModule.h>

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
  vtkSlicerTerminologiesModuleLogic* terminologiesLogic = vtkSlicerTerminologiesModuleLogic::SafeDownCast(qSlicerCoreApplication::application()->moduleLogic("Terminologies"));
  if (!terminologiesLogic)
  {
    qCritical() << Q_FUNC_INFO << ": Terminologies logic is not found";
  }
  return terminologiesLogic;
}

//-----------------------------------------------------------------------------
qSlicerTerminologyItemDelegate::qSlicerTerminologyItemDelegate(QObject* parent)
  : QStyledItemDelegate(parent)
{
  // By default do not use terminology selector
  this->UseTerminologySelectorCallback = [] { return false; };
  this->UserSetCustomNameOrColorCallback = [] { return false; };
}

//-----------------------------------------------------------------------------
QWidget* qSlicerTerminologyItemDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  QColor color = index.data(Qt::DecorationRole).value<QColor>();
  QColor invalidColor = QColor(0, 0, 0, 0);
  // QColor(0, 0, 0, 0) is used to discriminate between nodes that have display color properties
  // (which will return QColor != QColor(0, 0, 0, 0)) and nodes without display color properties
  // (which will return a QColor(0, 0, 0, 0))
  if (color == invalidColor)
  {
    return QStyledItemDelegate::createEditor(parent, option, index);
  }

  bool useTerminologySelector = this->UseTerminologySelectorCallback();
  if (useTerminologySelector)
  {
    qSlicerTerminologySelectorButton* terminologyButton = new qSlicerTerminologySelectorButton(parent);
    terminologyButton->setProperty("changeDataOnSet", true);
    connect(terminologyButton, SIGNAL(terminologyChanged()), this, SLOT(commitAndClose()), Qt::QueuedConnection);
    connect(terminologyButton, SIGNAL(canceled()), this, SLOT(close()), Qt::QueuedConnection);
    connect(terminologyButton, SIGNAL(userSetCustomNameOrColor()), this, SLOT(userSetCustomNameOrColor()), Qt::QueuedConnection);
    return terminologyButton;
  }
  else
  {
    if (this->UseDecorationRole)
    {
      // Edit and set color if decoration role is used
      ctkColorPickerButton* colorPicker = new ctkColorPickerButton(parent);
      colorPicker->setProperty("changeColorOnSet", true);
      colorPicker->setDisplayColorName(false);
      ctkColorPickerButton::ColorDialogOptions options = ctkColorPickerButton::ShowAlphaChannel | ctkColorPickerButton::UseCTKColorDialog;
      colorPicker->setDialogOptions(options);
      connect(colorPicker, SIGNAL(colorChanged(QColor)), this, SLOT(commitAndClose()), Qt::QueuedConnection);
      // To Do: add in ctkColorPickerButton a cancel signal and connect it with close method
      return colorPicker;
    }
    else
    {
      // Edit name otherwise
      return QStyledItemDelegate::createEditor(parent, option, index);
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerTerminologyItemDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
  ctkColorPickerButton* colorPickerButton = qobject_cast<ctkColorPickerButton*>(editor);
  qSlicerTerminologySelectorButton* terminologyButton = qobject_cast<qSlicerTerminologySelectorButton*>(editor);

  bool useTerminologySelector = this->UseTerminologySelectorCallback();
  if (!useTerminologySelector)
  {
    if (colorPickerButton)
    {
      QColor color = index.data(Qt::DecorationRole).value<QColor>();
      ctkColorPickerButton* colorPicker = qobject_cast<ctkColorPickerButton*>(editor);
      if (colorPicker) // colorPicker may be nullptr, don't make the application crash when that happens
      {
        colorPicker->blockSignals(true);
        colorPicker->setColor(color);
        colorPicker->blockSignals(false);
        if (colorPicker->property("changeColorOnSet").toBool())
        {
          colorPicker->setProperty("changeColorOnSet", false);
          colorPicker->changeColor();
        }
      }
      else
      {
        qWarning() << Q_FUNC_INFO << " failed: colorPicker is invalid";
      }
    }
    else
    {
      QStyledItemDelegate::setEditorData(editor, index);
    }
  }
  else if (terminologyButton)
  {
    if (!terminologyButton->property("changeDataOnSet").toBool())
    {
      return;
    }
    vtkSlicerTerminologiesModuleLogic* logic = terminologiesLogic();
    terminologyButton->setProperty("changeDataOnSet", false);

    // Convert string list to VTK terminology entry. Do not check success, as an empty terminology is also a valid starting point
    vtkNew<vtkSlicerTerminologyEntry> terminologyEntry;

    if (logic)
    {
      // Get string list value from model index
      QString terminologyString = index.model()->data(index, TerminologyRole).toString();
      if (!terminologyString.isEmpty())
      {
        logic->DeserializeTerminologyEntry(terminologyString.toUtf8().constData(), terminologyEntry);
      }
      // Use default terminology if current terminology is not defined
      if (terminologyEntry->IsEmpty())
      {
        terminologyString = index.model()->data(index, DefaultTerminologyRole).toString();
        if (!terminologyString.isEmpty())
        {
          logic->DeserializeTerminologyEntry(terminologyString.toUtf8().constData(), terminologyEntry);
        }
      }
      // Get default color and other metadata from loaded terminologies, but only for non-color nodes,
      // because terminologies in color nodes are fully defined in the table, so we do not no need to look up additional metadata.
      if (!logic->GetFirstCompatibleColorNodeByName(terminologyEntry->GetTerminologyContextName() ? terminologyEntry->GetTerminologyContextName() : ""))
      {
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
        logic->UpdateEntryFromLoadedTerminologies(terminologyEntry, preferredTerminologyNames, preferredAnatomicalContextNames);
      }
    }

    // Get metadata
    QString name = index.model()->data(index, NameRole).toString();
    bool nameAutoGenerated = index.model()->data(index, NameAutoGeneratedRole).toBool();
    int colorRole = (this->UseDecorationRole ? Qt::DecorationRole : ColorRole);
    QColor color = index.model()->data(index, colorRole).value<QColor>();
    bool colorAutoGenerated = index.model()->data(index, ColorAutoGeneratedRole).toBool();
    QColor generatedColor = index.model()->data(index, qSlicerTerminologyItemDelegate::GeneratedColorRole).value<QColor>();

    qSlicerTerminologyNavigatorWidget::TerminologyInfoBundle terminologyInfo(terminologyEntry, name, nameAutoGenerated, color, colorAutoGenerated, generatedColor);
    terminologyButton->setTerminologyInfo(terminologyInfo);

    terminologyButton->changeTerminology();
  }
}

//-----------------------------------------------------------------------------
void qSlicerTerminologyItemDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
  QLineEdit* lineEdit = qobject_cast<QLineEdit*>(editor);
  ctkColorPickerButton* colorPickerButton = qobject_cast<ctkColorPickerButton*>(editor);
  qSlicerTerminologySelectorButton* terminologyButton = qobject_cast<qSlicerTerminologySelectorButton*>(editor);
  if (lineEdit)
  {
    QStyledItemDelegate::setModelData(editor, model, index);
    model->setData(index, lineEdit->text(), NameRole);
    return;
  }
  else if (colorPickerButton)
  {
    ctkColorPickerButton* colorPicker = qobject_cast<ctkColorPickerButton*>(editor);
    Q_ASSERT(colorPicker);
    QColor color = colorPicker->color();
    model->setData(index, color, Qt::DecorationRole);
  }
  else if (terminologyButton)
  {
    vtkSlicerTerminologiesModuleLogic* logic = terminologiesLogic();
    if (!logic)
    {
      return;
    }
    // Get terminology (changed by the user) from the terminology button
    qSlicerTerminologyNavigatorWidget::TerminologyInfoBundle terminologyInfo;
    terminologyButton->terminologyInfo(terminologyInfo);

    // Set color to model
    model->setData(index, terminologyInfo.ColorAutoGenerated, ColorAutoGeneratedRole);
    int colorRole = (this->UseDecorationRole ? Qt::DecorationRole : ColorRole);
    model->setData(index, terminologyInfo.Color, colorRole);
    // Set name to model
    model->setData(index, terminologyInfo.NameAutoGenerated, NameAutoGeneratedRole);
    model->setData(index, terminologyInfo.Name, NameRole);
    // Set terminology string to model
    model->setData(index, logic->SerializeTerminologyEntry(terminologyInfo.GetTerminologyEntry()).c_str(), TerminologyRole);
  }
}

//-----------------------------------------------------------------------------
void qSlicerTerminologyItemDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& /* index */) const
{
  editor->setGeometry(option.rect);
}

//------------------------------------------------------------------------------
void qSlicerTerminologyItemDelegate::commitSenderData()
{
  QWidget* editor = qobject_cast<QWidget*>(this->sender());
  if (!editor)
  {
    return;
  }
  emit commitData(editor);
}

//------------------------------------------------------------------------------
void qSlicerTerminologyItemDelegate::commitAndClose()
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
void qSlicerTerminologyItemDelegate::close()
{
  QWidget* editor = qobject_cast<QWidget*>(this->sender());
  if (!editor)
  {
    return;
  }
  emit closeEditor(editor);
}

//------------------------------------------------------------------------------
void qSlicerTerminologyItemDelegate::setUseTerminologySelectorCallback(std::function<bool()> callback)
{
  this->UseTerminologySelectorCallback = callback;
}

//------------------------------------------------------------------------------
void qSlicerTerminologyItemDelegate::setUserSetCustomNameOrColorCallback(std::function<bool()> callback)
{
  this->UserSetCustomNameOrColorCallback = callback;
}

//------------------------------------------------------------------------------
void qSlicerTerminologyItemDelegate::userSetCustomNameOrColor()
{
  this->UserSetCustomNameOrColorCallback();
}

//------------------------------------------------------------------------------
void qSlicerTerminologyItemDelegate::setUseDecorationRole(bool useDecorationRole)
{
  this->UseDecorationRole = useDecorationRole;
}

//------------------------------------------------------------------------------
bool qSlicerTerminologyItemDelegate::useDecorationRole()
{
  return this->UseDecorationRole;
}
