/**
 * @file Connections.cpp
 * 
 * @ingroup Simulator/Connections
 * 
 * @brief Methods for creating and updating connections
 * 
 * Below all of the resources for the various
 * connections are instantiated and initialized.
 * All of the allocation for memory is done in the
 * constructor’s parameters and not in the body of
 * the function. Once all memory has been allocated
 * the constructor fills in known information
 * into “radii” and “rates”.
 *  
 */

#include "Connections.h"
#include "AllEdges.h"
#include "AllVertices.h"
#include "OperationManager.h"
#include "ParameterManager.h"
#include "EdgesFactory.h"

Connections::Connections() {
   // Create Edges class using type definition in configuration file
   string type;
   ParameterManager::getInstance().getStringByXpath("//EdgesParams/@class", type);
   edges_ = EdgesFactory::getInstance()->createEdges(type);

   // Register printParameters function as a printParameters operation in the OperationManager
   function<void()> printParametersFunc = bind(&Connections::printParameters, this);
   OperationManager::getInstance().registerOperation(Operations::printParameters, printParametersFunc);

   // Register loadParameters function with Operation Manager
   function<void()> function = std::bind(&Connections::loadParameters, this);
   OperationManager::getInstance().registerOperation(Operations::op::loadParameters, function);

   // Get a copy of the file logger to use log4cplus macros
   fileLogger_ = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("file"));
   edgeLogger_ = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("edge"));
}

Connections::~Connections() {
}

shared_ptr<AllEdges> Connections::getEdges() const {
   return edges_;
}

shared_ptr<EdgeIndexMap> Connections::getEdgeIndexMap() const {
   return edgeIndexMap_;
}

void Connections::createEdgeIndexMap() {
   Simulator& simulator = Simulator::getInstance();
   int vertexCount = simulator.getTotalVertices();
   int maxEdges = vertexCount * edges_->maxEdgesPerVertex_;

   if (edgeIndexMap_ == nullptr) {
      edgeIndexMap_ = shared_ptr<EdgeIndexMap>(new EdgeIndexMap(vertexCount, maxEdges));
   }

   fill_n(edgeIndexMap_->incomingEdgeBegin_, vertexCount, 0);
   fill_n(edgeIndexMap_->incomingEdgeCount_, vertexCount, 0);
   fill_n(edgeIndexMap_->incomingEdgeIndexMap_, maxEdges, 0);
   fill_n(edgeIndexMap_->outgoingEdgeBegin_, vertexCount, 0);
   fill_n(edgeIndexMap_->outgoingEdgeCount_, vertexCount, 0);
   fill_n(edgeIndexMap_->outgoingEdgeIndexMap_, maxEdges, 0);

   edges_->createEdgeIndexMap(edgeIndexMap_);
}

///  Update the connections status in every epoch.
///
///  @param  vertices  The vertex list to search from.
///  @param  layout   Layout information of the neural network.
///  @return true if successful, false otherwise.
bool Connections::updateConnections(AllVertices &vertices, Layout *layout) {
   return false;
}

#if defined(USE_GPU)
void Connections::updateEdgesWeights(const int numVertices, AllVertices &vertices, AllEdges &edges, AllSpikingNeuronsDeviceProperties* allVerticesDevice, AllSpikingSynapsesDeviceProperties* allEdgesDevice, Layout *layout)
{
}
#else

///  Update the weight of the edges in the simulation.
///  Note: Platform Dependent.
///
///  @param  numVertices  Number of vertices to update.
///  @param  vertices     The vertex list to search from.
///  @param  edges    The Edge list to search from.
void Connections::updateEdgesWeights(const int numVertices, AllVertices &vertices, AllEdges &edges, Layout *layout) {
}

#endif // !USE_GPU

///  Creates edges from edge weights saved in the serialization file.
///
///  @param  numVertices  Number of vertices to update.
///  @param  layout      Layout information of the neural network.
///  @param  ivertices    The vertex list to search from.
///  @param  iedges   The Edge list to search from.
void Connections::createEdgesFromWeights(const int numVertices, Layout *layout, AllVertices &vertices,
                                            AllEdges &edges) {
   // for each vertex
   for (int i = 0; i < numVertices; i++) {
      // for each edge in the vertex
      for (BGSIZE edgeIndex = 0;
           edgeIndex < Simulator::getInstance().getMaxEdgesPerVertex(); edgeIndex++) {
         BGSIZE iEdg = Simulator::getInstance().getMaxEdgesPerVertex() * i + edgeIndex;
         // if the edge weight is not zero (which means there is a connection), create the edge
         if (edges.W_[iEdg] != 0.0) {
            BGFLOAT theW = edges.W_[iEdg];
            BGFLOAT *sumPoint = &(vertices.summationMap_[i]);
            int srcVertex = edges.sourceVertexIndex_[iEdg];
            int destVertex = edges.destVertexIndex_[iEdg];
            edgeType type = layout->edgType(srcVertex, destVertex);
            edges.edgeCounts_[i]++;
            edges.createEdge(iEdg, srcVertex, destVertex, sumPoint, Simulator::getInstance().getDeltaT(),
                                   type);
            edges.W_[iEdg] = theW;
         }
      }
   }
}



