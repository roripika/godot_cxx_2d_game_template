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
  if (p_what == NOTIFICATION_POSTINITIALIZE) {
    set_physics_process(true);
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

  // Create Floor
  JPH::BoxShapeSettings floor_shape_settings(JPH::Vec3(50.0f, 1.0f, 50.0f));
  JPH::Shape::ShapeResult floor_shape_result = floor_shape_settings.Create();
  if (floor_shape_result.HasError()) {
    UtilityFunctions::printerr(
        "BilliardsManager: Failed to create floor shape: ",
        floor_shape_result.GetError().c_str());
    return;
  }
  JPH::ShapeRefC floor_shape = floor_shape_result.Get();

  JPH::BodyCreationSettings floor_settings(
      floor_shape, JPH::RVec3(0.0f, -1.0f, 0.0f), JPH::Quat::sIdentity(),
      JPH::EMotionType::Static, Layers::NON_MOVING);
  body_interface.CreateAndAddBody(floor_settings,
                                  JPH::EActivation::DontActivate);

  // Create Cue Ball
  JPH::SphereShapeSettings ball_shape_settings(1.0f);
  JPH::ShapeRefC ball_shape = ball_shape_settings.Create().Get();

  JPH::BodyCreationSettings ball_settings(
      ball_shape, JPH::RVec3(0.0f, 1.0f, 0.0f), JPH::Quat::sIdentity(),
      JPH::EMotionType::Dynamic, Layers::MOVING);
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
    JPH::Vec3 impulse(p_direction.x * p_power, p_direction.y * p_power,
                      p_direction.z * p_power);
    // スリープ中でも力が反映されるよう、先にボディをアクティブ化する
    body_interface.ActivateBody(jolt_data->cue_ball_id);
    body_interface.AddImpulse(jolt_data->cue_ball_id, impulse);

    UtilityFunctions::print("BilliardsManager: Struck cue ball with power: ",
                            p_power);
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

      emit_signal("ball_position_updated", gpos);
    }
  }
}

} // namespace billiards
