#!/usr/bin/env python3
import os, subprocess

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

# Query ue4cli for the root directory of the UE4 source tree
proc = subprocess.Popen(['ue4', 'root'], stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
(stdout, stderr) = proc.communicate(input)
if proc.returncode != 0:
	raise Exception("failed to retrieve UE4 root directory")

# Patch the problematic #include directive in AudioMixerDevice.h
headerFile = os.path.join(stdout.strip(), 'Engine', 'Source', 'Runtime', 'AudioMixer', 'Public', 'AudioMixerDevice.h')
patchFile(headerFile, {'#include "AudioMixerSourceManager.h"': '#include "Runtime/AudioMixer/Private/AudioMixerSourceManager.h"'})
