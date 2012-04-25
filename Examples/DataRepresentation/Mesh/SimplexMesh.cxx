#include "itkQuadEdgeMesh.h"
#include "itkQuadEdgeMeshWithDual.h"
#include "itkQuadEdgeMeshWithDualAdaptor.h"
#include "itkDefaultStaticMeshTraits.h"
#include "itkVTKPolyDataWriter.h"
#include "itkQuadEdgeMeshBoundaryEdgesMeshFunction.h"
#include "itkQuadEdgeMeshPolygonCell.h"
#include "itkQuadEdgeMeshEulerOperatorsTestHelper.h"

#include <iostream>


template< typename MeshType >
bool
ComputeDualPointFromFaceAndSet(
  MeshType* myPrimalMesh,
  typename MeshType::CellsContainer::ConstIterator cellIterator,
  typename MeshType::PointIdentifier numberOfDualPoints );

template< typename MeshType >
bool
ComputeDualPointsForAllPolygons(
  MeshType* myPrimalMesh
  );

template< typename MeshType >
bool
ComputeDualPolygonsForAllPoints(
  MeshType* myPrimalMesh
  );

int main( int, char ** )
{

  //-----------------------------------------
  //  Define all the types we will work with
  //-----------------------------------------

  const unsigned int dimension = 3;

  typedef itk::QuadEdgeMeshWithDual< float, dimension > SimplexMeshType;

  typedef SimplexMeshType::PointIdentifier PointIdentifier;
  typedef SimplexMeshType::CellIdentifier  CellIdentifier;
  typedef SimplexMeshType::PointsContainer PointsContainer;
  typedef SimplexMeshType::CellsContainer  CellsContainer;
  typedef SimplexMeshType::CellType        CellType;
  typedef SimplexMeshType::PointType       PointType;
  typedef SimplexMeshType::PointIdList     PointIdList;
  typedef SimplexMeshType::QEType          QuadEdgeType;
  typedef SimplexMeshType::PolygonCellType PolygonCellType;

  typedef SimplexMeshType::EdgeListPointerType EdgeListPointerType;

  typedef CellType::PointIdConstIterator PointIdConstIterator;

  typedef PointsContainer::ConstIterator PointIterator;
  typedef CellsContainer::ConstIterator  CellIterator;

  // this will be use to find and track boundaries
  typedef itk::QuadEdgeMeshBoundaryEdgesMeshFunction< SimplexMeshType > BoundaryLocatorType;

  //--------------------------------------------------------------
  // Create the DataStructures that will hold the data in memory
  //--------------------------------------------------------------

  // main DataStructure
  SimplexMeshType::Pointer myPrimalMesh = SimplexMeshType::New();

  //-----------------------------------------
  // Create an input mesh to toy around with
  //-----------------------------------------

  std::cout << "Create a square triangulated planar mesh." << std::endl;
  CreateSquareTriangularMesh< SimplexMeshType >( myPrimalMesh );

  std::cout << "Poke a hole in the mesh to have two boundaries." << std::endl;
  myPrimalMesh->LightWeightDeleteEdge( myPrimalMesh->FindEdge( 11, 12 ) );

  //-------------------------------------------------------
  // First pass: dual points for 2D cells (triangles here)
  //-------------------------------------------------------

  std::cout << "Generation of dual points: barycenter of primal cells" << std::endl;
  ComputeDualPointsForAllPolygons< SimplexMeshType >( myPrimalMesh );

  //-------------------------------------------------------
  // Second pass: dual cells (polygons) for primal points
  //-------------------------------------------------------

  std::cout << "Generate Dual Cells from the primal points" << std::endl;
  ComputeDualPolygonsForAllPoints< SimplexMeshType >( myPrimalMesh );

  //-----------------------------------------
  // last pass: Treat the borders as 1D cells
  //-----------------------------------------

  BoundaryLocatorType::Pointer boundaryEdges = BoundaryLocatorType::New();
  EdgeListPointerType boundaryEdgesPointerList = boundaryEdges->Evaluate( *myPrimalMesh );

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
    PointIdentifier firstPointId = myPrimalMesh->GetNumberOfDualPoints();
    do
      {
      // create a new point in the middle of the edge
      // this is a dual point. Cells are sample in 2d, boundaries are sample in 1D
      PointIdentifier originPointId      = currentEdge->GetOrigin().first;
      PointIdentifier destinationPointId = currentEdge->GetDestination().first;

      PointType originPoint = myPrimalMesh->GetPoint( originPointId );
      PointType destinationPoint = myPrimalMesh->GetPoint( destinationPointId );
      PointType currentPoint;
      for( unsigned int k =0; k<dimension; k++ )
        {
        currentPoint[k] = (originPoint[k] + destinationPoint[k]) / 2 ;
        }

      // add the new border point P1 to the dual point container
      currentPointId = myPrimalMesh->AddDualPoint( currentPoint );

      // find the dual point P2 associated with the face on the right
      QuadEdgeType::DualOriginRefType leftTriangle = currentEdge->GetRight();

      // add the dual edge P1-P2
      PointIdentifier leftDualPointId = leftTriangle.second;
      myPrimalMesh->AddDualEdge( currentPointId, leftDualPointId );

      // add the edge linking two new border dual points
      // either previous-current, or current-next
      if( firstTime == true )
        firstTime = false;
      else
        {
        myPrimalMesh->AddDualEdge( previousPointId, currentPointId );

        // create a point ID list to hold the dual point IDs while iterating to create the dual cell
        PointIdList pointidlist;
        pointidlist.push_back( previousPointId );
        QuadEdgeType *myEdge = currentEdge->GetOnext();
        do
          {
          QuadEdgeType::DualOriginRefType myleftTriangle = myEdge->GetLeft();
          PointIdentifier myleftDualPointId =  myleftTriangle.second;
          pointidlist.push_back( myleftDualPointId );
          myEdge = myEdge->GetOnext();
          }
        while( !myEdge->IsAtBorder() );

        pointidlist.push_back( currentPointId );

        // point list is complete, add the dual cell to the dual mesh;
        myPrimalMesh->AddDualFace( pointidlist );
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
      PointIdentifier myleftDualPointId =  myleftTriangle.second;
      pointidlist.push_back( myleftDualPointId );
      myEdge = myEdge->GetOnext();
      }
    while( !myEdge->IsAtBorder() );

    pointidlist.push_back( firstPointId );

    // point list is complete, add the dual cell to the dual mesh;
    myPrimalMesh->AddDualFace( pointidlist );
    }

  //-----------------------------------------------------
  // Write the original mesh, and the computed dual mesh
  //-----------------------------------------------------

  typedef itk::VTKPolyDataWriter<SimplexMeshType> MeshWriterType;
  MeshWriterType::Pointer writer1 = MeshWriterType::New();
  writer1->SetInput( myPrimalMesh );
  writer1->SetFileName( "TestSquareTriangularMesh.vtk" );
  writer1->Write();

  typedef itk::QuadEdgeMeshWithDualAdaptor< SimplexMeshType >  AdaptorType;
  AdaptorType* adaptor = new AdaptorType();
  adaptor->SetInput( myPrimalMesh );

  typedef itk::VTKPolyDataWriter< AdaptorType > DualMeshWriterType;
  DualMeshWriterType::Pointer writer2 = DualMeshWriterType::New();
  writer2->SetInput( adaptor );
  writer2->SetFileName( "TestSquareTriangularSimplexMesh.vtk" );
  writer2->Write();

  return EXIT_SUCCESS;
}




template< typename MeshType >
bool
ComputeDualPointFromFaceAndSet(
  MeshType* myPrimalMesh,
  typename MeshType::CellsContainer::ConstIterator cellIterator,
  typename MeshType::PointIdentifier  numberOfDualPoints )
{
  // NOTE ALEX: to extract from MeshType
  const unsigned int dimension = 3;

  // typedef typename MeshType::PointIdentifier PointIdentifier;
  typedef typename MeshType::CellIdentifier  CellIdentifier;
  // typedef typename MeshType::PointsContainer PointsContainer;
  // typedef typename MeshType::CellsContainer  CellsContainer;
  typedef typename MeshType::CellType        CellType;
  typedef typename MeshType::PointType       PointType;
  typedef typename MeshType::QEType          QuadEdgeType;
  typedef typename MeshType::PolygonCellType PolygonCellType;

  typedef typename CellType::PointIdConstIterator PointIdConstIterator;

  // typedef PointsContainer::ConstIterator PointIterator;
  // typedef CellsContainer::ConstIterator  CellIterator;
  typedef typename QuadEdgeType::DualOriginRefType DOrgRefType;
  typedef typename MeshType::CellAutoPointer CellAutoPointer;

  // 1. compute dual point coordinate and push it to the container
  PointIdConstIterator current= cellIterator.Value()->PointIdsBegin();
  PointIdConstIterator end    = cellIterator.Value()->PointIdsEnd();
  PointType d_point;
  for( unsigned int i = 0; i < 3; i++ ) // dimension; i++ )
    {
    d_point[i] = 0.0;
    }
  while( current != end )
    {
    PointType point = myPrimalMesh->GetPoint( *current );
    for( unsigned int i = 0; i < 3; i++ ) // dimension; i++ )
      {
      d_point[i] += point[i];
      }
    current++;
    }
  for( unsigned int i =0; i < 3; i++ ) // dimension; i++ )
    {
    d_point[i] /= dimension;
    }
  myPrimalMesh->SetDualPoint( numberOfDualPoints, d_point );

  // 2. Compute the new OriginRefType and set all the QEdges

  CellIdentifier cellIdentifier = cellIterator.Index();
  DOrgRefType dualOriginRef = DOrgRefType( cellIdentifier, numberOfDualPoints );


  // NOTE ALEX: isn't there a method in QE to do that?
  CellAutoPointer cellPointer;
  myPrimalMesh->GetCell( cellIdentifier, cellPointer );
  PolygonCellType* myCell = dynamic_cast< PolygonCellType* >( cellPointer.GetPointer() );
  QuadEdgeType *currentEdge = myCell->GetEdgeRingEntry();
  QuadEdgeType *firstEdge = currentEdge;
  do
    {
    currentEdge->SetLeft( dualOriginRef );
    currentEdge = currentEdge->GetLnext();
    }
  while( currentEdge != firstEdge );

  return true;
}



template< typename MeshType >
bool
ComputeDualPointsForAllPolygons(
  MeshType* myPrimalMesh
  )
{
  typedef typename MeshType::CellsContainer       CellsContainer;
  typedef typename CellsContainer::ConstIterator  CellIterator;

  const CellsContainer *primalCells = myPrimalMesh->GetCells();
  if( primalCells )
    {
    CellIterator cellIterator = primalCells->Begin();
    CellIterator cellEnd = primalCells->End();

    bool found = false;
    // NOTE ALEX: type assumption! this should be PointIdentifier.
    unsigned int numberOfDualPoints = 0;
    while( ( cellIterator != cellEnd ) && !found )
      {
      switch ( cellIterator.Value()->GetType() )
        {
        case 0: //VERTEX_CELL:
        case 1: //LINE_CELL:
        case 2: //TRIANGLE_CELL:
        case 3: //QUADRILATERAL_CELL:
          // NOTE ALEX: all those should not happen in a QEMesh
          break;
        case 4: //POLYGON_CELL:
          if( cellIterator.Value()->GetNumberOfPoints() > 3 )
            {
            // NOTE ALEX: this is nto true, the code works with polygons as well
            std::cout << "We found a polygon, this is not handled right now." << std::endl;
            found = true;
            }
          else  // triangle
            {
            ComputeDualPointFromFaceAndSet< MeshType >( myPrimalMesh, cellIterator, numberOfDualPoints );
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
  return true;
}

template< typename MeshType >
bool
ComputeDualPolygonsForAllPoints(
  MeshType* myPrimalMesh
  )
{
  typedef typename MeshType::PointsContainer       PointsContainer;
  typedef typename PointsContainer::ConstIterator  PointIterator;
  typedef typename MeshType::PointIdList           PointIdList;
  typedef typename MeshType::PointType             PointType;
  typedef typename MeshType::QEType                QuadEdgeType;

  const PointsContainer *primalPoints = myPrimalMesh->GetPoints();
  if( primalPoints )
    {
    PointIterator pointIterator = primalPoints->Begin();
    PointIterator pointEnd      = primalPoints->End();

    while( pointIterator != pointEnd )
      {
      // grab the QEdge
      PointType point = pointIterator.Value();
      QuadEdgeType * start   = point.GetEdge();
      QuadEdgeType * current = start;

      // create a point ID list to hold the dual point IDs while
      // we are iterating around a primal point to create the dual cell
      PointIdList pointidlist;

      // check that we are not on the border
      if( point.IsInternal() )
        {
        // iterate around the o-ring
        do
          {
          // push the dual point ID to the point ID list
          pointidlist.push_back( current->GetLeft().second );
          current = current->GetOnext();

          } while( current != start );

        // point list is complete, add the dual cell to the dual mesh;
        myPrimalMesh->AddDualFace( pointidlist );
        }
      // next point
      pointIterator++;
      }
    }
  return true;
}
