#include "GameObjects/Enemy/Enemy.h"
#include "Audio/Audio.h"

// ============================================================================
// アニメーションインデックス
// ============================================================================

int Enemy::GetAnimIndex(EnemyAnim anim) const
{
    switch (anim) {
    case EnemyAnim::IDLE_COMBAT:
        return model->GetAnimationIndex("Idle_Combat_Seq_0");
    case EnemyAnim::WALK_START_F:
        return model->GetAnimationIndex("Walk_Combat_Start_F_0_RM_Seq_0");
    case EnemyAnim::WALK_LOOP_F:
        return model->GetAnimationIndex("Walk_Combat_Loop_F_0_RM_Seq_0");
    case EnemyAnim::WALK_STOP_F:
        return model->GetAnimationIndex("Walk_Combat_Stop_F_0_RM_Seq_0");
    case EnemyAnim::WALK_START_F_L_90:
        return model->GetAnimationIndex("Walk_Combat_Start_F_L_90_RM_Seq_0");
    case EnemyAnim::WALK_LOOP_F_L_90:
        return model->GetAnimationIndex("Walk_Combat_Loop_F_L_90_RM_Seq_0");
    case EnemyAnim::WALK_STOP_F_L_90:
        return model->GetAnimationIndex("Walk_Combat_Stop_F_L_90_RM_Seq_0");
    case EnemyAnim::RUN_START_F:
        return model->GetAnimationIndex("Run_Combat_Start_F_0_RM_Seq_0");
    case EnemyAnim::RUN_LOOP_F:
        return model->GetAnimationIndex("Run_Combat_Loop_F_0_RM_Seq_0");
    case EnemyAnim::RUN_ATTACK_01:
        return model->GetAnimationIndex("Run_Attack_01_Seq_0");
    case EnemyAnim::DODGE_B:
        return model->GetAnimationIndex("Dodge_Combat_B_Seq_0");
    case EnemyAnim::BLOCK_START:
        return model->GetAnimationIndex("Block_Start_Seq_0");
    case EnemyAnim::BLOCK_LOOP:
        return model->GetAnimationIndex("Block_Loop_Seq_0");
    case EnemyAnim::BLOCK_HIT:
        return model->GetAnimationIndex("Block_Hit_Seq_0");
    case EnemyAnim::BLOCK_END:
        return model->GetAnimationIndex("Block_End_Seq_0");
    case EnemyAnim::BLOCK_HIT_BREAK:
        return model->GetAnimationIndex("Block_Hit_Break_Seq_0");
    case EnemyAnim::JUMP_START:
        return model->GetAnimationIndex("Jump_Combat_Start_0_Seq1_0");
    case EnemyAnim::JUMP_LOOP:
        return model->GetAnimationIndex("Jump_Combat_Loop_0_Seq_0");
    case EnemyAnim::JUMP_STOP:
        return model->GetAnimationIndex("Jump_Combat_End_0_Seq1_0");
    case EnemyAnim::POWER_ATTACK_START:
        return model->GetAnimationIndex("Power_Attack_Start_Seq_0");
    case EnemyAnim::POWER_ATTACK_LOOP:
        return model->GetAnimationIndex("Power_Attack_Loop_Seq_0");
    case EnemyAnim::POWER_ATTACK_END:
        return model->GetAnimationIndex("Power_Attack_End_Seq_0");
    case EnemyAnim::COMBO_ATTACK_01:
        return model->GetAnimationIndex("Combo_Attack_03_01_Seq_0");
    case EnemyAnim::COMBO_ATTACK_02:
        return model->GetAnimationIndex("Combo_Attack_03_02_Seq_0");
    case EnemyAnim::COMBO_ATTACK_03:
        return model->GetAnimationIndex("Combo_Attack_03_03_Seq_0");
    case EnemyAnim::COMBO_ATTACK_04:
        return model->GetAnimationIndex("Combo_Attack_03_04_Seq_0");
    case EnemyAnim::HIT_LAUNCH:
        return model->GetAnimationIndex("Hit_Large_Combat_F_Seq_0");
    case EnemyAnim::KNOCK_DOWN_START:
        return model->GetAnimationIndex("Knock_Down_Combat_Start_Seq_0");
    case EnemyAnim::KNOCK_DOWN_LOOP:
        return model->GetAnimationIndex("Knock_Down_Combat_Loop_Seq_0");
    case EnemyAnim::KNOCK_DOWN_END:
        return model->GetAnimationIndex("Knock_Down_Combat_End_Seq_0");
    case EnemyAnim::B_IDLE:
        return model->GetAnimationIndex("AS_Idle_Combat_Seq_0");
    case EnemyAnim::B_JUMP_END:
        return model->GetAnimationIndex("AS_Jump_Combat_Stop_0_Seq_0");
    case EnemyAnim::AIM_START:
        return model->GetAnimationIndex("AS_Aim_The_Target_Start_Seq_0");
    case EnemyAnim::AIM_LOOP:
        return model->GetAnimationIndex("AS_Aim_The_Target_Loop_Seq_0");
    case EnemyAnim::AIM_END:
        return model->GetAnimationIndex("AS_Aim_The_Target_End_Seq_0");
    case EnemyAnim::METEOR_SLASH01:
        return model->GetAnimationIndex("AS_Attack_01_01_Seq_0");
    case EnemyAnim::METEOR_SLASH02:
        return model->GetAnimationIndex("AS_Attack_01_02_Seq_0");
    case EnemyAnim::METEOR_BLAST01:
        return model->GetAnimationIndex("AS_Attack_02_01_Seq_0");
    case EnemyAnim::METEOR_BLAST02:
        return model->GetAnimationIndex("AS_Attack_02_02_Seq_0");
    case EnemyAnim::DEATH:
        return model->GetAnimationIndex("Hit_Large_Death_Seq_0");
    default:
        return -1;
    }
}

// ============================================================================
// サウンド読み込み
// ============================================================================

void Enemy::LoadAudio()
{
    SoundLoadInfo<EnemySound> soundFiles[] = {
        {EnemySound::SPAWN,           "Data/Audio/EnemySpawn.wav"},
        {EnemySound::SLASH,           "Data/Audio/EnemySlash.wav"},
        {EnemySound::MULTI_SLASH,     "Data/Audio/EnemyMultiSlash.wav"},
        {EnemySound::PROJECTILE,      "Data/Audio/EnemyProjectile.wav"},
        {EnemySound::METEOR_SPAWN,    "Data/Audio/EnemyMeteorSpwan.wav"},
        {EnemySound::METEOR_STRIKE,   "Data/Audio/EnemyMeteorImpact.wav"},
        {EnemySound::AREA_WARNING,    "Data/Audio/AreaWarning.wav"},
        {EnemySound::TELEPORT,        "Data/Audio/Teleport.wav"},
        {EnemySound::BERSERK_ON,      "Data/Audio/BersekOn.wav"},
        {EnemySound::HIT,             "Data/Audio/EnemyHit.wav"},
        {EnemySound::HIT_PROJECTILE,  "Data/Audio/EnemyHitProjectile.wav"},
        {EnemySound::DEATH,           "Data/Audio/EnemyDeath.wav"},
    };

    for (const auto& info : soundFiles) {
        auto audio = Audio::Instance().LoadAudioSource(info.filename);
        if (audio) {
            enemySounds[info.soundId] = std::move(audio);
        }
    }
}