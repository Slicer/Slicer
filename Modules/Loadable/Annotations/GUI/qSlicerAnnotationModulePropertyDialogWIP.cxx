#include "qSlicerAnnotationModulePropertyDialog.h"
#include "ui_qSlicerAnnotationModulePropertyDialog.h"

// Qt includes
#include <QButtonGroup>
#include <QList>
#include <QFontMetrics>
#include <QDebug>
#include <QMessageBox>
#include <QComboBox>
#include <QTabWidget>
#include <QTableWidget>
#include <QMotifStyle>

#include "Logic/vtkSlicerAnnotationModuleLogic.h"
#include "vtkMRMLAnnotationNode.h"
#include "vtkMRMLAnnotationPointDisplayNode.h"
#include "vtkMRMLAnnotationLineDisplayNode.h"
#include "vtkMRMLAnnotationTextDisplayNode.h"
#include "vtkMRMLAnnotationControlPointsNode.h"
#include "vtkMRMLAnnotationFiducialNode.h"
#include "vtkMRMLAnnotationAngleNode.h"
#include "vtkMRMLAnnotationRulerNode.h"
#include "vtkMRMLAnnotationLinesNode.h"
#include "vtkMRMLAnnotationROINode.h"
#include "vtkMRMLFiducialListNode.h"

//------------------------------------------------------------------------------
qSlicerAnnotationModulePropertyDialog::~qSlicerAnnotationModulePropertyDialog()
{
  this->m_id = 0;
  this->m_logic = 0;
}

//------------------------------------------------------------------------------
qSlicerAnnotationModulePropertyDialog::qSlicerAnnotationModulePropertyDialog(const char * id, vtkSlicerAnnotationModuleLogic* logic)
{
  this->m_id = vtkStdString(id);
  this->m_logic = logic;

  // now build the user interface
  ui.setupUi(this);

  if (this->m_logic->IsAnnotationHierarchyNode(id))
    {
    // hierarchies

    // TODO show the display widget

    //ui.tabWidget->hide();

    }
  else
    {
    // annotations

    this->initialize();

    }
  ui.toolBox->setStyle(new QMotifStyle());

  // create the slot and signal connections
  this->createConnection();


}

//------------------------------------------------------------------------------
void qSlicerAnnotationModulePropertyDialog::initialize()
{
  // backup the current annotationNode
  this->m_logic->BackupAnnotationNode(this->m_id);

  // build the typeLabelText including name and id of the annotation
  QString * typeLabelText = new QString("Name: ");
  typeLabelText->append(this->m_logic->GetAnnotationName(this->m_id));
  typeLabelText->append(" (");
  typeLabelText->append(this->m_id);
  typeLabelText->append(")");

  ui.typeLabel->setText(*typeLabelText);

  // update the typeIcon according to the annotation type
  QIcon icon = QIcon(this->m_logic->GetAnnotationIcon(this->m_id.c_str()));
  QPixmap pixmap = icon.pixmap(32, 32);

  ui.typeIcon->setPixmap(pixmap);

  // load the current annotation text
  vtkStdString text = this->m_logic->GetAnnotationText(this->m_id.c_str());

  ui.annotationTextEdit->setText(text.c_str());

  // load the current measurement
  const char * measurement = this->m_logic->GetAnnotationMeasurement(
      this->m_id.c_str(), true);

  ui.measurementLineEdit->setText(measurement);

/*
  // load the current annotation text scale
  vtkMRMLAnnotationTextDisplayNode *textDisplayNode = this->m_logic->GetTextDisplayNode(this->m_id.c_str());
  if (textDisplayNode)
    {
    double textScale = textDisplayNode->GetTextScale();
    ui.textScaleSliderSpinBoxWidget->setMaximum(120);
    ui.textScaleSliderSpinBoxWidget->setValue(textScale);

    double opacity = textDisplayNode->GetOpacity();
    ui.textOpacitySliderSpinBoxWidget->setValue(opacity);
    }



  // load the unselected text color
  double * unselectedColor = this->m_logic->GetAnnotationTextUnselectedColor(
      this->m_id.c_str());
  QColor unselectedQColor;
  if (unselectedColor)
  {
    this->TurnColorArrayToQColor(unselectedColor,unselectedQColor);
  }

  ui.textUnselectedColorPickerButton->setDisplayColorName(false);
  ui.textUnselectedColorPickerButton->setColor(unselectedQColor);
  ui.textUnselectedColorPickerButton->setDialogOptions(ctkColorPickerButton::UseCTKColorDialog);

  // load the selected text color
  double * selectedColor = this->m_logic->GetAnnotationTextSelectedColor(
      this->m_id.c_str());
  QColor selectedQColor;
  if (selectedColor)
  {
    this->TurnColorArrayToQColor(selectedColor,selectedQColor);
  }

  ui.textSelectedColorPickerButton->setDisplayColorName(false);
  ui.textSelectedColorPickerButton->setColor(selectedQColor);
  ui.textSelectedColorPickerButton->setDialogOptions(ctkColorPickerButton::UseCTKColorDialog);

  // load the lock/unlock status
  int locked = this->m_logic->GetAnnotationLockedUnlocked(this->m_id.c_str());

  if (!locked)
    {
    ui.lockUnlockButton->setIcon(QIcon(":/Icons/AnnotationUnlock.png"));
    ui.lockUnlockButton->setToolTip(QString("Click to lock this annotation"));
    }
  else
    {
    ui.lockUnlockButton->setIcon(QIcon(":/Icons/AnnotationLock.png"));
    ui.lockUnlockButton->setToolTip(QString("This annotation is locked. Click to unlock!"));
    }

  this->lockUnlockInterface(locked);

  // load the visibility status
  int visible = this->m_logic->GetAnnotationVisibility(this->m_id.c_str());

  if (!visible)
    {
    ui.visibleInvisibleButton->setIcon(QIcon(":/Icons/AnnotationInvisible.png"));
    ui.visibleInvisibleButton->setToolTip(QString("Click to show this annotation"));
    }
  else
    {
    ui.visibleInvisibleButton->setIcon(QIcon(":/Icons/AnnotationVisibility.png"));
    ui.visibleInvisibleButton->setToolTip(QString("Click to hide this annotation"));
    }

  vtkMRMLNode *mrmlNode = this->m_logic->GetMRMLScene()->GetNodeByID(this->m_id.c_str());
  if (!mrmlNode)
    {
    return;
    }
  vtkMRMLAnnotationControlPointsNode *pointsNode = vtkMRMLAnnotationControlPointsNode::SafeDownCast(mrmlNode);
  vtkMRMLAnnotationPointDisplayNode *pointDisplayNode = nullptr;
  if (pointsNode)
    {
    // get the point display node
    pointDisplayNode = pointsNode->GetAnnotationPointDisplayNode();

    // point values
    // clear out the table first?
    //ui.pointsTableWidget->clear();
  /*  for (int p = 0; p < pointsNode->GetNumberOfControlPoints(); p++)
      {
      ui.pointsTableWidget->insertRow(p);
      const char *name = pointsNode->GetName();
      if (name)
        {
        ui.pointsTableWidget->setItem(p,0,new QTableWidgetItem(name));
        }
      double *coord = pointsNode->GetControlPointCoordinates(p);

      if (coord)
        {
        QString qnum;
        qnum.setNum(coord[0]);
        ui.pointsTableWidget->setItem(p,1,new QTableWidgetItem(qnum));
        qnum.setNum(coord[1]);
        ui.pointsTableWidget->setItem(p,2,new QTableWidgetItem(qnum));
        qnum.setNum(coord[2]);
        ui.pointsTableWidget->setItem(p,3,new QTableWidgetItem(qnum));
        }
      }
    }
  if (pointDisplayNode)
    {


    // unselected color
    double *pointUnSelColor = pointDisplayNode->GetColor();
    QColor pointUnSelQColor;
    this->TurnColorArrayToQColor(pointUnSelColor,pointUnSelQColor);
    ui.pointUnselectedColorPickerButton->setDisplayColorName(false);
    ui.pointUnselectedColorPickerButton->setColor(pointUnSelQColor);
    ui.pointUnselectedColorPickerButton->setDialogOptions(ctkColorPickerButton::UseCTKColorDialog);

    // selected color
    double *pointSelColor = pointDisplayNode->GetSelectedColor();
    QColor pointSelQColor;
    this->TurnColorArrayToQColor(pointSelColor, pointSelQColor);
    ui.pointSelectedColorPickerButton->setDisplayColorName(false);
    ui.pointSelectedColorPickerButton->setColor(pointSelQColor);
    ui.pointSelectedColorPickerButton->setDialogOptions(ctkColorPickerButton::UseCTKColorDialog);

    // load the glyph type


    // fill in the combo box with glyph types if not already done
    if (ui.pointGlyphTypeComboBox->count() == 0)
      {
      vtkMRMLAnnotationPointDisplayNode *displayNode = vtkMRMLAnnotationPointDisplayNode::New();
      int min = displayNode->GetMinimumGlyphType();
      int max = displayNode->GetMaximumGlyphType();
      ui.pointGlyphTypeComboBox->setEnabled(false);
      for (int i = min; i <= max; i++)
        {
        displayNode->SetGlyphType(i);
        ui.pointGlyphTypeComboBox->addItem(displayNode->GetGlyphTypeAsString());
        //std::cout << "Adding glyphs to the combo box i = " << i << ", glyph type = " << displayNode->GetGlyphType() << ", as string = " << displayNode->GetGlyphTypeAsString() << ", combo box current index = " << ui.pointGlyphTypeComboBox->currentIndex() << ", count = " << ui.pointGlyphTypeComboBox->count() << std::endl;
        }
      ui.pointGlyphTypeComboBox->setEnabled(true);
      displayNode->Delete();
      }
    int index =  ui.pointGlyphTypeComboBox->findData(QString(pointDisplayNode->GetGlyphTypeAsString()));
//    std::cout << "glyph type = " << pointDisplayNode->GetGlyphType() << ", as string = " << pointDisplayNode->GetGlyphTypeAsString() << ", index = " << index << ", combo box size = " << ui.pointGlyphTypeComboBox->count() << std::endl;
    if (index != -1)
      {
      ui.pointGlyphTypeComboBox->setCurrentIndex(index);
      }
    else
      {
      // glyph types start at 1, combo box is 0 indexed
      ui.pointGlyphTypeComboBox->setCurrentIndex(pointDisplayNode->GetGlyphType() - 1);
//      std::cout << "\tset current index via the glyph type " << pointDisplayNode->GetGlyphType() << ", current index = " << ui.pointGlyphTypeComboBox->currentIndex() << std::endl;
      }
    // glyph size
    ui.pointSizeSliderSpinBoxWidget->setValue(pointDisplayNode->GetGlyphScale());

    // glyph material properties
    ui.pointOpacitySliderSpinBoxWidget->setValue(pointDisplayNode->GetOpacity());
    ui.pointAmbientSliderSpinBoxWidget->setValue(pointDisplayNode->GetAmbient());
    ui.pointDiffuseSliderSpinBoxWidget->setValue(pointDisplayNode->GetDiffuse());
    ui.pointSpecularSliderSpinBoxWidget->setValue(pointDisplayNode->GetSpecular());
    }

  /// Lines
  // get the line version of the node
  vtkMRMLAnnotationLinesNode *linesNode = vtkMRMLAnnotationLinesNode::SafeDownCast(mrmlNode);
  vtkMRMLAnnotationLineDisplayNode *lineDisplayNode = nullptr;
  if (linesNode)
    {
    lineDisplayNode = linesNode->GetAnnotationLineDisplayNode();
    }

  if (!lineDisplayNode)
    {
    // disable the lines tab
    ui.tabWidget->setTabEnabled(2, false);
    }
  else
    {
    // enable the lines tab
    ui.tabWidget->setTabEnabled(2, true);
    // colours

    // unselected
    double *lineUnSelColor = lineDisplayNode->GetColor();
    QColor lineUnSelQColor;
    this->TurnColorArrayToQColor(lineUnSelColor,lineUnSelQColor);
    ui.lineUnselectedColorPickerButton->setDisplayColorName(false);
    ui.lineUnselectedColorPickerButton->setColor(lineUnSelQColor);
    ui.lineUnselectedColorPickerButton->setDialogOptions(ctkColorPickerButton::UseCTKColorDialog);

    // selected
    double *lineSelColor = lineDisplayNode->GetSelectedColor();
    QColor lineSelQColor;
    this->TurnColorArrayToQColor(lineSelColor, lineSelQColor);
    ui.lineSelectedColorPickerButton->setDisplayColorName(false);
    ui.lineSelectedColorPickerButton->setColor(lineSelQColor);
    ui.lineSelectedColorPickerButton->setDialogOptions(ctkColorPickerButton::UseCTKColorDialog);

    // width
    ui.lineWidthSliderSpinBoxWidget_2->setValue(lineDisplayNode->GetLineThickness());
    // label position
    ui.lineLabelPositionSliderSpinBoxWidget->setValue(lineDisplayNode->GetLabelPosition());
    // label visibility
    ui.lineLabelVisibilityCheckBox->setChecked(lineDisplayNode->GetLabelVisibility());
    // tick spacing
    QString plainText;
    plainText.setNum(lineDisplayNode->GetTickSpacing());
    ui.lineTickSpacingLineEdit->setText(plainText);

    // max ticks
    ui.lineMaxTicksSliderSpinBoxWidget->setValue(lineDisplayNode->GetMaxTicks());

    // line material properties
    ui.lineOpacitySliderSpinBoxWidget_2->setValue(lineDisplayNode->GetOpacity());
    ui.lineAmbientSliderSpinBoxWidget_2->setValue(lineDisplayNode->GetAmbient());
    ui.lineDiffuseSliderSpinBoxWidget_2->setValue(lineDisplayNode->GetDiffuse());
    ui.lineSpecularSliderSpinBoxWidget_2->setValue(lineDisplayNode->GetSpecular());
    }

  /// ROI
  // get the line version of the node
  vtkMRMLAnnotationROINode *roiNode = vtkMRMLAnnotationROINode::SafeDownCast(mrmlNode);
  if (roiNode)
    {
    ui.tabWidget->setTabEnabled(3, true);
    ui.ROIWidget->setMRMLAnnotationROINode(roiNode);
    }

  else
    {
    ui.tabWidget->setTabEnabled(3, false);
    }
  */
    /*
   this->setWindowTitle(
   "Annotation Properties");
   this->SaveStateForUndo();

   // COORDINATES
   QDoubleValidator *doubleVal = new QDoubleValidator(
   this);

   for (int i = 0; i < m_logic->GetNumberOfControlPoints(
   vtkMRMLAnnotationControlPointsNode::SafeDownCast(
   node)) * 3; ++i)
   {
   m_lineEditList.push_back(
   new QLineEdit(
   ui.CTKCollapsibleGroupBox_4));
   m_lineEditList[i]->setValidator(
   doubleVal);
   }

   m_gridLayout = new QGridLayout(
   ui.CTKCollapsibleGroupBox_4);
   m_gridLayout->addWidget(
   ui.annotationTypeBrowser,
   0,
   0,
   2,
   3);
   m_gridLayout->addWidget(
   ui.textLabel,
   2,
   0,
   1,
   1);
   m_gridLayout->addWidget(
   ui.annotationTextEdit,
   2,
   1,
   2,
   2);
   m_gridLayout->addWidget(
   ui.annotationValueBrowser,
   4,
   0,
   2,
   3);
   m_gridLayout->addWidget(
   ui.coordinatesLabel,
   6,
   0,
   1,
   3);

   double* pos;
   for (int id = 0; id < m_logic->GetNumberOfControlPoints(
   node); ++id)
   {
   pos = m_logic->GetAnnotationControlPointsCoordinate(
   node,
   id);

   for (int i = 0; i < 3; i++)
   {
   QString posString(
   "");
   posString.append(
   QString(
   "%1").arg(
   QString::number(
   double(
   pos[i]),
   'f',
   2)));
   this->m_lineEditList[i + id * 3]->setText(
   posString);
   this->connect(
   this->m_lineEditList[i + id * 3],
   SIGNAL(textChanged(QString)),
   this,
   SLOT(onCoordinateChanged(QString)));
   this->m_gridLayout->addWidget(
   m_lineEditList[i + id * 3],
   7 + id,
   i % 3,
   1,
   1);
   }
   }

   // Lock/Unlock properties
   if (vtkMRMLAnnotationNode::SafeDownCast(
   node) != nullptr)
   {
   if (vtkMRMLAnnotationNode::SafeDownCast(
   node)->GetLocked() == 1)
   {
   ui.annotationTextEdit->setEnabled(
   false);
   for (int i = 0; i < m_logic->GetNumberOfControlPoints(
   vtkMRMLAnnotationControlPointsNode::SafeDownCast(
   node)) * 3; ++i)
   {
   m_lineEditList[i]->setEnabled(
   false);
   }
   ui.textTab->setEnabled(
   false);
   ui.pointTab->setEnabled(
   false);
   ui.lineTab->setEnabled(
   false);
   }
   else
   {
   ui.annotationTextEdit->setEnabled(
   true);
   for (int i = 0; i < m_logic->GetNumberOfControlPoints(
   vtkMRMLAnnotationControlPointsNode::SafeDownCast(
   node)) * 3; ++i)
   {
   m_lineEditList[i]->setEnabled(
   true);
   }
   ui.textTab->setEnabled(
   true);
   ui.pointTab->setEnabled(
   true);
   ui.lineTab->setEnabled(
   true);
   }

   }

   // Type
   QString typeString;
   typeString.append(
   "<p>Annotation Type: ") .append(
   "<img src='") .append(
   m_logic->GetIconName(
   node,
   false)) .append(
   "'>") .append(
   "</p>");
   ui.annotationTypeBrowser->setHtml(
   typeString);

   // Text Properties
   QString textString = QString(
   m_logic->GetAnnotationTextProperty(
   node));
   ui.annotationTextEdit->setText(
   textString);

   // Text Format
   //const char* textFormat = m_logic->GetAnnotationTextFormatProperty(node);

   // Value
   std::vector<double> vv = m_logic->GetAnnotationMeasurement(
   node);
   char valuechar[100];
   QString valueString;
   //sprintf( valuechar, textFormat, vv);
   valueString.append(
   "<p>Value: <b>").append(
   QString(
   valuechar)).append(
   "</b></p>");
   ui.annotationValueBrowser->setHtml(
   valueString);

   double value;

   // Default CollapsibleGroupBox Properties
   QVBoxLayout* groupBoxLayout = new QVBoxLayout;
   ui.displayPropertiesCTKCollapsibleGroupBox->setLayout(
   groupBoxLayout);
   ui.displayPropertiesCTKCollapsibleGroupBox->setEnabled(
   true);
   ui.displayPropertiesCTKCollapsibleGroupBox->setChecked(
   false);

   if (node->IsA(
   "vtkMRMLAnnotationStickyNode"))
   {
   ui.coordinatesLabel->setVisible(
   false);
   ui.annotationValueBrowser->setVisible(
   false);
   ui.displayPropertiesCTKCollapsibleGroupBox->setEnabled(
   false);
   ui.displayPropertiesCTKCollapsibleGroupBox->setVisible(
   false);
   return;
   }

   if (node->IsA(
   "vtkMRMLAnnotationTextNode"))
   {
   ui.coordinatesLabel->setVisible(
   true);
   ui.annotationValueBrowser->setVisible(
   false);
   ui.displayPropertiesCTKCollapsibleGroupBox->setEnabled(
   true);
   ui.displayPropertiesCTKCollapsibleGroupBox->setChecked(
   true);

   ui.lineTab->setVisible(
   false);
   ui.pointTab->setVisible(
   false);
   return;
   }

   // Text Display Properties
   if (node->IsA(
   "vtkMRMLFiducialListNode"))
   {
   vtkMRMLFiducialListNode* fNode = vtkMRMLFiducialListNode::SafeDownCast(
   node);
   double* color;
   QColor qcolor;

   color = fNode->GetColor();
   this->TurnColorArrayToQColor(
   color,
   qcolor);
   ui.textUnselectedColorPickerButton->setColor(
   qcolor);
   ui.textUnselectedColorPickerButton->setText(
   "Selected Text Color");
   ui.pointUnselectedColorPickerButton->setColor(
   qcolor);
   ui.pointUnselectedColorPickerButton->setText(
   "Selected Point Color");
   color = fNode->GetSelectedColor();
   this->TurnColorArrayToQColor(
   color,
   qcolor);
   ui.textSelectedColorPickerButton->setColor(
   qcolor);
   ui.textSelectedColorPickerButton->setText(
   "Text Color");
   ui.pointSelectedColorPickerButton->setColor(
   qcolor);
   ui.pointSelectedColorPickerButton->setText(
   "Point Color");

   ui.textScaleSliderSpinBoxWidget->setValue(
   fNode->GetTextScale());
   ui.pointOpacitySliderSpinBoxWidget->setValue(
   fNode->GetOpacity());
   ui.pointAmbientSliderSpinBoxWidget->setValue(
   fNode->GetAmbient());
   ui.pointDiffuseSliderSpinBoxWidget->setValue(
   fNode->GetDiffuse());
   ui.pointSpecularSliderSpinBoxWidget->setValue(
   fNode->GetSpecular());

   ui.lineTab->setEnabled(
   false);
   return;
   }

   double* color;
   QColor qcolor;
   color = m_logic->GetAnnotationLinesPropertiesColor(
   node,
   m_logic->TEXT_COLOR);
   this->TurnColorArrayToQColor(
   color,
   qcolor);
   ui.textUnselectedColorPickerButton->setColor(
   qcolor);
   //ui.textUnselectedColorPickerButton->setColor(m_logic->getColor(node, m_logic->TEXT_COLOR));
   color = m_logic->GetAnnotationLinesPropertiesColor(
   node,
   m_logic->TEXT_SELECTED_COLOR);
   this->TurnColorArrayToQColor(
   color,
   qcolor);
   ui.textSelectedColorPickerButton->setColor(
   qcolor);
   //ui.textSelectedColorPickerButton->setColor(m_logic->getColor(node, m_logic->TEXT_SELECTED_COLOR));
   ui.textUnselectedColorPickerButton->setText(
   "Selected Text Color");
   ui.textSelectedColorPickerButton->setText(
   "Text Color");
   m_logic->GetAnnotationLinesPropertiesDouble(
   node,
   m_logic->TEXT_SCALE,
   value);
   ui.textScaleSliderSpinBoxWidget->setValue(
   value);

   // Point Display Properties
   color = m_logic->GetAnnotationLinesPropertiesColor(
   node,
   m_logic->POINT_COLOR);
   this->TurnColorArrayToQColor(
   color,
   qcolor);
   ui.pointUnselectedColorPickerButton->setColor(
   qcolor);
   color = m_logic->GetAnnotationLinesPropertiesColor(
   node,
   m_logic->POINT_SELECTED_COLOR);
   this->TurnColorArrayToQColor(
   color,
   qcolor);
   ui.pointSelectedColorPickerButton->setColor(
   qcolor);
   ui.pointUnselectedColorPickerButton->setText(
   "Selected Point Color");
   ui.pointSelectedColorPickerButton->setText(
   "Point Color");
   //ui.pointSizeSliderSpinBoxWidget->setValue( m_logic->getPointSize(node) ); // ToDo
   m_logic->GetAnnotationLinesPropertiesDouble(
   node,
   m_logic->POINT_OPACITY,
   value);
   ui.pointOpacitySliderSpinBoxWidget->setValue(
   value);
   m_logic->GetAnnotationLinesPropertiesDouble(
   node,
   m_logic->POINT_AMBIENT,
   value);
   ui.pointAmbientSliderSpinBoxWidget->setValue(
   value);
   m_logic->GetAnnotationLinesPropertiesDouble(
   node,
   m_logic->POINT_DIFFUSE,
   value);
   ui.pointDiffuseSliderSpinBoxWidget->setValue(
   value);
   m_logic->GetAnnotationLinesPropertiesDouble(
   node,
   m_logic->POINT_SPECULAR,
   value);
   ui.pointSpecularSliderSpinBoxWidget->setValue(
   value);

   // Line Display Properties
   color = m_logic->GetAnnotationLinesPropertiesColor(
   node,
   m_logic->LINE_COLOR);
   this->TurnColorArrayToQColor(
   color,
   qcolor);
   ui.lineUnselectedColorPickerButton->setColor(
   qcolor);
   color = m_logic->GetAnnotationLinesPropertiesColor(
   node,
   m_logic->LINE_SELECTED_COLOR);
   this->TurnColorArrayToQColor(
   color,
   qcolor);
   ui.lineSelectedColorPickerButton->setColor(
   qcolor);
   ui.lineUnselectedColorPickerButton->setText(
   "Selected Line Color");
   ui.lineSelectedColorPickerButton->setText(
   "Line Color");
   m_logic->GetAnnotationLinesPropertiesDouble(
   node,
   m_logic->LINE_WIDTH,
   value);
   ui.lineWidthSliderSpinBoxWidget->setValue(
   value);
   m_logic->GetAnnotationLinesPropertiesDouble(
   node,
   m_logic->LINE_OPACITY,
   value);
   ui.lineOpacitySliderSpinBoxWidget->setValue(
   value);
   m_logic->GetAnnotationLinesPropertiesDouble(
   node,
   m_logic->LINE_AMBIENT,
   value);
   ui.lineAmbientSliderSpinBoxWidget->setValue(
   value);
   m_logic->GetAnnotationLinesPropertiesDouble(
   node,
   m_logic->LINE_DIFFUSE,
   value);
   ui.lineDiffuseSliderSpinBoxWidget->setValue(
   value);
   m_logic->GetAnnotationLinesPropertiesDouble(
   node,
   m_logic->LINE_SPECULAR,
   value);
   ui.lineSpecularSliderSpinBoxWidget->setValue(
   value);
   */
}

//------------------------------------------------------------------------------
void qSlicerAnnotationModulePropertyDialog::createConnection()
{
  this->connect(this, SIGNAL(rejected()), this, SLOT(onDialogRejected()));
  this->connect(this, SIGNAL(accepted()), this, SLOT(onDialogAccepted()));

  // text
  this->connect(ui.annotationTextEdit, SIGNAL(textChanged()), this,
      SLOT(onTextChanged()));

  this->connect(ui.textScaleSliderSpinBoxWidget, SIGNAL(valueChanged(double)),
      this, SLOT(onTextScaleChanged(double)));

  this->connect(ui.textSelectedColorPickerButton, SIGNAL(colorChanged(QColor)),
      this, SLOT(onTextSelectedColorChanged(QColor)));

  this->connect(ui.textUnselectedColorPickerButton, SIGNAL(colorChanged(QColor)),
      this, SLOT(onTextUnselectedColorChanged(QColor)));

  this->connect(ui.textOpacitySliderSpinBoxWidget, SIGNAL(valueChanged(double)),
      this, SLOT(onTextOpacityChanged(double)));

  // point
  //this->connect(ui.pointsTableWidget, SIGNAL(itemChanged(QTableWidgetItem*)),
  //              this, SLOT(onPointsTableWidgetChanged(QTableWidgetItem*)));
  this->connect(ui.pointUnselectedColorPickerButton, SIGNAL(colorChanged(QColor)),
                this, SLOT(onPointColorChanged(QColor)));
  this->connect(ui.pointSelectedColorPickerButton, SIGNAL(colorChanged(QColor)),
                this, SLOT(onPointSelectedColorChanged(QColor)));

  this->connect(ui.pointSizeSliderSpinBoxWidget, SIGNAL(valueChanged(double)),
                this, SLOT(onPointSizeChanged(double)));

  this->connect(ui.pointGlyphTypeComboBox, SIGNAL(currentIndexChanged(QString)),
                this, SLOT(onPointGlyphChanged(QString)));

//  this->connect(ui.pointOpacitySliderSpinBoxWidget, SIGNAL(valueChanged(double)),
  //              this, SLOT(onPointOpacityChanged(double)));
  //this->connect(ui.pointAmbientSliderSpinBoxWidget, SIGNAL(valueChanged(double)),
  //              this, SLOT(onPointAmbientChanged(double)));
  //this->connect(ui.pointDiffuseSliderSpinBoxWidget, SIGNAL(valueChanged(double)),
   //             this, SLOT(onPointDiffuseChanged(double)));
  //this->connect(ui.pointSpecularSliderSpinBoxWidget, SIGNAL(valueChanged(double)),
  //              this, SLOT(onPointSpecularChanged(double)));

  // line
  this->connect(ui.lineUnselectedColorPickerButton, SIGNAL(colorChanged(QColor)),
                this, SLOT(onLineColorChanged(QColor)));
  this->connect(ui.lineSelectedColorPickerButton, SIGNAL(colorChanged(QColor)),
                this, SLOT(onLineSelectedColorChanged(QColor)));

  this->connect(ui.lineLabelPositionSliderSpinBoxWidget, SIGNAL(valueChanged(double)),
                this, SLOT(onLineLabelPositionChanged(double)));
  this->connect(ui.lineLabelVisibilityCheckBox, SIGNAL(stateChanged(int)),
                this, SLOT(onLineLabelVisibilityStateChanged(int)));
  this->connect(ui.lineTickSpacingLineEdit, SIGNAL(textChanged(QString)),
                this, SLOT(onLineTickSpacingChanged()));

  this->connect(ui.lineMaxTicksSliderSpinBoxWidget, SIGNAL(valueChanged(double)),
                this, SLOT(onLineMaxTicksChanged(double)));

  // line material properties
  //this->connect(ui.lineOpacitySliderSpinBoxWidget_2, SIGNAL(valueChanged(double)),
  //              this, SLOT(onLineOpacityChanged(double)));
  //this->connect(ui.lineAmbientSliderSpinBoxWidget_2, SIGNAL(valueChanged(double)),
//                this, SLOT(onLineAmbientChanged(double)));
 // this->connect(ui.lineDiffuseSliderSpinBoxWidget_2, SIGNAL(valueChanged(double)),
  //              this, SLOT(onLineDiffuseChanged(double)));
  //this->connect(ui.lineSpecularSliderSpinBoxWidget_2, SIGNAL(valueChanged(double)),
   //             this, SLOT(onLineSpecularChanged(double)));

  this->connect(ui.lockUnlockButton, SIGNAL(clicked()), this, SLOT(onLockUnlockButtonClicked()));
  this->connect(ui.visibleInvisibleButton, SIGNAL(clicked()), this, SLOT(onVisibleInvisibleButtonClicked()));
   /*
   this->connect(
   ui.annotationTextEdit,
   SIGNAL(textChanged()),
   this,
   SLOT(onTextChanged()));
   this->connect(
   this,
   SIGNAL(rejected()),
   this,
   SLOT(onDialogRejected()));
   this->connect(
   this,
   SIGNAL(accepted()),
   this,
   SLOT(onDialogAccepted()));

   this->connect(
   ui.textUnselectedColorPickerButton,
   SIGNAL(colorChanged(QColor)),
   this,
   SLOT(onTextColorChanged(QColor)));
   this->connect(
   ui.textSelectedColorPickerButton,
   SIGNAL(colorChanged(QColor)),
   this,
   SLOT(onTextSelectedColorChanged(QColor)));
   this->connect(
   ui.textScaleSliderSpinBoxWidget,
   SIGNAL(valueChanged(double)),
   this,
   SLOT(onTextScaleChanged(double)));

   this->connect(
   ui.pointUnselectedColorPickerButton,
   SIGNAL(colorChanged(QColor)),
   this,
   SLOT(onPointColorChanged(QColor)));
   this->connect(
   ui.pointSelectedColorPickerButton,
   SIGNAL(colorChanged(QColor)),
   this,
   SLOT(onPointSelectedColorChanged(QColor)));
   this->connect(
   ui.pointSizeSliderSpinBoxWidget,
   SIGNAL(valueChanged(double)),
   this,
   SLOT(onPointSizeChanged(double)));
   this->connect(
   ui.pointOpacitySliderSpinBoxWidget,
   SIGNAL(valueChanged(double)),
   this,
   SLOT(onPointOpacityChanged(double)));
   this->connect(
   ui.pointAmbientSliderSpinBoxWidget,
   SIGNAL(valueChanged(double)),
   this,
   SLOT(onPointAmbientChanged(double)));
   this->connect(
   ui.pointDiffuseSliderSpinBoxWidget,
   SIGNAL(valueChanged(double)),
   this,
   SLOT(onPointDiffuseChanged(double)));
   this->connect(
   ui.pointSpecularSliderSpinBoxWidget,
   SIGNAL(valueChanged(double)),
   this,
   SLOT(onPointSpecularChanged(double)));

   this->connect(
   ui.lineUnselectedColorPickerButton,
   SIGNAL(colorChanged(QColor)),
   this,
   SLOT(onLineColorChanged(QColor)));
   this->connect(
   ui.lineSelectedColorPickerButton,
   SIGNAL(colorChanged(QColor)),
   this,
   SLOT(onLineSelectedColorChanged(QColor)));
   this->connect(
   ui.lineWidthSliderSpinBoxWidget,
   SIGNAL(valueChanged(double)),
   this,
   SLOT(onLineWidthChanged(double)));
   this->connect(
   ui.lineOpacitySliderSpinBoxWidget,
   SIGNAL(valueChanged(double)),
   this,
   SLOT(onLineOpacityChanged(double)));
   this->connect(
   ui.lineAmbientSliderSpinBoxWidget,
   SIGNAL(valueChanged(double)),
   this,
   SLOT(onLineAmbientChanged(double)));
   this->connect(
   ui.lineDiffuseSliderSpinBoxWidget,
   SIGNAL(valueChanged(double)),
   this,
   SLOT(onLineDiffuseChanged(double)));
   this->connect(
   ui.lineSpecularSliderSpinBoxWidget,
   SIGNAL(valueChanged(double)),
   this,
   SLOT(onLineSpecularChanged(double)));

   this->connect(
   ui.CTKCollapsibleGroupBox_4,
   SIGNAL(clicked()),
   this,
   SLOT(onCollapsibleGroupBoxClicked()));
   */
}

//------------------------------------------------------------------------------
void qSlicerAnnotationModulePropertyDialog::onCoordinateChanged(QString text)
{
  Q_UNUSED(text)
  /*
   if (this->m_isUpdated)
   {
   return;
   }

   std::vector<double> positions;
   QString valueString;
   std::vector<double> thevalue;
   const char* format;

   for (int i = 0; i < m_lineEditList.size(); ++i)
   {
   positions.push_back(m_lineEditList[i]->text().toDouble());
   }

   // update widget
   vtkMRMLNode* node = this->m_logic->GetMRMLScene()->GetNodeByID(m_nodeId);
   int num = positions.size() / 3;
   double pos[3];
   for (int id = 0; id < num; ++id)
   {
   pos[0] = positions[id * 3];
   pos[1] = positions[id * 3 + 1];
   pos[2] = positions[id * 3 + 2];
   this->m_logic->SetAnnotationControlPointsCoordinate(node, pos, id);
   }

   // update value in the property dialog
   thevalue = this->m_logic->GetAnnotationMeasurement(node);
   format = this->m_logic->GetAnnotationTextFormatProperty(node);
   this->FormatValueToChar(format, thevalue, valueString);
   this->updateValue(valueString);

   emit coordinateChanged(valueString, m_nodeId);
   */
}

//------------------------------------------------------------------------------
void qSlicerAnnotationModulePropertyDialog::onTextChanged()
{
  QString text = ui.annotationTextEdit->toPlainText();
  this->m_logic->SetAnnotationText(this->m_id.c_str(), text.toUtf8());
}

//------------------------------------------------------------------------------
void qSlicerAnnotationModulePropertyDialog::updateTextFromTable(QString text)
{
  Q_UNUSED(text)
  // Text Properties
  // ui.annotationTextEdit->setText(
  //    text);
}

//------------------------------------------------------------------------------
void qSlicerAnnotationModulePropertyDialog::updateValue(QString valueString)
{
  QString valueStr;
  valueStr.append("<p>Value: <b>").append(valueString).append("</b></p>");
  // ui.annotationValueBrowser->setHtml(
  //    valueStr);
}

//------------------------------------------------------------------------------
void qSlicerAnnotationModulePropertyDialog::SaveLinesNode(vtkMRMLAnnotationLinesNode* node)
{
  Q_UNUSED(node);

  /*
 if (!node)
 {
 return;
 }
 if (!this->m_lineDispCopy)
 {
 this->m_lineDispCopy = vtkMRMLAnnotationLineDisplayNode::New();
 }

 node->CreateAnnotationLineDisplayNode();
 this->m_lineDispCopy->Copy(node->GetAnnotationLineDisplayNode());
 this->SaveControlPoints(node);
 */
}

//------------------------------------------------------------------------------
void qSlicerAnnotationModulePropertyDialog::SaveControlPoints(vtkMRMLAnnotationControlPointsNode* node)
{
  Q_UNUSED(node);
  /*
   if (!node)
   {
   return;
   }

   if (!this->m_pointDispCopy)
   {
   this->m_pointDispCopy = vtkMRMLAnnotationPointDisplayNode::New();
   }
   node->CreateAnnotationPointDisplayNode();
   this->m_pointDispCopy->Copy(node->GetAnnotationPointDisplayNode());
   this->SaveAnnotationNode((vtkMRMLAnnotationNode*) node);
   */
}

//------------------------------------------------------------------------------
void qSlicerAnnotationModulePropertyDialog::SaveAnnotationNode(vtkMRMLAnnotationNode* node)
{
  Q_UNUSED(node);
  /*
   if (!node)
   {
   return;
   }

   if (!this->m_textDispCopy)
   {
   this->m_textDispCopy = vtkMRMLAnnotationTextDisplayNode::New();
   }
   node->CreateAnnotationTextDisplayNode();
   this->m_textDispCopy->Copy(node->GetAnnotationTextDisplayNode());
   */
}

//------------------------------------------------------------------------------
void qSlicerAnnotationModulePropertyDialog::SaveStateForUndo(vtkMRMLNode* node)
{
  Q_UNUSED(node);
  /*
   if (node->IsA("vtkMRMLAnnotationAngleNode"))
   {
   vtkMRMLAnnotationAngleNode* mynode =
   vtkMRMLAnnotationAngleNode::SafeDownCast(node);
   if (!this->m_angleCopy)
   {
   this->m_angleCopy = vtkMRMLAnnotationAngleNode::New();
   }
   this->m_angleCopy->Copy(mynode);
   this->SaveLinesNode(mynode);
   }
   else if (node->IsA("vtkMRMLAnnotationRulerNode"))
   {
   vtkMRMLAnnotationRulerNode* mynode =
   vtkMRMLAnnotationRulerNode::SafeDownCast(node);
   if (!this->m_rulerCopy)
   {
   this->m_rulerCopy = vtkMRMLAnnotationRulerNode::New();
   }
   this->m_rulerCopy->Copy(mynode);
   this->SaveLinesNode(mynode);
   }
   else if (node->IsA("vtkMRMLAnnotationFiducialNode"))
   {
   vtkMRMLAnnotationFiducialNode* fiducialNode =
   vtkMRMLAnnotationFiducialNode::SafeDownCast(node);
   fiducialNode->CreateAnnotationTextDisplayNode();
   fiducialNode->CreateAnnotationPointDisplayNode();
   fiducialNode->GetScene()->SaveStateForUndo();
   }
   */
}

//------------------------------------------------------------------------------
void qSlicerAnnotationModulePropertyDialog::UndoLinesNode(vtkMRMLAnnotationLinesNode* node)
{
  Q_UNUSED(node);
  /*
   if (!node)
   {
   return;
   }
   node->CreateAnnotationLineDisplayNode();
   node->GetAnnotationLineDisplayNode()->Copy(m_lineDispCopy);
   this->UndoControlPoints(node);
   */
}

//------------------------------------------------------------------------------
void qSlicerAnnotationModulePropertyDialog::UndoControlPoints(vtkMRMLAnnotationControlPointsNode* node)
{
  Q_UNUSED(node);
  /*
   if (!node)
   {
   return;
   }
   node->CreateAnnotationPointDisplayNode();
   node->GetAnnotationPointDisplayNode()->Copy(m_pointDispCopy);
   this->UndoAnnotationNode((vtkMRMLAnnotationNode*) node);
   */
}

//------------------------------------------------------------------------------
void qSlicerAnnotationModulePropertyDialog::UndoAnnotationNode(vtkMRMLAnnotationNode* node)
{
  Q_UNUSED(node);
  /*
   if (!node)
   {
   return;
   }
   node->CreateAnnotationTextDisplayNode();
   node->GetAnnotationTextDisplayNode()->Copy(m_textDispCopy);
   */
}

//------------------------------------------------------------------------------
void qSlicerAnnotationModulePropertyDialog::Undo(vtkMRMLNode* node)
{
  Q_UNUSED(node);
  /*
   if (node->IsA("vtkMRMLAnnotationAngleNode"))
   {
   vtkMRMLAnnotationAngleNode* anode =
   vtkMRMLAnnotationAngleNode::SafeDownCast(node);
   anode->Copy(m_angleCopy);
   this->UndoLinesNode(anode);
   }
   else if (node->IsA("vtkMRMLAnnotationRulerNode"))
   {
   vtkMRMLAnnotationRulerNode* rnode =
   vtkMRMLAnnotationRulerNode::SafeDownCast(node);
   rnode->Copy(m_rulerCopy);
   this->UndoLinesNode(rnode);
   }
   else if (node->IsA("vtkMRMLAnnotationFiducialNode"))
   {
   //ToDo
   }
   */
}

//------------------------------------------------------------------------------
void qSlicerAnnotationModulePropertyDialog::onTextUnselectedColorChanged(QColor qcolor)
{
  double color[3];
  this->TurnQColorToColorArray(color, qcolor);

  this->m_logic->SetAnnotationTextUnselectedColor(this->m_id.c_str(),color);
}

//------------------------------------------------------------------------------
void qSlicerAnnotationModulePropertyDialog::onTextSelectedColorChanged(QColor qcolor)
{
  double color[3];
  this->TurnQColorToColorArray(color, qcolor);

  this->m_logic->SetAnnotationTextSelectedColor(this->m_id.c_str(),color);
}

//------------------------------------------------------------------------------
void qSlicerAnnotationModulePropertyDialog::onTextScaleChanged(double value)
{
  this->m_logic->SetAnnotationTextScale(this->m_id.c_str(), value);
}

//------------------------------------------------------------------------------
void qSlicerAnnotationModulePropertyDialog::onTextOpacityChanged(double value)
{
  // get the text display node
  vtkMRMLAnnotationTextDisplayNode *textDisplayNode = this->m_logic->GetTextDisplayNode(this->m_id.c_str());
  if (!textDisplayNode)
    {
    return;
    }
  textDisplayNode->SetOpacity(value);
}

//------------------------------------------------------------------------------
void qSlicerAnnotationModulePropertyDialog::onLockUnlockButtonClicked()
{
  // toggle the lock flag
  this->m_logic->SetAnnotationLockedUnlocked(this->m_id.c_str());

  int locked = this->m_logic->GetAnnotationLockedUnlocked(this->m_id.c_str());

  if (!locked)
    {
    ui.lockUnlockButton->setIcon(QIcon(":/Icons/AnnotationUnlock.png"));
    ui.lockUnlockButton->setToolTip(QString("Click to lock this annotation"));
    }
  else
    {
    ui.lockUnlockButton->setIcon(QIcon(":/Icons/AnnotationLock.png"));
    ui.lockUnlockButton->setToolTip(QString("This annotation is locked. Click to unlock!"));
    }

  this->lockUnlockInterface(locked);
}

//------------------------------------------------------------------------------
void qSlicerAnnotationModulePropertyDialog::onVisibleInvisibleButtonClicked()
{
  this->m_logic->SetAnnotationVisibility(this->m_id.c_str());

  // load the visibility status
  int visible = this->m_logic->GetAnnotationVisibility(this->m_id.c_str());

  if (!visible)
    {
    ui.visibleInvisibleButton->setIcon(QIcon(":/Icons/AnnotationInvisible.png"));
    ui.visibleInvisibleButton->setToolTip(QString("Click to show this annotation"));
    }
  else
    {
    ui.visibleInvisibleButton->setIcon(QIcon(":/Icons/AnnotationVisibility.png"));
    ui.visibleInvisibleButton->setToolTip(QString("Click to hide this annotation"));
    }
}
/*
//------------------------------------------------------------------------------
void qSlicerAnnotationModulePropertyDialog::onPointsTableWidgetChanged(QTableWidgetItem *tableItem)
{
  if (tableItem == nullptr)
    {
    return;
    }
  int row = tableItem->row();
  int col = tableItem->column();
  QString newString = tableItem->text();
  double newValue = newString.toDouble();
  vtkMRMLNode *node = this->m_logic->GetMRMLScene()->GetNodeByID(this->m_id.c_str());
  if (!node)
    {
    return;
    }
  vtkMRMLAnnotationControlPointsNode *pointsNode = vtkMRMLAnnotationControlPointsNode::SafeDownCast(node);
  if (!pointsNode)
    {
    return;
    }
//  std::cout << "onPointsTableWidgetChanged: row = " << row << ", col = " << col << ", newValue = " << newValue << std::endl;
  // get the point coordinates corresponding to this row
  double *oldCoords = pointsNode->GetControlPointCoordinates(row);
  double newCoords[3];
  newCoords[0] = oldCoords[0];
  newCoords[1] = oldCoords[1];
  newCoords[2] = oldCoords[2];
  if (!oldCoords)
    {
    return;
    }
  if (col == 1)
    {
    // x
    if (newCoords[0] != newValue)
      {
      newCoords[0] = newValue;
      }
    }
  else if (col == 2)
    {
    // y
    if (newCoords[1] != newValue)
      {
      newCoords[1] = newValue;
      }
    }
  else if (col == 3)
    {
    // z
    if (newCoords[2] != newValue)
      {
      newCoords[2] = newValue;
      }
    }
  if (newCoords[0] != oldCoords[0] ||
      newCoords[1] != oldCoords[1] ||
      newCoords[2] != oldCoords[2])
    {
    //std::cout << "Setting control point for point " << row << ", to " << newCoords[0] << ", " << newCoords[1] << ", " << newCoords[2] << std::endl;
    pointsNode->SetControlPoint(row, newCoords);
    }
}*/
//------------------------------------------------------------------------------
void qSlicerAnnotationModulePropertyDialog::onPointColorChanged(QColor qcolor)
{
  double color[3];
  this->TurnQColorToColorArray(color, qcolor);

  this->m_logic->SetAnnotationPointUnselectedColor(this->m_id.c_str(),color);
}

//------------------------------------------------------------------------------
void qSlicerAnnotationModulePropertyDialog::onPointSelectedColorChanged(QColor qcolor)
{
  double color[3];
  this->TurnQColorToColorArray(color, qcolor);

  this->m_logic->SetAnnotationPointColor(this->m_id.c_str(),color);
}

//------------------------------------------------------------------------------
void qSlicerAnnotationModulePropertyDialog::onPointSizeChanged(double value)
{
  vtkMRMLNode* node = this->m_logic->GetMRMLScene()->GetNodeByID(this->m_id.c_str());
  if (!node)
    {
    return;
    }
  vtkMRMLAnnotationControlPointsNode *pointsNode = vtkMRMLAnnotationControlPointsNode::SafeDownCast(node);
  if (!pointsNode)
    {
    return;
    }
  // get the point display node
  vtkMRMLAnnotationPointDisplayNode *pointDisplayNode = pointsNode->GetAnnotationPointDisplayNode();
  if (!pointDisplayNode)
    {
    return;
    }
  pointDisplayNode->SetGlyphScale(value);
}

//------------------------------------------------------------------------------
void qSlicerAnnotationModulePropertyDialog::onPointOpacityChanged(double value)
{
  // get the point display node
  vtkMRMLAnnotationPointDisplayNode *pointDisplayNode = this->m_logic->GetPointDisplayNode(this->m_id.c_str());
  if (!pointDisplayNode)
    {
    return;
    }
  pointDisplayNode->SetOpacity(value);
}

//------------------------------------------------------------------------------
void qSlicerAnnotationModulePropertyDialog::onPointAmbientChanged(double value)
{
  // get the point display node
  vtkMRMLAnnotationPointDisplayNode *pointDisplayNode = this->m_logic->GetPointDisplayNode(this->m_id.c_str());
  if (!pointDisplayNode)
    {
    return;
    }
  pointDisplayNode->SetAmbient(value);
}

//------------------------------------------------------------------------------
void qSlicerAnnotationModulePropertyDialog::onPointDiffuseChanged(double value)
{
  // get the point display node
  vtkMRMLAnnotationPointDisplayNode *pointDisplayNode = this->m_logic->GetPointDisplayNode(this->m_id.c_str());
  if (!pointDisplayNode)
    {
    return;
    }
  pointDisplayNode->SetDiffuse(value);
}

//------------------------------------------------------------------------------
void qSlicerAnnotationModulePropertyDialog::onPointSpecularChanged(double value)
{
  // get the point display node
  vtkMRMLAnnotationPointDisplayNode *pointDisplayNode = this->m_logic->GetPointDisplayNode(this->m_id.c_str());
  if (!pointDisplayNode)
    {
    return;
    }
  pointDisplayNode->SetSpecular(value);
}

//------------------------------------------------------------------------------
void qSlicerAnnotationModulePropertyDialog::onPointGlyphChanged(QString value)
{
//  std::cout << "OnPointGlyphChanged: " << qPrintable(value) << std::endl;
  this->m_logic->SetAnnotationPointGlyphTypeFromString(this->m_id.c_str(),value.toUtf8());
}

//------------------------------------------------------------------------------
void qSlicerAnnotationModulePropertyDialog::onLineColorChanged(QColor qcolor)
{
  double color[3];
  this->TurnQColorToColorArray(color, qcolor);

  this->m_logic->SetAnnotationLineUnselectedColor(this->m_id.c_str(),color);
}

//------------------------------------------------------------------------------
void qSlicerAnnotationModulePropertyDialog::onLineSelectedColorChanged(QColor qcolor)
{
  double color[3];
  this->TurnQColorToColorArray(color, qcolor);

  this->m_logic->SetAnnotationLineColor(this->m_id.c_str(),color);
}

//------------------------------------------------------------------------------
void qSlicerAnnotationModulePropertyDialog::onLineWidthChanged(double value)
{
  // get the line display node
  vtkMRMLAnnotationLineDisplayNode *lineDisplayNode = this->m_logic->GetLineDisplayNode(this->m_id.c_str());
  if (!lineDisplayNode)
    {
    return;
    }
  lineDisplayNode->SetLineThickness(value);
}

//------------------------------------------------------------------------------
void qSlicerAnnotationModulePropertyDialog::onLineLabelPositionChanged(double value)
{
  // get the line display node
  vtkMRMLAnnotationLineDisplayNode *lineDisplayNode = this->m_logic->GetLineDisplayNode(this->m_id.c_str());
  if (!lineDisplayNode)
    {
    return;
    }
  lineDisplayNode->SetLabelPosition(value);
}

//------------------------------------------------------------------------------
void qSlicerAnnotationModulePropertyDialog::onLineLabelVisibilityStateChanged(int state)
{
  // get the line display node
  vtkMRMLAnnotationLineDisplayNode *lineDisplayNode = this->m_logic->GetLineDisplayNode(this->m_id.c_str());
  if (!lineDisplayNode)
    {
    return;
    }
  if (state)
    {
    lineDisplayNode->LabelVisibilityOn();
    }
  else
    {
    lineDisplayNode->LabelVisibilityOff();
    }
}

//------------------------------------------------------------------------------
void qSlicerAnnotationModulePropertyDialog::onLineTickSpacingChanged()
{
  // get the line display node
  vtkMRMLAnnotationLineDisplayNode *lineDisplayNode = this->m_logic->GetLineDisplayNode(this->m_id.c_str());
  if (!lineDisplayNode)
    {
    return;
    }
  QString plainText = ui.lineTickSpacingLineEdit->text();
  double value = plainText.toDouble();
  lineDisplayNode->SetTickSpacing(value);
}

//------------------------------------------------------------------------------
void qSlicerAnnotationModulePropertyDialog::onLineMaxTicksChanged(double value)
{
  // get the line display node
  vtkMRMLAnnotationLineDisplayNode *lineDisplayNode = this->m_logic->GetLineDisplayNode(this->m_id.c_str());
  if (!lineDisplayNode)
    {
    return;
    }
  lineDisplayNode->SetMaxTicks(int(value));
}

//------------------------------------------------------------------------------
void qSlicerAnnotationModulePropertyDialog::onLineOpacityChanged(double value)
{
  // get the line display node
  vtkMRMLAnnotationLineDisplayNode *lineDisplayNode = this->m_logic->GetLineDisplayNode(this->m_id.c_str());
  if (!lineDisplayNode)
    {
    return;
    }
  lineDisplayNode->SetOpacity(value);
}

//------------------------------------------------------------------------------
void qSlicerAnnotationModulePropertyDialog::onLineAmbientChanged(double value)
{
  // get the line display node
  vtkMRMLAnnotationLineDisplayNode *lineDisplayNode = this->m_logic->GetLineDisplayNode(this->m_id.c_str());
  if (!lineDisplayNode)
    {
    return;
    }
  lineDisplayNode->SetAmbient(value);
}

//------------------------------------------------------------------------------
void qSlicerAnnotationModulePropertyDialog::onLineDiffuseChanged(double value)
{
  // get the line display node
  vtkMRMLAnnotationLineDisplayNode *lineDisplayNode = this->m_logic->GetLineDisplayNode(this->m_id.c_str());
  if (!lineDisplayNode)
    {
    return;
    }
  lineDisplayNode->SetDiffuse(value);
}

//------------------------------------------------------------------------------
void qSlicerAnnotationModulePropertyDialog::onLineSpecularChanged(double value)
{
  // get the line display node
  vtkMRMLAnnotationLineDisplayNode *lineDisplayNode = this->m_logic->GetLineDisplayNode(this->m_id.c_str());
  if (!lineDisplayNode)
    {
    return;
    }
  lineDisplayNode->SetSpecular(value);
}

//------------------------------------------------------------------------------
void qSlicerAnnotationModulePropertyDialog::UpdateLockUnlockStatus(bool isLock)
{
  Q_UNUSED(isLock);
  /* if (isLock)
   {
   ui.annotationTextEdit->setEnabled(
   false);
   for (int i = 0; i < m_lineEditList.size(); ++i)
   {
   m_lineEditList[i]->setEnabled(
   false);
   }
   //   ui.textTab->setEnabled(
   //       false);
   //   ui.pointTab->setEnabled(
   //       false);
   //  ui.lineTab->setEnabled(
   //    false);
   }
   else
   {
   ui.annotationTextEdit->setEnabled(
   true);
   for (int i = 0; i < m_lineEditList.size(); ++i)
   {
   m_lineEditList[i]->setEnabled(
   true);
   }
   ui.textTab->setEnabled(
   true);
   ui.pointTab->setEnabled(
   true);
   ui.lineTab->setEnabled(
   true);
   }*/

}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModulePropertyDialog::TurnColorArrayToQColor(double* color, QColor &qcolor)
{
  qcolor.setRed(color[0] * 255.0001);
  qcolor.setGreen(color[1] * 255.0001);
  qcolor.setBlue(color[2] * 255.0001);
}

//------------------------------------------------------------------------------
void qSlicerAnnotationModulePropertyDialog::FormatValueToChar(const char* format, std::vector<
    double> vv, QString &valueString)
{
  char valuechar[100];
  QString tempString;
  valueString = "";
  foreach(double v, vv)
      {
      sprintf(valuechar, format, v);
      tempString = valuechar;
      tempString.append(" ");
      valueString.append(tempString);
      }

  //valueString = valuechar;
}

//------------------------------------------------------------------------------
void qSlicerAnnotationModulePropertyDialog::TurnQColorToColorArray(double* color, QColor &qcolor)
{
  color[0] = qcolor.red() / 255.0;
  color[1] = qcolor.green() / 255.0;
  color[2] = qcolor.blue() / 255.0;
}

//-----------------------------------------------------------------------------
// Methods for closing the property dialog
//-----------------------------------------------------------------------------
void qSlicerAnnotationModulePropertyDialog::onDialogRejected()
{
  // the user clicked cancel, now restore the backuped node
  this->m_logic->RestoreAnnotationNode(this->m_id.c_str());

  // delete all backups
  this->m_logic->DeleteBackupNodes(this->m_id.c_str());


  emit dialogRejected();
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModulePropertyDialog::onDialogAccepted()
{

  // delete all backups
  this->m_logic->DeleteBackupNodes(this->m_id.c_str());

  emit dialogAccepted();
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModulePropertyDialog::lockUnlockInterface(bool lock)
{
  lock = !lock;
/*
  ui.annotationTextEdit->setEnabled(lock);
  ui.pointsTableWidget->setEnabled(lock);
  ui.measurementLineEdit->setEnabled(lock);
  ui.textSelectedColorPickerButton->setEnabled(lock);
  ui.textUnselectedColorPickerButton->setEnabled(lock);
  ui.textScaleSliderSpinBoxWidget->setEnabled(lock);
  ui.textOpacitySliderSpinBoxWidget->setEnabled(lock);
  ui.visibleInvisibleButton->setEnabled(lock);
  ui.pointSelectedColorPickerButton->setEnabled(lock);
  ui.pointUnselectedColorPickerButton->setEnabled(lock);
  ui.pointGlyphTypeComboBox->setEnabled(lock);
  ui.lineSelectedColorPickerButton->setEnabled(lock);
  ui.lineUnselectedColorPickerButton->setEnabled(lock);
  ui.pointAmbientSliderSpinBoxWidget->setEnabled(lock);
  ui.pointDiffuseSliderSpinBoxWidget->setEnabled(lock);
  ui.pointOpacitySliderSpinBoxWidget->setEnabled(lock);
  ui.pointSizeSliderSpinBoxWidget->setEnabled(lock);
  ui.pointSpecularSliderSpinBoxWidget->setEnabled(lock);
  ui.lineAmbientSliderSpinBoxWidget_2->setEnabled(lock);
  ui.lineDiffuseSliderSpinBoxWidget_2->setEnabled(lock);
  ui.lineOpacitySliderSpinBoxWidget_2->setEnabled(lock);
  ui.lineSpecularSliderSpinBoxWidget_2->setEnabled(lock);
  ui.lineWidthSliderSpinBoxWidget_2->setEnabled(lock);
  ui.lineLabelPositionSliderSpinBoxWidget->setEnabled(lock);
  ui.lineLabelVisibilityCheckBox->setEnabled(lock);
  ui.lineTickSpacingLineEdit->setEnabled(lock);
  ui.lineMaxTicksSliderSpinBoxWidget->setEnabled(lock);
  */
}

