/**
 * @file Connections.h
 * 
 * @ingroup Simulator/Connections
 *
 * @brief The base class of all connections classes
 *
 * The Connections class is the manager class for Edges and is managed by Model. 
 * It is responsible for the creation and deletion of edges.
 * 
 * In graph-based networks, vertices are connected through edges where messages are exchanged.
 * The strength of connections is characterized by edge's weight. 
 * The connections classes define topologies, the way to connect vertices,  
 * and dynamics, the way to change connections as time elapses, of the networks. 
 * 
 * Connections can be either static or dynamic. The static connectons are ones where
 * connections are established at initialization and never change. 
 * The dynamic connections can be changed as the networks evolve, so in the dynamic networks'
 * edges will be created, deleted, or their weight will be modifed.  
 *
 * Connections classes may maintain intra-epoch states of connections in the network. 
 * This includes history and parameters that inform how new connections are made during growth.
 * Therefore, connections classes will have customized recorder classes, and provide
 * a function to create the recorder class.
 */

#pragma once

#include <memory>

#include <log4cplus/loggingmacros.h>

#include "AllVertices.h"
#include "AllEdges.h"
#include "AllSpikingNeurons.h"
#include "AllSpikingSynapses.h"
#include "Layout.h"
#include "IRecorder.h"
#include "EdgeIndexMap.h"

using namespace std;

class Connections {
public:
   Connections();

   ///  Destructor
   virtual ~Connections();

   /// Returns shared pointer to edges 
   shared_ptr<AllEdges> getEdges() const;


   /// Returns a shared pointer to the EdgeIndexMap
   shared_ptr<EdgeIndexMap> getEdgeIndexMap() const;

   /// Calls Edges to create EdgeIndexMap and stores it as a member variable
   void createEdgeIndexMap();

   ///  Setup the internal structure of the class (allocate memories and initialize them).
   ///
   ///  @param  layout    Layout information of the neural network.
   ///  @param  vertices   The vertex list to search from.
   ///  @param  edges  The edge list to search from.
   virtual void setupConnections(Layout *layout, AllVertices *vertices, AllEdges *edges) = 0;

   /// Load member variables from configuration file.
   /// Registered to OperationManager as Operations::op::loadParameters
   virtual void loadParameters() = 0;

   ///  Prints out all parameters to logging file.
   ///  Registered to OperationManager as Operation::printParameters
   virtual void printParameters() const = 0;
   
   ///  Update the connections status in every epoch.
   ///
   ///  @param  vertices  The vertex list to search from.
   ///  @param  layout   Layout information of the neural network.
   ///  @return true if successful, false otherwise.
   virtual bool updateConnections(AllVertices &vertices, Layout *layout);

   ///  Creates edges from edge weights saved in the serialization file.
   ///
   ///  @param  numVertices Number of vertices to update.
   ///  @param  layout      Layout information of the neural network.
   ///  @param  vertices    The vertex list to search from.
   ///  @param  iedges   The Edge list to search from.
   void createEdgesFromWeights(const int numVertices, Layout *layout, AllVertices &vertices, AllEdges &edges);

#if defined(USE_GPU)
   public:
       ///  Update the weight of the Edges in the simulation.
       ///  Note: Platform Dependent.
       ///
       ///  @param  numVertices          number of vertices to update.
       ///  @param  vertices             the Neuron list to search from.
       ///  @param  edges            the Edge list to search from.
       ///  @param  allVerticesDevice    GPU address of the allVertices struct on device memory.
       ///  @param  allEdgesDevice   GPU address of the allEdges struct on device memory.
       ///  @param  layout              Layout information of the neural network.
       virtual void updateEdgesWeights(const int numVertices, AllVertices &vertices, AllEdges &edges, AllSpikingNeuronsDeviceProperties* allVerticesDevice, AllSpikingSynapsesDeviceProperties* allEdgesDevice, Layout *layout);
#else
public:
   ///  Update the weight of the Edges in the simulation.
   ///  Note: Platform Dependent.
   ///
   ///  @param  numVertices Number of vertices to update.
   ///  @param  ineurons    The Neuron list to search from.
   ///  @param  iedges   The Edge list to search from.
   virtual void
   updateEdgesWeights(const int numVertices, AllVertices &vertices, AllEdges &edges, Layout *layout);

#endif // USE_GPU

protected:

   shared_ptr<AllEdges> edges_;

   shared_ptr<EdgeIndexMap> edgeIndexMap_;

   log4cplus::Logger fileLogger_;
   log4cplus::Logger edgeLogger_;
};

