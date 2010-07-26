/// Qt includes
#include <QDebug>
#include <QFileInfo>

// CTK includes
#include "qCTKFlowLayout.h"

/// Volumes includes
#include "qSlicerVolumesIOOptionsWidget.h"
#include "ui_qSlicerVolumesIOOptionsWidget.h"

//-----------------------------------------------------------------------------
class qSlicerVolumesIOOptionsWidgetPrivate:
  public ctkPrivate<qSlicerVolumesIOOptionsWidget>,
  public Ui_qSlicerVolumesIOOptionsWidget
{
  CTK_DECLARE_PUBLIC(qSlicerVolumesIOOptionsWidget);
public:
  //void init();
};


//-----------------------------------------------------------------------------
qSlicerVolumesIOOptionsWidget::qSlicerVolumesIOOptionsWidget(QWidget* parentWidget)
{
  CTK_INIT_PRIVATE(qSlicerVolumesIOOptionsWidget);
  CTK_D(qSlicerVolumesIOOptionsWidget);
  ctk_d()->setupUi(this);

  // Replace the horizontal layout with a flow layout
  qCTKFlowLayout* flowLayout = new qCTKFlowLayout;
  QLayout* oldLayout = this->layout();
  int margins[4];
  oldLayout->getContentsMargins(&margins[0],&margins[1],&margins[2],&margins[3]);
  QLayoutItem* item = 0;
  while((item = oldLayout->takeAt(0)))
    {
    if (item->widget())
      {
      flowLayout->addWidget(item->widget());
      }
    }
  // setLayout() will take care or reparenting layouts and widgets
  delete oldLayout;
  flowLayout->setContentsMargins(0,0,0,0);
  this->setLayout(flowLayout);

  connect(d->NameLineEdit, SIGNAL(textChanged(const QString&)),
          this, SLOT(updateProperties()));
  connect(d->LabelMapCheckBox, SIGNAL(toggled(bool)),
          this, SLOT(updateProperties()));
  connect(d->CenteredCheckBox, SIGNAL(toggled(bool)),
          this, SLOT(updateProperties()));
  connect(d->SingleFileCheckBox, SIGNAL(toggled(bool)),
          this, SLOT(updateProperties()));
  connect(d->OrientationCheckBox, SIGNAL(toggled(bool)),
          this, SLOT(updateProperties()));
}

//-----------------------------------------------------------------------------
void qSlicerVolumesIOOptionsWidget::updateProperties()
{
  CTK_D(const qSlicerVolumesIOOptionsWidget);
  if (!d->NameLineEdit->text().isEmpty())
    {
    this->Properties["name"] = d->NameLineEdit->text();
    }
  else
    {
    this->Properties.remove("name");
    }
  this->Properties["labelmap"] = d->LabelMapCheckBox->isChecked();
  this->Properties["center"] = d->CenteredCheckBox->isChecked();
  this->Properties["singleFile"] = d->SingleFileCheckBox->isChecked();
  this->Properties["discardOrientation"] = d->OrientationCheckBox->isChecked();
}

//-----------------------------------------------------------------------------
void qSlicerVolumesIOOptionsWidget::setFileName(const QString& fileName)
{
  CTK_D(qSlicerVolumesIOOptionsWidget);
  QFileInfo fileInfo(fileName);
  if (fileInfo.isFile())
    {
    d->NameLineEdit->setText(fileInfo.baseName());
    }
  this->qSlicerIOOptionsWidget::setFileName(fileName);
}

//-----------------------------------------------------------------------------
void qSlicerVolumesIOOptionsWidget::setFileNames(const QStringList& fileNames)
{
  CTK_D(qSlicerVolumesIOOptionsWidget);
  d->NameLineEdit->setText("");
  this->qSlicerIOOptionsWidget::setFileNames(fileNames);
}
