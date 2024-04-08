#include "atto_sim_load_assets.h"
#include "../../shared/atto_core.h"

namespace atto {
    void LoadAllAssets( Core * core ) {
        if ( GameAssetsLoaded == true ) {
            return;
        }
        GameAssetsLoaded = true;
        sprKlaedScoutBase = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Base/PNGs/Kla'ed - Scout - Base.png", 1, 64, 64, 0 );
        sprKlaedScoutEngine = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Engine/PNGs/Kla'ed - Scout - Engine.png", 10, 64, 64, 10 );
        sprKlaedScoutShield = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Engine/PNGs/Kla'ed - Scout - Shield.png", 14, 64, 64, 10 );
        sprKlaedScoutWeapon = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Base/PNGs/Kla'ed - Scout - Weapon.png", 6, 64, 64, 10 );
        sprKlaedScoutBaseDestruction;
        sprKlaedWorkerBase;
        sprKlaedWorkerEngine;
        sprKlaedWorkerShield;
        sprKlaedWorkerWeapon;
        sprKlaedWorkerDestruction;
        sprKlaedFighterBase;
        sprKlaedFighterEngine;
        sprKlaedFighterShield;
        sprKlaedFighterWeapon;
        sprKlaedFighterDestruction;
        sprKlaedBomberBase;
        sprKlaedBomberEngine;
        sprKlaedBomberShield;
        sprKlaedBomberWeapon;
        sprKlaedBomberDestruction;
        sprKlaedTorpedeBase;
        sprKlaedTorpedeEngine;
        sprKlaedTorpedeShield;
        sprKlaedTorpedeWeapon;
        sprKlaedTorpedeDestruction;
        sprKlaedFrigateBase;
        sprKlaedFrigateEngine;
        sprKlaedFrigateShield;
        sprKlaedFrigateWeapon;
        sprKlaedFrigateDestruction;
        sprKlaedBattleCruiserBase;
        sprKlaedBattleCruiserEngine;
        sprKlaedBattleCruiserShield;
        sprKlaedBattleCruiserWeapon;
        sprKlaedBattleCruiserDestruction;
        sprKlaedDreadnoughtBase;
        sprKlaedDreadnoughtEngine;
        sprKlaedDreadnoughtShield;
        sprKlaedDreadnoughtWeapon;
        sprKlaedDreadnoughtDestruction;
        sprKlaedProjectileRay;
        sprKlaedProjectileWave;
        sprKlaedProjectileBullet = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Projectiles/PNGs/Kla'ed - Bullet.png", 4, 4, 16, 10 );
        sprKlaedProjectileBigBullet;
        sprKlaedProjectileTorpedo;
    }
}
