#include <SoftwareSerial.h> 					// Delete for use with boards without SoftwareSerial (Due, Zero)! (Then you also have to outcommand the line "#define SOFTWARESERIAL_AVAILABLE" in the herader file)
#include <RESOLino_Pt1000.h>

#define NR_OF_Pt1000_TERMINALS 8
#define UPDATE_INTERVAL 2000

double pt1000Data[NR_OF_Pt1000_TERMINALS]; 		// Array for measurement results.

unsigned long timestamp;						// Timestamp for the update interval.
int cycleNr;									// Nr of current cycle (for printValues).

SoftwareSerial ssPt1000(5, 6, false); 			// SoftwareSerial for Arduino Uno. Please check which pins can be used on other boards.
RESOLino_Pt1000 pt1000Shield(&ssPt1000);		// Create a new instance of RESOLino_Pt1000 and use SoftwareSerail for communication. 
// RESOLino_Pt1000 pt1000Shield(&Serial1);		// For Arduino Mega it is recommended to use HardwareSerial for communication.

SoftwareSerial ssPt1000_2(2, 3, false); 			// SoftwareSerial for Arduino Uno. Please check which pins can be used on other boards.
RESOLino_Pt1000 pt1000Shield_2(&ssPt1000_2);		// Create a second instance of RESOLino_Pt1000 and use SoftwareSerail for communication. 
// RESOLino_Pt1000 pt1000Shield_2(&Serial2);		// For Arduino Mega it is recommended to use HardwareSerial for communication.

void setup() {
	Serial.begin(57600);						// Initializing the Serial connection to the PC.
	pt1000Shield.init(19200);					// Initializing the connection with 19200 baud. See discription in the librarys header which baud is recommended.
	pt1000Shield.setPriorityTerminal(4);		// Setting the priority terminal. 
	pt1000Shield_2.init(19200);					// Initializing the connection with 19200 baud. See discription in the librarys header which baud is recommended.
	pt1000Shield_2.setPriorityTerminal(3);		// Setting the priority terminal. 
	timestamp = millis();
}

void loop() {
	if ((millis() - timestamp) > UPDATE_INTERVAL) {	// Update the measurement values every two seconds.
		// Read values from first Pt1000-Shield
		for (int i = 0; i < NR_OF_Pt1000_TERMINALS; i++) { // Loop to get all terminals.
			// Receive a new value of the terminal. Keep in mind, the terminals are numbered 1 to 8. 
			// Also setting the individual conversion function for this terminal.
			// Measurement result is stored in the global array.
			pt1000Shield.update(i + 1, &pt1000Data[i], pt1000Shield.Conversion_Pt1000_To_DegreeCentigrade, NULL); 
		}
		printValues(1);								// Print all received values.
		// Read values from second Pt1000-Shield
		for (int i = 0; i < NR_OF_Pt1000_TERMINALS; i++) { // Loop to get all terminals.
			// Receive a new value of the terminal. Keep in mind, the terminals are numbered 1 to 8. 
			// Also setting the individual conversion function for this terminal.
			// Measurement result is stored in the global array.
			pt1000Shield_2.update(i + 1, &pt1000Data[i], pt1000Shield_2.Conversion_Pt1000_To_DegreeCentigrade, NULL); 
		}
		printValues(2);								// Print all received values.
		cycleNr++;									// Increase cycle.
		timestamp = millis();						// Restart interval.
	}
}

void printValues(int shieldNr) {
	double value;
	for (int i = 0; i < NR_OF_Pt1000_TERMINALS; i++) {	// For each terminal, print formatted measurement result, or error message.
		value = pt1000Data[i];
		Serial.print("Shield-Nr: ");
		Serial.print(shieldNr);
		Serial.print("Cycle: ");
		Serial.print(cycleNr);
		Serial.print("\tTerminal: ");
		Serial.print(i + 1);
		// Check for errors
		if (value == -1.0) { 					// No connection
			Serial.println("\tThe RESOLino_Pt1000 didn't send a value.");
		} else if (value == 3383.8) {			// Equivalent to a raw Ohm value of 2147483.75. This Value means open terminal, no sensor/resistor connected.
			Serial.println("\tNo sensor connected.");
		} else {								// No errors occured, printing value.
			Serial.print("\tValue: ");
			Serial.print(value, 4);
			Serial.println(" °C");
		}
	}
}