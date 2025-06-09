
#include "vtkMRMLOpenGLLineStippleMapper2D.h"

#include <vtkActor2D.h>
#include <vtkObjectFactory.h>
#include <vtkOpenGLShaderProperty.h>
#include <vtkOpenGLUniforms.h>
#include <vtkShaderProgram.h>
#include <vtkViewport.h>

// Bring in our shader symbols.
#include "vtkLineStippleGS.h"

//------------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLOpenGLLineStippleMapper2D);

//------------------------------------------------------------------------------
void vtkMRMLOpenGLLineStippleMapper2D::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//------------------------------------------------------------------------------
void vtkMRMLOpenGLLineStippleMapper2D::ReplaceShaderTCoord(
  std::map<vtkShader::Type, vtkShader*> shaders, vtkRenderer* vtkNotUsed(ren), vtkActor2D* vtkNotUsed(act))
{
  std::string FSSource = shaders[vtkShader::Fragment]->GetSource();

  vtkShaderProgram::Substitute(
    FSSource, "//VTK::TCoord::Dec",
    "noperspective in float stippleCoord;\n"
    "uniform int StipplePattern;");

  vtkShaderProgram::Substitute(
    FSSource, "//VTK::TCoord::Impl",
    "int stip = 0x1 & (StipplePattern >> int(fract(stippleCoord)*16.0));\n"
    "if (stip == 0)\n"
    "{\n"
    "  discard;\n"
    "}");

  shaders[vtkShader::Fragment]->SetSource(FSSource);
}

//------------------------------------------------------------------------------
void vtkMRMLOpenGLLineStippleMapper2D::ReplaceShaderValues(
  std::map<vtkShader::Type, vtkShader*> shaders, vtkViewport* viewport, vtkActor2D* act)
{
  this->Superclass::ReplaceShaderValues(shaders, viewport, act);

  // Replace geometry shader ignoring all prior replacements
  shaders[vtkShader::Geometry]->SetSource(vtkLineStippleGS);
}

//------------------------------------------------------------------------------
void vtkMRMLOpenGLLineStippleMapper2D::SetMapperShaderParameters(
  vtkOpenGLHelper& cellBO, vtkViewport* viewport, vtkActor2D* actor)
{
  this->Superclass::SetMapperShaderParameters(cellBO, viewport, actor);

  cellBO.Program->SetUniformi("StipplePattern", 0xF0F0);
  cellBO.Program->SetUniform2i("ViewportSize", viewport->GetSize());
}
