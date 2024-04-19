#include "atto_sim_load_assets.h"
#include "../../shared/atto_core.h"
#include "atto_sim_base.h"

namespace atto {
    void LoadAllAssets( Core * core ) {
        if ( GameAssetsLoaded == true ) {
            return;
        }

        sprSmolSelection = core->ResourceGetAndCreateSprite( "res/ents/test/ship_selected.png", 1, 48, 48, 0 );

        sprBuildingStationBlueOff = core->ResourceGetAndCreateSprite( "res/ents/test/building_station_blue_off.png", 1, 64, 64, 0 );
        sprBuildingStationBlueOn =  core->ResourceGetAndCreateSprite( "res/ents/test/building_station_blue_on.png", 1, 64, 64, 0 );
        sprBuildingStationRedOff = core->ResourceGetAndCreateSprite( "res/ents/test/building_station_red_off.png", 1, 64, 64, 0 );
        sprBuildingStationRedOn = core->ResourceGetAndCreateSprite( "res/ents/test/building_station_red_on.png", 1, 64, 64, 0 );
        colBuildingStation.type = COLLIDER_TYPE_AXIS_BOX;
        colBuildingStation.box = FpAxisBoxCreateFromCenterSize( Fp2( 0, 0 ), Fp2( 48, 48 ) );

        sprBuildingTradeBlue = core->ResourceGetAndCreateSprite( "res/game/buildings/trade_center_blue.png", 1, 64, 64, 0 );
        sprBuildingTradeRed = core->ResourceGetAndCreateSprite( "res/game/buildings/trade_center_red.png", 1, 64, 64, 0 );
        colBuildingTrade.type = COLLIDER_TYPE_AXIS_BOX;
        colBuildingTrade.box = FpAxisBoxCreateFromCenterSize( Fp2( 0, 0 ), Fp2( 48, 48 ) );

        sprBuildingSolarBlue = core->ResourceGetAndCreateSprite( "res/ents/test/building_solar_array_blue.png", 1, 64, 32, 0 );
        sprBuildingSolarRed = core->ResourceGetAndCreateSprite( "res/ents/test/building_solar_array_red.png", 1, 64, 32, 0 );
        colBuildingSolar.type = COLLIDER_TYPE_AXIS_BOX;
        colBuildingSolar.box = FpAxisBoxCreateFromCenterSize( Fp2( 0, 0 ), Fp2( 48, 24 ) );

        sprBuildingComputeBlueOff = core->ResourceGetAndCreateSprite( "res/ents/test/building_cpu_blue_off.png", 1, 48, 48, 0 );
        sprBuildingComputeBlueOn = core->ResourceGetAndCreateSprite( "res/ents/test/building_cpu_blue_on.png", 1, 48, 48, 0 );
        sprBuildingComputeRedOff = core->ResourceGetAndCreateSprite( "res/ents/test/building_cpu_red_off.png", 1, 48, 48, 0 );
        sprBuildingComputeRedOn = core->ResourceGetAndCreateSprite( "res/ents/test/building_cpu_red_on.png", 1, 48, 48, 0 );
        colBuildingCompute.type = COLLIDER_TYPE_CIRCLE;
        colBuildingCompute.circle.rad = Fp( 22 );

        sndCantPlaceBuilding = core->ResourceGetAndCreateAudio( "res/sounds/not_legal/magic_immune.wav", true, false, 0, 0 );
        sndCantPlaceBuilding->stealMode = AudioStealMode::NONE;
        sndCantPlaceBuilding->maxInstances = 0;
        sndCantPlaceBuilding->volumeMultiplier = 1.5f;
        sndCantPlaceBuilding->minTimeToPassForAnotherSubmission = 0.4;

        /*
            ======================================================
            ========================KLAED=========================
            ======================================================
        */

        GameAssetsLoaded = true;

        sprKlaedWorkerBase                  = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Base/PNGs/Kla'ed - Support Ship - Base.png", 1, 64, 64, 0 );
        sprKlaedWorkerEngine                = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Engine/PNGs/Kla'ed - Support Ship - Engine.png", 10, 64, 64, 10 );
        sprKlaedWorkerDestruction           = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Destruction/PNGs/Kla'ed - Support Ship - Destruction.png", 10, 64, 64, 10 );
        sndKlaedWorkerHello                 = core->ResourceGetAndCreateAudio( "res/sounds/not_legal/huh.mp3", true, false, 0, 0 );
        sndKlaedWorkerBuilt                 = core->ResourceGetAndCreateAudio( "res/sounds/not_legal/starcraft/scv.mp3", true, false, 0, 0 );
        sndKlaedWorkerMove                  = core->ResourceGetAndCreateAudio( "res/sounds/not_legal/starcraft/overtime.mp3", true, false, 0, 0 );
        sndKlaedWorkerDestruction1          = core->ResourceGetAndCreateAudio( "res/sounds/worker_explosion_01.wav", true, false, 0, 0 );
        sndKlaedWorkerDestruction2          = core->ResourceGetAndCreateAudio( "res/sounds/worker_explosion_02.wav", true, false, 0, 0 );
        sndKlaedWorkerDestruction3          = core->ResourceGetAndCreateAudio( "res/sounds/worker_explosion_03.wav", true, false, 0, 0 );
        sndKlaedWorkerDestruction4          = core->ResourceGetAndCreateAudio( "res/sounds/worker_explosion_04.wav", true, false, 0, 0 );
        cfgKlaedWorkerTrainTimeTurns        = SecondsToTurns( 12 );
        cfgKlaedWorkerCost                  = { 50, 10, 5 };

        sprKlaedScoutBase                   = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Base/PNGs/Kla'ed - Scout - Base.png", 1, 64, 64, 0 );
        sprKlaedScoutEngine                 = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Engine/PNGs/Kla'ed - Scout - Engine.png", 10, 64, 64, 10 );
        sprKlaedScoutShield                 = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Shield/PNGs/Kla'ed - Scout - Shield.png", 14, 64, 64, 10 );
        sprKlaedScoutWeapon                 = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Weapons/PNGs/Kla'ed - Scout - Weapons.png", 6, 64, 64, 10 );
        sprKlaedScoutDestruction            = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Destruction/PNGs/Kla'ed - Scout - Destruction.png", 10, 64, 64, 10 );
        sndKlaedScoutHello                  = core->ResourceGetAndCreateAudio( "res/sounds/not_legal/starcraft/shoot.mp3", true, false, 0, 0 );
        sndKlaedScoutBuilt                  = core->ResourceGetAndCreateAudio( "res/sounds/not_legal/starcraft/five-by-five-hele.mp3", true, false, 0, 0 );
        sndKlaedScoutMove                   = core->ResourceGetAndCreateAudio( "res/sounds/not_legal/starcraft/proceeding-seige.mp3", true, false, 0, 0 );
        cfgKlaedScoutTrainTimeTurns         = SecondsToTurns( 18 );
        cfgKlaedScoutCost                   = { 70, 20, 10 };

        sprKlaedFighterBase                 = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Base/PNGs/Kla'ed - Fighter - Base.png", 1, 64, 64, 0 );
        sprKlaedFighterEngine               = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Engine/PNGs/Kla'ed - Fighter - Engine.png", 10, 64, 64, 10 );
        sprKlaedFighterShield               = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Shield/PNGs/Kla'ed - Fighter - Shield.png", 10, 64, 64, 10 );
        sprKlaedFighterWeapon               = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Weapons/PNGs/Kla'ed - Fighter - Weapons.png", 6, 64, 64, 10 );
        sprKlaedFighterDestruction          = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Destruction/PNGs/Kla'ed - Fighter - Destruction.png", 9, 64, 64, 10 );
        cfgKlaedFighterTrainTimeTurns       = SecondsToTurns( 38 );
        cfgKlaedFighterCost                 = { 100, 50, 50 };

        sprKlaedBomberBase                  = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Base/PNGs/Kla'ed - Bomber - Base.png", 1, 64, 64, 0 );
        sprKlaedBomberEngine                = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Engine/PNGs/Kla'ed - Bomber - Engine.png", 10, 64, 64, 10 );
        sprKlaedBomberShield                = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Shield/PNGs/Kla'ed - Bomber - Shield.png", 6, 64, 64, 10 );
        sprKlaedBomberDestruction           = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Destruction/PNGs/Kla'ed - Bomber - Destruction.png", 8, 64, 64, 10 );

        sprKlaedTorpedoBase                 = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Base/PNGs/Kla'ed - Torpedo Ship - Base.png", 1, 64, 64, 0 );
        sprKlaedTorpedoEngine               = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Engine/PNGs/Kla'ed - Torpedo Ship - Engine.png", 10, 64, 64, 10 );
        sprKlaedTorpedoShield               = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Shield/PNGs/Kla'ed - Torpedo Ship - Shield.png", 10, 64, 64, 10 );
        sprKlaedTorpedoWeapon               = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Weapons/PNGs/Kla'ed - Torpedo Ship - Weapons.png", 16, 64, 64, 10 );
        sprKlaedTorpedoDestruction          = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Destruction/PNGs/Kla'ed - Torpedo Ship - Destruction.png", 10, 64, 64, 10 );

        sprKlaedFrigateBase                 = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Base/PNGs/Kla'ed - Frigate - Base.png", 1, 64, 64, 0 );
        sprKlaedFrigateEngine               = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Engine/PNGs/Kla'ed - Frigate - Engine.png", 12, 64, 64, 10 );
        sprKlaedFrigateShield               = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Shield/PNGs/Kla'ed - Frigate - Shield.png", 40, 64, 64, 10 );
        sprKlaedFrigateWeapon               = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Weapons/PNGs/Kla'ed - Frigate - Weapons.png", 6, 64, 64, 10 );
        sprKlaedFrigateDestruction          = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Destruction/PNGs/Kla'ed - Frigate - Destruction.png", 9, 64, 64, 10 );
        
        sprKlaedBattleCruiserBase           = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Base/PNGs/Kla'ed - Battlecruiser - Base.png", 1, 128, 128, 0 );
        sprKlaedBattleCruiserEngine         = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Engine/PNGs/Kla'ed - Battlecruiser - Engine.png", 12, 128, 128, 10 );
        sprKlaedBattleCruiserShield         = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Shield/PNGs/Kla'ed - Battlecruiser - Shield.png", 16, 128, 128, 10 );
        sprKlaedBattleCruiserWeapon         = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Weapons/PNGs/Kla'ed - Battlecruiser - Weapons.png", 30, 128, 128, 10 );
        sprKlaedBattleCruiserDestruction    = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Destruction/PNGs/Kla'ed - Battlecruiser - Destruction.png", 14, 128, 128, 10 );
        
        sprKlaedDreadnoughtBase             = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Base/PNGs/Kla'ed - Dreadnought - Base.png", 1, 128, 128, 0 );
        sprKlaedDreadnoughtEngine           = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Engine/PNGs/Kla'ed - Dreadnought - Engine.png", 12, 128, 128, 10 );
        sprKlaedDreadnoughtShield           = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Shield/PNGs/Kla'ed - Dreadnought - Shield.png", 10, 128, 128, 10 );
        sprKlaedDreadnoughtWeapon           = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Weapons/PNGs/Kla'ed - Dreadnought - Weapons.png", 60, 128, 128, 10 );
        sprKlaedDreadnoughtDestruction      = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Destruction/PNGs/Kla'ed - Dreadnought - Destruction.png", 12, 128, 128, 10 );

        sprKlaedProjectileRay               = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Projectiles/PNGs/Kla'ed - Ray.png", 4, 18, 38, 10 );
        sprKlaedProjectileWave              = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Projectiles/PNGs/Kla'ed - Wave.png", 6, 64, 64, 10 );
        sprKlaedProjectileBullet            = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Projectiles/PNGs/Kla'ed - Bullet.png", 4, 4, 16, 10 );
        sprKlaedProjectileBigBullet         = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Projectiles/PNGs/Kla'ed - Big Bullet.png", 4, 8, 16, 10 );
        sprKlaedProjectileTorpedo           = core->ResourceGetAndCreateSprite( "res/game/fleet_1/Klaed/Projectiles/PNGs/Kla'ed - Torpedo.png", 4, 11, 32, 10 );

        sprKlaedProjectileBulletHit         = core->ResourceGetAndCreateSprite( "res/game/particles/bullet_hit_smol.png", 3, 16, 16, 10 );

        sndKlaedProjectileBulletDestruction1 = core->ResourceGetAndCreateAudio( "res/sounds/bullet_hit_smol_01.wav", true, false, 0, 0 );
        sndKlaedProjectileBulletDestruction1->maxInstances = 2;
        sndKlaedProjectileBulletDestruction1->stealMode = AudioStealMode::OLDEST;
        sndKlaedProjectileBulletDestruction1->volumeMultiplier = 1.75f;

        sndKlaedProjectileBulletDestruction2 = core->ResourceGetAndCreateAudio( "res/sounds/bullet_hit_smol_02.wav", true, false, 0, 0 );
        sndKlaedProjectileBulletDestruction2->maxInstances = 2;
        sndKlaedProjectileBulletDestruction2->stealMode = AudioStealMode::OLDEST;
        sndKlaedProjectileBulletDestruction2->volumeMultiplier = 1.75f;

        sndKlaedBuildBuilding               = core->ResourceGetAndCreateAudio( "res/sounds/not_legal/starcraft/protoss-electric.mp3", true, false, 0, 0 );
        sndKlaedBuildBuilding->maxInstances = 3;
        sndKlaedBuildBuilding->stealMode = AudioStealMode::OLDEST;

        sndKlaedBuidlingQueue               = core->ResourceGetAndCreateAudio( "res/sounds/not_legal/starwars/rogerroger.mp3", true, false, 0, 0 );
        sndKlaedNotEnoughResources          = core->ResourceGetAndCreateAudio( "res/sounds/not_legal/starcraft/vespene.mp3", true, false, 0, 0 );


        /*
            ======================================================
            ========================Nairan========================
            ======================================================
        */

        sprNairanWorkerBase             = core->ResourceGetAndCreateSprite( "res/game/fleet_2/Nairan/Base/PNGs/Nairan - Support Ship - Base.png", 1, 64, 64, 0 );
        sprNairanWorkerEngine           = core->ResourceGetAndCreateSprite( "res/game/fleet_2/Nairan/Engine/PNGs/Nairan - Support Ship - Engine.png", 8, 64, 64, 10 );
        sprNairanWorkerDestruction      = core->ResourceGetAndCreateSprite( "res/game/fleet_2/Nairan/Destruction/PNGs/Nairan - Support Ship - Destruction.png", 16, 64, 64, 10 );

        sprNairanScoutBase              = core->ResourceGetAndCreateSprite( "res/game/fleet_2/Nairan/Base/PNGs/Nairan - Scout - Base.png", 1, 64, 64, 0 );
        sprNairanScoutEngine            = core->ResourceGetAndCreateSprite( "res/game/fleet_2/Nairan/Engine/PNGs/Nairan - Scout - Engine.png", 8, 64, 64, 10 );
        sprNairanScoutShield            = core->ResourceGetAndCreateSprite( "res/game/fleet_2/Nairan/Shield/PNGs/Nairan - Scout - Shield.png", 18, 64, 64, 10 );
        sprNairanScoutWeapon            = core->ResourceGetAndCreateSprite( "res/game/fleet_2/Nairan/Weapons/PNGs/Nairan - Scout - Weapons.png", 6, 64, 64, 10 );
        sprNairanScoutDestruction       = core->ResourceGetAndCreateSprite( "res/game/fleet_2/Nairan/Destruction/PNGs/Nairan - Scout - Destruction.png", 16 ,64, 64, 10 );

        sprNairanFighterBase            = core->ResourceGetAndCreateSprite( "res/game/fleet_2/Nairan/Base/PNGs/Nairan - Fighter - Base.png", 1, 64, 64, 0 );
        sprNairanFighterEngine          = core->ResourceGetAndCreateSprite( "res/game/fleet_2/Nairan/Engine/PNGs/Nairan - Fighter - Engine.png", 8, 64, 64, 10 );
        sprNairanFighterShield          = core->ResourceGetAndCreateSprite( "res/game/fleet_2/Nairan/Shield/PNGs/Nairan - Fighter - Shield.png", 20, 64, 64, 10 );
        sprNairanFighterWeapon          = core->ResourceGetAndCreateSprite( "res/game/fleet_2/Nairan/Weapons/PNGs/Nairan - Fighter - Weapons.png", 28, 64, 64, 10 );
        sprNairanFighterDestruction     = core->ResourceGetAndCreateSprite( "res/game/fleet_2/Nairan/Destruction/PNGs/Nairan - Fighter - Destruction.png", 18, 64, 64, 10 );

        sprNairanProjectileBolt         = core->ResourceGetAndCreateSprite( "res/game/fleet_2/Nairan/Projectiles/PNGs/Nairan - Bolt.png", 5, 9, 9, 10 );
        sprNairanProjectileRocket       = core->ResourceGetAndCreateSprite( "res/game/fleet_2/Nairan/Projectiles/PNGs/Nairan - Rocket.png", 4, 9, 16, 10 );

        /*
            ======================================================
            ========================Nautolan========================
            ======================================================
        */

        sprNautolanWorkerBase           = core->ResourceGetAndCreateSprite( "res/game/fleet_3/Nautolan/Base/PNGs/Nautolan Ship - Support - Base.png", 1, 64, 64, 0 );
        sprNautolanWorkerEngine         = core->ResourceGetAndCreateSprite( "res/game/fleet_3/Nautolan/Engine/PNGs/Nautolan Ship - Support - Engine.png", 8, 64, 64, 10 );
        sprNautolanWorkerDestruction    = core->ResourceGetAndCreateSprite( "res/game/fleet_3/Nautolan/Destruction/PNGs/Nautolan Ship - Support.png", 8, 64, 64, 10 );
        cfgNautolanWorkerTrainTimeTurns = SecondsToTurns( 12 );
        cfgNautolanWorkerCost           = { 50, 10, 5 };

        sprNautolanScoutBase            = core->ResourceGetAndCreateSprite( "res/game/fleet_3/Nautolan/Base/PNGs/Nautolan Ship - Scout - Base.png", 1, 64, 64, 0 );
        sprNautolanScoutEngine          = core->ResourceGetAndCreateSprite( "res/game/fleet_3/Nautolan/Engine/PNGs/Nautolan Ship - Scout - Engine.png", 8, 64, 64, 10 );
        sprNautolanScoutShield          = core->ResourceGetAndCreateSprite( "res/game/fleet_3/Nautolan/Shield/PNGs/Nautolan Ship - Scout - Shield.png", 13, 64, 64, 10 );
        sprNautolanScoutWeapon          = core->ResourceGetAndCreateSprite( "res/game/fleet_3/Nautolan/Weapons/PNGs/Nautolan Ship - Scout - Weapons.png", 7, 64, 64, 10 );
        sprNautolanScoutDestruction     = core->ResourceGetAndCreateSprite( "res/game/fleet_3/Nautolan/Destruction/PNGs/Nautolan Ship - Scout.png", 9 ,64, 64, 10 );
        cfgNautolanScoutTrainTimeTurns  = SecondsToTurns( 18 );
        cfgNautolanScoutCost            = { 70, 20, 10 };

        sprNautolanFighterBase          = core->ResourceGetAndCreateSprite( "res/game/fleet_3/Nautolan/Base/PNGs/Nautolan Ship - Fighter - Base.png", 1, 64, 64, 0 );
        sprNautolanFighterEngine        = core->ResourceGetAndCreateSprite( "res/game/fleet_3/Nautolan/Engine/PNGs/Nautolan Ship - Fighter - Engine.png", 8, 64, 64, 10 );
        sprNautolanFighterShield        = core->ResourceGetAndCreateSprite( "res/game/fleet_3/Nautolan/Shield/PNGs/Nautolan Ship - Fighter - Shield.png", 10, 64, 64, 10 );
        sprNautolanFighterWeapon        = core->ResourceGetAndCreateSprite( "res/game/fleet_3/Nautolan/Weapons/PNGs/Nautolan Ship - Fighter - Weapons.png", 9, 64, 64, 10 );
        sprNautolanFighterDestruction   = core->ResourceGetAndCreateSprite( "res/game/fleet_3/Nautolan/Destruction/PNGs/Nautolan Ship - Fighter.png", 9, 64, 64, 10 );
        cfgNautolanFighterTrainTimeTurns = SecondsToTurns( 30 );
        cfgNautolanFighterCost          = { 100, 50, 50 };

    }
}