// Functional Test sequence

#include <userint.h>
#include <windows.h>
#include "inet.h"
#include "toolbar.h"
#include "combobox.h"
#include <aardvark.h>
#include "regexpr.h"
#include "FatTestGui.h"
#include "FatTestMain.h"

#define kMargin 5

int PAGE_BYTE_COUNT = 128;
const int HEADER_PCODE_LENGTH = 16;
const int HEADER_PN_LENGTH = 20;
const int HEADER_SN_LENGTH = 16;
const int HEADER_CLEI_LENGTH = 10;
const int HEADER_MFGDATE_LENGTH = 10;
const int HEADER_HWOPTION_SIZE = 2;

word CURRENT_FORMAT = 0x0100;
word MAGIC_NUMBER   = 0xAAAA;
word SCHEMA_VERSION = 0x0100;
word ABS_OFFSET     = 0x0000;

static CAObjHandle webHandle;
static ToolbarType hToolbar;
static int hMenu;

extern const int CSPAGE_BYTE_SIZE;
extern const int CRC_CALC_START_OFFSET;

HRESULT CVICALLBACK NavigateComplete_CB (CAObjHandle caServerObjHandle, void *caCallbackData, CAObjHandle  pDisp, VARIANT *URL);

/*
 *	RunFunctionalTestsAll()
 *
 *  this function sequences through the list of tests for the selected uut
 */
int RunFunctionalTestsAll()
{
	int giStatus = 0;
	
	Run (DataEntry()); 			// enter the uut data information
	
	Run (PrgmPowerSequencer()); // program the power sequencer
	
	Run (ChkPUPSVoltages());  	// check and verify the dppm voltages

	Run (PgrmJtagChain ());   	// program the xilinx cpld

	Run (PgrmRedBoot());	   	// program the reboot flash
	
	Run (RedBootMemTests());   	// run the redboot memory tests
	
	Run (PgrmTmmBrdEeprom()); 	// programm the dppm base board eeprom
	
	Run (PgrmSdbBrdEeprom()); 	// program the sdb board eeprom

	Run (PgrmNimBrdEeprom()); 	// program the nim board eeprom
	
	Run (PgrmMacPhy());       	// program the mac phy device
	
	Run (ChkStatusLEDs());	   	// check and verify the faceplate status leds
	
	Run (ChkNimLEDs());	   		// check and verify the nim status leds

	Run (PAXMemoryTest());	   	// run the pax memory test
	
	if (giIsDppm10Ge) 			// run the qdram test for the 10Ge board only
		Run (QDRamMemoryTest());
	
	Run (CAMMemoryTest());	   	// run the cam memory test
	
	Run (SDBMemoryTest());	   	// run the sdb memory test

	if (giIsDppm10Ge) 			// check if 10Ge board before running rocket IO
		Run (RocketIO10GeTest());
	if (giIsDppmGe || giIsDppmSonet) 
		Run (RocketIOTest()); 	// run the rocket io test
	
	Run (UutTrafficTest());	   	// run the sdb memory test
	
	Run (CapturePortTest());	// run the sdb memory test
Exit:
	if (giStopFlg)				// if stop flag, set status = 2
		giStatus = 2;
	
	return giStatus;
}

/*
 *	RunFunctionalTestsChk()
 *
 *  this function runs selected tests that are checked in the gui 
 */
int RunFunctionalTestsSel()
{
	int iStatus = 0;
	int itemChked = 0;
	int ctrlIndex = 0;
	char testStr[75] = "\0";

	GetTreeItemFromLabel (gMainPanel, PANEL_TESTLIST, VAL_ALL, 0, VAL_FIRST, VAL_NEXT_PLUS_SELF, 0, 
						  "Enter UUT barcode data.", &gCurListBoxLine);
	GetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_MARK_STATE, &itemChked);
	if (itemChked && !giStopFlg)
		if (DataEntry(itemChked)) iStatus++;   // enter the uut data information

	GetTreeItemFromLabel (gMainPanel, PANEL_TESTLIST, VAL_ALL, 0, VAL_FIRST, VAL_NEXT_PLUS_SELF, 0, 
		                  "Program the ADM1060 Power Sequencer!", &gCurListBoxLine);
	GetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_MARK_STATE, &itemChked);
	if (itemChked && !giStopFlg)
		if (PrgmPowerSequencer()) iStatus++; // program the power sequencer
	
	GetTreeItemFromLabel (gMainPanel, PANEL_TESTLIST, VAL_ALL, 0, VAL_FIRST, VAL_NEXT_PLUS_SELF, 0, 
		                  "Aquire and verify the DPPM voltages.", &gCurListBoxLine);
	GetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_MARK_STATE, &itemChked);
	if (itemChked && !giStopFlg)
		if (ChkPUPSVoltages()) iStatus++;  // check and verify the dppm voltages

	GetTreeItemFromLabel (gMainPanel, PANEL_TESTLIST, VAL_ALL, 0, VAL_FIRST, VAL_NEXT_PLUS_SELF, 0, 
		                  "Program the JTAG CPLD device!", &gCurListBoxLine);
	GetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_MARK_STATE, &itemChked);
	if (itemChked && !giStopFlg)
		if (PgrmJtagChain ()) iStatus++;   // program the xilinx cpld

	GetTreeItemFromLabel (gMainPanel, PANEL_TESTLIST, VAL_ALL, 0, VAL_FIRST, VAL_NEXT_PLUS_SELF, 0, 
		                  "Program the Redboot Flash device!", &gCurListBoxLine);
	GetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_MARK_STATE, &itemChked);
	if (itemChked && !giStopFlg)
		if (PgrmRedBoot()) iStatus++;	   // program the reboot flash
	
	GetTreeItemFromLabel (gMainPanel, PANEL_TESTLIST, VAL_ALL, 0, VAL_FIRST, VAL_NEXT_PLUS_SELF, 0, 
		                  "DPPM RedBoot Memory tests.", &gCurListBoxLine);
	GetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_MARK_STATE, &itemChked);
	if (itemChked && !giStopFlg)
		if (RedBootMemTests()) iStatus++;  // run the redboot memory tests
	
	GetTreeItemFromLabel (gMainPanel, PANEL_TESTLIST, VAL_ALL, 0, VAL_FIRST, VAL_NEXT_PLUS_SELF, 0, 
		                  "Program the Baseboard EEPROM device!", &gCurListBoxLine);
	GetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_MARK_STATE, &itemChked);
	if (itemChked && !giStopFlg)
		if (PgrmTmmBrdEeprom()) iStatus++; // programm the dppm base board eeprom
	
	GetTreeItemFromLabel (gMainPanel, PANEL_TESTLIST, VAL_ALL, 0, VAL_FIRST, VAL_NEXT_PLUS_SELF, 0, 
		                  "Program the SDB EEPROM device!", &gCurListBoxLine);
	GetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_MARK_STATE, &itemChked);
	if (itemChked && !giStopFlg)
		if (PgrmSdbBrdEeprom()) iStatus++; // program the sdb board eeprom

	GetTreeItemFromLabel (gMainPanel, PANEL_TESTLIST, VAL_ALL, 0, VAL_FIRST, VAL_NEXT_PLUS_SELF, 0, 
		                  "Program the NIM EEPROM device!", &gCurListBoxLine);
	GetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_MARK_STATE, &itemChked);
	if (itemChked && !giStopFlg)
		if (PgrmNimBrdEeprom()) iStatus++; // program the nim board eeprom
	
	GetTreeItemFromLabel (gMainPanel, PANEL_TESTLIST, VAL_ALL, 0, VAL_FIRST, VAL_NEXT_PLUS_SELF, 0, 
		                  "Program the MAC Phy device!", &gCurListBoxLine);
	GetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_MARK_STATE, &itemChked);
	if (itemChked && !giStopFlg)
		if (PgrmMacPhy()) iStatus++;       // program the mac phy device
	
	GetTreeItemFromLabel (gMainPanel, PANEL_TESTLIST, VAL_ALL, 0, VAL_FIRST, VAL_NEXT_PLUS_SELF, 0, 
		                  "Status LEDs verification test.", &gCurListBoxLine);
	GetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_MARK_STATE, &itemChked);
	if (itemChked && !giStopFlg)
		if (ChkStatusLEDs()) iStatus++;	   // check and verify the faceplate status leds
	
	if (giIsDppmGe) sprintf(testStr,"GigE NIM LEDs verification test.");
	else if (giIsDppmSonet) sprintf(testStr,"Sonet NIM LEDs verification test.");
	else if (giIsDppm10Ge) sprintf(testStr,"10Ge NIM LEDs verification test.");
	GetTreeItemFromLabel (gMainPanel, PANEL_TESTLIST,VAL_ALL,0,VAL_FIRST,VAL_NEXT_PLUS_SELF,0,testStr,&gCurListBoxLine);
	GetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_MARK_STATE, &itemChked);
	if (itemChked && !giStopFlg)
		if (ChkNimLEDs()) iStatus++;	   // check and verify the nim status leds

	if (giIsDppmGe) sprintf(testStr,"GigE PAX Memory test.");
	else if (giIsDppmSonet) sprintf(testStr,"Sonet PAX Memory test.");
	else if (giIsDppm10Ge) sprintf(testStr,"10Ge PAX Memory test.");
	GetTreeItemFromLabel (gMainPanel, PANEL_TESTLIST,VAL_ALL,0,VAL_FIRST,VAL_NEXT_PLUS_SELF,0,testStr,&gCurListBoxLine);
	GetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_MARK_STATE, &itemChked);
	if (itemChked && !giStopFlg)
		if (PAXMemoryTest()) iStatus++;	   // run the pax memory test
	
	if (giIsDppm10Ge)
	{
		sprintf(testStr,"10Ge QDRam Memory test.");
		GetTreeItemFromLabel (gMainPanel, PANEL_TESTLIST,VAL_ALL,0,VAL_FIRST,VAL_NEXT_PLUS_SELF,0,testStr,&gCurListBoxLine);
		GetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_MARK_STATE, &itemChked);
		if (itemChked && !giStopFlg)
			if (QDRamMemoryTest()) iStatus++;
	}
	if (giIsDppmGe) sprintf(testStr,"GigE CAM Memory test.");
	else if (giIsDppmSonet) sprintf(testStr,"Sonet CAM Memory test.");
	else if (giIsDppm10Ge) sprintf(testStr,"10Ge CAM Memory test.");
	GetTreeItemFromLabel (gMainPanel, PANEL_TESTLIST,VAL_ALL,0,VAL_FIRST,VAL_NEXT_PLUS_SELF,0,testStr,&gCurListBoxLine);
	GetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_MARK_STATE, &itemChked);
	if (itemChked && !giStopFlg)
		if (CAMMemoryTest()) iStatus++;	   // run the cam memory test
	
	if (giIsDppmGe) sprintf(testStr,"GigE SDB Memory test.");
	else if (giIsDppmSonet) sprintf(testStr,"Sonet SDB Memory test.");
	else if (giIsDppm10Ge) sprintf(testStr,"10Ge SDB Memory test.");
	GetTreeItemFromLabel (gMainPanel, PANEL_TESTLIST,VAL_ALL,0,VAL_FIRST,VAL_NEXT_PLUS_SELF,0,testStr,&gCurListBoxLine);
	GetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_MARK_STATE, &itemChked);
	if (itemChked && !giStopFlg)
		if (SDBMemoryTest()) iStatus++;	   // run the sdb memory test

	if (giIsDppmGe) sprintf(testStr,"GigE Rocket IO Ports test.");
	else if (giIsDppmSonet) sprintf(testStr,"Sonet Rocket IO Ports test.");
	else if (giIsDppm10Ge) sprintf(testStr,"10Ge Rocket IO Ports test.");
	GetTreeItemFromLabel (gMainPanel, PANEL_TESTLIST,VAL_ALL,0,VAL_FIRST,VAL_NEXT_PLUS_SELF,0,testStr,&gCurListBoxLine);
	GetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_MARK_STATE, &itemChked);
	if (itemChked && !giStopFlg)
	{
		if (giIsDppmGe || giIsDppmSonet) 
		{
			if (RocketIOTest()) iStatus++; // run the GigE or Sonet rocket io test
		}
		else
		{
			if (RocketIO10GeTest()) iStatus++;  // run the 10Ge rocket io test
		}
	}
	if (giIsDppmGe) sprintf(testStr,"GigE Traffic test.");
	else if (giIsDppmSonet) sprintf(testStr,"Sonet Traffic test.");
	else if (giIsDppm10Ge) sprintf(testStr,"10Ge Traffic test.");
	GetTreeItemFromLabel (gMainPanel, PANEL_TESTLIST,VAL_ALL,0,VAL_FIRST,VAL_NEXT_PLUS_SELF,0,testStr,&gCurListBoxLine);
	GetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_MARK_STATE, &itemChked);
	if (itemChked && !giStopFlg)
		if (UutTrafficTest()) iStatus++;  // run the traffic test
		
	if (giIsDppmGe) sprintf(testStr,"GigE Capture Port Traffic test.");
	else if (giIsDppmSonet) sprintf(testStr,"Sonet Capture Port Traffic test.");
	else if (giIsDppm10Ge) sprintf(testStr,"10Ge Capture Port Traffic test.");
	GetTreeItemFromLabel (gMainPanel, PANEL_TESTLIST,VAL_ALL,0,VAL_FIRST,VAL_NEXT_PLUS_SELF,0,testStr,&gCurListBoxLine);
	GetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_MARK_STATE, &itemChked);
	if (itemChked && !giStopFlg)
		if (CapturePortTest()) iStatus++;  // run the capture port traffic test

	if (giStopFlg)	// if stop flag, set status = 2
		iStatus = 2;
	
	return iStatus;
}

void SetTestStatus(int color, char* status)
{
	int iStatus = 0;
	
	SetTreeCellAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, 1, ATTR_LABEL_COLOR, color);  
	SetTreeCellAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, 1, ATTR_LABEL_TEXT, status);
	ProcessDrawEvents();
}

/*
 *	DataEntry()
 *
 *  this function opens up the uut data information panel for data entry.
 */
int DataEntry(int checked)
{
	GetTreeItemFromLabel (gMainPanel, PANEL_TESTLIST, VAL_ALL, 0, VAL_FIRST, VAL_NEXT_PLUS_SELF, 0, 
						  "Enter UUT barcode data.", &gCurListBoxLine);
	
	CheckListItem(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, 1); 
	SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, -1); 	
	SetTestStatus(VAL_BLUE, "Entering UUR data");
	if (checked)
		giSingleTestFlg = 0; // clear the single test flag in order to clear the scan entry fields
	
	ScanAndVerifyUUTData();  // open an entry panel and scan in the serial numbers and mac address

	if (checked)
		giSingleTestFlg = 1; // reset the single test flag
	SetTestStatus(VAL_DK_GREEN, "Done");
	SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, 1); 	

	return 0;
}

/*
 *	PgrmPowerSequencer()
 *
 *  this function programs the power sequencer device using the Aardvark I2C device.
 */
int PrgmPowerSequencer ()
{
	int   iStatus = 0;
    int   iPort   = 0;
    int   bitrate = 100;
    int   res     = 0;
	int   iBytesRead;
	int	  i, j, ii, x, y, fh;
    int   logfile = 0;
	int   lineCnt = 0;
	unsigned int iByteVal = 0;
	byte  bData[9][32]; // 8 blocks of data with 32 bytes of code in each block
	word  eepromAddr[9];
	word  uiPort[10];
	char  cmdStr[100] = "\0";
	char  addrStr[5];
	char  cCurrLine[150] = "\0";
	char  pathStr[75]  = "\0";
	char  dataStr[110] = "\0";
	char  dataByte[3]  = "\0";
	
	GetTreeItemFromLabel (gMainPanel, PANEL_TESTLIST, VAL_ALL, 0, VAL_FIRST, VAL_NEXT_PLUS_SELF, 0, 
		                  "Program the ADM1060 Power Sequencer!", &gCurListBoxLine);
	
	CheckListItem(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, 1); 
	sprintf(cmdStr,"\n########## Power Sequencer Programming ##########\n"); // write 
	WriteFile (giLogFileHandle,cmdStr,StringLength(cmdStr));
	SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, -1); 	
	SetTestStatus(VAL_BLUE, "Initializing.......");
	
	giStatus = 0; // clear the error status flag
	memset(addrStr,0,5);
	
	I2CEEPromTargetInfo(PwrSequencer); // get the I2C slave address for the power sequencer

	// get the available port
	aa_find_devices(10, uiPort);
	
	Try (OpenAardvarkPort(0, TRUE)); // Open the device at port 0 and enable the i2c

    // Enable logging
	sprintf(pathStr,"C:\\Cloudshield\\Test_Log_Files\\Aardvark\\ardvarklog.txt");
    logfile = OpenFile (pathStr, VAL_READ_WRITE, VAL_TRUNCATE, VAL_ASCII);
    if (logfile != 0) 
	{
        aa_log(hAardvarkHandle, 3, logfile);
    }
	// open the power sequencer programming file
	fh = OpenFile ("C:\\Cloudshield\\Pgm Files\\DPPM\\ADM1060-DPPM-RegularSeq.txt", VAL_READ_ONLY , VAL_OPEN_AS_IS, VAL_ASCII);
	
	do // loop through the program file and parse out the hex bytes, 32 bytes per block
	{
		iBytesRead = ReadLine ( fh, cCurrLine, -1); // read in a line of data
		
		if ((cCurrLine == "") || (cCurrLine[0] == ';')) // skip comments which start with a ';'
			continue;									// as well as any empty lines
		
		strcat(cCurrLine,"\r");  // add a cr to the end of the line
		x = FindPattern(cCurrLine, 0, -1,"=", 1, 0) + 2;  // get the start of the line
		y = FindPattern(cCurrLine, 0, -1,"\r", 1, 0) - 1; // get the end of the line

		for (j=0; j<4; j++)  // parse the address string
		{
			addrStr[j] = cCurrLine[j];
		}
		Scan(addrStr,"%s>%x[b2]",&eepromAddr[lineCnt]); // convert it to unsigned short (word)
		
		for (i=0; i<y; i++, x++)  // copy the line without the address and '=' sign
			dataStr[i] = cCurrLine[x];
		
		x = strlen(dataStr) - 1; // get the length of the string without the cr
		memset(dataByte,0,3);	 // clear out the data byte location
		
		for (i=0, j=0; i<32; i++, j++) // parse through each line for the 32 hex bytes per line
		{							   // 6 lines of code with 32 bytes per line
			if (dataStr[j] != ' ')	   // skip the spaces
			{
				for (ii=0; ii<2; ii++) // each hex value = 2 hex characters: example "FF"
				{
					dataByte[ii] = dataStr[j]; // save the 2 char hex value
					j++;					   // and bump the valid hex char pointer
				}
				Scan(dataByte,"%s>%x",&iByteVal);   // convert the hex chars to hex values
				bData[lineCnt][i] = (byte)iByteVal; // and save them to the write block array
			}
		}
		lineCnt++; // bump the line counter
		
	} while (iBytesRead >= 0 && lineCnt < 8); // read until all configuration lines have been parsed
	
	SetTestStatus(VAL_BLUE, "Erasing Device");
	
	if (ADM1060EraseConfigEEprom()) // erase the config section of the adm1060 eeprom
	{
		SetTestStatus(VAL_BLUE, "Programming Device");
	
		for (i=0; i<8; i++) // write the program data blocks to the eeprom
		{
			iStatus ^= ADM1060WriteEEprom(eepromAddr[i], bData[i], 32);
		}
	}
	else
		iStatus = 1;
	
	CloseAardvarkPort(hAardvarkHandle); // close the aardvark i2c device port

    CloseFile(logfile); // Close the logging file
	
	CloseFile(fh); // close the programming file
exit:	
	if (giStatus) // check if global error status error flag is set
		iStatus = 1;
	
	// display pass or fail statusand save the test result  
	if (iStatus)
	{
		strcpy(gsTestStatus.cPgmPwrSeq,"Fail");
		SetTestStatus(VAL_RED, "Fail");
		SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, 4);
		strcat(gsComment,"Pgm Pwr Seq, ");
	}													  
	else
	{
		strcpy(gsTestStatus.cPgmPwrSeq,"Pass");
		SetTestStatus(VAL_DK_GREEN, "Pass");
		SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, 1); 	
	}
	return iStatus;
}

/*
 *	ChkPUPSVoltages()
 *
 *  this function verifies that the dppm uut voltages are within spec
 */
int ChkPUPSVoltages()
{
	int i, ch, iStatus = 0;
	char testStr[75] = "\0";
	char voltStr[6][20] = {0};
	float voltVal[6] = {0};
	
	// get the list box line
	GetTreeItemFromLabel (gMainPanel, PANEL_TESTLIST, VAL_ALL, 0, VAL_FIRST, VAL_NEXT_PLUS_SELF, 0, 
		                  "Aquire and verify the DPPM voltages.", &gCurListBoxLine);
	
	CheckListItem(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, 1); 
	sprintf(testStr,"\n########## PUPS Voltage check ##########\n"); // write 
	WriteFile (giLogFileHandle,testStr,StringLength(testStr));
	SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, -1); 	
	SetTestStatus(VAL_BLUE, "Initializing Agilent Devices");

	ViOpenAgilentSwitch(); // open a session to the agilent 34970A switch
	ViOpenAgilentDmm();    // open a session to the agilent 34401A dmm
	
	WriteGPIB(hViDmm,"FUNC:RANG:AUTO ON\n");       // turn on the dvm autoranging feature
	WriteGPIB(hViSwitch,"ROUT:OPEN (@101:120)\n"); // open channels 1 to 20 on slot 100 of the mux module
	Delay(2.0); // delay for power to settle before taking readings
	
	// get the +1.0V voltage reading
	WriteGPIB(hViSwitch,"ROUT:CLOSE:EXCL (@101)\n"); // close the channel 1 switch
	Delay(0.1);										 // delay after closing the switch
	WriteGPIB(hViDmm,"TRIG:SOUR IMM\n"); 			 // set the internal trigger source
	viQueryf(hViDmm, "READ?\n","%t", voltStr[0]);	 // send the read command and read back the value
	WriteGPIB(hViSwitch,"ROUT:OPEN (@101)\n"); 		 // open the channel 1 switch
	voltVal[0] = atof(voltStr[0]);					 // convert the string value to a float value
	SetTestStatus(VAL_BLUE, "Measuring 1.0Vdc");
	
	// get the +1.3V voltage reading
	WriteGPIB(hViSwitch,"ROUT:CLOSE:EXCL (@102)\n"); // close the channel 2 switch
	Delay(0.1);
	WriteGPIB(hViDmm,"TRIG:SOUR IMM\n");
	viQueryf(hViDmm,"READ?\n","%t", voltStr[1]);
	WriteGPIB(hViSwitch,"ROUT:OPEN (@102)\n"); 		 // open the channel 2 switch
	voltVal[1] = atof(voltStr[1]);
	SetTestStatus(VAL_BLUE, "Measuring 1.3Vdc");
	
	// get the +1.5V voltage reading
	WriteGPIB(hViSwitch,"ROUT:CLOSE:EXCL (@103)\n"); // close the channel 3 switch
	Delay(0.1);
	WriteGPIB(hViDmm,"TRIG:SOUR IMM\n");
	viQueryf(hViDmm,"READ?\n","%t", voltStr[2]);
	WriteGPIB(hViSwitch,"ROUT:OPEN (@103)\n");  	 // open the channel 3 switch
	voltVal[2] = atof(voltStr[2]);
	SetTestStatus(VAL_BLUE, "Measuring 1.5Vdc");
	
	// get the +1.8V voltage reading
	WriteGPIB(hViSwitch,"ROUT:CLOSE:EXCL (@104)\n"); // close the channel 4 switch
	Delay(0.1);
	WriteGPIB(hViDmm,"TRIG:SOUR IMM\n");
	viQueryf(hViDmm,"READ?\n","%t", voltStr[3]);
	WriteGPIB(hViSwitch,"ROUT:OPEN (@104)\n"); 		 // open the channel 4 switch
	voltVal[3] = atof(voltStr[3]);
	SetTestStatus(VAL_BLUE, "Measuring 1.8Vdc");
	
	// get the +2.5V voltage reading
	WriteGPIB(hViSwitch,"ROUT:CLOSE:EXCL (@105)\n"); // close the channel 5 switch
	Delay(0.1);
	WriteGPIB(hViDmm,"TRIG:SOUR IMM\n");
	viQueryf(hViDmm,"READ?\n","%t", voltStr[4]);
	WriteGPIB(hViSwitch,"ROUT:OPEN (@105)\n"); 		 // open the channel 5 switch
	voltVal[4] = atof(voltStr[4]);
	SetTestStatus(VAL_BLUE, "Measuring 2.5Vdc");
	
	// get the +3.3V voltage reading
	WriteGPIB(hViSwitch,"ROUT:CLOSE:EXCL (@106)\n"); // close the channel 6 switch
	Delay(0.1);
	WriteGPIB(hViDmm,"TRIG:SOUR IMM\n");
	viQueryf(hViDmm,"READ?\n","%t", voltStr[5]);
	WriteGPIB(hViSwitch,"ROUT:OPEN (@106)\n"); 		 // open the channel 6 switch
	voltVal[5] = atof(voltStr[5]);
	SetTestStatus(VAL_BLUE, "Measuring 3.3Vdc");
	
	ViCloseAgilentSwitch(); // close the session to the agilent 34970 switch unit
	ViCloseAgilentDmm();    // close the session to the agilent 34401 DMM unit
	
	SetTreeCellAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, 1, ATTR_LABEL_COLOR, VAL_BLUE);  
	SetTreeCellAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, 1, ATTR_LABEL_TEXT, "Verifying Voltages");
	
	// verify that all of the measured voltages are within spec
	if (voltVal[0] < 0.95 || voltVal[0] > 1.05) giStatus++; // verify +1.00V is within spec
	if (voltVal[1] < 1.24 || voltVal[1] > 1.36) giStatus++; // verify +1.30V is within spec
	if (voltVal[2] < 1.43 || voltVal[2] > 1.57) giStatus++; // verify +1.50V is within spec
	if (voltVal[3] < 1.71 || voltVal[3] > 1.89) giStatus++; // verify +1.80V is within spec
	if (voltVal[4] < 2.38 || voltVal[4] > 2.62) giStatus++; // verify +2.50V is within spec
	if (voltVal[5] < 3.14 || voltVal[5] > 3.46) giStatus++; // verify +3.30V is within spec
	
	// display pass or fail and save the test result
	if (iStatus)
	{
		strcpy(gsTestStatus.cChkPupVolt,"Fail");
		SetTestStatus(VAL_RED, "Fail");
		SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, 4); 	
		strcat(gsComment,"UUT Voltages, ");
	}
	else
	{
		strcpy(gsTestStatus.cChkPupVolt,"Pass");
		SetTestStatus(VAL_DK_GREEN, "Pass");
		SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, 1); 	
	}	
	return iStatus;
}

/*
 *	PgrmJtagChain()
 *
 *  this function programs the jtag chain cpld using the xilinx programming device .
 */
int PgrmJtagChain ()
{
	int i, fh;
	int iStatus = 0;
	int iBytesRead = 0;
	bool bCheckSumFound1 = FALSE;
	bool bCheckSumFound2 = FALSE;
	char cmdStr[100] = "\0";
	char testStr[75] = "\0";
	char cLineBuf[150] = "\0";
	char cCpldCS1[10] = "\0";
	char cCpldCS2[10] = "\0";
	
	// get the list box line
	GetTreeItemFromLabel (gMainPanel, PANEL_TESTLIST, VAL_ALL, 0, VAL_FIRST, VAL_NEXT_PLUS_SELF, 0, 
		                  "Program the JTAG CPLD device!", &gCurListBoxLine);
	
	CheckListItem(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, 1); 
	sprintf(cmdStr,"\n########## Xilinx JTAG Chain Programming ##########\n"); // write 
	WriteFile (giLogFileHandle,cmdStr,StringLength(cmdStr));
	SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, -1); 	
	SetTestStatus(VAL_BLUE, "Programming CPLD device");
	
	// get the xilinx command string and path
	Ini_GetStringIntoBuffer(gIniText,"Filename Paths","Xilinx", gcXilinxPath, sizeof(gcXilinxPath));
	// get the expected cpld programming checksum value
	Ini_GetStringIntoBuffer(gIniText,"Checksum Values","JTAG1", cCpldCS1, sizeof(cCpldCS1));
	Ini_GetStringIntoBuffer(gIniText,"Checksum Values","JTAG2", cCpldCS2, sizeof(cCpldCS2));
	
	if (giIsDppmSonet || giIsDppmGe) // get the correct xilinx command file based on the uut type
		strcat(gcXilinxPath," 800-0067-01.cmd");
	else if (giIsDppm10Ge)
		strcat(gcXilinxPath," 800-0059-01.cmd");
	else
	{   // come here if there is a problem
		iStatus = 1; // set the error status flag
		goto exit;   // and exit the function
	}
	
	// launch the xilinx impact executable to program the cpld(s) based on the uut
	if ((LaunchExecutableEx(gcXilinxPath, LE_HIDE, &gXilinxHandle)) == 0)
	{
		while (!ExecutableHasTerminated (gXilinxHandle))  // wait until the programming has completed
			ProcessSystemEvents();						  // allow system events to be processed
		RetireExecutableHandle (gXilinxHandle);			  // terminate the executable handle when program has terminated
	}
	else
	{   // come here if there is a problem
//		SetListItemTextAttr("Problem opening the Xilinx programming file!", BLACK, WHITE,"Fail", RED, WHITE, --gCurListBoxLine, REPLACE, 1);
		iStatus = 1; // set the error status flag
		goto exit;   // and exit the function
	}
	fh = OpenFile ("C:\\Cloudshield\\Xilinx\\XilinxRes.txt", VAL_READ_ONLY, VAL_OPEN_AS_IS, VAL_ASCII);
	
	SetTestStatus(VAL_BLUE, "Verifying Checksum(s)");
	
	do // loop through the file until the checksum(s) has/have been found or 'end of file' has been reached
	{
		iBytesRead = ReadLine ( fh, cLineBuf, -1); // read in a line of data
		
		if (giIsDppmGe  || giIsDppmSonet) // the dppm 510 and 600 have 2 cpld devices
		{
			if (FindPattern(cLineBuf, 0, -1, cCpldCS1, 0, 0) > -1) // filter out the checksum for cpld1, if found
				bCheckSumFound1 = TRUE;
		
			if (FindPattern(cLineBuf, 0, -1, cCpldCS2, 0, 0) > -1) // filter out the checksum for cpld2, if found
				bCheckSumFound2 = TRUE;
		
			if (bCheckSumFound1 && bCheckSumFound2) // if the checksum values are found, break out of the loop
				break;
		}
		else if (giIsDppm10Ge) // the dppm 800 only has 1 cpld device
		{
			if (FindPattern(cLineBuf, 0, -1, cCpldCS1, 0, 0) > -1) // filter out the checksum for the cpld if found
			{
				bCheckSumFound1 = TRUE; // set the checksum found flag and break out of the loop
				break;
			}
		}
		
	} while (iBytesRead >= 0); // read until end of file is reached
	
	CloseFile(fh); // close the xilinx log file
	
	if (giIsDppmGe || giIsDppmSonet) // check if the dppm checksum is found and matches
	{
		if (!bCheckSumFound1 || !bCheckSumFound2) // if either of the checksum values are not found, set error status 
			iStatus = 1;
	}
	else if (giIsDppm10Ge) //if the checksum value is not found or doesnt match, set the error status flag 
	{
		if (!bCheckSumFound1) 
			iStatus = 1;	  
	}
exit:	
	// display pass or fail and save the test result  
	if (iStatus)
	{
		strcpy(gsTestStatus.cPgmJtagChn,"Fail");
		SetTestStatus(VAL_RED, "Fail");
		SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, 4); 	
		strcat(gsComment,"Pgm JTAG, ");
	}
	else
	{
		strcpy(gsTestStatus.cPgmJtagChn,"Pass");
		SetTestStatus(VAL_DK_GREEN, "Pass");
		SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, 1); 	
	}
	return iStatus;
}

/*
 *	PgrmRedBoot()
 *
 *  this function programs the redboot ixp2800 flash device .
 */
int PgrmRedBoot ()
{
	int  fh, iBytesRead;
	int  iStatus    = 0;
	int  iCurrMode  = 0;
	int  stopSize   = 0;
	int  iESTStatus = 0;
	int  iDLStatus  = 0;
	int  iIteratCnt = 0;
	int  iDLSize    = 0;
	int  iOffset    = 0;
	long fileSize   = 0;
	bool bBKM = FALSE;
	char testStr[75] = "\0";
	char cRBConnect[30] = "\0";
	char cNpuPgmCfg[60] = "\0";
	char cNpuRedBin[60] = "\0";
	char cLineBuf[100]  = "\0";
	char cRetBuf[100]   = "\0";
	char cOffset[15]    = "\0";
	char cStart[15]     = "\0";
	char cStop[15]      = "\0";
	
	// get the list box line
	GetTreeItemFromLabel (gMainPanel, PANEL_TESTLIST, VAL_ALL, 0, VAL_FIRST, VAL_NEXT_PLUS_SELF, 0, 
		                  "Program the Redboot Flash device!", &gCurListBoxLine);
	
	CheckListItem(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, 1); 
	sprintf(testStr,"\n########## RedBoot Flash Programming ##########\n"); // write 
	WriteFile (giLogFileHandle,testStr,StringLength(testStr));
	SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, -1); 	
	SetTestStatus(VAL_BLUE, "Initializing.....");
	
	// get the redboot connection string, configuration and bin files 
	Ini_GetStringIntoBuffer(gIniText,"Redboot Cfg","Connect String",  cRBConnect, sizeof(cRBConnect));
	Ini_GetStringIntoBuffer(gIniText,"Redboot Cfg","NPU Program Cfg", cNpuPgmCfg, sizeof(cNpuPgmCfg));
	Ini_GetStringIntoBuffer(gIniText,"Redboot Cfg","NPU Redboot Bin", cNpuRedBin, sizeof(cNpuRedBin));
	// get and save the flash offset, start and stop addresses
	Ini_GetStringIntoBuffer(gIniText,"Redboot Cfg","Offset Address", cOffset, sizeof(cOffset));
	Ini_GetStringIntoBuffer(gIniText,"Redboot Cfg","Start Address",  cStart,  sizeof(cStart));
	Ini_GetStringIntoBuffer(gIniText,"Redboot Cfg","Stop Address",   cStop,   sizeof(cStop));
	
    giVisionProbePanel = LoadPanel (0, "FatTestGui.uir", VISIONPROB); // load the vision probe help panel

	SetPanelAttribute (giVisionProbePanel, ATTR_LEFT, giLeftPos + 492);

	InstallPopup(giVisionProbePanel);   // display the help panel

	EST_SetOpenMode(EST_MODE_UNCHANGED); // set the initial open mode to unchanged
	
	if (EST_OpenConnection(&vpHandle, cRBConnect) < 0) // open a connection to the vision probe and assign a handle
	{
		iStatus = 1;
		goto exit;
	}
	gTimeOutCnt = 0;
	do // loop until background ("BKM") mode is running
	{
		// initialize the target and the vision probe emulator
		EST_Initialize(vpHandle, EST_INN_COMMAND);
		// get the current mode, should be in BKM mode
		iCurrMode = EST_GetCurrentMode(vpHandle, EST_CHECK_MODE_BR);
		if (iCurrMode == 1)
			bBKM = TRUE;
	} while (!bBKM && gTimeOutCnt < 5);
	
	if (gTimeOutCnt >= 5)
	{
		iStatus = 1;
		goto exit;
	}
	SetTestStatus(VAL_BLUE, "Configuring IXP2800");
	
	// open and run the macro file which configures the ixp2800 for the download of the redboot binary 
	fh = OpenFile (cNpuPgmCfg, VAL_READ_ONLY , VAL_OPEN_AS_IS, VAL_ASCII);
	
	do // loop through the file and ouput each valid command line to the vision probe
	{
		iBytesRead = ReadLine ( fh, cLineBuf, -1); // read in a line of data
		
		if ((cLineBuf == "") || (cLineBuf[0] == ';')) // skip comments which start with a ';'
			continue;								  // as well as any empty lines
		
		// Send command to the emulator/probe
		if (EST_Command(vpHandle, EST_CAPTURE_RESULT, cLineBuf, cRetBuf) != EST_SUCCESS)
			iStatus = 1;
		
	} while (iBytesRead >= 0); // read until end of macro file is reached
	
	SetTestStatus(VAL_BLUE, "Erasing the Flash");
	
	// erase the flash from start address (0xC4000000) to stop address (0xC407FFFF)
	sprintf(cLineBuf,"TF ERASE %s %s", cStart, cStop);
	if (EST_Command(vpHandle, EST_CAPTURE_RESULT, cLineBuf, cRetBuf) == EST_SUCCESS)
	{
		// If buffer contains the words "...Done", it means the erasing was properly performed!
		if(FindPattern (cRetBuf, 0, -1,"... Done", 0, 1) < 0)
		{
			iStatus = 1; // exit after setting the error status flag
			goto exit;
		}
	}
	else
	{   // exit after setting the error status flag
		iStatus = 1;
		goto exit;
	}
	// close the redboot binary file
	CloseFile(fh);
	
	if (GetFileInfo(cNpuRedBin, &fileSize) > 0) // check if the file exists and get the file size
	{ 
		if ((fileSize / 1024) > 100)
			stopSize = 5; // sets the incremental download size to 5 kBytes of data
		else
			stopSize = 2; // sets the incremental download size to 2 kBytes of data

		// convert offset address string to integer value
		iOffset = atoi(cOffset);
		
		SetTestStatus(VAL_BLUE, "Programming Redboot to Flash");
		
		// download redboot binary to the target
		EST_DownLoad(vpHandle, cNpuRedBin, TARGET_FLASH, iOffset, EST_VERIFY, stopSize, DL_IS_NORMAL, &iDLStatus);
		
		// both the api status and the download status should indicate success, if not set the error status
		if (iESTStatus != EST_SUCCESS || iDLStatus != DL_SUCCESS)
			iStatus = 1;
	}
	else
		iStatus = 1; // set the error status flag if binary file does not exist
exit:
	RemovePopup(giVisionProbePanel);   // display the help panel

	EST_CloseConnection(vpHandle); // close the connection to the vision probe
	
	SetTestStatus(VAL_BLUE, "Rebooting the UUT");
	
	RedbootReboot(); // reset the dppm after redboot programming
	
	// display pass or fail and save the test result  
	if (iStatus)
	{
		strcpy(gsTestStatus.cPgmRedBoot,"Fail");
		SetTestStatus(VAL_RED, "Fail");
		SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, 4); 	
		strcat(gsComment,"Pgm Redboot, ");
	}
	else
	{
		strcpy(gsTestStatus.cPgmRedBoot,"Pass");
		SetTestStatus(VAL_DK_GREEN, "Pass");
		SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, 1); 	
	}
	return iStatus;
}

/*
 *	RedBootMemTests()()
 *
 *  this function runs the redboot memory tests and verifies the test results
 */
int RedBootMemTests()
{
	int ii, iStatus = 0;
	char testStr[75] = "\0";
	char* cQDRamMemTestKeyWord = "Test PASSED";
	
	giStatus = 0; // clear the error status flag
	
	// get the list box line
	GetTreeItemFromLabel (gMainPanel, PANEL_TESTLIST, VAL_ALL, 0, VAL_FIRST, VAL_NEXT_PLUS_SELF, 0, 
		                  "DPPM RedBoot Memory tests.", &gCurListBoxLine);
	
	CheckListItem(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, 1); 
	sprintf(testStr,"\n########## RedBoot Memory tests ##########\n"); // write 
	WriteFile (giLogFileHandle,testStr,StringLength(testStr));
	SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, -1); 	
	
	Try (OpenConnectionRS232(DPPM)); // open a com port connection to the DPPM
	
	Try (GoToRedBootPrompt()); // connect to the dppm linux login prompt
//goto exit;	
	// run the SRAM test
	SetTestStatus(VAL_BLUE, "Testing SRAM");
	WriteReadUntil(hHandle1RS232, gcReadBuffer,"memtest -s\r", gsRedBootPrompt, 10);
	
	if(FindPattern (gcReadBuffer, 0, -1,"FAILED", 0, 1) > -1) iStatus = 1;
	Delay(1.0);
	// run the RDRAM test
	SetTestStatus(VAL_BLUE, "Testing DRAM");
	WriteReadUntil(hHandle1RS232, gcReadBuffer,"memtest -d\r", gsRedBootPrompt, 210);
	
	if(FindPattern (gcReadBuffer, 0, -1,"FAILED", 0, 1) > -1) iStatus = 1;
	Delay(1.0);
	// run the NBPUScratch test
	SetTestStatus(VAL_BLUE, "NBPU Scratch Test");
 	WriteReadUntil(hHandle1RS232, gcReadBuffer,"memtest -x\r", gsRedBootPrompt, 5);
	
	if(FindPattern (gcReadBuffer, 0, -1,"FAILED", 0, 1) > -1) iStatus = 1;
	Delay(1.0);
	// check the RedBoot log for the correct version of RedBoot and CPLDs, 
//	giStatus = ChkRedBootLog();
exit:
	LogOutFromRedBoot(); // logout from redboot
	
	CloseConnection(hHandle1RS232); // disconnect the rs232 port
	
	if (giStatus) // check if global error status error flag is set
		iStatus = 1;
	
	// display pass or fail and save the test result  
	if (iStatus)
	{
		strcpy(gsTestStatus.cRedBootMem,"Fail");
		SetTestStatus(VAL_RED, "Fail");
		SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, 4); 	
		strcat(gsComment,"Redboot Memory, ");
	}
	else
	{   
		strcpy(gsTestStatus.cRedBootMem,"Pass");
		SetTestStatus(VAL_DK_GREEN, "Pass");
		SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, 1); 	
	}	
	return iStatus;
}

/*
 *	PgrmTmmBrdEeprom()
 *
 *  this function programs the uut dppm baseboard eeprom .
 */
int PgrmTmmBrdEeprom ()
{
	int  i, iBytesRead;
	int iStatus = 0;
	int iDataPointer;
	byte bPageBytes[128];
	byte bDataBytes[128];
	word iHWOption = 0;
	char testStr[75] = "\0";
	char cClei[4] = {"N/A\0"};
	char cHWOption[10] = "\0";
	char cProductCode[15] = "\0";											 
	char cPartNum[20] = "\0";
//	char* cMacAddr[8] = {"000BA9002CC6","000BA9002CC7","000BA9002CC8","000BA9002CC9",
//						 "000BA9002CCA","000BA9002CCB","000BA9002CCC","000BA9002CCD"};
	
	// get the list box line
	GetTreeItemFromLabel (gMainPanel, PANEL_TESTLIST, VAL_ALL, 0, VAL_FIRST, VAL_NEXT_PLUS_SELF, 0, 
		                  "Program the Baseboard EEPROM device!", &gCurListBoxLine);
	
	CheckListItem(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, 1); 
	sprintf(testStr,"\n########## TMM Baseboard EEPROM Programming ##########\n"); // write 
	WriteFile (giLogFileHandle,testStr,StringLength(testStr));
	SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, -1); 	
	SetTestStatus(VAL_BLUE, "Programming Common Infos Page");
	
	I2CEEPromTargetInfo(Dppm1MotherBoard); // get the I2C slave address for the base board eeprom
	
	strcpy(cProductCode, GetProductCode()); // get the product code
	
	GetHardwareOption(); // get the product code
	
	giEepromPgrmFlg = 1; // set the redboot eeprom program flag to search for the redboot prompt
	giStatus = 0;		

	Try (GoToRedBootPrompt()); // connect to the dppm redboot prompt
	
	Try (EnableEEPromWrite()); // enable the eeprom for writing
	
	Try (OpenAardvarkPort(0, TRUE)); // Open the aardvark i2c device at port 0 and enable the i2c
	
/*##### build the 128 byte array for the 'common infos' page #####*/
	// fill array with 0xFF
	for (i=0; i<128; i++)
		bPageBytes[i] = 0xFF;

	if (giIsDppmGe) // check if GigE board
		sprintf(cPartNum,gsPartNum.dppm510);
	else if (giIsDppmSonet) // check if Sonet board
		sprintf(cPartNum,gsPartNum.dppm600);
	else if (giIsDppm10Ge) // check if Sonet board
		sprintf(cPartNum,gsPartNum.dppm800);
	
	// copy the common info to the data array
	CreateDateTimeStr(); // get the manufacturing date
	CopyBytes(bPageBytes, 0, WriteCommonPage( cProductCode, cPartNum, gcUutSerNum, cClei, gcMfgDate, gcTmmHWOpt, bPageBytes), 0, 128);
		
	// copy the magic number and crc value to the 1st 4 byte locations
	InitHeaderPage(bPageBytes); 
	// write the 'Common Infos' 128 byte data block to the eeprom at starting offset address of 0x0000
	if (!WriteBytesToDevice(GetPageOffset(CommonInfos), 0, bPageBytes))
		iStatus = 1;
	
	SetTestStatus(VAL_BLUE, "Programming MAC Addresses Page");
	
// ##### Second, erase then properly program the MAC Addresses page, #####
	for (i=0; i<128; i++) // fill array with 0xFF 
		bPageBytes[i] = 0xFF;

	// format and copy the mac addresses to the data block array
	CopyBytes(bPageBytes, 0, WriteMACPage(gsMacAddr, bPageBytes), 0, 56);
	
	InitHeaderPage(bPageBytes); // copy the magic number and crc value to the 1st 4 byte locations
	
	// write the mac address data block to the eeprom at starting offset of 0x0080
	if (!WriteBytesToDevice(GetPageOffset(MacAddresses), 0, bPageBytes))
		iStatus = 1;		
	
	SetTestStatus(VAL_BLUE, "Programming Specific Infos Page");

// ##### Third, build the 128 byte array for the 'specific infos' page, #####
	// initialize the data pointer
	iDataPointer = CRC_CALC_START_OFFSET;
	// fill array with 0xFF
	for (i=0; i<128; i++)
		bPageBytes[i] = 0xFF;
	
	// copy page byte count to 1st 2 offset byte locations	
	bPageBytes[iDataPointer] = 128 >> 8;  
	iDataPointer++;	
	bPageBytes[iDataPointer] = PAGE_BYTE_COUNT & 0xFF;
	iDataPointer++;	
	// copy schema version to next 2 byte locations	
	bPageBytes[iDataPointer] = SCHEMA_VERSION >> 8;  
	iDataPointer++;	
	bPageBytes[iDataPointer] = SCHEMA_VERSION & 0xFF;

	// store the formatted sdb part number	
	for (i=0; i<HEADER_PN_LENGTH; i++)
	{
		if (i < strlen(gsPartNum.sdbBd))
			bPageBytes[iDataPointer] = gsPartNum.sdbBd[i];
		else
			bPageBytes[iDataPointer] = ' '; // pad with spaces
		iDataPointer++;
	}
	// store the formatted sdb serial number
	for (i=0; i<HEADER_SN_LENGTH; i++)
	{
		if (i < strlen(gcSdbSerNum))
			bPageBytes[iDataPointer] = gcSdbSerNum[i];
		else
			bPageBytes[iDataPointer] = ' '; // pad with spaces
		iDataPointer++;
	}
	// store the manufacturing date in following format mm-dd-yyy 
	CreateDateTimeStr(); // get the manufacturing date
	for (i=0; i<HEADER_MFGDATE_LENGTH; i++)
	{
		if (i < strlen(gcMfgDate))
			bPageBytes[iDataPointer] = gcMfgDate[i];
		else
			bPageBytes[iDataPointer] = ' '; // pad with spaces
		iDataPointer++;
	}
	Scan(gcSdbHWOpt,"%s>%x[b2u]",&iHWOption);  // convert string value to integer value
	// store the sdb hardware option number	
	bPageBytes[iDataPointer] = iHWOption & 0xFF; // Minor
	iDataPointer++;	
	bPageBytes[iDataPointer] = iHWOption >> 8;   // Major
	
	// store the formatted nim part number	
	strcpy(cPartNum, CSBoardType());
	for (i=0; i<HEADER_PN_LENGTH; i++)
	{
		if (i < strlen(cPartNum))
			bPageBytes[iDataPointer] = cPartNum[i];
		else
			bPageBytes[iDataPointer] = ' '; // pad with spaces
		iDataPointer++;
	}
	// store the formatted nim serial number
	for (i=0; i<HEADER_SN_LENGTH; i++)
	{
		if (i < strlen(gcNimSerNum))
			bPageBytes[iDataPointer] = gcNimSerNum[i];
		else
			bPageBytes[iDataPointer] = ' '; // pad with spaces
		iDataPointer++;
	}
	// store the manufacturing date in following format mm-dd-yyy 
	CreateDateTimeStr(); // get the manufacturing date
	for (i=0; i<HEADER_MFGDATE_LENGTH; i++)
	{
		if (i < strlen(gcMfgDate))
			bPageBytes[iDataPointer] = gcMfgDate[i];
		else
			bPageBytes[iDataPointer] = ' '; // pad with spaces
		iDataPointer++;
	}
	Scan(gcNimHWOpt,"%s>%x[b2u]",&iHWOption);  // convert string value to integer value
	// store the sdb hardware option number	
	bPageBytes[iDataPointer] = iHWOption & 0xFF; // Minor
	iDataPointer++;	
	bPageBytes[iDataPointer] = iHWOption >> 8;   // Major
	
	InitHeaderPage(bPageBytes); // copy the magic number and crc value to the 1st 4 byte locations
	
	// write the specific infos data block to the eeprom at starting offset of 0x0100 
	if (!WriteBytesToDevice(GetPageOffset(SpecificInfos), 0, bPageBytes))
		iStatus = 1;		
	
	SetTestStatus(VAL_BLUE, "Programming Custom Infos Page");

/*##### build the 128 byte array for the 'custom infos' page #####*/
	// initialize the data pointer
	iDataPointer = CRC_CALC_START_OFFSET;
	// fill array with 0xFF for the 1st 8 bytes and with 0x20 for the remainder of the 128 byte block
	for (i=0; i<128; i++)
	{
		if (i < 8)
			bPageBytes[i] = 0xFF; // fill the first 8 bytes with 0xFF
		if (i >= 8)
			bPageBytes[i] = 0x20; // fill the remainder of the 128 bytes with spaces
	}
	// copy page byte count to 1st 2 offset byte locations	
	bPageBytes[iDataPointer] = 128 >> 8;  
	iDataPointer++;	
	bPageBytes[iDataPointer] = PAGE_BYTE_COUNT & 0xFF;
	iDataPointer++;	
	// copy schema version to next 2 byte locations	
	bPageBytes[iDataPointer] = SCHEMA_VERSION >> 8;  
	iDataPointer++;	
	bPageBytes[iDataPointer] = SCHEMA_VERSION & 0xFF;

	InitHeaderPage(bPageBytes); // copy the magic number and crc value to the 1st 4 byte locations
	
	// write the custom infos data block to the eeprom at starting offset of 0x0180 
	if (!WriteBytesToDevice(GetPageOffset(CustomInfos), 0, bPageBytes))
		iStatus = 1;		
exit:
	CloseAardvarkPort(hAardvarkHandle);
	
	CloseConnection(hHandle1RS232); // close the com port
	
	if (giStatus) // check if global error status error flag is set
		iStatus = 1;
	
	// display pass or fail and save the test result  
	if (iStatus)
	{
		strcpy(gsTestStatus.cPgmTmmEeprom,"Fail");
		SetTestStatus(VAL_RED, "Fail");
		SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, 4); 	
		strcat(gsComment,"Pgm Baseboard, ");
	}
	else
	{
		strcpy(gsTestStatus.cPgmTmmEeprom,"Pass");
		SetTestStatus(VAL_DK_GREEN, "Pass");
		SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, 1); 	
	}	
	return giStatus;
}

/*
 *	PgrmSdbBrdEeprom()
 *
 *  this function programs the uut dppm sdb eeprom .
 */
int PgrmSdbBrdEeprom ()
{
	int  i, fh, iBytesRead, iDataPointer;
	int  iStatus = 0;
	byte bPageBytes[128];
	byte bDataBytes[128];
	word wCRC = 0;
	word iHWOption = 0;
	char cNA[4] = {"N/A\0"};
	char testStr[75] = "\0";
	char cHWOption[10] = "\0";
	
	// get the list box line
	GetTreeItemFromLabel (gMainPanel, PANEL_TESTLIST, VAL_ALL, 0, VAL_FIRST, VAL_NEXT_PLUS_SELF, 0, 
		                  "Program the SDB EEPROM device!", &gCurListBoxLine);
	
	CheckListItem(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, 1); 
	sprintf(testStr,"\n########## SDB Board EEPROM Programming ##########\n"); // write 
	WriteFile (giLogFileHandle,testStr,StringLength(testStr));
	SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, -1); 	
	
	giEepromPgrmFlg = 1; // set the redboot eeprom program flag for searching the redboot prompt
	giStatus = 0;		
	
	GetHardwareOption(); // get the hardware option
	
	Try (OpenConnectionRS232(DPPM)); // open a com port connection to the DPPM
	
	Try (GoToRedBootPrompt()); // connect to the dppm redboot prompt

	Try (EnableEEPromWrite()); // enable the eeprom for writing
	
	SetTestStatus(VAL_BLUE, "Programming Common Infos Page");

/*##### build the 128 byte array for the 'common infos' page #####*/
	
	iDataPointer = CRC_CALC_START_OFFSET; // initialize the data pointer
	
	for (i=0; i<128; i++) // fill array with 0xFF
	{
		bPageBytes[i] = 0xFF;
		bDataBytes[i] = 0xFF;
	}
	// copy page byte count to 1st 2 offset byte locations	
	bPageBytes[iDataPointer] = 128 >> 8;  
	iDataPointer++;	
	bPageBytes[iDataPointer] = PAGE_BYTE_COUNT & 0xFF;
	iDataPointer++;	
	// copy schema version to next 2 byte locations	
	bPageBytes[iDataPointer] = SCHEMA_VERSION >> 8;  
	iDataPointer++;	
	bPageBytes[iDataPointer] = SCHEMA_VERSION & 0xFF;
	iDataPointer++;	
	// store the formatted product code, for the sdb it = "N/A"	
	for (i=0; i<HEADER_PCODE_LENGTH; i++)
	{
		if (i < strlen(cNA))
			bPageBytes[iDataPointer] = cNA[i];
		else
			bPageBytes[iDataPointer] = ' '; // pad with spaces
		iDataPointer++;
	}
	// store the formatted sdb part number
	for(i=0; i<HEADER_PN_LENGTH; i++)
	{
		if (i < strlen(gsPartNum.sdbBd))
			bPageBytes[iDataPointer] = gsPartNum.sdbBd[i];
		else
			bPageBytes[iDataPointer] = ' '; // pad with spaces
		iDataPointer++;
	}
	// store the formatted serial number
	for (i=0; i<HEADER_SN_LENGTH; i++)
	{
		if (i < strlen(gcSdbSerNum))
			bPageBytes[iDataPointer] = gcSdbSerNum[i];
		else
			bPageBytes[iDataPointer] = ' '; // pad with spaces
		iDataPointer++;
	}
	// store the CLEI code, in this case it is "N/A"
	for (i=0; i<HEADER_CLEI_LENGTH; i++)
	{
		if (i < strlen(cNA))
			bPageBytes[iDataPointer] = cNA[i];
		else
			bPageBytes[iDataPointer] = ' '; // pad with spaces
		iDataPointer++;
	}
	// store the manufacturing date in following format mm-dd-yyy 
	CreateDateTimeStr(); // get the manufacturing date
	for (i=0; i<HEADER_MFGDATE_LENGTH; i++)
	{
		if (i < strlen(gcMfgDate))
			bPageBytes[iDataPointer] = gcMfgDate[i];
		else
			bPageBytes[iDataPointer] = ' '; // pad with spaces
		iDataPointer++;
	}
	Scan(gcSdbHWOpt,"%s>%x[b2u]",&iHWOption);  // convert string value to integer value
	// store the hardware option number	
	bPageBytes[iDataPointer] = iHWOption & 0xFF; // Minor
	iDataPointer++;	
	bPageBytes[iDataPointer] = iHWOption >> 8;   // Major
	
	iDataPointer = 0; // reset the pointer to 0 to store the magic number and the crc value
	// store the magic number to 1st 2 byte locations	
	bPageBytes[iDataPointer] = MAGIC_NUMBER >> 8; 
	iDataPointer++;	
	bPageBytes[iDataPointer] = MAGIC_NUMBER & 0xFF;
	iDataPointer++;
	
	// copy 124 bytes of data starting from offset address 4 to the data byte buffer starting at location 0	
	CopyBytes(bDataBytes, 0, bPageBytes, CRC_CALC_START_OFFSET, 124);
	
	// calculate and store the crc value	
	wCRC = CalculateCRC(bDataBytes, 124);
	bPageBytes[iDataPointer] = wCRC >> 8;  
	iDataPointer++;	
	bPageBytes[iDataPointer] = wCRC & 0xFF;
	
	// write the 128 byte data block to the eeprom at starting offset address of 0x0000
	if (WriteBytesToEEPROM("nim", 0, bPageBytes, 128))
		iStatus = 1;
	
	SetTestStatus(VAL_BLUE, "Programming MAC Addresses Page");

/*##### build the 128 byte array for the 'mac addresses' page #####*/
	// initialize the data pointer
	iDataPointer = CRC_CALC_START_OFFSET;
	// fill array with 0xFF
	for (i=0; i<128; i++)
	{
		bPageBytes[i] = 0xFF;
		bDataBytes[i] = 0xFF;
	}
	// copy page byte count to 1st 2 offset byte locations	
	bPageBytes[iDataPointer] = PAGE_BYTE_COUNT >> 8;  
	iDataPointer++;	
	bPageBytes[iDataPointer] = PAGE_BYTE_COUNT & 0xFF;
	iDataPointer++;
	SCHEMA_VERSION = 0x0100;
	// copy schema version to next 2 byte locations	
	bPageBytes[iDataPointer] = SCHEMA_VERSION >> 8;  
	iDataPointer++;	
	bPageBytes[iDataPointer] = SCHEMA_VERSION & 0xFF;
	iDataPointer++;	
	
	iDataPointer = 0; // reset the pointer to 0 to store the magic number and the crc value
	// store the magic number to 1st 2 byte locations	
	bPageBytes[iDataPointer] = MAGIC_NUMBER >> 8; 
	iDataPointer++;	
	bPageBytes[iDataPointer] = MAGIC_NUMBER & 0xFF;
	iDataPointer++;
	
	// copy 124 bytes of data starting from offset address 4 to the data byte buffer starting at location 0	
	CopyBytes(bDataBytes, 0, bPageBytes, CRC_CALC_START_OFFSET, 124);
	
	// calculate and store the crc value	
	wCRC = CalculateCRC(bDataBytes, 124);
	bPageBytes[iDataPointer] = wCRC >> 8;  
	iDataPointer++;	
	bPageBytes[iDataPointer] = wCRC & 0xFF;
	
	// write the 128 byte data block to the eeprom at starting offset address of 0x0080
	if (WriteBytesToEEPROM("sdb", GetPageOffset(MacAddresses), bPageBytes, 128))
		iStatus = 1;
	
	SetTestStatus(VAL_BLUE, "Programming Specific Infos Page");

/*##### build the 128 byte array for the 'specific infos' page #####*/
	// initialize the data pointer
	iDataPointer = CRC_CALC_START_OFFSET;
	// fill array with 0xFF
	for (i=0; i<128; i++)
	{
		bPageBytes[i] = 0xFF;
		bDataBytes[i] = 0xFF;
	}
	// copy page byte count to 1st 2 offset byte locations	
	bPageBytes[iDataPointer] = PAGE_BYTE_COUNT >> 8;  
	iDataPointer++;	
	bPageBytes[iDataPointer] = PAGE_BYTE_COUNT & 0xFF;
	iDataPointer++;
	SCHEMA_VERSION = 0x0100;
	// copy schema version to next 2 byte locations	
	bPageBytes[iDataPointer] = SCHEMA_VERSION >> 8;  
	iDataPointer++;	
	bPageBytes[iDataPointer] = SCHEMA_VERSION & 0xFF;
	iDataPointer++;	
	
	iDataPointer = 0; // reset the pointer to 0 to store the magic number and the crc value
	// store the magic number to 1st 2 byte locations	
	bPageBytes[iDataPointer] = MAGIC_NUMBER >> 8; 
	iDataPointer++;	
	bPageBytes[iDataPointer] = MAGIC_NUMBER & 0xFF;
	iDataPointer++;
	
	// copy 124 bytes of data starting from offset address 4 to the data byte buffer starting at location 0	
	CopyBytes(bDataBytes, 0, bPageBytes, CRC_CALC_START_OFFSET, 124);
	
	// calculate and store the crc value	
	wCRC = CalculateCRC(bDataBytes, 124);
	bPageBytes[iDataPointer] = wCRC >> 8;  
	iDataPointer++;	
	bPageBytes[iDataPointer] = wCRC & 0xFF;
	
	// write the 128 byte data block to the eeprom at starting offset address of 0x0100
	if (WriteBytesToEEPROM("sdb", GetPageOffset(SpecificInfos), bPageBytes, 128))
		iStatus = 1;
	
	SetTestStatus(VAL_BLUE, "Programming Custom Infos Page");

/*##### build the 128 byte array for the 'custom infos' page #####*/
	// initialize the data pointer
	iDataPointer = CRC_CALC_START_OFFSET;
	// fill array with spaces (0x20)
	for (i=0; i<128; i++)
	{
		bPageBytes[i] = 0xFF;
		bDataBytes[i] = 0xFF;
	}
	// copy page byte count to 1st 2 offset byte locations	
	bPageBytes[iDataPointer] = PAGE_BYTE_COUNT >> 8;  
	iDataPointer++;	
	bPageBytes[iDataPointer] = PAGE_BYTE_COUNT & 0xFF;
	iDataPointer++;
	SCHEMA_VERSION = 0x0100;
	// copy schema version to next 2 byte locations	
	bPageBytes[iDataPointer] = SCHEMA_VERSION >> 8;  
	iDataPointer++;	
	bPageBytes[iDataPointer] = SCHEMA_VERSION & 0xFF;
	iDataPointer++;	
	
	iDataPointer = 0; // reset the pointer to 0 to store the magic number and the crc value
	// store the magic number to 1st 2 byte locations	
	bPageBytes[iDataPointer] = MAGIC_NUMBER >> 8; 
	iDataPointer++;	
	bPageBytes[iDataPointer] = MAGIC_NUMBER & 0xFF;
	iDataPointer++;
	
	// copy 124 bytes of data starting from offset address 4 to the data byte buffer starting at location 0	
	CopyBytes(bDataBytes, 0, bPageBytes, CRC_CALC_START_OFFSET, 124);
	
	// calculate and store the crc value	
	wCRC = CalculateCRC(bDataBytes, 124);
	bPageBytes[iDataPointer] = wCRC >> 8;  
	iDataPointer++;	
	bPageBytes[iDataPointer] = wCRC & 0xFF;
	
	// write the 128 byte data block to the eeprom at starting offset address of 0x0180
	if (WriteBytesToEEPROM("sdb", GetPageOffset(SpecificInfos), bPageBytes, 128))
		iStatus = 1;

exit:
//	LogOutFromRedBoot(); // logout from redboot and go to the dppm linux login prompt
	
	CloseConnection(hHandle1RS232); // disconnect the rs232 port
	
	if (giStatus) // check if global error status error flag is set
		iStatus = 1;
	
	// display pass or fail and save the test result  
	if (iStatus)
	{
		strcpy(gsTestStatus.cPgmSdbEeprom,"Fail");
		SetTestStatus(VAL_RED, "Fail");
		SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, 4); 	
		strcat(gsComment,"Pgm SDB, ");
	}
	else
	{
		strcpy(gsTestStatus.cPgmSdbEeprom,"Pass");
		SetTestStatus(VAL_DK_GREEN, "Pass");
		SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, 1); 	
	}	
	return 	iStatus;
}

/*
 *	PgrmNimBrdEeprom()
 *
 *  this function programs the uut dppm baseboard eeprom .
 */
int PgrmNimBrdEeprom ()
{
	int  i, fh, iBytesRead, iDataPointer;
	int  iStatus = 0;
	word iHWOption = 0;
	byte bPageBytes[128];
	byte bDataBytes[128];
	word wCRC = 0;
	char cNA[4] = {"N/A\0"};
	char testStr[75] = "\0";
	char cHWOption[10] = "\0";
	char cPartNum[20] = "\0";
	
	// get the list box line
	GetTreeItemFromLabel (gMainPanel, PANEL_TESTLIST, VAL_ALL, 0, VAL_FIRST, VAL_NEXT_PLUS_SELF, 0, 
		                  "Program the NIM EEPROM device!", &gCurListBoxLine);
	
	CheckListItem(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, 1); 
	sprintf(testStr,"\n########## NIM Board EEPROM Programming ##########\n"); // write 
	WriteFile (giLogFileHandle,testStr,StringLength(testStr));
	SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, -1); 	
	
	giEepromPgrmFlg = 1; // set the redboot eeprom program flag for searching the redboot prompt
	giStatus = 0;		
	
	GetHardwareOption(); // get the hardware option
	
	Try (OpenConnectionRS232(DPPM)); // open a com port connection to the DPPM
	
	Try (GoToRedBootPrompt()); // connect to the dppm redboot prompt

	Try (EnableEEPromWrite()); // enable the eeprom for writing
	
	SetTestStatus(VAL_BLUE, "Programming Common Infos Page");
	
/*##### build the 128 byte array for the 'common infos' page #####*/
	// initialize the data pointer
	iDataPointer = CRC_CALC_START_OFFSET;
	// fill array with 0xFF
	for (i=0; i<128; i++)
	{
		bPageBytes[i] = 0xFF;
		bDataBytes[i] = 0xFF;
	}
	// copy page byte count to 1st 2 offset byte locations	
	bPageBytes[iDataPointer] = 128 >> 8;  
	iDataPointer++;	
	bPageBytes[iDataPointer] = PAGE_BYTE_COUNT & 0xFF;
	iDataPointer++;	
	// copy schema version to next 2 byte locations	
	bPageBytes[iDataPointer] = SCHEMA_VERSION >> 8;  
	iDataPointer++;	
	bPageBytes[iDataPointer] = SCHEMA_VERSION & 0xFF;
	iDataPointer++;	
	// store the formatted product code, for the NIM it = "N/A"	
	for (i=0; i<HEADER_PCODE_LENGTH; i++)
	{
		if (i < strlen(cNA))
			bPageBytes[iDataPointer] = cNA[i];
		else
			bPageBytes[iDataPointer] = ' '; // pad with spaces
		iDataPointer++;
	}
	// store the formatted part number
	strcpy(cPartNum, CSBoardType());
	for(i=0; i<HEADER_PN_LENGTH; i++)
	{
		if (i < strlen(cPartNum))
			bPageBytes[iDataPointer] = cPartNum[i];
		else
			bPageBytes[iDataPointer] = ' '; // pad with spaces
		iDataPointer++;
	}
	// store the formatted serial number
	for (i=0; i<HEADER_SN_LENGTH; i++)
	{
		if (i < strlen(gcNimSerNum))
			bPageBytes[iDataPointer] = gcNimSerNum[i];
		else
			bPageBytes[iDataPointer] = ' '; // pad with spaces
		iDataPointer++;
	}
	// store the CLEI code, in this case it is "N/A"
	for (i=0; i<HEADER_CLEI_LENGTH; i++)
	{
		if (i < strlen(cNA))
			bPageBytes[iDataPointer] = cNA[i];
		else
			bPageBytes[iDataPointer] = ' '; // pad with spaces
		iDataPointer++;
	}
	// store the manufacturing date in following format mm-dd-yyy 
	CreateDateTimeStr(); // get the manufacturing date
	for (i=0; i<HEADER_MFGDATE_LENGTH; i++)
	{
		if (i < strlen(gcMfgDate))
			bPageBytes[iDataPointer] = gcMfgDate[i];
		else
			bPageBytes[iDataPointer] = ' '; // pad with spaces
		iDataPointer++;
	}
	Scan(gcNimHWOpt,"%s>%x[b2u]",&iHWOption);  // convert string value to integer value
	// store the hardware option number	
	bPageBytes[iDataPointer] = iHWOption & 0xFF; // Minor
	iDataPointer++;	
	bPageBytes[iDataPointer] = iHWOption >> 8;   // Major
	
	iDataPointer = 0; // reset the pointer to 0 to store the magic number and the crc value
	// store the magic number to 1st 2 byte locations	
	bPageBytes[iDataPointer] = MAGIC_NUMBER >> 8; 
	iDataPointer++;	
	bPageBytes[iDataPointer] = MAGIC_NUMBER & 0xFF;
	iDataPointer++;
	
	// copy 124 bytes of data starting from offset address 4 to the data byte buffer starting at location 0	
	CopyBytes(bDataBytes, 0, bPageBytes, CRC_CALC_START_OFFSET, 124);
	
	// calculate and store the crc value	
	wCRC = CalculateCRC(bDataBytes, 124);
	bPageBytes[iDataPointer] = wCRC >> 8;  
	iDataPointer++;	
	bPageBytes[iDataPointer] = wCRC & 0xFF;
	
	// write the 128 byte data block to the eeprom at starting offset address of 0x0000
	if (WriteBytesToEEPROM("nim", 0, bPageBytes, 128))
		iStatus = 1;
	
	SetTestStatus(VAL_BLUE, "Programming MAC Addresses Page");

/*##### build the 128 byte array for the 'mac addresses' page #####*/
	// initialize the data pointer
	iDataPointer = CRC_CALC_START_OFFSET;
	// fill array with 0xFF
	for (i=0; i<128; i++)
	{
		bPageBytes[i] = 0xFF;
		bDataBytes[i] = 0xFF;
	}
	// copy page byte count to 1st 2 offset byte locations	
	bPageBytes[iDataPointer] = PAGE_BYTE_COUNT >> 8;  
	iDataPointer++;	
	bPageBytes[iDataPointer] = PAGE_BYTE_COUNT & 0xFF;
	iDataPointer++;
	SCHEMA_VERSION = 0x0100;
	// copy schema version to next 2 byte locations	
	bPageBytes[iDataPointer] = SCHEMA_VERSION >> 8;  
	iDataPointer++;	
	bPageBytes[iDataPointer] = SCHEMA_VERSION & 0xFF;
	iDataPointer++;	
	
	iDataPointer = 0; // reset the pointer to 0 to store the magic number and the crc value
	// store the magic number to 1st 2 byte locations	
	bPageBytes[iDataPointer] = MAGIC_NUMBER >> 8; 
	iDataPointer++;	
	bPageBytes[iDataPointer] = MAGIC_NUMBER & 0xFF;
	iDataPointer++;
	
	// copy 124 bytes of data starting from offset address 4 to the data byte buffer starting at location 0	
	CopyBytes(bDataBytes, 0, bPageBytes, CRC_CALC_START_OFFSET, 124);
	
	// calculate and store the crc value	
	wCRC = CalculateCRC(bDataBytes, 124);
	bPageBytes[iDataPointer] = wCRC >> 8;  
	iDataPointer++;	
	bPageBytes[iDataPointer] = wCRC & 0xFF;
	
	// write the 128 byte data block to the eeprom at starting offset address of 0x0080
	if (WriteBytesToEEPROM("nim", GetPageOffset(MacAddresses), bPageBytes, 128))
		iStatus = 1;
	
	SetTestStatus(VAL_BLUE, "Programming Specfic Infos Page");

/*##### build the 128 byte array for the 'specific infos' page #####*/
	// initialize the data pointer
	iDataPointer = CRC_CALC_START_OFFSET;
	// fill array with 0xFF
	for (i=0; i<128; i++)
	{
		bPageBytes[i] = 0xFF;
		bDataBytes[i] = 0xFF;
	}
	// copy page byte count to 1st 2 offset byte locations	
	bPageBytes[iDataPointer] = PAGE_BYTE_COUNT >> 8;  
	iDataPointer++;	
	bPageBytes[iDataPointer] = PAGE_BYTE_COUNT & 0xFF;
	iDataPointer++;
	SCHEMA_VERSION = 0x0100;
	// copy schema version to next 2 byte locations	
	bPageBytes[iDataPointer] = SCHEMA_VERSION >> 8;  
	iDataPointer++;	
	bPageBytes[iDataPointer] = SCHEMA_VERSION & 0xFF;
	iDataPointer++;	
	
	iDataPointer = 0; // reset the pointer to 0 to store the magic number and the crc value
	// store the magic number to 1st 2 byte locations	
	bPageBytes[iDataPointer] = MAGIC_NUMBER >> 8; 
	iDataPointer++;	
	bPageBytes[iDataPointer] = MAGIC_NUMBER & 0xFF;
	iDataPointer++;
	
	// copy 124 bytes of data starting from offset address 4 to the data byte buffer starting at location 0	
	CopyBytes(bDataBytes, 0, bPageBytes, CRC_CALC_START_OFFSET, 124);
	
	// calculate and store the crc value	
	wCRC = CalculateCRC(bDataBytes, 124);
	bPageBytes[iDataPointer] = wCRC >> 8;  
	iDataPointer++;	
	bPageBytes[iDataPointer] = wCRC & 0xFF;
	
	// write the 128 byte data block to the eeprom at starting offset address of 0x0100
	if (WriteBytesToEEPROM("nim", GetPageOffset(SpecificInfos), bPageBytes, 128))
		iStatus = 1;
	
	SetTestStatus(VAL_BLUE, "Programming Custom Infos Page");

/*##### build the 128 byte array for the 'custom infos' page #####*/
	// initialize the data pointer
	iDataPointer = CRC_CALC_START_OFFSET;
	// fill array with spaces (0x20)
	for (i=0; i<128; i++)
	{
		bPageBytes[i] = 0xFF;
		bDataBytes[i] = 0xFF;
	}
	// copy page byte count to 1st 2 offset byte locations	
	bPageBytes[iDataPointer] = PAGE_BYTE_COUNT >> 8;  
	iDataPointer++;	
	bPageBytes[iDataPointer] = PAGE_BYTE_COUNT & 0xFF;
	iDataPointer++;
	SCHEMA_VERSION = 0x0100;
	// copy schema version to next 2 byte locations	
	bPageBytes[iDataPointer] = SCHEMA_VERSION >> 8;  
	iDataPointer++;	
	bPageBytes[iDataPointer] = SCHEMA_VERSION & 0xFF;
	iDataPointer++;	
	
	iDataPointer = 0; // reset the pointer to 0 to store the magic number and the crc value
	// store the magic number to 1st 2 byte locations	
	bPageBytes[iDataPointer] = MAGIC_NUMBER >> 8; 
	iDataPointer++;	
	bPageBytes[iDataPointer] = MAGIC_NUMBER & 0xFF;
	iDataPointer++;
	
	// copy 124 bytes of data starting from offset address 4 to the data byte buffer starting at location 0	
	CopyBytes(bDataBytes, 0, bPageBytes, CRC_CALC_START_OFFSET, 124);
	
	// calculate and store the crc value	
	wCRC = CalculateCRC(bDataBytes, 124);
	bPageBytes[iDataPointer] = wCRC >> 8;  
	iDataPointer++;	
	bPageBytes[iDataPointer] = wCRC & 0xFF;
	
	// write the 128 byte data block to the eeprom at starting offset address of 0x0180
	if (WriteBytesToEEPROM("nim", GetPageOffset(SpecificInfos), bPageBytes, 128))
		iStatus = 1;

exit:
//	LogOutFromRedBoot(); // logout from redboot and go to the dppm linux login prompt
	
	CloseConnection(hHandle1RS232); // disconnect the rs232 port
	
	if (iStatus) // check if global error status error flag is set
		iStatus = 1;
	
	// display pass or fail and save the test result  
	if (iStatus)
	{
		strcpy(gsTestStatus.cPgmNimEeprom,"Fail");
		SetTestStatus(VAL_RED, "Fail");
		SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, 4); 	
		strcat(gsComment,"Pgm NIM, ");
	}
	else
	{
		strcpy(gsTestStatus.cPgmNimEeprom,"Pass");
		SetTestStatus(VAL_DK_GREEN, "Pass");
		SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, 1); 	
	}	
	return iStatus;
}

/*
 *	PgrmMacPhy()
 *
 *  this function programs the mac phy device .
 */
int PgrmMacPhy ()
{
	int  i, j, x;
	int  iStatus = 0;
	byte bPageBytes[128];
	byte bDataBytes[128];
	byte lsb,mid,msb;
	char testStr[75] = "\0";
	char cCmdStr[75] = "\0";
	char cMacStr[20] = "\0";
	char cHWOption[10] = "\0";
	char cMacPhy[20] = "000BA9001F2A\0";
	unsigned long lMacPhy;
	char cKeyWordPattern[7] = {"0x0022\0"};
	char cDeviceType[3];
	int bDeviceType;
	
	// get the list box line
	GetTreeItemFromLabel (gMainPanel, PANEL_TESTLIST, VAL_ALL, 0, VAL_FIRST, VAL_NEXT_PLUS_SELF, 0, 
		                  "Program the MAC Phy device!", &gCurListBoxLine);
	
	CheckListItem(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, 1); 
	SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, -1); 	
	giStatus = 0; // clear the error status flag
	memset(cMacPhy,0,20); // initialize mac phy address buffers
	memset(cMacStr,0,20);

	strcpy(cMacPhy,"000BA9001F2A");
	
	Try (OpenConnectionRS232(DPPM)); // open a virtual com port connection to the DPPM

	// see if at the ixp prompt before rebooting
	if (GoToIxpPrompt(hHandle1RS232) != 0) 
	{
		SetTestStatus(VAL_BLUE, "Rebooting the UUT");
		
		ViOpenXantrex(); // open a com session to both power supplies
	
		PowerCycle(); // do a power cycle
	
		ViCloseXantrex(); // close the com session to both power supplies

		Try (WriteReadUntil(hHandle1RS232, gcReadBuffer,"",gsUNIxpPrompt, 420)); // wait for the boot up prompt

		Try (GoToIxpPrompt(hHandle1RS232)); // go the the linux prompt, exit if not found 
	}
	SetTestStatus(VAL_BLUE, "Programming the MAC Phy");
	
	// get the device type
	sprintf(cCmdStr,"/cs/bin/ethtool-ixp -e eth1 offset 34 length 1\r");
	WriteReadUntil(hHandle1RS232, gcReadBuffer,cCmdStr, gsIxpLogPrompt, 5);
	x = (FindPattern (gcReadBuffer, 0, -1, cKeyWordPattern, 0, 1) + 8); // get the position of the 1st value
	
	cDeviceType[0] = gcReadBuffer[x++]; // parse out the device type
	cDeviceType[1] = gcReadBuffer[x];
	cDeviceType[2] = '\0';
	bDeviceType = atoi(cDeviceType);
	
	if (bDeviceType == 12) // load the mac phy shell script based on the device type
		sprintf(cCmdStr,"/cs/bin/ProgMacPhy-DPPM1012.sh");
	else if (bDeviceType == 10)
		sprintf(cCmdStr,"/cs/bin/ProgMacPhy-DPPM1010.sh");
	else
	{
		iStatus = 1; // exit if device type unknown or not captured
		goto exit;
	}
	// construct the mac phy programming command string
	Scan(cMacPhy,"%s>%x",&lMacPhy); // convert the mac address to a long value
	msb = (lMacPhy >> 16) & 0xFF;   // get the 3 last bytes from the mac address
	mid = (lMacPhy >> 8) & 0xFF;    // and convert them to individual values
	lsb = lMacPhy & 0xFF;			// lsb, mid and msb bytes
	Fmt(cMacStr," %i[b1u] %i[b1u] %i[b1u]\r",msb,mid,lsb); // convert byte values back to a mac value string
	strcat(cCmdStr, cMacStr); // and concatenate the mac value string onto the shell command string
	
	// program the mac phy with the constructed shell command string
	Try (WriteReadUntil(hHandle1RS232, gcReadBuffer,cCmdStr, gsIxpLogPrompt, 60));
	
	// Read back the MAC just burned: The MAC is saved at the first 6 bytes of the EEPROM
	sprintf(cCmdStr,"/cs/bin/ethtool-ixp -e eth1 offset 0 length 6\r");
	WriteReadUntil(hHandle1RS232, gcReadBuffer,cCmdStr, gsIxpLogPrompt, 5);
	// get the position of the 1st mac value
	x = (FindPattern (gcReadBuffer, 0, -1, "0x0000", 0, 1) + 8);
	// parse out the mac address from the return string
	for (i=0, j=0; i<17; i++)
	{
		if (gcReadBuffer[x] != ' ') // skip the whitespace characters
		{
			cMacStr[j] = gcReadBuffer[x]; // save the mac address characters
			cMacStr[j] = toupper(cMacStr[j]);
			j++;
		}
		x++;
	}
	if (strcmp(cMacStr, cMacPhy) != 0) //  verify the programmed mac value
		iStatus = 1; // set the error status if the values don't match
exit:
	LogOutFromIxp(hHandle1RS232); // logout from the ixp
	
	CloseConnection(hHandle1RS232); // close the com port
	
	if (giStatus) // check if global error status error flag is set
		iStatus = 1;
	
	// display pass or fail and save the test result  
	if (iStatus)
	{
		strcpy(gsTestStatus.cPgrmMacPhy,"Fail");
		SetTestStatus(VAL_RED, "Fail");
		SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, 4); 	
		strcat(gsComment,"Pgm MAC Phy, ");
	}
	else
	{
		strcpy(gsTestStatus.cPgrmMacPhy,"Pass");
		SetTestStatus(VAL_DK_GREEN, "Pass");
		SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, 1); 	
	}	
	return iStatus;
}

/*
 *	ChkStatusLEDs()()
 *
 *  this function runs the dppm status led verification test
 */
int ChkStatusLEDs()
{
	int i, statStr = 3, iStatus = 0;
	int iRegState1, iRegState2;
	char regState[10] = "\0";
	char cmdStr[75] = "\0";
	char testStr[75] = "\0";
	char* regAddr[] = {"0xc6000040","0xc6428008"};
	
	// get the list box line for this test
	GetTreeItemFromLabel (gMainPanel, PANEL_TESTLIST, VAL_ALL, 0, VAL_FIRST, VAL_NEXT_PLUS_SELF, 0, 
		                  "Status LEDs verification test.", &gCurListBoxLine);
	
	CheckListItem(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, 1); 
	sprintf(cmdStr,"\n########## Status LEDs verification test ##########\n"); // write 
	WriteFile (giLogFileHandle,cmdStr,StringLength(cmdStr));
	SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, -1); 	
	SetTestStatus(VAL_BLUE, "Verifying Status LEDs");
	
	giStatus = 0; // clear the error status flag
	
	giStatusLedPanel = LoadPanel (0, "FatTestGui.uir", STATUSLED); // load the status led panel
	
	Try (OpenConnectionRS232(DPPM)); // open a com port connection to the DPPM
	
	Try (GoToIxpPrompt(hHandle1RS232));	 // connect to the dppm ixp
	
	strcpy(regState,GetLedRegState("0xc6000040")); // save the initial register states
	iRegState1 = atoi(regState);
	strcpy(regState,GetLedRegState("0xc6428008"));
	iRegState2 = atoi(regState);
	
	HidePanel(gMainPanel); // hide the main panel

	for (i=0; i<2; i++) // loop through status led on/off panels
	{
		giButtonPressFlg = 0;
	
		sprintf(cmdStr,"/nfs/ixp2800/bin/poke %s %i\r",regAddr[0],statStr);
	
		// turn on the OK and the Fail leds
		if (WriteReadUntil(hHandle1RS232, gcReadBuffer, cmdStr, gsIxpLogPrompt, 5) != 0)
			iStatus = 1;
		
		sprintf(cmdStr,"/nfs/ixp2800/bin/poke %s %i\r",regAddr[1],statStr);

		// turn on the Standby and the Online LEDs
		if (WriteReadUntil(hHandle1RS232, gcReadBuffer, cmdStr, gsIxpLogPrompt, 5) != 0)
			iStatus = 1;
		
		InstallPopup(giStatusLedPanel); // open the status led panel
		
		while (!giButtonPressFlg) // wait here until either the Pass or Fail button is pressed
			ProcessSystemEvents();

		RemovePopup(giStatusLedPanel);  // close the status led panel
		
		// display the status leds off and reference picture
		InsertTextBoxLine (giStatusLedPanel, STATUSLED_TEXTBOX, -1,"Press the 'Pass' button if all Status LEDS are OFF as shown in.");
		InsertTextBoxLine (giStatusLedPanel, STATUSLED_TEXTBOX, -1,"the picture below.\n");
		InsertTextBoxLine (giStatusLedPanel, STATUSLED_TEXTBOX, -1,"\n");
		InsertTextBoxLine (giStatusLedPanel, STATUSLED_TEXTBOX, -1,"Press the 'Fail' button if any of the LEDs are not OFF or if any of");
		InsertTextBoxLine (giStatusLedPanel, STATUSLED_TEXTBOX, -1,"the colors don't match.");
		DisplayImageFile  (giStatusLedPanel, STATUSLED_PICTURE,"C:\\Cloudshield\\Pictures\\FacePlate_LEDs_Off.jpg");
		
		Delay(1.0); // delay before opening next popup panel
		
		if (giStatusLedFlg == 0) giStatus = 1; // see if the pass button was pressed, if not set the error flag
		
		statStr = 0; // set the turn all status off register value
		giStatusLedFlg = 0; // clear the pass/fail status flag
	
	}	
	DisplayPanel(gMainPanel); // display the main panel
	// set the OK and Fail LEDs to their initial states
	sprintf(cmdStr,"/nfs/ixp2800/bin/poke 0xc6000040 %i\r",iRegState1);
	if (WriteReadUntil(hHandle1RS232, gcReadBuffer, cmdStr, gsIxpLogPrompt, 5) != 0)
		iStatus = 1;
	// set the Standby and Online LEDs  to their initial states
	sprintf(cmdStr,"/nfs/ixp2800/bin/poke 0xc6428008 %i\r",iRegState2);
	if (WriteReadUntil(hHandle1RS232, gcReadBuffer, cmdStr, gsIxpLogPrompt, 5) != 0)
		iStatus = 1;
exit:
	Delay(1.0);
	
	LogOutFromIxp(hHandle1RS232); // logout from the ixp
	
	CloseConnection(hHandle1RS232); // close the com port
	
	if (giStatus) // check if global error status error flag is set
		iStatus = 1;
	
	// display pass or fail and save the test result  
	if (iStatus)
	{
		strcpy(gsTestStatus.cChkStatLEDs,"Fail");
		SetTestStatus(VAL_RED, "Fail");
		SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, 4); 	
		strcat(gsComment,"Status LEDs, ");
	}
	else
	{   
		strcpy(gsTestStatus.cChkStatLEDs,"Pass");
		SetTestStatus(VAL_DK_GREEN, "Pass");
		SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, 1); 	
	}	
	return iStatus;
}

/*
 *	ChkNimLEDs()()
 *
 *  this function runs the dppm nim led verification test
 */
int ChkNimLEDs()
{
	unsigned int i, valueToSet, iRegState=0, iStatus = 0;
	char cmdStr[75] = "\0";
	char testStr[75] = "\0";
	char regVal[10] = "\0";
	char initRegState[10];
	char* regAddr[] = {"0xC7000020","0xC7600028","0xC7000004"};

	if (giIsDppmGe) sprintf(testStr,"GigE NIM LEDs verification test.");
	else if (giIsDppmSonet) sprintf(testStr,"Sonet NIM LEDs verification test.");
	else if (giIsDppm10Ge) sprintf(testStr,"10Ge NIM LEDs verification test.");
		
	// get the list box line for this test
	GetTreeItemFromLabel (gMainPanel, PANEL_TESTLIST,VAL_ALL,0,VAL_FIRST,VAL_NEXT_PLUS_SELF,0,testStr,&gCurListBoxLine);
	
	CheckListItem(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, 1); 
	sprintf(cmdStr,"\n########## NIM LEDs verification test ##########\n"); // write 
	WriteFile (giLogFileHandle,cmdStr,StringLength(cmdStr));
	SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, -1); 	
	SetTestStatus(VAL_BLUE, "Verifying NIM LEDs");

	giStatus = 0; // clear the error status flag		

	Try (OpenConnectionRS232(DPPM)); // open a com port connection to the DPPM
	
	Try (GoToIxpPrompt(hHandle1RS232));	 // connect to the dppm ixp
	
	giStatus = 0; // clear the error status flag
	giNimLedPanel = LoadPanel (0, "FatTestGui.uir", NIMLED); // load the status led panel
	
	HidePanel(gMainPanel); // hide the main panel
	
	if (giIsDppmGe) // check to see if the uut is a dppm-510 board
	{
		for (i=1; i<=2; i++) // loop through nim led on/off panels
		{
			giButtonPressFlg = 0; // clear the button press flag
	
			sprintf(cmdStr,"/nfs/ixp2800/bin/poke %s %i\r",regAddr[0],i);
	
			// first turn on all nim leds then on the 2nd pass, turn them all off
			if (WriteReadUntil(hHandle1RS232, gcReadBuffer, cmdStr, gsIxpLogPrompt, 5) != 0)
				giStatus = 1;
		
			InstallPopup(giNimLedPanel); // open the status led panel
		
			while (!giButtonPressFlg) // wait here until either the Pass or Fail button is pressed
				ProcessSystemEvents();

			RemovePopup(giNimLedPanel);  // close the status led panel
		
			// display the status leds off and reference picture
			ResetTextBox      (giNimLedPanel, NIMLED_TEXTBOX, ""); // clear the text box
			SetPanelAttribute (giNimLedPanel, ATTR_TITLE,"DPPM-510 NIM LEDs Verification");
			InsertTextBoxLine (giNimLedPanel, NIMLED_TEXTBOX, -1,"Press the 'Pass' button if all NIM LEDS are OFF as shown in.");
			InsertTextBoxLine (giNimLedPanel, NIMLED_TEXTBOX, -1,"the picture below.\n");
			InsertTextBoxLine (giNimLedPanel, NIMLED_TEXTBOX, -1,"\n");
			InsertTextBoxLine (giNimLedPanel, NIMLED_TEXTBOX, -1,"Press the 'Fail' button if any of the LEDs are not OFF or are still ON.");
			DisplayImageFile  (giNimLedPanel, NIMLED_PICTURE,"C:\\Cloudshield\\Pictures\\NIM_LEDs_Off.jpg");
		
			Delay(1.0); // delay before opening next popup panel
		
			if (giStatusLedFlg == 0) iStatus = 1; // see if the pass button was pressed, if not set the error flag
		
			giStatusLedFlg = 0; // clear the pass/fail status flag
		}
		sprintf(cmdStr,"/nfs/ixp2800/bin/poke %s %i\r",regAddr[0], 0);

		// set value for all leds unchanged
		if (WriteReadUntil(hHandle1RS232, gcReadBuffer, cmdStr, gsIxpLogPrompt, 5) != 0)
			iStatus = 1;
	}
	else if (giIsDppmSonet) // is the uut a SONET board
	{
		strcpy(regVal,"FFFF00"); // register value to turn on all sonet nim leds
		
		// display the status leds off and reference picture
		ResetTextBox      (giNimLedPanel, NIMLED_TEXTBOX, ""); // clear the text box
		SetPanelAttribute (giNimLedPanel, ATTR_TITLE,"DPPM-600 NIM LEDs Verification");
		InsertTextBoxLine (giNimLedPanel, NIMLED_TEXTBOX, -1,"Press 'Pass' if all SONET NIM Faceplate LEDS are ON.");
		InsertTextBoxLine (giNimLedPanel, NIMLED_TEXTBOX, -1,"There should be two(2) LEDs per port on all 8 Fiber ports");
		InsertTextBoxLine (giNimLedPanel, NIMLED_TEXTBOX, -1,"(8 RED and 8 GREEN) for a  total of 16 LEDs turned On.\n");
		InsertTextBoxLine (giNimLedPanel, NIMLED_TEXTBOX, -1,"Press 'No' if at least one LED is OFF or looks different");
		InsertTextBoxLine (giNimLedPanel, NIMLED_TEXTBOX, -1,"than the others.");
		DisplayImageFile  (giNimLedPanel, NIMLED_PICTURE,"C:\\Cloudshield\\Pictures\\NIM_10Ge_LEDs_On.jpg");
		
		// load the SONET NIM LEDs reference picture
		DisplayImageFile  (giNimLedPanel, STATUSLED_PICTURE,"C:\\Cloudshield\\Pictures\\Sonet_NIM_LEDs_On.jpg");
		
		for (i=1; i<=2; i++) // loop through nim led on/off panels
		{
			giButtonPressFlg = 0; // clear the button press flag
	
			sprintf(cmdStr,"/nfs/ixp2800/bin/poke %s %s\r",regAddr[1],regVal);
	
			// first turn on all nim leds then on the 2nd pass, turn them all off
			if (WriteReadUntil(hHandle1RS232, gcReadBuffer, cmdStr, gsIxpLogPrompt, 5) != 0)
				giStatus = 1;
		
			InstallPopup(giNimLedPanel); // open the status led panel
		
			while (!giButtonPressFlg) // wait here until either the Pass or Fail button is pressed
				ProcessSystemEvents();

			RemovePopup(giNimLedPanel);  // close the status led panel
		
			// display the status leds off and reference picture
			ResetTextBox      (giNimLedPanel, NIMLED_TEXTBOX, ""); // clear the text box
			InsertTextBoxLine (giNimLedPanel, NIMLED_TEXTBOX, -1,"Press the 'Pass' button if all NIM LEDS are Off as shown");
			InsertTextBoxLine (giNimLedPanel, NIMLED_TEXTBOX, -1,"in the picture below.\n");
			InsertTextBoxLine (giNimLedPanel, NIMLED_TEXTBOX, -1,"All LEDs should be turned Off.\n");
			InsertTextBoxLine (giNimLedPanel, NIMLED_TEXTBOX, -1,"Press the 'Fail' button if any of the LEDs are still On");
			DisplayImageFile  (giNimLedPanel, NIMLED_PICTURE,"C:\\Cloudshield\\Pictures\\Sonet_NIM_LEDs_Off.jpg");
		
			Delay(1.0); // delay before opening next popup panel
		
			strcpy(regVal,"000000"); // register value to turn on all sonet nim leds
		
			if (giStatusLedFlg == 0) iStatus = 1; // see if the pass button was pressed, if not set the error flag
		
			giStatusLedFlg = 0; // clear the pass/fail status flag
		}
	}
	else if (giIsDppm10Ge) // is the uut a 10Ge board
	{
		strcpy(initRegState,GetLedRegState(regAddr[2])); // save the initial register value state
		Scan(initRegState,"%s>%x",&iRegState);           // convert string value to integer value
		
		// display the status leds off and reference picture
		ResetTextBox      (giNimLedPanel, NIMLED_TEXTBOX, ""); // clear the text box
		SetPanelAttribute (giNimLedPanel, ATTR_TITLE,"DPPM-800 NIM LEDs Verification");
		InsertTextBoxLine (giNimLedPanel, NIMLED_TEXTBOX, -1,"Press the 'Pass' button if both 10Ge NIM Capture LEDs are");
		InsertTextBoxLine (giNimLedPanel, NIMLED_TEXTBOX, -1,"ON as shown in the picture below.\n");
		InsertTextBoxLine (giNimLedPanel, NIMLED_TEXTBOX, -1,"Port 2 controlled by Transceiver HW so we cannot force LED");
		InsertTextBoxLine (giNimLedPanel, NIMLED_TEXTBOX, -1,"status.\n");
		InsertTextBoxLine (giNimLedPanel, NIMLED_TEXTBOX, -1,"Press the 'Fail' button if either of the 2 LEDs are OFF.");
		DisplayImageFile  (giNimLedPanel, NIMLED_PICTURE,"C:\\Cloudshield\\Pictures\\NIM_10Ge_LEDs_On.jpg");
		
		for (i=0; i<2; i++)
		{
			giButtonPressFlg = 0; // clear the button press flag
	
			if (i == 0)
				valueToSet = (iRegState | 0x00780000);  // set value to turn on all leds
			else
			{
				valueToSet = (iRegState  | 0x00500000); // set value to turn off all leds
				valueToSet = (valueToSet & 0xFFD7FFFF);
			}
			sprintf(cmdStr,"/nfs/ixp2800/bin/poke %s %x\r",regAddr[2],valueToSet); // init the command string
	
			// first turn on all nim leds then on the 2nd pass, turn them all off
			if (WriteReadUntil(hHandle1RS232, gcReadBuffer, cmdStr, gsIxpLogPrompt, 5) != 0)
				iStatus = 1;
		
			InstallPopup(giNimLedPanel); // open the status led panel
		
			while (!giButtonPressFlg)    // wait here until either the Pass or Fail button is pressed
				ProcessSystemEvents();

			RemovePopup(giNimLedPanel);  // close the status led panel
		
			// display the status leds off and reference picture
			ResetTextBox      (giNimLedPanel, NIMLED_TEXTBOX, ""); // clear the text box
			InsertTextBoxLine (giNimLedPanel, NIMLED_TEXTBOX, -1,"Press the 'Pass' button if both 10Ge NIM Capture LEDS are OFF");
			InsertTextBoxLine (giNimLedPanel, NIMLED_TEXTBOX, -1,"as shown in the picture below.\n");
			InsertTextBoxLine (giNimLedPanel, NIMLED_TEXTBOX, -1,"Port 2 controlled by Transceiver HW so we cannot force LED");
			InsertTextBoxLine (giNimLedPanel, NIMLED_TEXTBOX, -1,"status.\n");
			InsertTextBoxLine (giNimLedPanel, NIMLED_TEXTBOX, -1,"Press the 'Fail' button if any of the LEDs are still ON.");
			DisplayImageFile  (giNimLedPanel, NIMLED_PICTURE,"C:\\Cloudshield\\Pictures\\NIM_10Ge_LEDs_Off.jpg");
		
			Delay(1.0); // delay before opening next popup panel
		
			if (giStatusLedFlg == 0) iStatus = 1; // see if the pass button was pressed, if not set the error flag
		
			giStatusLedFlg = 0; // clear the pass/fail status flag
		}
		valueToSet = (iRegState & 0xFFAFFFFF); // leave all leds unchanged
		sprintf(cmdStr,"/nfs/ixp2800/bin/poke %s %i\r",regAddr[2], valueToSet);
		// set value for all leds unchanged
		if (WriteReadUntil(hHandle1RS232, gcReadBuffer, cmdStr, gsIxpLogPrompt, 5) != 0)
			giStatus = 1;
	}
	DisplayPanel(gMainPanel); // display the main panel
exit:
	LogOutFromIxp(hHandle1RS232); // logout from the ixp
	
	CloseConnection(hHandle1RS232); // close the com port
	
	if (giStatus) // check if global error status error flag is set
		iStatus = 1;
	
	// display pass or fail and save the test result  
	if (iStatus)
	{
		strcpy(gsTestStatus.cChkNimLEDs,"Fail");
		SetTestStatus(VAL_RED, "Fail");
		SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, 4); 	
		strcat(gsComment,"NIM LEDs, ");
	}
	else
	{
		strcpy(gsTestStatus.cChkNimLEDs,"Pass");
		SetTestStatus(VAL_DK_GREEN, "Pass");
		SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, 1); 	
	}	
	return iStatus;
}

/*
 *	GetLedRegState()()
 *
 *  this function gets the initial register value for the led tests
 */
char* GetLedRegState(char* regAddr)
{
	int i, x, y;
	char addrStr[75] = "\0";
	char* sRegState = "        \0";
	
	sprintf(addrStr,"/nfs/ixp2800/bin/peek %s\r",regAddr); // init the command string to peek the register value
	
	WriteReadUntil(hHandle1RS232, gcReadBuffer, addrStr, gsIxpLogPrompt, 10); // now write the command
	
	if ((x = FindPattern (gcReadBuffer, 0, -1,":", 0, 1) + 2) > -1) // parse the value from the buffer
	{
		for (i=0; i<8; i++)
		{
			sRegState[i] = gcReadBuffer[x];
			x++;
		}
	}
	return sRegState; // return with the parsed register value
}

/*
 *	StatusLeds_CB()
 *
 *  this callback function sets the button press flag for the led function to exit.
 *  also, the pass of rail status flag is also set depending on which button was pressed.
 */
int CVICALLBACK StatusLeds_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			giButtonPressFlg = 1; // set the button pressed flag
			if (control == STATUSLED_PASS)
				giStatusLedFlg = TRUE; // sets the flag when the pass button is clicked
			break;
	}
	return 0;
}

/*
 *	PAXMemoryTest()
 *
 *  this function runs the pax memory test and verifies the test results
 */
int PAXMemoryTest()
{
	int ii, port, bank;
	int iStatus = 0;
	int keyPattFound[4];
	char testStr[75] = "\0";
	char* cPaxMemTestKeyWord = "Overall test result: PASSED";
	char cmd[75] = "\0";
	
	giStatus = 0; // clear the error status flag
	
	if (giIsDppmGe) sprintf(testStr,"GigE PAX Memory test.");
	else if (giIsDppmSonet) sprintf(testStr,"Sonet PAX Memory test.");
	else if (giIsDppm10Ge) sprintf(testStr,"10Ge PAX Memory test.");
		
	// get the list box line for this test
	GetTreeItemFromLabel (gMainPanel, PANEL_TESTLIST,VAL_ALL,0,VAL_FIRST,VAL_NEXT_PLUS_SELF,0,testStr,&gCurListBoxLine);
	
	CheckListItem(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, 1); 
	sprintf(testStr,"\n########## PAX Memory tests ##########\n"); // write 
	WriteFile (giLogFileHandle,testStr,StringLength(testStr));
	SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, -1); 	
	
	Try (OpenConnectionRS232(DPPM)); // open a com port connection to the DPPM
	
	Try (GoToIxpPrompt(hHandle1RS232));	 // connect to the dppm ixp
	//
	for (ii=0; ii<4; ii++)
	 	keyPattFound[ii] = 0; // clear the key pattern string flags

	for (port=0, ii=0; port<2; port++) // loop through the 2 ports and 2 banks per port
	{
		for (bank=0; bank<2; bank++) // looping through the 2 banks
		{
			sprintf(testStr,"Testing PAX Memory Port %i, Bank %i",port, bank);
			SetTestStatus(VAL_BLUE, testStr);
	
			// run the pax port memory test
			sprintf(cmd,"/nfs/ixp2800/bin/pp2500memtest -t -s 0 -n 0x100000 -b %i -c 1 /dev/pax%i\r",bank,port);
			WriteReadUntil(hHandle1RS232, gcReadBuffer, cmd, gsIxpLogPrompt, 45);
			//search for the key pattern passed result and set the pattern found flag if key pattern found
			if(FindPattern (gcReadBuffer, 0, -1, cPaxMemTestKeyWord, 0, 1) > -1) keyPattFound[ii]++;
			ii++; // bump the array pointer
		}
	}
	// see if all of the passed key pattern strings were found
	if (!keyPattFound[0] || !keyPattFound[1] || !keyPattFound[2] || !keyPattFound[3]) // check if the keyword was found
		iStatus++;	   // if not, mark as error
exit:
	LogOutFromIxp(hHandle1RS232); // logout from the ixp
	
	CloseConnection(hHandle1RS232); // disconnect the rs232 port
	
	if (giStatus) // check if global error status error flag is set
		iStatus = 1;
	
	// display pass or fail and save the test result
	if (iStatus)
	{
		strcpy(gsTestStatus.cPaxMemory,"Fail");
		SetTestStatus(VAL_RED, "Fail");
		SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, 4); 	
		strcat(gsComment,"PAX Memory, ");
	}
	else
	{   
		strcpy(gsTestStatus.cPaxMemory,"Pass");
		SetTestStatus(VAL_DK_GREEN, "Pass");
		SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, 1); 	
	}
	return iStatus;
}

/*
 *	QDRamMemoryTest()()
 *
 *  this function runs the qdram memory test on the 10Ge dppm board only and verifies the test results
 */
int QDRamMemoryTest()
{
	int ii, iStatus = 0;
	char testStr[75] = "\0";
	char* cQDRamMemTestKeyWord = "Test PASSED";
	
	giStatus = 0; // clear the error status flag
	
	sprintf(testStr,"10Ge QDRam Memory test.");
		
	// get the list box line for this test
	GetTreeItemFromLabel (gMainPanel, PANEL_TESTLIST,VAL_ALL,0,VAL_FIRST,VAL_NEXT_PLUS_SELF,0,testStr,&gCurListBoxLine);
	
	CheckListItem(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, 1); 
	sprintf(testStr,"\n########## QDRam Memory tests ##########\n"); // write 
	WriteFile (giLogFileHandle,testStr,StringLength(testStr));
	SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, -1); 	
	SetTestStatus(VAL_BLUE, "Testing QDRAM Memory");
	
	Try (OpenConnectionRS232(ASMCAP)); // open a com port 4 connection to the captive asm

	Try (ConnectToASMPort(hHandle4RS232));	// connect to the asm and go to the cli prompt
	
	Try (GoToLinuxPrompt(hHandle4RS232)); // go to the goshell prompt
	
	//  run the gdram test
	Try (WriteReadUntil(hHandle4RS232, gcReadBuffer,"/cs/bin/qdramtest 4\r", gsGoshellPrompt, 120));
		
	if(FindPattern (gcReadBuffer, 0, -1, cQDRamMemTestKeyWord, 0, 1) < 0) // parse the buffer for the key word string
	    iStatus = 1;  // set error status if key word not found
	
	SetTreeCellAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, 1, ATTR_LABEL_COLOR, VAL_BLUE);  
	SetTreeCellAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, 1, ATTR_LABEL_TEXT, "Initializing UUT");
	
	SetTestStatus(VAL_BLUE, "Initializing UUT..........");
	//  initialize the uut dppm back to a good state
	Try (WriteReadUntil(hHandle4RS232, gcReadBuffer,"/cs/bin/goixps 4\r", gsGoshellPrompt, 300));
exit:
	GoToCliPrompt(hHandle4RS232); // logout from Goshell
	
	LogOutFromCli(hHandle4RS232); // logout from the CLI
	
	CloseConnection(hHandle4RS232); // close the virtual telnet com port
	
	if (giStatus) // check if global error status error flag is set
		iStatus = 1;
	
	// display pass or fail  and save the test result  
	if (iStatus)
	{
		strcpy(gsTestStatus.cQDRamMem,"Fail");
		SetTestStatus(VAL_RED, "Fail");
		SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, 4); 	
		strcat(gsComment,"QDRam Memory, ");
	}
	else
	{   
		strcpy(gsTestStatus.cQDRamMem,"Pass");
		SetTestStatus(VAL_DK_GREEN, "Pass");
		SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, 1); 	
	}	
	return iStatus;
}

/*
 *	CAMMemoryTest()
 *
 *  this function runs the cam memory test and verifies the test results
 */
int CAMMemoryTest()
{
	int ii, iStatus = 0;
	char testStr[75] = "\0";
	char* cCamMemTestKeyWord = "Cam Test PASSED";
	
	giStatus = 0; // clear the error status flag
	
	if (giIsDppmGe) sprintf(testStr,"GigE CAM Memory test.");
	else if (giIsDppmSonet) sprintf(testStr,"Sonet CAM Memory test.");
	else if (giIsDppm10Ge) sprintf(testStr,"10Ge CAM Memory test.");
		
	// get the list box line for this test
	GetTreeItemFromLabel (gMainPanel, PANEL_TESTLIST,VAL_ALL,0,VAL_FIRST,VAL_NEXT_PLUS_SELF,0,testStr,&gCurListBoxLine);
	
	CheckListItem(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, 1); 
	sprintf(testStr,"\n########## CAM Memory tests ##########\n"); // write 
	WriteFile (giLogFileHandle,testStr,StringLength(testStr));
	SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, -1); 	
	SetTestStatus(VAL_BLUE, "Testing CAM Memory");
	
	Try (OpenConnectionRS232(DPPM)); // open a com port connection to the DPPM
	
	Try (GoToIxpPrompt(hHandle1RS232));	 // connect to the dppm ixp
	
	Try (WriteReadUntil(hHandle1RS232, gcReadBuffer,"/nfs/ixp2800/bin/cam_memtest\r", gsIxpLogPrompt, 60));
		
	if(FindPattern (gcReadBuffer, 0, -1, cCamMemTestKeyWord, 0, 1) < 0)
	    giStatus = 1;
exit:
	LogOutFromIxp(hHandle1RS232); // logout from the ixp
	
	CloseConnection(hHandle1RS232); // disconnect the rs232 port
	
	if (giStatus) // check if global error status error flag is set
		iStatus = 1;
	
	// display pass or fail and save the test result
	if (iStatus)
	{
		strcpy(gsTestStatus.cCamMemory,"Fail");
		SetTestStatus(VAL_RED, "Fail");
		SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, 4); 	
		strcat(gsComment,"CAM Memory, ");
	}
	else
	{
		strcpy(gsTestStatus.cCamMemory,"Pass");
		SetTestStatus(VAL_DK_GREEN, "Pass");
		SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, 1); 	
	}
	return iStatus;
}

/*
 *	SDBMemoryTest()
 *
 *  this function runs the sdb memory tests and veryfies that they all pass
 */
int SDBMemoryTest()
{
	int ii, iStatus = 0;
	int keyPattFound[4];
	char testStr[75] = "\0";
	char cCmdStr[75] = "\0";
	char* cMemTestKeyWords[] = {"SDB DDR_SDRAM 32-bit Memory Test Passed",
								"SDB DDR_SDRAM 16-bit Memory Test Passed",
								"SDB DDR_SDRAM 8-bit Memory Test Passed",
								"SDB DDR_SDRAM Memory Test Passed"};
	
	giStatus = 0; // clear the error status flag		
	
	if (giIsDppmGe) sprintf(testStr,"GigE SDB Memory test.");
	else if (giIsDppmSonet) sprintf(testStr,"Sonet SDB Memory test.");
	else if (giIsDppm10Ge) sprintf(testStr,"10Ge SDB Memory test.");
		
	// get the list box line for this test
	GetTreeItemFromLabel (gMainPanel, PANEL_TESTLIST,VAL_ALL,0,VAL_FIRST,VAL_NEXT_PLUS_SELF,0,testStr,&gCurListBoxLine);
	
	CheckListItem(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, 1); 
	sprintf(testStr,"\n########## SDB Memory tests ##########\n"); // write 
	WriteFile (giLogFileHandle,testStr,StringLength(testStr));
	SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, -1); 	
	SetTestStatus(VAL_BLUE, "Testing SDB Memory");
	
	Try (OpenConnectionRS232(DPPM)); // open a com port connection to the DPPM
	
	Try (GoToIxpPrompt(hHandle1RS232));	 // connect to the dppm ixp linux prompt
	
	if(WriteReadUntil(hHandle1RS232, gcReadBuffer,"/nfs/bin/loadsdbtestfpga.sh\r", gsIxpLogPrompt, 30) == 0);
	// 
	if(WriteReadUntil(hHandle1RS232, gcReadBuffer,"/nfs/bin/diagsh ram memtest start\r", gsIxpLogPrompt, 10) == 0);
	// 
	if(WriteReadUntil(hHandle1RS232, gcReadBuffer,"/nfs/bin/diagsh ram memtest status wait\r", gsIxpLogPrompt, 300) == 0);
	
	 for (ii=0; ii<4; ii++)
		 keyPattFound[ii] = 0; // clear the key pattern string found flags
	 
	// search for the memory test passed keyword strings	
	if(FindPattern (gcReadBuffer, 0, -1, cMemTestKeyWords[0], 0, 1) > -1) keyPattFound[0]++;
	if(FindPattern (gcReadBuffer, 0, -1, cMemTestKeyWords[1], 0, 1) > -1) keyPattFound[1]++;
	if(FindPattern (gcReadBuffer, 0, -1, cMemTestKeyWords[2], 0, 1) > -1) keyPattFound[2]++;
	if(FindPattern (gcReadBuffer, 0, -1, cMemTestKeyWords[3], 0, 1) > -1) keyPattFound[3]++;
	
	// see if all of the passed key pattern strings were found
	if (!keyPattFound[0] || !keyPattFound[1] || !keyPattFound[2] || !keyPattFound[3]) // check if the keyword was found
		iStatus++;	   // if not, mark as error
exit:
	LogOutFromIxp(hHandle1RS232); // logout from the ixp linux prompt
	
	CloseConnection(hHandle1RS232); // disconnect the rs232 port
	
	if (giStatus) // check if global error status error flag is set
		iStatus = 1;
	
	// display pass or fail and save the test result
	if (iStatus)
	{
		strcpy(gsTestStatus.cSdbMemory,"Fail");
		SetTestStatus(VAL_RED, "Fail");
		SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, 4); 	
		strcat(gsComment,"SDB Memory, ");
	}
	else
	{
		strcpy(gsTestStatus.cSdbMemory,"Pass");
		SetTestStatus(VAL_DK_GREEN, "Pass");
		SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, 1); 	
	}
	return iStatus;
}

/*
 *	RocketIOTest()
 *
 *  this function runs the rocket io test and verifies the test results
 *  this test requires a captive dppm in slot 5
 */
int RocketIOTest()
{
	int ii, iStatus = 0;
	char cmdStr[75] = "\0";
	char testStr[75] = "\0";
	char cBitFile1[25] = "\0";
	char cBitFile2[25] = "\0";
	char* cTmpBuf1 = malloc(5000);
	char* cTmpBuf2 = malloc(5000);
	const char* RESULT_1F[] = {"0000001F"};
	const char* RESULT_03[] = {"00000003"};
	
	giStatus = 0; // clear the error status flag		
	giRocketIOFlg = 1;
	
	if (giIsDppmGe) sprintf(testStr,"GigE Rocket IO Ports test.");
	else if (giIsDppmSonet) sprintf(testStr,"Sonet Rocket IO Ports test.");
		
	// get the list box line for this test
	GetTreeItemFromLabel (gMainPanel, PANEL_TESTLIST,VAL_ALL,0,VAL_FIRST,VAL_NEXT_PLUS_SELF,0,testStr,&gCurListBoxLine);
	
	CheckListItem(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, 1); 
	
	sprintf(testStr,"\n########## Rocket IO Test ##########\n"); // write test identifier to log file
	WriteFile (giLogFileHandle,testStr,StringLength(testStr));
	SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, -1); 	
	SetTestStatus(VAL_BLUE, "Initializing for Rocket IO.....");
	
    giRocketIoPanel = LoadPanel (0, "FatTestGui.uir", ROCKETIO); // load the web browser panel

	SetPanelAttribute (giRocketIoPanel, ATTR_LEFT, giLeftPos + 492);
	
	if (!giSingleTestFlg) // check to see if single step flag is set
		InstallPopup(giRocketIoPanel);   // display the help panel
	
	Try (OpenConnectionRS232(DPPM)); // open a com port connection to the DPPM UUT
	
	Try (GoToIxpPrompt(hHandle1RS232));	// go to the uut dppm linux prompt
	
	Try (ConnectToCaptiveDppm()); // connect to the cative dppm and go to the linux prompt
	
	Try (GetReadyForRocketIO()); // get ready to run the rocket io test
	
	// get the fpga bit file names
	Ini_GetStringIntoBuffer(gIniText,"FPGA Files","File1", cBitFile1, sizeof(cBitFile1));
	Ini_GetStringIntoBuffer(gIniText,"FPGA Files","File2", cBitFile2, sizeof(cBitFile2));
	
	SetTestStatus(VAL_BLUE, "Loading UUT FPGA files");
	
	// load the first fpga bit file to the uut
	sprintf(cmdStr,"/nfs/ixp2800/bin/fpgaloader -f psw2 -c /nfs/extra/%s\r",cBitFile1);
	if (WriteReadUntil(hHandle1RS232, gcReadBuffer, cmdStr, gsIxpLogPrompt, 20) != 0) iStatus = 1;
	ProcessSystemEvents();
	// load the second fpga bit file 
	sprintf(cmdStr,"/nfs/ixp2800/bin/fpgaloader -f sdb -c /nfs/extra/%s\r",cBitFile2);
	if (WriteReadUntil(hHandle1RS232, gcReadBuffer, cmdStr, gsIxpLogPrompt, 20) != 0) iStatus = 1;
	ProcessSystemEvents();
	SetTestStatus(VAL_BLUE, "UUT Rocket IO Clear");
	
	// send an rio clear to the uut 
	sprintf(cmdStr,"/nfs/mfgtools/rio_clear\r");
	if (WriteReadUntil(hHandle1RS232, gcReadBuffer, cmdStr, gsIxpLogPrompt, 5) != 0) iStatus = 1;
	ProcessSystemEvents();
	SetTestStatus(VAL_BLUE, "Loading Captive FPGA files");

	// load the first fpga bit file to the captive dppm
	sprintf(cmdStr,"/nfs/ixp2800/bin/fpgaloader -f psw2 -c /nfs/extra/%s\r",cBitFile1);
	if (WriteReadUntil(hHandle3RS232, gcReadBuffer, cmdStr, gsIxpLogPrompt, 20) != 0) iStatus = 1;
	ProcessSystemEvents();
	// load the second fpga bit file 
	sprintf(cmdStr,"/nfs/ixp2800/bin/fpgaloader -f sdb -c /nfs/extra/%s\r",cBitFile2);
	if (WriteReadUntil(hHandle3RS232, gcReadBuffer, cmdStr, gsIxpLogPrompt, 20) != 0) iStatus = 1;
	ProcessSystemEvents();
	SetTestStatus(VAL_BLUE, "Captive Rocket IO Clear");
	
	// send an rio clear to the captive dppm 
	sprintf(cmdStr,"bash /nfs/mfgtools/rio_clear\r");
	if (WriteReadUntil(hHandle3RS232, gcReadBuffer, cmdStr, gsIxpLogPrompt, 5) != 0) iStatus = 1;
	ProcessSystemEvents();
	SetTestStatus(VAL_BLUE, "Start Rocket IO Transmit");
	
	// send an rio transmit to the uut dppm 
	sprintf(cmdStr,"bash /nfs/mfgtools/rio_transmit\r");
	if (WriteReadUntil(hHandle1RS232, gcReadBuffer, cmdStr, gsIxpLogPrompt, 5) != 0) iStatus = 1;
	
	// send an rio transmit to the captive dppm 
	sprintf(cmdStr,"bash /nfs/mfgtools/rio_transmit\r");
	if (WriteReadUntil(hHandle3RS232, gcReadBuffer, cmdStr, gsIxpLogPrompt, 5) != 0) iStatus = 1;
	ProcessSystemEvents();
	Delay(1.0); // delay 1 second before stopping
	SetTestStatus(VAL_BLUE, "Stop Rocket IO Transmit");

	// send an rio stop to the uut dppm 
	sprintf(cmdStr,"bash /nfs/mfgtools/rio_stop\r");
	if (WriteReadUntil(hHandle1RS232, gcReadBuffer, cmdStr, gsIxpLogPrompt, 5) != 0) iStatus = 1;
	
	// send an rio stop to the captive dppm 
	sprintf(cmdStr,"bash /nfs/mfgtools/rio_stop\r");
	if (WriteReadUntil(hHandle3RS232, gcReadBuffer, cmdStr, gsIxpLogPrompt, 5) != 0) iStatus = 1;
	ProcessSystemEvents();
	SetTestStatus(VAL_BLUE, "Read Rocket IO Counters");
	
	// send an rio read to the uut dppm 
	sprintf(cmdStr,"bash /nfs/mfgtools/rio_read\r");
	if (WriteReadUntil(hHandle1RS232, gcReadBuffer, cmdStr, gsIxpLogPrompt, 5) != 0) iStatus = 1;
	strcpy(cTmpBuf1,gcReadBuffer);
	ProcessSystemEvents();
	
	// send an rio read to the captive dppm 
	sprintf(cmdStr,"bash /nfs/mfgtools/rio_read\r");
	if (WriteReadUntil(hHandle3RS232, gcReadBuffer, cmdStr, gsIxpLogPrompt, 5) != 0) iStatus = 1;
	strcpy(cTmpBuf2,gcReadBuffer);
	ProcessSystemEvents();
	SetTestStatus(VAL_BLUE, "Validating Test Results");
		
	if(ParseRocketIOTestResults(cTmpBuf1, cTmpBuf2) != 0)
	    iStatus = 1;
exit:
	if (!giSingleTestFlg) // check to see if single step flag is set
		RemovePopup(giRocketIoPanel);   // display the help panel
	
	LogOutFromTheCaptiveDppm(); // logout from the CLI
	
	LogOutFromIxp(hHandle1RS232); // logout from the ixp
	
	CloseConnection(hHandle1RS232);
	CloseConnection(hHandle3RS232);
	
	if (giStatus) // check if global error status error flag is set
		iStatus = 1;
	
	// display pass or fail and save the test result  
	if (iStatus)
	{
		strcpy(gsTestStatus.cRocketIO,"Fail");
		SetTestStatus(VAL_RED, "Fail");
		SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, 4); 	
		strcat(gsComment,"Rocket IO, ");
	}
	else
	{   
		strcpy(gsTestStatus.cRocketIO,"Pass");
		SetTestStatus(VAL_DK_GREEN, "Pass");
		SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, 1); 	
	}	
	free(cTmpBuf1); // free allocated temporary read buffers
	free(cTmpBuf2);
	giRocketIOFlg = 0; // clear rocket io test flag
	
	return iStatus;
}

/*
 *	ParseRocketIOTestResults()
 *
 *  this function parses the test results and compares the uut counters with the captive counters and
 *  clears or sets the status flag based on the results.
 */
int ParseRocketIOTestResults(char* cTmpBuf1, char* cTmpBuf2)
{
	int i, j, x, iStatus = 0;
	int cntVal1, cntVal2;
	char RESULT_1F[9] = {"0000001F\0"};
	char RESULT_03[9] = {"00000003\0"};
	char RESULT_00[9] = {"00000000\0"};
	char CounterValUUT1[10] = {"\0"};
	char CounterValCAP1[10] = {"\0"};
	char CounterValUUT2[10] = {"\0"};
	char CounterValCAP2[10] = {"\0"};
	char cntValStr1[10],cntValStr2[10];
	
	// get the index position for the first value from the buffer, both buffers are the same
	x = FindPattern(cTmpBuf1, 0, -1,":", 1, 0) + 2;
	
	// ##### Compare UUT PSW2 Counters to Captive PSW2 Counters ##### //
	// verify that the PSW2-to-PSW2 uut and captive counters compare between the rx and tx
	for (j=89, i=0; j<97; j++, i++)
	{   // get the PSW2-to-PSW2 counter values @ location C7604000
		CounterValUUT1[i] = cTmpBuf1[j];
		CounterValCAP1[i] = cTmpBuf2[j];
	}
	// compare the values on both the uut and the captive dppms, they should be "0000001F"
	if (strcmp(CounterValUUT1, RESULT_1F) != 0) iStatus = 1;
	if (strcmp(CounterValCAP1, RESULT_1F) != 0) iStatus = 1;

//	x = FindPattern(cTmpBuf1, x, -1,":", 1, 0) + 2;
	for (j=149, i=0; j<157; j++, i++)
	{   // get the PSW2-to-PSW2 counter values @ location C7604008
		CounterValUUT1[i] = cTmpBuf1[j];
		CounterValCAP1[i] = cTmpBuf2[j];
	}
	for (j=179, i=0; j<187; j++, i++)
	{   // get the PSW2-to-PSW2 counter values @ location C760400C
		CounterValUUT2[i] = cTmpBuf1[j];
		CounterValCAP2[i] = cTmpBuf2[j];
	}
	// compare the values between the uut and the captive dppms, they should be equal
	if (strcmp(CounterValUUT1, CounterValCAP2) != 0) iStatus = 1;
	if (strcmp(CounterValUUT2, CounterValCAP1) != 0) iStatus = 1;
	
	// if the uut is a Sonet board, then compare the SONOP counter values
	if (giIsDppmSonet)
	{
		// ##### Compare UUT SONOP Counters to Captive SONOP Counters ##### //
		for (j=950, i=0; j<958; j++, i++)
		{   // get the SONOP-to-SONOP counter values @ location C7030000
			CounterValUUT1[i] = cTmpBuf1[j];
			CounterValCAP1[i] = cTmpBuf2[j];
		}
		// compare the values between the uut and the captive dppms, they should be equal
		if (strcmp(CounterValUUT1, RESULT_03) != 0) iStatus = 1;
		if (strcmp(CounterValCAP1, RESULT_03) != 0) iStatus = 1;
		for (j=1010, i=0; j<1018; j++, i++)
		{   // get the SONOP-to-SONOP counter values @ location C7030008
			CounterValUUT1[i] = cTmpBuf1[j];
			CounterValCAP1[i] = cTmpBuf2[j];
		}
		for (j=1070, i=0; j<1078; j++, i++)
		{   // get the SONOP-to-SONOP counter values @ location C7030010
			CounterValUUT2[i] = cTmpBuf1[j];
			CounterValCAP2[i] = cTmpBuf2[j];
		}
		// compare the values between the uut and the captive dppms, they should be equal
		if (strcmp(CounterValUUT1, CounterValCAP2) != 0) iStatus = 1;
		if (strcmp(CounterValUUT2, CounterValCAP1) != 0) iStatus = 1;
	}
	// ##### Compare UUT SDB Counters to Captive SDB Counters ##### //
	for (j=1228, i=0; j<1236; j++, i++)
	{   // get the SDB-to-SDB counter values @ location C7830000
		CounterValUUT1[i] = cTmpBuf1[j];
		CounterValCAP1[i] = cTmpBuf2[j];
	}
	// compare the values on both the uut and the captive dppms, they should be "0000001F"
	if (strcmp(CounterValUUT1, RESULT_1F) != 0) iStatus = 1;
	if (strcmp(CounterValCAP1, RESULT_1F) != 0) iStatus = 1;
	
	for (j=1288, i=0; j<1296; j++, i++)
	{   // get the SDB-to-SDB counter values @ location C7830008
		CounterValUUT1[i] = cTmpBuf1[j];
		CounterValCAP1[i] = cTmpBuf2[j];
	}
	for (j=1318, i=0; j<1326; j++, i++)
	{   // get the SDB-to-SDB counter values @ location C783000C
		CounterValUUT2[i] = cTmpBuf1[j];
		CounterValCAP2[i] = cTmpBuf2[j];
	}   
	// compare the values between the uut and the captive dppms, they should be equal
	if (strcmp(CounterValUUT1, CounterValCAP2) != 0) iStatus = 1;
	if (strcmp(CounterValUUT2, CounterValCAP1) != 0) iStatus = 1;
	
	return iStatus;
}

/*
 *	RocketIO10GeTest()
 *
 *  this function runs the rocket io test for the 10Ge and verifies the test results
 *  this test requires a captive dppm in slot 5
 */
int RocketIO10GeTest()
{
	int ii, iStatus = 0;
	char cmdStr[75] = "\0";
	char testStr[75] = "\0";
	char cFpgaPswFile[25] = "\0";
	char cFpgaSdbFile[25] = "\0";
	char* cTmpBuf1 = malloc(5000);
	char* cTmpBuf2 = malloc(5000);
	const char* RESULT_1F[] = {"0000001F"};
	const char* RESULT_03[] = {"00000003"};
	
	giStatus = 0; // clear the error status flag		
	giRocketIOFlg = 1;
	
	sprintf(testStr,"10Ge Rocket IO Ports test.");
		
	// get the list box line for this test
	GetTreeItemFromLabel (gMainPanel, PANEL_TESTLIST,VAL_ALL,0,VAL_FIRST,VAL_NEXT_PLUS_SELF,0,testStr,&gCurListBoxLine);
	
	CheckListItem(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, 1); 
	
	sprintf(testStr,"\n########## Rocket IO 10Ge Test ##########\n"); // write test identifier to log file
	WriteFile (giLogFileHandle,testStr,StringLength(testStr));
	SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, -1); 	
	SetTestStatus(VAL_BLUE, "Initializing for Rocket IO.....");
	
    giRocketIoPanel = LoadPanel (0, "FatTestGui.uir", ROCKETIO); // load the rocket io help panel

	SetPanelAttribute (giRocketIoPanel, ATTR_LEFT, giLeftPos + 492);
	
	if (!giSingleTestFlg) // check to see if single step flag is set
		InstallPopup(giRocketIoPanel);   // display the help panel
	
	Try (GetReadyForRocketIO()); // get ready to run the rocket io test
	
	if (OpenConnectionRS232(DPPM)) goto exit1; // open a com port connection to the DPPM UUT
	
	if (OpenConnectionRS232(DPPMCAP)) goto exit2; // open a com port connection to the DPPM UUT
	
	Try (GoToIxpPrompt(hHandle1RS232));	// go to the uut dppm linux prompt
	
	Try (GoToIxpPrompt(hHandle3RS232));	// go to the captive dppm linux prompt
	
	// get the fpga bit file names
	Ini_GetStringIntoBuffer(gIniText,"FPGA Files","File3", cFpgaPswFile, sizeof(cFpgaPswFile));
	Ini_GetStringIntoBuffer(gIniText,"FPGA Files","File2", cFpgaSdbFile, sizeof(cFpgaSdbFile));
	ProcessSystemEvents();
	SetTestStatus(VAL_BLUE, "Loading UUT FPGA files");
	SetTreeCellAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, 1, ATTR_LABEL_TEXT, "Loading UUT FPGA files");
	
	// load the first fpga bit file to the uut
	sprintf(cmdStr,"/nfs/ixp2800/bin/fpgaloader -f aux10 -c /nfs/extra/%s\r",cFpgaPswFile);
	if (WriteReadUntil(hHandle1RS232, gcReadBuffer, cmdStr, gsIxpLogPrompt, 45) != 0) iStatus = 1;
	ProcessSystemEvents();
	Delay(1.0);
	// load the second fpga bit file 
	sprintf(cmdStr,"/nfs/ixp2800/bin/fpgaloader -f sdb -c /nfs/extra/%s\r",cFpgaSdbFile);
	if (WriteReadUntil(hHandle1RS232, gcReadBuffer, cmdStr, gsIxpLogPrompt, 90) != 0) iStatus = 1;
	ProcessSystemEvents();
	Delay(1.0);
	
	SetTestStatus(VAL_BLUE, "Loading Captive FPGA files");

	// load the first fpga bit file to the captive dppm
	sprintf(cmdStr,"/nfs/ixp2800/bin/fpgaloader -f aux10 -c /nfs/extra/%s\r",cFpgaPswFile);
	if (WriteReadUntil(hHandle3RS232, gcReadBuffer, cmdStr, gsIxpLogPrompt, 45) != 0) iStatus = 1;
	ProcessSystemEvents();
	Delay(1.0);
	// load the second fpga bit file 
	sprintf(cmdStr,"/nfs/ixp2800/bin/fpgaloader -f sdb -c /nfs/extra/%s\r",cFpgaSdbFile);
	if (WriteReadUntil(hHandle3RS232, gcReadBuffer, cmdStr, gsIxpLogPrompt, 90) != 0) iStatus = 1;
	ProcessSystemEvents();
	Delay(2.0);
	SetTestStatus(VAL_BLUE, "Clear Rocket IO counters");
	
	// send a rio clear to the uut 
	sprintf(cmdStr,"/nfs/mfgtools/rio_clear_10G\r");
	if (WriteReadUntil(hHandle1RS232, gcReadBuffer, cmdStr, gsIxpLogPrompt, 5) != 0) iStatus = 1;
	ProcessSystemEvents();
	Delay(1.0);
	
	// send a rio clear to the captive dppm 
	sprintf(cmdStr,"/nfs/mfgtools/rio_clear_10G\r");
	if (WriteReadUntil(hHandle3RS232, gcReadBuffer, cmdStr, gsIxpLogPrompt, 5) != 0) iStatus = 1;
	ProcessSystemEvents();
	Delay(1.0);
	SetTestStatus(VAL_BLUE, "Start Rocket IO Transmit & wait 10 seconds");
	
	// send a rio transmit to the uut 
	sprintf(cmdStr,"/nfs/mfgtools/rio_transmit_10G\r");
	if (WriteReadUntil(hHandle1RS232, gcReadBuffer, cmdStr, gsIxpLogPrompt, 5) != 0) iStatus = 1;
	ProcessSystemEvents();
	Delay(1.0);
	
	// send a rio transmit to the captive dppm 
	sprintf(cmdStr,"/nfs/mfgtools/rio_transmit_10G\r");
	if (WriteReadUntil(hHandle3RS232, gcReadBuffer, cmdStr, gsIxpLogPrompt, 5) != 0) iStatus = 1;
	ProcessSystemEvents();
	Delay(10.0); // delay 10 seconds before stopping
	SetTestStatus(VAL_BLUE, "Stop Rocket IO Transmit");
	
	// send an rio stop to the uut 
	sprintf(cmdStr,"/nfs/mfgtools/rio_stop_10G\r");
	if (WriteReadUntil(hHandle1RS232, gcReadBuffer, cmdStr, gsIxpLogPrompt, 5) != 0) iStatus = 1;
	
	// send an rio stop to the captive dppm 
	sprintf(cmdStr,"/nfs/mfgtools/rio_stop_10G\r");
	if (WriteReadUntil(hHandle3RS232, gcReadBuffer, cmdStr, gsIxpLogPrompt, 5) != 0) iStatus = 1;
	ProcessSystemEvents();
	Delay(1.0);
	SetTestStatus(VAL_BLUE, "Read Rocket IO counters");

	// send an rio read to the uut 
	sprintf(cmdStr,"/nfs/mfgtools/rio_read_10G\r");
	if (WriteReadUntil(hHandle1RS232, gcReadBuffer, cmdStr, gsIxpLogPrompt, 10) != 0) iStatus = 1;
	strcpy(cTmpBuf1,gcReadBuffer);
	
	// send an rio read to the captive dppm 
	sprintf(cmdStr,"/nfs/mfgtools/rio_read_10G\r");
	if (WriteReadUntil(hHandle3RS232, gcReadBuffer, cmdStr, gsIxpLogPrompt, 10) != 0) iStatus = 1;
	strcpy(cTmpBuf2,gcReadBuffer);
	ProcessSystemEvents();
	SetTestStatus(VAL_BLUE, "Validating test results");
		
	if(ParseRocketIO10GeTestResults(cTmpBuf1, cTmpBuf2) != 0)
	    iStatus = 1;
exit:
	if (!giSingleTestFlg) // check to see if single step flag is set
		RemovePopup(giRocketIoPanel);   // display the help panel
	
	LogOutFromIxp(hHandle3RS232); // logout from the ixp
	
	LogOutFromIxp(hHandle1RS232); // logout from the ixp
	
	CloseConnection(hHandle3RS232); // close connections to uut and captive boards
exit2:
	CloseConnection(hHandle1RS232);
exit1:	
	if (giStatus) // check if global error status error flag is set
		iStatus = 1;
	
	// display pass or fail and save the test result  
	if (iStatus)
	{
		strcpy(gsTestStatus.cRocketIO10Ge,"Fail");
		SetTestStatus(VAL_RED, "Fail");
		SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, 4); 	
		strcat(gsComment,"Rocket IO, ");
	}
	else
	{   
		strcpy(gsTestStatus.cRocketIO10Ge,"Pass");
		SetTestStatus(VAL_DK_GREEN, "Pass");
		SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, 1); 	
	}	
	free(cTmpBuf1); // free allocated temporary read buffers
	free(cTmpBuf2);
	giRocketIOFlg = 0; // clear rocket io test flag
	
	return iStatus;
}

/*
 *	ParseRocketIO10GeTestResults()
 *
 *  this function parses the test results and compares the uut counters with the captive counters and
 *  clears or sets the status flag based on the results.
 */
int ParseRocketIO10GeTestResults(char* cTmpBuf1, char* cTmpBuf2)
{
	int i, j, x, iStatus = 0;
	int cntVal1, cntVal2;
	char RESULT_00[9] = {"00000000\0"};
	char CounterValUUT[10] = {"\0"};
	char CounterValCAP[10] = {"\0"};
	char CounterValTx1[10] = {"\0"};
	char CounterValTx2[10] = {"\0"};
	char CounterValRx1[10] = {"\0"};
	char CounterValRx2[10] = {"\0"};
	char cntValStr1[10],cntValStr2[10];
	
	// get the initial index position for the first value from the buffer, both buffers are the same
	x = FindPattern(cTmpBuf1, 0, -1,":", 1, 0) + 2;
	
	// verify that the RIO Tx packet counters compare between the rx and tx
	for (j=x, i=0; j<x+8; j++, i++)
	{   // get the uut rio Tx and captive rio Tx counter values @ location C7000048
		CounterValUUT[i] = cTmpBuf1[j];
		CounterValCAP[i] = cTmpBuf2[j];
	}
	// compare the values on both the uut and the captive dppms, they should be "00000000"
	if (strcmp(CounterValUUT, RESULT_00) != 0) iStatus = 1;
	if (strcmp(CounterValCAP, RESULT_00) != 0) iStatus = 1;
	
	x = FindPattern(cTmpBuf1, x, -1,":", 1, 0) + 2;
	for (j=x, i=0; j<x+8; j++, i++)
	{   // get the RIO Tx packet counter values @ location C700004C
		CounterValTx1[i] = cTmpBuf1[j];
		CounterValRx1[i] = cTmpBuf2[j];
	}
	x = FindPattern(cTmpBuf1, x, -1,":", 1, 0) + 2;
	for (j=x, i=0; j<x+8; j++, i++)
	{   // get the good Rx packet counter values @ location C7000050
		CounterValUUT[i] = cTmpBuf1[j];
		CounterValCAP[i] = cTmpBuf2[j];
	}
	x = FindPattern(cTmpBuf1, x, -1,":", 1, 0) + 2;
	for (j=x, i=0; j<x+8; j++, i++)
	{   // get the good Rx packet counter values @ location C7000054
		CounterValRx2[i] = cTmpBuf1[j];
		CounterValTx2[i] = cTmpBuf2[j];
	}
	x = FindPattern(cTmpBuf1, x, -1,":", 1, 0) + 2;
	for (j=x, i=0; j<x+8; j++, i++)
	{   // get the bad Rx packet counter values @ location C7000058
		CounterValUUT[i] = cTmpBuf1[j];
		CounterValCAP[i] = cTmpBuf2[j];
	}
	// compare the values on both the uut and the captive dppms, they should be "00000000"
	if (strcmp(CounterValUUT, RESULT_00) != 0) iStatus = 1;
	if (strcmp(CounterValCAP, RESULT_00) != 0) iStatus = 1;
	
	x = FindPattern(cTmpBuf1, x, -1,":", 1, 0) + 2;
	for (j=x, i=0; j<x+8; j++, i++)
	{   // get the frame error counter values @ location C700005C
		CounterValUUT[i] = cTmpBuf1[j];
		CounterValCAP[i] = cTmpBuf2[j];
	}   
	// compare the values on both the uut and the captive dppms, they should be "00000000"
	if (strcmp(CounterValUUT, RESULT_00) != 0) iStatus = 1;
	if (strcmp(CounterValCAP, RESULT_00) != 0) iStatus = 1;
	
	x = FindPattern(cTmpBuf1, x, -1,":", 1, 0) + 2;
	for (j=x, i=0; j<x+8; j++, i++)
	{   // get the soft error counter values @ location C7000060
		CounterValUUT[i] = cTmpBuf1[j];
		CounterValCAP[i] = cTmpBuf2[j];
	}   
	// compare the values on both the uut and the captive dppms, they should be "00000000"
	if (strcmp(CounterValUUT, RESULT_00) != 0) iStatus = 1;
	if (strcmp(CounterValCAP, RESULT_00) != 0) iStatus = 1;

	x = FindPattern(cTmpBuf1, x, -1,":", 1, 0) + 2;
	for (j=x, i=0; j<x+8; j++, i++)
	{   // get the hard error counter values @ location C7000064
		CounterValUUT[i] = cTmpBuf1[j];
		CounterValCAP[i] = cTmpBuf2[j];
	}   
	// compare the values on both the uut and the captive dppms, they should be "00000000"
	if (strcmp(CounterValUUT, RESULT_00) != 0) iStatus = 1;
	if (strcmp(CounterValCAP, RESULT_00) != 0) iStatus = 1;
	
	// compare the uut Tx/Rx to the Captive Rx/Tx, they should be equal
	if (strcmp(CounterValTx1, CounterValTx2) != 0) iStatus = 1;
	if (strcmp(CounterValRx1, CounterValRx2) != 0) iStatus = 1;
	
	return iStatus;
}

int GetReadyForRocketIO()
{
	int iStatus = 0;
	char cmdStr[50] = "\0";
	
	Try (OpenConnectionRS232(ASMCAP)); // open a virtual com port connection to the DPPM

	if (ConnectToASMPort(hHandle4RS232) == 0)   // go the asm cli prompt
	{
		if (GoToLinuxPrompt(hHandle4RS232) == 0)	   // go to the goshell prompt
		{
			// initialize the get ready for rocket io test
			sprintf(cmdStr,"/cs/bin/getreadyforriotest1.sh\r");
			if (WriteReadUntil(hHandle4RS232, gcReadBuffer, cmdStr, gsGoshellPrompt, 15) != 0) 
				iStatus = 1;
		}
		GoToCliPrompt(hHandle4RS232); // logout from Goshell
	
		LogOutFromCli(hHandle4RS232); // logout from the CLI
	}
	CloseConnection(hHandle4RS232); // close the captive asm com port
exit:
	return iStatus;
}

/*
 *	UutTrafficTest()
 *
 *  this function assigns a static IP address to the system and then opens a WMI browser for the
 *  operator to set up the uut for the traffic test.
 *  A set of instructions is also opened in the browser to set up and start the Ixia traffic test.
 */
int UutTrafficTest()
{
	int res, webPanel, iStatus = 0;
	char cWmiUrl[20] = "\0";
	char cPort[75] = "\0";
	char cRoute[50] = "\0";
	char cAuthHost[75] = "\0";
	char testStr[75] = "\0";
	char testStr2[75] = "\0";
	
	giStatus = 0; // clear the global error status register
	
    if ((giWebPanel  = LoadPanel (0, "FatTestGui.uir", TRAFFIC)) < 0) // load the web browser panel
        return -1;
	
	if (giIsDppmGe) sprintf(testStr,"GigE Traffic test.");
	else if (giIsDppmSonet) sprintf(testStr,"Sonet Traffic test.");
	else if (giIsDppm10Ge) sprintf(testStr,"10Ge Traffic test.");
		
	// get the list box line for this test
	GetTreeItemFromLabel (gMainPanel, PANEL_TESTLIST,VAL_ALL,0,VAL_FIRST,VAL_NEXT_PLUS_SELF,0,testStr,&gCurListBoxLine);
	
	SetTreeCellAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, 1, ATTR_LABEL_COLOR, VAL_BLUE);
	SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, -1); 	
	
	if (giIsDppmGe)
	{
		sprintf(gsInstrPath,"C:\\Cloudshield\\DPPM\\GigE\\Functional - DPPM - Fiber Ports GigE Traffic Test.htm");
		sprintf(gsInstrPath2,"C:\\Cloudshield\\Functional\\DPPM\\GigE\\Functional - DPPM - Copper Ports GigE Traffic Test.htm");
		SetPanelAttribute(giWebPanel, ATTR_TITLE,"UUT GigE Traffic Test - Instructions and WMI Browser");
		sprintf(testStr2,"\n########## DPPM GigE Capture Port Traffic Test ##########\n"); // display completed status
		SetTestStatus(VAL_BLUE, "Start GigE Traffic test");
	}
	else if (giIsDppmSonet)
	{
		sprintf(gsInstrPath,"C:\\Cloudshield\\DPPM\\Sonet\\Functional - DPPM - Fiber Ports OC12 Traffic Test.htm");
		SetPanelAttribute(giWebPanel, ATTR_TITLE,"UUT Sonet Traffic Test - Instructions and WMI Browser");
		sprintf(testStr2,"\n########## DPPM Sonet Capture Port Traffic Test ##########\n"); // display completed status
		SetTestStatus(VAL_BLUE, "Start Sonet Traffic test");
	}
	else if (giIsDppm10Ge)
	{
		sprintf(gsInstrPath,"C:\\Cloudshield\\DPPM\\10GigE\\Functional - DPPM - 10GigE Ports Traffic Test.htm");
		SetPanelAttribute(giWebPanel, ATTR_TITLE,"UUT 10GigE Traffic Test - Instructions and WMI Browser");
		sprintf(testStr,"UUT DPPM 10GigE Traffic test."); // display completed status
		sprintf(testStr2,"\n########## DPPM 10GigE Capture Port Traffic Test ##########\n"); // display completed status
		SetTestStatus(VAL_BLUE, "Start 10Ge Traffic test");
	}
	// place a check mark next to the current test
	CheckListItem(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, 1); 
	
	WriteFile (giLogFileHandle,testStr2,StringLength(testStr));
	// get the test station wmi url from the ini file
	Ini_GetStringIntoBuffer(gIniText,"WMI","URL", gsWmiPath, sizeof(gsWmiPath));
	
//	Try (OpenConnectionRS232(DPPMCAP)); // open the com port
	
//	Try (ConnectToASMPort(hHandle1Telnet));	// connect to the asm serial port and go to the cli prompt
	
//	HidePanel(gMainPanel); // hide the main panel when opening the wmi
	
    //Get Object Handle from ActiveX control and load initial page
    GetObjHandleFromActiveXCtrl (giWebPanel, TRAFFIC_WEBBROWSER, &webHandle);
	
    //Navigate to the wmi browser first
    INET_IWebBrowser2Navigate (webHandle, NULL, gsWmiPath, CA_DEFAULT_VAL, CA_DEFAULT_VAL, CA_DEFAULT_VAL, CA_DEFAULT_VAL);

    //Install event handler for navigate complete event from web browser
    INET_DWebBrwsrEvnts2RegOnNavigateComplete2 (webHandle, NavigateComplete_CB, NULL, 1, NULL);
	
	DisplayPanel (giWebPanel);

	giButtonPressFlg = 0; // clear the button press flag
	
	while (!giButtonPressFlg)    // wait here until either the Pass or Fail button is pressed
		ProcessSystemEvents();
	
  	//	Discard panel after it has been closed. 
	DiscardPanel(giWebPanel);
		
	if (giStatusFlg == 0) iStatus = 1; // see if the pass button was pressed, if not set the error flag
	
	giStatusFlg = 0; // clear the pass/fail status flag
	
	if (giIsDppmGe) // if a Ge board, then run the copper traffic test
	{
		strcpy(gsInstrPath, gsInstrPath2); // copy the copper traffic test instructions
	
		DisplayPanel (giWebPanel);

		giButtonPressFlg = 0; // clear the button press flag
	
		while (!giButtonPressFlg)    // wait here until either the Pass or Fail button is pressed
			ProcessSystemEvents();
	
	  	//	Discard panel after it has been closed. 
		DiscardPanel(giWebPanel);
		
		if (giStatusFlg == 0) iStatus = 1; // see if the pass button was pressed, if not set the error flag
	}
	DisplayPanel(gMainPanel); // display the main gui
	
	giStatusFlg = 0; // clear the pass/fail status flag
exit:	
//	LogOutFromCli(TELNET); // logout from the CLI
	
//	CloseConnection(hHandle1RS232, DPPMCAP); // close com port
	
	if (giStatus) // check if global error status error flag is set
		iStatus = 1;
	
	// display pass or fail and save the test result  
	if (iStatus)
	{
		strcpy(gsTestStatus.cUutTraffic,"Fail");
		SetTestStatus(VAL_RED, "Fail");
		SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, 4); 	
		strcat(gsComment,"UUT Traffic, ");
	}
	else
	{   
		strcpy(gsTestStatus.cUutTraffic,"Pass");
		SetTestStatus(VAL_DK_GREEN, "Pass");
		SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, 1); 	
	}	
	return giStatus;
}

/*
 *	CapturePortTest()
 *
 *  this function assigns a static IP address to the system and then opens a WMI browser for the
 *  operator to set up the uut for the traffic test.
 *  A set of instructions is also opened in the browser to set up and start the Ixia traffic test.
 */
int CapturePortTest()
{
	int res, webPanel, iStatus = 0;
	char cWmiUrl[20] = "\0";
	char cPort[75] = "\0";
	char cRoute[50] = "\0";
	char cAuthHost[75] = "\0";
	char testStr[75] = "\0";
	char testStr2[75] = "\0";
	char cmdStr[50] = "\0";
	
	giStatus = 0; // clear the global error status register
	
    if ((giWebPanel  = LoadPanel (0, "FatTestGui.uir", TRAFFIC)) < 0) // load the web browser panel
        return -1;
	
	if (giIsDppmGe) sprintf(testStr,"GigE Capture Port Traffic test.");
	else if (giIsDppmSonet) sprintf(testStr,"Sonet Capture Port Traffic test.");
	else if (giIsDppm10Ge) sprintf(testStr,"10Ge Capture Port Traffic test.");
		
	// get the list box line for this test
	GetTreeItemFromLabel (gMainPanel, PANEL_TESTLIST,VAL_ALL,0,VAL_FIRST,VAL_NEXT_PLUS_SELF,0,testStr,&gCurListBoxLine);
	// place a check mark next to the current test
	CheckListItem(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, 1);
	
	SetTreeCellAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, 1, ATTR_LABEL_COLOR, VAL_BLUE);  
	SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, -1); 	
	
	if (giIsDppmGe)
	{
		sprintf(gsInstrPath,"C:\\Cloudshield\\DPPM\\Functional - DPPM - GigE Capture Port Traffic Test.htm");
		SetPanelAttribute(giWebPanel, ATTR_TITLE,"DPPM-510 GigE Capture Port Traffic Test -  Instructions and WMI Browser");
		sprintf(testStr2,"\n########## DPPM GigE Capture Port Traffic Test ##########\n"); // display completed status
		SetTestStatus(VAL_BLUE, "Start GigE Capture Port test");
	}
	if (giIsDppmSonet)
	{
		sprintf(gsInstrPath,"C:\\Cloudshield\\DPPM\\Functional - DPPM - Sonet Capture Port Traffic Test.htm");
		SetPanelAttribute(giWebPanel, ATTR_TITLE,"DPPM-600 Sonet Capture Port Traffic Test -  Instructions and WMI Browser");
		sprintf(testStr2,"\n########## DPPM Sonet Capture Port Traffic Test ##########\n"); // display completed status
		SetTestStatus(VAL_BLUE, "Start Sonet Capture Port test");
	}
	else if (giIsDppm10Ge)
	{
		sprintf(gsInstrPath,"C:\\Cloudshield\\DPPM\\Functional - DPPM - 10GigE Capture Port Traffic Test.htm");
		SetPanelAttribute(giWebPanel, ATTR_TITLE,"DPPM-800 10Ge Capture Port Traffic Test -  Instructions and WMI Browser");
		sprintf(testStr2,"\n########## DPPM 10Ge Capture Port Traffic Test ##########\n"); // display completed status
		SetTestStatus(VAL_BLUE, "Start 10Ge Capture Port test");
	}
	WriteFile (giLogFileHandle,testStr,StringLength(testStr));

	// get the test station wmi url from the ini file
	Ini_GetStringIntoBuffer(gIniText,"WMI","URL", gsWmiPath, sizeof(gsWmiPath));
	
	HidePanel(gMainPanel); // hide the main panel when opening the wmi
	
    //Get Object Handle from ActiveX control and load initial page
    GetObjHandleFromActiveXCtrl (giWebPanel, TRAFFIC_WEBBROWSER, &webHandle);
	
    //Navigate to the wmi browser first
    INET_IWebBrowser2Navigate (webHandle, NULL, gsWmiPath, CA_DEFAULT_VAL, CA_DEFAULT_VAL, CA_DEFAULT_VAL, CA_DEFAULT_VAL);

    //Install event handler for navigate complete event from web browser
    INET_DWebBrwsrEvnts2RegOnNavigateComplete2 (webHandle, NavigateComplete_CB, NULL, 1, NULL);
	
	DisplayPanel (giWebPanel); // display the test wmi display panel

	giButtonPressFlg = 0; // clear the button press flag
	
	while (!giButtonPressFlg)  // wait here until either the Pass or Fail button is pressed
		ProcessSystemEvents();
	
  	//	Discard panel after it has been closed. 
	DiscardPanel(giWebPanel);
		
	if (giStatusFlg == 0) iStatus = 1; // see if the pass button was pressed, if not set the error flag
	
	DisplayPanel(gMainPanel); // display the main gui
	
	giStatusFlg = 0; // clear the pass/fail status flag
exit:	
	if (giStatus) // check if global error status error flag is set
		iStatus = 1;
	
	// display pass or fail and save the test result  
	if (iStatus)
	{
		strcpy(gsTestStatus.cCapturePort,"Fail");
		SetTestStatus(VAL_RED, "Fail");
		SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, 4); 	
		strcat(gsComment,"Capture Port, ");
	}
	else
	{   
		strcpy(gsTestStatus.cCapturePort,"Pass");
		SetTestStatus(VAL_DK_GREEN, "Pass");
		SetTreeItemAttribute(gMainPanel, PANEL_TESTLIST, gCurListBoxLine, ATTR_IMAGE_INDEX, 1); 	
	}	
	return giStatus;
}

    
// This function is called when the browser completes the load
// of a specific location
HRESULT CVICALLBACK NavigateComplete_CB (CAObjHandle caServerObjHandle, void *caCallbackData, 
									     CAObjHandle  pDisp, VARIANT *URL)
{
	char* URLstring = malloc(100);
	
	if (giWmiFlg)
	{
	    SetActiveCtrl (giWebPanel, TRAFFIC_WEBBROWSER);
	    CA_VariantGetCString (URL, &URLstring);
		strcpy(gsWmiPath, URLstring);
	    CA_FreeMemory(URLstring);
	}
    return 0;
}

int CVICALLBACK OpenWmiBrowser_CB (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		    //Navigate to the wmi browser first
			giWmiFlg = 1; // set the wmi browser flag
		    INET_IWebBrowser2Navigate (webHandle, NULL, gsWmiPath, CA_DEFAULT_VAL, CA_DEFAULT_VAL, CA_DEFAULT_VAL, CA_DEFAULT_VAL);
			break;
	}
	return 0;
}

int CVICALLBACK OpenInstructionHtmlPage_CB (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		    //Navigate to the wmi browser first
			giWmiFlg = 0; // clear the wmi browser flag
		    INET_IWebBrowser2Navigate (webHandle, NULL, gsInstrPath, CA_DEFAULT_VAL, CA_DEFAULT_VAL, CA_DEFAULT_VAL, CA_DEFAULT_VAL);
			break;
	}
	return 0;
}

int CVICALLBACK PassFailTrafficTest_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			giButtonPressFlg = 1; // set the button pressed flag
			if (control == TRAFFIC_PASS)
				giStatusFlg = TRUE; // sets the flag when the pass button is clicked
			break;
	}
	return 0;
}

/*
 *	RedbootReboot()
 *
 *  this function cycles the power to the uut and waits for the redboot prompt
 */
int RedbootReboot()
{
	char* cRedBootKeyPattern = "Enter ^C to abort";

	giStatus = 0; // clear the error status flag		
	
	ViOpenXantrex(); // open a com session to both power supplies
	
	Try (OpenConnectionRS232(DPPM)); // open a virtual com port connection to the DPPM
	
	PowerCycle(); // do a power cycle
	
	Try (WriteReadUntil(hHandle1RS232, gcReadBuffer,"", cRedBootKeyPattern, 30));
exit:
	ViCloseXantrex(); // close the sessions to the power supplies
	
	CloseConnection(hHandle1RS232); // close the virtual telnet com port
	
	return giStatus;
}

/*
 *	GetProductCode()
 *  this function gets the product code based on the uut dppm type
 */
char* GetProductCode()
{
	char cProductCode[15] = "\0";
	char* cPCode = "              \0";
	
	if (giIsDppmGe)
		Ini_GetStringIntoBuffer(gIniText,"Product Code","DPPM510", cProductCode, sizeof(cProductCode));
	else if (giIsDppmSonet)
		Ini_GetStringIntoBuffer(gIniText,"Product Code","DPPM600", cProductCode, sizeof(cProductCode));
	else if (giIsDppm10Ge)
		Ini_GetStringIntoBuffer(gIniText,"Product Code","DPPM800", cProductCode, sizeof(cProductCode));
	
	strcpy(cPCode, cProductCode);
	
	return cPCode;
}

/*
 *	GetHardwareOption()
 *  this function gets the hardware options based on the uut dppm type
 */
void GetHardwareOption()
{
	char cHWOption[10] = "\0";
	
	if (giIsDppmGe)
	{
		Ini_GetStringIntoBuffer(gIniText,"HW Options","DPPM510", gcTmmHWOpt, sizeof(gcTmmHWOpt));
		Ini_GetStringIntoBuffer(gIniText,"HW Options","SDB510",  gcSdbHWOpt, sizeof(gcSdbHWOpt));
		Ini_GetStringIntoBuffer(gIniText,"HW Options","NIM510",  gcNimHWOpt, sizeof(gcNimHWOpt));
	}
	else if (giIsDppmSonet)
	{
		Ini_GetStringIntoBuffer(gIniText,"HW Options","DPPM600", gcTmmHWOpt, sizeof(gcTmmHWOpt));
		Ini_GetStringIntoBuffer(gIniText,"HW Options","SDB600",  gcSdbHWOpt, sizeof(gcSdbHWOpt));
		Ini_GetStringIntoBuffer(gIniText,"HW Options","NIM600",  gcNimHWOpt, sizeof(gcNimHWOpt));
	}
	else if (giIsDppm10Ge)
	{
		Ini_GetStringIntoBuffer(gIniText,"HW Options","DPPM800", gcTmmHWOpt, sizeof(gcTmmHWOpt));
		Ini_GetStringIntoBuffer(gIniText,"HW Options","SDB600",  gcSdbHWOpt, sizeof(gcSdbHWOpt));
		Ini_GetStringIntoBuffer(gIniText,"HW Options","NIM800",  gcNimHWOpt, sizeof(gcNimHWOpt));
	}
}

byte* InitHeaderPage(byte* bData)
{
	int iDataPointer = 0;  // reset the pointer to 0 to store the magic number and the crc value
	word wCRC = 0;
	byte bPageBytes[CSPAGE_BYTE_SIZE];
	
	// store the magic number to 1st 2 byte locations	
	bData[iDataPointer] = MAGIC_NUMBER >> 8; 
	iDataPointer++;	
	bData[iDataPointer] = MAGIC_NUMBER & 0xFF;
	iDataPointer++;
	
	// copy 124 bytes of data starting from offset address 4 to the data byte buffer starting at location 0	
	CopyBytes(bPageBytes, 0, bData, CRC_CALC_START_OFFSET, 124);
	
	// calculate and store the crc value	
	wCRC = CalculateCRC(bPageBytes, 124);
	bData[iDataPointer] = wCRC >> 8;  
	iDataPointer++;	
	bData[iDataPointer] = wCRC & 0xFF;
	
	return bData;
}

int CVICALLBACK OkRio_CB (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			RemovePopup(giRocketIoPanel);
			break;
	}
	return 0;
}

int CVICALLBACK OkVProbe_CB (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			RemovePopup(giVisionProbePanel);
			break;
	}
	return 0;
}

int CVICALLBACK LoadData_CB (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			GetUutValsFromFile(); // load a specific uut data file
			break;
	}
	return 0;
}
