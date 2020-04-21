
// Qt includes
#include <QVBoxLayout>
#include <QDebug>

// CTK includes
#include <ctkComboBox.h>

// qMRML includes
#include "qMRMLLabelComboBox.h"
#include "qMRMLUtils.h"

// MRML includes
#include <vtkMRMLColorNode.h>

// VTK includes
#include <vtkLookupTable.h>

//-----------------------------------------------------------------------------
class qMRMLLabelComboBoxPrivate
{
  Q_DECLARE_PUBLIC(qMRMLLabelComboBox);
protected:
  qMRMLLabelComboBox* const q_ptr;
public:
  qMRMLLabelComboBoxPrivate(qMRMLLabelComboBox& object);

  void setMRMLColorNode(vtkMRMLColorNode *newMRMLColorNode);

  QColor colorFromIndex(int index) const;

  ctkComboBox *       ComboBox;
  bool                NoneEnabled;
  vtkMRMLColorNode *  ColorNode;
  int                 CurrentColor;
  int                 MaximumColorCount;
  bool                ColorNameVisible;
  bool                LabelValueVisible;
};

// --------------------------------------------------------------------------
// qMRMLLabelComboBoxPrivate methods

// --------------------------------------------------------------------------
qMRMLLabelComboBoxPrivate::qMRMLLabelComboBoxPrivate(qMRMLLabelComboBox& object)
  : q_ptr(&object)
{
  this->ComboBox = nullptr;
  this->NoneEnabled = false;
  this->ColorNode = nullptr;
  this->CurrentColor = -1;
  this->MaximumColorCount = 0;
  this->ColorNameVisible = true;
  this->LabelValueVisible = false;
}

// ------------------------------------------------------------------------------
void qMRMLLabelComboBoxPrivate::setMRMLColorNode(vtkMRMLColorNode * newMRMLColorNode)
{
  Q_Q(qMRMLLabelComboBox);

  if (this->ColorNode == newMRMLColorNode)
    {
    return;
    }
  q->qvtkReconnect(this->ColorNode, newMRMLColorNode, vtkCommand::ModifiedEvent,
                      q, SLOT(updateWidgetFromMRML()));
  q->setEnabled(newMRMLColorNode != nullptr);
  this->ColorNode = newMRMLColorNode;

  if (this->ColorNode)
    {
    q->updateWidgetFromMRML();
    }
  else
    {
    this->ComboBox->clear();
    }
}

// ------------------------------------------------------------------------------
QColor qMRMLLabelComboBoxPrivate::colorFromIndex(int index) const
{
  //qDebug() << "qMRMLLabelComboBox::colorFromIndex - index:" << index;
  if (index < 0 || this->ColorNode == nullptr)
    {
    return QColor::Invalid;
    }

  double colorTable[4];
  vtkLookupTable *table = this->ColorNode->GetLookupTable();

  table->GetTableValue(index, colorTable);

  // HACK - The alpha associated with Black was 0
  if (colorTable[0] == 0 && colorTable[1] == 0 && colorTable[2] == 0)
    {
    colorTable[3] = 1;
    }

  return QColor::fromRgbF(colorTable[0], colorTable[1], colorTable[2], colorTable[3]);
}

// --------------------------------------------------------------------------
// qMRMLLabelComboBox methods

// --------------------------------------------------------------------------
qMRMLLabelComboBox::qMRMLLabelComboBox(QWidget* newParent)
  : Superclass(newParent)
  , d_ptr(new qMRMLLabelComboBoxPrivate(*this))
{
  Q_D(qMRMLLabelComboBox);

  d->ComboBox = new ctkComboBox(this);
  d->ComboBox->setDefaultText("None");
  this->setLayout(new QVBoxLayout);
  this->layout()->addWidget(d->ComboBox);
  this->layout()->setContentsMargins(0, 0, 0, 0);

  this->connect(d->ComboBox, SIGNAL(currentIndexChanged(int)),
                this, SLOT(onCurrentIndexChanged(int)));

  this->setEnabled(false);
}

// --------------------------------------------------------------------------
qMRMLLabelComboBox::~qMRMLLabelComboBox() = default;

// ----------------------------------------------------------------
void qMRMLLabelComboBox::printAdditionalInfo()
{
  Q_D(qMRMLLabelComboBox);
  qDebug().nospace() << "qMRMLLabelComboBox:" << this << endl
      << " MRMLColorNode:" << d->ColorNode << endl
      << "  ClassName:" << (d->ColorNode ? d->ColorNode->GetClassName() : "null") << endl
      << "  ID:" << (d->ColorNode ? d->ColorNode->GetID() : "null") << endl
      << "  Type:" << (d->ColorNode ? d->ColorNode->GetTypeAsString() : "null") << endl
      << " CurrentColor:" << d->CurrentColor << endl
      << " NoneEnabled:" << d->NoneEnabled << endl
      << " ColorNameVisible:" << d->ColorNameVisible << endl;
}

// ---------------------------------------------------------------------------------
CTK_GET_CPP(qMRMLLabelComboBox, vtkMRMLColorNode*, mrmlColorNode, ColorNode);

// ------------------------------------------------------------------------------
CTK_GET_CPP(qMRMLLabelComboBox, int, currentColor, CurrentColor);

// ------------------------------------------------------------------------------
void qMRMLLabelComboBox::setCurrentColor(int index)
{
  Q_D(qMRMLLabelComboBox);
  //qDebug() << "qMRMLLabelComboBox::setCurrentColor - index:" << index;

  if (index == d->CurrentColor)
    {
    return;
    }

  if (d->NoneEnabled)
    {
    if (index < -1 || index >= (d->ComboBox->count() - 1) )
      {
      return;
      }
    index++;
    }
  else
    {
    if (index < 0 || index >= d->ComboBox->count())
      {
      return;
      }
    }

  // Will trigger onCurrentIndexChanged
  d->ComboBox->setCurrentIndex(index);
}

// ------------------------------------------------------------------------------
void qMRMLLabelComboBox::setCurrentColor(const QString& color)
{
  Q_D(qMRMLLabelComboBox);
  int index = d->ComboBox->findText(color);
  // Will trigger onCurrentIndexChanged
  d->ComboBox->setCurrentIndex(index);
}

// ------------------------------------------------------------------------------
QString qMRMLLabelComboBox::currentColorName()const
{
  Q_D(const qMRMLLabelComboBox);
  return d->ComboBox->currentText();
}

// ------------------------------------------------------------------------------
CTK_GET_CPP(qMRMLLabelComboBox, bool, noneEnabled, NoneEnabled);

// -------------------------------------------------------------------------------
void qMRMLLabelComboBox::setNoneEnabled(bool enabled)
{
  Q_D(qMRMLLabelComboBox);

  if (d->NoneEnabled == enabled)
    {
    return;
    }
  d->NoneEnabled = enabled;

  if (enabled)
    {
    d->ComboBox->insertItem(0, "None");
    }
  else
    {
    d->ComboBox->removeItem(0);
    }
}
// ------------------------------------------------------------------------------
CTK_GET_CPP(qMRMLLabelComboBox, bool, colorNameVisible, ColorNameVisible);

// -------------------------------------------------------------------------------
void qMRMLLabelComboBox::setColorNameVisible(bool visible)
{
  Q_D(qMRMLLabelComboBox);

  if ( visible != d->ColorNameVisible )
    {
    d->ColorNameVisible = visible;
    this->updateWidgetFromMRML();
    }
}

// ------------------------------------------------------------------------------
CTK_GET_CPP(qMRMLLabelComboBox, bool, labelValueVisible, LabelValueVisible);

// -------------------------------------------------------------------------------
void qMRMLLabelComboBox::setLabelValueVisible(bool visible)
{
  Q_D(qMRMLLabelComboBox);

  if ( visible != d->LabelValueVisible )
    {
    d->LabelValueVisible = visible;
    this->updateWidgetFromMRML();
    }
}

// ---------------------------------------------------------------------------------
void qMRMLLabelComboBox::setMaximumColorCount(int maximum)
{
  Q_D(qMRMLLabelComboBox);
  d->MaximumColorCount = maximum <= 0 ? 0 : maximum;
}

// ---------------------------------------------------------------------------------
CTK_GET_CPP(qMRMLLabelComboBox, int, maximumColorCount, MaximumColorCount);

// ---------------------------------------------------------------------------------
// qMRMLLabelComboBox Slots

// ---------------------------------------------------------------------------------
void qMRMLLabelComboBox::setMRMLColorNode(vtkMRMLNode *newMRMLColorNode)
{
  Q_D(qMRMLLabelComboBox);
  d->setMRMLColorNode(vtkMRMLColorNode::SafeDownCast(newMRMLColorNode));
}

// ------------------------------------------------------------------------------
void qMRMLLabelComboBox::onCurrentIndexChanged(int index)
{
  Q_D(qMRMLLabelComboBox);
  //qDebug() << "qMRMLLabelComboBox::onCurrentIndexChanged - index:" << index;
  if (d->NoneEnabled)
    {
    index--;
    }

  d->CurrentColor = index;

  emit currentColorChanged(d->colorFromIndex(index));
  emit currentColorChanged(d->ComboBox->itemText(index));
  emit currentColorChanged(index);
}

// ---------------------------------------------------------------------------------
void qMRMLLabelComboBox::updateWidgetFromMRML()
{
  Q_D(qMRMLLabelComboBox);
  Q_ASSERT(d->ColorNode);

  //qDebug() << "qMRMLLabelComboBox::updateWidgetFromMRML";

  d->ComboBox->clear();

  if (!d->ColorNode->GetNamesInitialised())
    {
    qCritical() << "qMRMLLabelComboBox::updateWidgetFromMRML - ColorNode names are NOT initialized !";
    return;
    }

  if(d->NoneEnabled)
    {
    d->ComboBox->insertItem(0, "None");
    }

  //LookUpTabletime.start();
  vtkLookupTable * lookupTable = d->ColorNode->GetLookupTable();
  Q_ASSERT(lookupTable);

  const int numberOfColors = lookupTable->GetNumberOfColors();
  //qDebug() << "updateWidgetFromMRML - NumberOfColors:" << numberOfColors;

  int actualMax = d->MaximumColorCount > 0 ? d->MaximumColorCount : numberOfColors;
  for (int i = 0 ; i < actualMax ; ++i)
    {
    QString colorName = QString::fromUtf8(d->ColorNode->GetColorName(i));
    //qDebug() << QString("updateWidgetFromMRML - Color(index:%1, name: %2)").arg(i).arg(colorName);

    QIcon colorIcon(qMRMLUtils::createColorPixmap(this->style(), d->colorFromIndex(i)));

    QString text = "";

    if (d->LabelValueVisible)
      {
      text.append(QString("%1").arg(i));

      if (d->ColorNameVisible)
        {
        // add delimiter if the colorName is visible as well
        text.append(", ");
        }

      }

    if ( d->ColorNameVisible )
      {
      text += colorName;
      }

    d->ComboBox->addItem(colorIcon, text);

    }
  d->CurrentColor = -1;
}
