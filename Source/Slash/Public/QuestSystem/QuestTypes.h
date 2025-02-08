#pragma once

UENUM(BlueprintType)
enum class EObjectiveType : uint8
{
	EOT_Location UMETA(DisplayName = "Location"),
	EOT_Kill UMETA(DisplayName = "Kill"),
	EOT_Interact UMETA(DisplayName = "Interact"),
	EOT_Collect UMETA(DisplayName = "Collect")
};
