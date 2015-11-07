#include <string.h>

#include "../headers/pcarsSource.h"


void loadDefaultpCarsSourceContext(pCarsSourceContext* ctx){
    memset(ctx, 0, sizeof(pCarsSourceContext));
}

void setPCarsSourcePCarsAPI(pCarsSourceContext* ctx, pCarsContext* pCarsCtx){
    ctx->pCarsCtx = pCarsCtx;
}

void setPCarsSourcePCarsDump(pCarsSourceContext* ctx, pCarsDumpReaderContext* pCarsDumpCtx){
    ctx->pCarsDumpCtx = pCarsDumpCtx;
}

int initializePCarsSourceContext(pCarsSourceContext* ctx){
    
    //TODO: Check and return errors
    if(ctx->pCarsDumpCtx != NULL){
        ctx->pCarsSHM = &ctx->pCarsDumpCtx->pCarsSHM;
        return 0;
    }
    
    if(ctx->pCarsCtx != NULL){
        ctx->pCarsSHM = ctx->pCarsCtx->shmMem;
        return 0;
    }
    
    ctx->dataExt.lastBestLapTime        = -1;
    ctx->dataExt.updatedLastBestLapTime = 0;
    
    return -1;
}

void freePCarsSourceContext(pCarsSourceContext* ctx){
    // Nothing to do
}

int getPCarsSourceFields(pCarsSourceContext* ctx, jSonDocument* jSonDoc){
    int i;
    initializeJSonDocument(jSonDoc);
    
    openJSonArray(jSonDoc, "fields");
    for(i = 0; i < END_PCARS_FIELDS; i++){
        addJSonArrayString(jSonDoc, enumPCarsFieldsToString(i));
    }
    closeJSonArray(jSonDoc);

    return 0;
}

void addSimpleStringValue(jSonDocument* doc, char* fieldName, char* value){
    addJSonStringField(doc, fieldName, value);
}

void addSimpleIntegerValue(jSonDocument* doc, char* fieldName, int value){
    addJSonIntegerField(doc, fieldName, value);
}

void addSimpleFloatValue(jSonDocument* doc, char* fieldName, float value){
    addJSonFloatField(doc, fieldName, value);
}

void addSimpleBoolValue(jSonDocument* doc, char* fieldName, bool value){
    addJSonBoolField(doc, fieldName, value);
}

void addArrayString(jSonDocument* doc, char* fieldName, char** arr, int dim){
    int i;
    
    openJSonArray(doc, fieldName);
    for(i = 0; i < dim; i++){
        addJSonArrayString(doc, arr[i]);
    }
    closeJSonArray(doc);
}

void addArrayInteger(jSonDocument* doc, char* fieldName, int* arr, int dim){
    int i;
    
    openJSonArray(doc, fieldName);
    for(i = 0; i < dim; i++){
        addJSonArrayInteger(doc, arr[i]);
    }
    closeJSonArray(doc);
}

void addArrayFloat(jSonDocument* doc, char* fieldName, float* arr, int dim){
    int i;
    
    openJSonArray(doc, fieldName);
    for(i = 0; i < dim; i++){
        addJSonArrayFloat(doc, arr[i]);
    }
    closeJSonArray(doc);
}

void addArrayBoolean(jSonDocument* doc, char* fieldName, bool* arr, int dim){
    int i;
    
    openJSonArray(doc, fieldName);
    for(i = 0; i < dim; i++){
        addJSonArrayBool(doc, arr[i]);
    }
    closeJSonArray(doc);
}

void getExtMSessionSectorGap(pCarsSourceContext* ctx, float* res){
    
    int sector = ctx->pCarsSHM->mParticipantInfo[ctx->pCarsSHM->mViewedParticipantIndex].mCurrentSector;
    
    // Update Data. Separate in refresh action?
    if(sector > 1){
        if(ctx->dataExt.updatedLastBestLapTime == 0){
            ctx->dataExt.lastBestLapTime = ctx->pCarsSHM->mBestLapTime;
            ctx->dataExt.updatedLastBestLapTime = 1;
        }else{
            ctx->dataExt.updatedLastBestLapTime = 0;
        }
    } 
    
    if(sector < 2 || ctx->pCarsSHM->mCurrentSector1Time == -1  || ctx->pCarsSHM->mFastestSector1Time == -1){
        res[0] = -999999;  
    } else {
        res[0] = ctx->pCarsSHM->mCurrentSector1Time - ctx->pCarsSHM->mFastestSector1Time;
    }
    
    if(sector < 3 ||ctx->pCarsSHM->mCurrentSector2Time == -1  || ctx->pCarsSHM->mFastestSector2Time == -1){
        res[1] = -999999;  
    } else {
        res[1] = ctx->pCarsSHM->mCurrentSector2Time - ctx->pCarsSHM->mFastestSector2Time;
    }
    
    if(sector == 1 && ctx->dataExt.lastBestLapTime != -1 && ctx->pCarsSHM->mBestLapTime != -1){
        res[2] = ctx->pCarsSHM->mLastLapTime - ctx->dataExt.lastBestLapTime;
    }else{
        res[2] = -999999;
    }
}

float getExtMSessionDelta(pCarsSourceContext* ctx){
    float res[3];
    float delta = 0;
    
    int sector = ctx->pCarsSHM->mParticipantInfo[ctx->pCarsSHM->mViewedParticipantIndex].mCurrentSector;
    
    getExtMSessionSectorGap(ctx, res);
    
    if(sector <= 1)
        delta = -999999;
        
    if(sector >= 1 && res[0] != -999999)
        delta += res[0];
    
    if(sector >= 2 && res[1] != -999999)
        delta += res[1];
    
    return delta;
}



void getExtMCurrentTime(pCarsSourceContext* ctx, float* res){
    res[0] = ctx->pCarsSHM->mCurrentTime;
    res[1] = ctx->pCarsSHM->mLapInvalidated || ctx->pCarsSHM->mLapInvalidated == -1;
}
                    
void getExtMLastTime(pCarsSourceContext* ctx, float* res){
    res[0] = ctx->pCarsSHM->mLastLapTime;
    res[1] = ctx->pCarsSHM->mLastLapTime != ctx->pCarsSHM->mBestLapTime;
}

void getExtMPosition(pCarsSourceContext* ctx, char* buff){
    sprintf(buff, "%d/%d", ctx->pCarsSHM->mViewedParticipantIndex+1, ctx->pCarsSHM->mNumParticipants);
}

char* getExtMPCrashState(pCarsSourceContext* ctx){
    switch(ctx->pCarsSHM->mCrashState){
        case CRASH_DAMAGE_NONE:
            return "NONE";
        case CRASH_DAMAGE_OFFTRACK:
            return "OFFTRACK";
        case CRASH_DAMAGE_LARGE_PROP:
            return "LARGE";
        case CRASH_DAMAGE_SPINNING:
            return "SPINNING";
        case CRASH_DAMAGE_ROLLING:
            return "ROLLING";
        default:
            return "-";
    }
}

int getPCarsData(pCarsSourceContext* ctx, int* fieldsArray, jSonDocument* out){
    int i;
    float res[4];
    char buff[1024];
    
    openJSonObject(out, "data");
    
    for(i = 0; i < END_PCARS_FIELDS; i++){
        if(fieldsArray[i] == 1){
            switch (i){
                case MVERSION: 
                    addSimpleIntegerValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mVersion);
                    break;
                case MBUILDVERSIONNUMBER:
                    addSimpleIntegerValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mBuildVersionNumber);
                    break;
                case MGAMESTATE:
                    addSimpleIntegerValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mGameState);
                    break;
                case MSESSIONSTATE:
                    addSimpleIntegerValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mSessionState);
                    break;
                case MRACESTATE:
                    addSimpleIntegerValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mRaceState);
                    break;
                case MVIEWEDPARTICIPANTINDEX:
                    addSimpleIntegerValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mViewedParticipantIndex);
                    break;
                case MNUMPARTICIPANTS:
                    addSimpleIntegerValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mNumParticipants);
                    break;
    //            case MPARTICIPANTINFO:
    //                    return "MPARTICIPANTINFO";
                case MUNFILTEREDTHROTTLE:
                    addSimpleFloatValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mUnfilteredThrottle);
                    break;
                case MUNFILTEREDBRAKE :
                    addSimpleFloatValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mUnfilteredBrake);
                    break;
                case MUNFILTEREDSTEERING:
                    addSimpleFloatValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mUnfilteredSteering);
                    break;
                case MUNFILTEREDCLUTCH:
                    addSimpleFloatValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mUnfilteredClutch);
                    break;
                case MCARNAME:
                    addSimpleStringValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mCarName);
                    break;
                case MCARCLASSNAME:
                    addSimpleStringValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mCarClassName);
                    break;
                case MLAPSINEVENT:
                    addSimpleIntegerValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mLapsInEvent);
                    break;
                case MTRACKLOCATION:
                    addSimpleStringValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mTrackLocation);
                    break;
                case MTRACKVARIATION:
                    addSimpleStringValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mTrackVariation);
                    break;
                case MTRACKLENGTH:
                    addSimpleFloatValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mTrackLength);
                    break;
                case MLAPINVALIDATED:
                    addSimpleBoolValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mTrackLength);
                    break;
                case MBESTLAPTIME:
                    addSimpleFloatValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mBestLapTime);
                    break;
                case MLASTLAPTIME:
                    addSimpleFloatValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mLastLapTime);
                    break;
                case MCURRENTTIME:
                    addSimpleFloatValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mCurrentTime);
                    break;
                case MSPLITTIMEAHEAD:
                    addSimpleFloatValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mSplitTimeAhead);
                    break;
                case MSPLITTIMEBEHIND:
                    addSimpleFloatValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mSplitTimeBehind);
                    break;
                case MSPLITTIME:
                    addSimpleFloatValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mSplitTime);
                    break;
                case MEVENTTIMEREMAINING:
                    addSimpleFloatValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mEventTimeRemaining);
                    break;
                case MPERSONALFASTESTLAPTIME:
                    addSimpleFloatValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mPersonalFastestLapTime);
                    break;
                case MWORLDFASTESTLAPTIME:
                    addSimpleFloatValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mWorldFastestLapTime);
                    break;
                case MCURRENTSECTOR1TIME:
                    addSimpleFloatValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mCurrentSector1Time);
                    break;
                case MCURRENTSECTOR2TIME:
                    addSimpleFloatValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mCurrentSector2Time);
                    break;
                case MCURRENTSECTOR3TIME:
                    addSimpleFloatValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mCurrentSector3Time);
                    break;
                case MFASTESTSECTOR1TIME:
                    addSimpleFloatValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mFastestSector1Time);
                    break;
                case MFASTESTSECTOR2TIME:
                    addSimpleFloatValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mFastestSector2Time);
                    break;
                case MFASTESTSECTOR3TIME:
                    addSimpleFloatValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mFastestSector3Time);
                    break;
                case MPERSONALFASTESTSECTOR1TIME:
                    addSimpleFloatValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mPersonalFastestSector1Time);
                    break;
                case MPERSONALFASTESTSECTOR2TIME:
                    addSimpleFloatValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mPersonalFastestSector2Time);
                    break;
                case MPERSONALFASTESTSECTOR3TIME:
                    addSimpleFloatValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mPersonalFastestSector3Time);
                    break;
                case MWORLDFASTESTSECTOR1TIME:
                    addSimpleFloatValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mWorldFastestSector1Time);
                    break;
                case MWORLDFASTESTSECTOR2TIME:
                    addSimpleFloatValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mWorldFastestSector2Time);
                    break;
                case MWORLDFASTESTSECTOR3TIME:
                    addSimpleFloatValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mWorldFastestSector3Time);
                    break;
                case MHIGHESTFLAGCOLOUR:
                    addSimpleIntegerValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mHighestFlagColour);
                    break;
                case MHIGHESTFLAGREASON:
                    addSimpleIntegerValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mHighestFlagReason);
                    break;
                case MPITMODE:
                    addSimpleIntegerValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mPitMode);
                    break;
                case MPITSCHEDULE:
                    addSimpleIntegerValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mPitSchedule);
                    break;
                case MCARFLAGS:
                    addSimpleIntegerValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mCarFlags);
                    break;
                case MOILTEMPCELSIUS:
                    addSimpleFloatValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mOilTempCelsius);
                    break;
                case MOILPRESSUREKPA:
                    addSimpleFloatValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mOilPressureKPa);
                    break;
                case MWATERTEMPCELSIUS:
                    addSimpleFloatValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mWaterTempCelsius);
                    break;
                case MWATERPRESSUREKPA:
                    addSimpleFloatValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mWaterPressureKPa);
                    break;
                case MFUELPRESSUREKPA:
                    addSimpleFloatValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mFuelPressureKPa);
                    break;
                case MFUELLEVEL:
                    addSimpleFloatValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mFuelLevel);
                    break;
                case MFUELCAPACITY :
                    addSimpleFloatValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mFuelCapacity);
                    break;
                case MSPEED:
                    addSimpleFloatValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mSpeed);
                    break;
                case MRPM:
                    addSimpleFloatValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mRpm);
                    break;
                case MMAXRPM:
                    addSimpleFloatValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mMaxRPM);
                    break;
                case MBRAKE:
                    addSimpleFloatValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mBrake);
                    break;
                case MTHROTTLE:
                    addSimpleFloatValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mThrottle);
                    break;
                case MCLUTCH:
                    addSimpleFloatValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mClutch);
                    break;
                case MSTEERING:
                    addSimpleFloatValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mSteering);
                    break;
                case MGEAR:
                    addSimpleIntegerValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mGear);
                    break;
                case MNUMGEARS:
                    addSimpleIntegerValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mNumGears);
                    break;
                case MODOMETERKM:
                    addSimpleFloatValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mOdometerKM);
                    break;
                case MANTILOCKACTIVE:
                    addSimpleBoolValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mAntiLockActive);
                    break;
                case MLASTOPPONENTCOLLISIONINDEX :
                    addSimpleIntegerValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mLastOpponentCollisionIndex);
                    break;
                case MLASTOPPONENTCOLLISIONMAGNITUDE :
                    addSimpleFloatValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mLastOpponentCollisionMagnitude);
                    break;
                case MBOOSTACTIVE:
                    addSimpleBoolValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mBoostActive);
                    break;
                case MBOOSTAMOUNT:
                    addSimpleFloatValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mBoostAmount);
                    break;
                case MORIENTATION:
                    addArrayFloat(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mOrientation, VEC_MAX);
                    break;
                case MLOCALVELOCITY:
                    addArrayFloat(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mLocalVelocity, VEC_MAX);
                    break;
                case MWORLDVELOCITY:
                    addArrayFloat(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mWorldVelocity, VEC_MAX);
                    break;
                case MANGULARVELOCITY:
                    addArrayFloat(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mAngularVelocity, VEC_MAX);
                    break;
                case MLOCALACCELERATION:
                    addArrayFloat(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mLocalAcceleration, VEC_MAX);
                    break;
                case MWORLDACCELERATION:
                    addArrayFloat(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mWorldAcceleration, VEC_MAX);
                    break;
                case MEXTENTSCENTRE:
                    addArrayFloat(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mExtentsCentre, VEC_MAX);
                    break;
                case MTYREFLAGS:
                    addArrayInteger(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mTyreFlags, TYRE_MAX);
                    break;
                case MTERRAIN:
                    addArrayInteger(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mTerrain, TYRE_MAX);
                    break;
                case MTYREY:
                    addArrayFloat(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mTyreY, TYRE_MAX);
                    break;
                case MTYRERPS:
                    addArrayFloat(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mTyreRPS, TYRE_MAX);
                    break;
                case MTYRESLIPSPEED:
                    addArrayFloat(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mTyreSlipSpeed, TYRE_MAX);
                    break;
                case MTYRETEMP:
                    addArrayFloat(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mTyreTemp, TYRE_MAX);
                    break;
                case MTYREGRIP:
                    addArrayFloat(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mTyreGrip, TYRE_MAX);
                    break;
                case MTYREHEIGHTABOVEGROUND:
                    addArrayFloat(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mTyreHeightAboveGround, TYRE_MAX);
                    break;
                case MTYRELATERALSTIFFNESS:
                    addArrayFloat(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mTyreLateralStiffness, TYRE_MAX);
                    break;
                case MTYREWEAR:
                    addArrayFloat(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mTyreWear, TYRE_MAX);
                    break;
                case MBRAKEDAMAGE:
                    addArrayFloat(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mBrakeDamage, TYRE_MAX);
                    break;
                case MSUSPENSIONDAMAGE:
                    addArrayFloat(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mSuspensionDamage, TYRE_MAX);
                    break;
                case MBRAKETEMPCELSIUS:
                    addArrayFloat(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mBrakeTempCelsius, TYRE_MAX);
                    break;
                case MTYRETREADTEMP:
                    addArrayFloat(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mTyreTreadTemp, TYRE_MAX);
                    break;
                case MTYRELAYERTEMP:
                    addArrayFloat(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mTyreLayerTemp, TYRE_MAX);
                    break;
                case MTYRECARCASSTEMP:
                    addArrayFloat(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mTyreCarcassTemp, TYRE_MAX);
                    break;
                case MTYRERIMTEMP:
                    addArrayFloat(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mTyreRimTemp, TYRE_MAX);
                    break;
                case MTYREINTERNALAIRTEMP:
                    addArrayFloat(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mTyreInternalAirTemp, TYRE_MAX);
                    break;
                case MCRASHSTATE:
                    addSimpleIntegerValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mCrashState);
                    break;
                case MAERODAMAGE:
                    addSimpleFloatValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mAeroDamage);
                    break;
                case MENGINEDAMAGE:
                    addSimpleFloatValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mEngineDamage);
                    break;
                case MAMBIENTTEMPERATURE:
                    addSimpleFloatValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mAmbientTemperature);
                    break;
                case MTRACKTEMPERATURE:
                    addSimpleFloatValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mTrackTemperature);
                    break;
                case MRAINDENSITY:
                    addSimpleFloatValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mRainDensity);
                    break;
                case MWINDSPEED:
                    addSimpleFloatValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mWindSpeed);
                    break;
                case MWINDDIRECTIONX:
                    addSimpleFloatValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mWindDirectionX);
                    break;
                case MWINDDIRECTIONY:
                    addSimpleFloatValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mWindDirectionY);
                    break;
                case MCLOUDBRIGHTNESS:
                    addSimpleFloatValue(out, enumPCarsFieldsToString(i), ctx->pCarsSHM->mCloudBrightness);
                    break;
                case EXT_MSESSIONSECTORGAP:
                    getExtMSessionSectorGap(ctx, res);
                    addArrayFloat(out, enumPCarsFieldsToString(i), res, 3 /*NUM SECTORS*/);
                    break;
                case EXT_MSESSIONSECTORDELTA:
                    addSimpleFloatValue(out, enumPCarsFieldsToString(i), getExtMSessionDelta(ctx));
                    break;                    
                case EXT_MCURRENTTIME:
                    getExtMCurrentTime(ctx, res);
                    addArrayFloat(out, enumPCarsFieldsToString(i), res, 2 /*NUM SECTORS*/);
                    break;
                case EXT_MLASTLAPTIME:
                    getExtMLastTime(ctx, res);
                    addArrayFloat(out, enumPCarsFieldsToString(i), res, 2 /*NUM SECTORS*/);
                    break;
                case EXT_MPOSITION:
                    getExtMPosition(ctx, buff);
                    addSimpleStringValue(out, enumPCarsFieldsToString(i), buff);
                    break;
                case EXT_MCRASHSTATE:
                    addSimpleStringValue(out, enumPCarsFieldsToString(i), getExtMPCrashState(ctx));
                    break;
                default:
                    addSimpleStringValue(out, enumPCarsFieldsToString(i), "NOT_AVAILABLE");
                    break;
            }
        }
    }
    
    closeJSonObject(out);
}


int enumPCarsFieldsFromString(const char *s){

    if(strcmp(s, "MVERSION") == 0) 
	return MVERSION; 
    else if(strcmp(s, "MBUILDVERSIONNUMBER") == 0)
            return MBUILDVERSIONNUMBER;
    else if(strcmp(s, "MGAMESTATE") == 0)
            return MGAMESTATE;
    else if(strcmp(s, "MSESSIONSTATE") == 0)
            return MSESSIONSTATE;
    else if(strcmp(s, "MRACESTATE") == 0)
            return MRACESTATE;
    else if(strcmp(s, "MVIEWEDPARTICIPANTINDEX") == 0)
            return MVIEWEDPARTICIPANTINDEX;
    else if(strcmp(s, "MNUMPARTICIPANTS") == 0)
            return MNUMPARTICIPANTS;
    else if(strcmp(s, "MPARTICIPANTINFO") == 0)
            return MPARTICIPANTINFO;
    else if(strcmp(s, "MUNFILTEREDTHROTTLE") == 0)
            return MUNFILTEREDTHROTTLE;
    else if(strcmp(s, "MUNFILTEREDBRAKE ") == 0)
            return MUNFILTEREDBRAKE ;
    else if(strcmp(s, "MUNFILTEREDSTEERING") == 0)
            return MUNFILTEREDSTEERING;
    else if(strcmp(s, "MUNFILTEREDCLUTCH") == 0)
            return MUNFILTEREDCLUTCH;
    else if(strcmp(s, "MCARNAME") == 0)
            return MCARNAME;
    else if(strcmp(s, "MCARCLASSNAME") == 0)
            return MCARCLASSNAME;
    else if(strcmp(s, "MLAPSINEVENT") == 0)
            return MLAPSINEVENT;
    else if(strcmp(s, "MTRACKLOCATION") == 0)
            return MTRACKLOCATION;
    else if(strcmp(s, "MTRACKVARIATION") == 0)
            return MTRACKVARIATION;
    else if(strcmp(s, "MTRACKLENGTH") == 0)
            return MTRACKLENGTH;
    else if(strcmp(s, "MLAPINVALIDATED") == 0)
            return MLAPINVALIDATED;
    else if(strcmp(s, "MBESTLAPTIME") == 0)
            return MBESTLAPTIME;
    else if(strcmp(s, "MLASTLAPTIME") == 0)
            return MLASTLAPTIME;
    else if(strcmp(s, "MCURRENTTIME") == 0)
            return MCURRENTTIME;
    else if(strcmp(s, "MSPLITTIMEAHEAD") == 0)
            return MSPLITTIMEAHEAD;
    else if(strcmp(s, "MSPLITTIMEBEHIND") == 0)
            return MSPLITTIMEBEHIND;
    else if(strcmp(s, "MSPLITTIME") == 0)
            return MSPLITTIME;
    else if(strcmp(s, "MEVENTTIMEREMAINING") == 0)
            return MEVENTTIMEREMAINING;
    else if(strcmp(s, "MPERSONALFASTESTLAPTIME") == 0)
            return MPERSONALFASTESTLAPTIME;
    else if(strcmp(s, "MWORLDFASTESTLAPTIME") == 0)
            return MWORLDFASTESTLAPTIME;
    else if(strcmp(s, "MCURRENTSECTOR1TIME") == 0)
            return MCURRENTSECTOR1TIME;
    else if(strcmp(s, "MCURRENTSECTOR2TIME") == 0)
            return MCURRENTSECTOR2TIME;
    else if(strcmp(s, "MCURRENTSECTOR3TIME") == 0)
            return MCURRENTSECTOR3TIME;
    else if(strcmp(s, "MFASTESTSECTOR1TIME") == 0)
            return MFASTESTSECTOR1TIME;
    else if(strcmp(s, "MFASTESTSECTOR2TIME") == 0)
            return MFASTESTSECTOR2TIME;
    else if(strcmp(s, "MFASTESTSECTOR3TIME") == 0)
            return MFASTESTSECTOR3TIME;
    else if(strcmp(s, "MPERSONALFASTESTSECTOR1TIME") == 0)
            return MPERSONALFASTESTSECTOR1TIME;
    else if(strcmp(s, "MPERSONALFASTESTSECTOR2TIME") == 0)
            return MPERSONALFASTESTSECTOR2TIME;
    else if(strcmp(s, "MPERSONALFASTESTSECTOR3TIME") == 0)
            return MPERSONALFASTESTSECTOR3TIME;
    else if(strcmp(s, "MWORLDFASTESTSECTOR1TIME") == 0)
            return MWORLDFASTESTSECTOR1TIME;
    else if(strcmp(s, "MWORLDFASTESTSECTOR2TIME") == 0)
            return MWORLDFASTESTSECTOR2TIME;
    else if(strcmp(s, "MWORLDFASTESTSECTOR3TIME") == 0)
            return MWORLDFASTESTSECTOR3TIME;
    else if(strcmp(s, "MHIGHESTFLAGCOLOUR") == 0)
            return MHIGHESTFLAGCOLOUR;
    else if(strcmp(s, "MHIGHESTFLAGREASON") == 0)
            return MHIGHESTFLAGREASON;
    else if(strcmp(s, "MPITMODE") == 0)
            return MPITMODE;
    else if(strcmp(s, "MPITSCHEDULE") == 0)
            return MPITSCHEDULE;
    else if(strcmp(s, "MCARFLAGS") == 0)
            return MCARFLAGS;
    else if(strcmp(s, "MOILTEMPCELSIUS") == 0)
            return MOILTEMPCELSIUS;
    else if(strcmp(s, "MOILPRESSUREKPA") == 0)
            return MOILPRESSUREKPA;
    else if(strcmp(s, "MWATERTEMPCELSIUS") == 0)
            return MWATERTEMPCELSIUS;
    else if(strcmp(s, "MWATERPRESSUREKPA") == 0)
            return MWATERPRESSUREKPA;
    else if(strcmp(s, "MFUELPRESSUREKPA") == 0)
            return MFUELPRESSUREKPA;
    else if(strcmp(s, "MFUELLEVEL") == 0)
            return MFUELLEVEL;
    else if(strcmp(s, "MFUELCAPACITY") == 0)
            return MFUELCAPACITY ;
    else if(strcmp(s, "MSPEED") == 0)
            return MSPEED;
    else if(strcmp(s, "MRPM") == 0)
            return MRPM;
    else if(strcmp(s, "MMAXRPM") == 0)
            return MMAXRPM;
    else if(strcmp(s, "MBRAKE") == 0)
            return MBRAKE;
    else if(strcmp(s, "MTHROTTLE") == 0)
            return MTHROTTLE;
    else if(strcmp(s, "MCLUTCH") == 0)
            return MCLUTCH;
    else if(strcmp(s, "MSTEERING") == 0)
            return MSTEERING;
    else if(strcmp(s, "MGEAR") == 0)
            return MGEAR;
    else if(strcmp(s, "MNUMGEARS") == 0)
            return MNUMGEARS;
    else if(strcmp(s, "MODOMETERKM") == 0)
            return MODOMETERKM;
    else if(strcmp(s, "MANTILOCKACTIVE") == 0)
            return MANTILOCKACTIVE;
    else if(strcmp(s, "MLASTOPPONENTCOLLISIONINDEX") == 0)
            return MLASTOPPONENTCOLLISIONINDEX ;
    else if(strcmp(s, "MLASTOPPONENTCOLLISIONMAGNITUDE") == 0)
            return MLASTOPPONENTCOLLISIONMAGNITUDE ;
    else if(strcmp(s, "MBOOSTACTIVE") == 0)
            return MBOOSTACTIVE;
    else if(strcmp(s, "MBOOSTAMOUNT") == 0)
            return MBOOSTAMOUNT;
    else if(strcmp(s, "MORIENTATION") == 0)
            return MORIENTATION;
    else if(strcmp(s, "MLOCALVELOCITY") == 0)
            return MLOCALVELOCITY;
    else if(strcmp(s, "MWORLDVELOCITY") == 0)
            return MWORLDVELOCITY;
    else if(strcmp(s, "MANGULARVELOCITY") == 0)
            return MANGULARVELOCITY;
    else if(strcmp(s, "MLOCALACCELERATION") == 0)
            return MLOCALACCELERATION;
    else if(strcmp(s, "MWORLDACCELERATION") == 0)
            return MWORLDACCELERATION;
    else if(strcmp(s, "MEXTENTSCENTRE") == 0)
            return MEXTENTSCENTRE;
    else if(strcmp(s, "MTYREFLAGS") == 0)
            return MTYREFLAGS;
    else if(strcmp(s, "MTERRAIN") == 0)
            return MTERRAIN;
    else if(strcmp(s, "MTYREY") == 0)
            return MTYREY;
    else if(strcmp(s, "MTYRERPS") == 0)
            return MTYRERPS;
    else if(strcmp(s, "MTYRESLIPSPEED") == 0)
            return MTYRESLIPSPEED;
    else if(strcmp(s, "MTYRETEMP") == 0)
            return MTYRETEMP;
    else if(strcmp(s, "MTYREGRIP") == 0)
            return MTYREGRIP;
    else if(strcmp(s, "MTYREHEIGHTABOVEGROUND") == 0)
            return MTYREHEIGHTABOVEGROUND;
    else if(strcmp(s, "MTYRELATERALSTIFFNESS") == 0)
            return MTYRELATERALSTIFFNESS;
    else if(strcmp(s, "MTYREWEAR") == 0)
            return MTYREWEAR;
    else if(strcmp(s, "MBRAKEDAMAGE") == 0)
            return MBRAKEDAMAGE;
    else if(strcmp(s, "MSUSPENSIONDAMAGE") == 0)
            return MSUSPENSIONDAMAGE;
    else if(strcmp(s, "MBRAKETEMPCELSIUS") == 0)
            return MBRAKETEMPCELSIUS;
    else if(strcmp(s, "MTYRETREADTEMP") == 0)
            return MTYRETREADTEMP;
    else if(strcmp(s, "MTYRELAYERTEMP") == 0)
            return MTYRELAYERTEMP;
    else if(strcmp(s, "MTYRECARCASSTEMP") == 0)
            return MTYRECARCASSTEMP;
    else if(strcmp(s, "MTYRERIMTEMP") == 0)
            return MTYRERIMTEMP;
    else if(strcmp(s, "MTYREINTERNALAIRTEMP") == 0)
            return MTYREINTERNALAIRTEMP;
    else if(strcmp(s, "MCRASHSTATE") == 0)
            return MCRASHSTATE;
    else if(strcmp(s, "MAERODAMAGE") == 0)
            return MAERODAMAGE;
    else if(strcmp(s, "MENGINEDAMAGE") == 0)
            return MENGINEDAMAGE;
    else if(strcmp(s, "MAMBIENTTEMPERATURE") == 0)
            return MAMBIENTTEMPERATURE;
    else if(strcmp(s, "MTRACKTEMPERATURE") == 0)
            return MTRACKTEMPERATURE;
    else if(strcmp(s, "MRAINDENSITY") == 0)
            return MRAINDENSITY;
    else if(strcmp(s, "MWINDSPEED") == 0)
            return MWINDSPEED;
    else if(strcmp(s, "MWINDDIRECTIONX") == 0)
            return MWINDDIRECTIONX;
    else if(strcmp(s, "MWINDDIRECTIONY") == 0)
            return MWINDDIRECTIONY;
    else if(strcmp(s, "MCLOUDBRIGHTNESS") == 0)
            return MCLOUDBRIGHTNESS;
    else if(strcmp(s, "EXT_MSESSIONSECTORGAP") == 0)
        return EXT_MSESSIONSECTORGAP;
    else if(strcmp(s, "EXT_MSESSIONSECTORDELTA") == 0)
            return EXT_MSESSIONSECTORDELTA;
    else if(strcmp(s, "EXT_MCURRENTTIME") == 0)
            return EXT_MCURRENTTIME;
    else if(strcmp(s, "EXT_MLASTLAPTIME") == 0)
        return EXT_MLASTLAPTIME;
    else if(strcmp(s, "EXT_MPOSITION") == 0)
        return EXT_MPOSITION;
    else if(strcmp(s, "EXT_MCRASHSTATE") == 0)
        return EXT_MCRASHSTATE;
    else
        return -1;
}

char* enumPCarsFieldsToString(int e){
    switch (e) {
        case MVERSION: 
            return "MVERSION"; 
        case MBUILDVERSIONNUMBER:
                return "MBUILDVERSIONNUMBER";
        case MGAMESTATE:
                return "MGAMESTATE";
        case MSESSIONSTATE:
                return "MSESSIONSTATE";
        case MRACESTATE:
                return "MRACESTATE";
        case MVIEWEDPARTICIPANTINDEX:
                return "MVIEWEDPARTICIPANTINDEX";
        case MNUMPARTICIPANTS:
                return "MNUMPARTICIPANTS";
        case MPARTICIPANTINFO:
                return "MPARTICIPANTINFO";
        case MUNFILTEREDTHROTTLE:
                return "MUNFILTEREDTHROTTLE";
        case MUNFILTEREDBRAKE :
                return "MUNFILTEREDBRAKE ";
        case MUNFILTEREDSTEERING:
                return "MUNFILTEREDSTEERING";
        case MUNFILTEREDCLUTCH:
                return "MUNFILTEREDCLUTCH";
        case MCARNAME:
                return "MCARNAME";
        case MCARCLASSNAME:
                return "MCARCLASSNAME";
        case MLAPSINEVENT:
                return "MLAPSINEVENT";
        case MTRACKLOCATION:
                return "MTRACKLOCATION";
        case MTRACKVARIATION:
                return "MTRACKVARIATION";
        case MTRACKLENGTH:
                return "MTRACKLENGTH";
        case MLAPINVALIDATED:
                return "MLAPINVALIDATED";
        case MBESTLAPTIME:
                return "MBESTLAPTIME";
        case MLASTLAPTIME:
                return "MLASTLAPTIME";
        case MCURRENTTIME:
                return "MCURRENTTIME";
        case MSPLITTIMEAHEAD:
                return "MSPLITTIMEAHEAD";
        case MSPLITTIMEBEHIND:
                return "MSPLITTIMEBEHIND";
        case MSPLITTIME:
                return "MSPLITTIME";
        case MEVENTTIMEREMAINING:
                return "MEVENTTIMEREMAINING";
        case MPERSONALFASTESTLAPTIME:
                return "MPERSONALFASTESTLAPTIME";
        case MWORLDFASTESTLAPTIME:
                return "MWORLDFASTESTLAPTIME";
        case MCURRENTSECTOR1TIME:
                return "MCURRENTSECTOR1TIME";
        case MCURRENTSECTOR2TIME:
                return "MCURRENTSECTOR2TIME";
        case MCURRENTSECTOR3TIME:
                return "MCURRENTSECTOR3TIME";
        case MFASTESTSECTOR1TIME:
                return "MFASTESTSECTOR1TIME";
        case MFASTESTSECTOR2TIME:
                return "MFASTESTSECTOR2TIME";
        case MFASTESTSECTOR3TIME:
                return "MFASTESTSECTOR3TIME";
        case MPERSONALFASTESTSECTOR1TIME:
                return "MPERSONALFASTESTSECTOR1TIME";
        case MPERSONALFASTESTSECTOR2TIME:
                return "MPERSONALFASTESTSECTOR2TIME";
        case MPERSONALFASTESTSECTOR3TIME:
                return "MPERSONALFASTESTSECTOR3TIME";
        case MWORLDFASTESTSECTOR1TIME:
                return "MWORLDFASTESTSECTOR1TIME";
        case MWORLDFASTESTSECTOR2TIME:
                return "MWORLDFASTESTSECTOR2TIME";
        case MWORLDFASTESTSECTOR3TIME:
                return "MWORLDFASTESTSECTOR3TIME";
        case MHIGHESTFLAGCOLOUR:
                return "MHIGHESTFLAGCOLOUR";
        case MHIGHESTFLAGREASON:
                return "MHIGHESTFLAGREASON";
        case MPITMODE:
                return "MPITMODE";
        case MPITSCHEDULE:
                return "MPITSCHEDULE";
        case MCARFLAGS:
                return "MCARFLAGS";
        case MOILTEMPCELSIUS:
                return "MOILTEMPCELSIUS";
        case MOILPRESSUREKPA:
                return "MOILPRESSUREKPA";
        case MWATERTEMPCELSIUS:
                return "MWATERTEMPCELSIUS";
        case MWATERPRESSUREKPA:
                return "MWATERPRESSUREKPA";
        case MFUELPRESSUREKPA:
                return "MFUELPRESSUREKPA";
        case MFUELLEVEL:
                return "MFUELLEVEL";
        case MFUELCAPACITY :
                return "MFUELCAPACITY";
        case MSPEED:
                return "MSPEED";
        case MRPM:
                return "MRPM";
        case MMAXRPM:
                return "MMAXRPM";
        case MBRAKE:
                return "MBRAKE";
        case MTHROTTLE:
                return "MTHROTTLE";
        case MCLUTCH:
                return "MCLUTCH";
        case MSTEERING:
                return "MSTEERING";
        case MGEAR:
                return "MGEAR";
        case MNUMGEARS:
                return "MNUMGEARS";
        case MODOMETERKM:
                return "MODOMETERKM";
        case MANTILOCKACTIVE:
                return "MANTILOCKACTIVE";
        case MLASTOPPONENTCOLLISIONINDEX :
                return "MLASTOPPONENTCOLLISIONINDEX";
        case MLASTOPPONENTCOLLISIONMAGNITUDE :
                return "MLASTOPPONENTCOLLISIONMAGNITUDE";
        case MBOOSTACTIVE:
                return "MBOOSTACTIVE";
        case MBOOSTAMOUNT:
                return "MBOOSTAMOUNT";
        case MORIENTATION:
                return "MORIENTATION";
        case MLOCALVELOCITY:
                return "MLOCALVELOCITY";
        case MWORLDVELOCITY:
                return "MWORLDVELOCITY";
        case MANGULARVELOCITY:
                return "MANGULARVELOCITY";
        case MLOCALACCELERATION:
                return "MLOCALACCELERATION";
        case MWORLDACCELERATION:
                return "MWORLDACCELERATION";
        case MEXTENTSCENTRE:
                return "MEXTENTSCENTRE";
        case MTYREFLAGS:
                return "MTYREFLAGS";
        case MTERRAIN:
                return "MTERRAIN";
        case MTYREY:
                return "MTYREY";
        case MTYRERPS:
                return "MTYRERPS";
        case MTYRESLIPSPEED:
                return "MTYRESLIPSPEED";
        case MTYRETEMP:
                return "MTYRETEMP";
        case MTYREGRIP:
                return "MTYREGRIP";
        case MTYREHEIGHTABOVEGROUND:
                return "MTYREHEIGHTABOVEGROUND";
        case MTYRELATERALSTIFFNESS:
                return "MTYRELATERALSTIFFNESS";
        case MTYREWEAR:
                return "MTYREWEAR";
        case MBRAKEDAMAGE:
                return "MBRAKEDAMAGE";
        case MSUSPENSIONDAMAGE:
                return "MSUSPENSIONDAMAGE";
        case MBRAKETEMPCELSIUS:
                return "MBRAKETEMPCELSIUS";
        case MTYRETREADTEMP:
                return "MTYRETREADTEMP";
        case MTYRELAYERTEMP:
                return "MTYRELAYERTEMP";
        case MTYRECARCASSTEMP:
                return "MTYRECARCASSTEMP";
        case MTYRERIMTEMP:
                return "MTYRERIMTEMP";
        case MTYREINTERNALAIRTEMP:
                return "MTYREINTERNALAIRTEMP";
        case MCRASHSTATE:
                return "MCRASHSTATE";
        case MAERODAMAGE:
                return "MAERODAMAGE";
        case MENGINEDAMAGE:
                return "MENGINEDAMAGE";
        case MAMBIENTTEMPERATURE:
                return "MAMBIENTTEMPERATURE";
        case MTRACKTEMPERATURE:
                return "MTRACKTEMPERATURE";
        case MRAINDENSITY:
                return "MRAINDENSITY";
        case MWINDSPEED:
                return "MWINDSPEED";
        case MWINDDIRECTIONX:
                return "MWINDDIRECTIONX";
        case MWINDDIRECTIONY:
                return "MWINDDIRECTIONY";
        case MCLOUDBRIGHTNESS:
                return "MCLOUDBRIGHTNESS";
        case EXT_MSESSIONSECTORGAP:
                return "EXT_MSESSIONSECTORGAP";
        case EXT_MSESSIONSECTORDELTA:
                return "EXT_MSESSIONSECTORDELTA";
        case EXT_MCURRENTTIME:
                return "EXT_MCURRENTTIME";                        
        case EXT_MLASTLAPTIME:
            return "EXT_MLASTLAPTIME";
        case EXT_MPOSITION:
                return "EXT_MPOSITION";
        case END_PCARS_FIELDS:
                return "END_PCARS_FIELDS";
        case EXT_MCRASHSTATE:
                return "EXT_MCRASHSTATE";
        default:
            return NULL;
    }   
}




