#include "qSlicermiAnnotationModuleAnnotationPropertyDialog.h"
#include "ui_qSlicermiAnnotationModuleAnnotationPropertyDialog.h"

// QT includes
#include <QButtonGroup>
#include <QList>
#include <QFontMetrics>
#include <QDebug>
#include <QMessageBox>
#include <QColorDialog>

#include "vtkSlicermiAnnotationModuleLogic.h"
#include "vtkMRMLAnnotationNode.h"
#include "vtkMRMLAnnotationPointDisplayNode.h"
#include "vtkMRMLAnnotationLineDisplayNode.h"
#include "vtkMRMLAnnotationTextDisplayNode.h"
#include "vtkMRMLAnnotationFiducialNode.h"
#include "vtkMRMLAnnotationAngleNode.h"
#include "vtkMRMLAnnotationRulerNode.h"


qSlicermiAnnotationModuleAnnotationPropertyDialog::~qSlicermiAnnotationModuleAnnotationPropertyDialog()
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

    for (int i=0; i<this->m_lineEditList.size(); ++i)
    {
        if ( this->m_lineEditList[i] != NULL )
        {
            delete this->m_lineEditList[i];
        }
    }
    this->m_lineEditList.clear();

    if ( m_gridLayout != NULL )
    {
        delete m_gridLayout;
    }



}
qSlicermiAnnotationModuleAnnotationPropertyDialog::qSlicermiAnnotationModuleAnnotationPropertyDialog(vtkMRMLNode * node, vtkSlicermiAnnotationModuleLogic* logic)
{
    this->m_rulerCopy = NULL;
    this->m_angleCopy = NULL;
    this->m_textDispCopy = NULL;
    this->m_lineDispCopy = NULL;
    this->m_pointDispCopy = NULL;

    this->m_logic = logic;
    this->m_nodeId = node->GetID();
    this->m_isUpdated = false;
    
    ui.setupUi(this);

    Initialize(node);
    createConnection();



}

void qSlicermiAnnotationModuleAnnotationPropertyDialog::Initialize(vtkMRMLNode * node)
{
    this->setWindowTitle("Annotation Properties");
    this->SaveStateForUndo(node);

    // COORDINATES
    QDoubleValidator *doubleVal = new QDoubleValidator(this);

    for (int i=0; i<m_logic->GetNumberOfControlPoints(vtkMRMLAnnotationControlPointsNode::SafeDownCast(node))*3; ++i)
    {
        m_lineEditList.push_back( new QLineEdit(ui.CTKCollapsibleGroupBox_4));
        m_lineEditList[i]->setValidator( doubleVal );
    }
    
    m_gridLayout = new QGridLayout( ui.CTKCollapsibleGroupBox_4 );
    m_gridLayout->addWidget( ui.annotationTypeBrowser,  0, 0, 2, 3);
    m_gridLayout->addWidget( ui.textLabel,                2, 0, 1, 1);
    m_gridLayout->addWidget( ui.annotationTextEdit,        2, 1, 2, 2);
    m_gridLayout->addWidget( ui.annotationValueBrowser, 4, 0, 2, 3);
    m_gridLayout->addWidget( ui.coordinatesLabel,        6, 0, 1, 3);

    double* pos;
    for (int id=0; id<m_logic->GetNumberOfControlPoints(node); ++id)
    {
        pos = m_logic->GetAnnotationControlPointsCoordinate(node, id);

        for (int i=0; i<3; i++) 
        {
            QString posString("");
            posString.append(QString("%1").arg( QString::number( double(pos[i]), 'f', 2 )));
            this->m_lineEditList[i+id*3]->setText( posString );
            this->connect(this->m_lineEditList[i+id*3], SIGNAL(textChanged(QString)), this, SLOT(onCoordinateChanged(QString)) );
            this->m_gridLayout->addWidget( m_lineEditList[i+id*3], 7+ id, i%3, 1, 1);
        }
    }

    // Lock/Unlock properties
    if (  vtkMRMLAnnotationNode::SafeDownCast(node)->GetLocked() == 1 )
    {
        ui.annotationTextEdit->setEnabled(false);
        for (int i=0; i<m_logic->GetNumberOfControlPoints(vtkMRMLAnnotationControlPointsNode::SafeDownCast(node))*3; ++i)
        {
            m_lineEditList[i]->setEnabled(false);
        }
        ui.textTab->setEnabled(false);
        ui.pointTab->setEnabled(false);
        ui.lineTab->setEnabled(false);
    } 
    else
    {
        ui.annotationTextEdit->setEnabled(true);
        for (int i=0; i<m_logic->GetNumberOfControlPoints(vtkMRMLAnnotationControlPointsNode::SafeDownCast(node))*3; ++i)
        {
            m_lineEditList[i]->setEnabled(true);
        }
        ui.textTab->setEnabled(true);
        ui.pointTab->setEnabled(true);
        ui.lineTab->setEnabled(true);
    }

    // Type
    QString typeString;
    typeString.append("<p>Annotation Type: ")
        .append("<img src='")
        .append( m_logic->GetIconName(node, false) )
        .append("'>")
        .append("</p>");
    ui.annotationTypeBrowser->setHtml(typeString);

    // Text Properties
    QString textString = QString( m_logic->GetAnnotationTextProperty(node) );
    ui.annotationTextEdit->setText(textString);

    // Text Format
    const char* textFormat = m_logic->GetAnnotationTextFormatProperty(node);

    // Value
    double value = m_logic->GetAnnotationMeasurement(node);
    char valuechar[100];
    QString valueString;
    sprintf( valuechar, textFormat, value);
    valueString.append("<p>Value: <b>").append(QString(valuechar)).append("</b></p>");
    ui.annotationValueBrowser->setHtml( valueString );


    // Default CollapsibleGroupBox Properties
    QVBoxLayout* groupBoxLayout = new QVBoxLayout;
    ui.displayPropertiesCTKCollapsibleGroupBox->setLayout(groupBoxLayout);
    ui.displayPropertiesCTKCollapsibleGroupBox->setEnabled(true);
    ui.displayPropertiesCTKCollapsibleGroupBox->setChecked(false);

    if ( node->IsA("vtkMRMLAnnotationStickyNode") || node->IsA("vtkMRMLAnnotationFiducialNode") )
    {
        ui.coordinatesLabel->setVisible(false);
        ui.annotationValueBrowser->setVisible(false);
        ui.displayPropertiesCTKCollapsibleGroupBox->setEnabled(false);
        ui.displayPropertiesCTKCollapsibleGroupBox->setVisible(false);
        return;
    }

    // Text Display Properties
    double* color;
    QColor qcolor;
    color = m_logic->GetAnnotationLinesPropertiesColor(node, m_logic->TEXT_COLOR);
    this->TurnColorArrayToQColor(color, qcolor);
    ui.textUnselectedColorPickerButton->setColor( qcolor );
    //ui.textUnselectedColorPickerButton->setColor(m_logic->getColor(node, m_logic->TEXT_COLOR));
    color = m_logic->GetAnnotationLinesPropertiesColor(node, m_logic->TEXT_SELECTED_COLOR);
    this->TurnColorArrayToQColor(color, qcolor);
    ui.textSelectedColorPickerButton->setColor( qcolor );
    //ui.textSelectedColorPickerButton->setColor(m_logic->getColor(node, m_logic->TEXT_SELECTED_COLOR));
    ui.textUnselectedColorPickerButton->setText("Selected Text Color");
    ui.textSelectedColorPickerButton->setText("Text Color");
    m_logic->GetAnnotationLinesPropertiesDouble(node, m_logic->TEXT_SCALE, value);
    ui.textScaleSliderSpinBoxWidget->setValue(value);

    // Point Display Properties
    color = m_logic->GetAnnotationLinesPropertiesColor(node, m_logic->POINT_COLOR);
    this->TurnColorArrayToQColor(color, qcolor);
    ui.pointUnselectedColorPickerButton->setColor( qcolor );
    color = m_logic->GetAnnotationLinesPropertiesColor(node, m_logic->POINT_SELECTED_COLOR);
    this->TurnColorArrayToQColor(color, qcolor);
    ui.pointSelectedColorPickerButton->setColor(qcolor);
    ui.pointUnselectedColorPickerButton->setText("Selected Point Color");
    ui.pointSelectedColorPickerButton->setText("Point Color");
    //ui.pointSizeSliderSpinBoxWidget->setValue( m_logic->getPointSize(node) ); // ToDo
    m_logic->GetAnnotationLinesPropertiesDouble(node, m_logic->POINT_OPACITY, value);
    ui.pointOpacitySliderSpinBoxWidget->setValue( value );
    m_logic->GetAnnotationLinesPropertiesDouble(node, m_logic->POINT_AMBIENT, value);
    ui.pointAmbientSliderSpinBoxWidget->setValue( value );
    m_logic->GetAnnotationLinesPropertiesDouble(node, m_logic->POINT_DIFFUSE, value);
    ui.pointDiffuseSliderSpinBoxWidget->setValue( value );
    m_logic->GetAnnotationLinesPropertiesDouble(node, m_logic->POINT_SPECULAR, value);
    ui.pointSpecularSliderSpinBoxWidget->setValue( value );

    // Line Display Properties
    color = m_logic->GetAnnotationLinesPropertiesColor(node, m_logic->LINE_COLOR);
    this->TurnColorArrayToQColor(color, qcolor);
    ui.lineUnselectedColorPickerButton->setColor(qcolor);
    color = m_logic->GetAnnotationLinesPropertiesColor(node, m_logic->LINE_SELECTED_COLOR);
    this->TurnColorArrayToQColor(color, qcolor);
    ui.lineSelectedColorPickerButton->setColor(qcolor);
    ui.lineUnselectedColorPickerButton->setText("Selected Line Color");
    ui.lineSelectedColorPickerButton->setText("Line Color");
    m_logic->GetAnnotationLinesPropertiesDouble(node, m_logic->LINE_WIDTH, value);
    ui.lineWidthSliderSpinBoxWidget->setValue( value );
    m_logic->GetAnnotationLinesPropertiesDouble(node, m_logic->LINE_OPACITY, value);
    ui.lineOpacitySliderSpinBoxWidget->setValue( value );
    m_logic->GetAnnotationLinesPropertiesDouble(node, m_logic->LINE_AMBIENT, value);
    ui.lineAmbientSliderSpinBoxWidget->setValue( value );
    m_logic->GetAnnotationLinesPropertiesDouble(node, m_logic->LINE_DIFFUSE, value);
    ui.lineDiffuseSliderSpinBoxWidget->setValue( value );
    m_logic->GetAnnotationLinesPropertiesDouble(node, m_logic->LINE_SPECULAR, value);
    ui.lineSpecularSliderSpinBoxWidget->setValue( value );


}

void qSlicermiAnnotationModuleAnnotationPropertyDialog::createConnection()
{
    this->connect(ui.annotationTextEdit, SIGNAL(textChanged()), this, SLOT(onTextChanged()) );
    this->connect(this, SIGNAL(rejected()), this, SLOT(onDialogRejected()) );
    this->connect(this, SIGNAL(accepted()), this, SLOT(onDialogAccepted()) );

    this->connect(ui.textUnselectedColorPickerButton, SIGNAL(colorChanged(QColor)), this, SLOT(onTextColorChanged(QColor)));
    this->connect(ui.textSelectedColorPickerButton, SIGNAL(colorChanged(QColor)), this, SLOT(onTextSelectedColorChanged(QColor)));
    this->connect(ui.textScaleSliderSpinBoxWidget, SIGNAL(valueChanged(double)), this, SLOT(onTextScaleChanged(double)) );

    this->connect(ui.pointUnselectedColorPickerButton, SIGNAL(colorChanged(QColor)), this, SLOT(onPointColorChanged(QColor)));
    this->connect(ui.pointSelectedColorPickerButton, SIGNAL(colorChanged(QColor)), this, SLOT(onPointSelectedColorChanged(QColor)));
    this->connect(ui.pointSizeSliderSpinBoxWidget, SIGNAL(valueChanged(double)), this, SLOT(onPointSizeChanged(double)) );
    this->connect(ui.pointOpacitySliderSpinBoxWidget, SIGNAL(valueChanged(double)), this, SLOT(onPointOpacityChanged(double)) );
    this->connect(ui.pointAmbientSliderSpinBoxWidget, SIGNAL(valueChanged(double)), this, SLOT(onPointAmbientChanged(double)) );
    this->connect(ui.pointDiffuseSliderSpinBoxWidget, SIGNAL(valueChanged(double)), this, SLOT(onPointDiffuseChanged(double)) );
    this->connect(ui.pointSpecularSliderSpinBoxWidget, SIGNAL(valueChanged(double)), this, SLOT(onPointSpecularChanged(double)) );

    this->connect(ui.lineUnselectedColorPickerButton, SIGNAL(colorChanged(QColor)), this, SLOT(onLineColorChanged(QColor)));
    this->connect(ui.lineSelectedColorPickerButton, SIGNAL(colorChanged(QColor)), this, SLOT(onLineSelectedColorChanged(QColor)));
    this->connect(ui.lineWidthSliderSpinBoxWidget, SIGNAL(valueChanged(double)), this, SLOT(onLineWidthChanged(double)) );
    this->connect(ui.lineOpacitySliderSpinBoxWidget, SIGNAL(valueChanged(double)), this, SLOT(onLineOpacityChanged(double)) );
    this->connect(ui.lineAmbientSliderSpinBoxWidget, SIGNAL(valueChanged(double)), this, SLOT(onLineAmbientChanged(double)));
    this->connect(ui.lineDiffuseSliderSpinBoxWidget, SIGNAL(valueChanged(double)), this, SLOT(onLineDiffuseChanged(double)));
    this->connect(ui.lineSpecularSliderSpinBoxWidget, SIGNAL(valueChanged(double)), this, SLOT(onLineSpecularChanged(double)));

  this->connect(ui.CTKCollapsibleGroupBox_4, SIGNAL(clicked()), this, SLOT(onCollapsibleGroupBoxClicked()));


}

void qSlicermiAnnotationModuleAnnotationPropertyDialog::onCoordinateChanged(QString text)
{
    if ( this->m_isUpdated )
    {
        return;
    }
    
    std::vector<double> positions;
    QString valueString;
    double thevalue;
    const char* format;

    for (int i=0; i<m_lineEditList.size(); ++i)
    {
        positions.push_back( m_lineEditList[i]->text().toDouble() );
    }

    // update widget
    vtkMRMLNode* node = this->m_logic->GetMRMLScene()->GetNodeByID( m_nodeId );
    int num = positions.size()/3;
    double pos[3];
    for (int id=0; id<num; ++id)
    {
        pos[0] = positions[id*3];
        pos[1] = positions[id*3+1];
        pos[2] = positions[id*3+2];
        this->m_logic->SetAnnotationControlPointsCoordinate(node, pos, id);
    }

    // update value in the property dialog
    thevalue = this->m_logic->GetAnnotationMeasurement(node);
    format = this->m_logic->GetAnnotationTextFormatProperty(node);
    this->FormatValueToChar(format, thevalue, valueString);
    this->updateValue(valueString);

    emit coordinateChanged( valueString, m_nodeId );

}

void qSlicermiAnnotationModuleAnnotationPropertyDialog::onTextChanged()
{
    QString text = ui.annotationTextEdit->toPlainText();
    ui.annotationTextEdit->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
    emit textChanged(text, m_nodeId);

}

void qSlicermiAnnotationModuleAnnotationPropertyDialog::updateTextFromTable(QString text)
{
    // Text Properties
    ui.annotationTextEdit->setText(text);
}

void qSlicermiAnnotationModuleAnnotationPropertyDialog::updateValue(QString valueString)
{
    QString valueStr;
    valueStr.append("<p>Value: <b>").append(valueString).append("</b></p>");
    ui.annotationValueBrowser->setHtml( valueStr );

}

void qSlicermiAnnotationModuleAnnotationPropertyDialog::updateCoordinates(double* pos, int id)
{
    // turn off onCoordinateChanged
    this->m_isUpdated = true;

    for (int i=0; i<3; ++i)
    {
        QString posString("");
        posString.append(QString("%1").arg( QString::number( double(pos[i]), 'f', 2 )));
        m_lineEditList[i+id*3]->setText( posString );
    }

    // turn on onCoordinateChanged
    this->m_isUpdated = false;
}

void qSlicermiAnnotationModuleAnnotationPropertyDialog::SaveLinesNode(vtkMRMLAnnotationLinesNode* node)
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
    this->m_lineDispCopy->Copy(node->GetAnnotationLineDisplayNode());
    this->SaveControlPoints(node);

}

void qSlicermiAnnotationModuleAnnotationPropertyDialog::SaveControlPoints(vtkMRMLAnnotationControlPointsNode* node)
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
    this->m_pointDispCopy->Copy(node->GetAnnotationPointDisplayNode());
    this->SaveAnnotationNode( (vtkMRMLAnnotationNode*) node);
}

void qSlicermiAnnotationModuleAnnotationPropertyDialog::SaveAnnotationNode(vtkMRMLAnnotationNode* node)
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
    this->m_textDispCopy->Copy(node->GetAnnotationTextDisplayNode());
}

void qSlicermiAnnotationModuleAnnotationPropertyDialog::SaveStateForUndo(vtkMRMLNode* node)
{
    if( node->IsA( "vtkMRMLAnnotationAngleNode" ) ) 
    {
        vtkMRMLAnnotationAngleNode* mynode = vtkMRMLAnnotationAngleNode::SafeDownCast(node);
        if (!this->m_angleCopy)
          {
            this->m_angleCopy = vtkMRMLAnnotationAngleNode::New();
          }
        this->m_angleCopy->Copy(mynode);
        this->SaveLinesNode(mynode);
    }
    else if (node->IsA( "vtkMRMLAnnotationRulerNode" ))
    {
        vtkMRMLAnnotationRulerNode* mynode = vtkMRMLAnnotationRulerNode::SafeDownCast(node);
        if (!this->m_rulerCopy)
          {
            this->m_rulerCopy = vtkMRMLAnnotationRulerNode::New();
          }
        this->m_rulerCopy->Copy(mynode);
        this->SaveLinesNode(mynode);
    }
    else if (node->IsA( "vtkMRMLAnnotationFiducialNode" ))
    {
        vtkMRMLAnnotationFiducialNode* fiducialNode = vtkMRMLAnnotationFiducialNode::SafeDownCast(node);
        fiducialNode->CreateAnnotationTextDisplayNode();
        fiducialNode->CreateAnnotationPointDisplayNode();
        fiducialNode->GetScene()->SaveStateForUndo(fiducialNode);
        fiducialNode->GetAnnotationTextDisplayNode()->GetScene()->SaveStateForUndo(fiducialNode->GetAnnotationTextDisplayNode());
        fiducialNode->GetAnnotationPointDisplayNode()->GetScene()->SaveStateForUndo(fiducialNode->GetAnnotationPointDisplayNode());
    }

}

void qSlicermiAnnotationModuleAnnotationPropertyDialog::UndoLinesNode(vtkMRMLAnnotationLinesNode* node)
{
    if (!node)
    {
        return;
    }
    node->CreateAnnotationLineDisplayNode();
    node->GetAnnotationLineDisplayNode()->Copy(m_lineDispCopy);
    this->UndoControlPoints(node);
}

void qSlicermiAnnotationModuleAnnotationPropertyDialog::UndoControlPoints(vtkMRMLAnnotationControlPointsNode* node)
{
    if (!node)
    {
        return;
    }
    node->CreateAnnotationPointDisplayNode();
    node->GetAnnotationPointDisplayNode()->Copy(m_pointDispCopy);
    this->UndoAnnotationNode( (vtkMRMLAnnotationNode*) node);
}

void qSlicermiAnnotationModuleAnnotationPropertyDialog::UndoAnnotationNode(vtkMRMLAnnotationNode* node)
{
    if (!node)
    {
        return;
    }
    node->CreateAnnotationTextDisplayNode();
    node->GetAnnotationTextDisplayNode()->Copy(m_textDispCopy);
}

void qSlicermiAnnotationModuleAnnotationPropertyDialog::Undo(vtkMRMLNode* node)
{
    if( node->IsA( "vtkMRMLAnnotationAngleNode" ) ) 
    {
        vtkMRMLAnnotationAngleNode* anode = vtkMRMLAnnotationAngleNode::SafeDownCast(node);
        anode->Copy(m_angleCopy);
        this->UndoLinesNode(anode);
    }
    else if (node->IsA( "vtkMRMLAnnotationRulerNode" ))
    {
        vtkMRMLAnnotationRulerNode* rnode = vtkMRMLAnnotationRulerNode::SafeDownCast(node);
        rnode->Copy(m_rulerCopy);
        this->UndoLinesNode(rnode);
    }
    else if (node->IsA( "vtkMRMLAnnotationFiducialNode" ))
    {
        //ToDo
    }

}

void qSlicermiAnnotationModuleAnnotationPropertyDialog::onDialogRejected()
{
    vtkMRMLNode* node = this->m_logic->GetMRMLScene()->GetNodeByID(m_nodeId);
    this->Undo(node);
    emit dialogRejected( m_nodeId );
}

void qSlicermiAnnotationModuleAnnotationPropertyDialog::onDialogAccepted()
{
    QString text = ui.annotationTextEdit->toPlainText();
    emit dialogAccepted( m_nodeId, text );
}

void qSlicermiAnnotationModuleAnnotationPropertyDialog::SetButtonText(int type)
{
    switch(type)
    {
    case vtkSlicermiAnnotationModuleLogic::TEXT_COLOR:
        this->ui.textUnselectedColorPickerButton->setText("Selected Text Color");
        return;
    case vtkSlicermiAnnotationModuleLogic::TEXT_SELECTED_COLOR:
        this->ui.textSelectedColorPickerButton->setText("Text Color");
        return;
    case vtkSlicermiAnnotationModuleLogic::POINT_COLOR:
        this->ui.pointUnselectedColorPickerButton->setText("Selected Point Color");
        return;
    case vtkSlicermiAnnotationModuleLogic::POINT_SELECTED_COLOR:
        this->ui.pointSelectedColorPickerButton->setText("Point Color");
        return;
    case vtkSlicermiAnnotationModuleLogic::LINE_COLOR:
        this->ui.lineUnselectedColorPickerButton->setText("Selected Line Color");
        return;
    case vtkSlicermiAnnotationModuleLogic::LINE_SELECTED_COLOR:
        this->ui.lineSelectedColorPickerButton->setText("Line Color");
        return;
    default:
        std::cout << "SetButtonText type invalid" << std::endl;
        return;
    }
}


void qSlicermiAnnotationModuleAnnotationPropertyDialog::onTextColorChanged(QColor qcolor)
{
    double color[3];
    this->TurnQColorToColorArray(color, qcolor);
    this->m_logic->ModifyPropertiesAndWidget(this->m_logic->GetMRMLScene()->GetNodeByID(m_nodeId), this->m_logic->TEXT_COLOR, color);
    this->SetButtonText(this->m_logic->TEXT_COLOR);
}

void qSlicermiAnnotationModuleAnnotationPropertyDialog::onTextSelectedColorChanged(QColor qcolor)
{
    double color[3];
    this->TurnQColorToColorArray(color, qcolor);
    this->m_logic->ModifyPropertiesAndWidget(this->m_logic->GetMRMLScene()->GetNodeByID(m_nodeId), this->m_logic->TEXT_SELECTED_COLOR, color);
    this->SetButtonText(this->m_logic->TEXT_SELECTED_COLOR);
}

void qSlicermiAnnotationModuleAnnotationPropertyDialog::onTextScaleChanged(double value)
{
    this->m_logic->ModifyPropertiesAndWidget(this->m_logic->GetMRMLScene()->GetNodeByID(m_nodeId), this->m_logic->TEXT_SCALE, &value);
}

void qSlicermiAnnotationModuleAnnotationPropertyDialog::onPointColorChanged(QColor qcolor)
{
    double color[3];
    this->TurnQColorToColorArray(color, qcolor);
    this->m_logic->ModifyPropertiesAndWidget(this->m_logic->GetMRMLScene()->GetNodeByID(m_nodeId), this->m_logic->POINT_COLOR, color);
    this->SetButtonText(this->m_logic->POINT_COLOR);
}

void qSlicermiAnnotationModuleAnnotationPropertyDialog::onPointSelectedColorChanged(QColor qcolor)
{
    double color[3];
    this->TurnQColorToColorArray(color, qcolor);
    this->m_logic->ModifyPropertiesAndWidget(this->m_logic->GetMRMLScene()->GetNodeByID(m_nodeId), this->m_logic->POINT_SELECTED_COLOR, color);
    this->SetButtonText(this->m_logic->POINT_SELECTED_COLOR);
}

void qSlicermiAnnotationModuleAnnotationPropertyDialog::onPointSizeChanged(double value)
{
    this->m_logic->ModifyPropertiesAndWidget(this->m_logic->GetMRMLScene()->GetNodeByID(m_nodeId), this->m_logic->POINT_SIZE, &value);
}

void qSlicermiAnnotationModuleAnnotationPropertyDialog::onPointOpacityChanged(double value)
{
    this->m_logic->ModifyPropertiesAndWidget(this->m_logic->GetMRMLScene()->GetNodeByID(m_nodeId), this->m_logic->POINT_OPACITY, &value);
}

void qSlicermiAnnotationModuleAnnotationPropertyDialog::onPointAmbientChanged(double value)
{
    this->m_logic->ModifyPropertiesAndWidget(this->m_logic->GetMRMLScene()->GetNodeByID(m_nodeId), this->m_logic->POINT_AMBIENT, &value);
}
void qSlicermiAnnotationModuleAnnotationPropertyDialog::onPointDiffuseChanged(double value)
{
    this->m_logic->ModifyPropertiesAndWidget(this->m_logic->GetMRMLScene()->GetNodeByID(m_nodeId), this->m_logic->POINT_DIFFUSE, &value);
}

void qSlicermiAnnotationModuleAnnotationPropertyDialog::onPointSpecularChanged(double value)
{
    this->m_logic->ModifyPropertiesAndWidget(this->m_logic->GetMRMLScene()->GetNodeByID(m_nodeId), this->m_logic->POINT_SPECULAR, &value);
}

void qSlicermiAnnotationModuleAnnotationPropertyDialog::onLineColorChanged(QColor qcolor)
{
    double color[3];
    this->TurnQColorToColorArray(color, qcolor);
    this->m_logic->ModifyPropertiesAndWidget(this->m_logic->GetMRMLScene()->GetNodeByID(m_nodeId), this->m_logic->LINE_COLOR, color);
    this->SetButtonText(this->m_logic->LINE_COLOR);
}

void qSlicermiAnnotationModuleAnnotationPropertyDialog::onLineSelectedColorChanged(QColor qcolor)
{
    double color[3];
    this->TurnQColorToColorArray(color, qcolor);
    this->m_logic->ModifyPropertiesAndWidget(this->m_logic->GetMRMLScene()->GetNodeByID(m_nodeId), this->m_logic->LINE_SELECTED_COLOR, color);
    this->SetButtonText(this->m_logic->LINE_SELECTED_COLOR);
}

void qSlicermiAnnotationModuleAnnotationPropertyDialog::onLineWidthChanged(double value)
{
    this->m_logic->ModifyPropertiesAndWidget(this->m_logic->GetMRMLScene()->GetNodeByID(m_nodeId), this->m_logic->LINE_WIDTH, &value);
}

void qSlicermiAnnotationModuleAnnotationPropertyDialog::onLineOpacityChanged(double value)
{
    this->m_logic->ModifyPropertiesAndWidget(this->m_logic->GetMRMLScene()->GetNodeByID(m_nodeId), this->m_logic->LINE_OPACITY, &value);
}

void qSlicermiAnnotationModuleAnnotationPropertyDialog::onLineAmbientChanged(double value)
{
    this->m_logic->ModifyPropertiesAndWidget(this->m_logic->GetMRMLScene()->GetNodeByID(m_nodeId), this->m_logic->LINE_AMBIENT, &value);
}

void qSlicermiAnnotationModuleAnnotationPropertyDialog::onLineDiffuseChanged(double value)
{
    this->m_logic->ModifyPropertiesAndWidget(this->m_logic->GetMRMLScene()->GetNodeByID(m_nodeId), this->m_logic->LINE_DIFFUSE, &value);
}

void qSlicermiAnnotationModuleAnnotationPropertyDialog::onLineSpecularChanged(double value)
{
    this->m_logic->ModifyPropertiesAndWidget(this->m_logic->GetMRMLScene()->GetNodeByID(m_nodeId), this->m_logic->LINE_SPECULAR, &value);
}

void qSlicermiAnnotationModuleAnnotationPropertyDialog::UpdateLockUnlockStatus(bool isLock)
{
    if (  isLock )
    {
        ui.annotationTextEdit->setEnabled(false);
        for (int i=0; i<m_lineEditList.size(); ++i)
        {
            m_lineEditList[i]->setEnabled(false);
        }
        ui.textTab->setEnabled(false);
        ui.pointTab->setEnabled(false);
        ui.lineTab->setEnabled(false);
    } 
    else
    {
        ui.annotationTextEdit->setEnabled(true);
        for (int i=0; i<m_lineEditList.size(); ++i)
        {
            m_lineEditList[i]->setEnabled(true);
        }
        ui.textTab->setEnabled(true);
        ui.pointTab->setEnabled(true);
        ui.lineTab->setEnabled(true);
    }

}

//-----------------------------------------------------------------------------
void qSlicermiAnnotationModuleAnnotationPropertyDialog::TurnColorArrayToQColor(double* color, QColor &qcolor)
{
    qcolor.setRed( color[0]*255.0001 );
    qcolor.setGreen( color[1]*255.0001 );
    qcolor.setBlue( color[2]*255.0001 );
}


void qSlicermiAnnotationModuleAnnotationPropertyDialog::FormatValueToChar(const char* format, double value, QString &valueString )
{
    char valuechar[100];
    sprintf(valuechar, format, value);
    valueString = valuechar;
}

void qSlicermiAnnotationModuleAnnotationPropertyDialog::TurnQColorToColorArray(double* color, QColor &qcolor)
{
    color[0] = qcolor.red() / 255.0;
    color[1] = qcolor.green() / 255.0;
    color[2] = qcolor.blue() / 255.0;
}

void qSlicermiAnnotationModuleAnnotationPropertyDialog::onCollapsibleGroupBoxClicked()
{
  vtkMRMLNode * node = this->m_logic->GetMRMLScene()->GetNodeByID(this->m_nodeId);
  if ( node->IsA("vtkMRMLAnnotationStickyNode") || node->IsA("vtkMRMLAnnotationFiducialNode") )
  {
    ui.coordinatesLabel->setVisible(false);
    ui.annotationValueBrowser->setVisible(false);
    ui.displayPropertiesCTKCollapsibleGroupBox->setEnabled(false);
    ui.displayPropertiesCTKCollapsibleGroupBox->setVisible(false);
  }
}
