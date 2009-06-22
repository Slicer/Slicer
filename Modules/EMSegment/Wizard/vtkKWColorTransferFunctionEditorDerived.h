#ifndef __vtkKWColorTransferFunctionEditorDerived_h
#define __vtkKWColorTransferFunctionEditorDerived_h

#include "vtkKWColorTransferFunctionEditor.h"


class vtkKWColorTransferFunctionEditorDerived : public vtkKWColorTransferFunctionEditor
{
public:
void value(){
  vtkKWColorTransferFunctionEditorDerived* pointer;// = vtkKWColorTransferFunctionEditorDerived::New();
  double* toto;
  int point;
  pointer->GetFunctionPointValues(point, toto);
}

};

#endif
