#!/usr/bin/env python3
import argparse, subprocess, sys

# Parse our command-line arguments
parser = argparse.ArgumentParser()
parser.add_argument("--upload", default=None, help="Upload built package to the specified remote")
args = parser.parse_args()

# Query ue4cli for the UE4 version string
proc = subprocess.Popen(["ue4", "version", "short"], stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
(stdout, stderr) = proc.communicate(input)
if proc.returncode != 0:
    raise Exception("failed to retrieve UE4 version string")

# Build the Conan package, using the Engine version as the channel name
channel = stdout.strip()
if subprocess.call(["conan", "create", ".", "adamrehn/{}".format(channel), "--profile", "ue4"]) != 0:
    sys.exit(1)

# Upload the package to the specified remote if the user provided one
if args.upload != None:
    if subprocess.call(["conan", "upload", "MediaIPC-ue4/*@adamrehn/*", "--all", "--confirm", "-r", args.upload]) != 0:
        sys.exit(1)
