extends Node2D

func _ready():
	print("Sandbox Demo: Init")
	
	# 1. Create World Data
	var world_data = UniversalWorldData.new()
	world_data.initialize(Vector3i(10, 10, 1))
	
	# Fill some blocks (e.g. at 1,1)
	world_data.set_voxel(Vector3i(1, 1, 0), 1) # Block ID 1 at (1,1)
	print("World Data Initialized. Voxel at (1,1): ", world_data.get_voxel(Vector3i(1, 1, 0)))
	
	# 2. Create Components
	var miner = MiningComponent.new()
	add_child(miner)
	miner.set_world_data(world_data)
	miner.position = Vector2(0, 0) # Player at 0,0
	
	var builder = BuildingComponent.new()
	add_child(builder)
	builder.set_world_data(world_data)
	builder.position = Vector2(0, 0)
	
	# 3. Test Mining
	print("Testing Mining at (32, 32)... (Grid 1,1)")
	var mine_result = miner.mine(Vector2(32, 32)) # target 1,1 (assuming 32px tile)
	print("Mining Result: ", mine_result)
	print("Voxel at (1,1) after mining: ", world_data.get_voxel(Vector3i(1, 1, 0)))
	
	# 4. Test Building
	print("Testing Building at (64, 64)... (Grid 2,2)")
	var build_result = builder.place_voxel(Vector2(64, 64), 2) # Place ID 2 at 2,2
	print("Building Result: ", build_result)
	print("Voxel at (2,2) after building: ", world_data.get_voxel(Vector3i(2, 2, 0)))
	
	if mine_result and build_result:
		print("Sandbox Verification: SUCCESS")
	else:
		print("Sandbox Verification: FAILURE")
	
	get_tree().quit()
