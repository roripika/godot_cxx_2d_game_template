#include "evidence_manager.h"

#include "mystery_manager.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace mystery {

EvidenceManager *EvidenceManager::singleton = nullptr;

EvidenceManager::EvidenceManager() {
  ERR_FAIL_COND(singleton != nullptr);
  singleton = this;
}

EvidenceManager::~EvidenceManager() {
  ERR_FAIL_COND(singleton != this);
  singleton = nullptr;
}

EvidenceManager *EvidenceManager::get_singleton() { return singleton; }

void EvidenceManager::_bind_methods() {
  ClassDB::bind_method(D_METHOD("add_evidence", "p_id"),
                       &EvidenceManager::add_evidence);
  ClassDB::bind_method(D_METHOD("remove_evidence", "p_id"),
                       &EvidenceManager::remove_evidence);
  ClassDB::bind_method(D_METHOD("has_evidence", "p_id"),
                       &EvidenceManager::has_evidence);
  ClassDB::bind_method(D_METHOD("get_all_evidence"),
                       &EvidenceManager::get_all_evidence);
  ClassDB::bind_method(D_METHOD("clear_all_evidence"),
                       &EvidenceManager::clear_all_evidence);
  ClassDB::bind_method(D_METHOD("serialize_evidence"),
                       &EvidenceManager::serialize_evidence);
  ClassDB::bind_method(D_METHOD("deserialize_evidence", "p_data"),
                       &EvidenceManager::deserialize_evidence);
}

bool EvidenceManager::add_evidence(const String &p_id) {
  if (has_evidence(p_id)) {
    UtilityFunctions::print("[EvidenceManager] Evidence already acquired: ",
                            p_id);
    return false;
  }

  collected_evidence.push_back(p_id);
  auto *mm = MysteryManager::get_singleton();
  if (mm) {
    mm->log_change("Evidence", p_id, "NONE", "COLLECTED", "add_evidence");
  } else {
    UtilityFunctions::print(
        "[CHANGE] Evidence: ", p_id,
        " | Old: NONE | New: COLLECTED | Caller: add_evidence");
  }
  return true;
}

bool EvidenceManager::remove_evidence(const String &p_id) {
  int index = collected_evidence.find(p_id);
  if (index != -1) {
    collected_evidence.remove_at(index);
    auto *mm = MysteryManager::get_singleton();
    if (mm) {
      mm->log_change("Evidence", p_id, "COLLECTED", "REMOVED",
                     "remove_evidence");
    } else {
      UtilityFunctions::print(
          "[CHANGE] Evidence: ", p_id,
          " | Old: COLLECTED | New: REMOVED | Caller: remove_evidence");
    }
    return true;
  }
  return false;
}

bool EvidenceManager::has_evidence(const String &p_id) const {
  return collected_evidence.has(p_id);
}

Array EvidenceManager::get_all_evidence() const {
  return collected_evidence.duplicate();
}

void EvidenceManager::clear_all_evidence() {
  collected_evidence.clear();
  UtilityFunctions::print("[CHANGE] Evidence: ALL | Old: NUMEROUS | New: "
                          "CLEARED | Caller: clear_all_evidence");
}

Array EvidenceManager::serialize_evidence() const { return get_all_evidence(); }

void EvidenceManager::deserialize_evidence(const Array &p_data) {
  collected_evidence = p_data.duplicate();
  UtilityFunctions::print("[EvidenceManager] Deserialized evidence. Count: ",
                          collected_evidence.size());
}

} // namespace mystery
