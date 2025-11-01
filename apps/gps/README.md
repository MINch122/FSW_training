# GPS Application

Repository for the OEM7 GPS receiver series driver and Application frames.



# Driver modules

  ┌──────────────┐
  │  Log Module  │
  └──────────────┘

 ┌────────────────┐      ┌────────────────┐
 │   Task Module  │      │ Command Module │
 └────────────────┘      └────────────────┘

             ┌────────────────┐
             │   I/O Module   │
             └────────────────┘

         ┌────────────────────────┐
         │   OEM7 Receiver H/W    │
         └────────────────────────┘

The OEM7 driver is composed of several modular components, each serving a
specific purpose.

1) The I/O module (oem_io.h)
The I/O module forms the lowest layer, interfacing directly with the physical
port connected to the OEM7 hardware. Users can attach mission-specific read
and write callbacks to handle all data transactions with the receiver,
allowing full customization of I/O behavior.


2) The Command module (oem_cmd.h)
The Command module sends OEM7 Commands such as Log requests or receiver
settings.


3) The Log module (oem_log.h)
The Log module is responsible for processing of incoming Log messages. Each
message is passed through a chain of callbacks that can be dynamically added
or removed at runtime, providing maximum flexibility in how log data is
handled. The module also provides utilities such as Log statistics (counts,
errors) or activation switches to turn on/off log processing.


4) The Task module (oem_task.h)
Lastly, the Task module provides the main entry point for Log handling.
The only userspace method in this layer is the OEM_Task_ReadTaskSingleRun()
function, which reads a single Log or Response message from a physical port,
verifies the data as per the sync word and the CRC, then eventually passes it
to the associated Log Handler. The context structure can be examined to
identify any errors or statistics during the processing.


 
# Driver Usage

1) Initialization
    - Initialize the physical I/O (serial) by calling OEM_IO_PortInit().
    - Call OEM_Log_HandlerInit().
    - Launch a mission-defined child task where OEM_Task_ReadTaskSingleRun()
    processes any incoming Log messages.

2) Adding a Log Handler
    - Register an empty handler for a Log message by calling
      OEM_Log_RegisterHandler(). At this point, the handler is at an
      inactivated status and no handling callbacks are present. As the
      designated Logs arrives, only the statistics (counts) are updated.
    - Add a processing callback using OEM_Log_AddCallback(). More than
      one callbacks can be attached to a handler by successively calling
      the method. Logs are passed to each callback at the order of
      attachment.
    - Activate the handler using OEM_Log_HandlerActivate() to fianlly
      allow the handler to pass the Logs to the added callbacks.

3) Runtime utilities
    - Handler statistics such as Log counts, error counts, current activation
      status can be retrieved from OEM_Log_GethandlerHousekeeping().
    - Use OEM_Log_DumpRecentMessage() to copy the latest Log message.
    - Handler status determines to what levels the Log processing is allowed.
      See oem_log_handler_status_t. For example, use OEM_Log_HandlerDeacivate()
      to temporarily pend the handler callback chain until reactivated by
      OEM_Log_HandlerActivate().      
    - When a critical failure occurs during processing, the handler status
      is automatically altered to the "broken" status. A ground operator
      can carefully examine the error, apply a fix then revive it by calling
      OEM_Log_SetHandlerStatus() with the override option enabled.
    


# Detailed 