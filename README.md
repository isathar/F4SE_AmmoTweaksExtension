F4SE AmmoTweaks Extension
=========================


WIP plugin and script source files adding the ability to influence Fallout 4 weapons' AimModels and ImpactDataSets via Papyrus scripting.


------------------------------------------------------------------------------------------------------- 


Warning: This is a work in progress and my C++ knowledge is pretty rudimentary. I probably wouldn't consider this stable or clean, but I guess it works so far. Some workarounds were involved in getting this to compile since I'm using VS2015.


-------------------------------------------------------------------------------------------------------


Additional Compiling Instructions:


- The AmmoTweaks folder needs to be with the F4SE source files to compile (src\f4se\)

-- It's probably best to copy F4SE's source files (along with the included source) to another directory to keep the base source files clean.


- Once it's loaded in Visual Studio, you'll need to add "$(SolutionDir)\\..\\.." (that's "$(SolutionDir)\..\.." if you're reading this in a text editor) to the f4se_common_vc14 project's 'Additional Include Directories' (found in the project's properties under 'C/C++ \ General' )

-- This is required to get the static library for f4se_common_vc14 to compile.


--------------------------------------------------------------------------------------------------------

Script Usage:

- Note: Only functions relevant to guns are included in this version.


- The AmmoTweaks:ATInstanceData functions work identically to F4SE's InstanceData.

-- The same downsides apply - attaching a new mod to a weapon will reset any script-edited values, and most Set functions will only affect the weapon once it's been re-equipped.

-- The weapon reference script in the new version of AmmoTweaks will include a method to update script-based changes, but this is not included here.


-- To edit all weapons of the same type, pass the result of Weapon.GetInstanceOwner()

-- To edit only your currently equipped weapon, pass the result of Actor.GetInstanceOwner(int slotIndex), slotIndex being 41 for guns

-- To edit a weapon's ObjectReference (generally only available for dropped or persistent weapons), create a new InstanceData:Owner struct and fill its owner property with the reference and pass it:

    InstanceData:Owner WeapInstance = new InstanceData:Owner
    WeapInstance.owner = (YourObjectReference)


- GetAimModel() returns 'none' if the weapon's AimModel has been modified by any scripts or ObjectMods. 

-- To get around this, only use it on Weapon base forms.

-- The same goes for GetZoomData()
