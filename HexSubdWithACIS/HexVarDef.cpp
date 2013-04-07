#pragma warning(disable:4786)

#include <CMATH>
#include <IOSTREAM>

#include "HexSubDef.h"







//-------------------------------------

Subdivision::Subdivision()  : subdivCounter(0)
{
	m_pHexModel = new HexModel;
	//InitCube();
}

//-------------------------------------

Subdivision::~Subdivision() {
	if (m_pHexModel)
		delete m_pHexModel;
	m_pHexModel = NULL;
	FreeMemory();
}



void Subdivision::FreeMemory() {
	newVertices.clear();
	edgePoints.clear();
	facePoints.clear();
	cellPoints.clear();
}


/************************************************************************/
/* calculate cell points after volume subdivision                       */
/************************************************************************/
void Subdivision::CalcCellPoints() {

	Point3D cent;
	std::vector<HexCell>::iterator cellIter = m_pHexModel->cells.begin();
	for (; cellIter != m_pHexModel->cells.end(); ++ cellIter) {
		cent.zero();
		int_set_iter vertexOfCellIter = cellIter->vertexIndices.begin();
		for (; vertexOfCellIter != cellIter->vertexIndices.end(); ++ vertexOfCellIter)
			cent = cent + m_pHexModel->vertices[*vertexOfCellIter].coordinate;
		cent = cent / cellIter->vertexIndices.size();
		cellPoints.push_back(cent);
	}
}
/************************************************************************/
/* calculate face points after approximating subdivision                */
/************************************************************************/
void Subdivision::CalcFacePoints() {

	Point3D cent;
	std::vector<HexFace>::iterator faceIter = m_pHexModel->faces.begin();
	for (; faceIter != m_pHexModel->faces.end(); ++faceIter) {
		cent.zero();
		int_set_iter vertexOfFaceIter = faceIter->vertexIndices.begin();
		for (; vertexOfFaceIter != faceIter->vertexIndices.end(); ++ vertexOfFaceIter) {
			cent = cent + m_pHexModel->vertices[*vertexOfFaceIter].coordinate;
		}
		cent = cent / faceIter->vertexIndices.size();

		if (faceIter->cellIndices[0] > -1 && 
			faceIter->cellIndices[1] > -1) {
				cent = cent * 0.5 + (cellPoints[faceIter->cellIndices[0]]
					+ cellPoints[faceIter->cellIndices[1]]) * 0.25;
		}

		facePoints.push_back(cent);
	}
}


/************************************************************************/
/* calculate face points after interpolating subdivision                */
/************************************************************************/
void Subdivision::CalcFacePointsInter() {

	Point3D cent;
	std::vector<HexFace>::iterator faceIter = m_pHexModel->faces.begin();
	for (; faceIter != m_pHexModel->faces.end(); ++faceIter) {
		cent.zero();
		int_set_iter vertexOfFaceIter = faceIter->vertexIndices.begin();
		for (; vertexOfFaceIter != faceIter->vertexIndices.end(); ++ vertexOfFaceIter) {
			cent = cent + m_pHexModel->vertices[*vertexOfFaceIter].coordinate;
		}
		cent = cent / faceIter->vertexIndices.size();

		/*if (faceIter->cellIndices[0] > -1 && 
			faceIter->cellIndices[1] > -1) {
				cent = cent * 0.5 + (cellPoints[faceIter->cellIndices[0]]
				+ cellPoints[faceIter->cellIndices[1]]) * 0.25;
		}*/

		facePoints.push_back(cent);
	}
}
/************************************************************************/
/* calculate edge points after approximating subdivision                */
/************************************************************************/
void Subdivision::CalcEdgePoints() {

	// calculate edge point
	Point3D cent;
	std::vector<HexEdge>::iterator edgeIter = m_pHexModel->edges.begin();
	for (; edgeIter != m_pHexModel->edges.end(); ++ edgeIter) {
		cent.zero();
		cent = (m_pHexModel->vertices[edgeIter->vertexIndices[0]].coordinate + 
			m_pHexModel->vertices[edgeIter->vertexIndices[1]].coordinate) * 0.5;
		int numOfCells = edgeIter->cellIndices.size();
		int numOfFaces = edgeIter->faceIndices.size(); 
		if (numOfCells == numOfFaces) {

			Point3D avgFacePoint;
			avgFacePoint.zero();
			int_set_iter faceWithEdgeIter = edgeIter->faceIndices.begin();
			for (; faceWithEdgeIter != edgeIter->faceIndices.end(); 
				++ faceWithEdgeIter) {
				avgFacePoint = avgFacePoint + facePoints[*faceWithEdgeIter] ;
			}

			avgFacePoint = avgFacePoint / numOfFaces;

			Point3D avgCellPoint;
			avgCellPoint.zero();
			int_set_iter cellWithEdgeIter = edgeIter->cellIndices.begin();
			for (; cellWithEdgeIter != edgeIter->cellIndices.end(); 
				++ cellWithEdgeIter) {
				avgCellPoint = avgCellPoint + cellPoints[*cellWithEdgeIter];
			}

			avgCellPoint = avgCellPoint / numOfCells;

			cent = (cent + avgFacePoint + avgCellPoint) / 3;

		} else {

			Point3D avgBorderFacePoint;
			avgBorderFacePoint.zero();
			int coutBorderFaces = 0;
			int_set_iter faceWithEdgeIter = edgeIter->faceIndices.begin();
			for (; faceWithEdgeIter != edgeIter->faceIndices.end();
				++ faceWithEdgeIter)
			{
				if (m_pHexModel->faces[*faceWithEdgeIter].cellIndices[0] == -1 ||
					m_pHexModel->faces[*faceWithEdgeIter].cellIndices[1] == -1)
				{
					avgBorderFacePoint = avgBorderFacePoint + facePoints[*faceWithEdgeIter];
					++ coutBorderFaces;
				}
			}

			avgBorderFacePoint = avgBorderFacePoint / coutBorderFaces;

			cent = (cent + avgBorderFacePoint) * 0.5;

		}

		edgePoints.push_back(cent);
	}

}



/************************************************************************/
/* calculate edge points after interpolating subdivision                */
/************************************************************************/
void Subdivision::CalcEdgePointsInter() {

	Point3D cent;
	std::vector<HexEdge>::iterator edgeIter = m_pHexModel->edges.begin();
	for (; edgeIter != m_pHexModel->edges.end(); ++ edgeIter) {
		cent.zero();
		cent = (m_pHexModel->vertices[edgeIter->vertexIndices[0]].coordinate + 
			m_pHexModel->vertices[edgeIter->vertexIndices[1]].coordinate) * 0.5;
		/*int numOfCells = edgeIter->cellIndices.size();
		int numOfFaces = edgeIter->faceIndices.size(); 
		if (numOfCells == numOfFaces) {

		Point3D avgFacePoint;
		avgFacePoint.zero();
		std::set<int>::iterator faceWithEdgeIter = edgeIter->faceIndices.begin();
		for (; faceWithEdgeIter != edgeIter->faceIndices.end(); 
		++ faceWithEdgeIter) {
		avgFacePoint = avgFacePoint + facePoints[*faceWithEdgeIter] ;
		}

		avgFacePoint = avgFacePoint / numOfFaces;

		Point3D avgCellPoint;
		avgCellPoint.zero();
		std::set<int>::iterator cellWithEdgeIter = edgeIter->cellIndices.begin();
		for (; cellWithEdgeIter != edgeIter->cellIndices.end(); 
		++ cellWithEdgeIter) {
		avgCellPoint = avgCellPoint + cellPoints[*cellWithEdgeIter];
		}

		avgCellPoint = avgCellPoint / numOfCells;

		cent = (cent + avgFacePoint + avgCellPoint) / 3;

		}*/ 
		/*else {

		  Point3D avgBorderFacePoint;
		  avgBorderFacePoint.zero();
		  int coutBorderFaces = 0;
		  std::set<int>::iterator faceWithEdgeIter = edgeIter->faceIndices.begin();
		  for (; faceWithEdgeIter != edgeIter->faceIndices.end();
		  ++ faceWithEdgeIter)
		  {
		  if (m_pHexModel->faces[*faceWithEdgeIter].cellIndices[0] == -1 ||
		  m_pHexModel->faces[*faceWithEdgeIter].cellIndices[1] == -1)
		  {
		  avgBorderFacePoint = avgBorderFacePoint + facePoints[*faceWithEdgeIter];
		  ++ coutBorderFaces;
		  }
		  }

		  avgBorderFacePoint = avgBorderFacePoint / coutBorderFaces;

		  cent = (cent + avgBorderFacePoint) * 0.5;

		  }*/

		edgePoints.push_back(cent);
	}

}
/************************************************************************/
/* calculate new vertices after approximating subdivision               */
/************************************************************************/
void Subdivision::CalcNewVertices() {

	Point3D cent;
	std::vector<HexVertex>::iterator vertexIter = m_pHexModel->vertices.begin();
	for (; vertexIter != m_pHexModel->vertices.end(); ++ vertexIter) 
	{
		cent.zero();
		cent = vertexIter->coordinate;
		int numOfFacesWithVert = vertexIter->faceIndices.size();

		if (numOfFacesWithVert != 12) 
		{
			int numOfBorderFaces = 0;
			Point3D avgBorderFacePoint;
			avgBorderFacePoint.zero();
			int_set_iter borderFaceWithVertIter = vertexIter->faceIndices.begin();
			for (; borderFaceWithVertIter != vertexIter->faceIndices.end(); 
				++ borderFaceWithVertIter)
			{
					if (m_pHexModel->faces[*borderFaceWithVertIter].cellIndices[0] == -1 ||
						m_pHexModel->faces[*borderFaceWithVertIter].cellIndices[1] == -1)
					{
						++ numOfBorderFaces;
						avgBorderFacePoint = avgBorderFacePoint + facePoints[*borderFaceWithVertIter];
					}
			}

			avgBorderFacePoint = avgBorderFacePoint / numOfBorderFaces;

			int numOfBorderEdges = 0;
			Point3D avgBorderEdgeMid;
			avgBorderEdgeMid.zero();
			int_set_iter borderEdgeWithVertIter = vertexIter->edgeIndices.begin();
			for (; borderEdgeWithVertIter != vertexIter->edgeIndices.end(); 
				++ borderEdgeWithVertIter)
			{
					if (m_pHexModel->edges[*borderEdgeWithVertIter].faceIndices.size() != 
						m_pHexModel->edges[*borderEdgeWithVertIter].cellIndices.size()) 
					{
						avgBorderEdgeMid = avgBorderEdgeMid + 
							(m_pHexModel->vertices[m_pHexModel->edges[*borderEdgeWithVertIter].vertexIndices[0]].coordinate +
							m_pHexModel->vertices[m_pHexModel->edges[*borderEdgeWithVertIter].vertexIndices[1]].coordinate) * 0.5;
						++ numOfBorderEdges;
					}
			}

			avgBorderEdgeMid = avgBorderEdgeMid / numOfBorderEdges;

			cent = (cent * (numOfBorderFaces - 3) + 
				avgBorderEdgeMid * 2 + avgBorderFacePoint) / numOfBorderFaces;

		} else {

			Point3D avgCellPoint;
			avgCellPoint.zero();
			int_set_iter cellWithVertIter = vertexIter->cellIndices.begin();
			for (; cellWithVertIter != vertexIter->cellIndices.end(); ++ cellWithVertIter) {
				avgCellPoint = avgCellPoint + cellPoints[*cellWithVertIter];
			}

			avgCellPoint = avgCellPoint / vertexIter->cellIndices.size();

			Point3D avgFacePoint;
			avgFacePoint.zero();
			int_set_iter faceWithVertIter = vertexIter->faceIndices.begin();
			for (; faceWithVertIter != vertexIter->faceIndices.end(); ++ faceWithVertIter) {
				avgFacePoint = avgFacePoint + facePoints[*faceWithVertIter];
			}

			avgFacePoint = avgFacePoint / vertexIter->faceIndices.size();

			Point3D avgEdgeMid;
			avgEdgeMid.zero();
			int_set_iter edgeWithVertIter = vertexIter->edgeIndices.begin();
			for (; edgeWithVertIter != vertexIter->edgeIndices.end(); ++ edgeWithVertIter) {
				avgEdgeMid = avgEdgeMid + 
					(m_pHexModel->vertices[m_pHexModel->edges[*edgeWithVertIter].vertexIndices[0]].coordinate +
					m_pHexModel->vertices[m_pHexModel->edges[*edgeWithVertIter].vertexIndices[1]].coordinate) * 0.5;
			}

			avgEdgeMid = avgEdgeMid / vertexIter->edgeIndices.size();

			cent = (cent * (numOfFacesWithVert - 6) + avgEdgeMid * 3 
				+ avgFacePoint * 2 + avgCellPoint) / numOfFacesWithVert;

		}

		newVertices.push_back(cent);
	}

}


/************************************************************************/
/* calculate new vertices after interpolating subdivision               */
/************************************************************************/
void Subdivision::CalcNewVerticesInter() {

	Point3D cent;
	std::vector<HexVertex>::iterator vertexIter = m_pHexModel->vertices.begin();
	for (; vertexIter != m_pHexModel->vertices.end(); ++ vertexIter) 
	{
		cent.zero();
		cent = vertexIter->coordinate;
		/*int numOfFacesWithVert = vertexIter->faceIndices.size();

		if (numOfFacesWithVert == 12) 
		{

		Point3D avgCellPoint;
		avgCellPoint.zero();
		std::set<int>::iterator cellWithVertIter = vertexIter->cellIndices.begin();
		for (; cellWithVertIter != vertexIter->cellIndices.end(); ++ cellWithVertIter) {
		avgCellPoint = avgCellPoint + cellPoints[*cellWithVertIter];
		}

		avgCellPoint = avgCellPoint / vertexIter->cellIndices.size();

		Point3D avgFacePoint;
		avgFacePoint.zero();
		std::set<int>::iterator faceWithVertIter = vertexIter->faceIndices.begin();
		for (; faceWithVertIter != vertexIter->faceIndices.end(); ++ faceWithVertIter) {
		avgFacePoint = avgFacePoint + facePoints[*faceWithVertIter];
		}

		avgFacePoint = avgFacePoint / vertexIter->faceIndices.size();

		Point3D avgEdgeMid;
		avgEdgeMid.zero();
		std::set<int>::iterator edgeWithVertIter = vertexIter->edgeIndices.begin();
		for (; edgeWithVertIter != vertexIter->edgeIndices.end(); ++ edgeWithVertIter) {
		avgEdgeMid = avgEdgeMid + 
		(m_pHexModel->vertices[m_pHexModel->edges[*edgeWithVertIter].vertexIndices[0]].coordinate +
		m_pHexModel->vertices[m_pHexModel->edges[*edgeWithVertIter].vertexIndices[1]].coordinate) * 0.5;
		}

		avgEdgeMid = avgEdgeMid / vertexIter->edgeIndices.size();

		cent = (cent * (numOfFacesWithVert - 6) + avgEdgeMid * 3 
		+ avgFacePoint * 2 + avgCellPoint) / numOfFacesWithVert;

		}*/

		newVertices.push_back(cent);
	}

}
/************************************************************************/
/*             new vertex link                                          */
/************************************************************************/
void Subdivision::NewVertexList(HexModel* pNewModel) {

	HexVertex vert;
	std::vector<Point3D>::iterator newVertIter;

	newVertIter = newVertices.begin();
	for (; newVertIter != newVertices.end(); ++ newVertIter) {
		vert.coordinate = *newVertIter;
		pNewModel->vertices.push_back(vert);
	}

	newVertIter = edgePoints.begin();
	for (; newVertIter != edgePoints.end(); ++ newVertIter) {
		vert.coordinate = *newVertIter;
		pNewModel->vertices.push_back(vert);
	}

	newVertIter = facePoints.begin();
	for (; newVertIter != facePoints.end(); ++ newVertIter) {
		vert.coordinate = *newVertIter;
		pNewModel->vertices.push_back(vert);
	}

	newVertIter = cellPoints.begin();
	for (; newVertIter != cellPoints.end(); ++ newVertIter) {
		vert.coordinate = *newVertIter;
		pNewModel->vertices.push_back(vert);
	}

}
/************************************************************************/
/*    new edge link                                                     */
/************************************************************************/
void Subdivision::NewEdgeList(HexModel* pNewModel) {

	HexEdge edge;
	int vertIndex = 0;
	int edgeIndex = 0;
	int pos1, pos2;
	std::vector<HexEdge>::iterator edgeIter = m_pHexModel->edges.begin();
	for (; edgeIter != m_pHexModel->edges.end(); ++ edgeIter, ++ vertIndex) {
		pos1 = edgeIter->vertexIndices[0];
		pos2 = vertIndex + newVertices.size();
		edge.vertexIndices[0] = pos1;
		edge.vertexIndices[1] = pos2;
		pNewModel->edges.push_back(edge);

		pNewModel->vertices[pos1].edgeIndices.insert(edgeIndex);
		pNewModel->vertices[pos2].edgeIndices.insert(edgeIndex);
		++ edgeIndex;

		pos1 = edgeIter->vertexIndices[1];
		edge.vertexIndices[0] = pos1;
		pNewModel->edges.push_back(edge);

		pNewModel->vertices[pos1].edgeIndices.insert(edgeIndex);
		pNewModel->vertices[pos2].edgeIndices.insert(edgeIndex);
		++ edgeIndex;
	}

	vertIndex = 0;
	std::vector<HexFace>::iterator faceIter = m_pHexModel->faces.begin();
	for (; faceIter != m_pHexModel->faces.end(); ++ faceIter, ++ vertIndex) {
		pos2 = vertIndex + newVertices.size() + edgePoints.size();
		edge.vertexIndices[1] = pos2;
		int_set_iter edgeOfFaceIter = faceIter->edgeIndices.begin();
		for (; edgeOfFaceIter != faceIter->edgeIndices.end(); ++ edgeOfFaceIter) {
			pos1 = *edgeOfFaceIter + newVertices.size();
			edge.vertexIndices[0] = pos1;
			pNewModel->edges.push_back(edge);
			pNewModel->vertices[pos1].edgeIndices.insert(edgeIndex);
			pNewModel->vertices[pos2].edgeIndices.insert(edgeIndex);
			++ edgeIndex;
		}
	}

	vertIndex = 0;
	std::vector<HexCell>::iterator cellIter = m_pHexModel->cells.begin();
	for (; cellIter != m_pHexModel->cells.end(); ++ cellIter, ++ vertIndex) {
		pos2 = vertIndex + newVertices.size() + edgePoints.size() + facePoints.size();
		edge.vertexIndices[1] = pos2;
		int_set_iter faceOfCellIter = cellIter->faceIndices.begin();
		for (; faceOfCellIter != cellIter->faceIndices.end(); ++ faceOfCellIter) {
			pos1 = *faceOfCellIter + newVertices.size() + edgePoints.size();
			edge.vertexIndices[0] = pos1;
			pNewModel->edges.push_back(edge);

			pNewModel->vertices[pos1].edgeIndices.insert(edgeIndex);
			pNewModel->vertices[pos2].edgeIndices.insert(edgeIndex);
			++ edgeIndex;
		}
	}

}
/************************************************************************/
/*    new face link                                                     */
/************************************************************************/
void Subdivision::NewFaceList(HexModel* pNewModel) {
		
	// in every face, it will subdivide one face to four faces
	int_set_iter SetIntIter;
	int i, j;

	int newFaceIndex;
	int faceIndex = 0;
	const int numOfNewFacesInFace = 4;
	HexFace faceInFace[numOfNewFacesInFace];
	std::vector<HexFace>::iterator faceIter = m_pHexModel->faces.begin();
	for (; faceIter != m_pHexModel->faces.end(); ++ faceIter, ++ faceIndex) {

		int facePointIndex = faceIndex + newVertices.size() + edgePoints.size();

		int vertexIndexInFace[4];
		SetIntIter = faceIter->vertexIndices.begin();
		for (i = 0;
			SetIntIter != faceIter->vertexIndices.end() && i < 4;
			++ SetIntIter, ++ i) {
				vertexIndexInFace[i] = *SetIntIter;				
		}

		int edgePointIndexInFace[4];
		SetIntIter = faceIter->edgeIndices.begin();
		for (i = 0;
			SetIntIter != faceIter->edgeIndices.end() && i < 4; 
			++ SetIntIter, ++ i) {
				edgePointIndexInFace[i] = *SetIntIter + newVertices.size();
		}

		for (i = 0; i < numOfNewFacesInFace; ++ i) {

			// vertices in face
			faceInFace[i].vertexIndices.insert(facePointIndex);
			faceInFace[i].vertexIndices.insert(vertexIndexInFace[i]);

			int edgePointIndexInIthFace[2];
			int k = 0;

			SetIntIter = pNewModel->vertices[vertexIndexInFace[i]].edgeIndices.begin();
			for (; SetIntIter != pNewModel->vertices[vertexIndexInFace[i]].edgeIndices.end(); 
				++ SetIntIter) {
					for (j = 0; j < 4; ++ j) {
						if (pNewModel->edges[*SetIntIter].vertexIndices[0] == edgePointIndexInFace[j] ||
							pNewModel->edges[*SetIntIter].vertexIndices[1] == edgePointIndexInFace[j]) {
								if (k < 2)
									edgePointIndexInIthFace[k++] = edgePointIndexInFace[j];
								// put the vertex into face
								faceInFace[i].vertexIndices.insert(edgePointIndexInFace[j]);
								// put the edge into face
								faceInFace[i].edgeIndices.insert(*SetIntIter);
						}
					}
			}

			newFaceIndex = i + numOfNewFacesInFace * faceIndex;

			SetIntIter = faceInFace[i].vertexIndices.begin();
			for (; SetIntIter != faceInFace[i].vertexIndices.end(); ++ SetIntIter) {
				pNewModel->vertices[*SetIntIter].faceIndices.insert(newFaceIndex);
			}

			// edges in face
			SetIntIter = pNewModel->vertices[facePointIndex].edgeIndices.begin();
			for (; SetIntIter != pNewModel->vertices[facePointIndex].edgeIndices.end(); ++ SetIntIter) {
				for (k = 0; k < 2; ++ k) {
					if (pNewModel->edges[*SetIntIter].vertexIndices[0] == edgePointIndexInIthFace[k] ||
						pNewModel->edges[*SetIntIter].vertexIndices[1] == edgePointIndexInIthFace[k])
						faceInFace[i].edgeIndices.insert(*SetIntIter);
				}
			}

			SetIntIter = faceInFace[i].edgeIndices.begin();
			for (; SetIntIter != faceInFace[i].edgeIndices.end(); ++ SetIntIter) {
				pNewModel->edges[*SetIntIter].faceIndices.insert(newFaceIndex);
			}

			pNewModel->faces.push_back(faceInFace[i]);

			faceInFace[i].vertexIndices.clear();
			faceInFace[i].edgeIndices.clear();

		}

	}

	// in every cell, it will create 12 new faces
	const int numOfNewFacesInCell = 12;
	HexFace faceInCell[numOfNewFacesInCell];
	int cellIndex = 0;
	std::vector<HexCell>::iterator cellIter = m_pHexModel->cells.begin();
	for (; cellIter != m_pHexModel->cells.end(); ++ cellIter, ++ cellIndex) {

		int cellPointIndex = cellIndex + newVertices.size() + edgePoints.size() + facePoints.size();

		int facePointIndexInCell[6];
		SetIntIter = cellIter->faceIndices.begin();
		for (i = 0;
			SetIntIter != cellIter->faceIndices.end() && i < 6; 
			++ SetIntIter, ++ i) {
				facePointIndexInCell[i] = *SetIntIter + newVertices.size() + edgePoints.size();
		}

		int edgePointIndexInCell[12];
		SetIntIter = cellIter->edgeIndices.begin();
		for (i = 0;
			SetIntIter != cellIter->edgeIndices.end() && i < 12; 
			++ SetIntIter, ++ i) {
				edgePointIndexInCell[i] = *SetIntIter + newVertices.size();
		}

		for (i = 0; i < numOfNewFacesInCell; ++ i) {

			faceInCell[i].vertexIndices.insert(cellPointIndex);
			faceInCell[i].vertexIndices.insert(edgePointIndexInCell[i]);

			int facePointIndexInIthCell[2];
			int k = 0;

			SetIntIter = pNewModel->vertices[edgePointIndexInCell[i]].edgeIndices.begin();
			for (; SetIntIter != pNewModel->vertices[edgePointIndexInCell[i]].edgeIndices.end();
				++ SetIntIter) {

					for (j = 0; j < 6; ++ j) {

						if (pNewModel->edges[*SetIntIter].vertexIndices[0] == facePointIndexInCell[j] ||
							pNewModel->edges[*SetIntIter].vertexIndices[1] == facePointIndexInCell[j]) {
								if (k < 2) 
									facePointIndexInIthCell[k++] = facePointIndexInCell[j];
								faceInCell[i].vertexIndices.insert(facePointIndexInCell[j]);

								faceInCell[i].edgeIndices.insert(*SetIntIter);
						}
					}

			} 

			newFaceIndex = i + numOfNewFacesInCell * cellIndex + numOfNewFacesInFace * faceIndex;

			SetIntIter = faceInCell[i].vertexIndices.begin();
			for (; SetIntIter != faceInCell[i].vertexIndices.end(); ++ SetIntIter) {
				pNewModel->vertices[*SetIntIter].faceIndices.insert(newFaceIndex);
			}
			// edges in face
			SetIntIter = pNewModel->vertices[cellPointIndex].edgeIndices.begin();
			for (; SetIntIter != pNewModel->vertices[cellPointIndex].edgeIndices.end(); ++ SetIntIter) {
				for (k = 0; k < 2; ++ k) {
					if (pNewModel->edges[*SetIntIter].vertexIndices[0] == facePointIndexInIthCell[k] ||
						pNewModel->edges[*SetIntIter].vertexIndices[1] == facePointIndexInIthCell[k])
						faceInCell[i].edgeIndices.insert(*SetIntIter);
				}
			}

			SetIntIter = faceInCell[i].edgeIndices.begin();
			for (; SetIntIter != faceInCell[i].edgeIndices.end(); ++ SetIntIter) {
				pNewModel->edges[*SetIntIter].faceIndices.insert(newFaceIndex);
			}

			pNewModel->faces.push_back(faceInCell[i]);

			faceInCell[i].vertexIndices.clear();
			faceInCell[i].edgeIndices.clear();

		}
	}
}
/************************************************************************/
/*  new cell link                                                       */
/************************************************************************/
void Subdivision::NewCellList(HexModel* pNewModel) {

	int_set_iter SetIntIter;
	int i, j;

	int newCellIndex;
	const int numOfNewCellsInCell = 8;
	HexCell cellInCell[numOfNewCellsInCell];
	int cellIndex = 0;
	std::vector<HexCell>::iterator cellIter = m_pHexModel->cells.begin();
	for (; cellIter != m_pHexModel->cells.end(); ++ cellIter, ++ cellIndex) {

		int cellPointIndex = cellIndex + newVertices.size() + edgePoints.size() + facePoints.size();

		int vertexIndexInCell[8];
		SetIntIter = cellIter->vertexIndices.begin();
		for (i = 0; 
			SetIntIter != cellIter->vertexIndices.end() && i < 8;
			++ SetIntIter, ++ i) {
				vertexIndexInCell[i] = *SetIntIter;
		}

		int edgePointIndexInCell[12];
		SetIntIter = cellIter->edgeIndices.begin();
		for (i = 0; 
			SetIntIter != cellIter->edgeIndices.end() && i < 12;
			++ SetIntIter, ++ i) {
				edgePointIndexInCell[i] = *SetIntIter + newVertices.size();
		}

		int facePointIndexInCell[6];
		SetIntIter = cellIter->faceIndices.begin();
		for (i = 0; 
			SetIntIter != cellIter->faceIndices.end() && i < 6; 
			++ SetIntIter, ++ i) {
				facePointIndexInCell[i] = *SetIntIter + newVertices.size() + edgePoints.size();
		}

		for (i = 0; i < numOfNewCellsInCell; ++ i) {

			// vertex
			cellInCell[i].vertexIndices.insert(cellPointIndex);
			cellInCell[i].vertexIndices.insert(vertexIndexInCell[i]);

			int k = 0;
			int edgePointIndexInIthCell[3];
			SetIntIter = pNewModel->vertices[vertexIndexInCell[i]].edgeIndices.begin();
			for (; SetIntIter != pNewModel->vertices[vertexIndexInCell[i]].edgeIndices.end(); 
				++ SetIntIter)
			{
				for (j = 0; j < 12; ++ j) 
				{
					if (pNewModel->edges[*SetIntIter].vertexIndices[0] == edgePointIndexInCell[j] ||
						pNewModel->edges[*SetIntIter].vertexIndices[1] == edgePointIndexInCell[j])
					{
						if (k < 3)
							edgePointIndexInIthCell[k++] = edgePointIndexInCell[j];

						cellInCell[i].vertexIndices.insert(edgePointIndexInCell[j]);

						cellInCell[i].edgeIndices.insert(*SetIntIter);
					}
				}
			}

			k = 0;
			int facePointIndexInIthCell[3];
			SetIntIter = pNewModel->vertices[vertexIndexInCell[i]].faceIndices.begin();
			for (; SetIntIter != pNewModel->vertices[vertexIndexInCell[i]].faceIndices.end();
				++ SetIntIter)
			{
				for (j = 0; j < 6; ++ j) 
				{
					int_set_iter vertOfFaceIter = pNewModel->faces[*SetIntIter].vertexIndices.begin();
					for (; vertOfFaceIter != pNewModel->faces[*SetIntIter].vertexIndices.end(); 
						++ vertOfFaceIter)
					{
						if (*vertOfFaceIter == facePointIndexInCell[j]) 
						{
							if (k < 3) 
								facePointIndexInIthCell[k++] = facePointIndexInCell[j];

							cellInCell[i].vertexIndices.insert(facePointIndexInCell[j]);

							cellInCell[i].faceIndices.insert(*SetIntIter);

						}
					}
				}
			}

			newCellIndex = i + numOfNewCellsInCell * cellIndex;

			SetIntIter = cellInCell[i].vertexIndices.begin();
			for (; SetIntIter != cellInCell[i].vertexIndices.end(); ++ SetIntIter) {
				pNewModel->vertices[*SetIntIter].cellIndices.insert(newCellIndex);
			}

			// edge
			for (k = 0; k < 3; ++ k) {
				SetIntIter = pNewModel->vertices[facePointIndexInIthCell[k]].edgeIndices.begin();
				for (; SetIntIter != pNewModel->vertices[facePointIndexInIthCell[k]].edgeIndices.end();
					++ SetIntIter) {
						for (j = 0; j < 3; ++ j) {
							if (pNewModel->edges[*SetIntIter].vertexIndices[0] == edgePointIndexInIthCell[j] ||
								pNewModel->edges[*SetIntIter].vertexIndices[1] == edgePointIndexInIthCell[j]) {
									cellInCell[i].edgeIndices.insert(*SetIntIter);
							}
						}

						if (pNewModel->edges[*SetIntIter].vertexIndices[0] == cellPointIndex ||
							pNewModel->edges[*SetIntIter].vertexIndices[1] == cellPointIndex) {
								cellInCell[i].edgeIndices.insert(*SetIntIter);
						}
				}
			}

			SetIntIter = cellInCell[i].edgeIndices.begin();
			for (; SetIntIter != cellInCell[i].edgeIndices.end(); ++ SetIntIter) {
				pNewModel->edges[*SetIntIter].cellIndices.insert(newCellIndex);
			}

			// face
			SetIntIter = pNewModel->vertices[cellPointIndex].faceIndices.begin();
			for (; SetIntIter != pNewModel->vertices[cellPointIndex].faceIndices.end(); ++ SetIntIter) {
				int_set_iter vertOfFaceIter = pNewModel->faces[*SetIntIter].vertexIndices.begin();
				for (; vertOfFaceIter != pNewModel->faces[*SetIntIter].vertexIndices.end(); 
					++ vertOfFaceIter) {
						for (j = 0; j < 3; ++ j) {
							if (*vertOfFaceIter == edgePointIndexInIthCell[j]) {
								cellInCell[i].faceIndices.insert(*SetIntIter);
							}
						}
				}
			}

			SetIntIter = cellInCell[i].faceIndices.begin();
			for (; SetIntIter != cellInCell[i].faceIndices.end(); ++ SetIntIter) {
				if (pNewModel->faces[*SetIntIter].cellIndices[0] == -1) 
					pNewModel->faces[*SetIntIter].cellIndices[0] = newCellIndex;
				else if (pNewModel->faces[*SetIntIter].cellIndices[1] == -1) {
					pNewModel->faces[*SetIntIter].cellIndices[1] = newCellIndex;
				}
			}

			pNewModel->cells.push_back(cellInCell[i]);

			cellInCell[i].vertexIndices.clear();
			cellInCell[i].edgeIndices.clear();
			cellInCell[i].faceIndices.clear();

		}

	}
}

/************************************************************************/
/*  Hexahedron Approximating Subdivision                                */
/************************************************************************/
void Subdivision::ApproximateSubdivision() {

	CalcCellPoints();
	CalcFacePoints();
	CalcEdgePoints();
	CalcNewVertices();
	
	// new model
	HexModel* pNewModel = new HexModel;

	NewVertexList(pNewModel);
	NewEdgeList(pNewModel);
	NewFaceList(pNewModel);
	NewCellList(pNewModel);

	FreeMemory();

	delete m_pHexModel;
	m_pHexModel = pNewModel;

	std::cerr << std::endl;
	std::cerr << "####### subdivision counter: " << ++subdivCounter << " #######" << std::endl;
	std::cerr << "#vertices: " << m_pHexModel->vertices.size() << std::endl;
	std::cerr << "#edges:    " << m_pHexModel->edges.size() << std::endl;
	std::cerr << "#faces:    " << m_pHexModel->faces.size() << std::endl;
	std::cerr << "#cells:    " << m_pHexModel->cells.size() << std::endl;
}


/************************************************************************/
/*  Hexahedron Interpolating Subdivision                                */
/************************************************************************/
void Subdivision::InterpolateSubdivision() {

	CalcCellPoints();
	CalcFacePointsInter();
	//CalcFacePoints();
	CalcEdgePointsInter();
	CalcNewVerticesInter();

	// new model
	HexModel* pNewModel = new HexModel;

	NewVertexList(pNewModel);
	NewEdgeList(pNewModel);
	NewFaceList(pNewModel);
	NewCellList(pNewModel);

	FreeMemory();

	delete m_pHexModel;
	m_pHexModel = pNewModel;

	std::cerr << std::endl;
	std::cerr << "####### subdivision counter: " << ++subdivCounter << " #######" << std::endl;
	std::cerr << "#vertices: " << m_pHexModel->vertices.size() << std::endl;
	std::cerr << "#edges:    " << m_pHexModel->edges.size() << std::endl;
	std::cerr << "#faces:    " << m_pHexModel->faces.size() << std::endl;
	std::cerr << "#cells:    " << m_pHexModel->cells.size() << std::endl;
}
