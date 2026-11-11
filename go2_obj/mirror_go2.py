import glob
import json
import os

SRC = r"C:\BUPT\go2_obj\parts_single"
DST = r"C:\BUPT\go2_obj\parts_ue"


def mirror_obj(src, dst):
    out = []
    with open(src, "r", encoding="utf-8", errors="ignore") as f:
        for line in f:
            s = line.split()
            if not s:
                continue
            if s[0] == "v":
                out.append(f"v {float(s[1]):.6f} {-float(s[2]):.6f} {float(s[3]):.6f}\n")
            elif s[0] == "vn":
                out.append(f"vn {float(s[1]):.6f} {-float(s[2]):.6f} {float(s[3]):.6f}\n")
            elif s[0] == "f":
                # reflect y and reverse winding so the surface stays outward
                out.append("f " + " ".join(reversed(s[1:])) + "\n")
            elif s[0] in ("mtllib", "usemtl"):
                continue
            else:
                out.append(line)
    with open(dst, "w", encoding="utf-8", newline="\n") as f:
        f.writelines(out)


def main():
    os.makedirs(DST, exist_ok=True)
    files = sorted(glob.glob(os.path.join(SRC, "*.obj")))
    for src in files:
        mirror_obj(src, os.path.join(DST, os.path.basename(src)))
    print("mirrored", len(files), "files ->", DST)

    manifest_path = r"C:\BUPT\go2_obj\manifest.json"
    with open(manifest_path, "r", encoding="utf-8") as f:
        manifest = json.load(f)

    for joint in manifest["joints"]:
        pos = joint["pos_in_parent_m"]
        joint["pos_in_parent_m"] = [pos[0], -pos[1], pos[2]]
        cm = joint["pos_in_parent_cm"]
        joint["pos_in_parent_cm"] = [cm[0], -cm[1], cm[2]]
        if joint["axis_mujoco"] == [0, 1, 0]:
            joint["angle_sign_ue"] = -1
        else:
            joint["angle_sign_ue"] = 1

    manifest["source"] = r"C:\BUPT\mujoco_menagerie\unitree_go2"
    manifest["coordinate_system"] = ("UE left-handed: +X forward, +Y right, +Z up; "
                                     "mirrored from MuJoCo (+Y left)")
    manifest["parts"] = [p for p in manifest["parts"]]
    for part in manifest["parts"]:
        part["obj"] = part["obj"].replace("parts/", "parts_ue/")

    with open(r"C:\BUPT\go2_obj\manifest_ue.json", "w", encoding="utf-8") as f:
        json.dump(manifest, f, indent=2, ensure_ascii=False)
    print("wrote manifest_ue.json")


if __name__ == "__main__":
    main()
