
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

	fprintf(f, "Line Format: time (in sec) since program started, length (in sec) of loop, primary rpm, secondary rpm\n\n");

	// Counter Channels
	int pCounter = 0; // Primary
	int sCounter = 1; // Secondary

	// Channel Data
	unsigned long long pData = 0;
	unsigned long long sData = 0;

	// Timing for Counters
	struct timespec start_ts, current_ts; // IN NANOSECONDS
	u_int64_t previousTime = 0;
	int timePerLoop = 100; // 100 milliseconds

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

	// Clear console / output screen
	//system("clear");

	// OFF TO THE RACES!	
	clock_gettime(CLOCK_MONOTONIC_RAW, &start_ts);
	
	while(err == ERR_NO_ERROR && !enter_press()) {
		

		// Abracadabra! Counters do count thing.
		err = ulCIn(daqDeviceHandle, pCounter, &pData);
		err = ulCIn(daqDeviceHandle, sCounter, &sData);

		// Display current data
		//system("clear");
		//resetCursor();

		clock_gettime(CLOCK_MONOTONIC_RAW, &current_ts);
		
		u_int64_t currentMilliseconds = ((current_ts.tv_sec - start_ts.tv_sec) * 1000000 + (current_ts.tv_nsec - start_ts.tv_nsec) / 1000) / 1000;

		// MICROSECONDS	
		// u_int64_t microseconds = (current_ts.tv_sec - start_ts.tv_sec) * 1000000 + (current_ts.tv_nsec - start_ts.tv_nsec) / 1000;
		
		//fprintf(stdout, "%"PRIu64"\n", currentMilliseconds);
		//fprintf(stdout, "Primary Counter %d: %lld\n", pCounter, pData);
		//fprintf(stdout, "Secondary Counter %d: %lld\n", sCounter, sData);
		//fprintf(stdout, "\n");
		//ratio = pData / (float) sData;
		//fprintf(stdout, "Ratio: %f\n", ratio);
		//fprintf(stdout, "\n");
		//fprintf(stdout, "Primary RPM: %f\n", pRPM);
		//fprintf(stdout, "Secondary RPM: %f\n", sRPM);

		if(currentMilliseconds >= previousTime + timePerLoop){
			
			pRPM = ((float) pData / ((currentMilliseconds - previousTime) * 1000));

			sRPM = ((float) sData / ((currentMilliseconds - previousTime) * 1000));

			long timestamp_seconds = current_ts.tv_sec - start_ts.tv_sec;

			long timestamp_msecs = (current_ts.tv_nsec / 1000000) - (start_ts.tv_nsec / 1000000);

			if (timestamp_msecs < 0) {
				timestamp_seconds = timestamp_seconds - (long)1;
				timestamp_msecs = 1000 + timestamp_msecs;
			}
				//(current_ts.tv_sec - start_ts.tv_sec) * 1000000 + current_ts.tv_nsec - start_ts.tv_nsec;

			fprintf(f, "%ld.%ld    %"PRIu64"    %f    %f\n", timestamp_seconds, timestamp_msecs, currentMilliseconds - previousTime, pRPM, sRPM);
			
				
			//fprintf(f, "%"PRIu64"    %f    %f\n", currentMilliseconds - previousTime, pRPM, sRPM);
			
			//fprintf(f, "%jd.%.9ld   %"PRIu64"    %f    %f\n", (intmax_t)(current_ts.tv_sec - start_ts.tv_sec), current_ts.tv_nsec - start_ts.tv_nsec, currentMilliseconds - previousTime, pRPM, sRPM);
			
			previousTime = currentMilliseconds;

			// Reset counters / channels
			ulCClear(daqDeviceHandle, pCounter);
			ulCClear(daqDeviceHandle, sCounter);
		}
		
		//usleep(100000);
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

	// fprintf(f, "\nTest Ended.\n");

	return 0;
}

