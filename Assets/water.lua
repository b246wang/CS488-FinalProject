rootNode = gr.node('water_root')
rootNode:scale(1.0, 0.1, 1.0)
rootNode:translate(0.5, 0.1, 0.5)

waterColor = gr.material({0.0, 0.46, 0.74}, {0.1, 0.1, 0.1}, 10)

water = gr.mesh('cube', 'water')
water:set_material(waterColor)
rootNode:add_child(water)

return rootNode;
