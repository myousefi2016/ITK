#include "itkQuadEdgeMesh.h"
#include "itkDefaultStaticMeshTraits.h"

#include <iostream>

namespace itk
{
/**
 * \class QuadEdgeMeshWithDual
 *
 * \brief Mesh class for 2D manifolds embedded in ND space and their dual.
 *
 * \author Alexandre Gouaillard, Humayun
 *
 * This implementation was contributed as a paper to the Insight Journal
 *
 * \ingroup ITK-QuadEdgeMesh
 */
template< typename TPixel, unsigned int VDimension,
          typename TTraits = QuadEdgeMeshTraits< TPixel, VDimension, bool, bool > >
class ITK_EXPORT QuadEdgeMeshWithDual : public QuadEdgeMesh< TPixel, VDimension, TTraits >
{
public:
  /** Input template parameters. */
  typedef TTraits Traits;
  typedef TPixel  PixelType;

  /** Standard typedefs. */
  typedef QuadEdgeMeshWithDual                       Self;
  typedef QuadEdgeMesh< TPixel, VDimension, Traits > Superclass;
  typedef SmartPointer< Self >                       Pointer;
  typedef SmartPointer< const Self >                 ConstPointer;

  /** Convenient constants obtained from MeshTraits. */
  itkStaticConstMacro(PointDimension, unsigned int,
                      Superclass::PointDimension);
  itkStaticConstMacro(MaxTopologicalDimension, unsigned int,
                      Superclass::MaxTopologicalDimension);

  /** Basic Object interface. */
  itkNewMacro(Self);
  itkTypeMacro(QuadEdgeMeshWithDual, QuadEdgeMesh);

  itkSuperclassTraitMacro( CellPixelType              );
  itkSuperclassTraitMacro( CoordRepType               );
  itkSuperclassTraitMacro( PointHashType              );
  itkSuperclassTraitMacro( CellTraits                 );
  itkSuperclassTraitMacro( PointIdInternalIterator    );
  itkSuperclassTraitMacro( PointIdIterator            );

  itkSuperclassTraitMacro( PointIdentifier );
  itkSuperclassTraitMacro( PointType       );
  itkSuperclassTraitMacro( PointIdList     );
  itkSuperclassTraitMacro( PointsContainer );
  itkSuperclassTraitMacro( PointsContainerPointer       );
  itkSuperclassTraitMacro( PointsContainerConstIterator );
  itkSuperclassTraitMacro( PointsContainerIterator      );
  itkSuperclassTraitMacro( PointDataContainer           );
  itkSuperclassTraitMacro( PointDataContainerPointer    );
  itkSuperclassTraitMacro( PointDataContainerIterator   );

  itkSuperclassTraitMacro( CellIdentifier  );
  itkSuperclassTraitMacro( CellType        );
  itkSuperclassTraitMacro( CellAutoPointer );
  itkSuperclassTraitMacro( CellMultiVisitorType  );
  itkSuperclassTraitMacro( CellsContainer        );
  itkSuperclassTraitMacro( CellsContainerPointer );
  itkSuperclassTraitMacro( CellsContainerConstIterator );
  itkSuperclassTraitMacro( CellsContainerIterator      );
  itkSuperclassTraitMacro( CellDataContainer           );
  itkSuperclassTraitMacro( CellDataContainerPointer    );
  itkSuperclassTraitMacro( CellDataContainerIterator   );

  itkSuperclassTraitMacro( PolygonCellType );
  itkSuperclassTraitMacro( EdgeCellType    );

  /** Accessors */
  const CellsContainerPointer  GetDualCells()     const { return m_DualCellsContainer;     };
  const CellsContainerPointer  GetDualEdgeCells() const { return m_DualEdgeCellsContainer; };
  const PointsContainerPointer GetDualPoints()    const { return m_DualPointsContainer;    };

  /** Add cells / edges / points */

  void SetDualPoint( PointIdentifier id, PointType p )
    {
    return m_DualPointsContainer->InsertElement( id, p );
    }
  PointIdentifier AddDualPoint( PointType p )
    {
    PointIdentifier pid = m_DualPointsContainer->size();
    this->SetDualPoint( pid, p );
    return( pid );
    }

  // NOTE ALEX: this does not create underlying QE layer for now
  void AddDualFace( const PointIdList & points )
    {
    // Create the cell without unterlying QE layer and add it to the container
    PolygonCellType *faceCell = new PolygonCellType( points.size() );

    CellIdentifier fid = m_DualCellsContainer->size();
    faceCell->SetIdent( fid );
    CellAutoPointer face;
    face.TakeOwnership( faceCell );
    PointIdentifier nbOfPoints = points.size();
    for( unsigned int i = 0; i < nbOfPoints; i++ )
      face->SetPointId( i, points[i] );

    m_DualCellsContainer->InsertElement( fid, face.ReleaseOwnership() );
    }

  void AddDualEdge( const PointIdentifier& pid1, const PointIdentifier& pid2 )
    {
    // Create the cell without unterlying QE layer and add it to the container
    EdgeCellType *edgeCell = new EdgeCellType( );

    CellIdentifier   eid = m_DualEdgeCellsContainer->size();
    edgeCell->SetIdent( eid );
    CellAutoPointer edge;
    edge.TakeOwnership( edgeCell );
    edge->SetPointId( 0, pid1 );
    edge->SetPointId( 1, pid2 );

    m_DualEdgeCellsContainer->InsertElement( eid, edge.ReleaseOwnership() );
    }

  PointIdentifier GetNumberOfDualPoints() { return m_DualPointsContainer->size(); };

private:
  CellsContainerPointer  m_DualCellsContainer;
  CellsContainerPointer  m_DualEdgeCellsContainer;
  PointsContainerPointer m_DualPointsContainer;

protected:
  /** Constructor and Destructor. */
  QuadEdgeMeshWithDual()
    {
    m_DualCellsContainer     = CellsContainer::New();
    m_DualEdgeCellsContainer = CellsContainer::New();
    m_DualPointsContainer    = PointsContainer::New();
    };

  virtual ~QuadEdgeMeshWithDual() { };

};
}
