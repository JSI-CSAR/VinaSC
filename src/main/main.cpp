#include "vina_main.h"

#ifdef __MIC__
COINATIVELIBEXPORT
void mic_bootup (uint32_t         in_BufferCount,
          void**           in_ppBufferPointers,
          uint64_t*        in_pBufferLengths,
          void*            in_pMiscData,
          uint16_t         in_MiscDataLength,
          void*            in_pReturnValue,
          uint16_t         in_ReturnValueLength)
{

    assert(in_BufferCount>1);
    char **argv;
    int argc;

    deserial_strings((char *)in_ppBufferPointers[0], in_pBufferLengths[0], argc, argv);

    for(int i = 0; i < argc; i++) {
		printf("### on mic_bootup arg %d: %s\n", i, argv[i]); fflush(stdout);
    }

}



#endif

#ifndef __MIC__
COIPROCESS          proc;
COIENGINE           engine;
COIFUNCTION         func;
COIPIPELINE         pipeline;
uint32_t            num_engines = 0;
#endif

int main(int argc, char** argv) {
	int ret = 0;
	
	#ifndef __MIC__ // --------------------- cpu side ------------------------------
	
    const char*         SINK_NAME = "vina.mic";

    CHECK_RESULT(
    COIEngineGetCount(COI_ISA_MIC, &num_engines));

    if (num_engines < 1) {
        printf("ERROR: Need at least 1 engine\n");
        return -1;
    }

    CHECK_RESULT(
    COIEngineGetHandle(COI_ISA_MIC, 0, &engine));

    // Process: Represents process created on the device enumerated(engine).
    //          Processes created on sink side are referenced by COIPROCESS
    //          instance

    // The following call creates a process on the sink.
    // We will automatically load any dependent libraries and run the "main"
    // function in the binary.
    //
    CHECK_RESULT(
    COIProcessCreateFromFile(
        engine,             // The engine to create the process on.
        SINK_NAME,          // The local path to the sink side binary to launch.
        argc, (const char **)argv,            // argc and argv for the sink process.
        false, NULL,        // Environment variables to set for the sink
                            // process.
        true, NULL,         // Enable the proxy but don't specify a proxy root
                            // path.
        1024*1024,          // The amount of memory to pre-allocate and
                            // register for use with COIBUFFERs.
        NULL,               // Path to search for dependencies
        &proc               // The resulting process handle.
    ));

    // Pipeline:
    // After a sink side process is created, multiple pipelines can be created
    // to that process. Pipelines are queues where functions(represented by
    // COIFUNCTION) to be executed on sink side can be enqueued.

    // The following call creates a pipeline associated with process created
    // earlier.

    CHECK_RESULT(
    COIPipelineCreate(
        proc,            // Process to associate the pipeline with
        NULL,            // Do not set any sink thread affinity for the pipeline
        0,               // Use the default stack size for the pipeline thread
        &pipeline        // Handle to the new pipeline
    ));

    // Retrieve handle to function belonging to sink side process
/*
    const char* func_name = "mic_bootup";

    CHECK_RESULT(
    COIProcessGetFunctionHandles(
        proc,         // Process to query for the function
        1,            // The number of functions to query
        &func_name,   // The name of the function
        &func         // A handle to the function
    ));

    #define N 1024
    char *buf = new char[N];
    int len = N;

    serial_strings(buf, len, argc, argv);

    COIBUFFER       input = NULL;
    COIBUFFER       output = NULL;

    // Buffers:
    // Buffers are data containers in Intel庐 Coprocessor Offload Infrastructure (Intel庐 COI) . They can be used to transfer large
    // chunks of data back and forth between sink and source. (Small chunks
    // can be passed directly to the function.)

    // Create an input buffer
    // Buffers are page aligned. Internally Intel庐 Coprocessor Offload Infrastructure (Intel庐 COI)  aligns it to make multiple of
    // page size
    CHECK_RESULT(
    COIBufferCreate(
        len,                // Size of the buffer
        COI_BUFFER_NORMAL,  // Type of the buffer
        0,                  // Buffer creation flags
        buf,        // Pointer to the initialization data
        1, &proc,           // Processes that will use the buffer
        &input              // Buffer handle that was created
    ));


    // Create an output buffer
    CHECK_RESULT(
    COIBufferCreate(
        len,                // Size of the buffer
        COI_BUFFER_NORMAL,  // Type of the buffer
        0,                  // Buffer creation flags
        NULL,               // Pointer to the Initialization data
        1, &proc,           // Processes that will use the buffer
        &output             // Buffer handle that was created
    ));

    COI_ACCESS_FLAGS flags[2] = { COI_SINK_READ , COI_SINK_WRITE };
    COIBUFFER buffers[2] = {input,output};


     COIEVENT              completion_event;
    // Enqueue the Function for execution
    // Pass the buffers created into run function with buffer flags.
    CHECK_RESULT(
    COIPipelineRunFunction(
        pipeline, func,    // Pipeline handle and function handle
        2, buffers, flags, // Buffers and access flags to pass to the function
        0, NULL,           // Input dependencies
        NULL,0,            // Misc data to pass to the function
        NULL,0,            // Return values that will be passed back
        &completion_event   // Event to signal when the function completes
    ));

    CHECK_RESULT(
        COIEventWait(
            1,                           // Number of events to wait for
            &completion_event,        // Event handles
            -1,                          // Wait indefinitely
            true,                        // Wait for all events
            NULL, NULL                   // Number of events signaled
                                         // and their indices
        ));
*/
    ret = boot_up(argc, argv);

   
    // Destroy the buffers
    CHECK_RESULT(
    COIBufferDestroy(input));

    CHECK_RESULT(
    COIBufferDestroy(output));

    // Destroy the pipeline
    //
    CHECK_RESULT(
    COIPipelineDestroy(pipeline));


    // Destroy the process
    //
    CHECK_RESULT(
    COIProcessDestroy(
       proc,           // Process handle to be destroyed
       -1,             // Wait indefinitely until main() (on sink side) returns
       false,          // Don't force to exit. Let it finish executing
                       // functions enqueued and exit gracefully
       NULL,           // Don't care about the exit result.
       NULL            // Also don't care what the exit reason was.
    ));
    printf("Destroyed process\n");

    printf("Exiting\n");

	#else  // --------------------- mic side ------------------------------
	
    ret = boot_up(argc-1, argv+1);

    printf("###get number of workers on MIC : %d\n", global_vina_main->worker_num); fflush(stdout);

	UNUSED_ATTR COIRESULT result;
    result = COIPipelineStartExecutingRunFunctions();
    assert(result == COI_SUCCESS);
    COIProcessWaitForShutdown();	

	#endif
	
	

	return ret;
}
