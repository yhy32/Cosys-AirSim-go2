import glob
import math
import os

import mujoco

PARTS = r"C:\BUPT\go2_obj\parts"

for path in sorted(glob.glob(os.path.join(PARTS, "*.obj"))):
    uri = path.replace("\\", "/")
    xml = (
        '<mujoco><asset><mesh name="m" file="%s"/></asset>'
        '<worldbody><geom type="mesh" mesh="m"/></worldbody></mujoco>' % uri
    )
    model = mujoco.MjModel.from_xml_string(xml)
    print(f"{os.path.basename(path):14s} verts={model.mesh_vertnum[0]:7d} faces={model.mesh_facenum[0]:7d}")

# full quadruped preview
def leg(side, x, y, hip_pos):
    return f"""
      <body name="{side}_hip" pos="{hip_pos}">
        <geom type="mesh" mesh="{side}_hip"/>
        <body name="{side}_thigh" pos="0 {0.0955 if y > 0 else -0.0955} 0" quat="0.90045 0 0.43497 0">
          <geom type="mesh" mesh="{side}_thigh"/>
          <body name="{side}_calf" pos="0 0 -0.213" quat="0.62161 0 -0.78333 0">
            <geom type="mesh" mesh="{side}_calf"/>
          </body>
        </body>
      </body>"""

body = "".join(
    leg(s, 0, y, f"{x} {y} 0")
    for s, x, y in [
        ("FL", 0.1934, 0.0465),
        ("FR", 0.1934, -0.0465),
        ("RL", -0.1934, 0.0465),
        ("RR", -0.1934, -0.0465),
    ]
)

xml = f"""<mujoco model="go2_parts_preview">
  <compiler angle="radian" meshdir="{PARTS.replace(chr(92), '/')}"/>
  <visual><global offwidth="800" offheight="600"/></visual>
  <asset>
    <mesh file="base.obj"/>
    <mesh file="FL_hip.obj"/><mesh file="FL_thigh.obj"/><mesh file="FL_calf.obj"/>
    <mesh file="FR_hip.obj"/><mesh file="FR_thigh.obj"/><mesh file="FR_calf.obj"/>
    <mesh file="RL_hip.obj"/><mesh file="RL_thigh.obj"/><mesh file="RL_calf.obj"/>
    <mesh file="RR_hip.obj"/><mesh file="RR_thigh.obj"/><mesh file="RR_calf.obj"/>
  </asset>
  <worldbody>
    <light pos="0 0 2" dir="0 0 -1"/>
    <geom type="plane" size="3 3 0.05" rgba=".7 .7 .7 1"/>
    <body name="base" pos="0 0 0.45">
      <geom type="mesh" mesh="base"/>
      {body}
    </body>
  </worldbody>
</mujoco>"""

model = mujoco.MjModel.from_xml_string(xml)
data = mujoco.MjData(model)
mujoco.mj_forward(model, data)

try:
    renderer = mujoco.Renderer(model, height=600, width=800)
    cam = mujoco.MjvCamera()
    cam.lookat[:] = [0, 0, 0.3]
    cam.distance = 1.8
    cam.azimuth = 135
    cam.elevation = -15
    renderer.update_scene(data, camera=cam)
    img = renderer.render()
    from PIL import Image

    Image.fromarray(img).save(r"C:\BUPT\go2_obj\preview_parts.png")
    print("preview written: C:\\BUPT\\go2_obj\\preview_parts.png")
except Exception as e:
    print("render skipped:", e)
