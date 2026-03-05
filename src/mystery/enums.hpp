#ifndef MYSTERY_ENUMS_HPP
#define MYSTERY_ENUMS_HPP

#include <godot_cpp/variant/string.hpp>

namespace mystery {

enum class MysteryFlag {
  INTRO_DONE,
  ALL_EVIDENCE_COLLECTED,
  DEDUCTION_COMPLETE,
  CASE_SOLVED,
  GAME_OVER,
  PERFECT_ENDING
};

enum class EvidenceID {
  ECTOPLASM,
  FOOTPRINT,
  TORN_MEMO,
  DELIVERY_LOG,
  WITNESS_REPORT
};

class MysteryEnumValueConverter {
public:
  static godot::String get_flag_key(MysteryFlag flag) {
    switch (flag) {
    case MysteryFlag::INTRO_DONE:
      return "intro_done";
    case MysteryFlag::ALL_EVIDENCE_COLLECTED:
      return "all_evidence_collected";
    case MysteryFlag::DEDUCTION_COMPLETE:
      return "deduction_complete";
    case MysteryFlag::CASE_SOLVED:
      return "case_solved";
    case MysteryFlag::GAME_OVER:
      return "game_over";
    case MysteryFlag::PERFECT_ENDING:
      return "perfect_ending";
    default:
      return "unknown";
    }
  }

  static godot::String get_evidence_key(EvidenceID id) {
    switch (id) {
    case EvidenceID::ECTOPLASM:
      return "ectoplasm";
    case EvidenceID::FOOTPRINT:
      return "footprint";
    case EvidenceID::TORN_MEMO:
      return "torn_memo";
    case EvidenceID::DELIVERY_LOG:
      return "delivery_log";
    case EvidenceID::WITNESS_REPORT:
      return "witness_report";
    default:
      return "unknown";
    }
  }
};

} // namespace mystery

#endif // MYSTERY_ENUMS_HPP
