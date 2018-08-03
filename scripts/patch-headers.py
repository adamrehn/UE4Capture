#!/usr/bin/env python3
import os, ue4cli

# Reads data from a file
def readFile(filename):
	with open(filename, 'rb') as f:
		return f.read().decode('utf-8')

# Writes data to a file
def writeFile(filename, data):
	with open(filename, 'wb') as f:
		f.write(data.encode('utf-8'))

# Applies the supplied list of replacements to a file
def patchFile(filename, replacements):
	patched = readFile(filename)
	for key in replacements:
		patched = patched.replace(key, replacements[key])
	writeFile(filename, patched)
	print('Patched file "{}".'.format(filename))

# Query ue4cli for the UE4 version string and the root directory of the UE4 source tree
ue4 = ue4cli.UnrealManagerFactory.create()
versionMinor = int(ue4.getEngineVersion('minor'))
engineRoot = ue4.getEngineRoot()

# We only need to patch the headers for 4.19.x
if versionMinor == 19:
	
	# Patch the problematic #include directive in AudioMixerDevice.h
	headerFile = os.path.join(engineRoot, 'Engine', 'Source', 'Runtime', 'AudioMixer', 'Public', 'AudioMixerDevice.h')
	patchFile(headerFile, {'#include "AudioMixerSourceManager.h"': '#include "Runtime/AudioMixer/Private/AudioMixerSourceManager.h"'})
