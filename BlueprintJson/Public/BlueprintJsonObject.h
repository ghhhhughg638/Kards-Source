#pragma once
#include "CoreMinimal.h"
#include "BlueprintJsonObject.generated.h"

USTRUCT(BlueprintType)
struct FBlueprintJsonObject {
    GENERATED_BODY()
public:
    BLUEPRINTJSON_API FBlueprintJsonObject();

    /**
     * JSON text is deliberately kept as a reflected string.  The original
     * game type is an opaque 16-byte value to the card Blueprints; retaining
     * the payload this way gives the mod project a usable, copyable carrier
     * without introducing UObject ownership into a USTRUCT.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString SerializedJson;
};

