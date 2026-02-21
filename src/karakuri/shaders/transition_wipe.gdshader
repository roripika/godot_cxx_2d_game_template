shader_type canvas_item;

// トランジション進行度 (0.0: 完全に表示, 1.0: 完全に非表示)
uniform float progress : hint_range(0.0, 1.0) = 0.0;

// タイルの分割数 (例: vec2(10.0, 10.0)). vec2(1.0, 1.0)にすると通常のグラデーションになります。
uniform vec2 tile_count = vec2(10.0, 10.0);

// ワイプの方向と広がり具合
// vec2(1.0, 0.0) -> 左から右
// vec2(0.0, 1.0) -> 上から下
// smooth_size はグラデーションの境界のぼかし幅
uniform vec2 wipe_direction = vec2(1.0, 0.0);
uniform float smooth_size : hint_range(0.0, 1.0) = 0.1;

// タイル内で縮小しながら消える効果を入れるか (0.0: 無効, 1.0: 有効)
uniform float scale_effect : hint_range(0.0, 1.0) = 0.0;

void fragment() {
    // 現在のUV
    vec2 uv = UV;

    // 現在のピクセルが属するタイルのインデックスと、タイル内のローカルUV(0.0~1.0)
    vec2 tile_id = floor(uv * tile_count);
    vec2 tile_uv = fract(uv * tile_count);

    // ノイズやワイプの基準となる、タイルの正規化された位置 (0.0 ~ 1.0)
    // タイル分割数が1の場合はUVそのものと同義になります
    vec2 norm_tile_pos = (tile_id + vec2(0.5)) / tile_count;

    // ベースのカラーを取得
    vec4 tex_color = texture(TEXTURE, uv);

    // タイルの位置とワイプ方向から、このタイルが消えるべきタイミング（閾値）を計算
    // dot演算でワイプ方向への投影距離を取得
    float wipe_val = dot(norm_tile_pos, wipe_direction);

    // wipe_directionがマイナスの成分を持つ場合（右から左など）に対応するための正規化
    float dir_length = abs(wipe_direction.x) + abs(wipe_direction.y);
    if (dir_length > 0.0) {
        wipe_val /= dir_length;
        // マイナス方向の場合は反転して 0.0 ~ 1.0 に収める
        if (wipe_direction.x < 0.0) wipe_val = 1.0 + wipe_val;
        if (wipe_direction.y < 0.0) wipe_val = 1.0 + wipe_val;
    }

    // smoothStepでグラデーションの境界を作る
    // progressが0の時は全タイルが1.0、progressが1の時は全タイルが0.0になるよう調整
    float adjusted_progress = progress * (1.0 + smooth_size) - smooth_size;
    float alpha_mask = smoothstep(adjusted_progress, adjusted_progress + smooth_size, wipe_val);

    // スケールエフェクト（タイルごとに小さくなりながら消える）
    if (scale_effect > 0.0) {
        // タイルの中心からの距離
        vec2 center_dist = abs(tile_uv - vec2(0.5)) * 2.0;
        // タイルの端にいくほど早く消えるようにする
        float dist_sq = max(center_dist.x, center_dist.y);
        // alpha_mask(0.0~1.0)を半径として利用
        float scale_mask = step(dist_sq, alpha_mask + (1.0 - scale_effect));
        alpha_mask = min(alpha_mask, scale_mask);
    }

    // 最終的なアルファ値を乗算
    COLOR = vec4(tex_color.rgb, tex_color.a * alpha_mask);
}
