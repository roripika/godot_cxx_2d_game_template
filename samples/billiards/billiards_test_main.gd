extends Node3D

var manager: BilliardsManager
var cue_ball_mesh: MeshInstance3D

func _ready():
    print("Billiards View Layer: Initializing...")
    
    # 1. Setup 3D Environment
    setup_environment()
    
    # 2. Setup BilliardsManager (C++)
    manager = BilliardsManager.new()
    add_child(manager)
    
    # 3. Setup Cue Ball Mesh
    setup_cue_ball()
    
    # 4. Connect Signals
    manager.ball_position_updated.connect(_on_ball_position_updated)
    
    # 5. Start Simulation
    manager.start_simulation()
    print("Billiards View Layer: Ready. Press SPACE to strike the ball!")

func setup_environment():
    # Camera
    var camera = Camera3D.new()
    camera.position = Vector3(0, 7, 7)
    add_child(camera)
    camera.look_at(Vector3.ZERO)
    
    # Light
    var light = DirectionalLight3D.new()
    light.position = Vector3(5, 10, 5)
    add_child(light)
    light.look_at(Vector3.ZERO)
    
    # Floor (Billiards Table Surface)
    var floor_mesh = MeshInstance3D.new()
    var plane = PlaneMesh.new()
    plane.size = Vector2(100, 100)
    floor_mesh.mesh = plane
    
    var material = StandardMaterial3D.new()
    material.albedo_color = Color(0.0, 0.4, 0.1) # Green
    floor_mesh.material_override = material
    
    add_child(floor_mesh)

func setup_cue_ball():
    cue_ball_mesh = MeshInstance3D.new()
    var sphere = SphereMesh.new()
    sphere.radius = 1.0 # Matches Jolt sphere radius
    sphere.height = 2.0
    cue_ball_mesh.mesh = sphere
    
    var material = StandardMaterial3D.new()
    material.albedo_color = Color(1.0, 1.0, 1.0) # White
    cue_ball_mesh.material_override = material
    
    add_child(cue_ball_mesh)

func _on_ball_position_updated(new_position: Vector3):
    if cue_ball_mesh:
        cue_ball_mesh.global_position = new_position

func _unhandled_input(event):
    if event.is_action_pressed("ui_accept"): # Default Space/Enter
        strike_ball()

func strike_ball():
    print("Billiards View Layer: Striking cue ball!")
    # Strike towards negative Z (into the screen)
    var direction = Vector3(0, 0, -1)
    var power = 8.0  # ビリヤード実速: ~2-10 m/s
    manager.strike_cue_ball(direction, power)
