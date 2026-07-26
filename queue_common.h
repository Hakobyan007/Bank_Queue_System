#ifndef QUEUE_COMMON
#define QUEUE_COMMON


/* =====================================================================
 * TICKET STRUCTURE
 * Represents a single queue ticket and its current lifecycle state.
 * ===================================================================== */

typedef struct 
{
    char ticket_place[7];      // Unique ticket identifier (e.g., "A105")
    int service_index;         // ID of the requested service (from service_choice)
    int status;                // Ticket state: 1=Waiting, 2=In Progress, 3=Served
    int priority;              // Client type: 1=Natural Person, 2=Legal Entity
    int window_num;            // Assigned service window number
    char create_time[20];      // Timestamp of creation (Format: HH:MM:SS)
    char create_date[20];      // Datestamp of creation (Format: YYYY-MM-DD)
} ticket_example;



/* =====================================================================
 * WINDOW STRUCTURE
 * Represents a service window, its capabilities, and connection state.
 * ===================================================================== */

typedef struct
{
    int window_number;         // Unique identifier for the service window
    int admin_fd;              // File descriptor for admin communication (socket/pipe)
    int allowed_services[7];   // Array of service IDs this window is authorized to process
    int status;                // Current window state: 0=Offline, 1=Available, 2=Busy
} window_example;

#endif