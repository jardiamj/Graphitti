/**
 * @file Layout.h
 * 
 * @ingroup Simulator/Layouts
 *
 * @brief The Layout class defines the layout of vertices in graph-based networks
 * 
 * The Layout class is the manager class for Vertices and is managed by Model. 
 * The Layout class maintains vertices locations (x, y coordinates),
 * distance of every couple vertices,
 * vertices type map,
 * and starter vertices map
 * 
 */

#pragma once

#include <iostream>
#include <memory>
#include <vector>

#include <log4cplus/loggingmacros.h>

#include "Utils/Global.h"
#include "AllVertices.h"

using namespace std;

class AllVertices;

class Layout {
public:
   Layout();

   virtual ~Layout();

   shared_ptr<AllVertices> getVertices() const;

   /// Setup the internal structure of the class.
   /// Allocate memories to store all layout state.
   virtual void setupLayout();

   /// Load member variables from configuration file. Registered to OperationManager as Operation::loadParameters
   virtual void loadParameters() = 0;

   /// Prints out all parameters to logging file. Registered to OperationManager as Operation::printParameters
   virtual void printParameters() const;

   /// Creates a vertices type map.
   /// @param  numVertices number of the vertices to have in the type map.
   virtual void generateVertexTypeMap(int numVertices);

   /// Populates the starter map.
   /// Selects num_endogenously_active_vertices excitory vertices
   /// and converts them into starter vertices.
   /// @param  numVertices number of vertices to have in the map.
   virtual void initStarterMap(const int numVertices);

   /// Returns the type of edge at the given coordinates
   /// @param    srcVertex  source vertex index
   /// @param    destVertex destination vertex index
   /// @return type of the edge.
   virtual edgeType edgType(const int srcVertex, const int destVertex) = 0;

   VectorMatrix *xloc_;  ///< Store vertex i's x location.

   VectorMatrix *yloc_;   ///< Store vertex i's y location.

   CompleteMatrix *dist2_;  ///< Inter-vertex distance squared.

   CompleteMatrix *dist_;    ///< The true inter-vertex distance.

   vector<int> probedNeuronList_;   ///< Probed vertices list. // ToDo: Move this to Hdf5 recorder once its implemented in project -chris

   vertexType *vertexTypeMap_;    ///< The vertex type map.

   bool *starterMap_; ///< The starter existence map (T/F).

   BGSIZE numEndogenouslyActiveNeurons_;    ///< Number of endogenously active vertices. TODO: move to lower level - tori

   BGSIZE numCallerVertices_;    ///< Number of caller vertices. TODO: move to lower level - tori


protected:
   shared_ptr<AllVertices> vertices_;

   vector<int> endogenouslyActiveNeuronList_;    ///< Endogenously active vertices list. TODO: move to lower level - tori

   vector<int> inhibitoryNeuronLayout_;    ///< Inhibitory vertices list. TODO: move to lower level - tori

   vector<int> callerVertexList_;    ///< Caller vertex list. TODO: move to lower level - tori

   vector<int> psapVertexList_;    ///< PSAP vertex list. TODO: move to lower level - tori
   
   vector<int> responderVertexList_;    ///< Responder vertex list. TODO: move to lower level - tori

   log4cplus::Logger fileLogger_;

private:
   /// initialize the location maps (xloc and yloc).
   void initVerticesLocs();

   bool gridLayout_;    ///< True if grid layout.

};

