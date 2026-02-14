# create_evidence_resources.gd
# 証拠品リソースを生成するスクリプト（エディタで実行）
extends EditorScript

func _run():
	# 証拠品1: エクトプラズム
	var ectoplasm = EvidenceItem.new()
	ectoplasm.id = "ectoplasm"
	ectoplasm.display_name = "Ectoplasm"  # 翻訳は evidence_%id% キーで行う
	ectoplasm.description = "Strange glowing substance"
	ectoplasm.category = "Evidence"
	ResourceSaver.save(ectoplasm, "res://samples/mystery/data/evidence/ectoplasm.tres")
	
	# 証拠品2: 血の足跡
	var footprint = EvidenceItem.new()
	footprint.id = "footprint"
	footprint.display_name = "Footprints"
	footprint.description = "Non-human footprints"
	footprint.category = "Evidence"
	ResourceSaver.save(footprint, "res://samples/mystery/data/evidence/footprint.tres")
	
	# 証拠品3: 破れたメモ
	var torn_memo = EvidenceItem.new()
	torn_memo.id = "torn_memo"
	torn_memo.display_name = "Torn Memo"
	torn_memo.description = "Partially readable text"
	torn_memo.category = "Evidence"
	ResourceSaver.save(torn_memo, "res://samples/mystery/data/evidence/torn_memo.tres")
	
	print("✓ 証拠品リソースを作成しました")
	print("  - ectoplasm.tres")
	print("  - footprint.tres")
	print("  - torn_memo.tres")
