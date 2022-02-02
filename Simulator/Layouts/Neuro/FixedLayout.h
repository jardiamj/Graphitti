/**
 * @file FixedLayout.h
 * 
 * @ingroup Simulator/Layouts
 *
 * @brief The Layout class defines the layout of vertices in neural networks
 *
 * The FixedLayout class maintains vertices locations (x, y coordinates), 
 * distance of every couple vertices,
 * vertices type map (distribution of excitatory and inhibitory vertices), and starter vertices map
 * (distribution of endogenously active vertices).  
 *
 * The FixedLayout class reads all layout information from parameter description file.
 */

#pragma once

#include "Layout.h"

using namespace std;

class FixedLayout : public Layout {
public:
   FixedLayout();

   virtual ~FixedLayout();

   static Layout *Create() { return new FixedLayout(); }

   ///  Prints out all parameters to logging file.
   ///  Registered to OperationManager as Operation::printParameters
   virtual void printParameters() const override;

   ///  Creates a vertex type map.
   ///
   ///  @param  numVertices number of the vertices to have in the type map.
   virtual void generateVertexTypeMap(int numVertices) override;

   ///  Populates the starter map.
   ///  Selects num_endogenously_active_neurons excitory vertices
   ///  and converts them into starter vertices.
   ///
   ///  @param  numVertices number of vertices to have in the map.
   virtual void initStarterMap(const int numVertices) override;

   /// Load member variables from configuration file. Registered to OperationManager as Operation::loadParameters
   virtual void loadParameters() override;

   /// Returns the type of edge at the given coordinates
   /// @param    srcVertex  source vertex index
   /// @param    destVertex destination vertex index
   /// @return type of the edge.
   virtual edgeType edgType(const int srcVertex, const int destVertex) override;
};

