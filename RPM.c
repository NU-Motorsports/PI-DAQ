
// RPM for Primary and Secondary on CVT
// A certified mic drop by Owen and Ella.

#include <stdio.h>
#include <stdlib.h>
#include "uldaq.h"
#include "utility.h"
#include <time.h>


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
	unsigned long long previousTime = t;
	int measureTime = 10; //How long to wait between resetting the counters

	float ratio = 0;
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
	system("clear");

	// OFF TO THE RACES!
	
	while(err == ERR_NO_ERROR && !enter_press()) {

		// Abracadabra! Counters do count thing.
		err = ulCIn(daqDeviceHandle, pCounter, &pData);
		err = ulCIn(daqDeviceHandle, sCounter, &sData);

		// Display current data
		system("clear");
		resetCursor();	
		
		fprintf(stdout, "Primary Counter %d: %lld\n", pCounter, pData);
		fprintf(stdout, "Secondary Counter %d: %lld\n", sCounter, sData);
		fprintf(stdout, "\n");

		ratio = pData / (float) sData;
		fprintf(stdout, "Ratio: %f\n", ratio);
		fprintf(stdout, "\n");
		
		fprintf(stdout, "Primary RPM: %f\n", pRPM);
		fprintf(stdout, "Secondary RPM: %f\n", sRPM);

		// check if the current time is at least 10s later than the last time data was cleared
		// clear the couters and record the current time
		// currently this is done with time(), but this only has second resolustion, might want to switch to something more precise later
		// -Owen
		if(time(0) >= previousTime + measureTime){
			
			pRPM = ((float) pData / ((time(0) - previousTime) * 60));
			//fprintf(stdout, "NEW PRIMARY RPM = %f\n", pRPM);

			sRPM = ((float) sData / ((time(0) - previousTime) * 60));
			//fprintf(stdout, "NEW SECONDARY RPM = %f\n\n", sRPM);
			
			fprintf(f, "%d, %lld, %f, %f\n", time(0) - t, time(0) - previousTime, pRPM, sRPM);
			//time(0), previousTime, measureTime, pData, sData, primaryRPM, secondaryRPM);

			previousTime = time(0);

			// Reset counters / channels
			ulCClear(daqDeviceHandle, pCounter);
			ulCClear(daqDeviceHandle, sCounter);
		}
		
		usleep(100000);
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

	fprintf(f, "\nTest Ended.\n");

	return 0;
}

