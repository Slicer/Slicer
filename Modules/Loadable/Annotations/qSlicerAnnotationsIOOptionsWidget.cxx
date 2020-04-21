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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

/// Qt includes
#include <QButtonGroup>
#include <QFileInfo>

// CTK includes
#include <ctkFlowLayout.h>
#include <ctkUtils.h>

/// Annotations includes
#include <qSlicerIOOptions_p.h>
#include "qSlicerAnnotationsIOOptionsWidget.h"
#include "ui_qSlicerAnnotationModuleIOOptionsWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Annotations
class qSlicerAnnotationsIOOptionsWidgetPrivate
  : public qSlicerIOOptionsPrivate
  , public Ui_qSlicerAnnotationModuleIOOptionsWidget
{
public:
  //void init();
};

//-----------------------------------------------------------------------------
qSlicerAnnotationsIOOptionsWidget::qSlicerAnnotationsIOOptionsWidget(QWidget* parentWidget)
  : qSlicerIOOptionsWidget(new qSlicerAnnotationsIOOptionsWidgetPrivate, parentWidget)
{
  Q_D(qSlicerAnnotationsIOOptionsWidget);
  d->setupUi(this);

  // Replace the horizontal layout with a flow layout
  ctkFlowLayout* flowLayout = new ctkFlowLayout;
  flowLayout->setPreferredExpandingDirections(Qt::Horizontal);
  flowLayout->setAlignItems(false);
  QLayout* oldLayout = this->layout();
  int margins[4];
  oldLayout->getContentsMargins(&margins[0],&margins[1],&margins[2],&margins[3]);
  QLayoutItem* item = nullptr;
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

  this->FileTypeButtonGroup = new QButtonGroup(flowLayout);
  this->FileTypeButtonGroup->addButton(d->FiducialRadioButton);
  this->FileTypeButtonGroup->addButton(d->RulerRadioButton);
  this->FileTypeButtonGroup->addButton(d->ROIRadioButton);
//  this->FileTypeButtonGroup->addButton(d->ListRadioButton);
  this->connect(this->FileTypeButtonGroup, SIGNAL(buttonClicked(int)),
                this, SLOT(updateProperties()));


  connect(d->NameLineEdit, SIGNAL(textChanged(QString)),
          this, SLOT(updateProperties()));
  /*
  connect(d->FiducialRadioButton, SIGNAL(toggled(bool)),
          this, SLOT(updateProperties()));
  connect(d->RulerRadioButton, SIGNAL(toggled(bool)),
          this, SLOT(updateProperties()));
  connect(d->ROIRadioButton, SIGNAL(toggled(bool)),
          this, SLOT(updateProperties()));
  */
  // fiducial file by default
  d->FiducialRadioButton->setChecked(true);
  // in case the user doesn't touch anything, set up the properties now
  this->updateProperties();
}

//-----------------------------------------------------------------------------
qSlicerAnnotationsIOOptionsWidget::~qSlicerAnnotationsIOOptionsWidget() = default;

//-----------------------------------------------------------------------------
void qSlicerAnnotationsIOOptionsWidget::updateProperties()
{
  Q_D(qSlicerAnnotationsIOOptionsWidget);
  if (!d->NameLineEdit->text().isEmpty())
    {
    QStringList names = d->NameLineEdit->text().split(';');
    for (int i = 0; i < names.count(); ++i)
      {
      names[i] = names[i].trimmed();
      }
    d->Properties["name"] = names;
    }
  else
    {
    d->Properties.remove("name");
    }
  d->Properties["fiducial"] = d->FiducialRadioButton->isChecked();
  d->Properties["ruler"] = d->RulerRadioButton->isChecked();
  d->Properties["roi"] = d->ROIRadioButton->isChecked();
//  d->Properties["list"] = d->ListRadioButton->isChecked();
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationsIOOptionsWidget::setFileName(const QString& fileName)
{
  this->setFileNames(QStringList(fileName));
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationsIOOptionsWidget::setFileNames(const QStringList& fileNames)
{
  Q_D(qSlicerAnnotationsIOOptionsWidget);
  QStringList names;
  foreach(const QString& fileName, fileNames)
    {
    QFileInfo fileInfo(fileName);
    if (fileInfo.isFile())
      {
      names << fileInfo.baseName();
      }
    // Because '_' is considered as a word character (\w), \b
    // doesn't consider '_' as a word boundary.
    QRegExp fiducialName("(\\b|_)(F)(\\b|_)");
    QRegExp rulerName("(\\b|_)(M)(\\b|_)");
    QRegExp roiName("(\\b|_)(R)(\\b|_)");
    QAbstractButton* activeButton = nullptr;
/*    QRegExp listName("(\\b|_)(List)(\\b|_)");
    if (fileInfo.baseName().contains(listName))
      {
      d->ListRadioButton->setChecked(true);
      }
    else
*/
    if (fileInfo.baseName().contains(fiducialName))
      {
      activeButton = d->FiducialRadioButton;
      }
    else if (fileInfo.baseName().contains(rulerName))
      {
      activeButton = d->RulerRadioButton;
      }
    else if (fileInfo.baseName().contains(roiName))
      {
      activeButton = d->ROIRadioButton;
      }
    if (activeButton)
      {
      activeButton->click();
      }
    }

  this->qSlicerIOOptionsWidget::setFileNames(fileNames);
}
