F4SE AmmoTweaks Extension
=========================


WIP plugin with some extra InstanceData-related functions for AmmoTweaks.


Notes for compiling:

- The Plugin\src\AmmoTweaks folder needs to be with the F4SE source files to compile ((Fallout 4 directory)\src\)
- Made with Visual Studio 2015/2017 (should work with 2017, not sure about backwards compatibility with 2015)

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
