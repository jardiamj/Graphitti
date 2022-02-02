/**
 * @file XmlGrowthRecorder.h
 * 
 * @ingroup Simulator/Recorders
 *
 * @brief Header file for XmlGrowthRecorder.h
 *
 * The XmlGrowthRecorder provides a mechanism for recording vertex's layout, spikes history,
 * and compile history information on xml file:
 *     -# vertex's locations, and type map,
 *     -# individual vertex's spike rate in epochs,
 *     -# network wide burstiness index data in 1s bins,
 *     -# network wide spike count in 10ms bins,
 *     -# individual vertex's radius history of every epoch.
 *
 */

#pragma once

#include <fstream>

#include "XmlRecorder.h"
#include "Model.h"

class XmlGrowthRecorder : public XmlRecorder {
public:
   /// THe constructor and destructor
   XmlGrowthRecorder();

   ~XmlGrowthRecorder();

   static IRecorder* Create() { return new XmlGrowthRecorder(); }

   /// Init radii and rates history matrices with default values
   virtual void initDefaultValues() override;

   /// Init radii and rates history matrices with current radii and rates
   virtual void initValues() override;

   /// Get the current radii and rates vlaues
   virtual void getValues() override;

   /// Compile history information in every epoch
   ///
   /// @param[in] vertices   The entire list of vertices.
   virtual void compileHistories(AllVertices &vertices) override;

   /// Writes simulation results to an output destination.
   ///
   /// @param  vertices the Neuron list to search from.
   virtual void saveSimData(const AllVertices &vertices) override;

   ///  Prints out all parameters to logging file.
   ///  Registered to OperationManager as Operation::printParameters
   virtual void printParameters() override;

private:
   
   // TODO: There seems to be multiple copies of this in different classes...
   void getStarterNeuronMatrix(VectorMatrix &matrix, const bool *starterMap);

   // track firing rate
   CompleteMatrix ratesHistory_;

   // track radii
   CompleteMatrix radiiHistory_;

};

