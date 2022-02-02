/**
 *      @file XmlSTDPRecorder.h
 *
 *       @ingroup Simulator/Recorders
 *
 *       @brief An implementation for recording spikes history in an XML file for spike timining dependent plasticity simulations
 */

/**
 ** \class XmlGrowthRecorder XmlGrowthRecorder.h "XmlGrowthRecorder.h"
 **
 ** \latexonly  \subsubsection*{Implementation} \endlatexonly
 ** \htmlonly   <h3>Implementation</h3> \endhtmlonly
 **
 ** The XmlGrowthRecorder provides a mechanism for recording vertex's layout, spikes history,
 ** and compile history information on xml file:
 **     -# vertex's locations, and type map,
 **     -# individual vertex's spike rate in epochs,
 **     -# network wide burstiness index data in 1s bins,
 **     -# network wide spike count in 10ms bins,
 **     -# individual vertex's radius history of every epoch.
 **
 ** \latexonly  \subsubsection*{Credits} \endlatexonly
 ** \htmlonly   <h3>Credits</h3> \endhtmlonly
 **
 ** Some models in this simulator is a rewrite of CSIM (2006) and other 
 ** work (Stiber and Kawasaki (2007?))
 **
 **
 **     @author Fumitaka Kawasaki, Snigdha Singh
 **/

#pragma once

#include <fstream>

#include "XmlRecorder.h"
#include "Model.h"

class XmlSTDPRecorder : public XmlRecorder {
public:
   //! THe constructor and destructor
   XmlSTDPRecorder();

   ~XmlSTDPRecorder();

   static IRecorder* Create() { return new XmlSTDPRecorder(); }

   /**
    * Init radii and rates history matrices with default values
    */
   virtual void initDefaultValues();

   /**
    * Init radii and rates history matrices with current radii and rates
    */
   virtual void initValues();

   /**
    * Get the current radii and rates vlaues
    */
   virtual void getValues();

   /**
    * Compile history information in every epoch
    *
    * @param[in] vertices   The entire list of vertices.
    */
   virtual void compileHistories(AllVertices &vertices);

   /**
    * Writes simulation results to an output destination.
    *
    * @param  vertices the Neuron list to search from.
    **/
   virtual void saveSimData(const AllVertices &vertices);

   /**
    *  Prints out all parameters to logging file.
    *  Registered to OperationManager as Operation::printParameters
    */
   virtual void printParameters();

   virtual string toXML(string name,vector<vector<BGFLOAT>> MatrixToWrite ) const;
   virtual string toXML(string name,vector<vector<int>> MatrixToWrite) const;

protected:
   vector<vector<BGFLOAT>> weightsHistory_;
   vector<vector<int>> sourceNeuronIndexHistory_;
   vector<vector<int>>  destNeuronIndexHistory_;

};

