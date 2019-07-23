rootNode = gr.node('balloon_root')
rootNode:scale(0.5, 0.5, 0.5)
rootNode:translate(0.5, 0.5, 0.5)

balloonColor = gr.material({0.2, 1.0, 1.0}, {0.1, 0.1, 0.1}, 10)

balloon = gr.mesh('sphere', 'balloon')
balloon:set_material(balloonColor)
rootNode:add_child(balloon)

return rootNode;
