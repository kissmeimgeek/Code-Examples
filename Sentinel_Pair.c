// Sentinel-to-Buckhorn Pairing Tool

// include header files
#include "Power_On.h"
#include <windows.h>
#include <formatio.h>
#include "asynctmr.h"
#include <userint.h>
#include <utility.h>
#include <rs232.h>
#include <ansi_c.h>
//#include <mscorlib.h>
#include "inifile.h"
#include "MavCtrlDll.h"
#include <Sentinel_Pair.h>
#include "Sentinel_Scan.h"
#include "Button_Sequence.h"

// misc defines
#define DISABLE 0
#define ENABLE 1
#define DIMMED 1
#define NORMAL 0
#define OFF 0
#define ON 1
#define OPEN 1
#define CLOSE 1

// variable declarations
time_t gtStart;
time_t gtEnd; 
time_t gtCurrentTime;
int  giMainPanel;
int  giScanPanel;
int  cmd_type = 0;
int  giTick;
int  giBlinkTimer;
int  giLogFileOpen;
int  giLogFileHandle;
int  giScanData;
int  giBtnSeqPanel;
int  giPwrOnPanel;
int  giButtonPressFlg;
int  giBtnPress;
int  giLogFlag;
int  giSeqFlag;
int  giBaudRate;
int  giComPort;
int  giTimeOutCnt;
int  gtStartTime;
int  giTimerId;
char gcComPort[10];
char gcPairCmdStr[50];
char gcInputBuffer[2000];
char gcDateTimeBuffer[30];
char gcLogFilePath[75];
char gcSentSerNum[20];
char gcBuckSerNum[20];
char gcSentinelVer[20];
char gcBuckhornVer[20];
IniText giIniText;

// forward function declarations
int ScanUUTData(void);
int LogFile(int state);
int SendMavlinkCmd(int iPort, char * cCmdStr, int iTimeout);
//int CVICALLBACK ScanUUTData_CB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
//static int CVICALLBACK BlinkTimerCallback(void * callbackData, CDotNetHandle state);


int PairingStart()
{
	int i, x;
	int iStatus;
	int iResult;
	char* cMessageStr;
	char* cSerNum;
	char* cFwVer;
	
	cMessageStr = (char *)malloc(300);
	cSerNum = (char *)malloc(20);
	cFwVer = (char *)malloc(20);
	
	int iErrStatus = 0;
	char cDisplayStr[500] = "\0";
	giTick = 0;
	giBtnPress = 0;
	gtStartTime = time(NULL); // get the test start time for the elapsed time timer display
	
	SetCtrlAttribute (giMainPanel, MAIN_EXIT, ATTR_DIMMED, DIMMED);    // dim the exit command button
	SetCtrlAttribute (giMainPanel, MAIN_START, ATTR_DIMMED, DIMMED);   // dim the start command button
	SetCtrlAttribute (giMainPanel, MAIN_STATUS, ATTR_OFF_TEXT,"PAIRING");
	SetCtrlAttribute (giMainPanel, MAIN_STATUS, ATTR_OFF_COLOR, VAL_YELLOW);
	SetCtrlVal(giMainPanel, MAIN_SENTINELSN, ""); // clear the sentinel serial number text box
	SetCtrlVal(giMainPanel, MAIN_SENTINELFW, ""); // clear the sentinel fw version text box
	SetCtrlVal(giMainPanel, MAIN_BUCKHORNSN, ""); // clear the buckhorn serial number text box
	SetCtrlVal(giMainPanel, MAIN_BUCKHORNFW, ""); // clear the buckhorn fw version text box
	SetCtrlVal(giMainPanel, MAIN_ELAPSEDTIME, "00:00:00"); // reset the elapsed timer display
	SetAsyncTimerAttribute (giTimerId, ASYNC_ATTR_ENABLED, ENABLE); // enable the elapsed timer
Retry:	
	SetBreakOnLibraryErrors (0);
	
	int portOpenRet = mavlinkAPI_Open(giComPort); // open the uart mavlink interface port
	
	SetBreakOnLibraryErrors (1);
	
	if( portOpenRet < 0) // if com port is not open, display an error popup
	{
		sprintf(cMessageStr,"Check the following:\n1. Close any PuTTY or TeraTerm Console windows that may be open.\n2. Make sure that the Serial cable is connected to the T-Connector UART port.\n\nClick the 'Yes' button to Retry opening the port!\n\nClick the 'No' button to Exit the test!");
		iResult = MessageBox(NULL, cMessageStr,"DM368 Debug Console Connection", MB_YESNO | MB_ICONINFORMATION | MB_TOPMOST);  
		if(iResult == IDNO) // if no is pressed, abort the test and display an com port error message in the text box 
		{
			sprintf(cDisplayStr,"\nERROR: Unable to open COM port %i, test aborted!", giComPort);
			if(giLogFileOpen) // write data to log file
				WriteFile(giLogFileHandle, cDisplayStr, strlen(cDisplayStr));
			SetCtrlAttribute (giMainPanel, MAIN_STATUS, ATTR_OFF_TEXT,"COM ERROR");
			SetCtrlAttribute (giMainPanel, MAIN_STATUS, ATTR_OFF_COLOR, VAL_RED);
			iErrStatus = 1; // stop the testing if there is a com port error
			goto Exit;
		}
		else if(iResult == IDYES) goto Retry; // retry opening the serial port
	}
	ResetTextBox(giMainPanel, MAIN_TEXTBOX,""); // clear out the text box
	
	if(giLogFlag) // if log file set, open the log file
		LogFile(OPEN); // open the pairing log file
	
	giBtnPress = 0;
	if(giSeqFlag) // if sequence popup flag is set, open the panel
	{
		InstallPopup(giPwrOnPanel); // open the pairing process panel
		
		while (!giBtnPress) // wait here until the 'OK' button is pressed
			ProcessSystemEvents();
	}
	if(giLogFileOpen) // write data to log file
		WriteFile(giLogFileHandle, cDisplayStr, strlen(cDisplayStr));
	
	sprintf(cDisplayStr,"WAIT: Wired pairing can take up to 20 seconds to complete..................\n");
 	InsertTextBoxLine(giMainPanel, MAIN_TEXTBOX, -1, cDisplayStr);
	
	iStatus = SendMavlinkCmd(giComPort,"t api wired_pair start", 20);  // start the wired pairing process
	
	if (iStatus == 3) goto Bypass; // if error, go to bypass and record the error
	
	sprintf(cDisplayStr,"WAIT: Delay of 5 seconds between the Pairing and the reading of the FW Version and Serial Numbers........\n");
 	InsertTextBoxLine(giMainPanel, MAIN_TEXTBOX, -1, cDisplayStr);
	ProcessSystemEvents();
	Delay(2.0); // delay between paring completion and reading the information

	iStatus = SendMavlinkCmd(giComPort,"adb shell mft -t", 10); // verify sentinel fw version
	
//	iStatus = SendMavlinkCmd(giComPort,"t api system version", 10); // verify sentinel fw version 
	
	if(iStatus == 3) goto Bypass;
	
	x = FindPattern(gcInputBuffer, 0, -1, "E51", 1, 0); // verify the sentinel serial number
	if(x < 0) 
	{
		SetCtrlVal(giMainPanel, MAIN_SENTINELSN,"Unknown!"); // display unknown for the sentinel serial number
		sprintf(cMessageStr,"Unable to read the Sentinel Serial Number.\n\nThe Pairing process was successful!\n\nClick the 'OK' button to Continue!");
		MessageBox(NULL, cMessageStr,"Sentinel FW Version", MB_OK | MB_ICONINFORMATION | MB_TOPMOST);
		goto Next1;
	}
	for (i=0; i<14; i++, x++)
		cSerNum[i] = gcInputBuffer[x];
	SetCtrlVal(giMainPanel, MAIN_SENTINELSN, cSerNum); // display the sentinel serial number
Next1:	
	x = FindPattern(gcInputBuffer, 0, -1, "SEN", 1, 0); // verify the sentinel fw version
	if(x < 0) 
	{
		SetCtrlVal(giMainPanel, MAIN_SENTINELFW,"Unknown!"); // display unknown for the sentinel fw version
		sprintf(cMessageStr,"Unable to read the Sentinel FW Version Number.\n\nThe Pairing process was successful!\n\nClick the 'OK' button to Continue!");
		MessageBox(NULL, cMessageStr,"Sentinel FW Version", MB_OK | MB_ICONINFORMATION | MB_TOPMOST);
		goto Next2;
	}
	for (i=0; i<15; i++, x++)
		cFwVer[i] = gcInputBuffer[x];
	SetCtrlVal(giMainPanel, MAIN_SENTINELFW, cFwVer); // display the sentinel fw version
Next2:	
	Delay(0.5);
	
	iStatus = SendMavlinkCmd(giComPort,"adb shell mft -i", 10); // verify buckhorn serial number 
	x = FindPattern(gcInputBuffer, 0, -1, "E51", 1, 1); 
	if(x < 0) 
	{
		SetCtrlVal(giMainPanel, MAIN_BUCKHORNSN,"Unknown!"); // display unknown for the buckhorn serial number
		sprintf(cMessageStr,"Unable to read the Buckhorn Serial Number.\n\nThe Pairing process was successful!\n\nClick the 'OK' button to Continue!");
		MessageBox(NULL, cMessageStr,"Sentinel FW Version", MB_OK | MB_ICONINFORMATION | MB_TOPMOST); 
		goto Next3;
	}
	for (i=0; i<14; i++, x++)
		cSerNum[i] = gcInputBuffer[x];
	SetCtrlVal(giMainPanel, MAIN_BUCKHORNSN, cSerNum); // display the buckhorn serial number
Next3:	
	x = FindPattern(gcInputBuffer, 0, -1, "BCK", 1, 1); 
	if(x < 0) 
	{
		SetCtrlVal(giMainPanel, MAIN_BUCKHORNFW,"Unknown!"); // display unknown for the buckhorn fw version
		sprintf(cMessageStr,"Unable to read the Buckhorn FW Version Number.\n\nThe Pairing process was successful!\n\nClick the 'OK' button to Continue!");
		MessageBox(NULL, cMessageStr,"Sentinel FW Version", MB_OK | MB_ICONINFORMATION | MB_TOPMOST);  
		goto Bypass;
	}
	for (i=0; i<15; i++, x++)
		cFwVer[i] = gcInputBuffer[x];
	SetCtrlVal(giMainPanel, MAIN_BUCKHORNFW, cFwVer); // display the buckhorn fw version
Bypass:	
	if((FindPattern(gcInputBuffer, 0, -1,"SUCCESS", 1, 1) < 0) || iStatus == 3)  // check for Success or Fail
	{
		SetCtrlAttribute (giMainPanel, MAIN_STATUS, ATTR_OFF_TEXT,"FAIL");
		SetCtrlAttribute (giMainPanel, MAIN_STATUS, ATTR_OFF_COLOR, VAL_RED);
	}
	else
	{
		SetCtrlAttribute (giMainPanel, MAIN_STATUS, ATTR_OFF_TEXT,"SUCCESS");
		SetCtrlAttribute (giMainPanel, MAIN_STATUS, ATTR_OFF_COLOR, VAL_GREEN);
		iErrStatus = 1;
	}
Exit:	
	mavlinkAPI_Close(giComPort); // close the gimbal uart port
	if(giLogFlag) // if log file set, close the log file
		LogFile(CLOSE); // close the pairing log file
	SetAsyncTimerAttribute (giTimerId, ASYNC_ATTR_ENABLED, DISABLE);
	SetCtrlAttribute (giMainPanel, MAIN_EXIT,  ATTR_DIMMED, NORMAL);    // dim the exit command button
	SetCtrlAttribute (giMainPanel, MAIN_START, ATTR_DIMMED, NORMAL);    // dim the start command button
	
	free(cMessageStr);
	free(cSerNum);
	free(cFwVer);
	
	return iErrStatus;
}

int SendMavlinkCmd(int iPort, char * cCmdStr, int iTimeout)
{
	int iStatus = 0;
	float fTimeout_ms;
	char* cDisplayStr;
	DWORD t_start, t_now;
	
	cDisplayStr = (char *)malloc(500);
	
	iTimeout = 1000 * iTimeout;
	
	sprintf(cDisplayStr,"DM36X> %s", cCmdStr); // display the dm36x prompt
 	InsertTextBoxLine(giMainPanel, MAIN_TEXTBOX, -1, cDisplayStr);
	ProcessSystemEvents();

	mavlinkAPI_Query(iPort, cmd_type, 100000, cCmdStr); // send the encoder command type, test command and com port
	
	t_start = GetTickCount(); // get the start time
	
	while(1) // loop here until a success/fail command response is returned, or a timeout occurs
	{
		iStatus = mavlinkAPI_GetCmdStatus(); // get the command status
		Sleep(1000); 
		ProcessSystemEvents();
		
		// status = 1, still running
		// status = 2, done with pass
		// status = 3, done with fail
		if(iStatus == 2 || iStatus == 3)
		{
			break; // if status = 2 or 3, test is finished
		}
		t_now = GetTickCount(); 
		fTimeout_ms = (float)(t_now - t_start) / 1000.0;
		
		if(t_now - t_start > iTimeout) // check for test timeout
		{
			sprintf(cDisplayStr,"\nERROR: Pairing setup exceeded %0.2f seconds\n", fTimeout_ms);
 			InsertTextBoxLine(giMainPanel, MAIN_TEXTBOX, -1, cDisplayStr);
			sprintf(cDisplayStr,"\nINFO: Make sure that the Sentinel and the Buckhorn are turned on!\n\nVerify that all cables are connected to the Sentinel and Buckhorn.\n\nRetry the Pairing process if power is off and/or any cables are not connected.\n");
 			InsertTextBoxLine(giMainPanel, MAIN_TEXTBOX, -1, cDisplayStr);
			iStatus = 3;
			goto Bypass;
		}
		ProcessSystemEvents();
	}
	sprintf(gcInputBuffer, mavlinkAPI_GetCmdResp());  // get the command response returned from the Sentinel

 	InsertTextBoxLine(giMainPanel, MAIN_TEXTBOX, -1, gcInputBuffer); // display the response in the gui window
Bypass:	
	free(cDisplayStr);
	
	return iStatus;
}

/********************************************************************************
 *  Function: TestTimer_CB		  												*
 *  Purpose:  This callback function provides an asynch timer for the elapsed   *
 *            test time and other functions that need a timeout counter.		*
 ********************************************************************************/
int CVICALLBACK TestTimer_CB (int reserved, int timerId, int event, void *callbackData, int eventData1, int eventData2)
{
	int seconds;
	int minutes;
	int hours;
	char* cTimeString;  // Buffer for timer string
	
	cTimeString = (char *)malloc(10);

	switch (event)
	{
		case EVENT_TIMER_TICK:
			
			time (&gtCurrentTime);  // Set the current time
			
			hours = ((gtCurrentTime - gtStartTime) % 86400); // Elapsed time since start
			seconds = hours % 60;  	// Calculate seconds
			hours /= 60;
			minutes = hours % 60;  	// Calculate minutes
			hours /= 60;
			// Format the timer string for output
			Fmt (cTimeString, "%d[w2p0]:%d[w2p0]:%d[w2p0]", hours, minutes, seconds);
																			
			// Set the elapsed time on the display
			SetCtrlVal(giMainPanel, MAIN_ELAPSEDTIME, cTimeString);
	}
	ProcessSystemEvents();  // Process system events
	free(cTimeString);
	
	return 0;
}

/********************************************************************************
 *  InitAsyncTimer()															*
 *  Purpose: This function creates and initializes the Async timer.    			*
 ********************************************************************************/
int InitAsyncTimer()
{
	char* cErrStr;
	
	cErrStr = (char *)malloc(50);
	
	// initialize the async elapsed test timer for 1/2 second intervals
    giTimerId = NewAsyncTimer (0.5, -1, DISABLE, TestTimer_CB, NULL);
    if (giTimerId <= 0)
   	{
		sprintf(cErrStr, "ERROR: Asynchronous Timer could not be created!");
		MessageBox(NULL, cErrStr,"Asynchronous Timer ERROR",MB_OK | MB_ICONERROR | MB_TOPMOST);     	
		giBlinkTimer = 0;
		return -1;
   	}
	free(cErrStr);
	
	return 0;
}

/********************************************************************************
 *  Function: CreateDateTimeStr													*
 *  Purpose:  This function creates a date and time string.						*
 ********************************************************************************/
char* CreateDateTimeStr()
{
	double currDateTime;
	int bufferLen = 30;

	GetCurrentDateTime (&currDateTime); // get the current date and time
	
	// create the date and time string for gui display
	FormatDateTimeString (currDateTime,"%m%d%y%H%M%S", gcDateTimeBuffer, bufferLen);
	
	return gcDateTimeBuffer;
}

/********************************************************************************
 *  GetConfigData()																*
 *  Purpose: This function opens and loads test configuration INI file data.    *
 ********************************************************************************/
int GetConfigData()
{
//	char tmpBuf[25] = "\0";
	int  error = 0;
	
	// create the ini file handle
	giIniText = Ini_New (1);
	
	// read in the ini config data
	error = Ini_ReadFromFile (giIniText, "C:\\GoPro\\Sentinel_Pair_Config.ini"); 
	if (error)																		
		goto Exit;
	
	Ini_GetInt (giIniText,"Flags","LOG",  &giLogFlag); // load the 
	Ini_GetInt (giIniText,"Flags","SEQ",  &giSeqFlag);
	
	// get and save the baud rates and external psu flag integer values
	Ini_GetInt (giIniText,"Baud Rate","BAUD", &giBaudRate);
	
	// get and save the serial rs232 COM port assignments
	Ini_GetInt (giIniText,"Com Ports","PORT", &giComPort);
	
	// get and save test prompts
	Ini_GetStringIntoBuffer(giIniText,"Pair Command","PAIR", gcPairCmdStr, sizeof(gcPairCmdStr));
	
	// get the sentinel and buckhorn fw version numbers
	Ini_GetStringIntoBuffer(giIniText,"FW Version","SENT", gcSentinelVer, sizeof(gcSentinelVer));
	Ini_GetStringIntoBuffer(giIniText,"FW Version","BUCK", gcBuckhornVer, sizeof(gcSentinelVer));
	
	// get and display the GUI version number
//	sprintf(tmpBuf,"Version %s",gcGuiVer);
Exit:	
	return error;
}
 
/*
 *  LogFile() 														
 *  																	
 *  This function opens or closes the serial capture log file.						
 */
 int LogFile(int state)
 {
	char* cTmpStr;
	char* cDirName;
	
	cTmpStr = (char *)malloc(100);
	cDirName = (char *)malloc(10);
	
	sprintf(cDirName,"Sentinel");
			
	if (state == OPEN) // if state is Open, then create and open the new logfile
	{
		CreateDateTimeStr(); // generate the date and time
		sprintf(gcLogFilePath,"C:\\GoPro\\Pair_Log_Files\\PairLogFile_%s_%s_%s.txt", gcSentSerNum, gcBuckSerNum, gcDateTimeBuffer);
		sprintf(cTmpStr,";########## Start of the %s Pairing logfile for Sentinel SN: %s and Buckhorn SN: %s##########\n", cDirName, gcSentSerNum, gcBuckSerNum);
		giLogFileHandle = OpenFile (gcLogFilePath, VAL_READ_WRITE, VAL_TRUNCATE, VAL_ASCII);
		WriteLine(giLogFileHandle,cTmpStr,StringLength(cTmpStr));
		giLogFileOpen = TRUE;
	}
	else if (state == CLOSE)
	{
		if (giLogFileOpen) // verify that the logfile is open
		{
			giLogFileOpen = FALSE;
			CloseFile(giLogFileHandle);
		}
	}
	free(cTmpStr);
	free(cDirName);
	
	return 0;
 }

int CVICALLBACK StartPairing_CB (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			PairingStart(); // start the Sentinel to Buckhorn pairing process
			break;
	}
	return 0;
}
																  
int CVICALLBACK PowerOk_CB (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			giBtnPress = 1;
			RemovePopup(giPwrOnPanel); // close the power ok popup window
			break;
	}
	return 0;
}

int CVICALLBACK Exit_CB (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			QuitUserInterface(0); // exit the gui
			break;
	}
	return 0;
}

// main calling routine
int main (int argc, char *argv[])
{
	if (InitCVIRTE (0, argv, 0) == 0)
		return -1;	/* out of memory */
	if ((giMainPanel = LoadPanel (0, "Sentinel_Pair.uir", MAIN)) < 0) // load the main panel gui
		return -1;
	if ((giPwrOnPanel = LoadPanel (0, "Sentinel_Pair.uir", POWER)) < 0) // load the button sequence popup panel
		return -1;																	   
	
	GetConfigData();  // load configuration data from ini file
	
	InitAsyncTimer(); // initialize the elapsed time timer
	
	// insert the instructions list into the text box
 	InsertTextBoxLine(giMainPanel, MAIN_TEXTBOX, -1,"============= Sentinel-to-Buckhorn Wired Pairing Ver 1.0.0.0 ============");
 	InsertTextBoxLine(giMainPanel, MAIN_TEXTBOX, -1,"Perform the following steps to setup, start and complete the Pairing process:");
 	InsertTextBoxLine(giMainPanel, MAIN_TEXTBOX, -1,"1. USB cable(1) from PC to gullet assembly UART port is always connected.");
 	InsertTextBoxLine(giMainPanel, MAIN_TEXTBOX, -1,"2. USB cable(2) from Gullet asssembly USB port always connected. ");
 	InsertTextBoxLine(giMainPanel, MAIN_TEXTBOX, -1,"3. Connect gullet assembly into Sentinel Coyote gimbal.");
 	InsertTextBoxLine(giMainPanel, MAIN_TEXTBOX, -1,"4. Connect USB cable(2) to Buckhorn USB port on front panel");
 	InsertTextBoxLine(giMainPanel, MAIN_TEXTBOX, -1,"5. Insert (slide in) battery into Sentinel");
 	InsertTextBoxLine(giMainPanel, MAIN_TEXTBOX, -1,"6. Power on Sentinel by pressing the power button and hold until the Status LED");
 	InsertTextBoxLine(giMainPanel, MAIN_TEXTBOX, -1,"    goes to a blinking Non-White color.");
 	InsertTextBoxLine(giMainPanel, MAIN_TEXTBOX, -1,"7. Power on Buckhorn by pressing the power button momentarally and wait until the");
 	InsertTextBoxLine(giMainPanel, MAIN_TEXTBOX, -1,"    LCD screen displays the 'FLIGHT SWIPE UP TO START' message");
 	InsertTextBoxLine(giMainPanel, MAIN_TEXTBOX, -1,"8. Enable the pairing process on Buckhorn by pressing the following buttons");
 	InsertTextBoxLine(giMainPanel, MAIN_TEXTBOX, -1,"    sequentially (in order), Start, then Home, then Shutter, then Mode.");
  	InsertTextBoxLine(giMainPanel, MAIN_TEXTBOX, -1,"9. Launch the pairing process by clicking on the GUI's, Start Pairing button");
 	InsertTextBoxLine(giMainPanel, MAIN_TEXTBOX, -1,"10. The Pairing will finish and indicate either a Success or Fail");
 	InsertTextBoxLine(giMainPanel, MAIN_TEXTBOX, -1,"11. Power off the Buckhorn by holding down the Power button for 3 seconds");
 	InsertTextBoxLine(giMainPanel, MAIN_TEXTBOX, -1,"12. Disconnect the USB cable(2) from the Buckhorn");
  	InsertTextBoxLine(giMainPanel, MAIN_TEXTBOX, -1,"13. Remove (slide out) battery from Sentinel");
 	InsertTextBoxLine(giMainPanel, MAIN_TEXTBOX, -1,"14. Disconnect the gullet assembly from the Sentinel");
 	InsertTextBoxLine(giMainPanel, MAIN_TEXTBOX, -1,"");
// 	InsertTextBoxLine(giMainPanel, MAIN_TEXTBOX, -1,"When ready, click the 'Start Pairing Process' button to start the wired pairing.");

	DisplayPanel (giMainPanel); // display the gui panel
	RunUserInterface ();
	DiscardPanel (giMainPanel);
	return 0;
}
