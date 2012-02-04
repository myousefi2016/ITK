#include "itkQuadEdgeMesh.h"
#include "itkQuadEdgeMeshPoint.h"
#include "itkRegularSphereMeshSource.h"
#include "itkDefaultStaticMeshTraits.h"
#include "itkVTKPolyDataWriter.h"
#include "itkQuadEdgeMeshBoundaryEdgesMeshFunction.h"
#include "itkQuadEdgeMeshTopologyChecker.h"
#include "itkQuadEdgeMeshPolygonCell.h"
#include "itkQuadEdgeMeshEulerOperatorsTestHelper.h"

#include <iostream>

int main( int argc, char ** argv )
{
  const unsigned int dimension = 3;
  typedef itk::QuadEdgeMesh< float, 3>   MeshType;
  typedef MeshType::PointIdentifier      PointIdentifier;
  typedef MeshType::CellIdentifier       CellIdentifier;
  typedef MeshType::PointsContainer      PointsContainer;
  typedef MeshType::CellsContainer       CellsContainer;
  typedef MeshType::CellType             CellType;
  typedef MeshType::PointType            PointType;
  typedef MeshType::PointIdList          PointIdList;
  typedef MeshType::QEType               QuadEdgeType;

  typedef CellType::PointIdConstIterator PointIdConstIterator;

  typedef PointsContainer::ConstIterator PointIterator;
  typedef CellsContainer::ConstIterator  CellIterator;

  typedef itk::QuadEdgeMeshBoundaryEdgesMeshFunction< MeshType > BoundaryLocatorType;

  std::map< CellIdentifier ,PointIdentifier >   DualPointFromPrimalTriangleLUT;

  MeshType::Pointer myDualMesh = MeshType::New();
  MeshType::Pointer myPrimalMesh = MeshType::New();

  std::cout << "Create a square triangulated planar mesh." << std::endl;
  CreateSquareTriangularMesh< MeshType >( myPrimalMesh );

  std::cout << "Poke a hole in the mesh to have two boundaries." << std::endl;
  myPrimalMesh->LightWeightDeleteEdge( myPrimalMesh->FindEdge( 11, 12 ) );

  std::cout << "Generation of dual points: barycenter of primal cells" << std::endl;

  const CellsContainer *primalCells = myPrimalMesh->GetCells();
  if( primalCells )
    {
    CellIterator cellIterator = primalCells->Begin();
    CellIterator cellEnd = primalCells->End();

    bool found = false;
    unsigned int numberOfDualPoints = 0;
    while( ( cellIterator != cellEnd ) && !found )
      {
      switch ( cellIterator.Value()->GetType() )
        {
        case 0: //VERTEX_CELL:
        case 1: //LINE_CELL:
        case 2: //TRIANGLE_CELL:
        case 3: //QUADRILATERAL_CELL:
          break;
        case 4: //POLYGON_CELL:
          if( cellIterator.Value()->GetNumberOfPoints() > 3 )
            {
            std::cout << "We found a polygon, this is not handled right now." << std::endl;
            found = true;
            }
          else
            {
            // compute dual point coordinate
            PointIdConstIterator current= cellIterator.Value()->PointIdsBegin();
            PointIdConstIterator end    = cellIterator.Value()->PointIdsEnd();
            PointType d_point;
            d_point[0] = 0.0;
            d_point[1] = 0.0;
            d_point[2] = 0.0;
            while( current != end )
              {
              PointType point = myPrimalMesh->GetPoint( *current );
              for( unsigned int i =0; i < dimension; i++ )
                {
                d_point[i] += point[i];
                }
              current++;
              }
            for( unsigned int i =0; i < dimension; i++ )
              {
              d_point[i] /= dimension;
              }
            // push dual point in dualPoints container
            myDualMesh->SetPoint( numberOfDualPoints, d_point );
            CellIdentifier cellIdentifier = cellIterator.Index();
            DualPointFromPrimalTriangleLUT[cellIdentifier] = numberOfDualPoints;
            numberOfDualPoints++;
            }
          break;
        case 7: //QUADRATIC_EDGE_CELL:
        case 8: //QUADRATIC_TRIANGLE_CELL:
          break;
        default:
          std::cerr << "Unhandled cell (volumic?)." << std::endl;
        }
      cellIterator++;
      }
    }

  std::cout << "Generate Dual Cells from the primal points" << std::endl;

  const PointsContainer *primalPoints = myPrimalMesh->GetPoints();
  const PointsContainer *dualPoints   = myDualMesh->GetPoints();
  if( primalPoints )
    {
    PointIterator pointIterator = primalPoints->Begin();
    PointIterator pointEnd      = primalPoints->End();

    while( pointIterator != pointEnd )
      {
      // grab the QEdge
      PointType point = pointIterator.Value();
      QuadEdgeType * start = point.GetEdge();
      QuadEdgeType * current = point.GetEdge();

      // create a point ID list to hold the dual point IDs while
      // we are iterating around a primal point to create the dual cell
      PointIdList pointidlist;

      if( point.IsInternal() )
        {
        // iterate around the o-ring
        do
          {
          // get the id of the face on the left
          QuadEdgeType::DualOriginRefType leftTriangle = current->GetLeft();

          // push the dual point ID to the point ID list
          pointidlist.push_back( DualPointFromPrimalTriangleLUT[ leftTriangle ] );

          current = current->GetOnext();

          } while( current != start );

        // point list is complete, add the dual cell to the dual mesh;
        myDualMesh->AddFace( pointidlist );
        }
      // next point
      pointIterator++;
      }
    }

  BoundaryLocatorType::Pointer boundaryEdges = BoundaryLocatorType::New();
  MeshType::EdgeListPointerType boundaryEdgesPointerList = boundaryEdges->Evaluate( *myPrimalMesh );

  // for each boundary
  unsigned int i = 0;
  while( !boundaryEdgesPointerList->empty() )
    {

    std::cout << "Boundary #" << i << std::endl;
    i++;

    // get the first edge and remove it from the list
    QuadEdgeType* firstEdge = boundaryEdgesPointerList->front();
    QuadEdgeType* currentEdge = firstEdge;
    boundaryEdgesPointerList->pop_front();

    // circulate around the boundary and do what you have to do
    bool firstTime = true;
    PointIdentifier previousPointId;
    PointIdentifier currentPointId;
    PointIdentifier firstPointId = myDualMesh->GetNumberOfPoints();
    do
      {
      // HOMEWORK 1
      // create a new point in the middle of the edge
      // this is a dual point. Cells are sample in 2d, boundaries are sample in 1D
      PointIdentifier originPointId = currentEdge->GetOrigin();
      PointIdentifier destinationPointId = currentEdge->GetDestination();

      PointType originPoint = myPrimalMesh->GetPoint( originPointId );
      PointType destinationPoint = myPrimalMesh->GetPoint( destinationPointId );
      PointType currentPoint;
      for( unsigned int k =0; k<dimension; k++ )
        {
        currentPoint[k] = (originPoint[k] + destinationPoint[k]) / 2 ;
        }

      // add the new border point P1 to the dual point container
      currentPointId = myDualMesh->AddPoint(currentPoint);

      // find the dual point P2 associated with the face on the left
      QuadEdgeType::DualOriginRefType leftTriangle = currentEdge->GetRight();

      // add the dual edge P1-P2
      PointIdentifier leftDualPointId = DualPointFromPrimalTriangleLUT[ leftTriangle ];
      myDualMesh->AddEdge( currentPointId, leftDualPointId );

      // HOMEWORK 2

      // beware border cases
      // add the edge linking two new border dual points
      // either previous-current, or current-next
      // be carefull of first, respectingly last case
      if( firstTime == true )
        firstTime = false;
      else
        {
        // HOMEWORK 3

        myDualMesh->AddEdge( previousPointId, currentPointId );

        // grab the QEdge

        // create a point ID list to hold the dual point IDs while iterating to create the dual cell
        PointIdList pointidlist;
        pointidlist.push_back( previousPointId );
        QuadEdgeType *myEdge = currentEdge->GetOnext();
        do
          {
          QuadEdgeType::DualOriginRefType myleftTriangle = myEdge->GetLeft();
          PointIdentifier myleftDualPointId =  DualPointFromPrimalTriangleLUT[ myleftTriangle ];
          pointidlist.push_back( myleftDualPointId );
          myEdge = myEdge->GetOnext();
          }
        while( !myEdge->IsAtBorder() );

        pointidlist.push_back( currentPointId );

        // point list is complete, add the dual cell to the dual mesh;
        myDualMesh->AddFace( pointidlist );
        }
      previousPointId = currentPointId;

      // Update currentEdge
      currentEdge = currentEdge->GetLnext();
      }
    while (currentEdge != firstEdge);

    PointIdList pointidlist;
    pointidlist.push_back( previousPointId );
    QuadEdgeType *myEdge = currentEdge->GetOnext();
    do
      {
      QuadEdgeType::DualOriginRefType myleftTriangle = myEdge->GetLeft();
      PointIdentifier myleftDualPointId =  DualPointFromPrimalTriangleLUT[ myleftTriangle ];
      pointidlist.push_back( myleftDualPointId );
      myEdge = myEdge->GetOnext();
      }
    while( !myEdge->IsAtBorder() );

    pointidlist.push_back( firstPointId );

    // point list is complete, add the dual cell to the dual mesh;
    myDualMesh->AddFace( pointidlist );
    }

  typedef itk::VTKPolyDataWriter<MeshType>   WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetInput( myPrimalMesh );
  writer->SetFileName( "TestSquareTriangularMesh.vtk" );
  writer->Write();

  writer->SetInput( myDualMesh );
  writer->SetFileName( "TestSquareTriangularSimplexMesh.vtk" );
  writer->Write();

  return EXIT_SUCCESS;
}
