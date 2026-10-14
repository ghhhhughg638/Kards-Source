#include "SaveGameFunctionLibrary.h"

void USaveGameFunctionLibrary::LoadOtherSettings(bool& IsStreamerMode, bool& SaveFileValid)
{
    // Signature-only editor shim.
    IsStreamerMode = false;
    SaveFileValid = false;
}
