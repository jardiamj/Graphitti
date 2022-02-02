/**
 * @file GPUModel.cu
 * 
 * @ingroup Simulator/Core
 *
 * @brief Implementation of Model for the spiking neural networks.
 * 
 */

#include "GPUModel.h"
#include "AllSynapsesDeviceFuncs.h"
#include "Connections.h"
#include "Global.h"
#include "AllVertices.h"
#include "AllEdges.h"

#ifdef PERFORMANCE_METRICS
float g_time;
cudaEvent_t start, stop;
#endif // PERFORMANCE_METRICS

__constant__ int d_debug_mask[1];

GPUModel::GPUModel() :
  Model::Model(),
  edgeIndexMapDevice_(nullptr),
  randNoise_d(nullptr),
  allVerticesDevice_(nullptr),
  allEdgesDevice_(nullptr)
{
}

GPUModel::~GPUModel() 
{
  //Let Model base class handle de-allocation
}

/// Allocates  and initializes memories on CUDA device.
/// @param[out] allVerticesDevice          Memory location of the pointer to the vertices list on device memory.
/// @param[out] allEdgesDevice         Memory location of the pointer to the edges list on device memory.
void GPUModel::allocDeviceStruct(void** allVerticesDevice, void** allEdgesDevice)
{
  // Get vertices and edges
  shared_ptr<AllVertices> vertices = layout_->getVertices();
  shared_ptr<AllEdges> edges = connections_->getEdges();

  // Allocate Vertices and Edges structs on GPU device memory
  vertices->allocNeuronDeviceStruct(allVerticesDevice);
  edges->allocEdgeDeviceStruct(allEdgesDevice);

  // Allocate memory for random noise array
  int numVertices = Simulator::getInstance().getTotalVertices();
  BGSIZE randNoise_d_size = numVertices * sizeof (float);	// size of random noise array
  HANDLE_ERROR( cudaMalloc ( ( void ** ) &randNoise_d, randNoise_d_size ) );

  // Copy host vertex and edge arrays into GPU device
  vertices->copyNeuronHostToDevice( *allVerticesDevice );
  edges->copyEdgeHostToDevice( *allEdgesDevice );

  // Allocate edge inverse map in device memory
  allocSynapseImap( numVertices );
}

/// Copies device memories to host memories and deallocates them.
/// @param[out] allVerticesDevice          Memory location of the pointer to the vertices list on device memory.
/// @param[out] allEdgesDevice         Memory location of the pointer to the edges list on device memory.
void GPUModel::deleteDeviceStruct(void** allVerticesDevice, void** allEdgesDevice)
{  
  // Get vertices and edges
  shared_ptr<AllVertices> vertices = layout_->getVertices();
  shared_ptr<AllEdges> edges = connections_->getEdges();

  // Copy device edge and vertex structs to host memory
  vertices->copyNeuronDeviceToHost( *allVerticesDevice);
  // Deallocate device memory
  vertices->deleteNeuronDeviceStruct( *allVerticesDevice);
  // Copy device edge and vertex structs to host memory
  edges->copyEdgeDeviceToHost( *allEdgesDevice);
  // Deallocate device memory
  edges->deleteEdgeDeviceStruct( *allEdgesDevice );
  HANDLE_ERROR( cudaFree( randNoise_d ) );
}

/// Sets up the Simulation.
void GPUModel::setupSim()
{
  // Set device ID
  HANDLE_ERROR( cudaSetDevice( g_deviceId ) );
  // Set DEBUG flag
  HANDLE_ERROR( cudaMemcpyToSymbol (d_debug_mask, &g_debug_mask, sizeof(int) ) );
  Model::setupSim();

  //initialize Mersenne Twister
  //assuming numVertices >= 100 and is a multiple of 100. Note rng_mt_rng_count must be <= MT_RNG_COUNT
  int rng_blocks = 25; //# of blocks the kernel will use
  int rng_nPerRng = 4; //# of iterations per thread (thread granularity, # of rands generated per thread)
  int rng_mt_rng_count = Simulator::getInstance().getTotalVertices() / rng_nPerRng; //# of threads to generate for numVertices rand #s
  int rng_threads = rng_mt_rng_count/rng_blocks; //# threads per block needed
  initMTGPU(Simulator::getInstance().getNoiseRngSeed(), rng_blocks, rng_threads, rng_nPerRng, rng_mt_rng_count);

#ifdef PERFORMANCE_METRICS
  cudaEventCreate( &start );
  cudaEventCreate( &stop );

  t_gpu_rndGeneration = 0.0;
  t_gpu_advanceNeurons = 0.0;
  t_gpu_advanceSynapses = 0.0;
  t_gpu_calcSummation = 0.0;
#endif // PERFORMANCE_METRICS

  // allocates memories on CUDA device
  allocDeviceStruct((void **)&allVerticesDevice_, (void **)&allEdgesDevice_);

  // copy inverse map to the device memory
  copyEdgeIndexMapHostToDevice(*(connections_->getEdgeIndexMap().get()), Simulator::getInstance().getTotalVertices());

  // set some parameters used for advanceVerticesDevice
  layout_->getVertices()->setAdvanceVerticesDeviceParams(*(connections_->getEdges().get()));

  // set some parameters used for advanceEdgesDevice
  connections_->getEdges()->setAdvanceEdgesDeviceParams();
}

/// Performs any finalization tasks on network following a simulation.
void GPUModel::finish()
{
  // deallocates memories on CUDA device
  deleteDeviceStruct((void**)&allVerticesDevice_, (void**)&allEdgesDevice_);
  deleteSynapseImap();

#ifdef PERFORMANCE_METRICS
  cudaEventDestroy( start );
  cudaEventDestroy( stop );
#endif // PERFORMANCE_METRICS
}

/// Advance everything in the model one time step. In this case, that
/// means calling all of the kernels that do the "micro step" updating
/// (i.e., NOT the stuff associated with growth).
void GPUModel::advance()
{
#ifdef PERFORMANCE_METRICS
  // Reset CUDA timer to start measurement of GPU operations
  cudaStartTimer();
#endif // PERFORMANCE_METRICS

  // Get vertices and edges
  shared_ptr<AllVertices> vertices = layout_->getVertices();
  shared_ptr<AllEdges> edges = connections_->getEdges();

  normalMTGPU(randNoise_d);

#ifdef PERFORMANCE_METRICS
  cudaLapTime(t_gpu_rndGeneration);
  cudaStartTimer();
#endif // PERFORMANCE_METRICS

  // display running info to console
  // Advance vertices ------------->
   dynamic_cast<AllSpikingNeurons *>(vertices.get())->advanceVertices(*(connections_->getEdges().get()), allVerticesDevice_, allEdgesDevice_, randNoise_d, edgeIndexMapDevice_);

#ifdef PERFORMANCE_METRICS
  cudaLapTime(t_gpu_advanceNeurons);
  cudaStartTimer();
#endif // PERFORMANCE_METRICS

  // Advance edges ------------->
  edges->advanceEdges(allEdgesDevice_, allVerticesDevice_, edgeIndexMapDevice_);

#ifdef PERFORMANCE_METRICS
  cudaLapTime(t_gpu_advanceSynapses);
  cudaStartTimer();
#endif // PERFORMANCE_METRICS

  // calculate summation point
  calcSummationMap();

#ifdef PERFORMANCE_METRICS
 cudaLapTime(t_gpu_calcSummation);
#endif // PERFORMANCE_METRICS
}

/// Add psr of all incoming edges to summation points.
void GPUModel::calcSummationMap()
{
  // CUDA parameters
  const int threadsPerBlock = 256;
  int blocksPerGrid = ( Simulator::getInstance().getTotalVertices() + threadsPerBlock - 1 ) / threadsPerBlock;

  calcSummationMapDevice <<< blocksPerGrid, threadsPerBlock >>> (
        Simulator::getInstance().getTotalVertices(), allVerticesDevice_, edgeIndexMapDevice_, allEdgesDevice_ );
}

/// Update the connection of all the Vertices and Edges of the simulation.
void GPUModel::updateConnections()
{
  // Get vertices and edges
  shared_ptr<AllVertices> vertices = layout_->getVertices();
  shared_ptr<AllEdges> edges = connections_->getEdges();

  dynamic_cast<AllSpikingNeurons*>(vertices.get())->copyNeuronDeviceSpikeCountsToHost(allVerticesDevice_);
  dynamic_cast<AllSpikingNeurons*>(vertices.get())->copyNeuronDeviceSpikeHistoryToHost(allVerticesDevice_);

  // Update Connections data
  if (connections_->updateConnections(*(vertices.get()), layout_.get())) {
    connections_->updateEdgesWeights(Simulator::getInstance().getTotalVertices(), *(vertices.get()), *(edges.get()), allVerticesDevice_, allEdgesDevice_, layout_.get());
    // create edge index map
    connections_->createEdgeIndexMap();
    // copy index map to the device memory
    copyEdgeIndexMapHostToDevice(*(connections_->getEdgeIndexMap().get()), Simulator::getInstance().getTotalVertices());
  }
}

/// Update the Neuron's history.
void GPUModel::updateHistory()
{
  Model::updateHistory();
  // clear spike count
  
  shared_ptr<AllVertices> vertices = layout_->getVertices();
  dynamic_cast<AllSpikingNeurons*>(vertices.get())->clearNeuronSpikeCounts(allVerticesDevice_);
}

/// Allocate device memory for edge inverse map.
/// @param  count	The number of vertices.
void GPUModel::allocSynapseImap( int count )
{
  EdgeIndexMap synapseIMapDevice;

  HANDLE_ERROR( cudaMalloc( ( void ** ) &synapseIMapDevice.outgoingEdgeBegin_, count * sizeof( BGSIZE ) ) );
  HANDLE_ERROR( cudaMalloc( ( void ** ) &synapseIMapDevice.outgoingEdgeCount_, count * sizeof( BGSIZE ) ) );
  HANDLE_ERROR( cudaMemset(synapseIMapDevice.outgoingEdgeBegin_, 0, count * sizeof( BGSIZE ) ) );
  HANDLE_ERROR( cudaMemset(synapseIMapDevice.outgoingEdgeCount_, 0, count * sizeof( BGSIZE ) ) );

  HANDLE_ERROR( cudaMalloc( ( void ** ) &synapseIMapDevice.incomingEdgeBegin_, count * sizeof( BGSIZE ) ) );
  HANDLE_ERROR( cudaMalloc( ( void ** ) &synapseIMapDevice.incomingEdgeCount_, count * sizeof( BGSIZE ) ) );
  HANDLE_ERROR( cudaMemset(synapseIMapDevice.incomingEdgeBegin_, 0, count * sizeof( BGSIZE ) ) );
  HANDLE_ERROR( cudaMemset(synapseIMapDevice.incomingEdgeCount_, 0, count * sizeof( BGSIZE ) ) );

  HANDLE_ERROR( cudaMalloc( ( void ** ) &edgeIndexMapDevice_, sizeof( EdgeIndexMap ) ) );
  HANDLE_ERROR( cudaMemcpy( edgeIndexMapDevice_, &synapseIMapDevice, sizeof( EdgeIndexMap ), 
        cudaMemcpyHostToDevice ) );
}

/// Deallocate device memory for edge inverse map.
void GPUModel::deleteSynapseImap(  )
{
  EdgeIndexMap synapseIMapDevice;

  HANDLE_ERROR( cudaMemcpy ( &synapseIMapDevice, edgeIndexMapDevice_, 
        sizeof( EdgeIndexMap ), cudaMemcpyDeviceToHost ) );

  HANDLE_ERROR( cudaFree( synapseIMapDevice.outgoingEdgeBegin_ ) );
  HANDLE_ERROR( cudaFree( synapseIMapDevice.outgoingEdgeCount_ ) );
  HANDLE_ERROR( cudaFree( synapseIMapDevice.outgoingEdgeIndexMap_ ) );

  HANDLE_ERROR( cudaFree( synapseIMapDevice.incomingEdgeBegin_ ) );
  HANDLE_ERROR( cudaFree( synapseIMapDevice.incomingEdgeCount_ ) );
  HANDLE_ERROR( cudaFree( synapseIMapDevice.incomingEdgeIndexMap_ ) );

  HANDLE_ERROR( cudaFree( edgeIndexMapDevice_ ) );
}

/// Copy EdgeIndexMap in host memory to EdgeIndexMap in device memory.
/// @param  synapseIndexMapHost		Reference to the EdgeIndexMap in host memory.
void GPUModel::copyEdgeIndexMapHostToDevice(EdgeIndexMap &synapseIndexMapHost, int numVertices)
{
  shared_ptr<AllEdges> edges = connections_->getEdges();
  int totalSynapseCount = dynamic_cast<AllEdges*>(edges.get())->totalEdgeCount_;

  if (totalSynapseCount == 0)
    return;

  // TODO: rename variable, DevicePointer
  EdgeIndexMap synapseIMapDevice;

  HANDLE_ERROR( cudaMemcpy ( &synapseIMapDevice, edgeIndexMapDevice_, 
        sizeof( EdgeIndexMap ), cudaMemcpyDeviceToHost ) );

  // forward map
  HANDLE_ERROR( cudaMemcpy ( synapseIMapDevice.outgoingEdgeBegin_, 
        synapseIndexMapHost.outgoingEdgeBegin_, numVertices * sizeof( BGSIZE ), cudaMemcpyHostToDevice ) );
  HANDLE_ERROR( cudaMemcpy ( synapseIMapDevice.outgoingEdgeCount_, 
        synapseIndexMapHost.outgoingEdgeCount_, numVertices * sizeof( BGSIZE ), cudaMemcpyHostToDevice ) );
  // the number of edges may change, so we reallocate the memory
  if (synapseIMapDevice.outgoingEdgeIndexMap_ != nullptr) {
    HANDLE_ERROR( cudaFree( synapseIMapDevice.outgoingEdgeIndexMap_ ) );
  }
  HANDLE_ERROR( cudaMalloc( ( void ** ) &synapseIMapDevice.outgoingEdgeIndexMap_, 
        totalSynapseCount * sizeof( BGSIZE ) ) );
  HANDLE_ERROR( cudaMemcpy ( synapseIMapDevice.outgoingEdgeIndexMap_, synapseIndexMapHost.outgoingEdgeIndexMap_, 
        totalSynapseCount * sizeof( BGSIZE ), cudaMemcpyHostToDevice ) );

  // active edge map
  HANDLE_ERROR( cudaMemcpy ( synapseIMapDevice.incomingEdgeBegin_, 
        synapseIndexMapHost.incomingEdgeBegin_, numVertices * sizeof( BGSIZE ), cudaMemcpyHostToDevice ) );
  HANDLE_ERROR( cudaMemcpy ( synapseIMapDevice.incomingEdgeCount_, 
        synapseIndexMapHost.incomingEdgeCount_, numVertices * sizeof( BGSIZE ), cudaMemcpyHostToDevice ) );
  // the number of edges may change, so we reallocate the memory
  if (synapseIMapDevice.incomingEdgeIndexMap_ != nullptr) {
    HANDLE_ERROR( cudaFree( synapseIMapDevice.incomingEdgeIndexMap_ ) );
  }
  HANDLE_ERROR( cudaMalloc( ( void ** ) &synapseIMapDevice.incomingEdgeIndexMap_, 
        totalSynapseCount * sizeof( BGSIZE ) ) );
  HANDLE_ERROR( cudaMemcpy ( synapseIMapDevice.incomingEdgeIndexMap_, synapseIndexMapHost.incomingEdgeIndexMap_, 
        totalSynapseCount * sizeof( BGSIZE ), cudaMemcpyHostToDevice ) );

  HANDLE_ERROR( cudaMemcpy ( edgeIndexMapDevice_, &synapseIMapDevice, 
        sizeof( EdgeIndexMap ), cudaMemcpyHostToDevice ) );
}

/// Calculate the sum of synaptic input to each vertex.
///
/// Calculate the sum of synaptic input to each vertex. One thread
/// corresponds to one vertex. Iterates sequentially through the
/// forward edge index map (edgeIndexMapDevice_) to access only
/// existing edges. Using this structure eliminates the need to skip
/// edges that have undergone lazy deletion from the main
/// (allEdgesDevice) edge structure. The forward map is
/// re-computed during each network restructure (once per epoch) to
/// ensure that all edge pointers for a vertex are stored
/// contiguously.
/// 
/// @param[in] totalVertices           Number of vertices in the entire simulation.
/// @param[in,out] allVerticesDevice   Pointer to Neuron structures in device memory.
/// @param[in] edgeIndexMapDevice_  Pointer to forward map structures in device memory.
/// @param[in] allEdgesDevice      Pointer to Edge structures in device memory.
__global__ void calcSummationMapDevice(int totalVertices, 
				       AllSpikingNeuronsDeviceProperties* __restrict__ allVerticesDevice, 
				       const EdgeIndexMap* __restrict__ edgeIndexMapDevice_, 
				       const AllSpikingSynapsesDeviceProperties* __restrict__ allEdgesDevice)
{
  // The usual thread ID calculation and guard against excess threads
  // (beyond the number of vertices, in this case).
  int idx = blockIdx.x * blockDim.x + threadIdx.x;
  if ( idx >= totalVertices )
    return;

  // Number of incoming edges
  const BGSIZE synCount = edgeIndexMapDevice_->incomingEdgeCount_[idx];
  // Optimization: terminate thread if no incoming edges
  if (synCount != 0) {
    // Index of start of this vertex's block of forward map entries
    const int beginIndex = edgeIndexMapDevice_->incomingEdgeBegin_[idx];
    // Address of the start of this vertex's block of forward map entries
    const BGSIZE* activeMapBegin = 
      &(edgeIndexMapDevice_->incomingEdgeIndexMap_[beginIndex]);
    // Summed post-synaptic response (PSR)
    BGFLOAT sum = 0.0;
    // Index of the current incoming edge
    BGSIZE synIndex;
    // Repeat for each incoming edge
    for (BGSIZE i = 0; i < synCount; i++) {
      // Get index of current incoming edge
      synIndex = activeMapBegin[i];
      // Fetch its PSR and add into sum
      sum += allEdgesDevice->psr_[synIndex];
    }
    // Store summed PSR into this vertex's summation point
    allVerticesDevice->summationMap_[idx] = sum;
  }
}

/// Copy GPU Edge data to CPU.
void GPUModel::copyGPUtoCPU()
{
  // copy device edge structs to host memory
  connections_->getEdges()->copyEdgeDeviceToHost(allEdgesDevice_);
}

/// Copy CPU Edge data to GPU.
void GPUModel::copyCPUtoGPU()
{
  // copy host edge structs to device memory
  connections_->getEdges()->copyEdgeHostToDevice(allEdgesDevice_);
}

/// Print out SynapseProps on the GPU.
void GPUModel::printGPUSynapsesPropsModel() const
{  
  connections_->getEdges()->printGPUEdgesProps(allEdgesDevice_);
}

