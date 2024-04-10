#pragma once
#include "../../shared/atto_defines.h"

namespace atto {
    class SpriteResource;
    class AudioResource;
    class Core;
    void LoadAllAssets( Core * core );
    
    inline bool GameAssetsLoaded = false;

    /*
    ======================================================
    ========================GENERAL=======================
    ======================================================
    */

    inline SpriteResource * sprSmolSelection = nullptr;

    /*
    ======================================================
    ========================KLAED=========================
    ======================================================
    */

    inline SpriteResource * sprKlaedScoutBase = nullptr;
    inline SpriteResource * sprKlaedScoutEngine = nullptr;
    inline SpriteResource * sprKlaedScoutShield = nullptr;
    inline SpriteResource * sprKlaedScoutWeapon = nullptr;
    inline SpriteResource * sprKlaedScoutDestruction = nullptr;
    inline i32              cfgKlaedScoutTrainTimeTurns = 0;

    inline SpriteResource * sprKlaedWorkerBase = nullptr;
    inline SpriteResource * sprKlaedWorkerEngine = nullptr;
    inline SpriteResource * sprKlaedWorkerDestruction = nullptr;
    inline AudioResource *  sndKlaedWorkerHello = nullptr;
    inline AudioResource *  sndKlaedWorkerBuilt = nullptr;
    inline AudioResource *  sndKlaedWorkerMove = nullptr;
    inline AudioResource *  sndKlaedWorkerDestruction1 = nullptr;
    inline AudioResource *  sndKlaedWorkerDestruction2 = nullptr;
    inline AudioResource *  sndKlaedWorkerDestruction3 = nullptr;
    inline AudioResource *  sndKlaedWorkerDestruction4= nullptr;
    inline i32              cfgKlaedWorkerTrainTimeTurns = 0;

    inline SpriteResource * sprKlaedFighterBase = nullptr;
    inline SpriteResource * sprKlaedFighterEngine = nullptr;
    inline SpriteResource * sprKlaedFighterShield = nullptr;
    inline SpriteResource * sprKlaedFighterWeapon = nullptr;
    inline SpriteResource * sprKlaedFighterDestruction = nullptr;
    inline i32              cfgKlaedFighterTrainTimeTurns = 0;

    inline SpriteResource * sprKlaedBomberBase = nullptr;
    inline SpriteResource * sprKlaedBomberEngine = nullptr;
    inline SpriteResource * sprKlaedBomberShield = nullptr;
    inline SpriteResource * sprKlaedBomberDestruction = nullptr;

    inline SpriteResource * sprKlaedTorpedoBase = nullptr;
    inline SpriteResource * sprKlaedTorpedoEngine = nullptr;
    inline SpriteResource * sprKlaedTorpedoShield = nullptr;
    inline SpriteResource * sprKlaedTorpedoWeapon = nullptr;
    inline SpriteResource * sprKlaedTorpedoDestruction = nullptr;

    inline SpriteResource * sprKlaedFrigateBase = nullptr;
    inline SpriteResource * sprKlaedFrigateEngine = nullptr;
    inline SpriteResource * sprKlaedFrigateShield = nullptr;
    inline SpriteResource * sprKlaedFrigateWeapon = nullptr;
    inline SpriteResource * sprKlaedFrigateDestruction = nullptr;

    inline SpriteResource * sprKlaedBattleCruiserBase = nullptr;
    inline SpriteResource * sprKlaedBattleCruiserEngine = nullptr;
    inline SpriteResource * sprKlaedBattleCruiserShield = nullptr;
    inline SpriteResource * sprKlaedBattleCruiserWeapon = nullptr;
    inline SpriteResource * sprKlaedBattleCruiserDestruction = nullptr;

    inline SpriteResource * sprKlaedDreadnoughtBase = nullptr;
    inline SpriteResource * sprKlaedDreadnoughtEngine = nullptr;
    inline SpriteResource * sprKlaedDreadnoughtShield = nullptr;
    inline SpriteResource * sprKlaedDreadnoughtWeapon = nullptr;
    inline SpriteResource * sprKlaedDreadnoughtDestruction = nullptr;

    inline SpriteResource * sprKlaedProjectileRay = nullptr;
    inline SpriteResource * sprKlaedProjectileWave = nullptr;
    inline SpriteResource * sprKlaedProjectileBullet = nullptr;
    inline SpriteResource * sprKlaedProjectileBigBullet = nullptr;
    inline SpriteResource * sprKlaedProjectileTorpedo = nullptr;

    inline SpriteResource * sprKlaedProjectileBulletHit = nullptr;

    /*
    ======================================================
    ========================Nairan========================
    ======================================================
    */

    inline SpriteResource * sprNairanScoutBase = nullptr;
    inline SpriteResource * sprNairanScoutEngine = nullptr;
    inline SpriteResource * sprNairanScoutShield = nullptr;
    inline SpriteResource * sprNairanScoutWeapon = nullptr;
    inline SpriteResource * sprNairanScoutDestruction = nullptr;

    inline SpriteResource * sprNairanWorkerBase = nullptr;
    inline SpriteResource * sprNairanWorkerEngine = nullptr;
    inline SpriteResource * sprNairanWorkerDestruction = nullptr;

    inline SpriteResource * sprNairanFighterBase = nullptr;
    inline SpriteResource * sprNairanFighterEngine = nullptr;
    inline SpriteResource * sprNairanFighterShield = nullptr;
    inline SpriteResource * sprNairanFighterWeapon = nullptr;
    inline SpriteResource * sprNairanFighterDestruction = nullptr;

    inline SpriteResource * sprNairanProjectileBolt = nullptr;
    inline SpriteResource * sprNairanProjectileRocket = nullptr;

    /*
    ======================================================
    ========================Nautolan========================
    ======================================================
    */

    inline SpriteResource * sprNautolanScoutBase = nullptr;
    inline SpriteResource * sprNautolanScoutEngine = nullptr;
    inline SpriteResource * sprNautolanScoutShield = nullptr;
    inline SpriteResource * sprNautolanScoutWeapon = nullptr;
    inline SpriteResource * sprNautolanScoutDestruction = nullptr;

    inline SpriteResource * sprNautolanWorkerBase = nullptr;
    inline SpriteResource * sprNautolanWorkerEngine = nullptr;
    inline SpriteResource * sprNautolanWorkerDestruction = nullptr;

    inline SpriteResource * sprNautolanFighterBase = nullptr;
    inline SpriteResource * sprNautolanFighterEngine = nullptr;
    inline SpriteResource * sprNautolanFighterShield = nullptr;
    inline SpriteResource * sprNautolanFighterWeapon = nullptr;
    inline SpriteResource * sprNautolanFighterDestruction = nullptr;

}
