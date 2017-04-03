/*	altairZ80_sio: MITS Altair serial I/O card
		Written by Peter Schorn, 2001
		Based on work by Charles E Owen ((c) 1997, Commercial use prohibited)

		These functions support a simulated MITS 2SIO interface card.
		The card had two physical I/O ports which could be connected
		to any serial I/O device that would connect to a current loop,
		RS232, or TTY interface. Available baud rates were jumper
		selectable for each port from 110 to 9600.

		All I/O is via programmed I/O. Each device has a status port
		and a data port. A write to the status port can select
		some options for the device (0x03 will reset the port).
		A read of the status port gets the port status:

		+---+---+---+---+---+---+---+---+
		| X	| X | X | X | X | X | O | I |
		+---+---+---+---+---+---+---+---+

		I - A 1 in this bit position means a character has been received
				on the data port and is ready to be read.
		O - A 1 in this bit means the port is ready to receive a character
				on the data port and transmit it out over the serial line.

		A read to the data port gets the buffered character, a write
		to the data port writes the character to the device.
*/

#include <stdio.h>
#include <ctype.h>

#include "altairZ80_defs.h"
#include "sim_sock.h"
#include "sim_tmxr.h"
#include <time.h>

#define	UNIT_V_ANSI			(UNIT_V_UF + 0)				/* ANSI mode, strip bit 8 on output		*/
#define UNIT_ANSI				(1 << UNIT_V_ANSI)
#define	UNIT_V_UPPER		(UNIT_V_UF + 1)				/* uppper case mode										*/
#define UNIT_UPPER			(1 << UNIT_V_UPPER)
#define	UNIT_V_BS				(UNIT_V_UF + 2)				/* map delete to backspace						*/
#define UNIT_BS					(1 << UNIT_V_BS)
#define	UNIT_V_TRAP			(UNIT_V_UF + 3)				/* trap usage of unassigned I/O ports	*/
#define UNIT_TRAP				(1 << UNIT_V_TRAP)

#define	UNIT_V_VERBOSE	(UNIT_V_UF + 0)				/* verbose mode for SIMH pseudo dev		*/
#define UNIT_VERBOSE		(1 << UNIT_V_VERBOSE)

#define Terminals				1											/* lines per mux											*/

TMLN TerminalLines[Terminals] = {	{ 0 } };		/* we only need one line							*/
TMXR altairTMXR = {Terminals, 0, &TerminalLines[0] };	/* mux descriptor							*/

t_stat sio_svc (UNIT *uptr);
t_stat sio_reset (DEVICE *dptr);
t_stat sio_attach (UNIT *uptr, char *cptr);
t_stat sio_detach (UNIT *uptr);
t_stat ptr_svc (UNIT *uptr);
t_stat ptr_reset (DEVICE *dptr);
t_stat ptp_svc (UNIT *uptr);
t_stat ptp_reset (DEVICE *dptr);
int32 nulldev(int32 port, int32 io, int32 data);
int32 sr_dev(int32 port, int32 io, int32 data);
int32 simh_dev(int32 port, int32 io, int32 data);
int32 simh_dev0xfd(int32 port, int32 io, int32 data);
int32 sio0d(int32 port, int32 io, int32 data);
int32 sio0s(int32 port, int32 io, int32 data);
int32 sio1d(int32 port, int32 io, int32 data);
int32 sio1s(int32 port, int32 io, int32 data);
t_stat simh_svc (UNIT *uptr);
void attachCPM();
t_stat simh_dev_reset();
int32 simh_in();
int32 simh_out(int32 data);
void setClockZSDOS();
void setClockCPM3();
time_t mkCPM3Origin();
int32 toBCD(int32 x);
int32 fromBCD(int32 x);

extern t_stat sim_activate (UNIT *uptr, int32 interval);
extern t_stat sim_cancel (UNIT *uptr);
extern t_stat sim_poll_kbd (void);
extern t_stat sim_putchar (int32 out);
extern t_stat attach_unit (UNIT *uptr, char *cptr);
extern t_bool rtc_avail;
extern FILE *sim_log;
extern int32 PCX;
extern int32 sim_switches;
extern uint32 sim_os_msec (void);
extern const char *scp_error_messages[];
extern int32 SR;
extern int32 bankSelect;
extern int32 common;
extern uint8 GetBYTEWrapper(register uint32 Addr);

/* the following variables define state for the SIMH pseudo device */
/* ZSDOS clock definitions */
int32 ClockZSDOSDelta		= 0;	/* delta between real clock and Altair clock									*/
int32 setClockZSDOSPos	= 0;	/* determines state for receiving address of parameter block	*/
int32 setClockZSDOSAdr	= 0;	/* address in M of 6 byte parameter block for setting time		*/
int32 getClockZSDOSPos	= 0;	/* determines state for sending clock information							*/

/* CPM3 clock definitions */
int32 ClockCPM3Delta		= 0;	/* delta between real clock and Altair clock									*/
int32 setClockCPM3Pos		= 0;	/* determines state for receiving address of parameter block	*/
int32 setClockCPM3Adr		= 0;	/* address in M of 5 byte parameter block for setting time		*/
int32 getClockCPM3Pos		= 0;	/* determines state for sending clock information							*/
int32 daysCPM3SinceOrg	= 0;	/* days since 1 Jan 1978																			*/

int32 markTimeSP				= 0;	/* stack pointer for timer stack															*/
int32 versionPos				= 0;	/* determines state for sending device identifier							*/
int32 lastCommand				= 0;	/* most recent command processed on port 0xfeh								*/
int32 getCommonPos			= 0;	/* determines state for sending the 'common' register					*/

/* 2SIO Standard I/O Data Structures */

UNIT sio_unit = { UDATA (&sio_svc, UNIT_ATTABLE, 0),
	KBD_POLL_WAIT };

REG sio_reg[] = {
	{ HRDATA (DATA, sio_unit.buf, 8) },
	{ HRDATA (STAT, sio_unit.u3, 8) },
	{ NULL } };

MTAB sio_mod[] = {
	{ UNIT_ANSI,	0,					"TTY",			"TTY",			NULL },	/* keep bit 8 as is for output						*/
	{ UNIT_ANSI,	UNIT_ANSI,	"ANSI",			"ANSI",			NULL },	/* set bit 8 to 0 before output						*/
	{ UNIT_UPPER,	0,					"ALL",			"ALL",			NULL },	/* do not change case of input characters	*/
	{ UNIT_UPPER,	UNIT_UPPER,	"UPPER",		"UPPER",		NULL },	/* change input characters to upper case	*/
	{ UNIT_BS,		0,					"BS",				"BS",				NULL },	/* map delete to backspace								*/
	{ UNIT_BS,		UNIT_BS,		"DEL",			"DEL",			NULL },	/* map backspace to delete								*/
	{ UNIT_TRAP,	0,					"NOTRAP",		"NOTRAP",		NULL },	/* do not trap unassigned I/O ports				*/
	{ UNIT_TRAP,	UNIT_TRAP,	"TRAP",			"TRAP",			NULL },	/* trap unassigned I/O ports							*/
	{ 0 } };

DEVICE sio_dev = {
	"SIO", &sio_unit, sio_reg, sio_mod,
	1, 10, 31, 1, 8, 8,
	NULL, NULL, &sio_reset,
	NULL, &sio_attach, &sio_detach };

UNIT ptr_unit = { UDATA (&ptr_svc, UNIT_SEQ + UNIT_ATTABLE + UNIT_ROABLE, 0),
	KBD_POLL_WAIT };

REG ptr_reg[] = {
	{ HRDATA (DATA,	ptr_unit.buf,	8)	},
	{ HRDATA (STAT,	ptr_unit.u3,	8)	},
	{ DRDATA (POS,	ptr_unit.pos,	31)	},
	{ NULL } };

DEVICE ptr_dev = {
	"PTR", &ptr_unit, ptr_reg, NULL,
	1, 10, 31, 1, 8, 8,
	NULL, NULL, &ptr_reset,
	NULL, NULL, NULL };

UNIT ptp_unit = { UDATA (&ptp_svc, UNIT_SEQ + UNIT_ATTABLE, 0),
	KBD_POLL_WAIT };

REG ptp_reg[] = {
	{ HRDATA (DATA,	ptp_unit.buf,	8)	},
	{ HRDATA (STAT,	ptp_unit.u3,	8)	},
	{ DRDATA (POS,	ptp_unit.pos,	31)	},
	{ NULL } };

DEVICE ptp_dev = {
	"PTP", &ptp_unit, ptp_reg, NULL,
	1, 10, 31, 1, 8, 8,
	NULL, NULL, &ptp_reset,
	NULL, NULL, NULL };

/*	Synthetic device SIMH for communication
		between Altair and SIMH environment using port 0xfe */
UNIT simh_unit = { UDATA (&simh_svc, 0, 0), KBD_POLL_WAIT };

REG simh_reg[] = {
	{ DRDATA (CZD,		ClockZSDOSDelta,		31)						},
	{ DRDATA (SCZP,		setClockZSDOSPos,		8),		REG_RO	},
	{ DRDATA (GCZP,		getClockZSDOSPos,		8),		REG_RO	},
	{ HRDATA (SCZA,		setClockZSDOSAdr,		17),	REG_RO	},
	{ DRDATA (CC3D,		ClockCPM3Delta,			31)						},
	{ DRDATA (SCC3P,	setClockCPM3Pos,		8),		REG_RO	},
	{ DRDATA (GCC3P,	getClockCPM3Pos,		8),		REG_RO	},
	{ HRDATA (SCC3A,	setClockCPM3Adr,		17),	REG_RO	},
	{ DRDATA (MTSP,		markTimeSP,					8),		REG_RO	},
	{ DRDATA (VP,			versionPos,					8),		REG_RO	},
	{ DRDATA (CP,			getCommonPos,				8),		REG_RO	},
	{ DRDATA (LC,			lastCommand,				8),		REG_RO	},
	{ NULL } };

MTAB simh_mod[] = {
	/* quite, no error messages	*/
	{ UNIT_VERBOSE,	0,						"QUIET",		"QUIET",		NULL },
	/* verbose, display error messages */
	{ UNIT_VERBOSE,	UNIT_VERBOSE,	"VERBOSE",	"VERBOSE",	NULL },
	{ 0 } };

DEVICE simh_device = {
	"SIMH", &simh_unit, simh_reg, simh_mod,
	1, 10, 31, 1, 16, 4,
	NULL, NULL, &simh_dev_reset,
	NULL, NULL, NULL };


t_stat sio_attach (UNIT *uptr, char *cptr)
{
return tmxr_attach (&altairTMXR, uptr, cptr);			/* attach mux */
}

/* Detach */

t_stat sio_detach (UNIT *uptr)
{
sio_unit.u3 = 0x02;	/* Status	*/
sio_unit.buf = 0;		/* Data		*/
return tmxr_detach (&altairTMXR, uptr);
}

/* Service routines to handle simulator functions */

/* service routine - actually gets char & places in buffer */

t_stat sio_svc (UNIT *uptr)
{
	int32 temp;

	sim_activate (&sio_unit, sio_unit.wait);		/* continue poll			*/

	if (sio_unit.flags & UNIT_ATT) {
		if (sim_poll_kbd () == SCPE_STOP) {				/* listen for ^E			*/
			return SCPE_STOP;
		}
		temp = tmxr_poll_conn(&altairTMXR, &sio_unit);	/* poll connection		*/
		if (temp >= 0) {
			altairTMXR.ldsc[temp] -> rcve = 1;			/* enable receive			*/
		}
		tmxr_poll_rx(&altairTMXR);								/* poll input					*/
		tmxr_poll_tx(&altairTMXR);								/* poll output				*/
	}
	else {
		if ((temp = sim_poll_kbd ()) < SCPE_KFLAG) {
			return temp;														/* no char or error?	*/
		}
		sio_unit.buf = temp & 0xff;								/* Save char					*/
		sio_unit.u3 |= 0x01;											/* Set status					*/
	}
	return SCPE_OK;
}


t_stat ptr_svc (UNIT *uptr)
{
	return SCPE_OK;
}

t_stat ptp_svc (UNIT *uptr)
{
	return SCPE_OK;
}


/* Reset routine */

t_stat sio_reset (DEVICE *dptr)
{
	if (sio_unit.flags & UNIT_ATT) {
		if (altairTMXR.ldsc[0]->conn > 0) {
			tmxr_reset_ln(altairTMXR.ldsc[0]);
		}
		sio_unit.u3 = 0;	/* Status */
	}
	else {
		sio_unit.u3 = 0x02;	/* Status */
	}
	sio_unit.buf = 0;						/* Data */
	sim_activate (&sio_unit, sio_unit.wait);	/* activate unit */
	return SCPE_OK;
}


t_stat ptr_reset (DEVICE *dptr)
{
	ptr_unit.buf = 0;
	ptr_unit.u3 = 0;
	ptr_unit.pos = 0;
	if (ptr_unit.flags & UNIT_ATT)	{	/* attached? */
		rewind(ptr_dev.units -> fileref);
	}
	sim_cancel (&ptp_unit);	/* deactivate unit */
	return SCPE_OK;
}

t_stat ptp_reset (DEVICE *dptr)
{
	ptp_unit.buf = 0;
	ptp_unit.u3 = 0x02;
	sim_cancel (&ptp_unit);	/* deactivate unit */
	return SCPE_OK;
}

/*	I/O instruction handlers, called from the CPU module when an
		IN or OUT instruction is issued.

		Each function is passed an 'io' flag, where 0 means a read from
		the port, and 1 means a write to the port. On input, the actual
		input is passed as the return value, on output, 'data' is written
		to the device.
*/

int32 sio0s(int32 port, int32 io, int32 data)
{
	if (io == 0) { /* IN */
		if (sio_unit.flags & UNIT_ATT) {
			sio_unit.u3 = (((tmxr_rqln(altairTMXR.ldsc[0]) > 0	? 0x01 : 0) | /* read possible if character available	*/
				(altairTMXR.ldsc[0]->conn == 0 ? 0 : 0x02)));										/* write possible if connected					*/
		}
		return (sio_unit.u3);
	}
	else { /* OUT */
		if (sio_unit.flags & UNIT_ATT) {
			if (data == 0x03) {		/* reset port! */
				sio_unit.u3 = 0;
				sio_unit.buf = 0;
			}
		}
		else {
			if (data == 0x03) {		/* reset port! */
				sio_unit.u3 = 0x02;
				sio_unit.buf = 0;
			}
		}
		return (0);	/* ignored since OUT */
	}
}

int32 sio0d(int32 port, int32 io, int32 data)
{
	if (io == 0) { /* IN */
		if (sio_unit.flags & UNIT_ATT) {
			sio_unit.buf = tmxr_getc_ln(altairTMXR.ldsc[0]) & 0xff;
		}
		sio_unit.u3 = sio_unit.u3 & 0xFE;
		if (sio_unit.flags & UNIT_BS) {
			if (sio_unit.buf == BACKSPACE_CHAR) {
				sio_unit.buf = DELETE_CHAR;
			}
		}
		else {
			if (sio_unit.buf == DELETE_CHAR) {
				sio_unit.buf = BACKSPACE_CHAR;
			}
		}
		return ((sio_unit.flags & UNIT_UPPER) ? toupper(sio_unit.buf) : sio_unit.buf);
	}
	else { /* OUT */
		if (sio_unit.flags & UNIT_ANSI) {
			data &= 0x7f;
		}
		if (sio_unit.flags & UNIT_ATT) {
			tmxr_putc_ln(altairTMXR.ldsc[0], data);
		}
		else {
			sim_putchar(data);
		}
		return (0);	/* ignored since OUT */
	}
}

/* Port 2 controls the PTR/PTP devices */

int32 sio1s(int32 port, int32 io, int32 data)
{
	if (io == 0) {
		/* reset I bit iff PTR unit not attached or no more data available.	*/
		/* O bit is always set since write always possible.									*/
		return ((ptr_unit.flags & UNIT_ATT) == 0) || (ptr_unit.u3 != 0) ? 0x02 : 0x03;
	}
	else { /* OUT */
		if (data == 0x03) {
			ptr_unit.u3 = 0;
			ptr_unit.buf = 0;
			ptr_unit.pos = 0;
			ptp_unit.u3 = 0;
			ptp_unit.buf = 0;
			ptp_unit.pos = 0;
		}
		return (0);						/* ignored since OUT */
	}
}

int32 sio1d(int32 port, int32 io, int32 data)
{
	int32 temp;
	if (io == 0) {	/* IN */
		if (((ptr_unit.flags & UNIT_ATT) == 0)	|| (ptr_unit.u3 != 0))
			return (0);	/* not attached or no more data available */
		if ((temp = getc(ptr_dev.units -> fileref)) == EOF) {	/* end of file? */
			ptr_unit.u3 = 0x01;
			return (CONTROLZ_CHAR);	/* control Z denotes end of text file in CP/M */
		}
		ptr_unit.pos++;
		return (temp & 0xFF);
	}
	else { /* OUT */
		putc(data, ptp_dev.units -> fileref);
		ptp_unit.pos++;
		return (0);	/* ignored since OUT */
	}
}

int32 nulldev(int32 port, int32 io, int32 data)
{
	if (sio_unit.flags & UNIT_TRAP) {
		if (io == 0) {
			printf("\n[%04xh] Unassigned IN(%2xh)", PCX, port);
			if (sim_log) {
				fprintf(sim_log, "\n[%04xh] Unassigned IN(%2xh)", PCX, port);
			}
		}
		else {
			printf("\n[%04xh] Unassigned OUT(%2xh) -> %2xh", PCX, port, data);
			if (sim_log) {
				fprintf(sim_log, "\n[%04xh] Unassigned OUT(%2xh) -> %2xh", PCX, port, data);
			}
		}
	}
	return (io == 0 ? 0xff : 0);
}

int32 sr_dev(int32 port, int32 io, int32 data) {
	return (io == 0 ? SR : 0);
}

t_stat simh_svc (UNIT *uptr)
{
	return SCPE_OK;
}

int32 toBCD(int32 x) {
	return (x / 10) * 16 + (x % 10);
}

int32 fromBCD(int32 x) {
	return 10*((0xf0 & x) >> 4) + (0x0f & x);
}

#define splimit								10
#define printTimeCmd					0
#define markTimeCmd						1
#define showTimeCmd						2
#define resetPTRCmd						3
#define attachPTRCmd					4
#define detachPTRCmd					5
#define resetVersionCmd				6
#define getClockZSDOSCmd			7
#define setClockZSDOSCmd			8
#define getClockCPM3Cmd				9
#define setClockCPM3Cmd				10
#define	getBankSelectCmd			11
#define	setBankSelectCmd			12
#define getCommonCmd					13
#define cpmCommandLineLength	128
struct tm *currentTime = NULL;
uint32 markTime[splimit];
char version[] = "SIMH002";

t_stat simh_dev_reset() {
	currentTime				= NULL;
	ClockZSDOSDelta		= 0;
	setClockZSDOSPos	= 0;
	getClockZSDOSPos	= 0;
	ClockCPM3Delta		= 0;
	setClockCPM3Pos		= 0;
	getClockCPM3Pos		= 0;
	getCommonPos			= 0;
	markTimeSP				= 0;
	versionPos				= 0;
	lastCommand				= 0;
	return SCPE_OK;
}

/* The CP/M commandline is used as the name of a file and PTR is attached to it */
void attachCPM() {
	char cpmCommandLine[cpmCommandLineLength];
	uint32 status, i, len = (GetBYTEWrapper(0x80) & 0x7f) - 1; /* 0x80 contains length of commandline, discard first char */
	for (i = 0; i < len; i++) {
		cpmCommandLine[i] = (char)GetBYTEWrapper(0x82+i); /* the first char, typically ' ', is discarded */
	}
	cpmCommandLine[i] = 0; /* make C string */
	sim_switches = SWMASK ('R');
	status = attach_unit(&ptr_unit, cpmCommandLine);
	if ((status != SCPE_OK) && (simh_unit.flags & UNIT_VERBOSE)) {
		printf("Cannot open '%s' (%s).\n", cpmCommandLine,
			scp_error_messages[status - SCPE_BASE]);
			if (sim_log) {
				fprintf(sim_log, "Cannot open '%s' (%s).\n", cpmCommandLine,
					scp_error_messages[status - SCPE_BASE]);
			}
	}
}

/* setClockZSDOSAdr points to 6 byte block in M: YY MM DD HH MM SS in BCD notation */
void setClockZSDOS() {
	struct tm newTime;
	int32 year = fromBCD(GetBYTEWrapper(setClockZSDOSAdr));
	newTime.tm_year	= year < 50 ? year + 100 : year;
	newTime.tm_mon	= fromBCD(GetBYTEWrapper(setClockZSDOSAdr + 1)) - 1;
	newTime.tm_mday	= fromBCD(GetBYTEWrapper(setClockZSDOSAdr + 2));
	newTime.tm_hour	= fromBCD(GetBYTEWrapper(setClockZSDOSAdr + 3));
	newTime.tm_min	= fromBCD(GetBYTEWrapper(setClockZSDOSAdr + 4));
	newTime.tm_sec	= fromBCD(GetBYTEWrapper(setClockZSDOSAdr + 5));
	ClockZSDOSDelta = mktime(&newTime) - time(NULL);
}

#define secondsPerMinute	60
#define secondsPerHour		(60*secondsPerMinute)
#define	secondsPerDay			(24*secondsPerHour)
time_t mkCPM3Origin() {
	struct tm date;
	date.tm_year	= 77;
	date.tm_mon		= 11;
	date.tm_mday	= 31;
	date.tm_hour	= 0;
	date.tm_min		= 0;
	date.tm_sec		= 0;
	return mktime(&date);
}

/* setClockCPM3Adr points to 5 byte block in M:
	0 - 1 int16:		days since 31 Dec 77
			2 BCD byte:	HH
			3 BCD byte:	MM
			4 BCD byte:	SS																*/
void setClockCPM3() {
	ClockCPM3Delta = mkCPM3Origin()																		+
		(GetBYTEWrapper(setClockCPM3Adr) + GetBYTEWrapper(setClockCPM3Adr+1)*256) * secondsPerDay	+
		fromBCD(GetBYTEWrapper(setClockCPM3Adr + 2)) * secondsPerHour								+
		fromBCD(GetBYTEWrapper(setClockCPM3Adr + 3)) * secondsPerMinute							+
		fromBCD(GetBYTEWrapper(setClockCPM3Adr + 4)) - time(NULL);
}

int32 simh_in() {
	int32 result;
	switch(lastCommand) {
		case getCommonCmd:
			if (getCommonPos == 0) {
				result = common & 0xff;
				getCommonPos = 1;
			}
			else {
				result = (common >> 8) & 0xff;
				getCommonPos = 0;
			}
			break;
		case getBankSelectCmd:
			result = bankSelect;
			break;
		case resetVersionCmd:
			result = version[versionPos++];
			if (result == 0) {
				versionPos = 0;
			}
			break;
		case getClockZSDOSCmd:
			if (currentTime) {
				switch(getClockZSDOSPos) {
					case 0:
						result = toBCD(currentTime -> tm_year > 99 ?
							currentTime -> tm_year - 100 : currentTime -> tm_year);
						break;
					case 1:
						result = toBCD(currentTime -> tm_mon + 1);
						break;
					case 2:
						result = toBCD(currentTime -> tm_mday);
						break;
					case 3:
						result = toBCD(currentTime -> tm_hour);
						break;
					case 4:
						result = toBCD(currentTime -> tm_min);
						break;
					case 5:
						result = toBCD(currentTime -> tm_sec);
						break;
					default:
						result = 0;
				}
				getClockZSDOSPos++;
			}
			else {
				result = 0;
			}
			break;
		case getClockCPM3Cmd:
			if (currentTime) {
				switch(getClockCPM3Pos) {
					case 0:
						result = daysCPM3SinceOrg & 0xff;
						break;
					case 1:
						result = (daysCPM3SinceOrg >> 8) & 0xff;
						break;
					case 2:
						result = toBCD(currentTime -> tm_hour);
						break;
					case 3:
						result = toBCD(currentTime -> tm_min);
						break;
					case 4:
						result = toBCD(currentTime -> tm_sec);
						break;
					default:
						result = 0;
				}
				getClockCPM3Pos++;
			}
			else {
				result = 0;
			}
			break;
		default:
			result = 0;
	}
	return (result);
}

int32 simh_out(int32 data) {
	uint32 delta;
	time_t now;
	switch(lastCommand) {
		case setBankSelectCmd:
			bankSelect = data & 3;
			lastCommand = 0;
			break;
		case setClockZSDOSCmd:
			switch(setClockZSDOSPos) {
				case 0:
					setClockZSDOSAdr = data;
					setClockZSDOSPos++;
					break;
				case 1:
					setClockZSDOSAdr += (data << 8);
					setClockZSDOS();
					lastCommand = 0;
					break;
				default:;
			}
			break;
		case setClockCPM3Cmd:
			switch(setClockCPM3Pos) {
				case 0:
					setClockCPM3Adr = data;
					setClockCPM3Pos++;
					break;
				case 1:
					setClockCPM3Adr += (data << 8);
					setClockCPM3();
					lastCommand = 0;
					break;
				default:;
			}
			break;
		default:
			lastCommand = data;
			switch(data) {
				case printTimeCmd:	/* print time */
					if (rtc_avail) {
						printf("Current time in milliseconds = %d.\n", sim_os_msec ());
						if (sim_log) {
							fprintf(sim_log, "Current time in milliseconds = %d.\n", sim_os_msec ());
						}
					}
					break;
				case markTimeCmd:		/* mark time */
					if (rtc_avail) {
						if (markTimeSP < splimit) {
							markTime[markTimeSP++] = sim_os_msec ();
						}
						else {
							printf("Mark stack overflow.\n");
							if (sim_log) {
								fprintf(sim_log, "Mark stack overflow.\n");
							}
						}
					}
					break;
				case showTimeCmd:		/* show time difference */
					if (rtc_avail) {
						if (markTimeSP > 0) {
							delta = sim_os_msec () - markTime[--markTimeSP];
							printf("Delta to mark in milliseconds = %d.\n", delta);
							if (sim_log) {
								fprintf(sim_log, "Delta to mark in milliseconds = %d.\n", delta);
							}
						}
						else {
							printf("Missing mark.\n");
							if (sim_log) {
								fprintf(sim_log, "Missing mark.\n");
							}
						}
					}
					break;
				case resetPTRCmd:		/* reset ptr device */
					ptr_reset(NULL);
					break;
				case attachPTRCmd:	/* attach ptr to the file with name at beginning of CP/M command line */
					attachCPM();
					break;
				case detachPTRCmd:	/* detach ptr */
					detach_unit(&ptr_unit);
					break;
				case resetVersionCmd:
					versionPos = 0;
					break;
				case getClockZSDOSCmd:
					time(&now);
					now += ClockZSDOSDelta;
					currentTime = localtime(&now);
					getClockZSDOSPos = 0;
					break;
				case setClockZSDOSCmd:
					setClockZSDOSPos = 0;
					break;
				case getClockCPM3Cmd:
					time(&now);
					now += ClockCPM3Delta;
					currentTime = localtime(&now);
					daysCPM3SinceOrg = (now - mkCPM3Origin()) / secondsPerDay;
					getClockCPM3Pos = 0;
					break;
				case setClockCPM3Cmd:
					setClockCPM3Pos = 0;
					break;
				default:;
			}
	}
	return 0; /* ignored, since OUT */
}

/* port 0xfe is a device for communication SIMH <--> Altair machine */
int32 simh_dev(int32 port, int32 io, int32 data) {
	return io == 0 ? simh_in() : simh_out(data);
}

/* port 0xfd is used to reset the port 0xfe state machine */
int32 simh_dev0xfd(int32 port, int32 io, int32 data) {
	markTimeSP	= 0;
	lastCommand	= 0;
	return 0; /* always */
}
