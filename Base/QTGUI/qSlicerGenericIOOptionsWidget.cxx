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

==============================================================================*/

// Qt includes
#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDebug>
#include <QDoubleSpinBox>
#include <QBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPointer>
#include <QSpinBox>

// STD includes
#include <algorithm>
#include <cmath>
#include <limits>

// MRML widgets includes
#include <qMRMLColorTableComboBox.h>
#include <qMRMLNodeComboBox.h>

// Slicer includes
#include "qSlicerCoreApplication.h"
#include "qSlicerGenericIOOptionsWidget.h"
#include "qSlicerIOOptions_p.h"
#include <vtkSlicerApplicationLogic.h>
#include <vtkMRMLFileIOManager.h>
#include <vtkMRMLFileReader.h>
#include <vtkMRMLFileWriter.h>
#include <vtkMRMLIOOptionsDescription.h>
#include <vtkMRMLIOProperties.h>

// MRML includes
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkNew.h>

namespace
{
//-----------------------------------------------------------------------------
/// Convert VTK value to QVariant, keeping the value type: string is converted to QString, floating-point
/// number to double, other numbers to int (or qlonglong if they are out of int range).
QVariant vtkVariantToQVariant(const vtkVariant& value)
{
  if (!value.IsValid())
  {
    return QVariant();
  }
  if (value.IsString())
  {
    return QVariant(QString::fromStdString(value.ToString()));
  }
  if (value.IsFloat() || value.IsDouble())
  {
    return QVariant(value.ToDouble());
  }
  if (value.IsNumeric())
  {
    const long long longValue = value.ToLongLong();
    if (longValue >= std::numeric_limits<int>::min() && longValue <= std::numeric_limits<int>::max())
    {
      return QVariant(static_cast<int>(longValue));
    }
    return QVariant(static_cast<qlonglong>(longValue));
  }
  return QVariant(QString::fromStdString(value.ToString()));
}

//-----------------------------------------------------------------------------
/// Set range of a spin box. Non-finite limits (infinity) are replaced by the limits of the value type.
template <class SpinBoxType, class ValueType>
void setSpinBoxRange(SpinBoxType* spinBox, double minimum, double maximum)
{
  const double lowest = static_cast<double>(std::numeric_limits<ValueType>::lowest());
  const double highest = static_cast<double>(std::numeric_limits<ValueType>::max());
  minimum = std::isfinite(minimum) ? std::max(minimum, lowest) : lowest;
  maximum = std::isfinite(maximum) ? std::min(maximum, highest) : highest;
  spinBox->setRange(static_cast<ValueType>(minimum), static_cast<ValueType>(maximum));
}
} // namespace

//-----------------------------------------------------------------------------
class qSlicerGenericIOOptionsWidgetPrivate : public qSlicerIOOptionsPrivate
{
public:
  /// Widgets of an option
  struct OptionWidgets
  {
    QString Type;
    QString WidgetHint;
    QString Separator;
    QPointer<QLabel> Label;
    QPointer<QWidget> Control;
  };

  vtkSmartPointer<vtkMRMLFileIOHandler> IOHandler;
  /// Values that the user set
  QVariantMap EditedValues;
  /// Values set by updateGUI
  QVariantMap SpecifiedValues;
  /// ID of the node to write (for writers)
  QString NodeID;
  QMap<QString, OptionWidgets> Options;
  QStringList OptionOrder;
  bool UpdatingOptions{ false };
};

//-----------------------------------------------------------------------------
qSlicerGenericIOOptionsWidget::qSlicerGenericIOOptionsWidget(QWidget* parentWidget)
  : Superclass(new qSlicerGenericIOOptionsWidgetPrivate, parentWidget)
{
  // All options are displayed in a single row (the options widget is displayed
  // in a single row of a table in the data dialogs)
  QHBoxLayout* layout = new QHBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  this->setLayout(layout);
  if (QCoreApplication::instance())
  {
    QObject::connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()), this, SLOT(onApplicationAboutToQuit()));
  }
}

//-----------------------------------------------------------------------------
qSlicerGenericIOOptionsWidget::qSlicerGenericIOOptionsWidget(vtkMRMLFileIOHandler* ioHandler, QWidget* parentWidget)
  : qSlicerGenericIOOptionsWidget(parentWidget)
{
  this->setIOHandler(ioHandler);
}

//-----------------------------------------------------------------------------
qSlicerGenericIOOptionsWidget::~qSlicerGenericIOOptionsWidget() = default;

//-----------------------------------------------------------------------------
void qSlicerGenericIOOptionsWidget::setIOHandler(vtkMRMLFileIOHandler* ioHandler)
{
  Q_D(qSlicerGenericIOOptionsWidget);
  d->IOHandler = ioHandler;
  if (ioHandler && !this->mrmlScene() && ioHandler->GetScene())
  {
    // Node selectors need a scene
    this->setMRMLScene(ioHandler->GetScene());
  }
  this->updateOptions();
}

//-----------------------------------------------------------------------------
vtkMRMLFileIOHandler* qSlicerGenericIOOptionsWidget::ioHandler() const
{
  Q_D(const qSlicerGenericIOOptionsWidget);
  return d->IOHandler;
}

//-----------------------------------------------------------------------------
vtkMRMLFileIOHandler* qSlicerGenericIOOptionsWidget::findIOHandler(const char* className)
{
  qSlicerCoreApplication* app = qSlicerCoreApplication::application();
  vtkMRMLFileIOManager* fileIOManager = (app && app->applicationLogic()) ? app->applicationLogic()->GetFileIOManager() : nullptr;
  if (!fileIOManager || !className)
  {
    return nullptr;
  }
  for (int i = 0; i < fileIOManager->GetNumberOfReaders(); ++i)
  {
    vtkMRMLFileIOHandler* handler = fileIOManager->GetNthReader(i);
    if (strcmp(handler->GetClassName(), className) == 0)
    {
      return handler;
    }
  }
  for (int i = 0; i < fileIOManager->GetNumberOfWriters(); ++i)
  {
    vtkMRMLFileIOHandler* handler = fileIOManager->GetNthWriter(i);
    if (strcmp(handler->GetClassName(), className) == 0)
    {
      return handler;
    }
  }
  return nullptr;
}

//-----------------------------------------------------------------------------
void qSlicerGenericIOOptionsWidget::initializeUnregisteredIOHandler(vtkMRMLFileIOHandler* handler)
{
  qSlicerCoreApplication* app = qSlicerCoreApplication::application();
  if (!handler || !app)
  {
    return;
  }
  handler->SetScene(app->mrmlScene());
  if (app->applicationLogic())
  {
    // The handler is not registered, but the manager gives access to the application logic
    handler->SetFileIOManager(app->applicationLogic()->GetFileIOManager());
  }
}

//-----------------------------------------------------------------------------
void qSlicerGenericIOOptionsWidget::setFileName(const QString& fileName)
{
  Q_D(qSlicerGenericIOOptionsWidget);
  this->Superclass::setFileName(fileName);
  // Default values depend on the file name, so previous edits are discarded
  d->EditedValues.clear();
  this->updateOptions();
}

//-----------------------------------------------------------------------------
void qSlicerGenericIOOptionsWidget::setFileNames(const QStringList& fileNames)
{
  Q_D(qSlicerGenericIOOptionsWidget);
  this->Superclass::setFileNames(fileNames);
  // Default values depend on the file names, so previous edits are discarded
  d->EditedValues.clear();
  this->updateOptions();
}

//-----------------------------------------------------------------------------
void qSlicerGenericIOOptionsWidget::setObject(vtkObject* object)
{
  Q_D(qSlicerGenericIOOptionsWidget);
  vtkMRMLNode* node = vtkMRMLNode::SafeDownCast(object);
  d->NodeID = (node && node->GetID()) ? QString::fromUtf8(node->GetID()) : QString();
  if (d->NodeID.isEmpty())
  {
    d->Properties.remove("nodeID");
  }
  else
  {
    d->Properties["nodeID"] = d->NodeID;
  }
  // Default values depend on the object, so previous edits are discarded
  d->EditedValues.clear();
  this->updateOptions();
}

//-----------------------------------------------------------------------------
void qSlicerGenericIOOptionsWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qSlicerGenericIOOptionsWidget);
  this->Superclass::setMRMLScene(scene);
  // Node selectors may select a node automatically when the scene is set.
  // That is not a change made by the user, so the selected node is restored.
  const bool wasUpdatingOptions = d->UpdatingOptions;
  d->UpdatingOptions = true;
  for (const QString& property : d->OptionOrder)
  {
    qMRMLNodeComboBox* nodeComboBox = qobject_cast<qMRMLNodeComboBox*>(d->Options[property].Control);
    if (nodeComboBox)
    {
      nodeComboBox->setMRMLScene(scene);
      nodeComboBox->setCurrentNodeID(d->Properties.value(property).toString());
    }
  }
  d->UpdatingOptions = wasUpdatingOptions;
  // Options are not updated here: the description does not depend on the scene of the widget
  // (and the scene is set to nullptr when the application is shutting down, when the reader
  // or writer may not be able to provide a description anymore).
}

//-----------------------------------------------------------------------------
void qSlicerGenericIOOptionsWidget::updateGUI(const qSlicerIO::IOProperties& ioProperties)
{
  Q_D(qSlicerGenericIOOptionsWidget);
  this->Superclass::updateGUI(ioProperties);
  for (qSlicerIO::IOProperties::const_iterator it = ioProperties.constBegin(); it != ioProperties.constEnd(); ++it)
  {
    if (it.key() == "fileName" || it.key() == "fileNames" || it.key() == "nodeID" || it.key() == "fileType")
    {
      continue;
    }
    d->SpecifiedValues[it.key()] = it.value();
    d->EditedValues.remove(it.key());
  }
  this->updateOptions();
}

//-----------------------------------------------------------------------------
void qSlicerGenericIOOptionsWidget::onApplicationAboutToQuit()
{
  Q_D(qSlicerGenericIOOptionsWidget);
  // Changes of the node selectors must not update the options
  d->UpdatingOptions = true;
  for (const qSlicerGenericIOOptionsWidgetPrivate::OptionWidgets& widgets : d->Options)
  {
    if (qMRMLNodeComboBox* nodeComboBox = qobject_cast<qMRMLNodeComboBox*>(widgets.Control))
    {
      nodeComboBox->setMRMLScene(nullptr);
    }
  }
  d->UpdatingOptions = false;
}

//-----------------------------------------------------------------------------
void qSlicerGenericIOOptionsWidget::onOptionChanged()
{
  Q_D(qSlicerGenericIOOptionsWidget);
  if (d->UpdatingOptions)
  {
    return;
  }
  if (!QCoreApplication::instance() || QCoreApplication::closingDown())
  {
    // The widget may still exist when the application exits (see onApplicationAboutToQuit()),
    // widgets must not be updated anymore.
    return;
  }
  QWidget* control = qobject_cast<QWidget*>(this->sender());
  QString property = control ? control->property("ioOptionProperty").toString() : QString();
  if (property.isEmpty() || !d->Options.contains(property))
  {
    return;
  }
  const qSlicerGenericIOOptionsWidgetPrivate::OptionWidgets& option = d->Options[property];
  QVariant value;
  if (QCheckBox* checkBox = qobject_cast<QCheckBox*>(control))
  {
    value = checkBox->isChecked();
  }
  else if (QSpinBox* spinBox = qobject_cast<QSpinBox*>(control))
  {
    value = spinBox->value();
  }
  else if (QDoubleSpinBox* doubleSpinBox = qobject_cast<QDoubleSpinBox*>(control))
  {
    value = doubleSpinBox->value();
  }
  else if (QLineEdit* lineEdit = qobject_cast<QLineEdit*>(control))
  {
    if (option.Type == "stringList")
    {
      QStringList items;
      for (const QString& item : lineEdit->text().split(option.Separator))
      {
        if (!item.trimmed().isEmpty())
        {
          items << item.trimmed();
        }
      }
      value = items;
    }
    else
    {
      value = lineEdit->text();
    }
  }
  else if (qMRMLNodeComboBox* nodeComboBox = qobject_cast<qMRMLNodeComboBox*>(control))
  {
    value = nodeComboBox->currentNodeID();
  }
  else if (QComboBox* comboBox = qobject_cast<QComboBox*>(control))
  {
    value = comboBox->currentData();
  }
  d->EditedValues[property] = value;
  this->updateOptions();
}

//-----------------------------------------------------------------------------
void qSlicerGenericIOOptionsWidget::updateOptions()
{
  Q_D(qSlicerGenericIOOptionsWidget);
  if (!d->IOHandler || d->UpdatingOptions)
  {
    return;
  }

  // Request the description with the current context and values
  qSlicerIO::IOProperties context;
  if (d->Properties.contains("fileName"))
  {
    // setFileNames() stores the list of file names in the fileName property
    QVariant fileNameValue = d->Properties["fileName"];
    if (fileNameValue.userType() == QMetaType::QStringList)
    {
      QStringList fileNames = fileNameValue.toStringList();
      context["fileNames"] = fileNames;
      if (!fileNames.isEmpty())
      {
        context["fileName"] = fileNames.first();
      }
    }
    else
    {
      context["fileName"] = fileNameValue;
    }
  }
  if (!d->NodeID.isEmpty())
  {
    context["nodeID"] = d->NodeID;
  }
  for (QVariantMap::const_iterator it = d->SpecifiedValues.constBegin(); it != d->SpecifiedValues.constEnd(); ++it)
  {
    context[it.key()] = it.value();
  }
  for (QVariantMap::const_iterator it = d->EditedValues.constBegin(); it != d->EditedValues.constEnd(); ++it)
  {
    context[it.key()] = it.value();
  }
  vtkNew<vtkMRMLIOProperties> properties;
  qSlicerIO::toVTKProperties(context, properties);
  vtkNew<vtkMRMLIOOptionsDescription> description;
  d->IOHandler->FillOptionsDescription(properties, description);

  d->UpdatingOptions = true;
  QStringList optionOrder;
  for (int optionIndex = 0; optionIndex < description->GetNumberOfOptions(); ++optionIndex)
  {
    const QString property = QString::fromStdString(description->GetNthOptionProperty(optionIndex));
    const QString type = QString::fromStdString(description->GetNthOptionType(optionIndex));
    const QString widgetHint = QString::fromStdString(description->GetNthOptionWidget(optionIndex));
    if (property.isEmpty())
    {
      continue;
    }
    optionOrder << property;

    // Create widgets (or re-create if the type changed)
    qSlicerGenericIOOptionsWidgetPrivate::OptionWidgets& widgets = d->Options[property];
    if (widgets.Control.isNull() || widgets.Type != type || widgets.WidgetHint != widgetHint)
    {
      delete widgets.Label;
      delete widgets.Control;
      widgets.Type = type;
      widgets.WidgetHint = widgetHint;
      QWidget* control = nullptr;
      if (type == "bool")
      {
        QCheckBox* checkBox = new QCheckBox(this);
        QObject::connect(checkBox, SIGNAL(toggled(bool)), this, SLOT(onOptionChanged()));
        control = checkBox;
      }
      else if (type == "int")
      {
        QSpinBox* spinBox = new QSpinBox(this);
        QObject::connect(spinBox, SIGNAL(valueChanged(int)), this, SLOT(onOptionChanged()));
        control = spinBox;
      }
      else if (type == "double")
      {
        QDoubleSpinBox* spinBox = new QDoubleSpinBox(this);
        QObject::connect(spinBox, SIGNAL(valueChanged(double)), this, SLOT(onOptionChanged()));
        control = spinBox;
      }
      else if (type == "string" || type == "stringList")
      {
        QLineEdit* lineEdit = new QLineEdit(this);
        QObject::connect(lineEdit, SIGNAL(textEdited(QString)), this, SLOT(onOptionChanged()));
        control = lineEdit;
      }
      else if (type == "enum")
      {
        QComboBox* comboBox = new QComboBox(this);
        QObject::connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onOptionChanged()));
        control = comboBox;
      }
      else if (type == "node")
      {
        qMRMLNodeComboBox* nodeComboBox = (widgetHint == "colorTable") ? new qMRMLColorTableComboBox(this) : new qMRMLNodeComboBox(this);
        nodeComboBox->setAddEnabled(false);
        nodeComboBox->setRemoveEnabled(false);
        nodeComboBox->setRenameEnabled(false);
        QObject::connect(nodeComboBox, SIGNAL(currentNodeIDChanged(QString)), this, SLOT(onOptionChanged()));
        control = nodeComboBox;
      }
      else
      {
        qCritical() << Q_FUNC_INFO << "Unsupported option type" << type << "for property" << property;
        continue;
      }
      control->setObjectName(property + "OptionWidget");
      control->setProperty("ioOptionProperty", property);
      if (type != "bool")
      {
        widgets.Label = new QLabel(this);
        this->layout()->addWidget(widgets.Label);
      }
      this->layout()->addWidget(control);
      widgets.Control = control;
    }

    // Update attributes
    QWidget* control = widgets.Control;
    const QString label = QString::fromStdString(description->GetNthOptionLabel(optionIndex));
    const QString toolTip = QString::fromStdString(description->GetNthOptionToolTip(optionIndex));
    const bool enabled = description->GetNthOptionEnabled(optionIndex);
    const bool visible = description->GetNthOptionVisible(optionIndex);
    control->setToolTip(toolTip);
    control->setEnabled(enabled);
    control->setVisible(visible);
    if (widgets.Label)
    {
      widgets.Label->setText(label);
      widgets.Label->setToolTip(toolTip);
      widgets.Label->setEnabled(enabled);
      widgets.Label->setVisible(visible && !label.isEmpty());
    }

    // Update value (values that the user set are kept)
    const vtkVariant value = (type == "stringList") ? vtkVariant() : description->GetNthOptionValue(optionIndex);
    const bool updateValue = !d->EditedValues.contains(property);
    if (QCheckBox* checkBox = qobject_cast<QCheckBox*>(control))
    {
      checkBox->setText(label);
      if (updateValue)
      {
        checkBox->setChecked(value.ToInt() != 0);
      }
      d->Properties[property] = checkBox->isChecked();
    }
    else if (QSpinBox* spinBox = qobject_cast<QSpinBox*>(control))
    {
      if (description->GetNthOptionHasRange(optionIndex))
      {
        setSpinBoxRange<QSpinBox, int>(spinBox, description->GetNthOptionMinimum(optionIndex), description->GetNthOptionMaximum(optionIndex));
      }
      if (updateValue)
      {
        spinBox->setValue(value.ToInt());
      }
      d->Properties[property] = spinBox->value();
    }
    else if (QDoubleSpinBox* doubleSpinBox = qobject_cast<QDoubleSpinBox*>(control))
    {
      if (description->GetNthOptionDecimals(optionIndex) >= 0)
      {
        doubleSpinBox->setDecimals(description->GetNthOptionDecimals(optionIndex));
      }
      if (description->GetNthOptionHasRange(optionIndex))
      {
        setSpinBoxRange<QDoubleSpinBox, double>(doubleSpinBox, description->GetNthOptionMinimum(optionIndex), description->GetNthOptionMaximum(optionIndex));
      }
      if (updateValue)
      {
        doubleSpinBox->setValue(value.ToDouble());
      }
      d->Properties[property] = doubleSpinBox->value();
    }
    else if (QLineEdit* lineEdit = qobject_cast<QLineEdit*>(control))
    {
      if (type == "stringList")
      {
        widgets.Separator = QString::fromStdString(description->GetNthOptionSeparator(optionIndex));
        if (widgets.Separator.isEmpty())
        {
          widgets.Separator = ";";
        }
        QStringList items;
        for (const std::string& item : description->GetNthOptionStringListValue(optionIndex))
        {
          items << QString::fromStdString(item);
        }
        if (updateValue)
        {
          lineEdit->setText(items.join(widgets.Separator + " "));
        }
        else
        {
          items = d->EditedValues[property].toStringList();
        }
        if (items.isEmpty())
        {
          d->Properties.remove(property);
        }
        else
        {
          d->Properties[property] = items;
        }
      }
      else
      {
        if (updateValue)
        {
          lineEdit->setText(QString::fromStdString(value.ToString()));
        }
        if (lineEdit->text().isEmpty())
        {
          d->Properties.remove(property);
        }
        else
        {
          d->Properties[property] = lineEdit->text();
        }
      }
    }
    else if (qMRMLNodeComboBox* nodeComboBox = qobject_cast<qMRMLNodeComboBox*>(control))
    {
      QStringList nodeClasses;
      for (const std::string& nodeClass : description->GetNthOptionNodeClasses(optionIndex))
      {
        nodeClasses << QString::fromStdString(nodeClass);
      }
      if (nodeComboBox->nodeTypes() != nodeClasses)
      {
        nodeComboBox->setNodeTypes(nodeClasses);
      }
      nodeComboBox->setNoneEnabled(description->GetNthOptionNoneEnabled(optionIndex));
      nodeComboBox->setShowHidden(description->GetNthOptionShowHidden(optionIndex));
      if (nodeComboBox->mrmlScene() != this->mrmlScene())
      {
        nodeComboBox->setMRMLScene(this->mrmlScene());
      }
      if (updateValue)
      {
        nodeComboBox->setCurrentNodeID(QString::fromStdString(value.ToString()));
      }
      d->Properties[property] = nodeComboBox->currentNodeID();
    }
    else if (QComboBox* comboBox = qobject_cast<QComboBox*>(control))
    {
      // Update choices if changed
      const int numberOfChoices = description->GetNthOptionNumberOfChoices(optionIndex);
      bool choicesChanged = (comboBox->count() != numberOfChoices);
      for (int i = 0; !choicesChanged && i < numberOfChoices; ++i)
      {
        choicesChanged = (comboBox->itemText(i) != QString::fromStdString(description->GetNthOptionChoiceLabel(optionIndex, i)) //
                          || comboBox->itemData(i) != vtkVariantToQVariant(description->GetNthOptionChoiceValue(optionIndex, i)));
      }
      if (choicesChanged)
      {
        QVariant currentValue = comboBox->currentData();
        comboBox->clear();
        for (int i = 0; i < numberOfChoices; ++i)
        {
          comboBox->addItem(QString::fromStdString(description->GetNthOptionChoiceLabel(optionIndex, i)), //
                            vtkVariantToQVariant(description->GetNthOptionChoiceValue(optionIndex, i)));
        }
        comboBox->setCurrentIndex(comboBox->findData(currentValue));
      }
      if (updateValue)
      {
        comboBox->setCurrentIndex(comboBox->findData(vtkVariantToQVariant(value)));
      }
      if (comboBox->currentIndex() >= 0)
      {
        d->Properties[property] = comboBox->currentData();
      }
      else
      {
        d->Properties.remove(property);
      }
    }
  }

  // Remove widgets of options that are not described anymore
  for (const QString& property : d->OptionOrder)
  {
    if (!optionOrder.contains(property))
    {
      delete d->Options[property].Label;
      delete d->Options[property].Control;
      d->Options.remove(property);
      d->Properties.remove(property);
    }
  }
  d->OptionOrder = optionOrder;

  // Make sure widgets are in the same order as options in the description
  // (widgets of options that changed type are re-created and added at the end)
  QBoxLayout* boxLayout = qobject_cast<QBoxLayout*>(this->layout());
  if (boxLayout)
  {
    int layoutIndex = 0;
    for (const QString& property : optionOrder)
    {
      const qSlicerGenericIOOptionsWidgetPrivate::OptionWidgets& widgets = d->Options[property];
      for (QWidget* widget : { static_cast<QWidget*>(widgets.Label), static_cast<QWidget*>(widgets.Control) })
      {
        if (widget && boxLayout->indexOf(widget) != layoutIndex)
        {
          boxLayout->removeWidget(widget);
          boxLayout->insertWidget(layoutIndex, widget);
        }
        if (widget)
        {
          ++layoutIndex;
        }
      }
    }
  }

  d->UpdatingOptions = false;
  this->updateValid();
}
