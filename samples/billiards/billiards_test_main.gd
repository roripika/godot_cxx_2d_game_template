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

    # 4. Setup Cushion Visuals
    setup_cushions()

    # 5. Connect Signals
    manager.ball_position_updated.connect(_on_ball_position_updated)

    # 6. Start Simulation
    manager.start_simulation()
    print("Billiards View Layer: Ready. Press SPACE to strike the ball!")

func setup_environment():
    # Camera — テーブル全体が見える俯瞰アングル
    var camera = Camera3D.new()
    camera.position = Vector3(0, 8, 5)
    add_child(camera)
    camera.look_at(Vector3.ZERO)

    # Light
    var light = DirectionalLight3D.new()
    light.position = Vector3(5, 10, 5)
    add_child(light)
    light.look_at(Vector3.ZERO)

    # Floor (テーブルサイズに合わせ 3x6m)
    var floor_mesh = MeshInstance3D.new()
    var plane = PlaneMesh.new()
    plane.size = Vector2(3.0, 6.0)
    floor_mesh.mesh = plane
    var mat = StandardMaterial3D.new()
    mat.albedo_color = Color(0.0, 0.45, 0.1) # ラシャグリーン
    floor_mesh.material_override = mat
    add_child(floor_mesh)

func setup_cue_ball():
    cue_ball_mesh = MeshInstance3D.new()
    var sphere = SphereMesh.new()
    sphere.radius = 0.3  # Jolt側と一致
    sphere.height = 0.6
    cue_ball_mesh.mesh = sphere
    var mat = StandardMaterial3D.new()
    mat.albedo_color = Color(1.0, 1.0, 1.0) # 白球
    cue_ball_mesh.material_override = mat
    add_child(cue_ball_mesh)

## クッション壁を茶色 BoxMesh で可視化。Jolt 側のサイズ・位置と完全一致させる。
func setup_cushions() -> void:
    var cushion_mat = StandardMaterial3D.new()
    cushion_mat.albedo_color = Color(0.5, 0.3, 0.1) # 木目ブラウン

    # [half_size_x, half_size_y, half_size_z, pos_x, pos_y, pos_z]
    var walls = [
        [1.5, 0.5, 0.25,  0.0,  0.5, -3.25],  # 奥壁
        [1.5, 0.5, 0.25,  0.0,  0.5,  3.25],  # 手前壁
        [0.25, 0.5, 3.5, -1.75, 0.5,  0.0 ],  # 左壁
        [0.25, 0.5, 3.5,  1.75, 0.5,  0.0 ],  # 右壁
    ]
    for w in walls:
        var mi = MeshInstance3D.new()
        var box = BoxMesh.new()
        box.size = Vector3(w[0] * 2, w[1] * 2, w[2] * 2)  # half → full size
        mi.mesh = box
        mi.material_override = cushion_mat
        mi.position = Vector3(w[3], w[4], w[5])
        add_child(mi)

func _on_ball_position_updated(new_position: Vector3):
    if cue_ball_mesh:
        cue_ball_mesh.global_position = new_position

func _unhandled_input(event):
    if event.is_action_pressed("ui_accept"): # Default Space/Enter
        strike_ball()

func strike_ball():
    print("Billiards View Layer: Striking cue ball!")
    # 手前(Z+2.0)から奥(-Z)へ向けて打つ
    var direction = Vector3(0, 0, -1)
    var power = 6.0  # m/s — クッションで跳ね返るのに十分な速度
    manager.strike_cue_ball(direction, power)
