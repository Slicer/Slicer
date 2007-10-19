#include "vtkEffectedArea.h"
#include "vtkObjectFactory.h"
#include "vtkObject.h"
#include "vtkRenderer.h"

vtkCxxRevisionMacro(vtkEffectedArea, "$Revision: 1.3 $");
vtkStandardNewMacro(vtkEffectedArea);

vtkEffectedArea::vtkEffectedArea(void)
{
    this->FirstPoint=true;
    this->Ren=NULL;
}

vtkEffectedArea::~vtkEffectedArea(void)
{
}

void vtkEffectedArea::AddWorldPoint(double x, double y,double z)
{
    //Use first point we get to set the boundaries
    if(this->FirstPoint)
    {
        for(int i=0;i<8;i++)
        {
            this->VerticesWorldCoordinates[i][0]=x;
            this->VerticesWorldCoordinates[i][1]=y;
            this->VerticesWorldCoordinates[i][2]=z;
        }
        this->FirstPoint=false;
        return;
    }
    //x
    if(x<this->VerticesWorldCoordinates[0][0])
    {
        this->VerticesWorldCoordinates[0][0]=x;
        this->VerticesWorldCoordinates[3][0]=x;
        this->VerticesWorldCoordinates[4][0]=x;
        this->VerticesWorldCoordinates[7][0]=x;
    }
    if(x>this->VerticesWorldCoordinates[1][0])
    {
        this->VerticesWorldCoordinates[1][0]=x;
        this->VerticesWorldCoordinates[2][0]=x;
        this->VerticesWorldCoordinates[5][0]=x;
        this->VerticesWorldCoordinates[6][0]=x;
    }
    //y
    if(y<this->VerticesWorldCoordinates[0][1])
    {
        this->VerticesWorldCoordinates[0][1]=y;
        this->VerticesWorldCoordinates[1][1]=y;
        this->VerticesWorldCoordinates[4][1]=y;
        this->VerticesWorldCoordinates[5][1]=y;
    }
    if(y>this->VerticesWorldCoordinates[2][1])
    {
        this->VerticesWorldCoordinates[2][1]=y;
        this->VerticesWorldCoordinates[3][1]=y;
        this->VerticesWorldCoordinates[6][1]=y;
        this->VerticesWorldCoordinates[7][1]=y;
    }

    //z
    if(z<this->VerticesWorldCoordinates[0][2])
    {
        this->VerticesWorldCoordinates[0][2]=z;
        this->VerticesWorldCoordinates[1][2]=z;
        this->VerticesWorldCoordinates[2][2]=z;
        this->VerticesWorldCoordinates[3][2]=z;
    }
    if(z>this->VerticesWorldCoordinates[4][2])
    {
        this->VerticesWorldCoordinates[4][2]=z;
        this->VerticesWorldCoordinates[5][2]=z;
        this->VerticesWorldCoordinates[6][2]=z;
        this->VerticesWorldCoordinates[7][2]=z;
    }
}

void vtkEffectedArea::GetViewArea(int viewarea[4])
{
    double viewareaEightPoints[8][3];
    for(int i=0;i<4;i++)
    {
        viewarea[0]=viewareaEightPoints[0][0];
        viewarea[1]=viewareaEightPoints[0][0];
        viewarea[2]=viewareaEightPoints[0][1];
        viewarea[3]=viewareaEightPoints[0][1];
    }
    for(int i=0;i<8;i++)
    {
         this->Ren->SetWorldPoint(VerticesWorldCoordinates[i][0],VerticesWorldCoordinates[i][1],VerticesWorldCoordinates[i][2],1);
         this->Ren->WorldToView();
         this->Ren->GetViewPoint(&viewareaEightPoints[i][0]);
         if(viewareaEightPoints[i][0]<viewarea[0])
         {
             viewarea[0]=(int)viewareaEightPoints[i][0];
         }
         if(viewareaEightPoints[i][0]>viewarea[1])
         {
             viewarea[1]=(int)(viewareaEightPoints[i][0]+1);
         }
         if(viewareaEightPoints[i][1]<viewarea[2])
         {
             viewarea[2]=(int)viewareaEightPoints[i][1];
         }
         if(viewareaEightPoints[i][1]<viewarea[3])
         {
             viewarea[3]=(int)(viewareaEightPoints[i][1]+1);
         }
    }//for
}
void vtkEffectedArea::Init(vtkRenderer *ren){
}
