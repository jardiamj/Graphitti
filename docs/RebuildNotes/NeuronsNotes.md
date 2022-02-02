

void AllNeurons::setupNeurons(SimulationInfo *sim_info)


    // summation map - summation points for neuron. allocated here. setup methods are also allocating internal storage.
    summation_map = new BGFLOAT[size];
    // init summation map. blank. zeroing out.
    for (int i = 0; i < size; ++i) {
        summation_map[i] = 0;
    }
    // giving simulator a pointer to the summation map.
    // TODO: change. synapses need to write into summation points.
    // grab global. alloc internal storage. glue.
    // toDo: when do the synapses pick this up?
    // possible they pick up during init time. then there is a dependency.
    sim_info->pSummationMap = summation_map;
}

Notes about neurons:
a neuron that is non spiking's output is a voltage level. different than inh/exc. assuming inh/exc are spiking neurons

Other kinds of neurons. primary output is releasing neuro hormones. might have two outputs.
main function is releasing chemicals in brain.

neurons in crustaceans control motor activity of stomachs, etc.

/*
 *  Setup the internal structure of the class (allocate memories).
 *
 *  @param  sim_info  SimulationInfo class to read information from.
 */
void AllSpikingNeurons::setupNeurons(SimulationInfo *sim_info)
{
    AllNeurons::setupNeurons(sim_info);

    // TODO: Rename variables for easier identification
    hasFired = new bool[size];
    spikeCount = new int[size];
    spikeCountOffset = new int[size];
    spike_history = new uint64_t*[size];

    for (int i = 0; i < size; ++i) {
        spike_history[i] = NULL;
        hasFired[i] = false;
        spikeCount[i] = 0;
        spikeCountOffset[i] = 0;
    }
    // seems to be duplicate of what is in allneurons::setup
    sim_info->pSummationMap = summation_map;
}

GPU side code. no setup because copy from CPU
 * AllSpikingNeurons_d.cu

All spiking neurons is still not a concrete class.

Captures characteristics of spiking neurons. All IF Neurons .cpp is implem.

/*
 *  Setup the internal structure of the class (allocate memories).
 *
 *  @param  sim_info  SimulationInfo class to read information from.
 */
void AllIFNeurons::setupNeurons(SimulationInfo *sim_info)
{
    AllSpikingNeurons::setupNeurons(sim_info);

    // TODO: Rename variables for easier identification
    C1 = new BGFLOAT[size];
    C2 = new BGFLOAT[size];
    Cm = new BGFLOAT[size];
    I0 = new BGFLOAT[size];
    Iinject = new BGFLOAT[size];
    Inoise = new BGFLOAT[size];
    Isyn = new BGFLOAT[size];
    Rm = new BGFLOAT[size];
    Tau = new BGFLOAT[size];
    Trefract = new BGFLOAT[size];
    Vinit = new BGFLOAT[size];
    Vm = new BGFLOAT[size];
    Vreset = new BGFLOAT[size];
    Vrest = new BGFLOAT[size];
    Vthresh = new BGFLOAT[size];
    nStepsInRefr = new int[size];

    // more allocating and initializing.
    for (int i = 0; i < size; ++i) {
        nStepsInRefr[i] = 0;
    }
}

AllLIFNeurons and ALLIZHNeurons are both integrate and fire neurons
(belong to same categ) but different implems of integ and fire model.

// creating actual neurons, reading from param file.
AllIFNeurons::createNeuron


rng = random number generator

// spike history data structure is being created in createNeuron, but everything else is just init.

allLIFNeurons doesnt do additional stuff. create all neurons inits everything and allocs one data structure for spikes.

createallneurons could be part of setupneurons.

todo: decide between one setup method or allocation and initialization being separate. alloc_CPU and init_CPU

createallneurons can be merged into alloc and init.


