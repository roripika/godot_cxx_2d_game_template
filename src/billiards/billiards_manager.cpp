#include <Jolt/Jolt.h>

// Jolt Physics Headers
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/RegisterTypes.h>

// Project Headers
#include "billiards_manager.h"

// Godot Headers
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

// Standard Headers
#include <map>
#include <vector>

using namespace godot;

namespace billiards {

// Layer definitions
namespace Layers {
static constexpr JPH::ObjectLayer NON_MOVING = 0;
static constexpr JPH::ObjectLayer MOVING = 1;
static constexpr JPH::ObjectLayer NUM_LAYERS = 2;
} // namespace Layers

namespace BroadPhaseLayers {
static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
static constexpr JPH::BroadPhaseLayer MOVING(1);
static constexpr JPH::uint NUM_LAYERS = 2;
} // namespace BroadPhaseLayers

// Opaque data implementation
struct BilliardsManager::JoltData {
  JPH::TempAllocatorImpl *temp_allocator = nullptr;
  JPH::JobSystemThreadPool *job_system = nullptr;
  JPH::PhysicsSystem *physics_system = nullptr;

  JPH::BodyID cue_ball_id;
  std::map<int, JPH::BodyID> balls; // ID maps to Jolt BodyID

  int debug_frame_count = 0;

  // Filters
  class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter {
  public:
    bool ShouldCollide(JPH::ObjectLayer inLayer1,
                       JPH::ObjectLayer inLayer2) const override {
      switch (inLayer1) {
      case Layers::NON_MOVING:
        return inLayer2 == Layers::MOVING;
      case Layers::MOVING:
        return true;
      default:
        return false;
      }
    }
  };

  class BpLayerInterfaceImpl : public JPH::BroadPhaseLayerInterface {
  public:
    BpLayerInterfaceImpl() {
      mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
      mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
    }
    JPH::uint GetNumBroadPhaseLayers() const override {
      return BroadPhaseLayers::NUM_LAYERS;
    }
    JPH::BroadPhaseLayer
    GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override {
      return mObjectToBroadPhase[inLayer];
    }
#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
    const char *
    GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override {
      switch ((JPH::BroadPhaseLayer::Type)inLayer) {
      case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:
        return "NON_MOVING";
      case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:
        return "MOVING";
      default:
        return "INVALID";
      }
    }
#endif
  private:
    JPH::BroadPhaseLayer mObjectToBroadPhase[Layers::NUM_LAYERS];
  };

  class ObjectVsBpLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter {
  public:
    bool ShouldCollide(JPH::ObjectLayer inLayer1,
                       JPH::BroadPhaseLayer inLayer2) const override {
      switch (inLayer1) {
      case Layers::NON_MOVING:
        return inLayer2 == BroadPhaseLayers::MOVING;
      case Layers::MOVING:
        return true;
      default:
        return false;
      }
    }
  };

  ObjectLayerPairFilterImpl object_vs_object_filter;
  BpLayerInterfaceImpl broad_phase_layer_interface;
  ObjectVsBpLayerFilterImpl object_vs_broadphase_filter;
};

void BilliardsManager::_bind_methods() {
  ClassDB::bind_method(D_METHOD("start_simulation"),
                       &BilliardsManager::start_simulation);
  ClassDB::bind_method(D_METHOD("spawn_ball", "id", "position", "is_cue_ball"),
                       &BilliardsManager::spawn_ball, DEFVAL(false));
  ClassDB::bind_method(D_METHOD("strike_cue_ball", "direction", "power"),
                       &BilliardsManager::strike_cue_ball);

  ClassDB::bind_method(D_METHOD("respawn_cue_ball"),
                       &BilliardsManager::respawn_cue_ball);

  ADD_SIGNAL(MethodInfo("ball_position_updated",
                        PropertyInfo(Variant::INT, "id"),
                        PropertyInfo(Variant::VECTOR3, "new_position")));
  ADD_SIGNAL(MethodInfo("ball_pocketed",
                        PropertyInfo(Variant::INT, "id")));
}

void BilliardsManager::_notification(int p_what) {
  if (p_what == NOTIFICATION_READY) {
    set_physics_process(true);
    UtilityFunctions::print("BilliardsManager: physics_process ENABLED");
  }
}

BilliardsManager::BilliardsManager() {
  UtilityFunctions::print("BilliardsManager: Constructor started.");

  if (JPH::Factory::sInstance == nullptr) {
    UtilityFunctions::print("BilliardsManager: Registering Jolt Types...");
    JPH::RegisterDefaultAllocator();
    JPH::Factory::sInstance = new JPH::Factory();
    JPH::RegisterTypes();
  }

  UtilityFunctions::print("BilliardsManager: Allocating JoltData...");
  jolt_data = new JoltData();

  UtilityFunctions::print(
      "BilliardsManager: Setting up TempAllocator and JobSystem...");
  jolt_data->temp_allocator =
      new JPH::TempAllocatorImpl(10 * 1024 * 1024); // 10MB
  jolt_data->job_system = new JPH::JobSystemThreadPool(
      JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, 4);

  UtilityFunctions::print("BilliardsManager: Initializing PhysicsSystem...");
  jolt_data->physics_system = new JPH::PhysicsSystem();
  jolt_data->physics_system->Init(1024, 0, 1024, 1024,
                                  jolt_data->broad_phase_layer_interface,
                                  jolt_data->object_vs_broadphase_filter,
                                  jolt_data->object_vs_object_filter);

  UtilityFunctions::print(
      "BilliardsManager: Jolt Physics Initialized Successfully.");
}

BilliardsManager::~BilliardsManager() {
  if (jolt_data) {
    UtilityFunctions::print("BilliardsManager: Cleaning up Jolt Physics...");
    delete jolt_data->physics_system;
    delete jolt_data->job_system;
    delete jolt_data->temp_allocator;
    delete jolt_data;
    jolt_data = nullptr;
  }
}

void BilliardsManager::start_simulation() {
  if (!jolt_data || !jolt_data->physics_system)
    return;

  UtilityFunctions::print("BilliardsManager: start_simulation() called.");
  JPH::BodyInterface &body_interface =
      jolt_data->physics_system->GetBodyInterface();

  auto add_static_box = [&](JPH::Vec3 half_size, JPH::RVec3 pos, float friction,
                            float restitution) {
    JPH::BoxShapeSettings s(half_size);
    JPH::ShapeRefC shape = s.Create().Get();
    JPH::BodyCreationSettings bs(shape, pos, JPH::Quat::sIdentity(),
                                 JPH::EMotionType::Static, Layers::NON_MOVING);
    bs.mFriction = friction;
    bs.mRestitution = restitution;
    body_interface.CreateAndAddBody(bs, JPH::EActivation::DontActivate);
  };

  // 床
  add_static_box(JPH::Vec3(500.0f, 1.0f, 500.0f), JPH::RVec3(0.0f, -1.0f, 0.0f),
                 0.5f, 0.5f);

  // クッション壁
  add_static_box(JPH::Vec3(1.5f, 0.5f, 0.25f), JPH::RVec3(0.0f, 0.5f, -3.25f),
                 0.2f, 0.8f);
  add_static_box(JPH::Vec3(1.5f, 0.5f, 0.25f), JPH::RVec3(0.0f, 0.5f, 3.25f),
                 0.2f, 0.8f);
  add_static_box(JPH::Vec3(0.25f, 0.5f, 3.5f), JPH::RVec3(-1.75f, 0.5f, 0.0f),
                 0.2f, 0.8f);
  add_static_box(JPH::Vec3(0.25f, 0.5f, 3.5f), JPH::RVec3(1.75f, 0.5f, 0.0f),
                 0.2f, 0.8f);

  UtilityFunctions::print("BilliardsManager: Table setup complete.");
}

void BilliardsManager::spawn_ball(int p_id, godot::Vector3 p_position,
                                  bool p_is_cue_ball) {
  if (!jolt_data || !jolt_data->physics_system)
    return;

  JPH::BodyInterface &body_interface =
      jolt_data->physics_system->GetBodyInterface();

  JPH::SphereShapeSettings ball_shape_settings(0.3f);
  JPH::ShapeRefC ball_shape = ball_shape_settings.Create().Get();

  JPH::BodyCreationSettings ball_settings(
      ball_shape, JPH::RVec3(p_position.x, p_position.y, p_position.z),
      JPH::Quat::sIdentity(), JPH::EMotionType::Dynamic, Layers::MOVING);

  ball_settings.mFriction = 0.2f;
  ball_settings.mRestitution = 0.9f;
  ball_settings.mLinearDamping = 0.6f;
  ball_settings.mAngularDamping = 0.6f;

  JPH::BodyID bid = body_interface.CreateAndAddBody(ball_settings,
                                                    JPH::EActivation::Activate);
  jolt_data->balls[p_id] = bid;

  if (p_is_cue_ball) {
    jolt_data->cue_ball_id = bid;
  }

  UtilityFunctions::print("BilliardsManager: Spawned ball ID=", p_id,
                          " bid=", bid.GetIndex(), " is_cue=", p_is_cue_ball);
}

void BilliardsManager::strike_cue_ball(godot::Vector3 p_direction,
                                       float p_power) {
  if (jolt_data && jolt_data->physics_system &&
      !jolt_data->cue_ball_id.IsInvalid()) {
    JPH::BodyInterface &body_interface =
        jolt_data->physics_system->GetBodyInterface();
    JPH::Vec3 velocity(p_direction.x * p_power, p_direction.y * p_power,
                       p_direction.z * p_power);

    body_interface.ActivateBody(jolt_data->cue_ball_id);
    body_interface.SetLinearVelocity(jolt_data->cue_ball_id, velocity);

    UtilityFunctions::print("BilliardsManager: Struck cue ball with power: ",
                            p_power);
  }
}

// ─── ポケット座標（テーブル内寸: X±1.5, Z±3.0）─────────────────────
static const JPH::Vec3 POCKET_POSITIONS[] = {
    {-1.5f, 0.0f, -3.0f}, // 左奥
    { 1.5f, 0.0f, -3.0f}, // 右奥
    {-1.5f, 0.0f,  0.0f}, // 左中
    { 1.5f, 0.0f,  0.0f}, // 右中
    {-1.5f, 0.0f,  3.0f}, // 左手前
    { 1.5f, 0.0f,  3.0f}, // 右手前
};
static constexpr float POCKET_RADIUS = 0.45f; // 球半径(0.3)×1.5
static constexpr int   POCKET_COUNT  = 6;

void BilliardsManager::_physics_process(double delta) {
  if (!jolt_data || !jolt_data->physics_system)
    return;

  jolt_data->physics_system->Update(
      (float)delta, 1, jolt_data->temp_allocator, jolt_data->job_system);

  JPH::BodyInterface &body_interface =
      jolt_data->physics_system->GetBodyInterface();

  // ─── 位置シグナル発行 & ポケット判定 ───────────────────────────────
  std::vector<int> pocketed_ids;

  for (auto const &[id, bid] : jolt_data->balls) {
    if (bid.IsInvalid())
      continue;

    JPH::RVec3 jpos = body_interface.GetPosition(bid);
    Vector3 gpos(jpos.GetX(), jpos.GetY(), jpos.GetZ());
    emit_signal("ball_position_updated", id, gpos);

    // ポケット距離チェック（XZ 平面のみ）
    for (int p = 0; p < POCKET_COUNT; ++p) {
      float dx = jpos.GetX() - POCKET_POSITIONS[p].GetX();
      float dz = jpos.GetZ() - POCKET_POSITIONS[p].GetZ();
      if (dx * dx + dz * dz < POCKET_RADIUS * POCKET_RADIUS) {
        pocketed_ids.push_back(id);
        break;
      }
    }
  }

  // ─── ポケットイン処理（イテレーション外で実施）────────────────────
  for (int id : pocketed_ids) {
    auto it = jolt_data->balls.find(id);
    if (it == jolt_data->balls.end())
      continue;

    JPH::BodyID bid = it->second;
    body_interface.RemoveBody(bid);
    body_interface.DestroyBody(bid);
    jolt_data->balls.erase(it);

    if (id == 0)
      jolt_data->cue_ball_id = JPH::BodyID(); // 無効化

    UtilityFunctions::print("BilliardsManager: Ball ", id, " pocketed.");
    emit_signal("ball_pocketed", id);
  }
}

void BilliardsManager::respawn_cue_ball() {
  if (!jolt_data || !jolt_data->physics_system)
    return;

  // 既存の手球が残っていれば削除
  if (!jolt_data->cue_ball_id.IsInvalid()) {
    JPH::BodyInterface &bi = jolt_data->physics_system->GetBodyInterface();
    bi.RemoveBody(jolt_data->cue_ball_id);
    bi.DestroyBody(jolt_data->cue_ball_id);
    jolt_data->balls.erase(0);
  }

  // 手球を初期位置で再生成
  spawn_ball(0, Vector3(0.0f, 0.3f, 2.0f), true);
  UtilityFunctions::print("BilliardsManager: Cue ball respawned.");
}

} // namespace billiards
