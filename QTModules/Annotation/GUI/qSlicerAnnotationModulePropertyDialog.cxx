#include "qSlicerAnnotationModulePropertyDialog.h"
#include "ui_qSlicerAnnotationModulePropertyDialog.h"

// QT includes
#include <QButtonGroup>
#include <QList>
#include <QFontMetrics>
#include <QDebug>
#include <QMessageBox>
#include <QColorDialog>

#include "Logic/vtkSlicerAnnotationModuleLogic.h"
#include "vtkMRMLAnnotationNode.h"
#include "vtkMRMLAnnotationPointDisplayNode.h"
#include "vtkMRMLAnnotationLineDisplayNode.h"
#include "vtkMRMLAnnotationTextDisplayNode.h"
#include "vtkMRMLAnnotationFiducialNode.h"
#include "vtkMRMLAnnotationAngleNode.h"
#include "vtkMRMLAnnotationRulerNode.h"
#include "vtkMRMLFiducialListNode.h"

qSlicerAnnotationModulePropertyDialog::~qSlicerAnnotationModulePropertyDialog()
{
  if (this->m_rulerCopy)
    {
    this->m_rulerCopy->Delete();
    this->m_rulerCopy = NULL;
    }
  if (this->m_angleCopy)
    {
    this->m_angleCopy->Delete();
    this->m_angleCopy = NULL;
    }
  if (this->m_textDispCopy)
    {
    this->m_textDispCopy->Delete();
    this->m_textDispCopy = NULL;
    }
  if (this->m_lineDispCopy)
    {
    this->m_lineDispCopy->Delete();
    this->m_lineDispCopy = NULL;
    }
  if (this->m_pointDispCopy)
    {
    this->m_pointDispCopy->Delete();
    this->m_pointDispCopy = NULL;
    }

  for (int i = 0; i < this->m_lineEditList.size(); ++i)
    {
    if (this->m_lineEditList[i] != NULL)
      {
      delete this->m_lineEditList[i];
      }
    }
  this->m_lineEditList.clear();

  if (m_gridLayout != NULL)
    {
    delete m_gridLayout;
    }

}
qSlicerAnnotationModulePropertyDialog::qSlicerAnnotationModulePropertyDialog(vtkMRMLNode * node,
                                                                                                 vtkSlicerAnnotationModuleLogic* logic)
{
  this->m_rulerCopy = NULL;
  this->m_angleCopy = NULL;
  this->m_textDispCopy = NULL;
  this->m_lineDispCopy = NULL;
  this->m_pointDispCopy = NULL;

  this->m_logic = logic;
  this->m_nodeId = node->GetID();
  this->m_isUpdated = false;

  ui.setupUi(
      this);

  Initialize(
      node);
  createConnection();

}

void qSlicerAnnotationModulePropertyDialog::Initialize(vtkMRMLNode * node)
{

  /*
  this->setWindowTitle(
      "Annotation Properties");
  this->SaveStateForUndo(
      node);

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
      node) != NULL)
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

void qSlicerAnnotationModulePropertyDialog::createConnection()
{
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

void qSlicerAnnotationModulePropertyDialog::onCoordinateChanged(QString text)
{
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
    positions.push_back(
        m_lineEditList[i]->text().toDouble());
    }

  // update widget
  vtkMRMLNode* node = this->m_logic->GetMRMLScene()->GetNodeByID(
      m_nodeId);
  int num = positions.size() / 3;
  double pos[3];
  for (int id = 0; id < num; ++id)
    {
    pos[0] = positions[id * 3];
    pos[1] = positions[id * 3 + 1];
    pos[2] = positions[id * 3 + 2];
    this->m_logic->SetAnnotationControlPointsCoordinate(
        node,
        pos,
        id);
    }

  // update value in the property dialog
  thevalue = this->m_logic->GetAnnotationMeasurement(
      node);
  format = this->m_logic->GetAnnotationTextFormatProperty(
      node);
  this->FormatValueToChar(
      format,
      thevalue,
      valueString);
  this->updateValue(
      valueString);

  emit coordinateChanged(
      valueString,
      m_nodeId);

}

void qSlicerAnnotationModulePropertyDialog::onTextChanged()
{
  /*
  QString text = ui.annotationTextEdit->toPlainText();
  ui.annotationTextEdit->moveCursor(
      QTextCursor::End,
      QTextCursor::MoveAnchor);
  emit textChanged(
      text,
      m_nodeId);
*/
}

void qSlicerAnnotationModulePropertyDialog::updateTextFromTable(QString text)
{
  // Text Properties
 // ui.annotationTextEdit->setText(
  //    text);
}

void qSlicerAnnotationModulePropertyDialog::updateValue(QString valueString)
{
  QString valueStr;
  valueStr.append(
      "<p>Value: <b>").append(
      valueString).append(
      "</b></p>");
 // ui.annotationValueBrowser->setHtml(
  //    valueStr);

}

void qSlicerAnnotationModulePropertyDialog::updateCoordinates(double* pos,
                                                                        int id)
{
  // turn off onCoordinateChanged
  this->m_isUpdated = true;

  for (int i = 0; i < 3; ++i)
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
    m_lineEditList[i + id * 3]->setText(
        posString);
    }

  // turn on onCoordinateChanged
  this->m_isUpdated = false;
}

void qSlicerAnnotationModulePropertyDialog::SaveLinesNode(vtkMRMLAnnotationLinesNode* node)
{
  if (!node)
    {
    return;
    }
  if (!this->m_lineDispCopy)
    {
    this->m_lineDispCopy = vtkMRMLAnnotationLineDisplayNode::New();
    }

  node->CreateAnnotationLineDisplayNode();
  this->m_lineDispCopy->Copy(
      node->GetAnnotationLineDisplayNode());
  this->SaveControlPoints(
      node);

}

void qSlicerAnnotationModulePropertyDialog::SaveControlPoints(vtkMRMLAnnotationControlPointsNode* node)
{
  if (!node)
    {
    return;
    }

  if (!this->m_pointDispCopy)
    {
    this->m_pointDispCopy = vtkMRMLAnnotationPointDisplayNode::New();
    }
  node->CreateAnnotationPointDisplayNode();
  this->m_pointDispCopy->Copy(
      node->GetAnnotationPointDisplayNode());
  this->SaveAnnotationNode(
      (vtkMRMLAnnotationNode*) node);
}

void qSlicerAnnotationModulePropertyDialog::SaveAnnotationNode(vtkMRMLAnnotationNode* node)
{
  if (!node)
    {
    return;
    }

  if (!this->m_textDispCopy)
    {
    this->m_textDispCopy = vtkMRMLAnnotationTextDisplayNode::New();
    }
  node->CreateAnnotationTextDisplayNode();
  this->m_textDispCopy->Copy(
      node->GetAnnotationTextDisplayNode());
}

void qSlicerAnnotationModulePropertyDialog::SaveStateForUndo(vtkMRMLNode* node)
{
  if (node->IsA(
      "vtkMRMLAnnotationAngleNode"))
    {
    vtkMRMLAnnotationAngleNode* mynode = vtkMRMLAnnotationAngleNode::SafeDownCast(
        node);
    if (!this->m_angleCopy)
      {
      this->m_angleCopy = vtkMRMLAnnotationAngleNode::New();
      }
    this->m_angleCopy->Copy(
        mynode);
    this->SaveLinesNode(
        mynode);
    }
  else if (node->IsA(
      "vtkMRMLAnnotationRulerNode"))
    {
    vtkMRMLAnnotationRulerNode* mynode = vtkMRMLAnnotationRulerNode::SafeDownCast(
        node);
    if (!this->m_rulerCopy)
      {
      this->m_rulerCopy = vtkMRMLAnnotationRulerNode::New();
      }
    this->m_rulerCopy->Copy(
        mynode);
    this->SaveLinesNode(
        mynode);
    }
  else if (node->IsA(
      "vtkMRMLAnnotationFiducialNode"))
    {
    vtkMRMLAnnotationFiducialNode* fiducialNode = vtkMRMLAnnotationFiducialNode::SafeDownCast(
        node);
    fiducialNode->CreateAnnotationTextDisplayNode();
    fiducialNode->CreateAnnotationPointDisplayNode();
    fiducialNode->GetScene()->SaveStateForUndo(
        fiducialNode);
    fiducialNode->GetAnnotationTextDisplayNode()->GetScene()->SaveStateForUndo(
        fiducialNode->GetAnnotationTextDisplayNode());
    fiducialNode->GetAnnotationPointDisplayNode()->GetScene()->SaveStateForUndo(
        fiducialNode->GetAnnotationPointDisplayNode());
    }

}

void qSlicerAnnotationModulePropertyDialog::UndoLinesNode(vtkMRMLAnnotationLinesNode* node)
{
  if (!node)
    {
    return;
    }
  node->CreateAnnotationLineDisplayNode();
  node->GetAnnotationLineDisplayNode()->Copy(
      m_lineDispCopy);
  this->UndoControlPoints(
      node);
}

void qSlicerAnnotationModulePropertyDialog::UndoControlPoints(vtkMRMLAnnotationControlPointsNode* node)
{
  if (!node)
    {
    return;
    }
  node->CreateAnnotationPointDisplayNode();
  node->GetAnnotationPointDisplayNode()->Copy(
      m_pointDispCopy);
  this->UndoAnnotationNode(
      (vtkMRMLAnnotationNode*) node);
}

void qSlicerAnnotationModulePropertyDialog::UndoAnnotationNode(vtkMRMLAnnotationNode* node)
{
  if (!node)
    {
    return;
    }
  node->CreateAnnotationTextDisplayNode();
  node->GetAnnotationTextDisplayNode()->Copy(
      m_textDispCopy);
}

void qSlicerAnnotationModulePropertyDialog::Undo(vtkMRMLNode* node)
{
  if (node->IsA(
      "vtkMRMLAnnotationAngleNode"))
    {
    vtkMRMLAnnotationAngleNode* anode = vtkMRMLAnnotationAngleNode::SafeDownCast(
        node);
    anode->Copy(
        m_angleCopy);
    this->UndoLinesNode(
        anode);
    }
  else if (node->IsA(
      "vtkMRMLAnnotationRulerNode"))
    {
    vtkMRMLAnnotationRulerNode* rnode = vtkMRMLAnnotationRulerNode::SafeDownCast(
        node);
    rnode->Copy(
        m_rulerCopy);
    this->UndoLinesNode(
        rnode);
    }
  else if (node->IsA(
      "vtkMRMLAnnotationFiducialNode"))
    {
    //ToDo
    }

}

void qSlicerAnnotationModulePropertyDialog::onDialogRejected()
{
  vtkMRMLNode* node = this->m_logic->GetMRMLScene()->GetNodeByID(
      m_nodeId);
  this->Undo(
      node);
  emit dialogRejected(
      m_nodeId);
}

void qSlicerAnnotationModulePropertyDialog::onDialogAccepted()
{
 // QString text = ui.annotationTextEdit->toPlainText();
//  emit dialogAccepted(
 //     m_nodeId,
  //    text);
}

void qSlicerAnnotationModulePropertyDialog::SetButtonText(int type)
{
  switch (type)
    {
    case vtkSlicerAnnotationModuleLogic::TEXT_COLOR:
      this->ui.textUnselectedColorPickerButton->setText(
          "Selected Text Color");
      return;
    case vtkSlicerAnnotationModuleLogic::TEXT_SELECTED_COLOR:
      this->ui.textSelectedColorPickerButton->setText(
          "Text Color");
      return;
    case vtkSlicerAnnotationModuleLogic::POINT_COLOR:
      this->ui.pointUnselectedColorPickerButton->setText(
          "Selected Point Color");
      return;
    case vtkSlicerAnnotationModuleLogic::POINT_SELECTED_COLOR:
      this->ui.pointSelectedColorPickerButton->setText(
          "Point Color");
      return;
    case vtkSlicerAnnotationModuleLogic::LINE_COLOR:
      this->ui.lineUnselectedColorPickerButton->setText(
          "Selected Line Color");
      return;
    case vtkSlicerAnnotationModuleLogic::LINE_SELECTED_COLOR:
      this->ui.lineSelectedColorPickerButton->setText(
          "Line Color");
      return;
    default:
      std::cout << "SetButtonText type invalid" << std::endl;
      return;
    }
}

void qSlicerAnnotationModulePropertyDialog::onTextColorChanged(QColor qcolor)
{
  double color[3];
  this->TurnQColorToColorArray(
      color,
      qcolor);
  this->m_logic->ModifyPropertiesAndWidget(
      this->m_logic->GetMRMLScene()->GetNodeByID(
          m_nodeId),
      this->m_logic->TEXT_COLOR,
      color);
  this->SetButtonText(
      this->m_logic->TEXT_COLOR);
}

void qSlicerAnnotationModulePropertyDialog::onTextSelectedColorChanged(QColor qcolor)
{
  double color[3];
  this->TurnQColorToColorArray(
      color,
      qcolor);
  this->m_logic->ModifyPropertiesAndWidget(
      this->m_logic->GetMRMLScene()->GetNodeByID(
          m_nodeId),
      this->m_logic->TEXT_SELECTED_COLOR,
      color);
  this->SetButtonText(
      this->m_logic->TEXT_SELECTED_COLOR);
}

void qSlicerAnnotationModulePropertyDialog::onTextScaleChanged(double value)
{
  this->m_logic->ModifyPropertiesAndWidget(
      this->m_logic->GetMRMLScene()->GetNodeByID(
          m_nodeId),
      this->m_logic->TEXT_SCALE,
      &value);
}

void qSlicerAnnotationModulePropertyDialog::onPointColorChanged(QColor qcolor)
{
  double color[3];
  this->TurnQColorToColorArray(
      color,
      qcolor);
  this->m_logic->ModifyPropertiesAndWidget(
      this->m_logic->GetMRMLScene()->GetNodeByID(
          m_nodeId),
      this->m_logic->POINT_COLOR,
      color);
  this->SetButtonText(
      this->m_logic->POINT_COLOR);
}

void qSlicerAnnotationModulePropertyDialog::onPointSelectedColorChanged(QColor qcolor)
{
  double color[3];
  this->TurnQColorToColorArray(
      color,
      qcolor);
  this->m_logic->ModifyPropertiesAndWidget(
      this->m_logic->GetMRMLScene()->GetNodeByID(
          m_nodeId),
      this->m_logic->POINT_SELECTED_COLOR,
      color);
  this->SetButtonText(
      this->m_logic->POINT_SELECTED_COLOR);
}

void qSlicerAnnotationModulePropertyDialog::onPointSizeChanged(double value)
{
  this->m_logic->ModifyPropertiesAndWidget(
      this->m_logic->GetMRMLScene()->GetNodeByID(
          m_nodeId),
      this->m_logic->POINT_SIZE,
      &value);
}

void qSlicerAnnotationModulePropertyDialog::onPointOpacityChanged(double value)
{
  this->m_logic->ModifyPropertiesAndWidget(
      this->m_logic->GetMRMLScene()->GetNodeByID(
          m_nodeId),
      this->m_logic->POINT_OPACITY,
      &value);
}

void qSlicerAnnotationModulePropertyDialog::onPointAmbientChanged(double value)
{
//  this->m_logic->ModifyPropertiesAndWidget(
 //     this->m_logic->GetMRMLScene()->GetNodeByID(
  //        m_nodeId),
   //   this->m_logic->POINT_AMBIENT,
    //  &value);
}
void qSlicerAnnotationModulePropertyDialog::onPointDiffuseChanged(double value)
{
  this->m_logic->ModifyPropertiesAndWidget(
      this->m_logic->GetMRMLScene()->GetNodeByID(
          m_nodeId),
      this->m_logic->POINT_DIFFUSE,
      &value);
}

void qSlicerAnnotationModulePropertyDialog::onPointSpecularChanged(double value)
{
  this->m_logic->ModifyPropertiesAndWidget(
      this->m_logic->GetMRMLScene()->GetNodeByID(
          m_nodeId),
      this->m_logic->POINT_SPECULAR,
      &value);
}

void qSlicerAnnotationModulePropertyDialog::onLineColorChanged(QColor qcolor)
{
  double color[3];
  this->TurnQColorToColorArray(
      color,
      qcolor);
  this->m_logic->ModifyPropertiesAndWidget(
      this->m_logic->GetMRMLScene()->GetNodeByID(
          m_nodeId),
      this->m_logic->LINE_COLOR,
      color);
  this->SetButtonText(
      this->m_logic->LINE_COLOR);
}

void qSlicerAnnotationModulePropertyDialog::onLineSelectedColorChanged(QColor qcolor)
{
  double color[3];
  this->TurnQColorToColorArray(
      color,
      qcolor);
  this->m_logic->ModifyPropertiesAndWidget(
      this->m_logic->GetMRMLScene()->GetNodeByID(
          m_nodeId),
      this->m_logic->LINE_SELECTED_COLOR,
      color);
  this->SetButtonText(
      this->m_logic->LINE_SELECTED_COLOR);
}

void qSlicerAnnotationModulePropertyDialog::onLineWidthChanged(double value)
{
  this->m_logic->ModifyPropertiesAndWidget(
      this->m_logic->GetMRMLScene()->GetNodeByID(
          m_nodeId),
      this->m_logic->LINE_WIDTH,
      &value);
}

void qSlicerAnnotationModulePropertyDialog::onLineOpacityChanged(double value)
{
  this->m_logic->ModifyPropertiesAndWidget(
      this->m_logic->GetMRMLScene()->GetNodeByID(
          m_nodeId),
      this->m_logic->LINE_OPACITY,
      &value);
}

void qSlicerAnnotationModulePropertyDialog::onLineAmbientChanged(double value)
{
  this->m_logic->ModifyPropertiesAndWidget(
      this->m_logic->GetMRMLScene()->GetNodeByID(
          m_nodeId),
      this->m_logic->LINE_AMBIENT,
      &value);
}

void qSlicerAnnotationModulePropertyDialog::onLineDiffuseChanged(double value)
{
  this->m_logic->ModifyPropertiesAndWidget(
      this->m_logic->GetMRMLScene()->GetNodeByID(
          m_nodeId),
      this->m_logic->LINE_DIFFUSE,
      &value);
}

void qSlicerAnnotationModulePropertyDialog::onLineSpecularChanged(double value)
{
  this->m_logic->ModifyPropertiesAndWidget(
      this->m_logic->GetMRMLScene()->GetNodeByID(
          m_nodeId),
      this->m_logic->LINE_SPECULAR,
      &value);
}

void qSlicerAnnotationModulePropertyDialog::UpdateLockUnlockStatus(bool isLock)
{
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
void qSlicerAnnotationModulePropertyDialog::TurnColorArrayToQColor(double* color,
                                                                             QColor &qcolor)
{
  qcolor.setRed(
      color[0] * 255.0001);
  qcolor.setGreen(
      color[1] * 255.0001);
  qcolor.setBlue(
      color[2] * 255.0001);
}

void qSlicerAnnotationModulePropertyDialog::FormatValueToChar(const char* format,
                                                                        std::vector<double> vv,
                                                                        QString &valueString)
{
  char valuechar[100];
  QString tempString;
  valueString = "";
  foreach(double v, vv)
      {
      sprintf(
          valuechar,
          format,
          v);
      tempString = valuechar;
      tempString.append(
          " ");
      valueString.append(
          tempString);
      }

  //valueString = valuechar;
}

void qSlicerAnnotationModulePropertyDialog::TurnQColorToColorArray(double* color,
                                                                             QColor &qcolor)
{
  color[0] = qcolor.red() / 255.0;
  color[1] = qcolor.green() / 255.0;
  color[2] = qcolor.blue() / 255.0;
}

void qSlicerAnnotationModulePropertyDialog::onCollapsibleGroupBoxClicked()
{
  vtkMRMLNode * node = this->m_logic->GetMRMLScene()->GetNodeByID(
      this->m_nodeId);
  if (node->IsA(
      "vtkMRMLAnnotationStickyNode") || node->IsA(
      "vtkMRMLAnnotationFiducialNode") || node->IsA(
      "vtkMRMLAnnotationTextNode"))
    {
/*    ui.coordinatesLabel->setVisible(
        false);
    ui.annotationValueBrowser->setVisible(
        false);
    ui.displayPropertiesCTKCollapsibleGroupBox->setEnabled(
        false);
    ui.displayPropertiesCTKCollapsibleGroupBox->setVisible(
        false);*/
    }
}
