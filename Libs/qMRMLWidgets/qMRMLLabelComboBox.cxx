// Qt includes
#include <QVBoxLayout>
#include <QPainter>

// CTK includes
#include <ctkComboBox.h>

// qMRML includes
#include "qMRMLLabelComboBox.h"

// MRML includes
#include <vtkMRMLColorNode.h>
#include <vtkMRMLNode.h>

//-----------------------------------------------------------------------------
class qMRMLLabelComboBoxPrivate: public ctkPrivate<qMRMLLabelComboBox>
{
public:
  qMRMLLabelComboBoxPrivate();

  QColor colorFromIndex(int index) const;
  void addColor(const QColor &color, const QString &name);
  
  ctkComboBox *ComboBox;
  qMRMLLabelComboBox::DisplayOption Option;
  vtkMRMLColorNode *ColorNode;
};

// --------------------------------------------------------------------------
// qMRMLLabelComboBoxPrivate methods

// --------------------------------------------------------------------------
qMRMLLabelComboBoxPrivate::qMRMLLabelComboBoxPrivate()
{
  this->ComboBox = NULL;
  this->Option = qMRMLLabelComboBox::OnlyColors;
  this->ColorNode = NULL;
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

  // /!\ the index of the combo ComboBox  can have 2 start point for the color, depending or not if the NoColorMode is activated:
  // NoColorMode == true implies the index for the colors starts at 1
  // NoColorMode == false implies the index for the colors starts at 0

  //N.B. if the comboBox is up-to-date, (and in NoColorMode == false)
  //Then the index of the combo box is the same as the index of the ColorNode, and its lookup table

  this->connect(d->ComboBox, SIGNAL(currentIndexChanged(int)),
                this, SLOT(colorSelected(int)));

  this->setEnabled(false);
  }

// ----------------------------------------------------------------
void qMRMLLabelComboBoxPrivate::addColor(const QColor &color, const QString &name)
{
  CTK_P(qMRMLLabelComboBox);
  //Create a pixmap
  const int size = p->style()->pixelMetric(QStyle::PM_SmallIconSize) - 5;
  QPixmap colorFieldPixmap(size, size);
  
  //Fill it with the color
  colorFieldPixmap.fill(color);
  
  //Make a black rectangle on the border
  QPainter painter(&colorFieldPixmap);
  painter.drawRect(0, 0, size-1, size-1);
 
  //Which finally allows me to create this icon
  QIcon colorField(colorFieldPixmap);

  this->ComboBox->addItem(colorField,name);
}

// ------------------------------------------------------------------------------
void qMRMLLabelComboBox::setMRMLColorNode(vtkMRMLColorNode *newNode)
{
  CTK_D(qMRMLLabelComboBox);

  this->qvtkReconnect(d->ColorNode,newNode, vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromMRML()));
  d->ColorNode = newNode;
  this->setEnabled(newNode != 0);
  this->updateWidgetFromMRML();
}

// ----------------------------------------------------------------------------------
qMRMLLabelComboBox::DisplayOption qMRMLLabelComboBox::displayOption()
{
  CTK_D(qMRMLLabelComboBox);
  return d->Option;
}

// ------------------------------------------------------------------------------
void qMRMLLabelComboBox::setCurrentColor(int index)
{
  CTK_D(qMRMLLabelComboBox);

  if ((index == -1 && d->Option != qMRMLLabelComboBox::WithNone) 
      || index < -1
      || index > d->ComboBox->count())
    {
    return;
    }
  
  this->colorSelected(index);
}

// -------------------------------------------------------------------------------
void qMRMLLabelComboBox::setDisplayOption(qMRMLLabelComboBox::DisplayOption newOption)
{
  CTK_D(qMRMLLabelComboBox);
  
  if (d->Option == newOption)
    {
    return ;
    }
  d->Option = newOption;

  if (newOption == qMRMLLabelComboBox::WithNone)
    {  
    d->ComboBox->insertItem(0, "None");
    }
  else
    {
    d->ComboBox->removeItem(d->ComboBox->findText("None", Qt::MatchExactly));
    }
}

// ------------------------------------------------------------------------------
QColor qMRMLLabelComboBoxPrivate::colorFromIndex(int index) const
{
  if ( index < 0 )
    {
    return QColor::Invalid;
    }

  double colorTable[4];
  vtkLookupTable *table = this->ColorNode->GetLookupTable();

  table->GetTableValue(index, colorTable);

  return QColor::fromRgbF(colorTable[0], colorTable[1], colorTable[2], colorTable[3]);
}

// ---------------------------------------------------------------------------------
// qMRMLLabelComboBox Slots

// ---------------------------------------------------------------------------------
void qMRMLLabelComboBox::setMRMLColorNode(vtkMRMLNode *newNode)
{
  this->setMRMLColorNode(vtkMRMLColorNode::SafeDownCast(newNode));
}

// ------------------------------------------------------------------------------
void qMRMLLabelComboBox::colorSelected(int index)
{
  CTK_D(qMRMLLabelComboBox);
  
  //two case, depending on the NoColorMode
  if (d->Option == qMRMLLabelComboBox::WithNone)
    {
    --index;
    }
 
  emit currentlabelChanged(d->colorFromIndex(index));
  emit currentlabelChanged(d->ComboBox->itemText(index));
  emit currentlabelChanged(index); 

}

// ---------------------------------------------------------------------------------
void qMRMLLabelComboBox::updateWidgetFromMRML()
{
  CTK_D(qMRMLLabelComboBox);
  
  d->ComboBox->clear();
  
  if (!d->ColorNode->GetNamesInitialised())
    {
    return;
    }

  if(d->Option == qMRMLLabelComboBox::WithNone)
    {
    d->ComboBox->insertItem(0,"No color");
    }

  vtkLookupTable *table = d->ColorNode->GetLookupTable();

  const int count = table->GetNumberOfColors();
  QString name;

  for (int i = 0 ; i < count ; ++i)
    {
    name = d->ColorNode->GetColorName(i);
    d->addColor(d->colorFromIndex(i), name);
    }
}
