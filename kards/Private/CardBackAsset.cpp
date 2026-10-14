#include "CardBackAsset.h"

#include "Engine/Texture.h"
#include "Materials/MaterialInstance.h"

bool UCardBackAsset::IsAvailable() const
{
    return true;
}

UMaterialInstance* UCardBackAsset::GetUniqueMaterial(bool bUseDeckMaterial, bool& bHasMaterial)
{
    // Signature-only editor shim; the game resolves the soft pointers.
    (void)bUseDeckMaterial;
    bHasMaterial = false;
    return nullptr;
}

UTexture* UCardBackAsset::GetNextTexture(bool useISBN)
{
    (void)useISBN;
    return nullptr;
}

UTexture* UCardBackAsset::GetMainTexture(bool useISBN)
{
    (void)useISBN;
    return nullptr;
}
