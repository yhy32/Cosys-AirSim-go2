import json
import math
import os

import numpy as np

SRC = r"C:\BUPT\mujoco_menagerie\unitree_go2"
ASSETS = os.path.join(SRC, "assets")
OUT = r"C:\BUPT\go2_obj\parts"

MATERIALS = {
    "black": (0.0, 0.0, 0.0),
    "white": (1.0, 1.0, 1.0),
    "gray": (0.671705, 0.692426, 0.774270),
    "metal": (0.9, 0.95, 0.95),
}

# part -> list of (obj file, material, quat wxyz, translation m)
PARTS = {
    "base": [
        ("base_0.obj", "black", None, (0, 0, 0)),
        ("base_1.obj", "black", None, (0, 0, 0)),
        ("base_2.obj", "black", None, (0, 0, 0)),
        ("base_3.obj", "white", None, (0, 0, 0)),
        ("base_4.obj", "gray", None, (0, 0, 0)),
    ],
    "FL_hip": [
        ("hip_0.obj", "metal", None, (0, 0, 0)),
        ("hip_1.obj", "gray", None, (0, 0, 0)),
    ],
    "FR_hip": [
        ("hip_0.obj", "metal", (4.63268e-05, 1, 0, 0), (0, 0, 0)),
        ("hip_1.obj", "gray", (4.63268e-05, 1, 0, 0), (0, 0, 0)),
    ],
    "RL_hip": [
        ("hip_0.obj", "metal", (4.63268e-05, 0, 1, 0), (0, 0, 0)),
        ("hip_1.obj", "gray", (4.63268e-05, 0, 1, 0), (0, 0, 0)),
    ],
    "RR_hip": [
        ("hip_0.obj", "metal", (2.14617e-09, 4.63268e-05, 4.63268e-05, -1), (0, 0, 0)),
        ("hip_1.obj", "gray", (2.14617e-09, 4.63268e-05, 4.63268e-05, -1), (0, 0, 0)),
    ],
}

THIGH = {"FL": "thigh", "FR": "thigh_mirror", "RL": "thigh", "RR": "thigh_mirror"}
CALF = {"FL": "calf", "FR": "calf_mirror", "RL": "calf", "RR": "calf_mirror"}

for side in ["FL", "FR", "RL", "RR"]:
    PARTS[f"{side}_thigh"] = [
        (f"{THIGH[side]}_0.obj", "metal", None, (0, 0, 0)),
        (f"{THIGH[side]}_1.obj", "gray", None, (0, 0, 0)),
    ]
    PARTS[f"{side}_calf"] = [
        (f"{CALF[side]}_0.obj", "gray", None, (0, 0, 0)),
        (f"{CALF[side]}_1.obj", "black", None, (0, 0, 0)),
        ("foot.obj", "black", None, (0, 0, -0.213)),
    ]

JOINTS = [
    ("FL_hip", "base", "FL_hip_joint", [1, 0, 0], [0.1934, 0.0465, 0], [-1.0472, 1.0472]),
    ("FR_hip", "base", "FR_hip_joint", [1, 0, 0], [0.1934, -0.0465, 0], [-1.0472, 1.0472]),
    ("RL_hip", "base", "RL_hip_joint", [1, 0, 0], [-0.1934, 0.0465, 0], [-1.0472, 1.0472]),
    ("RR_hip", "base", "RR_hip_joint", [1, 0, 0], [-0.1934, -0.0465, 0], [-1.0472, 1.0472]),
    ("FL_thigh", "FL_hip", "FL_thigh_joint", [0, 1, 0], [0, 0.0955, 0], [-1.5708, 3.4907]),
    ("FR_thigh", "FR_hip", "FR_thigh_joint", [0, 1, 0], [0, -0.0955, 0], [-1.5708, 3.4907]),
    ("RL_thigh", "RL_hip", "RL_thigh_joint", [0, 1, 0], [0, 0.0955, 0], [-0.5236, 4.5379]),
    ("RR_thigh", "RR_hip", "RR_thigh_joint", [0, 1, 0], [0, -0.0955, 0], [-0.5236, 4.5379]),
    ("FL_calf", "FL_thigh", "FL_calf_joint", [0, 1, 0], [0, 0, -0.213], [-2.7227, -0.83776]),
    ("FR_calf", "FR_thigh", "FR_calf_joint", [0, 1, 0], [0, 0, -0.213], [-2.7227, -0.83776]),
    ("RL_calf", "RL_thigh", "RL_calf_joint", [0, 1, 0], [0, 0, -0.213], [-2.7227, -0.83776]),
    ("RR_calf", "RR_thigh", "RR_calf_joint", [0, 1, 0], [0, 0, -0.213], [-2.7227, -0.83776]),
]


def quat_to_matrix(q):
    if q is None:
        return np.eye(3)
    w, x, y, z = q
    n = math.sqrt(w * w + x * x + y * y + z * z)
    w, x, y, z = w / n, x / n, y / n, z / n
    return np.array(
        [
            [1 - 2 * (y * y + z * z), 2 * (x * y - z * w), 2 * (x * z + y * w)],
            [2 * (x * y + z * w), 1 - 2 * (x * x + z * z), 2 * (y * z - x * w)],
            [2 * (x * z - y * w), 2 * (y * z + x * w), 1 - 2 * (x * x + y * y)],
        ]
    )


def read_obj(path):
    verts, uvs, normals, faces = [], [], [], []
    with open(path, "r", encoding="utf-8", errors="ignore") as f:
        for line in f:
            parts = line.split()
            if not parts:
                continue
            if parts[0] == "v":
                verts.append([float(parts[1]), float(parts[2]), float(parts[3])])
            elif parts[0] == "vt":
                uvs.append([float(parts[1]), float(parts[2])])
            elif parts[0] == "vn":
                normals.append([float(parts[1]), float(parts[2]), float(parts[3])])
            elif parts[0] == "f":
                face = []
                for token in parts[1:]:
                    idx = token.split("/")
                    vi = int(idx[0])
                    ti = int(idx[1]) if len(idx) > 1 and idx[1] else 0
                    ni = int(idx[2]) if len(idx) > 2 and idx[2] else 0
                    face.append((vi, ti, ni))
                faces.append(face)
    return np.array(verts, dtype=float), np.array(uvs, dtype=float), np.array(normals, dtype=float), faces


def export_part(name, sources):
    all_v, all_vt, all_vn = [], [], []
    face_blocks = []
    v_off = vt_off = vn_off = 0

    for obj, material, quat, trans in sources:
        v, vt, vn, faces = read_obj(os.path.join(ASSETS, obj))
        R = quat_to_matrix(quat)
        v = v @ R.T + np.array(trans, dtype=float)
        if len(vn):
            vn = vn @ R.T

        all_v.append(v)
        all_vt.append(vt)
        all_vn.append(vn)
        face_blocks.append((material, faces, v_off, vt_off, vn_off))
        v_off += len(v)
        vt_off += len(vt)
        vn_off += len(vn)

    out_path = os.path.join(OUT, f"{name}.obj")
    with open(out_path, "w", encoding="utf-8") as f:
        f.write(f"# Go2 part '{name}' exported from mujoco_menagerie (MuJoCo body frame, meters)\n")
        f.write("mtllib go2_parts.mtl\n")
        f.write(f"o {name}\n")
        for arr in all_v:
            for x, y, z in arr:
                f.write(f"v {x:.6f} {y:.6f} {z:.6f}\n")
        for arr in all_vt:
            for u, v in arr:
                f.write(f"vt {u:.6f} {v:.6f}\n")
        for arr in all_vn:
            for x, y, z in arr:
                f.write(f"vn {x:.6f} {y:.6f} {z:.6f}\n")
        for material, faces, vo, to, no in face_blocks:
            f.write(f"usemtl {material}\n")
            for face in faces:
                toks = []
                for vi, ti, ni in face:
                    vi = vi if vi > 0 else len(all_v) + vi
                    ti = ti if ti > 0 else 0
                    ni = ni if ni > 0 else 0
                    vi = vi + vo
                    ti = ti + to if ti else 0
                    ni = ni + no if ni else 0
                    toks.append(f"{vi}/{ti}/{ni}" if ti and ni else f"{vi}//{ni}" if ni else str(vi))
                f.write("f " + " ".join(toks) + "\n")

    verts = np.vstack(all_v)
    return {
        "obj": f"parts/{name}.obj",
        "vertices": int(len(verts)),
        "bounds_min_m": [round(float(x), 4) for x in verts.min(axis=0)],
        "bounds_max_m": [round(float(x), 4) for x in verts.max(axis=0)],
    }


def main():
    os.makedirs(OUT, exist_ok=True)

    with open(os.path.join(OUT, "go2_parts.mtl"), "w", encoding="utf-8") as f:
        for name, (r, g, b) in MATERIALS.items():
            f.write(f"newmtl {name}\nKd {r} {g} {b}\nKa 0 0 0\nKs 0.1 0.1 0.1\nNs 20\n\n")

    manifest = {
        "source": SRC,
        "unit": "meter",
        "coordinate_system": "MuJoCo body frames: +X forward, +Y left, +Z up (right-handed)",
        "ue_import_scale": 100,
        "ue_note": "Import OBJ at scale 100 to get centimeters. MuJoCo +Y is left; UE +Y is right, "
                   "so either mirror the root (Scale Y=-1) or treat +Y as left and keep joint signs as-is.",
        "parts": [],
        "joints": [],
    }

    for name, sources in PARTS.items():
        info = export_part(name, sources)
        info["name"] = name
        manifest["parts"].append(info)

    for name, parent, joint, axis, pos, limits in JOINTS:
        manifest["joints"].append(
            {
                "name": joint,
                "child_part": name,
                "parent_part": parent,
                "axis_mujoco": axis,
                "pos_in_parent_m": pos,
                "pos_in_parent_cm": [round(c * 100, 3) for c in pos],
                "range_rad": limits,
                "range_deg": [round(math.degrees(v), 2) for v in limits],
            }
        )

    manifest_path = os.path.join(os.path.dirname(OUT), "manifest.json")
    with open(manifest_path, "w", encoding="utf-8") as f:
        json.dump(manifest, f, indent=2, ensure_ascii=False)

    for part in manifest["parts"]:
        print(f"{part['name']:10s} verts={part['vertices']:7d}  bounds={part['bounds_min_m']} .. {part['bounds_max_m']}")


if __name__ == "__main__":
    main()
