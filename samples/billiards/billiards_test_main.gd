extends Node3D

# ─── 定数 ────────────────────────────────────────────────────────────
const MAX_POWER       := 80.0   ## ショット最大速度 (m/s)
const CHARGE_SPEED    := 40.0   ## パワー増加速度 (単位/秒)
const CAM_RADIUS      := 3.0    ## カメラ旋回半径 (m)
const CAM_HEIGHT      := 1.6    ## カメラ高さ (m)
const CUE_LENGTH      := 1.5    ## キュー全長 (m)
const CUE_PULL_MAX    := 0.8    ## フルチャージ時の引き量 (m)
const DRAG_SENS       := 0.008  ## ドラッグ感度 (rad/px)

# ─── ノード参照 ──────────────────────────────────────────────────────
var manager: BilliardsManager
var camera: Camera3D
var cue_mesh: MeshInstance3D
var power_bar: ColorRect   ## 2D HUD — パワーゲージ
var power_bg: ColorRect

# ─── ゲーム状態 ──────────────────────────────────────────────────────
var ball_meshes: Dictionary = {}   ## ID -> MeshInstance3D
var cue_ball_pos := Vector3(0.0, 0.3, 2.0)
var aim_yaw      := 0.0            ## XZ 平面の旋回角 (rad)
var strike_power := 0.0
var is_charging  := false
var already_struck := false        ## 連打防止フラグ

# ════════════════════════════════════════════════════════════════════
func _ready():
    print("Billiards: Initializing...")
    setup_environment()

    manager = BilliardsManager.new()
    add_child(manager)

    setup_cushions()
    setup_pockets()
    setup_cue()
    setup_hud()

    manager.ball_position_updated.connect(_on_ball_position_updated)
    manager.ball_pocketed.connect(_on_ball_pocketed)
    manager.start_simulation()
    setup_game_balls()

    # マウスはキャプチャしない — カーソルを表示したまま使う
    Input.set_mouse_mode(Input.MOUSE_MODE_VISIBLE)
    print("Billiards: Ready. Click & drag to aim, hold LMB to charge, release to shoot!")

# ────────────────────────────────────────────────────────────────────
func setup_environment():
    # ─── WorldEnvironment: 背景をダークグレーにして他シーンが透過しないようにする
    var world_env = WorldEnvironment.new()
    var env = Environment.new()
    env.background_mode = Environment.BG_COLOR
    env.background_color = Color(0.08, 0.08, 0.12)  # 暗い紺
    env.ambient_light_source = Environment.AMBIENT_SOURCE_COLOR
    env.ambient_light_color = Color(0.3, 0.3, 0.3)
    world_env.environment = env
    add_child(world_env)

    # カメラ（後で _process で動かす）
    camera = Camera3D.new()
    camera.position = Vector3(0.0, CAM_HEIGHT, CAM_RADIUS)
    add_child(camera)
    camera.look_at(Vector3.ZERO)

    # ディレクショナルライト
    var light = DirectionalLight3D.new()
    light.position = Vector3(5, 10, 5)
    light.shadow_enabled = true
    add_child(light)
    light.look_at(Vector3.ZERO)

    # 天井ライト（補助）
    var omni = OmniLight3D.new()
    omni.position = Vector3(0, 4, 0)
    omni.omni_range = 8.0
    omni.light_energy = 0.6
    add_child(omni)

    # ラシャ面（3 × 6 m）
    var floor_mi = MeshInstance3D.new()
    var plane = PlaneMesh.new()
    plane.size = Vector2(3.0, 6.0)
    floor_mi.mesh = plane
    var mat = StandardMaterial3D.new()
    mat.albedo_color = Color(0.0, 0.45, 0.1)
    floor_mi.material_override = mat
    add_child(floor_mi)

func setup_cue():
    cue_mesh = MeshInstance3D.new()
    var cyl = CylinderMesh.new()
    cyl.top_radius    = 0.012
    cyl.bottom_radius = 0.025
    cyl.height        = CUE_LENGTH
    cue_mesh.mesh = cyl
    var mat = StandardMaterial3D.new()
    mat.albedo_color = Color(0.85, 0.65, 0.3)  # 木材色
    cue_mesh.material_override = mat
    add_child(cue_mesh)

func setup_hud():
    var canvas = CanvasLayer.new()
    add_child(canvas)

    # 背景バー（グレー）
    power_bg = ColorRect.new()
    power_bg.color = Color(0.2, 0.2, 0.2, 0.75)
    power_bg.set_anchor_and_offset(SIDE_LEFT,   0.5, -60.0)
    power_bg.set_anchor_and_offset(SIDE_RIGHT,  0.5,  60.0)
    power_bg.set_anchor_and_offset(SIDE_TOP,    1.0, -30.0)
    power_bg.set_anchor_and_offset(SIDE_BOTTOM, 1.0, -10.0)
    canvas.add_child(power_bg)

    # パワーバー（赤→黄）
    power_bar = ColorRect.new()
    power_bar.color = Color(1.0, 0.2, 0.1)
    power_bar.set_anchor_and_offset(SIDE_LEFT,   0.5, -60.0)
    power_bar.set_anchor_and_offset(SIDE_RIGHT,  0.5, -60.0)  # 幅 0 から始める
    power_bar.set_anchor_and_offset(SIDE_TOP,    1.0, -30.0)
    power_bar.set_anchor_and_offset(SIDE_BOTTOM, 1.0, -10.0)
    canvas.add_child(power_bar)

func setup_game_balls():
    spawn_ball_view(0, Vector3(0.0, 0.3, 2.0), Color.WHITE, true)

    var apex    = Vector3(0.0, 0.3, -1.0)
    var spacing = 0.3 * 2.1
    var ball_id = 1
    for row in range(5):
        for col in range(row + 1):
            var x_off = (col - row * 0.5) * spacing
            var z_off = -row * spacing * 0.866
            var pos   = apex + Vector3(x_off, 0.0, z_off)
            var color = Color.from_hsv(float(ball_id) / 15.0, 0.8, 0.9)
            spawn_ball_view(ball_id, pos, color, false)
            ball_id += 1

func spawn_ball_view(id: int, pos: Vector3, color: Color, is_cue: bool):
    manager.spawn_ball(id, pos, is_cue)
    var mi = MeshInstance3D.new()
    var sphere = SphereMesh.new()
    sphere.radius = 0.3
    sphere.height = 0.6
    mi.mesh = sphere
    var mat = StandardMaterial3D.new()
    mat.albedo_color = color
    mi.material_override = mat
    add_child(mi)
    mi.global_position = pos
    ball_meshes[id] = mi

func setup_cushions() -> void:
    var cushion_mat = StandardMaterial3D.new()
    cushion_mat.albedo_color = Color(0.5, 0.3, 0.1)
    var walls = [
        [1.5, 0.5, 0.25,  0.0,  0.5, -3.25],
        [1.5, 0.5, 0.25,  0.0,  0.5,  3.25],
        [0.25, 0.5, 3.5, -1.75, 0.5,  0.0 ],
        [0.25, 0.5, 3.5,  1.75, 0.5,  0.0 ],
    ]
    for w in walls:
        var mi  = MeshInstance3D.new()
        var box = BoxMesh.new()
        box.size = Vector3(w[0] * 2, w[1] * 2, w[2] * 2)
        mi.mesh = box
        mi.material_override = cushion_mat
        mi.position = Vector3(w[3], w[4], w[5])
        add_child(mi)

## 6か所のポケットを黒い薄いディスクで可視化。C++側と同じ座標とする。
func setup_pockets() -> void:
    var pocket_mat = StandardMaterial3D.new()
    pocket_mat.albedo_color = Color(0.0, 0.0, 0.0)

    var pockets = [
        Vector3(-1.5, 0.0, -3.0),  # 左奥
        Vector3( 1.5, 0.0, -3.0),  # 右奥
        Vector3(-1.5, 0.0,  0.0),  # 左中
        Vector3( 1.5, 0.0,  0.0),  # 右中
        Vector3(-1.5, 0.0,  3.0),  # 左手前
        Vector3( 1.5, 0.0,  3.0),  # 右手前
    ]
    for p in pockets:
        var mi  = MeshInstance3D.new()
        var cyl = CylinderMesh.new()
        cyl.top_radius    = 0.42
        cyl.bottom_radius = 0.42
        cyl.height        = 0.01  # 薄いディスク
        mi.mesh = cyl
        mi.material_override = pocket_mat
        mi.position = p
        add_child(mi)

# ════════════════════════════════════════════════════════════════════
#  毎フレーム処理
# ════════════════════════════════════════════════════════════════════
func _process(delta: float) -> void:
    _update_charge(delta)
    _update_camera()
    _update_cue()
    _update_hud()

func _update_charge(delta: float) -> void:
    if is_charging and not already_struck:
        strike_power = min(strike_power + CHARGE_SPEED * delta, MAX_POWER)
        # チャージが満タンになったら自動発射
        if strike_power >= MAX_POWER:
            _do_strike()

func _update_camera() -> void:
    if not is_instance_valid(camera):
        return
    var cx = cue_ball_pos.x + sin(aim_yaw) * CAM_RADIUS
    var cz = cue_ball_pos.z + cos(aim_yaw) * CAM_RADIUS
    camera.position = Vector3(cx, cue_ball_pos.y + CAM_HEIGHT, cz)
    camera.look_at(cue_ball_pos, Vector3.UP)

func _update_cue() -> void:
    if not is_instance_valid(cue_mesh):
        return
    # キューの引き量 ── チャージ量に比例して後退
    var pull = (strike_power / MAX_POWER) * CUE_PULL_MAX
    # キューの中心 = 手球の後方 (カメラ側) へ (CUE_LENGTH/2 + pull) オフセット
    var back_dir = Vector3(sin(aim_yaw), 0.0, cos(aim_yaw))  # カメラ方向 = 手球の背後
    var cue_center = cue_ball_pos + back_dir * (CUE_LENGTH * 0.5 + 0.3 + pull)
    cue_mesh.position = Vector3(cue_center.x, cue_ball_pos.y, cue_center.z)
    # 先端を手球へ向ける
    cue_mesh.look_at(cue_ball_pos, Vector3.UP)
    # look_at は -Z を前方とするが CylinderMesh の軸は Y なので 90° 補正
    cue_mesh.rotate_object_local(Vector3.RIGHT, PI * 0.5)

func _update_hud() -> void:
    if not is_instance_valid(power_bar):
        return
    var ratio = strike_power / MAX_POWER
    # 幅 0〜120 px でパワーを表示（左端 -60 → 右端 -60 + 120*ratio）
    power_bar.set_anchor_and_offset(SIDE_RIGHT, 0.5, -60.0 + 120.0 * ratio)
    power_bar.color = Color(ratio, 1.0 - ratio * 0.8, 0.1)

# ════════════════════════════════════════════════════════════════════
#  入力処理  ── クリック&ドラッグでエイム。マウスは常に可視。
# ════════════════════════════════════════════════════════════════════
func _unhandled_input(event: InputEvent) -> void:
    if event is InputEventMouseButton and event.button_index == MOUSE_BUTTON_LEFT:
        if event.pressed:
            # ─── ドラッグ開始 + チャージ開始
            if not already_struck:
                is_charging  = true
                strike_power = 0.0
        else:
            # ─── ドラッグ終了 = 発射
            if is_charging:
                _do_strike()

    # ドラッグ中の左右移動でエイム
    if event is InputEventMouseMotion:
        if is_charging and not already_struck:
            aim_yaw -= event.relative.x * DRAG_SENS

func _do_strike() -> void:
    if already_struck:
        return
    already_struck = true
    is_charging = false

    # カメラから手球へ向かうベクトル = ショット方向
    var shot_dir = Vector3(-sin(aim_yaw), 0.0, -cos(aim_yaw)).normalized()
    print("Billiards: STRIKE! dir=", shot_dir, " power=", strike_power)
    manager.strike_cue_ball(shot_dir, strike_power)

    # 2 秒後にリセット可能にする（ボールが静止するまでの猶予）
    await get_tree().create_timer(2.0).timeout
    already_struck = false
    strike_power   = 0.0

# ════════════════════════════════════════════════════════════════════
#  シグナル受信
# ════════════════════════════════════════════════════════════════════
func _on_ball_position_updated(id: int, new_position: Vector3) -> void:
    if ball_meshes.has(id):
        ball_meshes[id].global_position = new_position
    if id == 0:
        cue_ball_pos = new_position

## ポケット判定シグナルのコールバック
func _on_ball_pocketed(id: int) -> void:
    if id == 0:
        # ─── スクラッチ（手球落下）
        print("Billiards: SCRATCH! 手球がポケットに落ちました...")
        # 手球メッシュを一時的に非表示
        if ball_meshes.has(0):
            ball_meshes[0].visible = false

        # already_struck リセット（発射禁止を解除しない — respawn後に解除）
        await get_tree().create_timer(2.0).timeout

        # C++ 側で再生成
        manager.respawn_cue_ball()

        # メッシュを初期位置に戻して再表示
        var initial_pos = Vector3(0.0, 0.3, 2.0)
        cue_ball_pos = initial_pos
        if ball_meshes.has(0):
            ball_meshes[0].global_position = initial_pos
            ball_meshes[0].visible = true

        already_struck = false
        strike_power   = 0.0
        print("Billiards: 手球を初期位置に復帰しました。ゲームを再開できます。")
    else:
        # ─── 的球ポケットイン
        print("Billiards: 的球 ", id, " をポケットイン！ ナイスショット！")
        if ball_meshes.has(id):
            ball_meshes[id].queue_free()
            ball_meshes.erase(id)
