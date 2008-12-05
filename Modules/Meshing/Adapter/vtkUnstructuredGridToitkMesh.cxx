
#include <iostream>
#include "vtkUnstructuredGridToitkMesh.h"
#include <vtkIdList.h>


#ifndef vtkDoubleType
#define vtkDoubleType double
#endif

#ifndef vtkFloatingPointType
# define vtkFloatingPointType vtkFloatingPointType
typedef float vtkFloatingPointType;
#endif

vtkUnstructuredGridToitkMesh
::vtkUnstructuredGridToitkMesh()
{

}


vtkUnstructuredGridToitkMesh
::~vtkUnstructuredGridToitkMesh()
{
  if (m_Grid)
    {
      m_Grid->Delete();
    }
}

void
vtkUnstructuredGridToitkMesh
::SetInput(vtkUnstructuredGrid * grid)
{
  m_Grid = grid;
  this->ConvertvtkToitk();
}

vtkUnstructuredGridToitkMesh::MeshType::Pointer
vtkUnstructuredGridToitkMesh
::GetOutput()
{
  return m_itkMesh;
}

void
vtkUnstructuredGridToitkMesh
::ConvertvtkToitk()
{
  m_itkMesh = MeshType::New();

  // Get the points from vtk
  vtkPoints* vtkpoints = m_Grid->GetPoints();
  int numPoints = vtkpoints->GetNumberOfPoints();

  // Create a compatible point container for the mesh
  // the mesh is created with a null points container
  MeshType::PointsContainer::Pointer points = 
  MeshType::PointsContainer::New();
  
  // Resize the point container to be able to fit the vtk points
  points->Reserve(numPoints);
  
  // Set the point container on the mesh
  m_itkMesh->SetPoints(points);
  for(int i =0; i < numPoints; i++)
    {
      double* apoint = vtkpoints->GetPoint(i);
      float apt[3];
      apt[0] = static_cast<float>(apoint[0]);
      apt[1] = static_cast<float>(apoint[1]);
      apt[2] = static_cast<float>(apoint[2]);
      m_itkMesh->SetPoint(i, MeshType::PointType(apt));
    }
  
  vtkCellArray* vtkcells = m_Grid->GetCells();
  MeshType::CellsContainerPointer cells = MeshType::CellsContainer::New();
  
  
  // extract the cell id's from the vtkUnstructuredGrid
  int numcells = vtkcells->GetNumberOfCells();
  
  // extract cell type for each cell
  int* vtkCellTypes = new int[numcells];
  int cellId =0;
  for(; cellId < numcells; cellId++)
    {
    vtkCellTypes[cellId] = m_Grid->GetCellType(cellId);
    }
    
    /*
    std::ofstream out;  
    out.open( "VTKtoITk.txt", std::ios::out );
    */
  cells->Reserve(numcells);
  m_itkMesh->SetCells(cells);
  
  vtkIdType npts;
  vtkIdType* pts;
  cellId = 0;
  for(vtkcells->InitTraversal(); vtkcells->GetNextCell(npts, pts); cellId++)
    {
      //out << " \n VTK cell ID = " << cellId << " " << vtkCellTypes[cellId] << "\n" << std::endl;
      MeshType::CellAutoPointer c;
      switch(vtkCellTypes[cellId])
          case VTK_HEXAHEDRON:
            {
              typedef itk::CellInterface<double, MeshType::CellTraits> CellInterfaceType;
              typedef itk::HexahedronCell<CellInterfaceType> HexahedronCellType;
              HexahedronCellType * t = new HexahedronCellType;
              for ( int i = 0; i < 8; i ++)
              {
                //out << "\tVTK Point ids = " << pts[i] << std::endl;
                t->SetPointId(i, static_cast<unsigned long>(pts[i]));
              }
              
              //t->SetPointIds((unsigned long*)pts);
              c.TakeOwnership( t );
              break;
         }
      m_itkMesh->SetCell(cellId, c);
    }

}

