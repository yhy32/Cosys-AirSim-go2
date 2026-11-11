import glob
import os

import mujoco

PARTS = r"C:\BUPT\go2_obj\parts_ue"
for path in sorted(glob.glob(os.path.join(PARTS, "*.obj"))):
    uri = path.replace("\\", "/")
    xml = (
        '<mujoco><asset><mesh name="m" file="%s"/></asset>'
        '<worldbody><geom type="mesh" mesh="m"/></worldbody></mujoco>' % uri
    )
    model = mujoco.MjModel.from_xml_string(xml)
    print(f"{os.path.basename(path):14s} verts={model.mesh_vertnum[0]:7d} faces={model.mesh_facenum[0]:7d}")
