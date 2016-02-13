/* 
 * File:   pcarsDump.h
 * Author: banshee
 *
 * Created on 3 de octubre de 2015, 18:31
 */

#ifndef SIMSOURCE_H
#define	SIMSOURCE_H

#include <stdbool.h>
#include <stdio.h>
#include "../../ext/SharedMemory.h"
#include "pcarsApi.h"
#include "ACApi.h"
#include "pcarsDump.h"
#include "jSon.h"

enum SIMS {
    PCARS_GAME,
    ASSETTO_GAME,
    IRACING_GAME 
} SIMS;

typedef struct dataExtension {
    float lastBestLapTime;
    float updatedLastBestLapTime;
} dataExtension;


typedef struct pCarsSourceContext {
    SharedMemory*           pCarsSHM;
    pCarsContext*           pCarsCtx;
    pCarsDumpReaderContext* pCarsDumpCtx;
    //dataExtension           dataExt;
} pCarsSourceContext;

typedef struct assetoSourceContext {
    aCContext*              acCtx;
} assetoSourceContext;

typedef struct iRacingSourceContext {
    // TODO:
} iRacingSourceContext;

typedef struct simSourceContext {
    int                     currentGame;
    dataExtension           dataExt;
    
    // Games contexts
    pCarsSourceContext      pCarsSourceCtx;
    assetoSourceContext     assettoSourceCtx;
    iRacingSourceContext    iRacingSourceCtx;
} simSourceContext;


typedef struct jSonData {
    int*  fields;
    char* jSonResult;
} jSonData;

void loadDefaultSimSourceContext(simSourceContext* ctx);
int  initializeSimSourceContext(simSourceContext* ctx);
void freeSimSourceContext(simSourceContext* ctx);
int getSimSourceFields(jSonDocument* jSonDoc);

// PCars
void setSimSourcePCarsAPI(simSourceContext* ctx, pCarsContext* pCarsCtx);
void setSimSourcePCarsDump(simSourceContext* ctx, pCarsDumpReaderContext* pCarsDumpCtx);

// Assetto
// TODO:

// iRacing
// TODO:


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
    EXT_MCRASHSTATE,
    END_PCARS_FIELDS
} PCARS_FIELDS;

int   enumPCarsFieldsFromString(const char *s);
char* enumPCarsFieldsToString(int e);


int getGameState(simSourceContext* ctx);
float getSpeed(simSourceContext* ctx);
float getMaxRpms(simSourceContext* ctx);
float getRpm(simSourceContext* ctx);
float getGear(simSourceContext* ctx);
float getThrottle(simSourceContext* ctx);



#endif	/* SIMSOURCE_H */

