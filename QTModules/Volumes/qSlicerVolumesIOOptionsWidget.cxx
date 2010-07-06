/// Qt includes
#include <QDebug>
#include <QFileInfo>


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
  // FIXME Replace the horizontal layout with a flow layout

  connect(d->NameLineEdit, SIGNAL(editingFinished()),
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
  if (d->NameLineEdit->text().isEmpty())
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
  d->NameLineEdit->setText(QFileInfo(fileName).baseName());
  this->qSlicerIOOptionsWidget::setFileName(fileName);
}

//-----------------------------------------------------------------------------
void qSlicerVolumesIOOptionsWidget::setFileNames(const QStringList& fileNames)
{
  CTK_D(qSlicerVolumesIOOptionsWidget);
  d->NameLineEdit->setText("");
  this->qSlicerIOOptionsWidget::setFileNames(fileNames);
}
