#!/usr/bin/env python3
import argparse, subprocess, sys, ue4cli

# Parse our command-line arguments
parser = argparse.ArgumentParser()
parser.add_argument("--upload", default=None, help="Upload built package to the specified remote")
args = parser.parse_args()

# Query ue4cli for the UE4 version string
ue4 = ue4cli.UnrealManagerFactory.create()
versionFull = ue4.getEngineVersion()
versionShort = ue4.getEngineVersion('short')
versionMinor = int(ue4.getEngineVersion('minor'))

# Verify that the detected version of UE4 is new enough
if versionMinor < 19:
    print('Error: UE4Capture requires Unreal Engine 4.19 or newer, detected version {}.'.format(versionFull), file=sys.stderr)
    sys.exit(1)

# Build the Conan package, using the short (major.minor) Engine version as the channel name
channel = versionShort
if subprocess.call(["conan", "create", ".", "adamrehn/{}".format(channel), "--profile", "ue4"]) != 0:
    sys.exit(1)

# Upload the package to the specified remote if the user provided one
if args.upload != None:
    if subprocess.call(["conan", "upload", "MediaIPC-ue4/*@adamrehn/*", "--all", "--confirm", "-r", args.upload]) != 0:
        sys.exit(1)
