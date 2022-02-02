/**
 * @file AllSpikingNeurons.h
 * 
 * @ingroup Simulator/Vertices
 *
 * @brief A container of all spiking neuron data
 * 
 * This is the base class of all spiking neuron classes.
 *
 * The class uses a data-centric structure, which utilizes a structure as the containers of
 * all vertices.
 *
 * The container holds neuron parameters of all vertices.
 * Each kind of vertex parameter is stored in a 1D array, of which length
 * is number of all vertices. Each array of a vertex parameter is pointed by a
 * corresponding member variable of the vertex parameter in the class.
 * 
 * This structure was originally designed for the GPU implementation of the
 * simulator, and this refactored version of the simulator simply uses that design for
 * all other implementations as well. This is to simplify transitioning from
 * single-threaded to multi-threaded.
 */

#pragma once

using namespace std;

#include "Global.h"
#include "AllVertices.h"
#include "AllSpikingSynapses.h"

struct AllSpikingNeuronsDeviceProperties;

class AllSpikingNeurons : public AllVertices {
public:
   AllSpikingNeurons();

   virtual ~AllSpikingNeurons();

   ///  Setup the internal structure of the class.
   ///  Allocate memories to store all vertices' state.
   virtual void setupVertices() override;

   ///  Clear the spike counts out of all Vertices.
   void clearSpikeCounts();

#if defined(USE_GPU)
   public:
       ///  Set some parameters used for advanceVerticesDevice.
       ///
       ///  @param  edges               Reference to the allEdges struct on host memory.
       virtual void setAdvanceVerticesDeviceParams(AllEdges &edges);

       ///  Copy spike counts data stored in device memory to host.
       ///
       ///  @param  allVerticesDevice   GPU address of the allVertices struct on device memory.
       virtual void copyNeuronDeviceSpikeCountsToHost( void* allVerticesDevice) = 0;

       ///  Copy spike history data stored in device memory to host.
       ///
       ///  @param  allVerticesDevice   GPU address of the allVertices struct on device memory.
       virtual void copyNeuronDeviceSpikeHistoryToHost( void* allVerticesDevice) = 0;

       ///  Clear the spike counts out of all vertices.
       ///
       ///  @param  allVerticesDevice   GPU address of the allVertices struct on device memory.
       virtual void clearNeuronSpikeCounts( void* allVerticesDevice) = 0;

   protected:
       ///  Copy spike history data stored in device memory to host.
       ///  (Helper function of copyNeuronDeviceSpikeHistoryToHost)
       ///
       ///  @param  allVerticesDevice   GPU address of the allVertices struct on device memory.
       void copyDeviceSpikeHistoryToHost( AllSpikingNeuronsDeviceProperties& allVerticesDevice);

       ///  Copy spike counts data stored in device memory to host.
       ///  (Helper function of copyNeuronDeviceSpikeCountsToHost)
       ///
       ///  @param  allVerticesDevice   GPU address of the allVertices struct on device memory.
       void copyDeviceSpikeCountsToHost( AllSpikingNeuronsDeviceProperties& allVerticesDevice);

       ///  Clear the spike counts out of all vertices in device memory.
       ///  (helper function of clearNeuronSpikeCounts)
       ///
       ///  @param  allVerticesDevice   GPU address of the allVertices struct on device memory.
       void clearDeviceSpikeCounts( AllSpikingNeuronsDeviceProperties& allVerticesDevice);
#else // !defined(USE_GPU)

public:
   ///  Update internal state of the indexed Neuron (called by every simulation step).
   ///  Notify outgoing edges if vertex has fired.
   ///
   ///  @param  edges         The Edge list to search from.
   ///  @param  edgeIndexMap  Reference to the EdgeIndexMap.
   virtual void advanceVertices(AllEdges &edges, const EdgeIndexMap *edgeIndexMap);

   /// Get the spike history of vertex[index] at the location offIndex.
   /// More specifically, retrieves the global simulation time step for the spike
   /// in question from the spike history record.
   ///
   /// @param  index            neuron index to get spike history.
   /// @param  offIndex         Offset of the history buffer to get from.
   uint64_t getSpikeHistory(int index, int offIndex);

protected:
   ///  Helper for #advanceNeuron. Updates state of a single vertex.
   ///
   ///  @param  index            neuron index to update.
   virtual void advanceNeuron(const int index) = 0;

   ///  Initiates a firing of a vertex to connected vertices
   ///
   ///  @param  index            neuron index to fire.
   virtual void fire(const int index) const;

#endif // defined(USE_GPU)

public:
   ///  The booleans which track whether the vertex has fired.
   bool *hasFired_;

   ///  The number of spikes since the last growth cycle.
   int *spikeCount_;

   ///  Offset of the spike_history buffer.
   int *spikeCountOffset_;

   ///  Step count (history) for each spike fired by each vertex.
   ///  The step counts are stored in a buffer for each vertex, and the pointers
   ///  to the buffer are stored in a list pointed by spike_history.
   ///  Each buffer is a circular, and offset of top location of the buffer i is
   ///  specified by spikeCountOffset[i].
   uint64_t **spikeHistory_;

protected:
   ///  True if back propagaion is allowed.
   ///  (parameters used for advanceVerticesDevice.)
   bool fAllowBackPropagation_;

};

#if defined(USE_GPU)
struct AllSpikingNeuronsDeviceProperties : public AllVerticesDeviceProperties
{
        ///  The booleans which track whether the vertex has fired.
        bool *hasFired_;

        ///  The number of spikes since the last growth cycle.
        int *spikeCount_;

        ///  Offset of the spike_history buffer.
        int *spikeCountOffset_;

        ///  Step count (history) for each spike fired by each vertex.
        ///  The step counts are stored in a buffer for each vertex, and the pointers
        ///  to the buffer are stored in a list pointed by spike_history. 
        ///  Each buffer is a circular, and offset of top location of the buffer i is
        ///  specified by spikeCountOffset[i].
        uint64_t **spikeHistory_;
};
#endif // defined(USE_GPU)
