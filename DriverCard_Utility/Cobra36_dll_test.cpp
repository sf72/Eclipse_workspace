/*
  Name:
  Copyright:
  Author:
  Date: 22/11/06 11.42 iniziato
  Description: utilizzo di dll senza libreria .a

  recupera questo: http://it.rs-online.com/web/p/kit-di-sviluppo-per-processori-e-microcontrollori/7154081/
  documenta example: http://arduino.cc/en/Tutorial/SwitchCase2
*/

//--------- #DEFINE  ---------//

//---------
//--------- #INCLUDE FILES ---------//
#include <iostream>    //.h backword warning because old
#include  <iomanip>	//per cout setfill & setw
#include <windows.h>
#include "ni488.h"	//gpib header 'SERVE !!'
// #include "cobra36.h"
//---------
using namespace std;



//--------- GLOBAL VARIABLES ---------//
HANDLE hCOM;	//handle alla porta com
char articolo[13]="------------"; //es.: 3T2791BAG-xx 12char

struct limits_struct {
  int mA_off_L;
  int mA_off_H;
  int mA_tx_L;
  int mA_tx_H;
  int Khz_L;
  int Khz_H;
  int dbm_L;
  int dbm_H;
} limit;

//---------

//--------- PROTO FUNCTIONS ---------//
void presentation (void);						//CRT anagrafica codice
void menu(char *); //riporta in char* l'articolo scelto
int read_limits(limits_struct* limit);	//passo il pointer alla struct per caricare i limits da file ini
int cobra36_decrypt(void);	//decripta il frame ricevuto dalla dc
int gpib_spettro(void);	//legge il valore di potenza e frequenza dal GPIB interface
int com_init (HANDLE*); //init della porta com x arduino
//---------



int main (void)
{

	presentation();
	menu(articolo);
		cout <<"Articolo scelto: " << articolo<<endl;
	read_limits(&limit);	//struct pointer
		cout <<"solo x test dbm_H="<<limit.dbm_H<<endl;
	com_init(&hCOM);
	/*
	sub wait fixt close > arduino tx serial.print
	delay
	sub insert shunt  > arduino tx relè shunt ON [open]
	sub read AD > arduino tx<>rx save corrente a vuoto ->msgbox operatore premi il pulsante
	sub wait read press key arduino rx pulsante ok AD?
	sub retain pulsante close by relè close comando digitale arduino.
	comando readAD in trasmissione sulla shunt.
	comando rilascia relè per cortocircuitare la shunt NC.
	sub read  AD per lettura tensione su led (acceso)
	comando pull-up 3V per blocco modulatore uscita digitale su relè NO
	sub gpib_spettro per lettura freq&power
	pull-up modulatore off.
	comando rilascio relè pulsante off.
	attesa 1° frame trasmessa dopo 3sec circa
	attesa 2°frame trasmessa
	confronto se fix code uguali pass.(fix code lo chiama ???).
	pass ->oggettivazione

	P.S.:il led viene testato solo durante la pressione conntinua del pulsante

	int cmd_arduino(char*) if no error return 0 char* =comando inviato
	int arduino_repply(char*) if no error return 0 scrive in char* la risposta di ardu dalla seriale.

	RELE':
	1-pws 3V
	2-EV oggettivazione +12V
	3-pulsante
	4-pullup modulatore

	 */
	cobra36_decrypt();
	gpib_spettro();
	CloseHandle(hCOM);
}



int cobra36_decrypt(void) {

/*
 * 260914: sub collaudata  su .vi labview cobra36-decrypt
 *payload= aabbccdd100807ff decrypt= 9f c0 70 32 82 95 7 ff
 *payload= 010203040A0B0C0D decrypt= 9 60 9a 60 12 60 c d
 *OK!!!
 *
*/

#define MAXMODULE 100
//#define PAYLOAD_DBG 	0xaa, 0xbb, 0xcc, 0xdd, 0x10, 0x8, 0x7, 0xff
#define PAYLOAD_DBG 	0x01, 0x02, 0x03, 0x04, 0x0A, 0x0B, 0x0C, 0x0D

typedef void (WINAPI*cfunc)();
typedef void (WINAPI*ifunc)(uint8_t*);	//puntatore a funct con argomenti

ifunc setFrame;
cfunc aceDecrypt;
ifunc getFrame;

	HINSTANCE hLib=LoadLibrary("cobra36.DLL");

       if(hLib==NULL) {

            cout << "Unable to load library!" << endl;
            cin.get();
            return(EXIT_SUCCESS);
       }

       char mod[MAXMODULE];

       GetModuleFileName((HMODULE)hLib, (LPTSTR)mod, MAXMODULE);
       cout << "Library loaded: " << mod << endl;


       setFrame=(ifunc)GetProcAddress((HMODULE)hLib, "setFrame");
       aceDecrypt=(cfunc)GetProcAddress((HMODULE)hLib, "aceDecrypt");
       getFrame=(ifunc)GetProcAddress((HMODULE)hLib, "getFrame");


       if((setFrame==NULL) || (aceDecrypt==NULL) || (getFrame==NULL) ) {

            cout << "Unable to load function(s)." << endl;

            FreeLibrary((HMODULE)hLib);
            return(EXIT_SUCCESS);
       }



       uint8_t  rx_frame[]= {PAYLOAD_DBG};

       setFrame(rx_frame);	//cobra36.DLL
       aceDecrypt();		//cobra36.DLL
       getFrame(rx_frame);	//cobra36.DLL

       for(int n=0;n<8;n++)
       	   {
    	   	   std::cout<<std::setfill('0') << setw(2)<< hex << (int)rx_frame[n] <<' '; //(uint8_t)rx_frame[0]-> per far funzionare std::hex
       	   }
       cout<<'\n';

       FreeLibrary((HMODULE)hLib);

       //cin.get(); //input char to continue
       return (EXIT_SUCCESS);
 }


int gpib_spettro(void)
{

	/* Filename - Dll4882query.c
	 *
	 *  This sample program is is comprised of three basic parts:
	 *
	 *  1. Initialization
	 *  2. Main Body
	 *  3. Cleanup
	 *
	 *  The Initialization portion consists of initializing the bus and the
	 *  GPIB interface board so that the GPIB board is Controller-In-Charge
	 *  (CIC). Next it finds all the listeners and then clears all the
	 *  devices on the bus.
	 *
	 *  In the Main Body, this application queries all devices for their
	 *  identification codes by issuing the '*IDN?' command. Many
	 *  instruments respond to this command with an identification string.
	 *  Note, 488.2 compliant devices are required to respond to this
	 *  command.
	 *
	 *  The last step, Cleanup, takes the board offline.
	 *
	 */

	//#include <stdio.h>
	//#include <stdlib.h>
	//#include <conio.h>

	/*
	 *  Include the WINDOWS.H and NI488.H files. The standard Windows
	 *  header file, WINDOWS.H, contains definitions used by NI488.H and
	 *  NI488.H contains prototypes for the GPIB routines and constants.
	 */
	//#include <windows.h>
	//#include "ni488.h"

	#define ARRAYSIZE  100                 // Size of read buffer
	#define GPIB0        0                 // Board handle



	int        loop,                       // Loop counter
	           Num_Listeners;              // Number of listeners on GPIB
	Addr4882_t Instruments[32],            // Array of primary addresses
	           Result[31];                 // Array of listen addresses
	char       ReadBuffer[ARRAYSIZE + 1];  // Read data buffer
	char ErrorMnemonic[29][5] = { "EDVR", "ECIC", "ENOL", "EADR", "EARG",
	                              "ESAC", "EABO", "ENEB", "EDMA", "",
	                              "EOIP", "ECAP", "EFSO", "",     "EBUS",
	                              "ESTB", "ESRQ", "",     "",      "",
	                              "ETAB", "ELCK", "EARM", "EHDL",  "",
	                              "",     "EWIP", "ERST", "EPWR" };

	int success=TRUE; //success=FALSE -> FAIL ; success=TRUE -> PASS

	//void GPIBCleanup(int ud, char* ErrorMsg); modificata e inclusa nel codice con goto


	/****************************************
	 ********  CODE TO ACCESS GPIB-32.DLL
	 ***********************************************/

	static void (__stdcall *PDevClearList)(int boardID, Addr4882_t * addrlist);
	static void (__stdcall *PFindLstn)(int boardID, Addr4882_t * addrlist, PSHORT results, int limit);
	static int  (__stdcall *Pibonl)(int ud, int v);
	static void (__stdcall *PReceive)(int boardID, Addr4882_t addr, PVOID buffer, LONG cnt, int Termination);
	static void (__stdcall *PSendList)(int boardID, Addr4882_t * addrlist, PVOID databuf, LONG datacnt, int eotMode);
	static void (__stdcall *PSendIFC)(int boardID);

	/*
	 *    This is private data for the language interface only so it is
	 *    defined as 'static'.
	 */
	static HINSTANCE Gpib32Lib = NULL;
	static int *Pibsta;
	static int *Piberr;
	static long *Pibcntl;


	   /*
	    *  Call LoadLibrary to load the 32-bit GPIB DLL.  Save the handle
	    *  into the global 'Gpib32Lib'.
	    */
	   Gpib32Lib = LoadLibrary ("GPIB-32.DLL");

	   if (!Gpib32Lib)  {
	      /*
	       *    The LoadLibrary call failed, return with an error.
	       */
		  cout << "Unable to correctly access the 32-bit GPIB DLL.\n"<< endl;
	      success=FALSE;
		  return success;	//se non apre la dll esce senza far altro
	   }


	   /*
	    *    OK, the GPIB library is loaded.  Let's get a pointer to the
	    *    requested function.  If the GetProcAddress call fails, then
	    *    return with an error.
	    */
	   Pibsta          = (int *) GetProcAddress(Gpib32Lib, (LPCSTR)"user_ibsta");
	   Piberr          = (int *) GetProcAddress(Gpib32Lib, (LPCSTR)"user_iberr");
	   Pibcntl         = (long *)GetProcAddress(Gpib32Lib, (LPCSTR)"user_ibcnt");

	   PDevClearList   = (void (__stdcall *)(int, Addr4882_t *))GetProcAddress(Gpib32Lib, (LPCSTR)"DevClearList");
	   PFindLstn       = (void (__stdcall *)(int, Addr4882_t *, PSHORT, int))GetProcAddress(Gpib32Lib, (LPCSTR)"FindLstn");
	   Pibonl          = (int  (__stdcall *)(int, int))GetProcAddress(Gpib32Lib, (LPCSTR)"ibonl");
	   PReceive        = (void (__stdcall *)(int, Addr4882_t, PVOID, LONG, int))GetProcAddress(Gpib32Lib, (LPCSTR)"Receive");
	   PSendList       = (void (__stdcall *)(int, Addr4882_t *, PVOID, LONG, int))GetProcAddress(Gpib32Lib, (LPCSTR)"SendList");
	   PSendIFC        = (void (__stdcall *)(int))GetProcAddress(Gpib32Lib, (LPCSTR)"SendIFC");


	   if ((Pibsta         == NULL) ||
	       (Piberr         == NULL) ||
	       (Pibcntl        == NULL) ||
	       (PDevClearList  == NULL) ||
	       (PFindLstn      == NULL) ||
	       (Pibonl         == NULL) ||
	       (PReceive       == NULL) ||
	       (PSendList      == NULL) ||
	       (PSendIFC       == NULL))  {

	      FreeLibrary (Gpib32Lib);
	      Gpib32Lib = NULL;
	      cout <<"Impossibile trovare l'entry point delle funct nella dll"<<endl;
	      success=FALSE;
	      return(success);	//se non apre la dll esce senza far altro
	   }

	  /* end of LoadDll */

	for (short uno=0; uno<=0; uno++)	//loop 1 sola volta solo per utilizzare i break per andare al cleanup!!
	{

/*
	static void FreeDll (void)
	{
	   FreeLibrary (Gpib32Lib);
	   Gpib32Lib = NULL;
	   return;
	}


	int __cdecl main() {

	    if (!LoadDll())  {
	       printf ("Unable to correctly access the 32-bit GPIB DLL.\n");
	       return 1;
	    }
*/
	/* ====================================================================
	 *
	 *  INITIALIZATION SECTION
	 *
	 * ====================================================================
	 */

	/*
	 *  Your board needs to be the Controller-In-Charge in order to find all
	 *  listeners on the GPIB.  To accomplish this, the function SendIFC is
	 *  called. If the error bit ERR is set in ibsta, call GPIBCleanup with
	 *  an error message.
	 */

	    (*PSendIFC)(GPIB0);
	    if ((*Pibsta) & ERR)
	    {
	       cout<< "Unable to open board"<<endl;
           success=FALSE;
           break; //esce dal for (uno).
	    }

	/*
	 *  Create an array containing all valid GPIB primary addresses,
	 *  except for primary address 0.  Your GPIB interface board is at
	 *  address 0 by default.  This array (Instruments) will be given to
	 *  the function FindLstn to find all listeners.  The constant NOADDR,
	 *  defined in NI4882.H, signifies the end of the array.
	 */

	    for (loop = 0; loop < 30; loop++) {
	       Instruments[loop] = (Addr4882_t)(loop + 1);
	    }
	    Instruments[30] = NOADDR;

	/*
	 *  Print message to tell user that the program is searching for all
	 *  active listeners.  Find all of the listeners on the bus.  Store
	 *  the listen addresses in the array Result. If the error bit ERR is
	 *  set in ibsta, call GPIBCleanup with an error message.
	 */

	    cout <<"Finding all listeners on the bus..." << endl;


	    (*PFindLstn)(GPIB0, Instruments, Result, 31);
	    if ((*Pibsta) & ERR)
	    {
	       cout<< "Unable to issue FindLstn call" <<endl;
           success=FALSE;
           break; //esce dal for (uno).
	    }

	/*
	 *  ibcntl contains the actual number of addresses stored in the
	 *  Result array. Assign the value of ibcntl to the variable
	 *  Num_Listeners. Print the number of listeners found.
	 */

	    Num_Listeners = (short)(*Pibcntl);

	    cout << "Number of instruments found = " << Num_Listeners <<endl;

	/*
	 *  The Result array contains the addresses of all listening devices
	 *  found by FindLstn. Use the constant NOADDR, as defined in
	 *  NI4882.H, to signify the end of the array.
	 */

	    Result[Num_Listeners] = NOADDR;

	/*
	 *  DevClearList will send the GPIB Selected Device Clear (SDC)
	 *  command message to all the devices on the bus.  If the error bit
	 *  ERR is set in ibsta, call GPIBCleanup with an error message.
	 */

	    (*PDevClearList)(GPIB0, Result);
	    if ((*Pibsta) & ERR)
	    {
	       cout<< "Unable to clear devices"<<endl;
           success=FALSE;
           break; //esce dal for (uno).
	    }

	/* ====================================================================
	 *
	 *  MAIN BODY SECTION
	 *
	 *  In this application, the Main Body communicates with the
	 *  instruments by writing a command to them and reading the individual
	 *  responses. This would be the right place to put other instrument
	 *  communication.
	 *
	 * ====================================================================
	 */

	/*
	 *  Send the identification query to each listen address in the array
	 *  (Result) using SendList.  The constant NLend, defined in NI4882.H,
	 *  instructs the function SendList to append a linefeed character
	 *  with EOI asserted to the end of the message.  If the error bit ERR
	 *  is set in ibsta, call GPIBCleanup with an error message.
	 */
	    char buffer[]="*IDN?";	//buffer for gpib msg by s.f.

	    (*PSendList)(GPIB0, Result, buffer, 5L, NLend);
	    if ((*Pibsta) & ERR)
	    {
	       cout<< "Unable to write to devices"<<endl;
           success=FALSE;
           break; //esce dal for (uno).
	    }

	/*
	 *  Read each device's identification code, one at a time.
	 *
	 *  Establish a FOR loop to read each one of the device's
	 *  identification code. The variable LOOP will serve as a counter
	 *  for the FOR loop and as the index to the array Result.
	 */

	    for (loop = 0; loop < Num_Listeners; loop++)
	    {
	       /*
	        *  Read the name identification response returned from each
	        *  device. Store the response in the array ReadBuffer.  The
	        *  constant STOPend, defined in NI4882.H, instructs the
	        *  function Receive to terminate the read when END is detected.
	        *  If the error bit ERR is set in ibsta, call GPIBCleanup with
	        *  an error message.
	        */

	           (*PReceive)(GPIB0, Result[loop], ReadBuffer, ARRAYSIZE, STOPend);
	           if ((*Pibsta) & ERR)
	           {
	              cout <<  "Unable to read from a device" <<endl;	//GPIBCleanup(GPIB0, "Unable to read from a device");
	              success=FALSE;
	              break; //esce dal for (uno).
	           }

	       /*
	        *  Assume that the returned string contains ASCII data. NULL
	        *  terminate the string using the value in ibcntl which is
	        *  the number of bytes read in. Use printf to display the
	        *  string.
	        */

	           ReadBuffer[(*Pibcntl)] = '\0';
	           cout << "Returned string:" << ReadBuffer << endl;

	    }      /*  End of FOR loop */

	/* ====================================================================
	 *
	 *  CLEANUP SECTION
	 *
	 * ====================================================================
	 */

	} //end for (uno) di un solo ciclo per indirizzare qui i break.


	/*  Take the board offline.                                                 */

	    (*Pibonl) (GPIB0, 0);
	    cout <<"Cleanup: Taking board offline" << endl;

	    //FreeDll();//sistemare

	      FreeLibrary (Gpib32Lib);
	      Gpib32Lib = NULL;
	      cout <<"Unload dll" << endl;

	      if (!success)
	      	  {
	    	  	  //cout <<"Error:" <<ErrorMsg<<endl; //scrive sopra
	    	  	  cout <<"ibsta =0x"<<std::hex<<(int)*Pibsta<<"iberr ="<<std::dec<<(int)*Piberr<<"("<<ErrorMnemonic[((int)*Piberr)]<<")"<<endl;
	      	  }

	/*0
	 *  After each GPIB call, the application checks whether the call
	 *  succeeded. If an NI-488.2 call fails, the GPIB driver sets the
	 *  corresponding bit in the global status variable. If the call
	 *  failed, this procedure prints an error message, takes the board
	 *  offline and exits.
	 */
	return (success); //success=FALSE -> FAIL ; success=TRUE -> PASS

} //end int gpib_spettro(void)

/*
void GPIBCleanup(int ud, char* ErrorMsg) <<<<<<integrata in  int gpib_spettro(void)

	{

	// setta una flag nel return 1 e fai uscire queste info riportate nella chiamante

	//printf("Error : %s\nibsta = 0x%x iberr = %d (%s)\n",
	    //        ErrorMsg, *Pibsta, *Piberr, ErrorMnemonic[(*Piberr)]);

	    cout <<"Error:" << ErrorMsg << endl;
	    //printf("Cleanup: Taking board offline\n");
	    cout <<"Cleanup: Taking board offline" << endl;

	    //(*Pibonl) (ud, 0);

	    //FreeDll();

	}
*/

int com_init (HANDLE* hCOM_sub)    //configurazione della porta com con diff.boudrate
{

	// Settings per com port
	#define TIMEOUTCOM     1500     //mS
	#define PORT  "\\\\.\\COM1"                  // porta da pc a dut
	#define COM_PAR  "baud=9600 parity=N data=8 stop=1"   // stringa di config COM da caricare in struct DCB

     bool FailFlag = FALSE;
     //HANDLE CreateFile(LPCTSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDistribution, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
     *hCOM_sub = CreateFile(PORT,GENERIC_WRITE|GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_FLAG_NO_BUFFERING,NULL);

    if (*hCOM_sub!=INVALID_HANDLE_VALUE)
       {

       char buffer[35];              //stringa di conf.della com port
       DCB dcb;
       //The ZeroMemory function fills a block of memory with zeros.
       //VOID ZeroMemory(PVOID Destination, address of block to fill with zeros  DWORD Length size, in bytes, of block to fill with zeros);
        ZeroMemory(&dcb,sizeof(DCB));

        strcpy(buffer, COM_PAR);        //è anche possibile caricare la struttura ......
        //verifica struttura dcb
        //BOOL BuildCommDCB(LPCTSTR lpDef,  pointer to device-control string LPDCB lpDCB pointer to device-control block);
        if (BuildCommDCB((char*)&buffer,&dcb))
        	{
        		if (dcb.BaudRate == CBR_9600) cout <<"BuildCommDCB funct. ok" << endl;
        			else {cout << "Errore in BuildCommDCB funct." << endl;
        				FailFlag=TRUE;
        				}
        	}

        //BOOL SetCommState(HANDLE hFile,LPDCB lpDCB ); configures a communications device
       if (SetCommState(*hCOM_sub, &dcb )) cout <<"COM port SetCommState funct. ok" <<endl;
          else  {cout <<"Errore in SetCommState funct" << endl; FailFlag=TRUE;}

        //BOOL SetupComm(HANDLE hFile, DWORD dwInQueue, DWORD dwOutQueue);
        if (!SetupComm(*hCOM_sub,1024,1024)) {cout << "ERRORE in SetupComm funct." << endl; FailFlag=TRUE;}

       } //if (hCOM_sub!=INVALID_HANDLE_VALUE)

         else  {
               cout << "2. ERRORE Open COM" <<endl; //if (hCOM_sub!=INVALID_HANDLE_VALUE)
               FailFlag=TRUE;    //uscita con errore di configurazione
               } //else if (hCOM_sub!=INVALID_HANDLE_VALUE)

       DWORD temp;
       COMSTAT ComState;
       //OVERLAPPED Overlap;

       /* BOOL ClearCommError(
          HANDLE hFile,     // handle to communications device
          LPDWORD lpErrors, // pointer to variable to receive error codes
          LPCOMSTAT lpStat  // pointer to buffer for communications status */

       //ZeroMemory(buf_rd,sizeof(buf_rd));
       ClearCommError(*hCOM_sub, &temp, &ComState); // if temp is not null, the port is in the error state

       //BOOL PurgeComm(HANDLE hFile, DWORD dwFlags); discard all characters from the output or input buffer
       PurgeComm(*hCOM_sub, PURGE_TXCLEAR);
       PurgeComm(*hCOM_sub, PURGE_RXCLEAR);

       /*
       BOOL WINAPI SetCommTimeouts(__in  HANDLE hFile, __in  LPCOMMTIMEOUTS lpCommTimeouts)  ; */
       //http://www.lookrs232.com/com_port_programming/api_timeout.htm
       COMMTIMEOUTS timeouts;

       timeouts.ReadIntervalTimeout = 30; //ex.30 ex.20 sets max period of time (in milliseconds) allowed between two sequential characters being read from the line of commutation.
       timeouts.ReadTotalTimeoutMultiplier = 20; //ex.20 ex.10 sets the multiplier (in milliseconds) used to calculate general timeout of the 'read' operation
       timeouts.ReadTotalTimeoutConstant = TIMEOUTCOM; //ex.100 sets a constant (in milliseconds) used to calculate the general timeout of the operation.
       timeouts.WriteTotalTimeoutMultiplier = 0;
       timeouts.WriteTotalTimeoutConstant = 0;

       if (!SetCommTimeouts(*hCOM_sub, &timeouts)) {cout << "ERRORE in SetCommTimeouts." << endl; FailFlag=TRUE;}

       if (FailFlag) return(0);
    	   else {
				   	   cout << PORT <<"  " << COM_PAR<<endl;
				   	   return(1);//porta configurata ok
    	   	   	   }
}

void presentation (void)
{
	cout << "Built on: Eclipse Compiler: GCC C++ Linker: MinGw" << endl;
	cout << "Source file: " << (__FILE__) << endl;
	cout << "Its compilation began: " << (__DATE__)<<" at: " << (__TIME__) << endl;
	cout << "By: s.f." << endl;
}

void menu(char *articolo)
{

/*
 * per cambiare le scelte modificare #define scelte e aggiungerle nel char pointer
 * es	char* opzioni[scelte]={"3T2781BAG", "-", "-", "-", "-"}; max 10 scelte 0-9
 * il for con lo scan code key può rimanere così
*/
#define SCELTE 1

int tasto; //tasto da 0 a 9
const char* opzioni[SCELTE]={"3T2781BAG"};

	for(tasto=0; tasto<SCELTE; tasto++) cout << tasto <<": "<<opzioni[tasto] <<endl;

	cout <<"*Premi il numero corrispondente all'articolo da collaudare." <<endl;

	bool loop=TRUE;
	while(loop)	//attesa pressione tasto loop
		{
			//SHORT WINAPI GetAsyncKeyState(virtual-key codes) :0x30=key0 - 0x39=key9
			for (tasto=0; tasto<=(SCELTE-1); tasto++) if (GetAsyncKeyState(tasto+0x30)) {loop=FALSE; break;}//scan key code
			Sleep(100); //per cpu load %0
		}//do
	strcpy(articolo,opzioni[tasto]);
}

int read_limits(limits_struct* plimit)	//add argomento articolo
{

	FILE *inifile;

	plimit->dbm_H=10;	//solo per dbg ok lo passa

	return 0;
}

