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

  This file was originally developed by Johan Andruejol, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QDebug>
#include <QFileDialog>
#include <QMainWindow>
#include <QSettings>
#include <QStyleFactory>

// QtGUI includes
#include "qSlicerApplication.h"
#include "qSlicerSettingsStylesPanel.h"
#include "ui_qSlicerSettingsStylesPanel.h"

// --------------------------------------------------------------------------
// qSlicerSettingsStylesPanelPrivate

//-----------------------------------------------------------------------------
class qSlicerSettingsStylesPanelPrivate: public Ui_qSlicerSettingsStylesPanel
{
  Q_DECLARE_PUBLIC(qSlicerSettingsStylesPanel);
  typedef qSlicerSettingsStylesPanelPrivate Self;
protected:
  qSlicerSettingsStylesPanel* const q_ptr;

public:
  qSlicerSettingsStylesPanelPrivate(qSlicerSettingsStylesPanel& object);
  void init();
  int styleIndex(const QString& styleName) const;
  void populateStyles();

  static bool isQtStyle(const QString& styleName);
  static QStringList qtStyles();

  QStringList AdditionalPaths;
};

// --------------------------------------------------------------------------
// qSlicerSettingsStylesPanelPrivate methods

// --------------------------------------------------------------------------
qSlicerSettingsStylesPanelPrivate
::qSlicerSettingsStylesPanelPrivate(qSlicerSettingsStylesPanel& object)
  :q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerSettingsStylesPanelPrivate::init()
{
  Q_Q(qSlicerSettingsStylesPanel);

  this->setupUi(q);

  // General appearance settings
  QObject::connect(this->FontButton, SIGNAL(currentFontChanged(QFont)),
                   q, SLOT(onFontChanged(QFont)));
  QObject::connect(this->ShowToolTipsCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(onShowToolTipsToggled(bool)));
  QObject::connect(this->ShowToolButtonTextCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(onShowToolButtonTextToggled(bool)));

  q->registerProperty("no-tooltip", this->ShowToolTipsCheckBox, "checked",
                      SIGNAL(toggled(bool)));
  q->registerProperty("font", this->FontButton, "currentFont",
                      SIGNAL(currentFontChanged(QFont)));
  q->registerProperty("MainWindow/ShowToolButtonText", this->ShowToolButtonTextCheckBox,
                      "checked", SIGNAL(toggled(bool)));
  q->registerProperty("MainWindow/RestoreGeometry", this->RestoreUICheckBox, "checked",
                      SIGNAL(toggled(bool)));

  // Additional Style paths
  this->AdditionalStylePathMoreButton->setChecked(false);

  // Additional path setting
  QObject::connect(this->AdditionalStylePathsView,
    SIGNAL(directoryListChanged()), q, SLOT(onAdditionalStylePathsChanged()));
  q->registerProperty("Styles/AdditionalPaths",
                      this->AdditionalStylePathsView,
                      "directoryList", SIGNAL(directoryListChanged()),
                      "Additional style paths",
                      ctkSettingsPanel::OptionRequireRestart);

  // Style setting
  this->populateStyles();
  q->setCurrentStyle("Slicer");
  QObject::connect(this->StyleComboBox, SIGNAL(currentIndexChanged(QString)),
                   q, SLOT(onStyleChanged(QString)));
  q->registerProperty("Styles/Style", q,
                      "currentStyle", SIGNAL(currentStyleChanged(QString)),
                      "Current style");

  // Connect AdditionalStylePaths buttons
  QObject::connect(this->AddAdditionalStylePathButton, SIGNAL(clicked()),
                   q, SLOT(onAddStyleAdditionalPathClicked()));
  QObject::connect(this->RemoveAdditionalStylePathButton, SIGNAL(clicked()),
                   q, SLOT(onRemoveStyleAdditionalPathClicked()));
}

// --------------------------------------------------------------------------
int qSlicerSettingsStylesPanelPrivate
::styleIndex(const QString& styleName) const
{
  int styleIndex = this->StyleComboBox->findText(styleName, Qt::MatchFixedString);
  if (styleIndex == -1)
    {
    // if not found (call setCurrentStyle with a wrong style for example),
    // default to slicer's style
    styleIndex = this->StyleComboBox->findText("Slicer", Qt::MatchFixedString);
    }
  return styleIndex;
}

namespace
{
QString toCamelCase(const QString& s)
{
  QString camelCase;
  if (s.size() <= 0)
    {
    return camelCase;
    }

  camelCase = s[0].toUpper();
  for (int i = 1; i < s.size(); ++i)
    {
    if (s[i - 1] == ' ')
      {
      camelCase.append(s[i].toUpper());
      }
    else
      {
      camelCase.append(s[i]);
      }
    };
  return camelCase;
}
}

// --------------------------------------------------------------------------
void qSlicerSettingsStylesPanelPrivate::populateStyles()
{
  Q_Q(qSlicerSettingsStylesPanel);
  QString currentStyle = this->StyleComboBox->currentText();

  bool wasBlocking = this->StyleComboBox->blockSignals(true);
  // Re-populate styles
  this->StyleComboBox->clear();
  foreach(const QString& style, q->availableSlicerStyles())
    {
    this->StyleComboBox->addItem(toCamelCase(style));
    }

  // Find the previously set style
  int currentStyleIndex = this->styleIndex(currentStyle);
  this->StyleComboBox->blockSignals(wasBlocking);

  this->StyleComboBox->setCurrentIndex(currentStyleIndex);
}

// --------------------------------------------------------------------------
QStringList qSlicerSettingsStylesPanelPrivate::qtStyles()
{
  return QStringList()
      << "Windows"
      << "WindowsCE"
      << "WindowsXP"
      << "WindowsVista"
      << "Motif"
      << "CDE"
      << "Plastique"
      << "Cleanlooks"
      << "Macintosh"
      << "Macintosh (aqua)"
      << "GTK+"
      << "Fusion";
}

// --------------------------------------------------------------------------
bool qSlicerSettingsStylesPanelPrivate::
isQtStyle(const QString& styleName)
{
  // Styles are case insensitive
  return Self::qtStyles().contains(styleName, Qt::CaseInsensitive);
}

// --------------------------------------------------------------------------
// qSlicerSettingsStylesPanel methods

// --------------------------------------------------------------------------
qSlicerSettingsStylesPanel::qSlicerSettingsStylesPanel(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerSettingsStylesPanelPrivate(*this))
{
  Q_D(qSlicerSettingsStylesPanel);
  d->init();
}

// --------------------------------------------------------------------------
qSlicerSettingsStylesPanel::~qSlicerSettingsStylesPanel() = default;

// --------------------------------------------------------------------------
void qSlicerSettingsStylesPanel::onAdditionalStylePathsChanged()
{
  Q_D(qSlicerSettingsStylesPanel);

  // Remove old paths
  foreach(const QString& path, d->AdditionalPaths)
    {
    QCoreApplication::removeLibraryPath(path);
    }

  // Add new ones
  d->AdditionalPaths = d->AdditionalStylePathsView->directoryList(true);
  foreach(const QString&  path, d->AdditionalPaths)
    {
    QCoreApplication::addLibraryPath(path);
    }

  d->populateStyles();
}

// --------------------------------------------------------------------------
void qSlicerSettingsStylesPanel::onAddStyleAdditionalPathClicked()
{
  Q_D(qSlicerSettingsStylesPanel);
  qSlicerCoreApplication* coreApp = qSlicerCoreApplication::application();
  QString extensionInstallPath =
    coreApp->revisionUserSettings()->value("Extensions/InstallPath").toString();
  QString path = QFileDialog::getExistingDirectory(
      this, tr("Select a path containing a \"styles\" plugin directory"),
      extensionInstallPath);
  // An empty directory means that the user cancelled the dialog.
  if (path.isEmpty())
    {
    return;
    }
  d->AdditionalStylePathsView->addDirectory(path);
}

// --------------------------------------------------------------------------
void qSlicerSettingsStylesPanel::onRemoveStyleAdditionalPathClicked()
{
  Q_D(qSlicerSettingsStylesPanel);
  // Remove all selected
  d->AdditionalStylePathsView->removeSelectedDirectories();
}

// --------------------------------------------------------------------------
QStringList qSlicerSettingsStylesPanel::availableSlicerStyles()
{
  QStringList styles;
  foreach(const QString& style, QStyleFactory::keys())
    {
    if (qSlicerSettingsStylesPanelPrivate::isQtStyle(style))
      {
      continue;
      }
    styles << style;
    }
  return styles;
}

// --------------------------------------------------------------------------
QString qSlicerSettingsStylesPanel::currentStyle() const
{
  Q_D(const qSlicerSettingsStylesPanel);
  return d->StyleComboBox->currentText();
}

// --------------------------------------------------------------------------
void qSlicerSettingsStylesPanel::setCurrentStyle(const QString& newStyleName)
{
  Q_D(qSlicerSettingsStylesPanel);
  d->StyleComboBox->setCurrentIndex(d->styleIndex(newStyleName));
}

// --------------------------------------------------------------------------
void qSlicerSettingsStylesPanel::onStyleChanged(const QString& newStyleName)
{
  qSlicerApplication* app = qSlicerApplication::application();
  app->removeEventFilter(app->style());

  QStyle* newStyle = QStyleFactory::create(newStyleName);
  if (!newStyle)
    {
    qWarning() << "Style named " << newStyleName << " not found !"
               <<" Defaulting to Slicer's style.";
    this->setCurrentStyle("Slicer");
    return;
    }

  app->setStyle(newStyle);
  app->installEventFilter(app->style());
  app->setPalette(newStyle->standardPalette());

  emit this->currentStyleChanged(newStyleName);
}

// --------------------------------------------------------------------------
void qSlicerSettingsStylesPanel::onFontChanged(const QFont& font)
{
  qSlicerApplication::application()->setFont(font);
}

// --------------------------------------------------------------------------
void qSlicerSettingsStylesPanel::onShowToolTipsToggled(bool disable)
{
  qSlicerApplication::application()->setToolTipsEnabled(!disable);
}

// --------------------------------------------------------------------------
void qSlicerSettingsStylesPanel::onShowToolButtonTextToggled(bool enable)
{
  QMainWindow* mainWindow = qSlicerApplication::application()->mainWindow();
  if (mainWindow)
    {
    mainWindow->setToolButtonStyle(enable ?
      Qt::ToolButtonTextUnderIcon : Qt::ToolButtonIconOnly);
    }
}
