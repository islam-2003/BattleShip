#include "ability.hh"

using enum Ability::Type;

void Ability::setType(Type abilityType){
    type = abilityType;
    switch (abilityType){
        case Diagonal:
        case XBomb:
        case PlusBomb:
        case Linear:
        default:
            break;
    }
}
