#include "atto_sim_load_assets.h"
#include "../../shared/atto_core.h"

namespace atto {
    void LoadAllAssets( Core * core ) {
        if ( GameAssetsLoaded == true ) {
            return;
        }
        GameAssetsLoaded = true;
        sprKlaedScoutBase                   = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Base/PNGs/Kla'ed - Scout - Base.png", 1, 64, 64, 0 );
        sprKlaedScoutEngine                 = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Engine/PNGs/Kla'ed - Scout - Engine.png", 10, 64, 64, 10 );
        sprKlaedScoutShield                 = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Shield/PNGs/Kla'ed - Scout - Shield.png", 14, 64, 64, 10 );
        sprKlaedScoutWeapon                 = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Weapons/PNGs/Kla'ed - Scout - Weapons.png", 6, 64, 64, 10 );
        sprKlaedScoutBaseDestruction        = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Destruction/PNGs/Kla'ed - Scout - Destruction.png", 10, 64, 64, 10 );

        sprKlaedWorkerBase                  = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Base/PNGs/Kla'ed - Support Ship - Base.png", 1, 64, 64, 0 );
        sprKlaedWorkerEngine                = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Engine/PNGs/Kla'ed - Support Ship - Engine.png", 10, 64, 64, 10 );
        sprKlaedWorkerDestruction           = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Destruction/PNGs/Kla'ed - Support Ship - Destruction.png", 10, 64, 64, 10 );

        sprKlaedFighterBase                 = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Base/PNGs/Kla'ed - Fighter - Base.png", 1, 64, 64, 0 );
        sprKlaedFighterEngine               = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Engine/PNGs/Kla'ed - Fighter - Engine.png", 10, 64, 64, 10 );
        sprKlaedFighterShield               = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Shield/PNGs/Kla'ed - Fighter - Shield.png", 10, 64, 64, 10 );
        sprKlaedFighterWeapon               = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Weapons/PNGs/Kla'ed - Fighter - Weapons.png", 6, 64, 64, 10 );
        sprKlaedFighterDestruction          = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Destruction/PNGs/Kla'ed - Fighter - Destruction.png", 9, 64, 64, 10 );

        sprKlaedBomberBase;
        sprKlaedBomberEngine;
        sprKlaedBomberShield;
        sprKlaedBomberWeapon;
        sprKlaedBomberDestruction;

        sprKlaedTorpedoBase;
        sprKlaedTorpedoEngine;
        sprKlaedTorpedoShield;
        sprKlaedTorpedoWeapon;
        sprKlaedTorpedoDestruction;

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
