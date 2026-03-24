# gltf2cpp

glTF 2.0 ファイルを xiamocon (xmc) 用の C++ ヘッダファイルに変換するツール。

## 必要なもの

- Python 3.8+

## セットアップ

```bash
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

## 使い方

```bash
source .venv/bin/activate
python gltf2cpp.py <入力ファイル> <出力ファイル> [--prefix <プレフィックス>]
```

### パラメータ

| パラメータ | 説明 |
|---|---|
| `<入力ファイル>` | 入力 glTF ファイル (`.gltf` または `.glb`) |
| `<出力ファイル>` | 出力 C++ ヘッダファイル (`.hpp`) |
| `--prefix, -p` | 生成する C++ 識別子のプレフィックス（省略時はファイル名から生成） |

## 生成されるコード

- ベースカラーテクスチャ → `xmc::Sprite` (ARGB4444 フォーマット)
- マテリアル (`baseColorFactor` + テクスチャ) → `xmc::Material`
- 頂点座標 → `xmc::Vec3Buffer`
- 法線 → `xmc::Vec3Buffer`
- 頂点カラー → `xmc::ColorBuffer`
- UV → `xmc::Vec2Buffer`
- インデックス → `xmc::IndexBuffer`
- プリミティブ → `xmc::Primitive`
- メッシュ → `xmc::Mesh`

属性やマテリアルが glTF ファイルに含まれない場合は `nullptr` が設定されます。
