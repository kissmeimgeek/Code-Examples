/**************************************************************

			COMMUNICATION CHANNELS SUPPORT FUNCTIONS

***************************************************************/

#include <rs232.h>
//#include <windows.h>
//#include <tcpsupp.h>
#include <formatio.h>
#include <utility.h>
#include <userint.h>
#include <ansi_c.h>
#include "vcommclient_exports.h"			
#include "FatTestGui.h"
#include "FatTestMain.h"

//DefineThreadSafeScalarVar(int, giExit, 0);

int CVICALLBACK Clb_Thrd_ReadTelnet (void *functionData);
short Thrd_ReadTelnet(LPVOID Handle, sTestData* LocalTestData);

/********************************************************************************
 *	Function: OpenConnectionRS232()												*
 *	Purpose:  This function establishes a comm port connection to an already 	*
 *            existing connection handle										*
 ********************************************************************************/
int	OpenConnectionRS232(int uut)
{
	//	Local variables
	int 	iStatus = 0;
	char	cValue[100] = "\0";
	char	cError[100] = "\0";
	
	// initialize the uut dppm com port 1
	if(hHandle1RS232 == NULL && uut == DPPM)
	{
		//	Get handle from Telnet plug-in
		hHandle1RS232 = GetRs232Handle(giComPortDPPM1, 9600);
		
		if(hHandle1RS232 != NULL)
		{
			SetResponseBufferSize(hHandle1RS232, 5000); //	Set buffer size
			SetTimeout(hHandle1RS232, 8); //	Set timeout	
		}
		else
			iStatus = 1;				
	}
	// initialize the uut asm com port 2
	if(hHandle2RS232 == NULL && uut == ASM)
	{
		//	Get handle from Telnet plug-in
		hHandle2RS232 = GetRs232Handle(giComPortASM1, 9600);  
		
		if(hHandle2RS232 != NULL)
		{
			SetResponseBufferSize(hHandle2RS232, 5000); //	Set buffer size
			SetTimeout(hHandle2RS232, 8); //	Set timeout	
		}
		else
			iStatus = 1;				
	}
	// initialize the captive dppm com port 3
	if(hHandle3RS232 == NULL && uut == DPPMCAP)
	{
		//	Get handle from Telnet plug-in
		hHandle3RS232 = GetRs232Handle(giComPortDPPM2, 9600);  
		
		if(hHandle3RS232 != NULL)
		{
			SetResponseBufferSize(hHandle3RS232, 5000); //	Set buffer size
			SetTimeout(hHandle3RS232, 8); //	Set timeout	
		}
		else
			iStatus = 1;				
	}
	// initialize the captive asm com port 4
	if(hHandle4RS232 == NULL && uut == ASMCAP)
	{
		//	Get handle from Telnet plug-in
		hHandle4RS232 = GetRs232Handle(giComPortASM2, 9600);  
		
		if(hHandle4RS232 != NULL)
		{
			SetResponseBufferSize(hHandle4RS232, 5000); //	Set buffer size
			SetTimeout(hHandle4RS232, 8); //	Set timeout	
		}
	}
	//#######################################################################
	//	Open connection to uut dppm or if handle is valid
	if(hHandle1RS232 != NULL && !giRS232Connected1 && uut == DPPM)
	{
		//	Proceed only if the connection is opened successfully
   		if(Connect(hHandle1RS232))
   		{
			giRS232Connected1 = 1;
   		}
   		else
   		{
			giRS232Connected1 = 0;   		
   			iStatus = 1;
		}
	}
	//	Open connection to uut dppm or if handle is valid
	if(hHandle2RS232 != NULL && !giRS232Connected2 && uut == ASM)
	{
		//	Proceed only if the connection is opened successfully
   		if(Connect(hHandle2RS232))
   		{
			giRS232Connected2 = 1;
   		}
   		else
   		{
			giRS232Connected2 = 0;   		
   			iStatus = 1;
		}
	}
	//	Open connection to uut dppm or if handle is valid
	if(hHandle3RS232 != NULL && !giRS232Connected3 && uut == DPPMCAP)
	{
		//	Proceed only if the connection is opened successfully
   		if(Connect(hHandle3RS232))
   		{
			giRS232Connected3 = 1;
   		}
   		else
   		{
			giRS232Connected3 = 0;   		
   			iStatus = 1;
		}
	}
	//	Open connection to captive asm or if handle is valid
	if(hHandle4RS232 != NULL && !giRS232Connected4 && uut == ASMCAP)
	{
		//	Proceed only if the connection is opened successfully
   		if(Connect(hHandle4RS232))
   		{
			giRS232Connected4 = 1;
   		}
   		else
   		{
			giRS232Connected4 = 0;   		
   			iStatus = 1;
		}
	}
	return iStatus;
}

/********************************************************************************
 *	WriteReadUntil()
 *
 *		This function sends a command over selected com channel.If desired the 
 *      function will monitor the response buffer for an expected prompt string 
 *      during a specified timout period. 
 *
 */
int	WriteReadUntil(LPVOID hHandle, char* readBuffer, char* cCommand, char* cExpectedString, int iTimeout)
{
	int 	iStatus = 0;
	int		iReadComplete = 0;
	char*	cpTemp;

	//	Init 1 second timeout counter
	gReadTimeOutCnt = 0;
	
	//	clear read buffer
	memset(gcReadBuffer,0,8000);
	
	if (strlen(cCommand) > 0)
	{
		//	Write command
		if (Write(hHandle, cCommand) == FAILURE)
		{
			iStatus = 1; // if write failure occurred, set the error status flag and exit
			goto exit;
		}
	}
	//	Write and then read until timeout or correct response string is returned.
   	while((!iReadComplete) && (gReadTimeOutCnt < iTimeout) && (!iStatus) && (strlen(cExpectedString) > 0))
   	{
		if (strlen(gcReadBuffer) > 7000)
			memset(gcReadBuffer, 0, 8000);
			
		//	Clear local read buffer memory
		memset(gcTelnetBuffer, 0, 5000);
		
   		//	Read response
		if (Read(hHandle, gcTelnetBuffer) == FAILURE) 
			iStatus = 1;
		
		//	write received data to log file if read buffer is not empty
		if (strlen(gcTelnetBuffer) > 0)
			WriteFile (giLogFileHandle,gcTelnetBuffer,StringLength(gcTelnetBuffer));
		
		// write data to the terminal window if open
		if(giDisplayTerminal)
			printf("%s",gcTelnetBuffer);
		
   		//	Append read buffers
		cpTemp = malloc(strlen(gcReadBuffer) + 1);
   		strcpy(cpTemp, gcReadBuffer);
		sprintf (gcReadBuffer,"%s%s",cpTemp, gcTelnetBuffer); 
		free(cpTemp); // free temp read buffer
		
		// search for expected pattern string
   		if(FindPattern (gcReadBuffer, 0, -1, cExpectedString, 0, 1) > -1)
   			iReadComplete = 1; // set read complete flag when search pattern is found
	}
exit:	
	//	If a read timeout occurred set the error flag.
	if(iReadComplete == 0 && (strlen(cExpectedString) > 0))
		iStatus = 1;

	return iStatus;
}

/*
 *	OpenTelnet()
 *
 *		This function opens up a telnet session 
 */
int OpenTelnet()
{
	int status = 0;
	char cIpAddr[20] = "\0";
	
	giTelnetConnected1 = 0; // clear telnet connected flag
	
	// get the captive asm ip address from the INI file
	Ini_GetStringIntoBuffer(gIniText,"Telnet","ADDR",  cIpAddr, sizeof(cIpAddr));
	
	if ((giHandle1Telnet = InetTelnetOpen (cIpAddr , 23, 0)) < 0)
		status++;
	else
		giTelnetConnected1 = 1; // set the telnet connected flag
	
	return status;
}

/*
 *	CloseConnection()
 *
 *		This function closes an open com port session 
 */
int CloseConnection(LPVOID hHandle)
{
	int status = 0;
	
    // close the com connection to the selected port
	if (hHandle)
	{
		if (hHandle == hHandle1RS232 && giRS232Connected1)
		{
			giRS232Connected1 = 0; // clear the dppm com connected flag
			Disconnect(hHandle); // close the com port
		}
		if (hHandle == hHandle2RS232 && giRS232Connected2)
		{
			giRS232Connected2 = 0; // clear the dppm com connected flag
			Disconnect(hHandle); // close the com port
		}
		if (hHandle == hHandle3RS232 && giRS232Connected3)
		{
			giRS232Connected3 = 0; // clear the dppm com connected flag
			Disconnect(hHandle); // close the com port
		}
		if (hHandle == hHandle4RS232 && giRS232Connected4)
		{
			giRS232Connected4 = 0; // clear the dppm com connected flag
			Disconnect(hHandle); // close the com port
		}
	}
	return status;
}

/*
 *	WriteCtrlC()
 *
 *		This function writes a Control-C to the uut 
 */
int WriteCtrlC()
{
	char ctrlC[4] = "\003"; // define Control-C escape character
	int status = 0;
	
	if (Write(hHandle1RS232, ctrlC) == FAILURE) // send second Ctrl-C character
		status = 1;
	Delay(0.25);
	if (Write(hHandle1RS232, ctrlC) == FAILURE) // send second Ctrl-C character
		status = 1;

	return status;
}

/*
 *	OpenAardvarkPort()
 *
 *	This function writes opens an Aardvark I2C device port
 *  Enables the I2C mode, sets the bit rate to 100kbs
 */
int OpenAardvarkPort(byte port, bool i2cEnable)
{
	int iStatus = 0;
	int res;
	
	giI2CConnected = 0; // clear the I2C connected flag
	
    // Open the device
    if ((hAardvarkHandle = aa_open(port)) < 1)
	{
		iStatus = 1;
		goto exit;
	}
	giI2CConnected = 1; // set the I2C connected flag
	
	if (i2cEnable)
	    // Ensure that the I2C subsystem is enabled
	    res = aa_configure(hAardvarkHandle,  AA_CONFIG_SPI_I2C);
	
	res = aa_i2c_monitor_disable(hAardvarkHandle);
	
	if (res) // if result flag is set, set the error status flag
		iStatus = 1;
	
	if (res = aa_i2c_bitrate(hAardvarkHandle, 100) != 100) // set the bit rate to 100kbs
		iStatus = 1; // set the error status flag if the bit rate is not equal to 100kbs
exit:
	return iStatus;
}

/*
 *	CloseAardvarkPort()
 *
 *	This function closes the Aardvark I2C device port
 */
void CloseAardvarkPort(Aardvark handle)
{
    // Close the aardvark device port
    aa_close(handle);
}

/*
 *	ViOpenXantrex()
 *
 *	This function opens a vi session to both xantrex power supplies
 */
void ViOpenXantrex()
{
  	ViSession defaultRM1, defaultRM2;
	char cInstDesc1[20] = "\0";
	char cInstDesc2[20] = "\0";
  
	// Open session to the Xantrex power supply #1
	strcpy(cInstDesc1,"GPIB0::2::INSTR");
	viOpenDefaultRM (&defaultRM1);
	viOpen (defaultRM1, cInstDesc1, VI_NULL,VI_NULL, &hVIPsu1);
	WriteGPIB(hVIPsu1,"*RST\n");
	// Open session to the Xantrex power supply #2
	strcpy(cInstDesc2,"GPIB0::3::INSTR");
	viOpenDefaultRM (&defaultRM2);
	viOpen (defaultRM2, cInstDesc2, VI_NULL,VI_NULL, &hVIPsu2);
	WriteGPIB(hVIPsu2,"*RST\n");
}

/*
 *	ViCloseXantrex()
 *
 *	This function closes the sessions to the xantrex power supplies
 */
void ViCloseXantrex()
{
	viClose (hVIPsu1);
	viClose (hVIPsu2);
}

/*
 *	ViOpenAgilentSwitch()
 *
 *	This function opens a vi session to the Agilent 34970A Data Aquisition /Switch unit
 */
void ViOpenAgilentSwitch()
{
  	ViSession defaultRM1;
	char cInstDesc1[20] = "\0";
  
	// Open session to the Agilent 34970A Data Aquisition /Switch unit
	strcpy(cInstDesc1,"GPIB0::5::INSTR");
	viOpenDefaultRM (&defaultRM1);
	viOpen (defaultRM1, cInstDesc1, VI_NULL,VI_NULL, &hViSwitch);
	WriteGPIB(hViSwitch,"*RST\n");
}

/*
 *	ViCloseAgilentSwitch()
 *
 *	This function closes the sessions to the agilent unit
 */
void ViCloseAgilentSwitch()
{
	viClose (hViSwitch);
}

/*
 *	ViOpenAgilentDmm()
 *
 *	This function opens a vi session to the Agilent 34401A DMM
 */
void ViOpenAgilentDmm()
{
  	ViSession defaultRM1;
	char cInstDesc1[20] = "\0";
  
	// Open session to the Agilent 34970A Data Aquisition /Switch unit
	strcpy(cInstDesc1,"GPIB0::4::INSTR");
	viOpenDefaultRM (&defaultRM1);
	viOpen (defaultRM1, cInstDesc1, VI_NULL,VI_NULL, &hViDmm);
	WriteGPIB(hViDmm,"*RST\n");
}

/*
 *	ViCloseAgilentDmm()
 *
 *	This function closes the sessions to the agilent 34401A DMM
 */
void ViCloseAgilentDmm()
{
	viClose (hViDmm);
}

/*
 *	WriteGPIB()
 *
 *	This function writes a command string to the gpib device
 */
void WriteGPIB(ViSession handle, ViString cmdStr)
{
	int iStatus = 0;
	
	// write the command string to the device
	iStatus = viPrintf (handle, cmdStr);
}

/*
 *	ReadGPIB()
 *
 *	This function reads back a value string from a previous write
 */
void ReadGPIB(ViSession handle, char* data)
{
	// read back the results from the previous write
	viScanf(handle, data);
}
