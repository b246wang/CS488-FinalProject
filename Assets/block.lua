rootNode = gr.node('block_root')
rootNode:translate(0.5, 0.5, 0.5)

blockColor = gr.material({1.0, 1.0, 1.0}, {0.1, 0.1, 0.1}, 10)

block = gr.mesh('cube', 'block')
block:set_material(blockColor)
rootNode:add_child(block)

return rootNode;
