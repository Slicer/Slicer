// Qt includes
#include <QVBoxLayout>
#include <QPainter>
#include <QDebug>
#include <QTime>

// CTK includes
#include <ctkComboBox.h>
#include <ctkLogger.h>

// qMRML includes
#include "qMRMLLabelComboBox.h"

// MRML includes
#include <vtkMRMLColorNode.h>
#include <vtkMRMLNode.h>

//--------------------------------------------------------------------------
static ctkLogger logger("org.slicer.libs.qmrmlwidgets.qMRMLLabelComboBox");
//--------------------------------------------------------------------------

//-----------------------------------------------------------------------------
class qMRMLLabelComboBoxPrivate: public ctkPrivate<qMRMLLabelComboBox>
{
public:
  qMRMLLabelComboBoxPrivate();

  void setMRMLColorNode(vtkMRMLColorNode *newMRMLColorNode);

  QColor colorFromIndex(int index) const;

  QIcon createIcon(const QColor &color);
  
  ctkComboBox *       ComboBox;
  bool                NoneEnabled;
  vtkMRMLColorNode *  ColorNode;
  int                 CurrentColor;
  int                 MaximumColorCount;
  bool                ColorNameVisible;
};

// --------------------------------------------------------------------------
// qMRMLLabelComboBoxPrivate methods

// --------------------------------------------------------------------------
qMRMLLabelComboBoxPrivate::qMRMLLabelComboBoxPrivate()
{
  this->ComboBox = 0;
  this->NoneEnabled = false;
  this->ColorNode = 0;
  this->CurrentColor = -1;
  this->MaximumColorCount = 0;
  this->ColorNameVisible = true;
}

// ------------------------------------------------------------------------------
void qMRMLLabelComboBoxPrivate::setMRMLColorNode(vtkMRMLColorNode * newMRMLColorNode)
{
  CTK_P(qMRMLLabelComboBox);

  p->qvtkReconnect(this->ColorNode, newMRMLColorNode, vtkCommand::ModifiedEvent,
                      p, SLOT(updateWidgetFromMRML()));
  p->setEnabled(newMRMLColorNode != 0);
  this->ColorNode = newMRMLColorNode;

  if (this->ColorNode)
    {
    p->updateWidgetFromMRML();
    }
  else
    {
    this->ComboBox->clear();
    }
}

// ------------------------------------------------------------------------------
QColor qMRMLLabelComboBoxPrivate::colorFromIndex(int index) const
{
  //logger.debug(QString("colorFromIndex - index: %1").arg(index));
  if (index < 0)
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

// ----------------------------------------------------------------
QIcon qMRMLLabelComboBoxPrivate::createIcon(const QColor &color)
{
  CTK_P(qMRMLLabelComboBox);
  // Create a pixmap
  
  const int size = p->style()->pixelMetric(QStyle::PM_ButtonIconSize) - 5; 

  QPixmap colorFieldPixmap(size, size);

  // Fill it with the color
  colorFieldPixmap.fill(color);

  // Make a black rectangle on the border
  QPainter painter(&colorFieldPixmap);
  painter.drawRect(0, 0, size - 1, size - 1);

  // Which finally allows to create this icon
  QIcon colorField(colorFieldPixmap);

  return colorField;
}

// --------------------------------------------------------------------------
// qMRMLLabelComboBox methods

// --------------------------------------------------------------------------
qMRMLLabelComboBox::qMRMLLabelComboBox(QWidget* newParent)
  : Superclass(newParent)
{
  CTK_INIT_PRIVATE(qMRMLLabelComboBox);
  CTK_D(qMRMLLabelComboBox);
  
  d->ComboBox = new ctkComboBox(this);
  d->ComboBox->setDefaultText("None");
  this->setLayout(new QVBoxLayout);
  this->layout()->addWidget(d->ComboBox);
  this->layout()->setContentsMargins(0, 0, 0, 0);

  this->connect(d->ComboBox, SIGNAL(currentIndexChanged(int)),
                this, SLOT(onCurrentIndexChanged(int)));

  this->setEnabled(false);
}

// ----------------------------------------------------------------
void qMRMLLabelComboBox::printAdditionalInfo()
{
  CTK_D(qMRMLLabelComboBox);
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
CTK_GET_CXX(qMRMLLabelComboBox, vtkMRMLColorNode*, mrmlColorNode, ColorNode);

// ------------------------------------------------------------------------------
CTK_GET_CXX(qMRMLLabelComboBox, int, currentColor, CurrentColor);

// ------------------------------------------------------------------------------
void qMRMLLabelComboBox::setCurrentColor(int index)
{
  CTK_D(qMRMLLabelComboBox);
  //logger.debug(QString("setCurrentColor - index: %1").arg(index));

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
CTK_GET_CXX(qMRMLLabelComboBox, bool, noneEnabled, NoneEnabled);

// -------------------------------------------------------------------------------
void qMRMLLabelComboBox::setNoneEnabled(bool enabled)
{
  CTK_D(qMRMLLabelComboBox);

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
CTK_GET_CXX(qMRMLLabelComboBox, bool, colorNameVisible, ColorNameVisible);

// -------------------------------------------------------------------------------
void qMRMLLabelComboBox::setColorNameVisible(bool visible)
{
  CTK_D(qMRMLLabelComboBox);

  if ( visible != d->ColorNameVisible )
    {
    d->ColorNameVisible = visible;
    this->updateWidgetFromMRML();
    }
}

// ---------------------------------------------------------------------------------
void qMRMLLabelComboBox::setMaximumColorCount(int maximum)
{
  CTK_D(qMRMLLabelComboBox);
  d->MaximumColorCount = maximum <= 0 ? 0 : maximum;
}

// ---------------------------------------------------------------------------------
CTK_GET_CXX(qMRMLLabelComboBox, int, maximumColorCount, MaximumColorCount);

// ---------------------------------------------------------------------------------
// qMRMLLabelComboBox Slots

// ---------------------------------------------------------------------------------
void qMRMLLabelComboBox::setMRMLColorNode(vtkMRMLNode *newMRMLColorNode)
{
  CTK_D(qMRMLLabelComboBox);
  d->setMRMLColorNode(vtkMRMLColorNode::SafeDownCast(newMRMLColorNode));
}

// ------------------------------------------------------------------------------
void qMRMLLabelComboBox::onCurrentIndexChanged(int index)
{
  CTK_D(qMRMLLabelComboBox);
  //logger.debug(QString("onCurrentIndexChanged - index: %1").arg(index));
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
  CTK_D(qMRMLLabelComboBox);
  Q_ASSERT(d->ColorNode);
  
  //logger.debug("updateWidgetFromMRML");

  d->ComboBox->clear();
  
  if (!d->ColorNode->GetNamesInitialised())
    {
    logger.error("updateWidgetFromMRML - ColorNode names are NOT initialized !");
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
  //logger.debug(QString("updateWidgetFromMRML - NumberOfColors: %1").arg(numberOfColors));

  int actualMax = d->MaximumColorCount > 0 ? d->MaximumColorCount : numberOfColors;
  for (int i = 0 ; i < actualMax ; ++i)
    {
    QString colorName = QLatin1String(d->ColorNode->GetColorName(i));
    //logger.debug(QString("updateWidgetFromMRML - Color(index:%1, name: %2)").arg(i).arg(colorName));
    
    QIcon colorIcon = d->createIcon(d->colorFromIndex(i));
    
    if ( d->ColorNameVisible )
      {
      d->ComboBox->addItem(colorIcon, colorName);
      }
    else
      {
      d->ComboBox->addItem(colorIcon,"");
      }

    }
  d->CurrentColor = -1;
}
