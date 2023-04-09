/**
 * OpenAL cross platform audio library
 * Copyright (C) 1999-2003 by authors.
 * This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the
 *  Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA  02111-1307, USA.
 * Or go to http://www.gnu.org/copyleft/lgpl.html
 */




#include "..\..\include\al\alc.h"
#include "OpenAL32.h"

typedef struct ALfunction_struct
{
	ALchar		*funcName;
	ALvoid		*address;
} ALfunction;

static ALfunction  function[]=   {	
	{ (ALchar*)"alEnable",                   (ALvoid *) alEnable                  },
	{ (ALchar*)"alDisable",                  (ALvoid *) alDisable                 },
	{ (ALchar*)"alIsEnabled",                (ALvoid *) alIsEnabled               },
	{ (ALchar*)"alGetString",                (ALvoid *) alGetString               },
	{ (ALchar*)"alGetBooleanv",              (ALvoid *) alGetBooleanv             },
	{ (ALchar*)"alGetIntegerv",              (ALvoid *) alGetIntegerv             },
	{ (ALchar*)"alGetFloatv",                (ALvoid *) alGetFloatv               },
	{ (ALchar*)"alGetDoublev",               (ALvoid *) alGetDoublev              },
	{ (ALchar*)"alGetBoolean",               (ALvoid *) alGetBoolean              },
	{ (ALchar*)"alGetInteger",               (ALvoid *) alGetInteger              },
	{ (ALchar*)"alGetFloat",                 (ALvoid *) alGetFloat                },
	{ (ALchar*)"alGetDouble",                (ALvoid *) alGetDouble               },
	{ (ALchar*)"alGetError",                 (ALvoid *) alGetError                },
	{ (ALchar*)"alIsExtensionPresent",       (ALvoid *) alIsExtensionPresent      },
	{ (ALchar*)"alGetProcAddress",           (ALvoid *) alGetProcAddress          },
	{ (ALchar*)"alGetEnumValue",             (ALvoid *) alGetEnumValue            },
	{ (ALchar*)"alListenerf",                (ALvoid *) alListenerf               },
	{ (ALchar*)"alListener3f",               (ALvoid *) alListener3f              },
	{ (ALchar*)"alListenerfv",               (ALvoid *) alListenerfv              },
	{ (ALchar*)"alListeneri",                (ALvoid *) alListeneri               },
	{ (ALchar*)"alListener3i",               (ALvoid *) alListener3i              },
	{ (ALchar*)"alListeneriv",               (ALvoid *) alListeneriv              },
	{ (ALchar*)"alGetListenerf",             (ALvoid *) alGetListenerf            },
	{ (ALchar*)"alGetListener3f",            (ALvoid *) alGetListener3f           },
	{ (ALchar*)"alGetListenerfv",            (ALvoid *) alGetListenerfv           },
	{ (ALchar*)"alGetListeneri",             (ALvoid *) alGetListeneri            },
	{ (ALchar*)"alGetListener3i",            (ALvoid *) alGetListener3i           },
	{ (ALchar*)"alGetListeneriv",            (ALvoid *) alGetListeneriv           },
	{ (ALchar*)"alGenSources",               (ALvoid *) alGenSources              },
	{ (ALchar*)"alDeleteSources",            (ALvoid *) alDeleteSources           },
	{ (ALchar*)"alIsSource",                 (ALvoid *) alIsSource                },
	{ (ALchar*)"alSourcef",                  (ALvoid *) alSourcef                 },
	{ (ALchar*)"alSource3f",                 (ALvoid *) alSource3f                },
	{ (ALchar*)"alSourcefv",                 (ALvoid *) alSourcefv                },
	{ (ALchar*)"alSourcei",                  (ALvoid *) alSourcei                 },
	{ (ALchar*)"alSource3i",                 (ALvoid *) alSource3i                },
	{ (ALchar*)"alSourceiv",                 (ALvoid *) alSourceiv                },
	{ (ALchar*)"alGetSourcef",               (ALvoid *) alGetSourcef              },
	{ (ALchar*)"alGetSource3f",              (ALvoid *) alGetSource3f             },
	{ (ALchar*)"alGetSourcefv",              (ALvoid *) alGetSourcefv             },
	{ (ALchar*)"alGetSourcei",               (ALvoid *) alGetSourcei              },
	{ (ALchar*)"alGetSource3i",              (ALvoid *) alGetSource3i             },
	{ (ALchar*)"alGetSourceiv",              (ALvoid *) alGetSourceiv             },
	{ (ALchar*)"alSourcePlayv",              (ALvoid *) alSourcePlayv             },
	{ (ALchar*)"alSourceStopv",              (ALvoid *) alSourceStopv             },
	{ (ALchar*)"alSourceRewindv",            (ALvoid *) alSourceRewindv           },
	{ (ALchar*)"alSourcePausev",             (ALvoid *) alSourcePausev            },
	{ (ALchar*)"alSourcePlay",               (ALvoid *) alSourcePlay              },
	{ (ALchar*)"alSourceStop",               (ALvoid *) alSourceStop              },
	{ (ALchar*)"alSourceRewind",             (ALvoid *) alSourceRewind            },
	{ (ALchar*)"alSourcePause",              (ALvoid *) alSourcePause             },
	{ (ALchar*)"alSourceQueueBuffers",       (ALvoid *) alSourceQueueBuffers      },
	{ (ALchar*)"alSourceUnqueueBuffers",     (ALvoid *) alSourceUnqueueBuffers    },
	{ (ALchar*)"alGenBuffers",               (ALvoid *) alGenBuffers              },
	{ (ALchar*)"alDeleteBuffers",            (ALvoid *) alDeleteBuffers           },
	{ (ALchar*)"alIsBuffer",                 (ALvoid *) alIsBuffer                },
	{ (ALchar*)"alBufferData",               (ALvoid *) alBufferData              },
	{ (ALchar*)"alBufferf",                  (ALvoid *) alBufferf                 },
	{ (ALchar*)"alBuffer3f",                 (ALvoid *) alBuffer3f                },
	{ (ALchar*)"alBufferfv",                 (ALvoid *) alBufferfv                },
	{ (ALchar*)"alBufferi",                  (ALvoid *) alBufferi                 },
	{ (ALchar*)"alBuffer3i",                 (ALvoid *) alBuffer3i                },
	{ (ALchar*)"alBufferiv",                 (ALvoid *) alBufferiv                },
	{ (ALchar*)"alGetBufferf",               (ALvoid *) alGetBufferf              },
	{ (ALchar*)"alGetBuffer3f",              (ALvoid *) alGetBuffer3f             },
	{ (ALchar*)"alGetBufferfv",              (ALvoid *) alGetBufferfv             },
	{ (ALchar*)"alGetBufferi",               (ALvoid *) alGetBufferi              },
	{ (ALchar*)"alGetBuffer3i",              (ALvoid *) alGetBuffer3i             },
	{ (ALchar*)"alGetBufferiv",              (ALvoid *) alGetBufferiv             },
	{ (ALchar*)"alDopplerFactor",            (ALvoid *) alDopplerFactor           },
	{ (ALchar*)"alDopplerVelocity",          (ALvoid *) alDopplerVelocity         },
	{ (ALchar*)"alSpeedOfSound",             (ALvoid *) alSpeedOfSound            },
	{ (ALchar*)"alDistanceModel",            (ALvoid *) alDistanceModel           },
	{ nullptr,						          (ALvoid *) nullptr                      } };


//*****************************************************************************
//*****************************************************************************
//
// Defines
//
//*****************************************************************************
//*****************************************************************************


#ifdef __MINGW32__
// fix for Mingw32.
#define AL_VOID_FXN(fxn)                                                    \
    ALCcontext* context;                                                     \
                                                                            \
    alListAcquireLock(alContextList);                                       \
    if(!alCurrentContext)                                                   \
    {                                                                       \
        alListReleaseLock(alContextList);                                   \
        return;                                                             \
    }                                                                       \
                                                                            \
    context = alCurrentContext;                                             \
    EnterCriticalSection(&context->Lock);                                   \
    alListReleaseLock(alContextList);                                       \
                                                                            \
    context->AlApi.fxn;                                                   \
    LeaveCriticalSection(&context->Lock);                                   \
    return
#define AL_RESULT_FXN(fxn, resultType, resultDefVal)                        \
    resultType result = resultDefVal;                                       \
    ALCcontext* context;                                                     \
                                                                            \
    alListAcquireLock(alContextList);                                       \
    if(!alCurrentContext)                                                   \
    {                                                                       \
        alListReleaseLock(alContextList);                                   \
        return result;                                                      \
    }                                                                       \
                                                                            \
    context = alCurrentContext;                                             \
    EnterCriticalSection(&context->Lock);                                   \
    alListReleaseLock(alContextList);                                       \
                                                                            \
    result = context->AlApi.fxn;                                          \
    LeaveCriticalSection(&context->Lock);                                   \
    return result	

#else
#define AL_RESULT_FXN(fxn, resultType, resultDefVal)                        \
    resultType result = resultDefVal;                                       \
    ALCcontext* context;                                                     \
                                                                            \
    alListAcquireLock(alContextList);                                       \
    if(!alCurrentContext)                                                   \
    {                                                                       \
        alListReleaseLock(alContextList);                                   \
        return result;                                                      \
    }                                                                       \
                                                                            \
    context = alCurrentContext;                                             \
    EnterCriticalSection(&context->Lock);                                   \
    alListReleaseLock(alContextList);                                       \
                                                                            \
    result = context->AlApi.##fxn;                                          \
    LeaveCriticalSection(&context->Lock);                                   \
    return result

#define AL_VOID_FXN(fxn)                                                    \
    ALCcontext* context;                                                     \
                                                                            \
    alListAcquireLock(alContextList);                                       \
    if(!alCurrentContext)                                                   \
    {                                                                       \
        alListReleaseLock(alContextList);                                   \
        return;                                                             \
    }                                                                       \
                                                                            \
    context = alCurrentContext;                                             \
    EnterCriticalSection(&context->Lock);                                   \
    alListReleaseLock(alContextList);                                       \
                                                                            \
    context->AlApi.##fxn;                                                   \
    LeaveCriticalSection(&context->Lock);                                   \
    return
#endif


//*****************************************************************************
//*****************************************************************************
//
// AL API Buffer Entry Points
//
//*****************************************************************************
//*****************************************************************************

//*****************************************************************************
// alGenBuffers
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alGenBuffers(ALsizei n, ALuint* bufferNames)
{
    AL_VOID_FXN(alGenBuffers(n, bufferNames));
}


//*****************************************************************************
// alDeleteBuffers
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alDeleteBuffers(ALsizei n, const ALuint* bufferNames)
{
    AL_VOID_FXN(alDeleteBuffers(n, bufferNames));
}


//*****************************************************************************
// alIsBuffer
//*****************************************************************************
//
ALAPI ALboolean ALAPIENTRY alIsBuffer(ALuint bufferName)
{
    AL_RESULT_FXN(alIsBuffer(bufferName), ALboolean, AL_FALSE);
}

//*****************************************************************************
// alBuffer3f
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alBuffer3f(ALuint bufferName, ALenum param, ALfloat v1, ALfloat v2, ALfloat v3)
{
    AL_VOID_FXN(alBuffer3f(bufferName, param, v1, v2, v3));
}

//*****************************************************************************
// alBuffer3i
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alBuffer3i(ALuint bufferName, ALenum param, ALint v1, ALint v2, ALint v3)
{
    AL_VOID_FXN(alBuffer3i(bufferName, param, v1, v2, v3));
}


//*****************************************************************************
// alBufferData
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alBufferData(ALuint bufferName, ALenum format, const ALvoid* data, ALsizei size, ALsizei freq)
{
    AL_VOID_FXN(alBufferData(bufferName, format, data, size, freq));
}


//*****************************************************************************
// alBufferf
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alBufferf(ALuint bufferName, ALenum param, ALfloat value)
{
    AL_VOID_FXN(alBufferf(bufferName, param, value));
}


//*****************************************************************************
// alBufferfv
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alBufferfv(ALuint bufferName, ALenum param, const ALfloat* values)
{
    AL_VOID_FXN(alBufferfv(bufferName, param, values));
}


//*****************************************************************************
// alBufferi
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alBufferi(ALuint bufferName, ALenum param, ALint value)
{
    AL_VOID_FXN(alBufferi(bufferName, param, value));
}


//*****************************************************************************
// alBufferiv
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alBufferiv(ALuint bufferName, ALenum param, const ALint* values)
{
    AL_VOID_FXN(alBufferiv(bufferName, param, values));
}

//*****************************************************************************
// alGetBuffer3f
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alGetBuffer3f(ALuint bufferName, ALenum param, ALfloat *v1, ALfloat *v2, ALfloat *v3)
{
    AL_VOID_FXN(alGetBuffer3f(bufferName, param, v1, v2, v3));
}

//*****************************************************************************
// alGetBuffer3i
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alGetBuffer3i(ALuint bufferName, ALenum param, ALint *v1, ALint *v2, ALint *v3)
{
    AL_VOID_FXN(alGetBuffer3i(bufferName, param, v1, v2, v3));
}

//*****************************************************************************
// alGetBufferf
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alGetBufferf(ALuint bufferName, ALenum param, ALfloat* value)
{
    AL_VOID_FXN(alGetBufferf(bufferName, param, value));
}

//*****************************************************************************
// alGetBufferfv
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alGetBufferfv(ALuint bufferName, ALenum param, ALfloat* values)
{
    AL_VOID_FXN(alGetBufferfv(bufferName, param, values));
}


//*****************************************************************************
// alGetBufferi
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alGetBufferi(ALuint bufferName, ALenum param, ALint* value)
{
    AL_VOID_FXN(alGetBufferi(bufferName, param, value));
}

//*****************************************************************************
// alGetBufferiv
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alGetBufferiv(ALuint bufferName, ALenum param, ALint* values)
{
    AL_VOID_FXN(alGetBufferiv(bufferName, param, values));
}


//*****************************************************************************
//*****************************************************************************
//
// AL API Generic Entry Points
//
//*****************************************************************************
//*****************************************************************************

//*****************************************************************************
// alEnable
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alEnable(ALenum capability)
{
    AL_VOID_FXN(alEnable(capability));
}


//*****************************************************************************
// alDisable
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alDisable(ALenum capability)
{
    AL_VOID_FXN(alDisable(capability));
}


//*****************************************************************************
// alDopplerFactor
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alDopplerFactor(ALfloat value)
{
    AL_VOID_FXN(alDopplerFactor(value));
}


//*****************************************************************************
// alDopplerVelocity
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alDopplerVelocity(ALfloat value)
{
    AL_VOID_FXN(alDopplerVelocity(value));
}


//*****************************************************************************
// alSpeedOfSound
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alSpeedOfSound(ALfloat value)
{
    ALCcontext* context;

    alListAcquireLock(alContextList);
    if(!alCurrentContext)
    {
        alListReleaseLock(alContextList);
        return;
    }

    context = alCurrentContext;
    EnterCriticalSection(&context->Lock);
    alListReleaseLock(alContextList);

	if (context->AlApi.alSpeedOfSound) { // protect against talking to a 1.0 lib
		context->AlApi.alSpeedOfSound(value);
	}
    LeaveCriticalSection(&context->Lock);
    return;
}


//*****************************************************************************
// alDistanceModel
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alDistanceModel(ALenum value)
{
    AL_VOID_FXN(alDistanceModel(value));
}


//*****************************************************************************
// alGetBoolean
//*****************************************************************************
//
ALAPI ALboolean ALAPIENTRY alGetBoolean(ALenum param)
{
    AL_RESULT_FXN(alGetBoolean(param), ALboolean, AL_FALSE);
}


//*****************************************************************************
// alGetBooleanv
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alGetBooleanv(ALenum param, ALboolean* data)
{
    AL_VOID_FXN(alGetBooleanv(param, data));
}


//*****************************************************************************
// alGetDouble
//*****************************************************************************
//
ALAPI ALdouble ALAPIENTRY alGetDouble(ALenum param)
{
    AL_RESULT_FXN(alGetDouble(param), ALdouble, 0.0);
}


//*****************************************************************************
// alGetDoublev
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alGetDoublev(ALenum param, ALdouble* data)
{
    AL_VOID_FXN(alGetDoublev(param, data));
}

//*****************************************************************************
// alGetFloat
//*****************************************************************************
//
ALAPI ALfloat ALAPIENTRY alGetFloat(ALenum param)
{
    AL_RESULT_FXN(alGetFloat(param), ALfloat, 0.0f);
}


//*****************************************************************************
// alGetFloatv
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alGetFloatv(ALenum param, ALfloat* data)
{
    AL_VOID_FXN(alGetFloatv(param, data));
}


//*****************************************************************************
// alGetInteger
//*****************************************************************************
//
ALAPI ALint ALAPIENTRY alGetInteger(ALenum param)
{
    AL_RESULT_FXN(alGetInteger(param), ALint, 0);
}


//*****************************************************************************
// alGetIntegerv
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alGetIntegerv(ALenum param, ALint* data)
{
    AL_VOID_FXN(alGetIntegerv(param, data));
}


//*****************************************************************************
// alGetEnumValue
//*****************************************************************************
//
ALAPI ALenum ALAPIENTRY alGetEnumValue(const ALCchar* ename)
{
    AL_RESULT_FXN(alGetEnumValue(ename), ALenum, AL_INVALID_ENUM);
}


//*****************************************************************************
// alGetError
//*****************************************************************************
//
ALAPI ALenum ALAPIENTRY alGetError(ALvoid)
{
    AL_RESULT_FXN(alGetError(), ALenum, AL_NO_ERROR);
}


//*****************************************************************************
// alGetProcAddress
//*****************************************************************************
//
ALAPI ALvoid* ALAPIENTRY alGetProcAddress(const ALCchar* fname)
{
	// return router's address if available
	ALsizei i=0;
	ALvoid *pAddress;

	while ((function[i].funcName)&&(strcmp((char *)function[i].funcName,(char *)fname)))
		i++;
	pAddress = function[i].address;

	if (pAddress != NULL) {
		return pAddress;
	}

	// router doesn't have this entry point, so go to the device...
	AL_RESULT_FXN(alGetProcAddress(fname), ALvoid*, 0);

	return pAddress;
}


//*****************************************************************************
// alGetString
//*****************************************************************************
//
ALAPI const ALCchar* ALAPIENTRY alGetString(ALenum param)
{
    AL_RESULT_FXN(alGetString(param), const ALCchar*, 0);
}


//*****************************************************************************
// alIsExtensionPresent
//*****************************************************************************
//
ALAPI ALboolean ALAPIENTRY alIsExtensionPresent(const ALCchar* ename)
{
    AL_RESULT_FXN(alIsExtensionPresent(ename), ALboolean, AL_FALSE);
}


//*****************************************************************************
// alIsEnabled
//*****************************************************************************
//
ALAPI ALboolean ALAPIENTRY alIsEnabled(ALenum capability)
{
    AL_RESULT_FXN(alIsEnabled(capability), ALboolean, AL_FALSE);
}



//*****************************************************************************
//*****************************************************************************
//
// AL API Listener Entry Points
//
//*****************************************************************************
//*****************************************************************************

//*****************************************************************************
// alListenerf
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alListenerf(ALenum param, ALfloat value)
{
    AL_VOID_FXN(alListenerf(param, value));
}


//*****************************************************************************
// alListener3f
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alListener3f(ALenum param, ALfloat v1, ALfloat v2, ALfloat v3)
{
    AL_VOID_FXN(alListener3f(param, v1, v2, v3));
}


//*****************************************************************************
// alListener3i
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alListener3i(ALenum param, ALint v1, ALint v2, ALint v3)
{
    AL_VOID_FXN(alListener3i(param, v1, v2, v3));
}


//*****************************************************************************
// alListenerfv
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alListenerfv(ALenum param, const ALfloat* values)
{
    AL_VOID_FXN(alListenerfv(param, values));
}


//*****************************************************************************
// alListeneri
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alListeneri(ALenum param, ALint value)
{
    AL_VOID_FXN(alListeneri(param, value));
}


//*****************************************************************************
// alListeneriv
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alListeneriv(ALenum param, const ALint *values)
{
    AL_VOID_FXN(alListeneriv(param, values));
}


//*****************************************************************************
// alGetListenerf
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alGetListenerf(ALenum param, ALfloat* value)
{
    AL_VOID_FXN(alGetListenerf(param, value));
}


//*****************************************************************************
// alGetListener3f
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alGetListener3f(ALenum param, ALfloat* v1, ALfloat* v2, ALfloat* v3)
{
    AL_VOID_FXN(alGetListener3f(param, v1, v2, v3));
}


//*****************************************************************************
// alGetListener3i
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alGetListener3i(ALenum param, ALint* v1, ALint* v2, ALint* v3)
{
    AL_VOID_FXN(alGetListener3i(param, v1, v2, v3));
}


//*****************************************************************************
// alGetListenerfv
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alGetListenerfv(ALenum param, ALfloat* values)
{
    AL_VOID_FXN(alGetListenerfv(param, values));
}


//*****************************************************************************
// alGetListeneri
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alGetListeneri(ALenum param, ALint* value)
{
    AL_VOID_FXN(alGetListeneri(param, value));
}


//*****************************************************************************
// alGetListeneriv
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alGetListeneriv(ALenum param, ALint* values)
{
    AL_VOID_FXN(alGetListeneriv(param, values));
}


//*****************************************************************************
//*****************************************************************************
//
// AL API Source Entry Points
//
//*****************************************************************************
//*****************************************************************************


//*****************************************************************************
// alGenSources
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alGenSources(ALsizei n, ALuint* sourceNames)
{
    AL_VOID_FXN(alGenSources(n, sourceNames));
}


//*****************************************************************************
// alDeleteSources
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alDeleteSources(ALsizei n, const ALuint* sourceNames)
{
    AL_VOID_FXN(alDeleteSources(n, sourceNames));
}


//*****************************************************************************
// alIsSource
//*****************************************************************************
//
ALAPI ALboolean ALAPIENTRY alIsSource(ALuint sourceName)
{
    AL_RESULT_FXN(alIsSource(sourceName), ALboolean, AL_FALSE);
}


//*****************************************************************************
// alSourcef
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alSourcef(ALuint sourceName, ALenum param, ALfloat value)
{
    AL_VOID_FXN(alSourcef(sourceName, param, value));
}


//*****************************************************************************
// alSourcefv
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alSourcefv(ALuint sourceName, ALenum param, const ALfloat* values)
{
    AL_VOID_FXN(alSourcefv(sourceName, param, values));
}


//*****************************************************************************
// alSource3f
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alSource3f(ALuint sourceName, ALenum param, ALfloat v1, ALfloat v2, ALfloat v3)
{
    AL_VOID_FXN(alSource3f(sourceName, param, v1, v2, v3));
}


//*****************************************************************************
// alSource3i
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alSource3i(ALuint sourceName, ALenum param, ALint v1, ALint v2, ALint v3)
{
    AL_VOID_FXN(alSource3i(sourceName, param, v1, v2, v3));
}


//*****************************************************************************
// alSourcei
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alSourcei(ALuint sourceName, ALenum param, ALint value)
{
    AL_VOID_FXN(alSourcei(sourceName, param, value));
}

//*****************************************************************************
// alSourceiv
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alSourceiv(ALuint sourceName, ALenum param, const ALint* values)
{
    AL_VOID_FXN(alSourceiv(sourceName, param, values));
}


//*****************************************************************************
// alGetSourcef
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alGetSourcef(ALuint sourceName, ALenum param, ALfloat* value)
{
    AL_VOID_FXN(alGetSourcef(sourceName, param, value));
}

//*****************************************************************************
// alGetSource3f
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alGetSource3f(ALuint sourceName, ALenum param, ALfloat* v1, ALfloat* v2, ALfloat* v3)
{
    AL_VOID_FXN(alGetSource3f(sourceName, param, v1, v2, v3));
}


//*****************************************************************************
// alGetSource3i
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alGetSource3i(ALuint sourceName, ALenum param, ALint* v1, ALint* v2, ALint* v3)
{
    AL_VOID_FXN(alGetSource3i(sourceName, param, v1, v2, v3));
}


//*****************************************************************************
// alGetSourcefv
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alGetSourcefv(ALuint sourceName, ALenum param, ALfloat* values)
{
    AL_VOID_FXN(alGetSourcefv(sourceName, param, values));
}


//*****************************************************************************
// alGetSourcei
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alGetSourcei(ALuint sourceName, ALenum param, ALint* value)
{
    AL_VOID_FXN(alGetSourcei(sourceName, param, value));
}


//*****************************************************************************
// alGetSourceiv
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alGetSourceiv(ALuint sourceName, ALenum param, ALint* values)
{
    AL_VOID_FXN(alGetSourceiv(sourceName, param, values));
}


//*****************************************************************************
// alSourcePlay
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alSourcePlay(ALuint sourceName)
{
    AL_VOID_FXN(alSourcePlay(sourceName));
}


//*****************************************************************************
// alSourcePlayv
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alSourcePlayv(ALsizei n, const ALuint* sourceNames)
{
    AL_VOID_FXN(alSourcePlayv(n, sourceNames));
}


//*****************************************************************************
// alSourcePause
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alSourcePause(ALuint sourceName)
{
    AL_VOID_FXN(alSourcePause(sourceName));
}


//*****************************************************************************
// alSourcePausev
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alSourcePausev(ALsizei n, const ALuint* sourceNames)
{
    AL_VOID_FXN(alSourcePausev(n, sourceNames));
}


//*****************************************************************************
// alSourceStop
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alSourceStop(ALuint sourceName)
{
    AL_VOID_FXN(alSourceStop(sourceName));
}


//*****************************************************************************
// alSourceStopv
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alSourceStopv(ALsizei n, const ALuint* sourceNames)
{
    AL_VOID_FXN(alSourceStopv(n, sourceNames));
}


//*****************************************************************************
// alSourceRewind
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alSourceRewind(ALuint sourceName)
{
    AL_VOID_FXN(alSourceRewind(sourceName));
}


//*****************************************************************************
// alSourceRewindv
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alSourceRewindv(ALsizei n, const ALuint* sourceNames)
{
    AL_VOID_FXN(alSourceRewindv(n, sourceNames));
}


//*****************************************************************************
// alSourceQueueBuffers
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alSourceQueueBuffers(ALuint sourceName, ALsizei n, const ALuint* buffers)
{
    AL_VOID_FXN(alSourceQueueBuffers(sourceName, n, buffers));
}


//*****************************************************************************
// alSourceUnqueueBuffers
//*****************************************************************************
//
ALAPI ALvoid ALAPIENTRY alSourceUnqueueBuffers(ALuint sourceName, ALsizei n, ALuint* buffers)
{
    AL_VOID_FXN(alSourceUnqueueBuffers(sourceName, n, buffers));
}

