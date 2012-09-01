/*******************************************************************************
 *
 * Copyright (c) 2008, 2009 Michael Schulze <mschulze@ivs.cs.uni-magdeburg.de>
 * All rights reserved.
 *
 *    Redistribution and use in source and binary forms, with or without
 *    modification, are permitted provided that the following conditions
 *    are met:
 *
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *    * Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in
 *      the documentation and/or other materials provided with the
 *      distribution.
 *
 *    * Neither the name of the copyright holders nor the names of
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *
 *
 *    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 *    IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 *    TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 *    PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *    OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * $Id: logging.h 1 2010-01-11 14:24:28Z perch $
 *
 ******************************************************************************/

#ifndef __logging_h__
#define __logging_h__

#include "logging/NullOutput.h"
#include "logging/OutputStream.h"
#include "logging/OutputLevelSwitchDisabled.h"
#include "logging/OutputLevelRunTimeSwitch.h"
#include "logging/Logger.h"

#ifndef LOGGING_DEFINE_OWN_OUTPUT_TYPE

#ifdef LOGGING_DISABLE

/*! \brief define NullOutput as output %device */
LOGGING_DEFINE_OUTPUT( ::logging::NullOutput )

// undefine the macro and redefine it as empty to switch off all
// user created output types and ensure that only the NullOutput
// is used as logging type
#undef LOGGING_DEFINE_OUTPUT
#define LOGGING_DEFINE_OUTPUT(NAME)

#else

#ifdef __AVR__
#include "logging/loggingConfigAVR.h"
#else
#include "logging/loggingConfigGeneralPurposeOS.h"
#endif /* __AVR__ */


#ifndef LOGGING_DEFINE_EXTENDED_OUTPUT_TYPE
LOGGING_DEFINE_OUTPUT( ::logging::LoggingType )
#endif /* LOGGING_DEFINE_EXTENDED_OUTPUT_TYPE */

#endif /* LOGGING_DISABLE */

#endif /* LOGGING_DEFINE_OWN_OUTPUT_TYPE */

#endif /* __logging_h__ */
