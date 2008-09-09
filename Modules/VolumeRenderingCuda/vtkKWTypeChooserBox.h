#ifndef VTKKWTYPECHOOSERBOX_H_
#define VTKKWTYPECHOOSERBOX_H_

#include "vtkKWMenuButton.h"
#include "vtkVolumeRenderingCudaModule.h"
#include "vtkstd/vector"

class VTK_VOLUMERENDERINGCUDAMODULE_EXPORT vtkKWTypeChooserBox : public vtkKWMenuButton
{
  vtkTypeRevisionMacro(vtkKWTypeChooserBox, vtkKWMenuButton);
  
  static vtkKWTypeChooserBox* New();

    int GetSelectedType();
    void SetSelectedType(int vtk_type);
    const char* GetSelectedName();

    virtual void PrintSelf(ostream& os, vtkIndent indent);
protected:
  vtkKWTypeChooserBox();
  virtual ~vtkKWTypeChooserBox();
  
  virtual void CreateWidget();
  
  //BTX
  struct TypeName
  {
    TypeName(const char* name, int type) : Name(name), Type(type) {};
    int Type;
    const char* Name;
  };
  
  typedef vtkstd::vector<TypeName> TypeList;
  static const TypeList Values;
  static TypeList CreateValues();
  //ETX
};

#endif /*VTKKWTYPECHOOSERBOX_H_*/
