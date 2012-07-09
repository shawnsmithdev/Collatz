#include <stdio.h>
#include <stdlib.h>
#include <gccore.h>
#include <wiiuse/wpad.h>

#define uint unsigned int
#define uint16 unsigned short
#define REALLOC_FCT 2
#define COLLATZ_LIM 16500000

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

/* Defines a convenient wrapper around a uint16 array. */
typedef struct {
    uint16 * ar;
    uint size;
    uint cap;
} uint16ArrayStr, *uint16Array;

/* Creates a convenient wrapper around a uint16 array. */
uint16Array uint16ArrayInit(uint capacity) {
    uint16Array results = (uint16Array) malloc(sizeof(uint16ArrayStr));
    results->ar = (uint16 *) malloc(sizeof(uint16) * capacity);
    results->cap = capacity;
    results->size = 0;
    return results;
}

/* Frees up ram allocated to given array. */
void uint16ArrayFree(uint16Array tofree) {
    free(tofree->ar);
    free(tofree);
}

/* Appends a value to the given array. */
void uint16ArrayAppend(uint16Array uintArr, uint16 append) {
    //Ensure there is room to append, reallocing if needed
    if (uintArr->size >= uintArr->cap) {
        uint newcap = uintArr->cap * REALLOC_FCT;
        uintArr->ar = (uint16 *) realloc(uintArr->ar, sizeof(uint16) * newcap);
        if (uintArr->ar == NULL) {
            printf("Failed to grow array, buy more ram\n");
            return;
        } else {
            uintArr->cap = newcap;
        }
    }
    uintArr->ar[uintArr->size] = append;
    uintArr->size++;
}

/* Determines the Collatz algo steps from n until 1,
   reading cached values from given array. */
uint16 collatz(uint16Array cache, uint n) {
    if (n < cache->size) { // In cache
        return cache->ar[n];
    } else if ((n & 1) == 0) {// even
        return 1 + collatz(cache, n>>1);
    } // odd
    return 1 + collatz(cache, (3*n)+1);
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------

    // Initialise the video system
    VIDEO_Init();

    // This function initialises the attached controllers
    WPAD_Init();

    // Obtain the preferred video mode from the system
    // This will correspond to the settings in the Wii menu
    rmode = VIDEO_GetPreferredMode(NULL);

    // Allocate memory for the display in the uncached region
    xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));

    // Initialise the console, required for printf
    console_init(xfb,20,20,rmode->fbWidth,rmode->xfbHeight,rmode->fbWidth*VI_DISPLAY_PIX_SZ);

    // Set up the video registers with the chosen mode
    VIDEO_Configure(rmode);

    // Tell the video hardware where our display memory is
    VIDEO_SetNextFramebuffer(xfb);

    // Make the display visible
    VIDEO_SetBlack(FALSE);

    // Flush the video register changes to the hardware
    VIDEO_Flush();

    // Wait for Video setup to complete
    VIDEO_WaitVSync();
    if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();


    // The console understands VT terminal escape codes
    // This positions the cursor on row 2, column 0
    // we can use variables for this with format codes too
    // e.g. printf ("\x1b[%d;%dH", row, column );
    printf("\x1b[2;0H");


    printf("Starting to print Collatz...");

    // cache->ar[n] holds how many steps of the
    // collatz algo it takes to get from n to 1.
    uint16Array cache = uint16ArrayInit(256);

    // Prefill with initial values
    uint16ArrayAppend(cache, 0); // 0 isn't defined
    uint16ArrayAppend(cache, 0); // 1 is the goal, the root of the tree ...
    uint i = 1; //  1 is the first element in the sequence.
    printf("%d in %d steps\n", i, 1);

    // Do the real work
    uint16 steps;
    uint16 maxSteps = 0;

    for (i = 2; i < COLLATZ_LIM; i++) {
        steps = collatz(cache, i);
        uint16ArrayAppend(cache, steps);
        if (steps > maxSteps) {
            printf("%d in %d steps\n", i, steps);
            maxSteps = steps;
        }
    }

    // Clean up after yourself.
    // you were not raised in a barn.
    uint16ArrayFree(cache);


    printf("Finished Printing Collatz, press home button!\n\n\n");

    while(1) {

        // Call WPAD_ScanPads each loop, this reads the latest controller states
        WPAD_ScanPads();

        // WPAD_ButtonsDown tells us which buttons were pressed in this loop
        // this is a "one shot" state which will not fire again until the button has been released
        u32 pressed = WPAD_ButtonsDown(0);

        // We return to the launcher application via exit
        if ( pressed & WPAD_BUTTON_HOME ) exit(0);

        // Wait for the next frame
        VIDEO_WaitVSync();
    }

    return 0;
}
