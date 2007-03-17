
#include "vtkIGTDataManager.h"
#include "vtkObjectFactory.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkCylinderSource.h"

#include <string>


vtkStandardNewMacro(vtkIGTDataManager);
vtkCxxRevisionMacro(vtkIGTDataManager, "$Revision: 1.0 $");

int vtkIGTDataManager::index = 0;

vtkIGTDataManager::vtkIGTDataManager()
{
    this->MRMLScene = NULL;
}


vtkIGTDataManager::~vtkIGTDataManager()
{

}


const char *vtkIGTDataManager::RegisterStream(int streamType)
{
    std::string id("");

    // streamType: 0 - matrix; 1 - image 
    switch (streamType) {
        case IGT_MATRIX_STREAM:
            {

            vtkMRMLModelNode *modelNode = vtkMRMLModelNode::New();
            vtkMRMLModelDisplayNode *dispNode = vtkMRMLModelDisplayNode::New();
            dispNode->SetVisibility(0);

            this->MRMLScene->SaveStateForUndo();
            this->MRMLScene->AddNode(dispNode);
            this->MRMLScene->AddNode(modelNode);  

            dispNode->SetScene(this->MRMLScene);

            char name[20];
            sprintf(name, "igt_matrix_%d", index);

            modelNode->SetName(name);
            modelNode->SetHideFromEditors(1);
            modelNode->SetScene(this->MRMLScene);
            modelNode->SetAndObserveDisplayNodeID(dispNode->GetID());  
            id = std::string(modelNode->GetID());

            vtkCylinderSource *cylinder = vtkCylinderSource::New();
            cylinder->SetRadius(1.5);
            cylinder->SetHeight(100);
            cylinder->Update();
            modelNode->SetAndObservePolyData(cylinder->GetOutput());
            this->Modified();  
            this->MRMLScene->Modified();

            modelNode->Delete();
            cylinder->Delete();
            dispNode->Delete();
            }
            break;

        case IGT_IMAGE_STREAM:
            break;
        default:
            break;
    }

    return id.c_str();
}



void vtkIGTDataManager::PrintSelf(ostream& os, vtkIndent indent)
{
}

