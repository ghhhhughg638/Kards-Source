#include "CombatHelperFunctions.h"

UCombatHelperFunctions::UCombatHelperFunctions() {
}

bool UCombatHelperFunctions::IsClientPlaying(const UObject* WorldContextObject) {
    return false;
}

ESideEnum UCombatHelperFunctions::GetTheOtherSide(ESideEnum SideToGet, const UObject* WorldContextObject) {
    return ESideEnum::NotAvailable;
}

ECardLocationEnum UCombatHelperFunctions::GetSupportLineBySide(ESideEnum SideToGet, const UObject* WorldContextObject) {
    return ECardLocationEnum::NotAvailable;
}

ESideEnum UCombatHelperFunctions::GetPlayingSide(const UObject* WorldContextObject) {
    return ESideEnum::NotAvailable;
}

ECardLocationEnum UCombatHelperFunctions::GetOpponentSideHandLocation(const UObject* WorldContextObject) {
    return ECardLocationEnum::NotAvailable;
}

ESideEnum UCombatHelperFunctions::GetOpponentSide(const UObject* WorldContextObject) {
    return ESideEnum::NotAvailable;
}

ECardLocationEnum UCombatHelperFunctions::GetHandLocationBySide(ESideEnum SideToGet, const UObject* WorldContextObject) {
    return ECardLocationEnum::NotAvailable;
}

ECardLocationEnum UCombatHelperFunctions::GetDeckLocationBySide(ESideEnum SideToGet, const UObject* WorldContextObject) {
    return ECardLocationEnum::NotAvailable;
}

ECardLocationEnum UCombatHelperFunctions::GetClientSideHandLocation(const UObject* WorldContextObject) {
    return ECardLocationEnum::NotAvailable;
}

ESideEnum UCombatHelperFunctions::GetClientSide(const UObject* WorldContextObject) {
    return ESideEnum::NotAvailable;
}


