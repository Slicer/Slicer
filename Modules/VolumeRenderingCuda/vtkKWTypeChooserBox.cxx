#include "vtkKWTypeChooserBox.h"

#include "vtkObjectFactory.h"
#include "vtkKWMenu.h"

vtkCxxRevisionMacro(vtkKWTypeChooserBox, "$Revision: 1.0 $");
vtkStandardNewMacro(vtkKWTypeChooserBox);

vtkKWTypeChooserBox::vtkKWTypeChooserBox()
{
  
}

vtkKWTypeChooserBox::~vtkKWTypeChooserBox()
{
}


void vtkKWTypeChooserBox::PrintSelf(ostream& os, vtkIndent indent)
{
  
}


int vtkKWTypeChooserBox::GetSelectedType()
{
  const char* value = this->GetValue();
  for (TypeList::const_iterator it = vtkKWTypeChooserBox::Values.begin();
  it != vtkKWTypeChooserBox::Values.end();
  ++it)
    if (!strcmp((*it).Name, value))
      return (*it).Type;
  return VTK_VOID;
}

void vtkKWTypeChooserBox::SetSelectedType(int vtk_type)
{
    for (unsigned int i = 0 ; i <  vtkKWTypeChooserBox::Values.size(); i ++)
      if (vtkKWTypeChooserBox::Values[i].Type == vtk_type)
        this->GetMenu()->SelectItem(i);
}

const char* vtkKWTypeChooserBox::GetSelectedName()
{
  return this->GetValue();  
}


void vtkKWTypeChooserBox::CreateWidget()
{
  this->Superclass::CreateWidget();
  
  for (TypeList::const_iterator it = vtkKWTypeChooserBox::Values.begin();
  it != vtkKWTypeChooserBox::Values.end();
  ++it)
  {
    this->GetMenu()->AddRadioButton((*it).Name);
  }
  
  this->SetSelectedType(VTK_UNSIGNED_CHAR);
}

const vtkKWTypeChooserBox::TypeList vtkKWTypeChooserBox::Values = vtkKWTypeChooserBox::CreateValues();
vtkKWTypeChooserBox::TypeList vtkKWTypeChooserBox::CreateValues()
{
  vtkKWTypeChooserBox::TypeList ret;
  
  ret.push_back(TypeName("void", VTK_VOID)); 
  ret.push_back(TypeName("bit", VTK_BIT));
  ret.push_back(TypeName("char", VTK_CHAR));
  ret.push_back(TypeName("signed char", VTK_SIGNED_CHAR));
  ret.push_back(TypeName("unsigned char", VTK_UNSIGNED_CHAR));
  ret.push_back(TypeName("short", VTK_SHORT));
  ret.push_back(TypeName("unsigned short", VTK_UNSIGNED_SHORT));
  ret.push_back(TypeName("int", VTK_INT));
  ret.push_back(TypeName("unsigned int", VTK_UNSIGNED_INT));
  ret.push_back(TypeName("long", VTK_LONG));
  ret.push_back(TypeName("unsigned long", VTK_UNSIGNED_LONG));
  ret.push_back(TypeName("float", VTK_FLOAT));
  ret.push_back(TypeName("double", VTK_DOUBLE));
  
  return ret;
}
