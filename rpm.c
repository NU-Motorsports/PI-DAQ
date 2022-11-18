
// RPM for Primary and Secondary on CVT
// A certified mic drop by Owen and Ella.

#include <stdio.h>
#include <stdlib.h>
#include "uldaq.h"
#include "utility.h"
#include <time.h>

#include <inttypes.h>


#define MAX_DEV_COUNT  100

int main(void) {
	// Create file for data output
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	char filename[19];

	sprintf(filename, "results/%d-%02d-%02d___%02d-%02d-%02d.txt", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

	FILE *f = fopen(filename, "w"); // create a file for output // write only

	fprintf(f, "Line Format: time (in sec) since program started, primary rpm, secondary rpm\n\n");

	// Counter Channels
	int pCounter = 0; // Primary
	int sCounter = 1; // Secondary

	// Channel Data
	unsigned long long pData = 0;
	unsigned long long sData = 0;

	// Timing for Counters
	struct timespec s, c; // IN NANOSECONDS
	// *.tv_sec (time_t)
	// *.tv_nsec (long)
	
	long int sInMilli, cInMilli, prevTime;
	
	int milliPerLoop = 3000; // milliseconds

	float perMinute;
	
	long int timestampInMilli;
	long int ts_sec;
	long int ts_milli;

	//float ratio = 0;
	float pRPM = 0;
	float sRPM = 0;

	UlError err = ERR_NO_ERROR;

	// Set up DAQ Device / ze hardware
	DaqDeviceDescriptor devDescriptors[MAX_DEV_COUNT];
	DaqDeviceHandle daqDeviceHandle = 0;
	unsigned int numDevs = MAX_DEV_COUNT;

	// Get descriptors for all of the available DAQ devices
	err = ulGetDaqDeviceInventory(ANY_IFC, devDescriptors, &numDevs);

	// get a handle to the DAQ device associated with the first descriptor
	daqDeviceHandle = ulCreateDaqDevice(devDescriptors[0]);

	// establish a connection to the DAQ device
	err = ulConnectDaqDevice(daqDeviceHandle);

	// Clear counters / channels
	err = ulCClear(daqDeviceHandle, pCounter);
	err = ulCClear(daqDeviceHandle, sCounter);

	if (err != ERR_NO_ERROR) {
		goto end;
	}

	// OFF TO THE RACES!	
	clock_gettime(CLOCK_MONOTONIC_RAW, &s);	
	sInMilli = (s.tv_sec * 1000) + (s.tv_nsec / 1000000);
	prevTime = sInMilli;

	while(err == ERR_NO_ERROR && !enter_press()) {		

		// Abracadabra! Counters do count thing.
		err = ulCIn(daqDeviceHandle, pCounter, &pData);
		err = ulCIn(daqDeviceHandle, sCounter, &sData);

		// Display current data
		//system("clear");
		//resetCursor();

		clock_gettime(CLOCK_MONOTONIC_RAW, &c);
		cInMilli = (c.tv_sec * 1000) + (c.tv_nsec / 1000000);

		//fprintf(stdout, "%"PRIu64"\n", currentMilliseconds);
		//fprintf(stdout, "Primary Counter %d: %lld\n", pCounter, pData);
		//fprintf(stdout, "Secondary Counter %d: %lld\n", sCounter, sData);
		//fprintf(stdout, "\n");
		//ratio = pData / (float) sData;
		//fprintf(stdout, "Ratio: %f\n", ratio);
		//fprintf(stdout, "\n");
		//fprintf(stdout, "Primary RPM: %f\n", pRPM);
		//fprintf(stdout, "Secondary RPM: %f\n", sRPM);

		if(cInMilli >= prevTime + milliPerLoop){

			perMinute = ((float) 60) / ((float) (cInMilli - prevTime));
			
			pRPM = ((float) pData) / perMinute;

			sRPM = ((float) sData) / perMinute;
			
			timestampInMilli = cInMilli - sInMilli;

			ts_sec = timestampInMilli / 1000;

			ts_milli = timestampInMilli % 1000;

			// Prints data kindly for Ella's human eyes
			//fprintf(f, "%5ld.%02lu    %20.2f    %20.2f\n", ts_sec, ts_milli, pRPM, sRPM);
			
			// Print data kindly for computer taking in data
			fprintf(f, "%ld.%ld, %.2f, %.2f\n", ts_sec, ts_milli, pRPM, sRPM);
							

			prevTime = cInMilli;

			// Reset counters / channels
			ulCClear(daqDeviceHandle, pCounter);
			ulCClear(daqDeviceHandle, sCounter);
		}
		
	}

	// disconnect from the DAQ device
	ulDisconnectDaqDevice(daqDeviceHandle);

end:

	// release the handle to the DAQ device
	ulReleaseDaqDevice(daqDeviceHandle);

	if(err != ERR_NO_ERROR)
	{
		char errMsg[ERR_MSG_LEN];
		ulGetErrMsg(err, errMsg);
		printf("Error Code: %d \n", err);
		printf("Error Message: %s \n", errMsg);
	}

	return 0;
}

