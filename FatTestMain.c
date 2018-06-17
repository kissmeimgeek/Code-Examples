/*
 *************************************************************
 
  Originator: Mike Hammersley
  File: FatTestMain.c
  Type: (C Source)
  Version: See SourceSafe for version tree.
  
  Description: This file contains all C functions used 
  	by the functional test solutions for the XXXXX product 
  	family. This software provides a general system test
 	interface for all XXXXX products.
  		
 **************************************************************/

#include <windows.h>
#include "asynctmr.h"
#include <utility.h>
#include <userint.h>
#include <ansi_c.h>
#include <cvi_db.h>
#include <lowlvlio.h>
#include "pwctrl.h"
#include "FatTestGui.h"
#include "FatTestMain.h"

#define MAX_NUM_THREADS 6

int giCheckAll = 0;

//
// ##### main program entry point ######
//
int main (int argc, char *argv[])
{
	int  status;
	char cString[100] = "\0";
	int data;
	
	//	Telnet/Comm global variables
	hHandle1Telnet 	= NULL;
	hHandle1RS232 	= NULL;
	
	if (InitCVIRTE (0, argv, 0) == 0)
		return -1;	/* out of memory */
	if ((gMainPanel = LoadPanel (0, "FatTestGui.uir", PANEL)) < 0)
		return -1;
	
	GetPanelAttribute (gMainPanel, ATTR_TOP,  &giTopPos);
	GetPanelAttribute (gMainPanel, ATTR_LEFT, &giLeftPos);
	
	//	Set defaults for the display window.
	SetStdioPort 				(CVI_STDIO_WINDOW);
	SetStdioWindowPosition 		(100, 100 + 455);
	SetStdioWindowOptions 		(1000, 0, 0);
	SetStdioWindowSize 			(579, 420);
	SetStdioWindowVisibility 	(0);	

//	SetPanelAttribute (giTestLog, ATTR_LEFT, giLeftPos + 500);
//	DisplayPanel(giTestLog); // display the main panel

	giPanelScanData = LoadPanel (0, "SysTestGui.uir", UUTDATA); // load the uut data entry panel
	
    giTestList = LoadPanel (0, "SysTestGui.uir", SCANLIST); // load the scanned data test list panel
	
	giMenuBar = GetPanelMenuBar (gMainPanel);
	InstallMenuCallback (giMenuBar, MENUBAR_FILE_ENABLE, About_CB, &data);
	InstallMenuCallback (giMenuBar, MENUBAR_FILE_ENABLE, EnableSingleTest_CB, &data);

//  Create a threadpool for all of our multi-thread functions
	CmtNewThreadPool(MAX_NUM_THREADS, &giThreadPoolHandle);
	
	GetCfgDataFromIniFile(); // get configuration data from INI file
	
	InitAsyncTimer();		 // create 1 second asynchronous timer

	Login(); // open login panel for username/password entry and verification

	ConfigureTreeToListbox(); // configure initial tree box to look like a list box

	DisplayPanel(gMainPanel); // display the main panel
	
	RunUserInterface();	  // run the GUI and wait for the start test event
	
	DiscardPanel (gMainPanel);
	
	return 0;
}

/*------------------------------------------------------------------------*/
/*  Configures a tree to look and behave like a listbox.                  */
/*------------------------------------------------------------------------*/
int ConfigureTreeToListbox()
{
	char cString[100] = "\0";
	
	SetMenuBarAttribute(giMenuBar, MENUBAR_FILE_ENABLE, ATTR_DIMMED, 1); // dim the single test enable item
    SetCtrlAttribute(gMainPanel, PANEL_TESTLIST, ATTR_SHOW_CONNECTION_LINES, 0);
    SetCtrlAttribute(gMainPanel, PANEL_TESTLIST, ATTR_SHOW_PLUS_MINUS, 0);
    SetCtrlAttribute(gMainPanel, PANEL_TESTLIST, ATTR_FULL_ROW_SELECT, 0);
    SetCtrlAttribute(gMainPanel, PANEL_TESTLIST, ATTR_ENABLE_POPUP_MENU, 0);
	
	sprintf(cString,"Ready to start the DPPM functional test..........!");
	InsertListItem (gMainPanel, PANEL_TESTLIST, 0, cString, 0); // insert a "Ready to test" message
	InsertListItem (gMainPanel, PANEL_TESTLIST, 1,"", 0);       // insert a blank line
	sprintf(cString,"Select a UUT from the \"UUT Type:\" drop down list to the left.");
	InsertListItem (gMainPanel, PANEL_TESTLIST, 2, cString, 0); // insert operator prompt #1
	InsertListItem (gMainPanel, PANEL_TESTLIST, 3,"", 0);       // insert a blank line
	sprintf(cString,"Click on the Start button when ready!");   // insert operator prompt #2
	InsertListItem (gMainPanel, PANEL_TESTLIST, 4, cString, 0); // insert a line of text into the list box

	gCurListBoxLine = 4; // bump the list box line counter
	
    return 0;
}

/********************************************************************************
 *  InitAsyncTimer()															*
 *  Purpose: This function creates and initializes the Async timer.    			*
 ********************************************************************************/
int InitAsyncTimer()
{
	char cErrStr[50] = "\0";
	
	// initialize the async elapsed test timer for 1 second intervals
    gTimerId = NewAsyncTimer (1.0, -1, ENABLE, TestTimer_CB, NULL);
    if (gTimerId <= 0)
   	{
//		InsertTextBoxLine (gMainPanel, PANEL_TESTEXEC, -1,"ERROR: Asynchronous Timer could not be created!");
		sprintf(cErrStr, "ERROR: Asynchronous Timer could not be created!");
		MessageBox(NULL, cErrStr,"Asynchronous Timer ERROR",MB_OK | MB_ICONERROR | MB_TOPMOST);     	
        gTimerId = 0;
        return -1;
   	}
	SuspendAsyncTimerCallbacks(); // initially suspend the async timer
	
	return 0;
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
	char timeString[10];  // Buffer for timer string

	switch (event)
	{
		Delay(0.1);
		case EVENT_TIMER_TICK:
	
			gTimeOutCnt++; // increment general timeout counter
			gReadTimeOutCnt++; // bump read timeout counter
			
			time (&gtCurrentTime);  	// Set the current time
			
			hours = ((gtCurrentTime - gtStartTime) % 86400); // Elapsed time since start
			seconds = hours % 60;  	// Calculate seconds
			hours /= 60;
			minutes = hours % 60;  	// Calculate minutes
			hours /= 60;
			// Format the timer string for output
			Fmt (timeString, "%d[w2p0]:%d[w2p0]:%d[w2p0]", hours, minutes, seconds);
																			
			// Set the elapsed time on the display
			SetCtrlVal(gMainPanel, PANEL_ELAPSEDTIME, timeString);
	}
	ProcessSystemEvents();  // Process system events
	return 0;
}

/********************************************************************************
 *  GetCfgDataFromIniFile()														*
 *  Purpose: This function opens and loads test configuration INI file data.    *
 ********************************************************************************/
int GetCfgDataFromIniFile()
{
	char tmpBuf[25] = "\0";
	long size = 0;
	short error = 0;
	
	// create the ini file handle
	gIniText = Ini_New (1);
	
	// read in the ini config data
	if (error = Ini_ReadFromFile (gIniText, "C:\\Cloudshield\\Config\\cloudshieldTest.ini")) 
		goto exit;
	
	// get and save latest uut part numbers
	Ini_GetStringIntoBuffer(gIniText,"Part Numbers","ASM1", 	 gsPartNum.asm1,     sizeof(gsPartNum.asm1));
//	Ini_GetStringIntoBuffer(gIniText,"Part Numbers","ASM2", 	 gsPartNum.asm2,     sizeof(gsPartNum.asm2));
	Ini_GetStringIntoBuffer(gIniText,"Part Numbers","DPPM 510",  gsPartNum.dppm510,  sizeof(gsPartNum.dppm510));
	Ini_GetStringIntoBuffer(gIniText,"Part Numbers","DPPM 600",  gsPartNum.dppm600,  sizeof(gsPartNum.dppm600));
	Ini_GetStringIntoBuffer(gIniText,"Part Numbers","DPPM 800",  gsPartNum.dppm800,  sizeof(gsPartNum.dppm800));
	Ini_GetStringIntoBuffer(gIniText,"Part Numbers","DPPM Base", gsPartNum.dppmBase, sizeof(gsPartNum.dppmBase));
	Ini_GetStringIntoBuffer(gIniText,"Part Numbers","SDB",       gsPartNum.sdbBd,    sizeof(gsPartNum.sdbBd));
	Ini_GetStringIntoBuffer(gIniText,"Part Numbers","NIM 800",   gsPartNum.nim510,   sizeof(gsPartNum.nim510));
	Ini_GetStringIntoBuffer(gIniText,"Part Numbers","NIM 800",   gsPartNum.nim600,   sizeof(gsPartNum.nim600));
	Ini_GetStringIntoBuffer(gIniText,"Part Numbers","NIM 800",   gsPartNum.nim800,   sizeof(gsPartNum.nim800));
	
	// get and save prompts and login username and password	      dppmBase
	Ini_GetStringIntoBuffer(gIniText,"Prompts","CLIPrompt",     gsCliPrompt,     sizeof(gsCliPrompt));
	Ini_GetStringIntoBuffer(gIniText,"Prompts","GoShLinux",     gsGoshellPrompt, sizeof(gsGoshellPrompt));
	Ini_GetStringIntoBuffer(gIniText,"Prompts","CliUsername",   gsCliUserName,   sizeof(gsCliUserName));
	Ini_GetStringIntoBuffer(gIniText,"Prompts","CliPassword",   gsCliPassWord,   sizeof(gsCliPassWord));
	Ini_GetStringIntoBuffer(gIniText,"Prompts","UNCliPrompt",   gsUNCliPrompt,   sizeof(gsUNCliPrompt));
	Ini_GetStringIntoBuffer(gIniText,"Prompts","PWCliPrompt",   gsPWCliPrompt,   sizeof(gsPWCliPrompt));
	Ini_GetStringIntoBuffer(gIniText,"Prompts","IxpUserName",   gsIxpUserName,   sizeof(gsIxpUserName));
	Ini_GetStringIntoBuffer(gIniText,"Prompts","IxpPassword",   gsIxpPassWord,   sizeof(gsIxpPassWord));
	Ini_GetStringIntoBuffer(gIniText,"Prompts","UNIxpPrompt",   gsUNIxpPrompt,   sizeof(gsUNIxpPrompt));
	Ini_GetStringIntoBuffer(gIniText,"Prompts","PWIxpPrompt",   gsPWIxpPrompt,   sizeof(gsPWIxpPrompt));
	Ini_GetStringIntoBuffer(gIniText,"Prompts","IxpLogPrompt",  gsIxpLogPrompt,  sizeof(gsIxpLogPrompt));
	Ini_GetStringIntoBuffer(gIniText,"Prompts","RedBootPrompt", gsRedBootPrompt, sizeof(gsRedBootPrompt));
	
	// get and save the serial and part number lengths
	Ini_GetInt (gIniText,"Data Formats","SN1 Len", &giSerNum1Len);
	Ini_GetInt (gIniText,"Data Formats","SN2 Len", &giSerNum2Len);
	
	// get and save the serial rs232 COM port for both the asm and dppm
	Ini_GetInt (gIniText,"Com Ports","ASMUUT",  &giComPortASM1);
	Ini_GetInt (gIniText,"Com Ports","ASMCAP",  &giComPortASM2);
	Ini_GetInt (gIniText,"Com Ports","DPPMUUT", &giComPortDPPM1);
	Ini_GetInt (gIniText,"Com Ports","DPPMCAP", &giComPortDPPM2);
	
	// get the GUI software version number
	Ini_GetStringIntoBuffer(gIniText,"Software Versions","GUI Ver", gcGuiVer, sizeof(gcGuiVer));
	
	// get and display the GUI version number
	gAboutPanel = LoadPanel (0, "FatTestGui.uir", ABOUT);
	sprintf(tmpBuf,"Version %s",gcGuiVer);
	SetCtrlVal(gAboutPanel, ABOUT_VER, tmpBuf);
	
	// get the test station id
	Ini_GetStringIntoBuffer(gIniText,"Test Station","Number", gcTestStaId, sizeof(gcTestStaId));
	
	// get the cmmon application path
	Ini_GetStringIntoBuffer(gIniText,"Filename Paths","Cmmon", gcCmmonPath, sizeof(gcCmmonPath));
exit:	
	return error;
}

/********************************************************************************
 *  Function: GetSysTimeEx									   	   				*
 *  Purpose:																   	*
 *		This callback function gets the system time and date and enters it into	*
 *		the test start time and date display text box on the Test panel.		*
/*******************************************************************************/
char* GetSysTimeEx()
{
	time_t lt;
	char timeStr[25] = "\0";

	lt = time(NULL); // get the time in total seconds
	strftime (timeStr,25,"%I:%M:%S %p",localtime(&lt)); // format the time hh:mm:ss
	strftime (gDateStr,25,"%m/%d/%Y",localtime(&lt));  	// format the date mm/dd/yyyy
	strcat(gDateStr,"  ");
	strcat(gDateStr,timeStr); // create the date and time string

	return gDateStr;
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
	
	// create the date and time string for test result csv file
	FormatDateTimeString (currDateTime,"_%m%d%y_%H%M%S", gcSerNumTimeDate, bufferLen);
	
	// create the date and time string for mfg eeprom field
	FormatDateTimeString (currDateTime,"%m-%d-%Y", gcMfgDate, bufferLen);
	
	FormatDateTimeString (currDateTime,"%H:%M:%S", gcTime, bufferLen);
	
	FormatDateTimeString (currDateTime,"%m%y", gcMoYr, bufferLen);
	
	return gcDateTimeBuffer;
}

/********************************************************************************
 *  Function: ClrCheckMarks														*
 *  Purpose:  This function clears the check marks from the list box.		 	*
 *	 		  																	*
 ********************************************************************************/
int ClrCheckMarks ()
{
	int i, numItems; 
	int chkListNum[25];
	
	if (giIsDppmGe) // get the number of items in the GigE test list
		numItems = Ini_NumberOfItems (gIniText,"DPPM GigETest List");
	else if (giIsDppmSonet) // get the number of items in the Sonet test list
		numItems = Ini_NumberOfItems (gIniText,"DPPM Sonet Test List");
	else if (giIsDppm10Ge) // get the number of items in the 10Ge test list
		numItems = Ini_NumberOfItems (gIniText,"DPPM 10Ge Test List");
	
	for (i=0; i<numItems; i++) // clear the check marks from the list box
		CheckListItem(gMainPanel, PANEL_TESTLIST, i, 0); 
	
	return 0;
}

/********************************************************************************
 *  Function: ResetListBox														*
 *  Purpose:  This function clears the front panel list box and adds the test 	*
 *	 		  result column.													*
 ********************************************************************************/
int ResetListBox ()
{
	int i, numItems; 
	int n = 0, numChkItems = 0;
	int numVisibleItems;
	int chkListNum[25];
	
	if (giSingleTestFlg)
	{
		GetNumCheckedItems(gMainPanel, PANEL_TESTLIST, &numChkItems); // see if any items are checked
	
		if (giIsDppmGe) // get the number of items in the test list
			numItems = Ini_NumberOfItems (gIniText,"DPPM GigE Test List");
		else if (giIsDppmSonet)	
			numItems = Ini_NumberOfItems (gIniText,"DPPM Sonet Test List");
		else if (giIsDppm10Ge)	
			numItems = Ini_NumberOfItems (gIniText,"DPPM 10Ge Test List");
	
		for (i=0; i<numItems; i++) // see which of the test list items are checked and save the value
			GetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, i, ATTR_MARK_STATE, &chkListNum[i]);
	
		DeleteListItem(gMainPanel, PANEL_TESTLIST, 0, -1); // clear the list box
		
		gCurListBoxLine = 0; // set the list box index pointer to 0

		InitTestListBox(); // initialize the test list display in the gui list box

		for (i=0; i<numItems; i++) // put a check mark(s) next to the saved value
		{
			if (chkListNum[i] == 1)
				CheckListItem(gMainPanel, PANEL_TESTLIST, i, 1); 
			else
				CheckListItem(gMainPanel, PANEL_TESTLIST, i, 0); 
		}
	}
	else
	{
		DeleteListItem(gMainPanel,   PANEL_TESTLIST, 0, -1); // clear the list box
		
		InitTestListBox(); // initialize the test list display in the gui list box

		gCurListBoxLine = 0; // clear the current line counter
	}
	return 0;
}

/*
 *	InitTestListBox()
 *
 *  This function initializes the Test list display in the GUI for the selected UUT.
 */
void InitTestListBox()
{
	int i, numItems = 0;
	char testNum[10] = "\0";
	char testStr[75] = "\0";

    SetCtrlAttribute(gMainPanel, PANEL_TESTLIST, ATTR_COLUMN_LABELS_VISIBLE, 1);
    SetCtrlAttribute(gMainPanel, PANEL_TESTLIST, ATTR_SHOW_IMAGES, 1);
    SetCtrlAttribute(gMainPanel, PANEL_TESTLIST, ATTR_SHOW_CONNECTION_LINES, 1);
	
	if (giIsDppmGe) // check if Ge or Sonet board
	{   // get the number of test items in the list
		giNumItems = Ini_NumberOfItems (gIniText,"DPPM GigE Test List");
		// display the test list in the gui list box
		for (i=0; i<giNumItems; i++)
		{
			sprintf(testNum,"Test%i",i);
			Ini_GetStringIntoBuffer(gIniText,"DPPM GigE Test List", testNum, testStr, sizeof(testStr));
			InsertListItem(gMainPanel, PANEL_TESTLIST, i, testStr, 0); 
			SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, i, ATTR_IMAGE_INDEX, 0); 	
		    SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, i, ATTR_NO_EDIT_LABEL, 1);  
		    SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, i, ATTR_HORIZONTAL_GRID_VISIBLE, 1);
			gCurListBoxLine++;
		}
	}
	else if (giIsDppmSonet) // check if 10 Ge board
	{   // get the number of test items in the list
		giNumItems = Ini_NumberOfItems (gIniText,"DPPM Sonet Test List");
		// display the test list in the gui list box
		for (i=0; i<giNumItems; i++)
		{
			sprintf(testNum,"Test%i",i);
			Ini_GetStringIntoBuffer(gIniText,"DPPM Sonet Test List", testNum, testStr, sizeof(testStr));
			InsertListItem(gMainPanel, PANEL_TESTLIST, i, testStr, 0); 
			SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, i, ATTR_IMAGE_INDEX, 0); 	
		    SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, i, ATTR_NO_EDIT_LABEL, 1);  
		    SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, i, ATTR_HORIZONTAL_GRID_VISIBLE, 1);
			gCurListBoxLine++;
		}
	}
	else if (giIsDppm10Ge) // check if 10 Ge board
	{   // get the number of test items in the list
		giNumItems = Ini_NumberOfItems (gIniText,"DPPM 10Ge Test List");
		// display the test list in the gui list box
		for (i=0; i<giNumItems; i++)
		{
			sprintf(testNum,"Test%i",i);
			Ini_GetStringIntoBuffer(gIniText,"DPPM 10Ge Test List", testNum, testStr, sizeof(testStr));
			InsertListItem(gMainPanel, PANEL_TESTLIST, i, testStr, 0); 
			SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, i, ATTR_IMAGE_INDEX, 0); 	
		    SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, i, ATTR_NO_EDIT_LABEL, 1);  
		    SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, i, ATTR_HORIZONTAL_GRID_VISIBLE, 1);
			gCurListBoxLine++;
		}
	}
	ProcessSystemEvents();
}

/*
 *	InitTestStatusArrays()
 *
 *  This function initializes the test status variable array structure
 */
void InitTestStatusArrays()
{
	// initialize the test status array structure
	memset(gsTestStatus.cPgmPwrSeq,    0, 5);
	memset(gsTestStatus.cChkPupVolt,   0, 5);
	memset(gsTestStatus.cPgmJtagChn,   0, 5);
	memset(gsTestStatus.cPgmRedBoot,   0, 5);
	memset(gsTestStatus.cRedBootMem,   0, 5);
	memset(gsTestStatus.cPgmTmmEeprom, 0, 5);
	memset(gsTestStatus.cPgmSdbEeprom, 0, 5);
	memset(gsTestStatus.cPgmNimEeprom, 0, 5);
	memset(gsTestStatus.cPgrmMacPhy,   0, 5);
	memset(gsTestStatus.cChkStatLEDs,  0, 5);
	memset(gsTestStatus.cChkNimLEDs,   0, 5);
	memset(gsTestStatus.cPaxMemory,    0, 5);
	memset(gsTestStatus.cQDRamMem,     0, 5);
	memset(gsTestStatus.cCamMemory,    0, 5);
	memset(gsTestStatus.cSdbMemory,    0, 5);
	memset(gsTestStatus.cRocketIO,     0, 5);
	memset(gsTestStatus.cRocketIO10Ge, 0, 5);
	memset(gsTestStatus.cUutTraffic,   0, 5);
}

/*
 *	StartTest()
 *
 *    	Program comes here when the start test button is activated
 */
int StartTest()
{
	int iSingleStepFlg;
	int iStatus = 0;
	char cString[60] = "\0";
	
	if (!giItemChked && giSingleTestFlg) // if no item is checked, alert the operator to select a test
	{
		sprintf(cString,"Please select a test first, then hit the 'Start button'.");
		MessageBox(NULL, cString,"TEST SELECTION ERROR",MB_OK | MB_ICONERROR | MB_TOPMOST);
		goto Exit;
	}
	InitTestStartUp(); // initialize the test start up
	
	if (!giSingleTestFlg) // check to see if single step flag is set
	{	
		ScanAndVerifyUUTData(); // new board, so scan in uut data
		
		iStatus = RunFunctionalTestsAll(); // start the functional test sequence, not checked
	}
	else
	{
		iStatus = RunFunctionalTestsSel(); // start the selected functional test sequence
	}		
	if (iStatus == 0)
		TestStatus("PASS"); // if the test passed, then set the pass flags and messages
	else if (iStatus == 1)
		TestStatus("FAIL"); // if a failure occurred, then set the fail flags and messages
	else if (iStatus == 2) 
		TestStatus("STOP"); // the stop test button was activated

	CleanUp(); // clean up time after the test has finished
Exit:
	return 0;
}

void InitTestStartUp()
{
	gtStartTime = time(NULL); // get the test start time for the elapsed time timer display
	SetCtrlVal(gMainPanel, PANEL_ELAPSEDTIME, "00:00:00"); // reset the elapsed timer display
	ResumeAsyncTimerCallbacks (); // resume the async test timer
	SetCtrlVal(gMainPanel, PANEL_STARTTIME, GetSysTimeEx()); // reset the start test time display
	OpenLogFile(OPEN); // open the test log file
	SetCtrlAttribute(gMainPanel,PANEL_TAIL, ATTR_DIMMED, DISABLE);
	SetCtrlVal(gMainPanel, PANEL_STOPTIME, ""); // reset the test stop time display
	ResetListBox(); // reset and initialize the test list box
	InitTestStatusArrays(); // initialize the test status array
	SetCtrlAttribute (gMainPanel, PANEL_EXIT, ATTR_DIMMED, ENABLE);
	SetCtrlAttribute (gMainPanel, PANEL_STARTSTOP, ATTR_DIMMED, DISABLE);
	SetCtrlAttribute (gMainPanel, PANEL_TESTSTATUS, ATTR_LABEL_TEXT,"TESTING");
	SetCtrlAttribute (gMainPanel, PANEL_TESTSTATUS, ATTR_CMD_BUTTON_COLOR, VAL_YELLOW);
//	OpenTail(); // open the unix tail log utility
}

void CleanUp()
{
	ClrCheckMarks (); // clear the check marks from the list box
	SetCtrlAttribute(gMainPanel,PANEL_TAIL, ATTR_DIMMED, ENABLE);
	SetCtrlAttribute(gMainPanel, PANEL_STARTSTOP, ATTR_CTRL_VAL, 0);
	SetCtrlAttribute (gMainPanel, PANEL_EXIT,  ATTR_DIMMED, DISABLE);
	SetCtrlAttribute (gMainPanel, PANEL_STARTSTOP, ATTR_DIMMED, DISABLE);
	SetCtrlVal(gMainPanel, PANEL_STOPTIME, GetSysTimeEx()); // display the stop time
	SuspendAsyncTimerCallbacks (); // suspend the async timer
	giStopFlg = 0; // clear the stop test flag
	giScanDataFlg = 0;    // clear the scan data panel flag
	if (!giSingleTestFlg) // check to see if single step flag is set
		CreateCSVFile();  // create test report, but not in select test mode
}

int GetUutValsFromFile()
{
	int i, fh, iBytesRead;
	int returnValue;
	int itemIndex = 0;
	int numChkItems, numListItems, chkListNum; 
	int checked = 0, iStatus = 0;
	char cDateTime[30] = "\0";
	char cLineBuf[200] = "\0";
	char cPathStr[100] = "\0";
	char cTestStr[150] = "\0";
	double currDateTime;
	int bufferLen = 30;
	int index = 0;
	char *token;
    char *buffer;

	giSingleTestFlg = 1;
	
	Ini_GetStringIntoBuffer(gIniText,"Filename Paths","Select", cPathStr, sizeof(cPathStr));
	
	// open the uut test status csv file 
	fh = OpenFile (cPathStr, VAL_READ_ONLY, VAL_OPEN_AS_IS, VAL_ASCII);
	
	do // loop through the file and ouput each line of data to the scan list box
	{
		memset(cLineBuf, 0, 200);
	
		iBytesRead = ReadLine (fh, cLineBuf, 199); // read in a line of data
		
		if (cLineBuf == "") // skip empty lines
			continue;

	    buffer = StrDup(cLineBuf);  // save a copy of the read buffer

		token = strtok(buffer,";"); // get the uut type
		
		if (token != NULL)
		{
			strcpy(gsLogData.cType,token);   				// save the uut type
			strcpy(gsLogData.cUutSerNum, strtok(NULL,";")); // get the uut serial number
			strcpy(gsLogData.cUutPartNum,strtok(NULL,";")); // get the sdb part number
			strcpy(gsLogData.cDate,strtok(NULL,";"));       // get the time & date
			strcpy(gsLogData.cResult,strtok(NULL,";"));     // get the test result
		
			sprintf(cTestStr,"%s    %s    %s    %s    %s",gsLogData.cType,gsLogData.cUutSerNum,
				    gsLogData.cUutPartNum,gsLogData.cDate,gsLogData.cResult);

			InsertListItem(giTestList, SCANLIST_TREE, index++, cTestStr, 0);
			
	    	SetTreeItemAttribute (giTestList, SCANLIST_TREE, index-1, ATTR_MARK_TYPE, VAL_MARK_RADIO);
		}
	} while (iBytesRead >= 0); // read until end of test log file is reached
	
	giButtonPressFlg = 0;
	
	InstallPopup(giTestList); // display the select test panel

	while (!giButtonPressFlg) // wait here until either the OK button is pressed
		ProcessSystemEvents();
	
	GetNumCheckedItems(giTestList, SCANLIST_TREE, &numChkItems); // see if any items are checked

	if (numChkItems) // see if any items are checked
	{
		GetNumListItems (giTestList, SCANLIST_TREE, &numListItems);

		for (i=0; i<numListItems; i++) // see which of the list items are checked and save the value
		{
			IsListItemChecked (giTestList, SCANLIST_TREE, i, &checked);
			chkListNum = i; // save the index of the checked item
			if (checked) // if item checked, exit loop
				break;
		}
	}
	if (!checked) // couldnt find a checked item, set the status flag
	{
		iStatus = 1;
		goto Done;
	}
	index = 0; // clear the file index counter
	
	lseek (fh, 0, SEEK_SET); // seek back to the beginning of the file
	
	ResetTextBox (giPanelScanData, UUTDATA_UUTSN, "");
	ResetTextBox (giPanelScanData, UUTDATA_SDBSN, "");
	ResetTextBox (giPanelScanData, UUTDATA_NIMSN, "");
	ResetTextBox (giPanelScanData, UUTDATA_MAC,   "");
	
	do // loop through the file until the check line item is found
	{
		memset(cLineBuf, 0, 200); // initialize the line buffer
	
		iBytesRead = ReadLine (fh, cLineBuf, 199); // read in a line of data
		
		if (cLineBuf == "") // skip empty lines
			continue;
		
		if (chkListNum == index) // if itemIndex = index counter, the data has been found
		{
		    buffer = StrDup(cLineBuf); // save a copy of the read buffer

			token = strtok(buffer,";");
			sprintf(gcUutSerNum,strtok(NULL,";")); // get the uut serial number
			SetCtrlVal(giPanelScanData, UUTDATA_UUTSN, gcUutSerNum); // display uut serial number
			token = strtok(NULL,";");
			token = strtok(NULL,";");
			token = strtok(NULL,";");
			token = strtok(NULL,";"); // get the sdb serial number
			sprintf(gcSdbSerNum,token); // get the uut serial number
			SetCtrlVal(giPanelScanData, UUTDATA_SDBSN, token); // display sdb serial number
			token = strtok(NULL,";");
			token = strtok(NULL,";"); // get the nim serial number
			sprintf(gcNimSerNum,token); // get the uut serial number
			SetCtrlVal(giPanelScanData, UUTDATA_NIMSN, token); // display nim serial number
			token = strtok(NULL,";"); 
			token = strtok(NULL,";"); // get the mac address
			sprintf(gcMacAddr,token); // get the uut serial number
			SetCtrlVal(giPanelScanData, UUTDATA_MAC, token); // display mac address
				
			goto Done; // data found and saved, now break out of the while loop
		}
		else index++; // bump the file pointer
		
	} while (iBytesRead >= 0); // read until end of test log file is reached
Done:	
	CloseFile(fh); // close the test status csv file
Exit:	
	giScanDataFlg = 0; // clear the scan data panel flag
	
	return iStatus;
}

int CVICALLBACK ScanListOk_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			if (control == SCANLIST_OK)
			{
				giButtonPressFlg = 1;
				RemovePopup(giTestList); // close the test list select panel
			}
			break;
	}
	return 0;
}

/*
 *	CreateCSVFile()
 *
 *  This function creates a CSV type of file for import into a data base.
 *  and a copy is also stored locally. 
 */
void CreateCSVFile()
{
	int fh;
	int bufferLen = 30;
	double currDateTime;
	char cCsvField[250] = "\0";
	char cUutPartNum[20] = "\0";
	char cNimPartNum[20] = "\0";
	char cDateTime[30] = "\0";
	char cUutType[15] = "\0";
	char cTestFileNameStr[100] = "\0";
	
	sprintf(gsOperator,"Mike");
	sprintf(gsComment,"No failures");

	GetCurrentDateTime (&currDateTime); // get the current date and time
	
	// create the date and time string for gui display
	FormatDateTimeString (currDateTime,"%a. %b %d. %Y %I:%M %p", cDateTime, bufferLen);
	
	memset(cCsvField, 0, 250);
	
	Ini_GetStringIntoBuffer(gIniText,"Filename Paths","Report", cTestFileNameStr, sizeof(cTestFileNameStr));

	if (giIsDppmGe) // check if GigE board
	{
		sprintf(cUutType,"DPPM510_");
		sprintf(cUutPartNum,gsPartNum.dppm510);
	}
	else if (giIsDppmSonet) // check if Sonet board
	{
		sprintf(cUutType,"DPPM600_");
		sprintf(cUutPartNum,gsPartNum.dppm600);
	}
	else if (giIsDppm10Ge) // check if 10Ge board
	{
		sprintf(cUutType,"DPPM800_");
		sprintf(cUutPartNum,gsPartNum.dppm800);
	}
	sprintf(cNimPartNum, CSBoardType()); // get the nim board part number
	
	CreateDateTimeStr(); // get the date and time
	
	// create and open the uut test status csv file 
	fh = OpenFile (cTestFileNameStr, VAL_READ_WRITE, VAL_APPEND, VAL_ASCII);
	
//	memset(cCsvField, 0, 250); // clear csv field
	cUutType[7] = '\0'; // remove the "_" character
	
	// format data field into csv file
	sprintf(cCsvField,"%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s",cUutType,gcUutSerNum,cUutPartNum,
			gcSdbSerNum,gsPartNum.sdbBd,gcNimSerNum,cNimPartNum,gcMfgDate,gcTime,gcMacAddr,
			gcTestStaId,gsOperator,gsResult,gsComment);
	WriteLine (fh, cCsvField, sizeof(cCsvField));
	
	CloseFile(fh); // close the test status csv file

	Ini_GetStringIntoBuffer(gIniText,"Filename Paths","Select", cTestFileNameStr, sizeof(cTestFileNameStr));

	fh = OpenFile (cTestFileNameStr, VAL_READ_WRITE, VAL_APPEND, VAL_ASCII);
	
	memset(cCsvField, 0, 250); // clear csv field
	
	// format data field into text file
	sprintf(cCsvField,"%s;%s;%s;%s;%s;%s;%s;%s;%s;%s\n",cUutType,gcUutSerNum,cUutPartNum,
			cDateTime,gsResult,gcSdbSerNum,gsPartNum.sdbBd,gcNimSerNum,cNimPartNum,gcMacAddr);
	WriteFile (fh, cCsvField, StringLength(cCsvField));
	
	CloseFile(fh); // close the test status text file
}

/*
 *	StopTest()
 *
 *  Program comes here when the stop test button is activated
 */
int StopTest()
{
	int status = 0;
	
	giStopFlg = 1; // set stop test flag
	
	return status;
}

int CVICALLBACK StartStopTest_CB (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	int  iStartStopFlg;
	int  status = 0;
	int  listVal;
	char cString[100];
		
	switch (event)
	{
		Delay(0.5);
		case EVENT_COMMIT:
			GetCtrlVal (gMainPanel, PANEL_STARTSTOP, &iStartStopFlg);
			GetCtrlVal(gMainPanel, PANEL_UUTTYPELIST, &listVal);
			if (listVal == 0)
			{
				if (iStartStopFlg)
					SetCtrlAttribute(gMainPanel, PANEL_STARTSTOP, ATTR_CTRL_VAL, 0);
				sprintf(cString,"Please select a UUT type from the drop down list.");
				MessageBox(NULL, cString,"CHASSIS SELECTION ERROR",MB_OK | MB_ICONERROR | MB_TOPMOST);
				return status;
			}
			if (iStartStopFlg)
				status = StartTest();
			else 
				status = StopTest();
			break;
	}
	return status;
}

/*
 *	TestStatus()
 *
 *		This function checks if the test passed or failed and displays the appropriate flags and messages.
 */
void TestStatus(char* cpMessage)
{
	char chStr[150] = "\0";
	int uut = 1;
	
	gCurListBoxLine = giNumItems; // get the number of tests
	sprintf(gsResult,cpMessage); // save the test result
	
	if (strcmp(cpMessage,"PASS") == 0) // check if test passed
	{
		strcpy(chStr,"Test Completed: "); // come here if test was completed and passed
		strcat(chStr, gDateStr); 		  // display the stop time and date
		SetCtrlAttribute(gMainPanel, PANEL_TESTSTATUS, ATTR_CMD_BUTTON_COLOR, VAL_GREEN);
		SetCtrlAttribute (gMainPanel, PANEL_TESTSTATUS, ATTR_LABEL_TEXT,"PASS");
		InsertListItem(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, chStr, 0); 
		SetTestStatus(VAL_DK_GREEN, "TEST PASSED");
		SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, 1); 	
	}
	else if (strcmp(cpMessage,"FAIL") == 0) // check if test failed
	{   // if test
		strcpy(chStr,"Test Completed: "); // come here if test was completed but failed
		strcat(chStr, gDateStr); 		  // display the stop time and date
		SetCtrlAttribute(gMainPanel, PANEL_TESTSTATUS, ATTR_CMD_BUTTON_COLOR, VAL_RED);
		SetCtrlAttribute (gMainPanel, PANEL_TESTSTATUS, ATTR_LABEL_TEXT,"FAIL");
		InsertListItem(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, chStr, 0); 
		SetTestStatus(VAL_RED, "TEST FAILED");
		SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, 4); 	
	}
	else if (strcmp(cpMessage,"STOP") == 0) // check if test was stopped
	{   // if test
		strcpy(chStr,"Test Stopped: "); // come here if test was stopped
		strcat(chStr, gDateStr); 		  // display the stop time and date
		SetCtrlAttribute(gMainPanel, PANEL_TESTSTATUS, ATTR_CMD_BUTTON_COLOR, VAL_CYAN);
		SetCtrlAttribute (gMainPanel, PANEL_TESTSTATUS, ATTR_LABEL_TEXT,"STOPPED");
		InsertListItem(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, chStr, 0); 
		SetTestStatus(VAL_CYAN, "STOPPED");
		SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, 3); 	
	}
    SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_HORIZONTAL_GRID_VISIBLE, 1);
}

int CVICALLBACK Exit_CB (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			QuitUserInterface(0);
			break;
	}
	return 0;
}

int CVICALLBACK SelectUutType_CB (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	int listVal;

	switch (event)
	{
		case EVENT_COMMIT:
			
			GetCtrlVal(gMainPanel, PANEL_UUTTYPELIST, &listVal); // get the list value
			
			// initialize the uut type flags and messages
			switch (listVal)
			{
				case DPPM510: // dppm-510 selected	    
					SetCtrlVal(gMainPanel, PANEL_UUTNAME,"DPPM-510");
					SetCtrlVal(gMainPanel, PANEL_PARTNUM, gsPartNum.dppm510);
					giIsDppmGe = 1;
					giIsDppmSonet = 0;
					giIsDppm10Ge = 0;
					break;
				case DPPM600: // dppm-600 selected
					SetCtrlVal(gMainPanel, PANEL_UUTNAME,"DPPM-600");
					SetCtrlVal(gMainPanel, PANEL_PARTNUM, gsPartNum.dppm600);
					giIsDppmSonet = 1;
					giIsDppmGe = 0;
					giIsDppm10Ge = 0;
					break;
				case DPPM800: // dppm-800 selected
					SetCtrlVal(gMainPanel, PANEL_UUTNAME,"DPPM-800");
					SetCtrlVal(gMainPanel, PANEL_PARTNUM, gsPartNum.dppm800);
					giIsDppm10Ge = 1;
					giIsDppmGe = 0;
					giIsDppmSonet = 0;
					break;
				case 0: // no uut selected
					SetCtrlVal(gMainPanel, PANEL_UUTNAME,"DPPM");
					SetCtrlVal(gMainPanel, PANEL_PARTNUM,"");
					giIsDppmGe = 0;
					giIsDppmSonet = 0;
					giIsDppm10Ge = 0;
					break;
					
			}
			if (giIsDppmGe || giIsDppmSonet || giIsDppm10Ge)
			{
				DeleteListItem(gMainPanel,   PANEL_TESTLIST, 0, -1); // clear the list box
				gCurListBoxLine = 0; // clear the current line counter
				InitTestListBox(); // display the test list based on the selected uut
				SetMenuBarAttribute(giMenuBar, MENUBAR_FILE_ENABLE, ATTR_DIMMED, 0); // undim the single test enable item
			}
		break;
	}
	return 0;
}

/********************************************************************************
 *  About_CB()																	*
 *  Purpose: This function displays the GUI software version .					*
 ********************************************************************************/
void CVICALLBACK About_CB (int menuBar, int menuItem, void *callbackData, int panel)
{
	gAboutPanel = LoadPanel (0, "FatTestGui.uir", ABOUT);
	InstallPopup (gAboutPanel);
}

int CVICALLBACK AboutExit_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			RemovePopup (gAboutPanel);
			break;
	}
	return 0;
}

/********************************************************************************
 *  Login()																		*
 *  Purpose: This function opens a login panel and verifies the login entries.	*
 ********************************************************************************/
int Login()
{
	// create and load the login panel
    giLoginPanel = LoadPanel(0, "SysTestGui.uir", LOGIN);
	
	//	Install login entry callback functions
	InstallCtrlCallback (giLoginPanel, LOGIN_USERNAME, Clb_LoginData, 0); 
	InstallCtrlCallback (giLoginPanel, LOGIN_PASSWORD, Clb_LoginData, 0); 
	InstallCtrlCallback (giLoginPanel, LOGIN_OK, 	   Clb_LoginData, 0); 

	// set the username entry box to the active control
	SetActiveCtrl (giLoginPanel, LOGIN_USERNAME);

	// open the login data panel
	InstallPopup (giLoginPanel);

	//	Start processing panel events.
    giRunLogin = RunUserInterface();
	
   	//	Discard login panel. 
	DiscardPanel(giLoginPanel);
	
	return 0;
}

/*
 *	Clb_LoginData()
 *
 *	Purpose:
 *		This callback function handles login entry inputs.
 */
int  CVICALLBACK Clb_LoginData(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	//	Local variables
	char	cUserName[50] = "\0";
	char	cPassWord[50] = "\0";
	char	cErrStr[50] = "\0";
	char	cUnPw[50] = "\0";
	char*	cUNStr;
	char*	cPWStr;
	
    switch (event)
    {
		case EVENT_COMMIT:
	
		    /* Convert the string control into a password control! */
		    giPasswordCtrlID = PasswordCtrl_ConvertFromString (giLoginPanel, LOGIN_PASSWORD);
			
			switch (control)
	   		{
				case LOGIN_OK:
					
					// get the correct login information from the ini file
					Ini_GetStringIntoBuffer(gIniText,"Login","UnPw", cUnPw, sizeof(cUnPw));
					
					// strip the first token to get the inital values from the login entry boxes
					cUNStr = strtok(cUnPw,":");
					cPWStr = strtok(NULL,":");
					
				    // loop until a match or an end of string NULL value is reached
					while (1)
					{
				    	// get the entered login information
	    				GetCtrlVal (giLoginPanel, LOGIN_USERNAME, cUserName);
						// convert the masked entry string into an unmasked password string
	    				PasswordCtrl_GetAttribute (giLoginPanel, LOGIN_PASSWORD, ATTR_PASSWORD_VAL, cPassWord);
						
						// compare the login that was entered to the login information from the ini file
						if (strcmp(cUserName, cUNStr) == 0 && strcmp(cPassWord, cPWStr) == 0)
						{
				   			//	Close login data panel and break out from the 'while' loop
							QuitUserInterface (giRunLogin);
							break;
						}
						// not a NULL value, so save the login entries
						cUNStr = strtok(NULL,":");
						cPWStr = strtok(NULL,":");
						
						// if no match is found, open up an error message box and try again
						if(cUNStr == NULL)
						{
							sprintf(cErrStr, "ERROR: Unable to find a Login match!");
							MessageBox(NULL, cErrStr,"TEST LOGIN ERROR",MB_YESNO | MB_ICONERROR | MB_TOPMOST);
							// clear out the information and enter it again
			   				SetCtrlVal (giLoginPanel, LOGIN_USERNAME, "");	   			
			   				PasswordCtrl_SetAttribute (giLoginPanel, LOGIN_PASSWORD, ATTR_PASSWORD_VAL, "");
			   				SetActiveCtrl (giLoginPanel, LOGIN_USERNAME);	   			
							SetCtrlAttribute (giLoginPanel, LOGIN_OK, ATTR_DIMMED, 1);
							break;
						}
					}
					break;
					
				case LOGIN_USERNAME:
					
		   			//	Advance to the next field if the username length is greater than 0.
    				GetCtrlVal (giLoginPanel, LOGIN_USERNAME, cUserName);
				
		   			if(strlen(cUserName))
		   			{
		   				//	Select CID SN field as active
		   				SetActiveCtrl (giLoginPanel, LOGIN_PASSWORD);
		   			}
		   			else
		   			{
		   				SetCtrlVal (giLoginPanel, LOGIN_USERNAME, "");	   			
						SetCtrlAttribute (giLoginPanel, LOGIN_OK, ATTR_DIMMED, 1);
						SetActiveCtrl (giLoginPanel, LOGIN_USERNAME);
						ProcessDrawEvents();
		   			}
					break;
					
				case LOGIN_PASSWORD:
					
		   			//	Advance to the next field if the password field is not empty.
    				GetCtrlVal (giLoginPanel, LOGIN_PASSWORD, cPassWord);
				
		   			if(strlen(cPassWord))
		   			{
						// Un-dim the OK button
						SetCtrlAttribute (giLoginPanel, LOGIN_OK, ATTR_DIMMED, 0);
		   				//	Set the OK button as active
		   				SetActiveCtrl (giLoginPanel, LOGIN_OK);
		   			}
		   			else
		   			{
		   				SetCtrlVal (giLoginPanel, LOGIN_PASSWORD, "");	   			
						SetCtrlAttribute (giLoginPanel, LOGIN_OK, ATTR_DIMMED, 1);
						SetActiveCtrl (giLoginPanel, LOGIN_PASSWORD);
						ProcessDrawEvents();
		   			}
					break;
	   		}	 
	   		break;
	}
	return 0;
}

/********************************************************************
 *	ScanAndVerifyUUTData()
 *
 *	Purpose:
 *		This callback function displays the scan UUT data panel
 *
 */
int ScanAndVerifyUUTData()
{
	int listVal;
	
	//	Load and setup scan UUT data panel
	HidePanel(gMainPanel); // hide the main panel
	
    SetIdleEventRate(100); 

	//	Install callback functions
	InstallCtrlCallback (giPanelScanData, UUTDATA_UUTSN, Clb_ScanUUTData, 0); 
	InstallCtrlCallback (giPanelScanData, UUTDATA_SDBSN, Clb_ScanUUTData, 0); 
	InstallCtrlCallback (giPanelScanData, UUTDATA_NIMSN, Clb_ScanUUTData, 0); 
	InstallCtrlCallback (giPanelScanData, UUTDATA_MAC,   Clb_ScanUUTData, 0); 

	//	Set first active control to point to serial number entry box
	SetActiveCtrl(giPanelScanData, UUTDATA_UUTSN);
	
	// open the scan data panel
	InstallPopup (giPanelScanData);
	
	if (!giSingleTestFlg) // check to see if single step flag is set
	{
		ResetTextBox (giPanelScanData, UUTDATA_UUTSN, "");
		ResetTextBox (giPanelScanData, UUTDATA_SDBSN, "");
		ResetTextBox (giPanelScanData, UUTDATA_NIMSN, "");
		ResetTextBox (giPanelScanData, UUTDATA_MAC,   "");
		
		//	Dim the remaining entry fields at startup
		SetCtrlAttribute (giPanelScanData, UUTDATA_SDBSN, 	ATTR_DIMMED, 1);
		SetCtrlAttribute (giPanelScanData, UUTDATA_NIMSN, 	ATTR_DIMMED, 1);
		SetCtrlAttribute (giPanelScanData, UUTDATA_MAC, 	ATTR_DIMMED, 1);
		
		//	Dim OK button until all fields are valid
		SetCtrlAttribute (giPanelScanData, UUTDATA_OK, ATTR_DIMMED, 1);
	}
	//	Start processing panel events.
    giRunScanUUTPanel = RunUserInterface();
   	 
	RemovePopup(giPanelScanData); // discard the entry panel 
	DisplayPanel(gMainPanel); // display the main panel
	ProcessSystemEvents();
	giScanDataFlg = 0;      // set the scan data panel flag

	return 0;
}

int ValSerNum(int serNumEntry)
{
	int 	status = 1;
	int		iValidData 	= 1;
	char	cValue[50]	= "\0";
	char	cSerNumStr[75];
	
	GetCtrlVal(giPanelScanData, serNumEntry, cSerNumStr); // Read scanned serial number
	
	//	Validate serial number that is entered and check that the cm characters are alphabetic 
	if(strcmp (cSerNumStr, "") == 0 || !isalpha(cSerNumStr[4]) || !isalpha(cSerNumStr[5]))
	{
		sprintf(cSerNumStr," Serial Number prefix does not match expected format. Scanned = %s, Expected = %s ",cValue, gsLogData.cUutSerNum);
		MessageBox(NULL, cSerNumStr,"SERIAL NUMBER FORMAT ERROR",MB_OK | MB_ICONERROR | MB_TOPMOST);  
		status = 0;
	}
	SetCtrlVal(gMainPanel, PANEL_SERIALNUM, gcUutSerNum); // display the serial number in the gui panel box
	
	return status;
}

int ValPartNum(int partNumEntry)
{
	int 	iStatus = 1;
	int		iValidData 	= 1;
	char	cValue[50]	= "\0";
	char	cPartNumStr[50];
	
	GetCtrlVal(giPanelScanData, partNumEntry, cPartNumStr); // Read scanned part number
	
	if      (strcmp(cPartNumStr, gsPartNum.dppm510) == 0) giIsDppmGe;
	else if (strcmp(cPartNumStr, gsPartNum.dppm600) == 0) giIsDppmSonet;
	else if (strcmp(cPartNumStr, gsPartNum.dppm800) == 0) giIsDppm10Ge;
	
	//	Validate serial number is entered and check that the cm characters are alphabetic 
	if(strcmp (cPartNumStr, "") == 0 || !isalpha(cValue[4]) || !isalpha(cValue[5]))
	{
		sprintf(cPartNumStr," Part Number prefix does not match expected format. Scanned = %s, Expected = %s ",cValue, gsLogData.cUutSerNum);
		MessageBox(NULL, cPartNumStr,"SERIAL NUMBER FORMAT ERROR",MB_OK | MB_ICONERROR | MB_TOPMOST);  
		iStatus = 0;
	}
	return iStatus;
}

int ValMacAddr(int macAddrEntry)
{
	int i, j, iStatus = 1;
	long long lMacAddr;
	char tempMac[20] = "\0";
	char macAddrStr[25] = "\0";
	char cMacAddr[8][20];
	
	for (i=0; i<8; i++) // initialize the mac address array
		sprintf(gsMacAddr[i],"\0");
	
	GetCtrlVal(giPanelScanData, macAddrEntry, macAddrStr); // Read scanned part number

	strcpy(tempMac, macAddrStr);	      // save the mac address to a temporary buffer
	sscanf(macAddrStr,"%I64x",&lMacAddr); // convert it to a 64 bit integer value
	
	// verify that the mac address is 12 characters long and has an even starting address
	if ((strlen(macAddrStr) == 12) && ((lMacAddr & 1) == 0)) 
	{
		for (i=0; i<8; i++) // create 8 mac addresses each incremented by 1 from the initial value
		{
			strcpy(gsMacAddr[i],tempMac);	   	  // save the mac address
			sscanf(tempMac,"%I64x",&lMacAddr); 	  // convert the mac address to a long value
			lMacAddr++;						      // increment the mac address by 1
			sprintf(tempMac,"%012I64X",lMacAddr); // convert mac address back to a string
			for(j=0; tempMac[j]; j++)
			    tempMac[j] = toupper(tempMac[j]); // convert to upper case

  		}
	}
	else
		iStatus = 0; // set the error status flag
	
	return iStatus;
}

/********************************************************************
 *	Clb_ScanUUTData()
 *
 *	Purpose:
 *		This callback function handles data entry inputs.
 */
int  CVICALLBACK Clb_ScanUUTData(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	//	Local variables
	int		i, ii, status, listVal;
	int		iValidData;
	char	cScanVal[50];
	
    switch (event)
    {
		case EVENT_COMMIT:
			
			switch (control)
	   		{
				case UUTDATA_OK:
			    	//	Validate the Serial & Part Numbers. The CM code at bytes 4 and 5 should be alpha
					iValidData = ValSerNum (UUTDATA_UUTSN);
					iValidData = ValSerNum (UUTDATA_SDBSN);
					iValidData = ValSerNum (UUTDATA_NIMSN);
					iValidData = ValMacAddr(UUTDATA_MAC);
		    	
					if (iValidData)
			   			//	Close scan UUT data panel
						QuitUserInterface (giRunScanUUTPanel);
				
					else
		   				ResetUUTScanBox();
					break;
					
				case UUTDATA_REFRESH:
	   				ResetUUTScanBox();
					break;
					
				case UUTDATA_UUTSN:
		   			//	Advance to the next field if SN length is equal to the standard twelve characters.
			    	GetCtrlVal(giPanelScanData, UUTDATA_UUTSN, cScanVal);
				
		   			if(strlen(cScanVal) == giSerNum1Len)
		   			{
						SetCtrlVal(gMainPanel, PANEL_SERIALNUM, cScanVal);
					
	   					//	Un-Dim uut PN entry box
						SetCtrlAttribute (giPanelScanData, UUTDATA_SDBSN, ATTR_DIMMED, 0);	   			
	   			
		   				//	Select uut PN field as active
		   				SetActiveCtrl (giPanelScanData, UUTDATA_SDBSN);
						
						// save the uut serial number
						strcpy(gcUutSerNum,cScanVal);
		   			}
		   			else
		   			{
		   				ResetTextBox (giPanelScanData, UUTDATA_UUTSN, "");	   			
						SetCtrlAttribute (giPanelScanData, UUTDATA_OK, ATTR_DIMMED, 1);
						ProcessDrawEvents();
		   			}
					break;
					
				case UUTDATA_SDBSN:
		   			//	Advance to the next field if SN length is equal to the standard twelve characters.
			    	GetCtrlVal(giPanelScanData, UUTDATA_SDBSN, cScanVal);
				
		   			if(strlen(cScanVal) == giSerNum1Len)
		   			{
	   					//	Un-Dim NIM entry box
						SetCtrlAttribute (giPanelScanData, UUTDATA_NIMSN, ATTR_DIMMED, 0);	   			
	   			
		   				//	Select NIM entry box as active
		   				SetActiveCtrl (giPanelScanData, UUTDATA_NIMSN);
						
						// save the sdb serial number
						strcpy(gcSdbSerNum,cScanVal);
		   			}
		   			else
		   			{
		   				ResetTextBox (giPanelScanData, UUTDATA_SDBSN, "");	   			
						SetCtrlAttribute (giPanelScanData, UUTDATA_OK, ATTR_DIMMED, 1);
						ProcessDrawEvents();
		   			}
					break;
					
				case UUTDATA_NIMSN:
		   			//	Advance to the next field if SN length is equal to the standard twelve characters.
			    	GetCtrlVal(giPanelScanData, UUTDATA_NIMSN, cScanVal);
				
		   			if(strlen(cScanVal) == giSerNum1Len)
		   			{
	   					//	Un-Dim OK button
						SetCtrlAttribute (giPanelScanData, UUTDATA_MAC, ATTR_DIMMED, 0);	   			
	   			
		   				//	Select mac address box as active
		   				SetActiveCtrl (giPanelScanData, UUTDATA_MAC);
						
						// save the uut serial number
						strcpy(gcNimSerNum,cScanVal);
		   			}
		   			else
		   			{
		   				ResetTextBox (giPanelScanData, UUTDATA_NIMSN, "");	   			
						SetCtrlAttribute (giPanelScanData, UUTDATA_OK, ATTR_DIMMED, 1);
						ProcessDrawEvents();
		   			}
					break;
					
				case UUTDATA_MAC:
		   			//	Advance to the next field if SN length is equal to the standard twelve characters.
			    	GetCtrlVal(giPanelScanData, UUTDATA_MAC, cScanVal);
				
		   			if(strlen(cScanVal) == giSerNum1Len)
		   			{
	   					//	Un-Dim OK button
						SetCtrlAttribute (giPanelScanData, UUTDATA_OK, ATTR_DIMMED, 0);	   			
	   			
		   				//	Select OK button as active
		   				SetActiveCtrl (giPanelScanData, UUTDATA_OK);
						
						// save the uut serial number
						strcpy(gcMacAddr,cScanVal);
		   			}
		   			else
		   			{
		   				ResetTextBox (giPanelScanData, UUTDATA_MAC, "");	   			
						SetCtrlAttribute (giPanelScanData, UUTDATA_OK, ATTR_DIMMED, 1);
						ProcessDrawEvents();
		   			}
					break;
			}	 
	   		break;
	}
	return 0;
}

/*
 *	ResetUUTScanBox()
 *
 *	Purpose: This function clears all scan UUT data fields
 *
 */
int ResetUUTScanBox()
{
	//	Reset all text fields
	ResetTextBox (giPanelScanData, UUTDATA_UUTSN, "");
	ResetTextBox (giPanelScanData, UUTDATA_SDBSN, "");
	ResetTextBox (giPanelScanData, UUTDATA_NIMSN, "");
	ResetTextBox (giPanelScanData, UUTDATA_MAC,   "");
	
	//	Make Chassis SN field entry box active
	SetActiveCtrl(giPanelScanData, UUTDATA_UUTSN);
	
	//	Dim OK button until all fields are valid
	SetCtrlAttribute (giPanelScanData, UUTDATA_OK, ATTR_DIMMED, 1);
	
	ProcessDrawEvents();

	return 0;
}
 
/*
 *  Function: OpenLogFile 
 *
 *  Purpose:  This function opens or closes the serial log file.						
 */
 int OpenLogFile(int state)
 {
	char tmpStr[100];
	
	if (state == OPEN) // if state is Open, then create and open the new logfile
	{
		CreateDateTimeStr(); // generate the date and time
		sprintf(gcLogFilePath,"C:\\Cloudshield\\Test_Log_Files\\FAT_Test_Log_Files\\FatTestSerLogFile_%s_%s.txt",gcSerialNum,gcDateTimeBuffer);
		sprintf(tmpStr,";########## Start of the Functional Test logfile for CS2000 UUT Board: %s ##########\n",gcSerialNum);
		giLogFileHandle = OpenFile (gcLogFilePath, VAL_READ_WRITE, VAL_TRUNCATE, VAL_ASCII);
		WriteLine(giLogFileHandle,tmpStr,StringLength(tmpStr));
		giLogFileOpen = TRUE;
	}
	else
		CloseFile(giLogFileHandle);
	return 0;
 }

/********************************************************************************
 *  Function: OpenTail_CB																	
 *  Purpose:																	
 *      This callback function starts the tail utility that displays the serial	
 *		input from the system tests and is logged into the test log file.			
 */
int CVICALLBACK OpenTail_CB (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	char logFilePath[100] = {"\0"};
	int fileExists;
	long size;
	
	switch (event)
	{
		Delay(0.1);
		case EVENT_COMMIT:
			
			if (!giConsoleOpen) // if a console is already open, dont try to open another one
			{
				// open the serial log file to track test files
				giConsoleOpen = TRUE; // set the tail utility open flag
				fileExists = FileExists(gcLogFilePath,&size); // see if file exists first before launching console
				if (fileExists) // see if the logfile exists
				{   // copy the log file path into a global variable
					sprintf(gcTailLogFile,"C:\\UnxUtils\\usr\\local\\wbin\\tail.exe -f -n 400 %s",gcLogFilePath);
					// start the tail utility in a separate thread
					CmtScheduleThreadPoolFunction(DEFAULT_THREAD_POOL_HANDLE, TailThreadFunct, NULL, &gFunctionId);
					ProcessSystemEvents(); // Allow other system events to be processed while we're waiting 
				}
			}
			break;
	}
	return 0;
}

/********************************************************************************
 *  Function: OpenTail()																	
 *  Purpose:																	
 *      This non-callback function starts the tail utility that displays the serial	
 *		input from the system tests and is logged into the test log file.			
 */
int OpenTail()
{
	char logFilePath[100] = {"\0"};
	char *fileName[] = {"C:\\Cloudshield\\Config\\Test_Log_Files\\FAT_Test_Log_Files\\testLogFile.txt"}; 
	int fileExists;
	long size;
	
	if (!giConsoleOpen) // if a console is already open, dont try to open another one
	{
		// open the serial log file to track test files
		giConsoleOpen = TRUE; // set the tail utility open flag
		fileExists = FileExists(gcLogFilePath,&size); // see if file exists first before launching console
		
		if (fileExists) // see if the logfile exists
		{   // copy the log file path into a global variable
			sprintf(gcTailLogFile,"C:\\UnxUtils\\usr\\local\\wbin\\tail.exe -f -n 300 %s",gcLogFilePath);
			// start the tail utility in a separate thread
			CmtScheduleThreadPoolFunction(DEFAULT_THREAD_POOL_HANDLE, TailThreadFunct, NULL, &gFunctionId);
			ProcessSystemEvents(); // Allow other system events to be processed while we're waiting 
		}
	}
	return 0;
}

/********************************************************************************
 *  Function: TailThreadFunct															  	
 *  Purpose:																	
 *      This callback function opens the tail utility in a separate thread. 	
 */
int CVICALLBACK TailThreadFunct (void *functionData)
{
	int giTailHandle;
	int returnValue;
	
	LaunchExecutableEx (gcTailLogFile, LE_SHOWNORMAL, &giTailHandle); // launch the tail console utility
	
	while (!ExecutableHasTerminated (giTailHandle)) // stay here while the executable is open
		ProcessSystemEvents(); // Allow other system events to be processed while the tail program is running 
		
//	CmtExitThreadPoolThread (returnValue);

	giConsoleOpen = FALSE;
	
	return 0;
}

void CVICALLBACK GuiExit_CB (int menuBar, int menuItem, void *callbackData, int panel)
{
	if (menuItem == MENUBAR_FILE_EXIT) // exit if menu exit clicked
		QuitUserInterface(0);
}

int CVICALLBACK CmdOk_CB (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			QuitUserInterface(0);
			break;
	}
	return 0;
}

void CVICALLBACK EnableSingleTest_CB (int menubar, int menuItem, void *callbackData, int panel)
{
	int i;
	
	if (menuItem == MENUBAR_FILE_ENABLE) // exit if menu exit clicked
	{
		giSingleTestFlg ^= 1;

		if (giSingleTestFlg)
		{																			
	    	SetCtrlAttribute (gMainPanel, PANEL_SELECT, ATTR_VISIBLE, 1);
//	    	SetCtrlAttribute (gMainPanel, PANEL_SCANIN, ATTR_VISIBLE, 1);
	    	SetCtrlAttribute (gMainPanel, PANEL_LOADDATA, ATTR_VISIBLE, 1);
	    	SetCtrlAttribute (gMainPanel, PANEL_TESTLIST, ATTR_SHOW_MARKS, 1);
			SetMenuBarAttribute(giMenuBar, MENUBAR_FILE_ENABLE, ATTR_CHECKED, 1);
			
			// display the test list in the gui list box
			for (i=0; i<giNumItems; i++)
		    	SetTreeItemAttribute (gMainPanel, PANEL_TESTLIST, i, ATTR_MARK_TYPE, VAL_MARK_CHECK);
		}
		else
		{
	    	SetCtrlAttribute (gMainPanel, PANEL_SELECT, ATTR_VISIBLE, 0);
	    	SetCtrlAttribute (gMainPanel, PANEL_SCANIN, ATTR_VISIBLE, 0);
	    	SetCtrlAttribute (gMainPanel, PANEL_LOADDATA, ATTR_VISIBLE, 0);
	    	SetCtrlAttribute (gMainPanel, PANEL_TESTLIST, ATTR_SHOW_MARKS, 0);
			SetMenuBarAttribute(giMenuBar, MENUBAR_FILE_ENABLE, ATTR_CHECKED, 0);
			
			// display the test list in the gui list box
			for (i=0; i<giNumItems; i++)
		    	SetTreeItemAttribute (gMainPanel, PANEL_TESTLIST, i, ATTR_MARK_TYPE, VAL_MARK_NONE);
		}
	}
}

int CVICALLBACK SelectAllTests_CB (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	int i, numItems; 
	int chkListNum[25];
	
	switch (event)
	{
		case EVENT_COMMIT:
			
			giCheckAll ^= 1;
			
			if (giIsDppmGe) // get the number of items in the GigE test list
				numItems = Ini_NumberOfItems (gIniText,"DPPM GigETest List");
			else if (giIsDppmSonet) // get the number of items in the Sonet test list
				numItems = Ini_NumberOfItems (gIniText,"DPPM Sonet Test List");
			else if (giIsDppm10Ge) // get the number of items in the 10Ge test list
				numItems = Ini_NumberOfItems (gIniText,"DPPM 10Ge Test List");
	
			for (i=0; i<numItems; i++) // clear the check marks from the list box
				CheckListItem(gMainPanel, PANEL_TESTLIST, i, giCheckAll); 
			
			if (giCheckAll)
				SetCtrlAttribute(gMainPanel, PANEL_SELECT, ATTR_CTRL_VAL, giCheckAll);
			else
				SetCtrlAttribute(gMainPanel, PANEL_SELECT, ATTR_CTRL_VAL, giCheckAll);
			break;
	}
	return 0;
}

int CVICALLBACK ScanData_CB (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			giSingleTestFlg = 0;    // clear the single test flag in order to clear the scan entry fields
			ScanAndVerifyUUTData(); // new board, so scan in uut data
			giSingleTestFlg = 1;    // reset the single test flag
			break;
	}
	return 0;
}

int CVICALLBACK MakeScanInButtonVisible_CB (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	int numChkItems;
	
	switch (event)
	{
		case EVENT_COMMIT:
			giItemChked = 1;      // set the item checked flag
//    		SetCtrlAttribute (gMainPanel, PANEL_SCANIN, ATTR_VISIBLE, 1);
			break;
	}
	return 0;
}

int CVICALLBACK TestLogOK_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

/*
 *	Clb_ToggleTerminalWindow()
 *
 *	Purpose:
 *		This callback function toggles the terminal window
 *
 */
int  CVICALLBACK Clb_ToggleTerminalWindow(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    switch (event)
    {
	    case EVENT_COMMIT: 	
	    
	    	//	Hide terminal window
			if(giDisplayTerminal)
			{
				giDisplayStdWindow = 0;
				giDisplayTerminal = 0;
				SetStdioWindowVisibility (0);
				SetPanelAttribute (gMainPanel, ATTR_LEFT, giLeftPos);
			}
			//	Show terminal window
			else
			{
				//	Configure terminal window parameters
				SetPanelAttribute (gMainPanel, ATTR_LEFT, giLeftPos - 255);
				SetStdioWindowPosition (giTopPos, giLeftPos + 492);
				SetStdioPort (CVI_STDIO_WINDOW);
				SetStdioWindowOptions (200, 0, 0);
				SetStdioWindowSize (684, 500);
				SetStdioWindowVisibility (1);
				giDisplayStdWindow = 1;
				giDisplayTerminal = 1;
			}
			
			break;
	    default: 	
	    	break;
	}
	return 0;
}
