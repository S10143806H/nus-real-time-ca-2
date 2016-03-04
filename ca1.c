/*
EE 5903 - REAL TIME SYSTEMS
CA1 EXERCISE
AY 2015/2016
SEMESTER 2

Singapore, 16.02.2016

Student: Lukas Philipp Brunke
Student ID: A0149064A
 */

// libraries
#include "apples.h"
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <pthread.h>
#include <math.h>

// number of apples (for testing purposes)
int const numApples = 30;

// structures
struct photoAndTime
{
	PHOTO photo;
	struct timeval startingTime;
};

struct photo_msgbuf
{
	long mtype;
	struct photoAndTime photoTime;
};

struct qualAndTime
{
	QUALITY photoQ;
	struct timeval startingTime;
};

struct qualTime_msgbuf
{
	long mtype;
	struct qualAndTime qualTime;
};

// thread declarations
pthread_t photoid;
pthread_t processid;
pthread_t actuatorid;

// message queue declarations
key_t keyPhoto = 1234;
int msqidPhoto;
int sizePhoto = sizeof((struct photo_msgbuf*)0)->photoTime;

key_t keyProcess = 2345;
int msqidProcess;
int sizeProcess = sizeof((struct qualTime_msgbuf*)0)->qualTime;

void* runPhoto(void* p) { // photo thread implementation

	printf("Photo thread started\n");

	int counter = 0;

	do { // begin do-loop
		counter++;
		// wait until apple under camera
		wait_until_apple_under_camera();

		// get starting time of the whole process
		struct timeval startTime;
		gettimeofday(&startTime, NULL);

		// take photo of an apple
		PHOTO photo = take_photo();
		printf("----------CLICK!----------_%d_\n",counter);

		// send message with photo and time data
		struct photo_msgbuf bufPhoto = {1, {photo, startTime}};
		msgsnd(msqidPhoto, &bufPhoto, sizePhoto, 0);

		usleep(0.75*1000000); // sleep in order to wait for the next apple

	//} while(counter <= numApples);
	} while(more_apples() == 1); // end do-looop
	printf("Photo thread ended\n");

	return NULL; // exit thread
}

void* runProcess(void* p) { // process thread implementation

	printf("Processing thread started\n");

	int counter = 0;

	do { // begin do-loop
		counter++;

		// receive photo and time data
		struct photo_msgbuf bufPempty;
		msgrcv(msqidPhoto, &bufPempty, sizePhoto, 1, 0); 

		// process apple and get its quality
		QUALITY photoQuality = process_photo(bufPempty.photoTime.photo);

		struct timeval startTime = bufPempty.photoTime.startingTime;

		if (photoQuality == GOOD)
		{	
			printf("Quality is GOOD for apple %d\n", counter);
		} else if (photoQuality == BAD) {
			printf("Quality is BAD for apple %d\n", counter);
		} else {
			printf("Quality is UNKNOWN for apple %d\n", counter);
		}

		// send message with apple quaility and time data
		struct qualTime_msgbuf bufQualTime = {2, {photoQuality, startTime}};
		msgsnd(msqidProcess, &bufQualTime, sizeProcess, 0);

	//} while(counter <= numApples);
	} while(more_apples() == 1); // end do-looop

	printf("Processing thread ended\n");

	return NULL; // exit thread
}

void* runActuator(void* p) { // actuator thread implementation

	printf("Actuator thread started\n");

	int counter = 0;

	do { // begin do-loop
		counter ++;

		// receive message of apple quaility and time data
		struct qualTime_msgbuf bufQTempty;
		msgrcv(msqidProcess, &bufQTempty, sizeProcess, 2, 0);

		struct timeval startTime = bufQTempty.qualTime.startingTime;

		// get the current time
		struct timeval actuatorTime;
		gettimeofday(&actuatorTime, NULL);

		// calculate the time of the whole process
		double processTimeS = actuatorTime.tv_sec - startTime.tv_sec;
		printf("SECONDS: %f\n", processTimeS);
		double processTimeMS = actuatorTime.tv_usec - startTime.tv_usec;
		printf("MILlISECONDS: %f\n", processTimeMS);
		double processTime = processTimeS + processTimeMS/1000000;
		printf("PROCESSING TIME: %f\n", processTime);

		// decision making whether to discard or keep an apple
		if (processTime > 5.0) // if process time is greater, apple has not been processed
			// and should be discarded because there is a chance that the apple is bad and 
			// therefore turns even more apples bad. Also messages about apples which can't
			// be processed anymore should be discarded
		{
			printf("WAAAAYYY TOOOOO LOOOONG! %f\n", processTime);

			// calculate how many apples couldn't be processed
			double processTimeCeil = ceil(processTime);
			int numMessages2Discard = processTimeCeil - 5;
			struct qualTime_msgbuf bufQTempty[numMessages2Discard];

			double waitingTime;

			// calculate the time at which an apple has to be discarded
			if (actuatorTime.tv_usec >= startTime.tv_usec)
			{
				waitingTime = 1 - (actuatorTime.tv_usec - startTime.tv_usec);
			} else {
				waitingTime = actuatorTime.tv_usec - startTime.tv_usec;	
			}

			int i = 0;
			for (i; i < numMessages2Discard-1; ++i)
			{
				if (more_apples() == 1) 
				{
					// read unnecessary messages
					msgrcv(msqidProcess, &bufQTempty[i], sizeProcess, 2, 0);
					printf("MESSAGE %d READ\n", i + 1);

					// make actuator wait long enough
					if (i > 0) 
					{
						sleep(1);
					} else {
						usleep(fabs(waitingTime));
					}
					
					// discard unprocessed apples
					discard_apple();
					printf("--------------->DISCARDED------------>UNPROCESSED\n");
				}
			}
		} else if (bufQTempty.qualTime.photoQ == BAD) // discard bad apples
		{
			usleep((5.0-processTime)*1000000);
			discard_apple();
			printf("--------------->DISCARDED\n");
		}

	//} while(counter <= numApples);
	} while(more_apples() == 1); // end do-looop

	printf("Actuator thread ended\n");

	return NULL; // exit thread
}

// begin main
int main()
{	
	// create message queues
	msqidPhoto = msgget(keyPhoto, 0666 | IPC_CREAT);
	msqidProcess = msgget(keyProcess, 0666 | IPC_CREAT);

	// delete any open message queues from previous runs
	msgctl(msqidPhoto, IPC_RMID, NULL);
	msgctl(msqidProcess, IPC_RMID, NULL);

	msqidPhoto = msgget(keyPhoto, 0666 | IPC_CREAT);
	msqidProcess = msgget(keyProcess, 0666 | IPC_CREAT);

	start_test(); // start the test
	printf("Test started\n");

	// starting threads
	// camera thread
	pthread_create(&photoid,0,&runPhoto,0); // create thread
	// processors thread
	pthread_create(&processid,0,&runProcess,0); // create thread
	// actuator thread
	pthread_create(&actuatorid,0,&runActuator,0); // create thread

	// join threads
	pthread_join(photoid, NULL);
	pthread_join(processid, NULL);
	pthread_join(actuatorid, NULL);

	end_test(); // end the test
	printf("Test ended\n");

	// destroy message queues
	msgctl(msqidPhoto, IPC_RMID, NULL);
	msgctl(msqidProcess, IPC_RMID, NULL);

	return 0;
} // end main