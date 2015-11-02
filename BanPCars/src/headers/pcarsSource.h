/* 
 * File:   pcarsDump.h
 * Author: banshee
 *
 * Created on 3 de octubre de 2015, 18:31
 */

#ifndef PCARSSOURCE_H
#define	PCARSSOURCE_H

#include <stdbool.h>
#include <stdio.h>
#include "../../ext/SharedMemory.h"
#include "pcarsApi.h"
#include "pcarsDump.h"
#include "jSon.h"


typedef struct pCarsSourceContext {
    SharedMemory*           pCarsSHM;
    pCarsContext*           pCarsCtx;
    pCarsDumpReaderContext* pCarsDumpCtx;
} pCarsSourceContext;


typedef struct jSonData {
    int*  fields;
    char* jSonResult;
} jSonData;

void loadDefaultpCarsSourceContext(pCarsSourceContext* ctx);
void setPCarsSourcePCarsAPI(pCarsSourceContext* ctx, pCarsContext* pCarsCtx);
void setPCarsSourcePCarsDump(pCarsSourceContext* ctx, pCarsDumpReaderContext* pCarsDumpCtx);
int  initializePCarsSourceContext(pCarsSourceContext* ctx);
void freePCarsSourceContext(pCarsSourceContext* ctx);

int getPCarsSourceFields(pCarsSourceContext* ctx, jSonDocument* jSonDoc);


////////////
// Fields
////////////
enum PCARS_FIELDS { 

    MVERSION, 
    MBUILDVERSIONNUMBER,
    MGAMESTATE,
    MSESSIONSTATE,
    MRACESTATE,
    MVIEWEDPARTICIPANTINDEX,
    MNUMPARTICIPANTS,
    MPARTICIPANTINFO,
    MUNFILTEREDTHROTTLE,
    MUNFILTEREDBRAKE ,
    MUNFILTEREDSTEERING,
    MUNFILTEREDCLUTCH,
    MCARNAME,
    MCARCLASSNAME,
    MLAPSINEVENT,
    MTRACKLOCATION,
    MTRACKVARIATION,
    MTRACKLENGTH,
    MLAPINVALIDATED,
    MBESTLAPTIME,
    MLASTLAPTIME,
    MCURRENTTIME,
    MSPLITTIMEAHEAD,
    MSPLITTIMEBEHIND,
    MSPLITTIME,
    MEVENTTIMEREMAINING,
    MPERSONALFASTESTLAPTIME,
    MWORLDFASTESTLAPTIME,
    MCURRENTSECTOR1TIME,
    MCURRENTSECTOR2TIME,
    MCURRENTSECTOR3TIME,
    MFASTESTSECTOR1TIME,
    MFASTESTSECTOR2TIME,
    MFASTESTSECTOR3TIME,
    MPERSONALFASTESTSECTOR1TIME,
    MPERSONALFASTESTSECTOR2TIME,
    MPERSONALFASTESTSECTOR3TIME,
    MWORLDFASTESTSECTOR1TIME,
    MWORLDFASTESTSECTOR2TIME,
    MWORLDFASTESTSECTOR3TIME,
    MHIGHESTFLAGCOLOUR,
    MHIGHESTFLAGREASON,
    MPITMODE,
    MPITSCHEDULE,
    MCARFLAGS,
    MOILTEMPCELSIUS,
    MOILPRESSUREKPA,
    MWATERTEMPCELSIUS,
    MWATERPRESSUREKPA,
    MFUELPRESSUREKPA,
    MFUELLEVEL,
    MFUELCAPACITY ,
    MSPEED,
    MRPM,
    MMAXRPM,
    MBRAKE,
    MTHROTTLE,
    MCLUTCH,
    MSTEERING,
    MGEAR,
    MNUMGEARS,
    MODOMETERKM,
    MANTILOCKACTIVE,
    MLASTOPPONENTCOLLISIONINDEX ,
    MLASTOPPONENTCOLLISIONMAGNITUDE ,
    MBOOSTACTIVE,
    MBOOSTAMOUNT,
    MORIENTATION,
    MLOCALVELOCITY,
    MWORLDVELOCITY,
    MANGULARVELOCITY,
    MLOCALACCELERATION,
    MWORLDACCELERATION,
    MEXTENTSCENTRE,
    MTYREFLAGS,
    MTERRAIN,
    MTYREY,
    MTYRERPS,
    MTYRESLIPSPEED,
    MTYRETEMP,
    MTYREGRIP,
    MTYREHEIGHTABOVEGROUND,
    MTYRELATERALSTIFFNESS,
    MTYREWEAR,
    MBRAKEDAMAGE,
    MSUSPENSIONDAMAGE,
    MBRAKETEMPCELSIUS,
    MTYRETREADTEMP,
    MTYRELAYERTEMP,
    MTYRECARCASSTEMP,
    MTYRERIMTEMP,
    MTYREINTERNALAIRTEMP,
    MCRASHSTATE,
    MAERODAMAGE,
    MENGINEDAMAGE,
    MAMBIENTTEMPERATURE,
    MTRACKTEMPERATURE,
    MRAINDENSITY,
    MWINDSPEED,
    MWINDDIRECTIONX,
    MWINDDIRECTIONY,
    MCLOUDBRIGHTNESS,
    EXT_MSESSIONSECTORGAP,
    EXT_MSESSIONSECTORDELTA,
    EXT_MCURRENTTIME,
    EXT_MLASTLAPTIME,
    EXT_MPOSITION,
    END_PCARS_FIELDS
} PCARS_FIELDS;

int   enumPCarsFieldsFromString(const char *s);
char* enumPCarsFieldsToString(int e);


#endif	/* PCARSSOURCE_H */

