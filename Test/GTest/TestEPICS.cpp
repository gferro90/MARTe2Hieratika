/*************************************************************************\
* Copyright (c) 2009 Helmholtz-Zentrum Berlin fuer Materialien und Energie.
 * Copyright (c) 2002 The University of Chicago, as Operator of Argonne
 *     National Laboratory.
 * Copyright (c) 2002 The Regents of the University of California, as
 *     Operator of Los Alamos National Laboratory.
 * Copyright (c) 2002 Berliner Elektronenspeicherringgesellschaft fuer
 *     Synchrotronstrahlung.
 * EPICS BASE is distributed subject to a Software License Agreement found
 * in file LICENSE that is included with this distribution.
 \*************************************************************************/

/*
 *  Author: Ralph Lange (BESSY)
 *
 *  Modification History
 *  2008/04/16 Ralph Lange (BESSY)
 *     Updated usage info
 *  2009/03/31 Larry Hoff (BNL)
 *     Added field separators
 *  2009/04/01 Ralph Lange (HZB/BESSY)
 *     Added support for long strings (array of char) and quoting of nonprintable characters
 *
 */

#include <stdio.h>
#include <epicsStdlib.h>
#include <string.h>

#include <cadef.h>
#include <epicsGetopt.h>

#include "tool_lib.h"

#define VALID_DOUBLE_DIGITS 18  /* Max usable precision for a double */

static unsigned long reqElems = 0;
static unsigned long eventMask = DBE_VALUE | DBE_ALARM; /* Event mask used */
static int floatAsString = 0; /* Flag: fetch floats as string */
static int nConn = 0; /* Number of connected PVs */

static int tsInitC = 0;
static epicsTimeStamp tsStart, tsFirst, tsPreviousC, tsPreviousS;
capri caPriority = DEFAULT_CA_PRIORITY;
int create_pvs(pv* pvs,
               int nPvs,
               caCh *pCB) {
    int n;
    int result;
    int returncode = 0;

    if (!tsInitC) /* Initialize start timestamp */
    {
        epicsTimeGetCurrent(&tsStart);
        tsInitC = 1;
    }
    /* Issue channel connections */
    for (n = 0; n < nPvs; n++) {
        result = ca_create_channel(pvs[n].name, pCB, &pvs[n], caPriority, &pvs[n].chid);
        if (result != ECA_NORMAL) {
            fprintf(stderr, "CA error %s occurred while trying "
                    "to create channel '%s'.\n",
                    ca_message(result), pvs[n].name);
            pvs[n].status = result;
            returncode = 1;
        }
    }

    return returncode;
}

void usage(void) {
    fprintf(stderr, "\nUsage: camonitor [options] <PV name> ...\n"
            "\n"
            "  -h:       Help; Print this message\n"
            "Channel Access options:\n"
            "  -w <sec>: Wait time, specifies CA timeout, default is %f second(s)\n"
            "  -m <msk>: Specify CA event mask to use.  <msk> is any combination of\n"
            "            'v' (value), 'a' (alarm), 'l' (log/archive), 'p' (property).\n"
            "            Default event mask is 'va'\n"
            "  -p <pri>: CA priority (0-%u, default 0=lowest)\n"
            "Timestamps:\n"
            "  Default:  Print absolute timestamps (as reported by CA server)\n"
            "  -t <key>: Specify timestamp source(s) and type, with <key> containing\n"
            "            's' = CA server (remote) timestamps\n"
            "            'c' = CA client (local) timestamps (shown in '()'s)\n"
            "            'n' = no timestamps\n"
            "            'r' = relative timestamps (time elapsed since start of program)\n"
            "            'i' = incremental timestamps (time elapsed since last update)\n"
            "            'I' = incremental timestamps (time since last update, by channel)\n"
            "            'r', 'i' or 'I' require 's' or 'c' to select the time source\n"
            "Enum format:\n"
            "  -n:       Print DBF_ENUM values as number (default is enum string)\n"
            "Array values: Print number of elements, then list of values\n"
            "  Default:  Request and print all elements (dynamic arrays supported)\n"
            "  -# <num>: Request and print up to <num> elements\n"
            "  -S:       Print arrays of char as a string (long string)\n"
            "Floating point format:\n"
            "  Default:  Use %%g format\n"
            "  -e <num>: Use %%e format, with a precision of <num> digits\n"
            "  -f <num>: Use %%f format, with a precision of <num> digits\n"
            "  -g <num>: Use %%g format, with a precision of <num> digits\n"
            "  -s:       Get value as string (honors server-side precision)\n"
            "  -lx:      Round to long integer and print as hex number\n"
            "  -lo:      Round to long integer and print as octal number\n"
            "  -lb:      Round to long integer and print as binary number\n"
            "Integer number format:\n"
            "  Default:  Print as decimal number\n"
            "  -0x:      Print as hex number\n"
            "  -0o:      Print as octal number\n"
            "  -0b:      Print as binary number\n"
            "Alternate output field separator:\n"
            "  -F <ofs>: Use <ofs> to separate fields in output\n"
            "\n"
            "Example: camonitor -f8 my_channel another_channel\n"
            "  (doubles are printed as %%f with precision of 8)\n\n",
            DEFAULT_TIMEOUT, CA_PRIORITY_MAX);
}

/*+**************************************************************************
 *
 * Function:event_handler
 *
 * Description:CA event_handler for request type callback
 * Prints the event data
 *
 * Arg(s) In:args  -  event handler args (see CA manual)
 *
 **************************************************************************-*/

struct myArgs {
    pv* mypv;
    char ts[1024];
    float x;
};

static void event_handler(evargs args) {
    myArgs *myargs = (myArgs *) (args.usr);
    pv* pv = myargs->mypv;

    pv->status = args.status;
    //if (args.status == ECA_NORMAL) {
    pv->dbrType = args.type;
    pv->nElems = args.count;
    pv->value = (void *) args.dbr; /* casting away const */
    //const int TIMETEXTLEN = 1024;
    char timeText[1024];
    epicsTimeStamp *ptsNewS = &((struct dbr_time_string *) (pv->value))->stamp;
    epicsTimeToStrftime(myargs->ts, 1024, "%Y-%m-%d %H:%M:%S.%06f\n", ptsNewS);
    //printf("%s", timeText);
    //}
}


static void event_handler1(evargs args) {
    myArgs *myargs = (myArgs *) (args.usr);
    myargs->x=*(float*)(args.dbr);
}

/*+**************************************************************************
 *
 * Function:connection_handler
 *
 * Description:CA connection_handler
 *
 * Arg(s) In:args  -  connection_handler_args (see CA manual)
 *
 **************************************************************************-*/

static void connection_handler(struct connection_handler_args args) {
    pv *ppv = (pv *) ca_puser(args.chid);
    if (args.op == CA_OP_CONN_UP) {
        nConn++;
        if (!ppv->onceConnected) {
            ppv->onceConnected = 1;
            /* Set up pv structure */
            /* ------------------- */

            /* Get natural type and array count */
            ppv->dbfType = ca_field_type(ppv->chid);
            ppv->dbrType = dbf_type_to_DBR_TIME(ppv->dbfType); /* Use native type */
            if (dbr_type_is_ENUM(ppv->dbrType)) /* Enums honour -n option */
            {
                ppv->dbrType = DBR_TIME_STRING;
            }
            else if (floatAsString && (dbr_type_is_FLOAT(ppv->dbrType) || dbr_type_is_DOUBLE(ppv->dbrType))) {
                ppv->dbrType = DBR_TIME_STRING;
            }
            /* Set request count */
            ppv->nElems = ca_element_count(ppv->chid);
            ppv->reqElems = reqElems > ppv->nElems ? ppv->nElems : reqElems;

            /* Issue CA request */
            /* ---------------- */
            /* install monitor once with first connect */
            ppv->status = ca_create_subscription(ppv->dbrType, ppv->reqElems, ppv->chid, eventMask, event_handler, (void*) ppv, NULL);
        }
    }
    else if (args.op == CA_OP_CONN_DOWN) {
        nConn--;
        ppv->status = ECA_DISCONN;
    }
}

/*+**************************************************************************
 *
 * Function:main
 *
 * Description:camonitor main()
 * Evaluate command line options, set up CA, connect the
 * channels, collect and print the data as requested
 *
 * Arg(s) In:[options] <pv-name> ...
 *
 * Arg(s) Out:none
 *
 * Return(s):Standard return code (0=success, 1=error)
 *
 **************************************************************************-*/

int main(int argc,
         char *argv[]) {
    int returncode = 0;
    int n;
    int result; /* CA result */
    IntFormatT outType; /* Output type */

    int opt; /* getopt() current option */
    int digits = 0; /* getopt() no. of float digits */

    int nPvs; /* Number of PVs */
    pv* pvs; /* Array of PV structures */

    int optind = 1;
    nPvs = argc - optind; /* Remaining arg list are PV names */

    if (nPvs < 1) {
        fprintf(stderr, "No pv name specified. ('camonitor -h' for help.)\n");
        return 1;
    }
    /* Start up Channel Access */

    result = ca_context_create(ca_disable_preemptive_callback);
    if (result != ECA_NORMAL) {
        fprintf(stderr, "CA error %s occurred while trying "
                "to start channel access.\n",
                ca_message(result));
        return 1;
    }
    /* Allocate PV structure array */

    pvs = calloc(nPvs, sizeof(pv));
    if (!pvs) {
        fprintf(stderr, "Memory allocation for channel structures failed.\n");
        return 1;
    }
    /* Connect channels */

    /* Copy PV names from command line */
    for (n = 0; optind < argc; n++, optind++) {
        pvs[n].name = argv[optind];
    }
    /* Create CA connections */
#if 1
    if (!tsInitC) /* Initialize start timestamp */
    {
        epicsTimeGetCurrent(&tsStart);
        tsInitC = 1;
    }
    for (n = 0; n < nPvs; n++) {
        result = ca_create_channel(pvs[n].name, NULL, &pvs[n], caPriority, &pvs[n].chid);
        if (result != ECA_NORMAL) {
            fprintf(stderr, "CA error %s occurred while trying "
                    "to create channel '%s'.\n",
                    ca_message(result), pvs[n].name);
            pvs[n].status = result;
            returncode = 1;
        }
    }

    for (n = 0; n < nPvs; n++) {
        result = ca_create_channel(pvs[n].name, NULL, &pvs[n], caPriority, &pvs[n].chid);
        if (result != ECA_NORMAL) {
            fprintf(stderr, "CA error %s occurred while trying "
                    "to create channel '%s'.\n",
                    ca_message(result), pvs[n].name);
            pvs[n].status = result;
            returncode = 1;
        }
    }

    myArgs *myargs = new myArgs[nPvs];
    for (n = 0; n < nPvs; n++) {
        nConn++;
        pv *ppv = (pv *) ca_puser(pvs[n].chid);
        printf("%s\n", ppv->name);
        ppv->onceConnected = 1;
        /* Set up pv structure */
        /* ------------------- */

        /* Get natural type and array count */
        ppv->dbfType = DBF_DOUBLE;
        ppv->dbrType = dbf_type_to_DBR_TIME(ppv->dbfType); /* Use native type */
        if (dbr_type_is_ENUM(ppv->dbrType)) /* Enums honour -n option */
        {
            ppv->dbrType = DBR_TIME_STRING;
        }
        else if (floatAsString && (dbr_type_is_FLOAT(ppv->dbrType) || dbr_type_is_DOUBLE(ppv->dbrType))) {
            ppv->dbrType = DBR_TIME_STRING;
        }
        /* Set request count */
        ppv->nElems = ca_element_count(ppv->chid);
        ppv->reqElems = reqElems > ppv->nElems ? ppv->nElems : reqElems;

        /* Issue CA request */
        /* ---------------- */
        /* install monitor once with first connect */
        myargs[n].mypv = ppv;
        ppv->status = ca_create_subscription(ppv->dbrType, ppv->reqElems, ppv->chid, eventMask, event_handler, (void*) &myargs[n], NULL);
        ppv->status = ca_create_subscription(DBR_FLOAT, ppv->reqElems, ppv->chid, eventMask, event_handler1, (void*) &myargs[n], NULL);

    }
#else
    returncode = create_pvs(pvs, nPvs, connection_handler);

    if ( returncode ) {
        return returncode;
    }
#endif
    /* Check for channels that didn't connect */
    //ca_pend_event(10000000);
    for (n = 0; n < nPvs; n++) {
        /*if (!pvs[n].onceConnected)
         print_time_val_sts(&pvs[n], reqElems);*/
    }

    /* Read and print data forever */
    //ca_pend_event(0);
    while (1) {
        sleep(1);
        printf("%s %f\n", myargs[0].ts, myargs[0].x);
        ca_pend_event(0.01);
    } /* Shut down Channel Access */
    ca_context_destroy();

    return result;
}
