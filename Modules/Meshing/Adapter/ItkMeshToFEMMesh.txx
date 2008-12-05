
#ifndef __ItkMeshToFEMMesh_cxx
#define __ItkMeshToFEMMesh_cxx

#include <iostream>
#include "ItkMeshToFEMMesh.h"

namesape itk
{

template <class TInputMesh>
ItkMeshToFEMMesh<TInputMesh>
::ItkMeshToFEMMesh()
  {
    m_FileName = "";
    m_Solver = new SolverType;
  }

template <class TInputMesh>
ItkMeshToFEMMesh<TInputMesh>
::~ItkMeshToFEMMesh()
  {
  delete m_Solver;
  }

template <class TInputMesh>
typename ItkMeshToFEMMesh<TInputMesh>::SolverPointerType
ItkMeshToFEMMesh<TInputMesh>
::GetOutput( )
  {
  return m_Solver;
  }


template <class TInputMesh>
void
ItkMeshToFEMMesh<TInputMesh>
::Update( )
{
  // Create dummy material
  MaterialType::Pointer mat = MaterialType::New();
  mat->GN = 0;
  mat->E = 200E6;
  mat->A = 1.0;
  mat->h = 1.0;
  mat->I = 1.0;
  mat->nu = 0.2;
  mat->RhoC = 1.0;

  // Create element type
  HexElementType::Pointer hexElement = HexElementType::New( );
  HexElementType::Pointer hexElement2;
  hexElement->m_mat = dynamic_cast<MaterialType*>( mat );

  TriangElementType::Pointer triangElement = TriangElementType::New( );
  TriangElementType::Pointer triangElement2;
  triangElement->m_mat = dynamic_cast<MaterialType*>( mat );

  // Convert mesh points into nodes
  PointType* ptr;
  PointType pt;
  ptr = &pt;

  int numofpoints = m_Input->GetNumberOfPoints();
  //std::cerr << "# points: " << numofpoints << std::endl;


  // Convert cells into elements
  CellsContainerPointer cellList = m_Input->GetCells();
  CellIterator cells = cellList->Begin();


  //std::cerr << m_Input << std::endl;
  int numberOfCells = m_Input->GetNumberOfCells( );
  //std::cerr << "# cells: " << numberOfCells << std::endl;

  bool Flag = true;

  for (int k=0; k < numberOfCells; k++) 
    {
      CellType* cellPtr = cells.Value();
      //std::cout << "CELL TYPE  = " << cellPtr->GetType() << " Hex Element Type " <<
      //CellType::HEXAHEDRON_CELL << std::endl;

      switch( cellPtr->GetType( ) )
        {
        case CellType::HEXAHEDRON_CELL:
          {
          //std::cout << "Add Hex Element" << std::endl;
          if ( Flag ) // To make sure that the nodes are created just once
            {
              for (int j=0; j < numofpoints; j++) 
                {
                  m_Input->GetPoint(j, ptr);

                  HexNodeType::Pointer hexNode;
                  hexNode = new HexNodeType(pt[0], pt[1], pt[2]);
                  hexNode->GN = j;
                  m_Solver->node.push_back(itk::fem::FEMP<HexNodeType>(hexNode));
                }
              Flag = false;
            }
            PointIdIterator pointIt = cellPtr->PointIdsBegin() ;
            int i=0;
            hexElement2 = dynamic_cast<HexElementType*> (hexElement->Clone());
            while (pointIt != cellPtr->PointIdsEnd() ) 
              {
              hexElement2->SetNode(i, m_Solver->node.Find( *pointIt ));  
              pointIt++;  
              i++;
              }
            cells++;
              
            hexElement2->GN = k;
            m_Solver->el.push_back(itk::fem::FEMP<itk::fem::Element>(hexElement2)); 

            break;
          }

        case CellType::TRIANGLE_CELL:
          {
            if ( Flag ) // To make sure that the nodes are created just once
              {
                for (int j=0; j < numofpoints; j++) 
                  {
                    m_Input->GetPoint(k, ptr);

                    TriangNodeType::Pointer triangNode;
                    triangNode = new TriangNodeType(pt[0], pt[1], pt[2]);
                    triangNode->GN = j;
                    m_Solver->node.push_back(itk::fem::FEMP<TriangNodeType>(triangNode));
                  }
                Flag = false;
              }
            PointIdIterator pointIt = cellPtr->PointIdsBegin() ;
            int i=0;
            triangElement2 = dynamic_cast<TriangElementType*> (triangElement->Clone());
            while (pointIt != cellPtr->PointIdsEnd() ) 
              {
              triangElement2->SetNode(i, m_Solver->node.Find( *pointIt ));  
              pointIt++;  i++;
              }
            cells++;

            triangElement2->GN = k;
            m_Solver->el.push_back(itk::fem::FEMP<itk::fem::Element>(triangElement2)); 
            break;
          }
        }
    }

  //std::cerr << "Converted Cells " << std::endl;

 /* #ifdef __sgi
  // Create the file. This is required on some older sgi's
  std::ofstream tFile(m_FileName.c_str( ),std::ios::out);
  tFile.close( );           
  #endif*/
  /*std::ofstream out;
  out.open( m_FileName.c_str( ), std::ios::out );
  m_Solver->Write( out );
  out.close( );
  m_Solver->Write(std::cout);*/
  
 }
}

#endif
