import mujoco
import numpy as np
from PIL import Image

path = r"C:\BUPT\mujoco_menagerie\unitree_go2\_marker_scene.xml"
model = mujoco.MjModel.from_xml_path(path)
data = mujoco.MjData(model)
data.qpos[:] = model.key_qpos[0]
mujoco.mj_forward(model, data)

renderer = mujoco.Renderer(model, height=600, width=800)
for name, azimuth in [("side", 90), ("iso", 135)]:
    cam = mujoco.MjvCamera()
    cam.lookat[:] = [0, 0, 0.27]
    cam.distance = 1.6
    cam.azimuth = azimuth
    cam.elevation = -10
    renderer.update_scene(data, camera=cam)
    Image.fromarray(renderer.render()).save(rf"C:\BUPT\go2_obj\head_{name}.png")
    print("wrote", name)
