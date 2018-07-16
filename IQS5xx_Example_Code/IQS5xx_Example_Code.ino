
/******************************************************************************
*   			...............,MMMMM...................
                ...............,MMMMM...................
                ....MM.........,MMMMM.........MM........
                ..MMMMM...... .,MMMMM........MMMMM......
                .MMMMM....... .,MMMMM. .... ..MMMMM ....
                MMMMM..........,MMMMM...........MMMM....
                MMMM.. .. ..................... MMMMD...
                MMMM.... .......................MMMMM...
                MMMM............................MMMM ...
                .MMMMO.........................MMMMM....
                .MMMMM8   ........ . ...  ....MMMMM ....
                ...=MMMMMM................DMMMMMM ......
                .... MMMMMMMM .........7MMMMMMM.........
                ......... MMMMMMMMMMMMMMMM~ ............
                ...............MMMMMMMMMMMMM............
                .................MMM$......MMMMMMMMMMMM.
                ................MMMM.........:MMMMMMM ..
                ....... . ........ . ..................
				
                   IQS5xx_B000 I2C Master example code
				   
   This program is free software; you can redistribute  it and/or modify it
   under  the terms of  the GNU General  Public License as published by the
   Free Software Foundation;  either version 2 of the  License, or (at your
   option) any later version.

   Azoteq (Pty) Ltd does not take responsibility for the use of this driver

   This driver is an example driver. It will need to be ported to
   the specific platform and for the specific case in which it is used.
   Copyright (C) 2016 Azoteq (Pty) Ltd                                       
   Name:              :   IQS5xx_B000 I2C Master example code
   Revision           :   V1.1
   Author             :   Azoteq
   Date               :   1/18/2016

                            Copyright by 
                           Azoteq (Pty) Ltd                              
                        Republic of South Africa                           
                                        
                         Tel: +27(0)21 863 0033                         
                         E-mail: info@azoteq.com  

       
    Connections between IQS5xx and Ardiuno for are shown below, please add pull 
	ups on the SDA and SCL if not present on the IQS5xx board 
		
			IQS5xx                            ARDUINO
			___________                _____________________
			|         |                |                     |
			|      RDY|----------------|DIGITAL PIN 2        |
			|      SDA|----------------|SDA                  |
			|      SCL|----------------|SCL                  |
			|    VDDHI|----------------|3v3                  |
			|      GND|----------------|GND                  |                                                
			-----------                -----------------------
******************************************************************************/
#include "IQS5xx.h"
#include "defs.h"
#include "I2C.h"
#include "Mouse.h"

#include <hidboot.h>
#include <usbhub.h>
#include <Mouse.h>

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#include <SPI.h>
#endif

#define MIDDLE_1 11
#define MIDDLE_2 12
#define LEFT_KEY 8
#define RIGHT_KEY 9

class MouseRptParser : public MouseReportParser
{
protected:
	void OnMouseMove	(MOUSEINFO *mi);
	void OnLeftButtonUp	(MOUSEINFO *mi);
	void OnLeftButtonDown	(MOUSEINFO *mi);
	void OnRightButtonUp	(MOUSEINFO *mi);
	void OnRightButtonDown	(MOUSEINFO *mi);
	void OnMiddleButtonUp	(MOUSEINFO *mi);
	void OnMiddleButtonDown	(MOUSEINFO *mi);
};

bool left_last=false, left_now=false;
bool right_last=false, right_now=false;
bool middle_1_last=false, middle_1_now=false;
bool middle_2_last=false, middle_2_now=false;
bool wheel_enabled=false;
int wheel_count=0;
#define WHEEL_RATE 35

int x_move, y_move;

void MouseRptParser::OnMouseMove(MOUSEINFO *mi)
{
#if 0
    Serial.print("dx=");
    Serial.print(mi->dX, DEC);
    Serial.print(" dy=");
    Serial.println(mi->dY, DEC);
#endif
    if(wheel_enabled){
      wheel_count+=mi->dY;
      int wheel_out = wheel_count/WHEEL_RATE;
      Mouse.move(0,0,-wheel_out);
      wheel_count = wheel_count - wheel_out*WHEEL_RATE;
    }else{
      //Mouse.move(mi->dX,mi->dY);
	  x_move = mi->dX;
	  y_move = mi->dY;
    }
    
};
void MouseRptParser::OnLeftButtonUp	(MOUSEINFO *mi)
{
    Serial.println("L Butt Up");
};
void MouseRptParser::OnLeftButtonDown	(MOUSEINFO *mi)
{
    Serial.println("L Butt Dn");
};
void MouseRptParser::OnRightButtonUp	(MOUSEINFO *mi)
{
    Serial.println("R Butt Up");
};
void MouseRptParser::OnRightButtonDown	(MOUSEINFO *mi)
{
    Serial.println("R Butt Dn");
};
void MouseRptParser::OnMiddleButtonUp	(MOUSEINFO *mi)
{
    Serial.println("M Butt Up");
};
void MouseRptParser::OnMiddleButtonDown	(MOUSEINFO *mi)
{
    Serial.println("M Butt Dn");
};

USB     Usb;
USBHub     Hub(&Usb);
HIDBoot<USB_HID_PROTOCOL_MOUSE>    HidMouse(&Usb);

MouseRptParser                               Prs;



uint8_t 	Data_Buff[44];
uint16_t	ui16SnapStatus[15], ui16PrevSnap[15];

//*****************************************************************************
//
//! General arduino device setup
//!                            
//! This function assumes that the IQS5xx device has already been configured in
//!	the recommended way.  That being that the parameters were determined using 
//!	the IQS5xx-B000 PC GUI, and then the configured settings were programmed 
//!	onto the device with the firmware, so that no further setup is required.  
//! See application note AZD087 on www.azoteq.com for more details.
//!	This function thus only handles a few minor setup actions. 				                         
//                                                      
//*****************************************************************************
void setup()
{
	// Setup serial baud rate
	//
	Serial.begin(115200);
	
	
#if !defined(__MIPSEL__)
    while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
#endif
    Serial.println("Start");
	
    if (Usb.Init() == -1)
        Serial.println("OSC did not start.");

    delay( 200 );
    HidMouse.SetReportParser(0, &Prs);
    
    pinMode(LEFT_KEY,OUTPUT);
    pinMode(RIGHT_KEY,OUTPUT);
    pinMode(MIDDLE_1,OUTPUT);
    pinMode(MIDDLE_2,OUTPUT);
    digitalWrite(LEFT_KEY, LOW);
    digitalWrite(RIGHT_KEY, LOW);
    digitalWrite(MIDDLE_1, LOW);
    digitalWrite(MIDDLE_2, LOW);
    Mouse.begin();
	
	//
	// Configure RDY pin
	//
	pinMode(RDY_PIN, INPUT);
	
	I2C_Setup();
	//
	// Clear the reset indication bit, so that from here on a reset can be 
	// detected if the bit becomes set
	//
	IQS5xx_AcknowledgeReset();
	//
	// Read the version and display on the serial terminal
	//
	IQS5xx_CheckVersion();
	//
	// End the communication window
	//
	Close_Comms();
	//Mouse.begin();
}

//*****************************************************************************
//
//! Arduino continuous loop function
//!                            
//! The loop function reads the data from the IQS5xx, and processes it under 
//! Process_XY function.	
//!		                         
//                                                      
//*****************************************************************************
void loop()
{
	uint8_t		ui8TempData[30], i;
	//
	// Wait for RDY to be set, this means that a communication window is 
	// available.  Then read the 'important' data from the IQS5xx (Address 
	// 0x000D to 0x0038).  This includes gesture status, system info flags and 
	// all XY data.  NOTE: if less multi-touches are enabled, then less XY
	// data can be read.  It would be good to configure the RDY pin as an
	// interrupt, and then trigger the i2c when the RDY interrupt sees a rising
	// edge.
	//
	Usb.Task();
	//RDY_wait();
	
	I2C_Read(GestureEvents0_adr, &Data_Buff[0], 44);

	if((Data_Buff[3] & SNAP_TOGGLE) != 0)
	{
		// If there was a change in a snap status, then read the snap status 
		// bytes additionally. Keep previous valus to identify a state change
		//
		I2C_Read(SnapStatus_adr, &ui8TempData[0], 30);
		for(i = 0; i < 15; i++)
		{
			ui16PrevSnap[i] = ui16SnapStatus[i];
			ui16SnapStatus[i] = ((uint16_t)(ui8TempData[2*i])<<8) + 
								 (uint16_t)ui8TempData[(2*i)+1];
		}
	}
	//
	// Terminate the communication session, so that the IQS5xx can continue 
	// with sensing and processing
	//
	Close_Comms();
	//
	// Process received data 
	//
	Process_XY();
}



