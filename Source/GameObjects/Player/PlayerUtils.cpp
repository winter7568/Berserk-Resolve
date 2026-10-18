#include "GameObjects/Player/Player.h"
#include "StateMachine/PlayerStates/MoveState.h"
#include "GameLib/GpuResourceUtils.h"
#include "Audio/Audio.h"

// ============================================================================
// PRIVATE METHODS - Setup Helpers
// ============================================================================

void Player::LoadModels(ID3D11Device* device)
{
    model = std::make_unique<Model>(device, "Data/Model/Player/Pla_2.fbx");
    staff.model = std::make_unique<Model>(device, "Data/Model/Staff/staff.gltf");
    model->emissiveStrengthMultiplier = PlayerConstants::MODEL_EMISSIVE_STRENGTH;
    staff.model->emissiveStrengthMultiplier = PlayerConstants::STAFF_EMISSIVE_STRENGTH;
}

void Player::SetupDissolveEffect(ID3D11Device* device)
{
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> dissolveTexture;
    GpuResourceUtils::LoadTexture(device, "Data/Textures/NoiseTexture.png",
        dissolveTexture.GetAddressOf());
    model->EnableDissolve(true);
    staff.model->EnableDissolve(true);
    model->SetDissolveMask(dissolveTexture.Get());
    staff.model->SetDissolveMask(dissolveTexture.Get());
    model->SetDissolveThreshold(1.0f);
    staff.model->SetDissolveThreshold(1.0f);
}

void Player::SetupInitialTransform()
{
    position = PlayerConstants::INITIAL_POSITION;
    scale = PlayerConstants::INITIAL_SCALE;
    angle = PlayerConstants::INITIAL_ANGLE;
    velocity = PlayerConstants::INITIAL_VELOCITY;
}

void Player::SetupWeapon()
{
    staff.scale = PlayerConstants::STAFF_SCALE;
    staff.position = PlayerConstants::STAFF_POSITION;
    staff.angle = PlayerConstants::STAFF_ANGLE;
}

void Player::SetupColliders()
{
    // 武器カプセル設定
    weaponCapsule.radius = PlayerConstants::WEAPON_CAPSULE_RADIUS;
    weaponCapsule.height = PlayerConstants::WEAPON_CAPSULE_HEIGHT;
    weaponCapsule.offset = PlayerConstants::WEAPON_CAPSULE_OFFSET;
    weaponCapsule.rotation = PlayerConstants::WEAPON_CAPSULE_ROTATION;
    weaponCapsule.color = PlayerConstants::WEAPON_CAPSULE_COLOR;

    // 武器スフィア設定
    weaponSphere.radius = PlayerConstants::WEAPON_SPHERE_RADIUS;
    weaponSphere.offset = PlayerConstants::WEAPON_SPHERE_OFFSET;
    weaponSphere.color = PlayerConstants::WEAPON_SPHERE_COLOR;

    // 本体コライダー設定
    capsuleParams.enabled = true;
    capsuleParams.radius = CharacterConstants::BODY_CAPSULE_RADIUS;
    capsuleParams.height = CharacterConstants::BODY_CAPSULE_HEIGHT;
    capsuleParams.offset = CharacterConstants::BODY_CAPSULE_OFFSET;
    capsuleParams.rotation = CharacterConstants::BODY_CAPSULE_ROTATION;
    capsuleParams.color = CharacterConstants::BODY_CAPSULE_COLOR;
}

void Player::SetupPlayerStats()
{
    health = PlayerConstants::MAX_HEALTH;
    weight = PlayerConstants::DEFAULT_WEIGHT;
    isGround = true;
    invincibleTimer = 0.0f;
    isLockOnActive = false;
}

void Player::SetupStateManager()
{
    stateManager.ClearStates();
    stateManager.PushState(&PlayerIdleState::Instance());
}

void Player::SetupTrailSystem(ID3D11Device* device)
{
    TrailSystem::TrailConfig trailConfig;
    trailConfig.startOffset = PlayerConstants::TRAIL_TIP_OFFSET;
    trailConfig.endOffset = PlayerConstants::TRAIL_BASE_OFFSET;
    trailConfig.maxLifetime = PlayerConstants::TRAIL_MAX_LIFETIME;
    trailConfig.maxSegments = CharacterConstants::TRAIL_MAX_SEGMENTS;
    trailConfig.normalMapPath = "Data/Textures/energy_Streak.png";
    trailConfig.maskPath = "Data/Textures/T_TrailMask_08.png";
    staffTrail.Initialize(device, trailConfig);
}

// ============================================================================
// アニメーションインデックス
// ============================================================================
int Player::GetAnimIndex(PlayerAnim anim) const
{
    switch (anim) {
    case PlayerAnim::IDLE_COMBAT:
        return model->GetAnimationIndex("AS_Idle_Combat_Seq_0");
    case PlayerAnim::RUN_START:
        return model->GetAnimationIndex("AS_Run_Combat_F_0_Start_Seq_0");
    case PlayerAnim::RUN_LOOP:
        return model->GetAnimationIndex("AS_Run_Combat_F_0_Loop_Seq_0");
    case PlayerAnim::RUN_STOP:
        return model->GetAnimationIndex("AS_Run_Combat_F_0_Stop_Seq_0");
    case PlayerAnim::JUMP_START:
        return model->GetAnimationIndex("AS_Jump_Combat_Start_0_Seq_0");
    case PlayerAnim::JUMP_LOOP:
        return model->GetAnimationIndex("AS_Jump_Combat_Loop_0_Seq_0");
    case PlayerAnim::JUMP_STOP:
        return model->GetAnimationIndex("AS_Jump_Combat_Stop_0_Seq_0");
    case PlayerAnim::DOUBLE_JUMP:
        return model->GetAnimationIndex("AS_Double_Jump_Combat_0_Seq_0");
    case PlayerAnim::DODGE_F:
        return model->GetAnimationIndex("AS_Dodge_Combat_F_0_Seq_0");
    case PlayerAnim::DODGE_B:
        return model->GetAnimationIndex("AS_Dodge_Combat_B_180_Seq_0");
    case PlayerAnim::AIR_DODGE_F:
        return model->GetAnimationIndex("AS_Dodge_Combat_Air_F_0_Seq_0");
    case PlayerAnim::ATTACK_01_01:
        return model->GetAnimationIndex("AS_Combo_Attack_01_01_Seq_0");
    case PlayerAnim::ATTACK_01_02:
        return model->GetAnimationIndex("AS_Combo_Attack_01_02_Seq_0");
    case PlayerAnim::ATTACK_01_03:
        return model->GetAnimationIndex("AS_Combo_Attack_01_03_Seq_0");
    case PlayerAnim::ATTACK_01_04:
        return model->GetAnimationIndex("AS_Combo_Attack_01_04_Seq_0");
    case PlayerAnim::PROJECTILE_ATTACK_01:
        return model->GetAnimationIndex("AS_Combo_Attack_04_01_Seq_0");
    case PlayerAnim::PROJECTILE_ATTACK_02:
        return model->GetAnimationIndex("AS_Combo_Attack_04_02_Seq_0");
    case PlayerAnim::PROJECTILE_ATTACK_03:
        return model->GetAnimationIndex("AS_Combo_Attack_04_03_Seq_0");
    case PlayerAnim::PROJECTILE_ATTACK_04:
        return model->GetAnimationIndex("AS_Combo_Attack_04_04_Seq_0");
    case PlayerAnim::AIR_ATTACK_01:
        return model->GetAnimationIndex("AS_Combo_Attack_Air_01_Seq_0");
    case PlayerAnim::AIR_ATTACK_02:
        return model->GetAnimationIndex("AS_Combo_Attack_Air_02_Seq_0");
    case PlayerAnim::AIR_ATTACK_03:
        return model->GetAnimationIndex("AS_Combo_Attack_Air_03_Seq_0");
    case PlayerAnim::AIR_ATTACK_04:
        return model->GetAnimationIndex("AS_Combo_Attack_Air_04_Seq_0");
    case PlayerAnim::JUMP_BACK_START:
        return model->GetAnimationIndex("AS_Jump_Combat_Start_B_180_Seq_0");
    case PlayerAnim::JUMP_BACK_END:
        return model->GetAnimationIndex("AS_Jump_Combat_End_B_180_Seq_0");
    case PlayerAnim::COUNTER_ATTACK:
        return model->GetAnimationIndex("AS_Dash_Air_Attack_Seq_0");
    case PlayerAnim::HIT_AIR:
        return model->GetAnimationIndex("AS_Hit_Combat_Air_F_Seq_0");
    case PlayerAnim::HIT_LAUNCH:
        return model->GetAnimationIndex("AS_Hit_Combat_Large_F_Seq_0");
    case PlayerAnim::GET_UP:
        return model->GetAnimationIndex("AS_Get_Up_Combat_B_Seq_0");
    case PlayerAnim::DEATH:
        return model->GetAnimationIndex("AS_Hit_Combat_Large_Death_Seq_0");
    default:
        return -1;
    }
}

// ============================================================================
// サウンド読み込み
// ============================================================================
void Player::LoadAudio()
{
    SoundLoadInfo<PlayerSound> soundFiles[] = {
        {PlayerSound::SPAWN,      "Data/Audio/LightStrike.wav"},
        {PlayerSound::SWING,      "Data/Audio/Swing.wav"},
        {PlayerSound::COUNTER,    "Data/Audio/Counter.wav"},
        {PlayerSound::PISTOL,     "Data/Audio/Pistol.wav"},
        {PlayerSound::STAFF,      "Data/Audio/StaffProj.wav"},
        {PlayerSound::BOW_QUICK,  "Data/Audio/BowQuick.wav"},
        {PlayerSound::BOW_LAUNCH, "Data/Audio/BowLaunch.wav"},
        {PlayerSound::AIR_SLASH,  "Data/Audio/AirSlash.wav"},
        {PlayerSound::STAFF_SPIN, "Data/Audio/StaffSpin.wav"},
        {PlayerSound::AIR_BEAM,   "Data/Audio/AirBeam.wav"},
        {PlayerSound::HIT_SLICE,  "Data/Audio/HitSlice.wav"},
        {PlayerSound::HIT_BLAST,  "Data/Audio/HitBlast.wav"},
        {PlayerSound::DEATH,      "Data/Audio/PlayerDeath.wav"}
    };

    for (const auto& info : soundFiles) {
        auto audio = Audio::Instance().LoadAudioSource(info.filename);
        if (audio) {
            sounds[info.soundId] = std::move(audio);
        }
    }
}