#!/bin/sh
#*************************************************************************
# Copyright (c) 2002 The University of Chicago, as Operator of Argonne
#     National Laboratory.
# Copyright (c) 2002 The Regents of the University of California, as
#     Operator of Los Alamos National Laboratory.
# EPICS BASE Versions 3.13.7
# and higher are distributed subject to a Software License Agreement found
# in file LICENSE that is included with this distribution. 
#*************************************************************************
#  Site-specific EPICS environment settings
#
#  sites should modify these definitions

# Location of epics base
if [ -z "${EPICS_BASE}" ] ; then
	EPICS_BASE=/usr/local/epics/extensions
	export EPICS_BASE
fi

# Location of epics extensions
if [ -z "${EPICS_EXTENSIONS}" ] ; then
	EPICS_EXTENSIONS=/usr/local/epics/extensions
	export EPICS_EXTENSIONS
fi

# Time service:
# EPICS_TS_NTP_INET ntp or Unix time server ip addr.

# Postscript printer definition needed by some extensions (eg medm, dp, dm, ...)
PSPRINTER=lp
export PSPRINTER

# Needed only by medm extension
#setenv EPICS_DISPLAY_PATH
#export EPICS_DISPLAY_PATH

# Needed only by orbitscreen extension
if [ -z "${ORBITSCREENHOME}" ] ; then
	ORBITSCREENHOME=$EPICS_EXTENSIONS/src/orbitscreen
	export ORBITSCREENHOME
fi

# Needed only by adt extension
#if [ -z "${ADTHOME}" ] ; then
#	ADTHOME=
#	export ADTHOME
#fi

# Needed only by ar extension (archiver)
#EPICS_AR_PORT=7002
#export EPICS_AR_PORT

# Needed for java extensions
if [ -z "${CLASSPATH}" ] ; then
	CLASSPATH="${EPICS_EXTENSIONS}/javalib"
else
	CLASSPATH="${CLASSPATH}:${EPICS_EXTENSIONS}/javalib"
fi

# Allow private versions of extensions without a bin subdir
if [ -n "${EPICS_EXTENSIONS_PVT}" ] ; then
	PATH="${PATH}:${EPICS_EXTENSIONS_PVT}"
fi

#---------------------------------------------------------------
# Start of set R3.14 environment variables

EPICS_HOST_ARCH=`/usr/local/epics/startup/EpicsHostArch.pl`
export EPICS_HOST_ARCH

# Allow private versions of base
if [ -n "${EPICS_BASE_PVT}" ] ; then
	if [ -d $EPICS_BASE_PVT/bin/$EPICS_HOST_ARCH ]; then
		PATH="${PATH}:${EPICS_BASE_PVT}/bin/${EPICS_HOST_ARCH}"
	fi
	# Needed if shared extension libraries are built
	if [ -d $EPICS_BASE_PVT/lib/$EPICS_HOST_ARCH ]; then
		if [ -z "${LD_LIBRARY_PATH}" ] ; then
			LD_LIBRARY_PATH="${EPICS_BASE_PVT}/lib/${EPICS_HOST_ARCH}"
		else
			LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${EPICS_BASE_PVT}/lib/${EPICS_HOST_ARCH}"
		fi
	fi
fi

PATH="${PATH}:${EPICS_BASE}/bin/${EPICS_HOST_ARCH}"

# Allow private versions of extensions
if [ -n "${EPICS_EXTENSIONS_PVT}" ] ; then
	if [ -d $EPICS_EXTENSIONS_PVT/bin/$EPICS_HOST_ARCH ]; then
		PATH="${PATH}:${EPICS_EXTENSIONS_PVT}/bin/${EPICS_HOST_ARCH}"
	fi
	# Needed if shared extension libraries are built
	if [ -d $EPICS_EXTENSIONS_PVT/lib/$EPICS_HOST_ARCH ]; then
		if [ -z "${LD_LIBRARY_PATH}" ] ; then
			LD_LIBRARY_PATH="${EPICS_EXTENSIONS_PVT}/lib/${EPICS_HOST_ARCH}"
		else
			LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${EPICS_EXTENSIONS_PVT}/lib/${EPICS_HOST_ARCH}"
		fi
	fi
fi

PATH="${PATH}:${EPICS_EXTENSIONS}/bin/${EPICS_HOST_ARCH}"

# Needed if shared base libraries are built
if [ -z "${LD_LIBRARY_PATH}" ] ; then
	LD_LIBRARY_PATH="${EPICS_BASE}/lib/${EPICS_HOST_ARCH}"
else
	LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${EPICS_BASE}/lib/${EPICS_HOST_ARCH}"
fi

# Needed if shared extension libraries are built
if [ -z "${LD_LIBRARY_PATH}" ] ; then
	LD_LIBRARY_PATH="${EPICS_EXTENSIONS}/lib/${EPICS_HOST_ARCH}"
else
	LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${EPICS_EXTENSIONS}/lib/${EPICS_HOST_ARCH}"
fi

# End of set R3.14 environment variables

#---------------------------------------------------------------

# Start of set pre R3.14 environment variables

# Time service:
# EPICS_TS_MIN_WEST the local time difference from GMT.
EPICS_TS_MIN_WEST=360
export EPICS_TS_MIN_WEST

HOST_ARCH=`/usr/local/epics/startup/HostArch.pl`
export HOST_ARCH

# Allow private versions of base
if [ -n "${EPICS_BASE_PVT}" ] ; then
	if [ -d $EPICS_BASE_PVT/bin/$HOST_ARCH ]; then
		PATH="${PATH}:${EPICS_BASE_PVT}/bin/${HOST_ARCH}"
	fi
	# Needed if shared extension libraries are built
	if [ -d $EPICS_BASE_PVT/lib/$HOST_ARCH ]; then
		if [ -z "${LD_LIBRARY_PATH}" ] ; then
			LD_LIBRARY_PATH="${EPICS_BASE_PVT}/lib/${HOST_ARCH}"
		else
			LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${EPICS_BASE_PVT}/lib/${HOST_ARCH}"
		fi
	fi
fi

PATH="${PATH}:${EPICS_BASE}/lib/${HOST_ARCH}"
# Needed if shared base libraries are built
LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${EPICS_BASE}/lib/${HOST_ARCH}"

# Allow private versions of extensions
if [ -n "${EPICS_EXTENSIONS_PVT}" ] ; then
	if [ -d $EPICS_EXTENSIONS_PVT/bin/$HOST_ARCH ]; then
		PATH="${PATH}:${EPICS_EXTENSIONS_PVT}/bin/${HOST_ARCH}"
	fi
	# Needed if shared extension libraries are built
	if [ -d $EPICS_EXTENSIONS_PVT/lib/$HOST_ARCH ]; then
		if [ -z "${LD_LIBRARY_PATH}" ] ; then
			LD_LIBRARY_PATH="${EPICS_EXTENSIONS_PVT}/lib/${HOST_ARCH}"
		else
			LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${EPICS_EXTENSIONS_PVT}/lib/${HOST_ARCH}"
		fi
	fi
fi

PATH="${PATH}:${EPICS_EXTENSIONS}/lib/${HOST_ARCH}"
# Needed if shared extension libraries are built
LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${EPICS_EXTENSIONS}/lib/${HOST_ARCH}"

# End of set pre R3.14 environment variables

#---------------------------------------------------------------

export PATH
export LD_LIBRARY_PATH
export CLASSPATH

