/**
 * @file DynamicLayout.h
 * 
 * @ingroup Simulator/Layouts
 *
 * @brief The DynamicLayout class defines the layout of vertices in neural networks
 *
 * The DynamicLayout class maintains vertices locations (x, y coordinates), 
 * distance of every couple vertices,
 * vertices type map (distribution of excitatory and inhibitory vertices), and starter vertices map
 * (distribution of endogenously active vertices).  
 *
 * The DynamicLayout class generates layout information dynamically.
 */

#pragma once

#include "Layout.h"

using namespace std;

class DynamicLayout : public Layout {
public:
   DynamicLayout();

   virtual ~DynamicLayout();

   static Layout *Create() { return new DynamicLayout(); }

   ///  Prints out all parameters to logging file.
   ///  Registered to OperationManager as Operation::printParameters
   virtual void printParameters() const override;

   ///  Creates a randomly ordered distribution with the specified numbers of vertex types.
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
   virtual void loadParameters(); 

   /// Returns the type of edge at the given coordinates
   /// @param    srcVertex  source vertex index
   /// @param    destVertex destination vertex index
   /// @return type of the edge.
   virtual edgeType edgType(const int srcVertex, const int destVertex);

private:
   /// Fraction of endogenously active vertices.
   BGFLOAT fractionEndogenouslyActive_;

   /// Fraction of exitatory vertices.
   BGFLOAT fractionExcitatory_;
};

