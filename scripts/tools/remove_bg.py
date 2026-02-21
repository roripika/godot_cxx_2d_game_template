"""
背景透過自動化スクリプト (remove_bg.py)

概要:
指定された画像ファイル、またはディレクトリ内の画像に対して、
rembg (U2-Net) を用いて自動的に背景を透過し保存する汎用ツール。

使い方:
  python remove_bg.py <input_path> [-o <output_path>] [-e <extension>]
"""

import os
import argparse
from pathlib import Path
from rembg import remove
from PIL import Image

def process_image(input_path, output_path=None):
    # 出力先が指定されていない場合は上書きする
    if not output_path:
        output_path = input_path
    
    print(f"Processing: {input_path}...")
    try:
        input_image = Image.open(input_path)
        output_image = remove(input_image)
        output_image.save(output_path)
        print(f"Success: Saved to {output_path}")
    except Exception as e:
        print(f"Error processing {input_path}: {e}")

def main():
    parser = argparse.ArgumentParser(description="rembgを使用して画像の背景を透過するスクリプト")
    parser.add_argument("input", help="入力する画像ファイル名、またはフォルダパス")
    parser.add_argument("-o", "--output", help="出力先のファイル名、またはフォルダパス (省略時は入力ファイルを上書き)")
    parser.add_argument("-e", "--extension", default=".png", help="フォルダ指定時に処理対象とする拡張子 (デフォルト: .png)")
    
    args = parser.parse_args()
    input_path = Path(args.input)
    
    if input_path.is_file():
        # 単一ファイル処理モード
        output_path = args.output if args.output else str(input_path)
        process_image(str(input_path), output_path)
    elif input_path.is_dir():
        # ディレクトリの一括処理モード
        output_dir = Path(args.output) if args.output else input_path
        if not output_dir.exists():
            output_dir.mkdir(parents=True) # 出力先フォルダがなければ作成
            
        for img_path in input_path.rglob(f"*{args.extension}"):
            if not img_path.is_file():
                continue
                
            out_file = output_dir / img_path.name
            process_image(str(img_path), str(out_file))
    else:
        print(f"Error: Path not found - {args.input}")

if __name__ == "__main__":
    main()
