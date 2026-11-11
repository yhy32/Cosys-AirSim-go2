import sys

import mujoco
import numpy as np
from PIL import Image

part = sys.argv[1] if len(sys.argv) > 1 else "base"
obj = rf"C:\BUPT\go2_obj\parts\{part}.obj".replace("\\", "/")
out = rf"C:\BUPT\go2_obj\preview_{part}.png"

xml = f"""
<mujoco>
  <visual>
    <global offwidth="800" offheight="600"/>
  </visual>
  <asset>
    <mesh name="m" file="{obj}"/>
  </asset>
  <worldbody>
    <light pos="0 0 2" dir="0 0 -1"/>
    <geom type="plane" size="2 2 0.05" rgba=".75 .75 .75 1"/>
    <body name="part">
      <geom type="mesh" mesh="m" rgba=".85 .87 .9 1"/>
    </body>
  </worldbody>
</mujoco>
"""

model = mujoco.MjModel.from_xml_string(xml)
data = mujoco.MjData(model)
mujoco.mj_forward(model, data)

# frame the mesh using its bounding box
adr = int(model.mesh_vertadr[0])
num = int(model.mesh_vertnum[0])
verts = model.mesh_vert[adr:adr + num]
mn = verts.min(axis=0)
mx = verts.max(axis=0)
center = (mn + mx) / 2
radius = float(np.linalg.norm(mx - mn)) / 2

renderer = mujoco.Renderer(model, height=600, width=800)
cam = mujoco.MjvCamera()
cam.lookat[:] = center
cam.distance = radius * 2.6
cam.azimuth = 135
cam.elevation = -18
renderer.update_scene(data, camera=cam)
Image.fromarray(renderer.render()).save(out)
print("wrote", out, "aabb", mn.round(4), mx.round(4))
