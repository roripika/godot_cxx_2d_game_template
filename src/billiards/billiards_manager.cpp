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
  int debug_frame_count = 0;  // 診断用フレームカウンター

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
  ClassDB::bind_method(D_METHOD("strike_cue_ball", "direction", "power"),
                       &BilliardsManager::strike_cue_ball);

  ADD_SIGNAL(MethodInfo("ball_position_updated",
                        PropertyInfo(Variant::VECTOR3, "new_position")));
}

void BilliardsManager::_notification(int p_what) {
  if (p_what == NOTIFICATION_READY) {
    set_physics_process(true);
    UtilityFunctions::print("BilliardsManager: physics_process ENABLED");
  }
}

BilliardsManager::BilliardsManager() {
  UtilityFunctions::print("BilliardsManager: Constructor started.");

  // 1. Initialize Factory
  if (JPH::Factory::sInstance == nullptr) {
    UtilityFunctions::print("BilliardsManager: Registering Jolt Types...");
    JPH::RegisterDefaultAllocator();
    JPH::Factory::sInstance = new JPH::Factory();
    JPH::RegisterTypes();
  }

  UtilityFunctions::print("BilliardsManager: Allocating JoltData...");
  jolt_data = new JoltData();

  // 2. Setup Allocator and Job System
  UtilityFunctions::print(
      "BilliardsManager: Setting up TempAllocator and JobSystem...");
  jolt_data->temp_allocator =
      new JPH::TempAllocatorImpl(10 * 1024 * 1024); // 10MB
  jolt_data->job_system = new JPH::JobSystemThreadPool(
      JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, 4);

  // 3. Initialize Physics System
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

  // ── ヘルパー: Static な壁を1行で追加 ──────────────────────────────────
  auto add_static_box = [&](JPH::Vec3 half_size, JPH::RVec3 pos,
                             float friction, float restitution) {
    JPH::BoxShapeSettings s(half_size);
    JPH::ShapeRefC shape = s.Create().Get();
    JPH::BodyCreationSettings bs(shape, pos, JPH::Quat::sIdentity(),
                                  JPH::EMotionType::Static, Layers::NON_MOVING);
    bs.mFriction    = friction;
    bs.mRestitution = restitution;
    body_interface.CreateAndAddBody(bs, JPH::EActivation::DontActivate);
  };

  // ── 床 ────────────────────────────────────────────────────────────────
  add_static_box(JPH::Vec3(500.0f, 1.0f, 500.0f),
                 JPH::RVec3(0.0f, -1.0f, 0.0f), 0.5f, 0.5f);

  // ── クッション壁（テーブル内寸 X:3m / Z:6m）─────────────────────────
  // 奥壁 (Z-)
  add_static_box(JPH::Vec3(1.5f, 0.5f, 0.25f),
                 JPH::RVec3(0.0f, 0.5f, -3.25f), 0.2f, 0.8f);
  // 手前壁 (Z+)
  add_static_box(JPH::Vec3(1.5f, 0.5f, 0.25f),
                 JPH::RVec3(0.0f, 0.5f, 3.25f), 0.2f, 0.8f);
  // 左壁 (X-)
  add_static_box(JPH::Vec3(0.25f, 0.5f, 3.5f),
                 JPH::RVec3(-1.75f, 0.5f, 0.0f), 0.2f, 0.8f);
  // 右壁 (X+)
  add_static_box(JPH::Vec3(0.25f, 0.5f, 3.5f),
                 JPH::RVec3(1.75f, 0.5f, 0.0f), 0.2f, 0.8f);

  // ── 手球 ──────────────────────────────────────────────────────────────
  JPH::SphereShapeSettings ball_shape_settings(0.3f); // 実寸: 直径 6cm
  JPH::ShapeRefC ball_shape = ball_shape_settings.Create().Get();

  JPH::BodyCreationSettings ball_settings(
      ball_shape, JPH::RVec3(0.0f, 1.0f, 2.0f), JPH::Quat::sIdentity(),
      JPH::EMotionType::Dynamic, Layers::MOVING);
  ball_settings.mFriction        = 0.2f;
  ball_settings.mRestitution     = 0.9f;
  ball_settings.mLinearDamping   = 0.6f;  // ラシャ摩擦で自然減速
  ball_settings.mAngularDamping  = 0.6f;
  jolt_data->cue_ball_id = body_interface.CreateAndAddBody(
      ball_settings, JPH::EActivation::Activate);

  UtilityFunctions::print("BilliardsManager: Simulation started. Cue Ball "
                          "created at (0, 1, 0). ID: ",
                          jolt_data->cue_ball_id.GetIndex());
}

void BilliardsManager::strike_cue_ball(godot::Vector3 p_direction,
                                       float p_power) {
  if (jolt_data && jolt_data->physics_system &&
      !jolt_data->cue_ball_id.IsInvalid()) {
    JPH::BodyInterface &body_interface =
        jolt_data->physics_system->GetBodyInterface();
    JPH::Vec3 velocity(p_direction.x * p_power, p_direction.y * p_power,
                       p_direction.z * p_power);
    // AddImpulse は質量依存 (4000kg球では 60N·s ≒ 0.014m/s にしかならない)
    // SetLinearVelocity で「方向 × 速度(m/s)」をダイレクトに指定する
    body_interface.ActivateBody(jolt_data->cue_ball_id);
    body_interface.SetLinearVelocity(jolt_data->cue_ball_id, velocity);

    JPH::RVec3 pos = body_interface.GetPosition(jolt_data->cue_ball_id);
    JPH::Vec3  vel = body_interface.GetLinearVelocity(jolt_data->cue_ball_id);
    UtilityFunctions::print("BilliardsManager: Struck cue ball with power: ", p_power,
                            "  pos=(", pos.GetX(), ",", pos.GetY(), ",", pos.GetZ(), ")",
                            "  vel=(", vel.GetX(), ",", vel.GetY(), ",", vel.GetZ(), ")");
  }
}

void BilliardsManager::_physics_process(double delta) {
  if (jolt_data && jolt_data->physics_system) {
    // Step the physics world
    jolt_data->physics_system->Update(
        (float)delta, 1, jolt_data->temp_allocator, jolt_data->job_system);

    // Sync position back to Godot
    if (!jolt_data->cue_ball_id.IsInvalid()) {
      JPH::BodyInterface &body_interface =
          jolt_data->physics_system->GetBodyInterface();
      JPH::RVec3 pos = body_interface.GetPosition(jolt_data->cue_ball_id);
      Vector3 gpos(pos.GetX(), pos.GetY(), pos.GetZ());

      // 60フレームに1回座標をログ出力（診断用）
      jolt_data->debug_frame_count++;
      if (jolt_data->debug_frame_count % 60 == 0) {
        UtilityFunctions::print(
            "[BilliardsManager] frame=", jolt_data->debug_frame_count,
            " ball_pos=(", gpos.x, ", ", gpos.y, ", ", gpos.z, ")");
      }

      emit_signal("ball_position_updated", gpos);
    } else {
      UtilityFunctions::printerr("[BilliardsManager] cue_ball_id is INVALID — start_simulation() が呼ばれていない可能性");
    }
  } else {
    UtilityFunctions::printerr("[BilliardsManager] physics_system is NULL");
  }
}

} // namespace billiards
