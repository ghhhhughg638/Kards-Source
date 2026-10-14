#include "FilterUGCTextAsyncAction.h"

UFilterUGCTextAsyncAction::UFilterUGCTextAsyncAction()
{
}

UFilterUGCTextAsyncAction* UFilterUGCTextAsyncAction::FilterUGCText(
    UObject* WorldContextObject,
    const FString& Input,
    EUGCTextFilterPolicy Policy)
{
    // Signature-only editor shim; the game supplies the platform text filter.
    (void)WorldContextObject;
    (void)Input;
    (void)Policy;
    return nullptr;
}
