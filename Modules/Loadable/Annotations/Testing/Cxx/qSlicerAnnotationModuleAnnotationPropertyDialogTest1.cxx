#include "qSlicerAnnotationModuleAnnotationPropertyDialog.h"
#include "vtkSlicerAnnotationModuleLogic.h"
#include "vtkMRMLAnnotationNode.h"
#include "vtkMRMLAnnotationPointDisplayNode.h"
#include "vtkMRMLAnnotationLineDisplayNode.h"
#include "vtkMRMLAnnotationTextDisplayNode.h"
#include "vtkMRMLAnnotationFiducialNode.h"
#include "vtkMRMLAnnotationAngleNode.h"
#include "vtkMRMLAnnotationRulerNode.h"

// VTK includes
#include <vtkSmartPointer.h>

vtkSmartPointer<vtkMRMLAnnotationRulerNode> m_rulerCopy;
vtkSmartPointer<vtkMRMLAnnotationAngleNode> m_angleCopy;
vtkSmartPointer<vtkMRMLAnnotationDisplayNode> m_textDispCopy;
vtkSmartPointer<vtkMRMLAnnotationLineDisplayNode> m_lineDispCopy;
vtkSmartPointer<vtkMRMLAnnotationPointDisplayNode> m_pointDispCopy;

void SaveLinesNode(vtkMRMLAnnotationLinesNode* node);
void SaveControlPoints(vtkMRMLAnnotationControlPointsNode* node);
void SaveAnnotationNode(vtkMRMLAnnotationNode* node);
void UndoLinesNode(vtkMRMLAnnotationLinesNode* node);
void UndoControlPoints(vtkMRMLAnnotationControlPointsNode* node);
void UndoAnnotationNode(vtkMRMLAnnotationNode* node);

//-----------------------------------------------------------------------------
void SaveLinesNode(vtkMRMLAnnotationLinesNode* node)
{
    if (!node)
    {
        return;
    }
    if (!m_lineDispCopy)
    {
        m_lineDispCopy = vtkSmartPointer<vtkMRMLAnnotationLineDisplayNode>::New();
    }

    node->CreateAnnotationLineDisplayNode();
    m_lineDispCopy->Copy(node->GetAnnotationLineDisplayNode());
    SaveControlPoints(node);

}

//-----------------------------------------------------------------------------
void SaveControlPoints(vtkMRMLAnnotationControlPointsNode* node)
{
    if (!node)
    {
        return;
    }

    if (!m_pointDispCopy)
    {
        m_pointDispCopy = vtkSmartPointer<vtkMRMLAnnotationPointDisplayNode>::New();
    }
    node->CreateAnnotationPointDisplayNode();
    m_pointDispCopy->Copy(node->GetAnnotationPointDisplayNode());
    SaveAnnotationNode( (vtkMRMLAnnotationNode*) node);
}

//-----------------------------------------------------------------------------
void SaveAnnotationNode(vtkMRMLAnnotationNode* node)
{
    if (!node)
    {
        return;
    }

    if (!m_textDispCopy)
    {
        m_textDispCopy = vtkSmartPointer<vtkMRMLAnnotationTextDisplayNode>::New();
    }
    node->CreateAnnotationTextDisplayNode();
    m_textDispCopy->Copy(node->GetAnnotationTextDisplayNode());
}

//-----------------------------------------------------------------------------
void SaveStateForUndo(vtkMRMLNode* node)
{
    if( node->IsA( "vtkMRMLAnnotationAngleNode" ) )
    {
        vtkMRMLAnnotationAngleNode* mynode = vtkMRMLAnnotationAngleNode::SafeDownCast(node);
        if (!m_angleCopy)
        {
            m_angleCopy = vtkSmartPointer<vtkMRMLAnnotationAngleNode>::New();
        }
        m_angleCopy->Copy(mynode);
        SaveLinesNode(mynode);
    }
    else if (node->IsA( "vtkMRMLAnnotationRulerNode" ))
    {
        vtkMRMLAnnotationRulerNode* mynode = vtkMRMLAnnotationRulerNode::SafeDownCast(node);
        if (!m_rulerCopy)
        {
            m_rulerCopy = vtkSmartPointer<vtkMRMLAnnotationRulerNode>::New();
        }
        m_rulerCopy->Copy(mynode);
        SaveLinesNode(mynode);
    }

}

//-----------------------------------------------------------------------------
void UndoLinesNode(vtkMRMLAnnotationLinesNode* node)
{
    if (!node)
    {
        return;
    }
    node->CreateAnnotationLineDisplayNode();
    node->GetAnnotationLineDisplayNode()->Copy(m_lineDispCopy);
    UndoControlPoints(node);
}

//-----------------------------------------------------------------------------
void UndoControlPoints(vtkMRMLAnnotationControlPointsNode* node)
{
    if (!node)
    {
        return;
    }
    node->CreateAnnotationPointDisplayNode();
    node->GetAnnotationPointDisplayNode()->Copy(m_pointDispCopy);
    UndoAnnotationNode( (vtkMRMLAnnotationNode*) node);
}

//-----------------------------------------------------------------------------
void UndoAnnotationNode(vtkMRMLAnnotationNode* node)
{
    if (!node)
    {
        return;
    }
    node->CreateAnnotationTextDisplayNode();
    node->GetAnnotationTextDisplayNode()->Copy(m_textDispCopy);
}

//-----------------------------------------------------------------------------
void Undo(vtkMRMLNode* node)
{
    if( node->IsA( "vtkMRMLAnnotationAngleNode" ) )
    {
        vtkMRMLAnnotationAngleNode* anode = vtkMRMLAnnotationAngleNode::SafeDownCast(node);
        anode->Copy(m_angleCopy);
        UndoLinesNode(anode);
    }
    else if (node->IsA( "vtkMRMLAnnotationRulerNode" ))
    {
        vtkMRMLAnnotationRulerNode* rnode = vtkMRMLAnnotationRulerNode::SafeDownCast(node);
        rnode->Copy(m_rulerCopy);
        UndoLinesNode(rnode);
    }
    else if (node->IsA( "vtkMRMLAnnotationFiducialNode" ))
    {
        //ToDo
    }

}

//-----------------------------------------------------------------------------
int qSlicerAnnotationModuleAnnotationPropertyDialogTest1( int, char * [] )
{
    // Basic Setup
    vtkSmartPointer<vtkSlicerAnnotationModuleLogic > pLogic = vtkSmartPointer< vtkSlicerAnnotationModuleLogic >::New();
    vtkSmartPointer<vtkMRMLScene> pMRMLScene = vtkSmartPointer<vtkMRMLScene>::New();
    vtkSmartPointer<vtkMRMLAnnotationRulerNode> pRulerNode = vtkSmartPointer<vtkMRMLAnnotationRulerNode>::New();
    pMRMLScene->RegisterNodeClass(pRulerNode);
    pMRMLScene->AddNode(pRulerNode);

    QString textString = "MyTestString";
    pLogic->SetAnnotationLinesProperties( (vtkMRMLAnnotationLinesNode*)pRulerNode, vtkSlicerAnnotationModuleLogic::TEXT, textString.toUtf8());

    QString text1 = QString(pLogic->GetAnnotationTextProperty(pRulerNode));

    SaveStateForUndo(pRulerNode);

    QString textString2 = "AnotherTestString";
    pLogic->SetAnnotationLinesProperties( (vtkMRMLAnnotationLinesNode*)pRulerNode, vtkSlicerAnnotationModuleLogic::TEXT, textString2.toUtf8());
    QString text22 = QString(pLogic->GetAnnotationTextProperty(pRulerNode));
    std::cout << qPrintable(text22) << std::endl;

    Undo((vtkMRMLNode*)pRulerNode);

    QString text2 = QString(pLogic->GetAnnotationTextProperty(pRulerNode));

    if ( text1 == text2 )
    {
        std::cout << "SaveStateForUndo/Undo okay!" << std::endl;
        std::cout << qPrintable(text1) << std::endl;
        return 0;
    }

    std::cout << "SaveStateForUndo/Undo Failed!" << std::endl;
    std::cout << "Text 1:" << qPrintable(text2) << " Text 2: " <<qPrintable(text2) << std::endl;
    return 1;
}


