#pragma once


namespace atto {
    class SpriteResource;
    class Core;
    void LoadAllAssets( Core * core );
    
    inline bool GameAssetsLoaded = false;
    inline SpriteResource * sprKlaedScoutBase = nullptr;
    inline SpriteResource * sprKlaedScoutEngine = nullptr;
    inline SpriteResource * sprKlaedScoutShield = nullptr;
    inline SpriteResource * sprKlaedScoutWeapon = nullptr;
    inline SpriteResource * sprKlaedScoutBaseDestruction = nullptr;

    inline SpriteResource * sprKlaedWorkerBase = nullptr;
    inline SpriteResource * sprKlaedWorkerEngine = nullptr;
    inline SpriteResource * sprKlaedWorkerShield = nullptr;
    inline SpriteResource * sprKlaedWorkerWeapon = nullptr;
    inline SpriteResource * sprKlaedWorkerDestruction = nullptr;

    inline SpriteResource * sprKlaedFighterBase = nullptr;
    inline SpriteResource * sprKlaedFighterEngine = nullptr;
    inline SpriteResource * sprKlaedFighterShield = nullptr;
    inline SpriteResource * sprKlaedFighterWeapon = nullptr;
    inline SpriteResource * sprKlaedFighterDestruction = nullptr;

    inline SpriteResource * sprKlaedBomberBase = nullptr;
    inline SpriteResource * sprKlaedBomberEngine = nullptr;
    inline SpriteResource * sprKlaedBomberShield = nullptr;
    inline SpriteResource * sprKlaedBomberWeapon = nullptr;
    inline SpriteResource * sprKlaedBomberDestruction = nullptr;

    inline SpriteResource * sprKlaedTorpedeBase = nullptr;
    inline SpriteResource * sprKlaedTorpedeEngine = nullptr;
    inline SpriteResource * sprKlaedTorpedeShield = nullptr;
    inline SpriteResource * sprKlaedTorpedeWeapon = nullptr;
    inline SpriteResource * sprKlaedTorpedeDestruction = nullptr;

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
}
