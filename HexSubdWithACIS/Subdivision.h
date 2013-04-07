#ifndef SUBDIVISION_H_
#define SUBDIVISION_H_

#include <VECTOR>

#include "Point3D.h"

class ENTITY_LIST;

namespace HexMesh {

	class HexModel;
	
	class Subdivision {
	public:
		Subdivision(HexMesh::HexModel* hex_model);
		
		virtual ~Subdivision();
		
		void ApproximateSubdivision(ENTITY_LIST&);
		void InterpolateSubdivision();
		
	private:
		void CalcCellPoints();
		void CalcFacePoints();
		void CalcFacePointsInter();
		void CalcEdgePoints();
		void CalcEdgePointsInter();
		void CalcNewVertices();
		void CalcNewVerticesInter();

		void ChangeNewVertices();
		void LaplacianSmoothing(HexModel*);

		void NewVertexList(HexModel* pNewModel);
		void NewEdgeList(HexModel* pNewModel);
		void NewFaceList(HexModel* pNewModel);
		void NewCellList(HexModel* pNewModel);
		
		void FreeMemory();
		
	private:
		HexModel* m_pHexModel;
		size_t subdiv_counter;	// count the number of  subdivision
		
		std::vector<Point3D> newVertices;
		std::vector<Point3D> edgePoints;
		std::vector<Point3D> facePoints;
		std::vector<Point3D> cellPoints; 
	};
}


#endif