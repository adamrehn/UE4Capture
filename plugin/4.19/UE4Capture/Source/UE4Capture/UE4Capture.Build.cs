using System.IO;
using UnrealBuildTool;
using System.Diagnostics;

//For Tools.DotNETCommon.JsonObject and Tools.DotNETCommon.FileReference
using Tools.DotNETCommon;

public class UE4Capture : ModuleRules
{
	private string ModuleRoot {
		get { return Path.GetFullPath(Path.Combine(ModuleDirectory, "../..")); }
	}
	
	private bool IsWindows(ReadOnlyTargetRules target) {
		return (target.Platform == UnrealTargetPlatform.Win32 || Target.Platform == UnrealTargetPlatform.Win64);
	}
	
	private void ProcessDependencies(string depsJson, ReadOnlyTargetRules target)
	{
		//We need to ensure libraries end with ".lib" under Windows
		string libSuffix = ((this.IsWindows(target)) ? ".lib" : "");
		
		//Attempt to parse the JSON file
		JsonObject deps = JsonObject.Read(new FileReference(depsJson));
		
		//Process the list of dependencies
		foreach (JsonObject dep in deps.GetObjectArrayField("dependencies"))
		{
			//Add the header and library paths for the dependency package
			PublicIncludePaths.AddRange(dep.GetStringArrayField("include_paths"));
			PublicLibraryPaths.AddRange(dep.GetStringArrayField("lib_paths"));
			
			//Add the preprocessor definitions from the dependency package
			PublicDefinitions.AddRange(dep.GetStringArrayField("defines"));
			
			//Link against the libraries from the package
			string[] libs = dep.GetStringArrayField("libs");
			foreach (string lib in libs)
			{
				string libFull = lib + ((libSuffix.Length == 0 || lib.EndsWith(libSuffix)) ? "" : libSuffix);
				PublicAdditionalLibraries.Add(libFull);
			}
		}
	}
	
	public UE4Capture(ReadOnlyTargetRules Target) : base(Target)
	{
		//Link against our engine dependencies
		
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange
		(
			new string[]
			{
				"UE4Capture/Public",
				"Runtime/AudioMixer/Private"
			}
		);
		
		PrivateIncludePaths.AddRange
		(
			new string[]
			{
				"UE4Capture/Private",
				"Runtime/AudioMixer/Private"
			}
		);
		
		PublicDependencyModuleNames.AddRange
		(
			new string[] {
				"Core"
			}
		);
		
		PrivateDependencyModuleNames.AddRange
		(
			new string[]
			{
				"AudioMixer",
				"CoreUObject",
				"Engine",
				"MovieSceneCapture"
			}
		);
		
		//Install third-party dependencies using conan
		Process.Start(new ProcessStartInfo
		{
			FileName = "conan",
			Arguments = "install . --profile ue4",
			WorkingDirectory = ModuleRoot
		})
		.WaitForExit();
		
		//Link against our conan-installed dependencies
		this.ProcessDependencies(Path.Combine(ModuleRoot, "conanbuildinfo.json"), Target);
	}
}
