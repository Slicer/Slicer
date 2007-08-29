
#include "vtkIGTDataManager.h"
#include "vtkObjectFactory.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkCylinderSource.h"
#include "vtkSphereSource.h"
#include "vtkAppendPolyData.h"




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

    this->StreamID = "";

    // streamType: 0 - matrix; 1 - image 
    switch (streamType) {
        case IGT_MATRIX_STREAM:
            {

            vtkMRMLModelNode *modelNode = vtkMRMLModelNode::New();
            vtkMRMLModelDisplayNode *dispNode = vtkMRMLModelDisplayNode::New();
            vtkMRMLLinearTransformNode *transform = vtkMRMLLinearTransformNode::New();
            transform->SetHideFromEditors(1);
            transform->SetName("IGTDataManagerTransform");
            dispNode->SetVisibility(0);

            this->MRMLScene->SaveStateForUndo();
            this->MRMLScene->AddNode(dispNode);
            this->MRMLScene->AddNode(transform);
            this->MRMLScene->AddNode(modelNode);  

            dispNode->SetScene(this->MRMLScene);

            char name[20];
            sprintf(name, "igt_matrix_%d", index);

            modelNode->SetName(name);
            modelNode->SetHideFromEditors(1);
            modelNode->SetScene(this->MRMLScene);
            modelNode->SetAndObserveDisplayNodeID(dispNode->GetID());  
            modelNode->SetAndObserveTransformNodeID(transform->GetID());  
            this->StreamID = std::string(modelNode->GetID());

            // Cylinder represents the locator stick
            vtkCylinderSource *cylinder = vtkCylinderSource::New();
            cylinder->SetRadius(1.5);
            cylinder->SetHeight(100);
            cylinder->Update();
            // Sphere represents the locator tip 
            vtkSphereSource *sphere = vtkSphereSource::New();
            sphere->SetRadius(3.0);
            sphere->SetCenter(0, -50, 0);
            sphere->Update();

            vtkAppendPolyData *apd = vtkAppendPolyData::New();
            apd->AddInput(sphere->GetOutput());
            apd->AddInput(cylinder->GetOutput());
            apd->Update();

            modelNode->SetAndObservePolyData(apd->GetOutput());
            dispNode->SetPolyData(modelNode->GetPolyData());
            
            this->Modified();  
            this->MRMLScene->Modified();

            modelNode->Delete();
            cylinder->Delete();
            sphere->Delete();
            apd->Delete();
            dispNode->Delete();
            transform->Delete();
            }
            break;

        case IGT_IMAGE_STREAM:
            break;
        default:
            break;
    }

    return this->StreamID.c_str();
}



void vtkIGTDataManager::PrintSelf(ostream& os, vtkIndent indent)
{
}

