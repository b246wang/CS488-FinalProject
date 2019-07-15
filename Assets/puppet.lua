-- Create the top level root node named 'root'.
rootNode = gr.node('root')
rootNode:translate(0.0, 0.02, 0.0)

skinColor = gr.material({1.0, 0.6313, 0.8352}, {0.1, 0.1, 0.1}, 10)
torsoColor = gr.material({0.2588, 0.5255, 0.9568}, {0.1, 0.1, 0.1}, 10)
noseColor = gr.material({0.6196, 0.1176, 0.3961}, {0.1, 0.1, 0.1}, 10)
footColor = gr.material({0.1019, 0.0509, 0.0}, {0.1, 0.1, 0.1}, 10)
handColor = gr.material({0.8, 0.6313, 0.8352}, {0.1, 0.1, 0.1}, 10)
white = gr.material({1.0, 1.0, 1.0}, {0.1, 0.1, 0.1}, 10)

bodyRatio = {1.0, 0.8, 0.8}
torso = gr.mesh('sphere', 'torso')
torso:scale(bodyRatio[1], bodyRatio[2], bodyRatio[3])
torso:set_material(torsoColor)
rootNode:add_child(torso)

neck = gr.node('neck')
torso:add_child(neck)
neck:scale(1.0/bodyRatio[1], 1.0/bodyRatio[2], 1.0/bodyRatio[3])
neck:translate(0.0, 0.4, 0.0)

neckJoint = gr.joint('neckJoint', {-50, 0, 50}, {-70, 0, 70});
neck:add_child(neckJoint)

headRatio = {0.7, 0.7, 0.7}
head = gr.mesh('sphere', 'head')
head:scale(headRatio[1], headRatio[2], headRatio[3])
head:translate(0.0, 0.7, 0.0)
head:set_material(skinColor)

neckJoint:add_child(head)

nose1 = gr.mesh('sphere', 'nose1')
nose1:scale(1/headRatio[1]*0.2, 1/headRatio[2]*0.2, 1/headRatio[3]*0.05)
nose1:translate(0.0, 0.0, 0.95)
nose1:set_material(skinColor)
head:add_child(nose1)

nose2 = gr.mesh('sphere', 'nose2')
nose2:scale(0.2, 0.2, 1.0)
nose2:translate(-0.25, 0.0, 0.95)
nose2:set_material(noseColor)
nose1:add_child(nose2)

nose3 = gr.mesh('sphere', 'nose3')
nose3:scale(0.2, 0.2, 1.0)
nose3:translate(0.25, 0.0, 0.95)
nose3:set_material(noseColor)
nose1:add_child(nose3)

-- left ear
leftEarNode = gr.node('leftEarNode')
head:add_child(leftEarNode)
leftEarNode:scale(1.0/bodyRatio[1], 1.0/bodyRatio[2], 1.0/bodyRatio[3])
leftEarNode:translate(-0.5, 0.8, 0.0)

leftEarJoint = gr.joint('leftEarJoint', {-25, 0, 25}, {0, 0, 0});
leftEarNode:add_child(leftEarJoint)

leftEar = gr.mesh('sphere', 'leftEar')
earRatio = {0.15, 0.2, 0.05}
leftEar:scale(earRatio[1], earRatio[2], earRatio[3])
leftEar:set_material(skinColor)
leftEarJoint:add_child(leftEar)

-- right ear
rightEarNode = gr.node('rightEarNode')
head:add_child(rightEarNode)
rightEarNode:scale(1.0/bodyRatio[1], 1.0/bodyRatio[2], 1.0/bodyRatio[3])
rightEarNode:translate(0.5, 0.8, 0.0)

rightEarJoint = gr.joint('rightEarJoint', {-25, 0, 25}, {0, 0, 0});
rightEarNode:add_child(rightEarJoint)

rightEar = gr.mesh('sphere', 'rightEar')
rightEar:scale(earRatio[1], earRatio[2], earRatio[3])
rightEar:set_material(skinColor)
rightEarJoint:add_child(rightEar)

-- left leg
leftThighNode = gr.node('leftThighNode')
torso:add_child(leftThighNode)

leftThighNode:scale(1.0/bodyRatio[1], 1.0/bodyRatio[2], 1.0/bodyRatio[3])
leftThighNode:translate(0.0, -0.3, 0.0)

leftThighJoint = gr.joint('leftThighJoint', {-45, 0, 10}, {0, 0, 0});
leftThighNode:add_child(leftThighJoint)

legRatio = {0.1, 0.2, 0.1}
leftThigh = gr.mesh('sphere', 'leftThigh')
leftThigh:scale(legRatio[1], legRatio[2], legRatio[3])
leftThigh:translate(-0.5, -0.6, 0.0)
leftThigh:set_material(skinColor)

leftThighJoint:add_child(leftThigh)

leftCalfNode = gr.node('leftCalfNode')
leftThigh:add_child(leftCalfNode)
leftCalfNode:scale(1.0/legRatio[1], 1.0/legRatio[2], 1.0/legRatio[3])
leftCalfNode:translate(0.0, -0.4, 0.0)

leftCalfJoint = gr.joint('leftCalfJoint', {-10, 0, 60}, {0, 0, 0});
leftCalfNode:add_child(leftCalfJoint)

leftCalf = gr.mesh('sphere', 'leftCalf')
leftCalf:scale(legRatio[1], legRatio[2], legRatio[3])
leftCalf:translate(0.0, -0.3, 0.0)
leftCalf:set_material(skinColor)

leftCalfJoint:add_child(leftCalf)

leftFootNode = gr.node('leftFootNode')
leftCalf:add_child(leftFootNode)
leftFootNode:scale(1.0/legRatio[1], 1.0/legRatio[2], 1.0/legRatio[3])
leftFootNode:translate(0.0, -0.3, 0.0)

leftFootJoint = gr.joint('leftFootJoint', {-45, 0, 15}, {0, 0, 0});
leftFootNode:add_child(leftFootJoint)

leftFoot = gr.mesh('sphere', 'leftFoot')
leftFoot:rotate('z', 90.0)
leftFoot:scale(legRatio[1], legRatio[2], legRatio[3])
leftFoot:translate(0.0, -0.3, 0.0)
leftFoot:set_material(footColor)

leftFootJoint:add_child(leftFoot)

-- right leg
rightThighNode = gr.node('rightThighNode')
torso:add_child(rightThighNode)

rightThighNode:scale(1.0/bodyRatio[1], 1.0/bodyRatio[2], 1.0/bodyRatio[3])
rightThighNode:translate(0.0, -0.3, 0.0)

rightThighJoint = gr.joint('rightThighJoint', {-45, 0, 10}, {0, 0, 0});
rightThighNode:add_child(rightThighJoint)

rightThigh = gr.mesh('sphere', 'rightThigh')
rightThigh:scale(legRatio[1], legRatio[2], legRatio[3])
rightThigh:translate(0.5, -0.6, 0.0)
rightThigh:set_material(skinColor)

rightThighJoint:add_child(rightThigh)

rightCalfNode = gr.node('rightCalfNode')
rightThigh:add_child(rightCalfNode)
rightCalfNode:scale(1.0/legRatio[1], 1.0/legRatio[2], 1.0/legRatio[3])
rightCalfNode:translate(0.0, -0.4, 0.0)

rightCalfJoint = gr.joint('rightCalfJoint', {-10, 0, 60}, {0, 0, 0});
rightCalfNode:add_child(rightCalfJoint)

rightCalf = gr.mesh('sphere', 'rightCalf')
rightCalf:scale(legRatio[1], legRatio[2], legRatio[3])
rightCalf:translate(0.0, -0.3, 0.0)
rightCalf:set_material(skinColor)

rightCalfJoint:add_child(rightCalf)

rightFootNode = gr.node('rightFootNode')
rightCalf:add_child(rightFootNode)
rightFootNode:scale(1.0/legRatio[1], 1.0/legRatio[2], 1.0/legRatio[3])
rightFootNode:translate(0.0, -0.3, 0.0)

rightFootJoint = gr.joint('rightFootJoint', {-45, 0, 15}, {0, 0, 0});
rightFootNode:add_child(rightFootJoint)

rightFoot = gr.mesh('sphere', 'rightFoot')
rightFoot:rotate('z', 90.0)
rightFoot:scale(legRatio[1], legRatio[2], legRatio[3])
rightFoot:translate(0.0, -0.3, 0.0)
rightFoot:set_material(footColor)

rightFootJoint:add_child(rightFoot)

-- left arm
leftUpperArmNode = gr.node('leftUpperArmNode')
torso:add_child(leftUpperArmNode)
leftUpperArmNode:scale(1.0/bodyRatio[1], 1.0/bodyRatio[2], 1.0/bodyRatio[3])
leftUpperArmNode:translate(-0.6, 0.0, 0.0)

leftUpperArmJoint = gr.joint('leftUpperArmJoint', {-65, 0, 65}, {0, 0, 0});
leftUpperArmNode:add_child(leftUpperArmJoint)

armRatio = {0.1, 0.1, 0.1}
leftUpperArm = gr.mesh('sphere', 'leftUpperArm')
leftUpperArm:scale(armRatio[1], armRatio[2], armRatio[3])
leftUpperArm:rotate('z', -45.0)
leftUpperArm:translate(-0.45, 0.0, 0.0)
leftUpperArm:set_material(skinColor)
leftUpperArmJoint:add_child(leftUpperArm)

leftForeArmNode = gr.node('leftForeArmNode')
leftUpperArm:add_child(leftForeArmNode)
leftForeArmNode:scale(1.0/armRatio[1], 1.0/armRatio[2], 1.0/armRatio[3])
leftForeArmNode:translate(0.0, -0.1, 0.0)

leftForeArmJoint = gr.joint('leftForeArmJoint', {-75, 0, 5}, {0, 0, 0});
leftForeArmNode:add_child(leftForeArmJoint)

leftForeArm = gr.mesh('sphere', 'leftForeArm')
leftForeArm:scale(armRatio[1], armRatio[2], armRatio[3])
leftForeArm:translate(0.0, -0.16, 0.0)
leftForeArm:set_material(skinColor)
leftForeArmJoint:add_child(leftForeArm)

leftHandNode = gr.node('leftHandNode')
leftForeArm:add_child(leftHandNode)
leftHandNode:scale(1.0/armRatio[1], 1.0/armRatio[2], 1.0/armRatio[3])
leftHandNode:translate(0.0, -0.1, 0.0)

leftHandJoint = gr.joint('leftHandJoint', {-30, 0, 30}, {0, 0, 0});
leftHandNode:add_child(leftHandJoint)

leftHand = gr.mesh('sphere', 'leftHand')
leftHand:scale(armRatio[1], armRatio[2], armRatio[3])
leftHand:translate(0.0, -0.16, 0.0)
leftHand:set_material(handColor)
leftHandJoint:add_child(leftHand)

-- right arm
rightUpperArmNode = gr.node('rightUpperArmNode')
torso:add_child(rightUpperArmNode)
rightUpperArmNode:scale(1.0/bodyRatio[1], 1.0/bodyRatio[2], 1.0/bodyRatio[3])
rightUpperArmNode:translate(0.6, 0.0, 0.0)

rightUpperArmJoint = gr.joint('rightUpperArmJoint', {-60, 0, 60}, {0, 0, 0});
rightUpperArmNode:add_child(rightUpperArmJoint)

armRatio = {0.1, 0.1, 0.1}
rightUpperArm = gr.mesh('sphere', 'rightUpperArm')
rightUpperArm:scale(armRatio[1], armRatio[2], armRatio[3])
rightUpperArm:rotate('z', 45.0)
rightUpperArm:translate(0.45, 0.0, 0.0)
rightUpperArm:set_material(skinColor)
rightUpperArmJoint:add_child(rightUpperArm)

rightForeArmNode = gr.node('rightForeArmNode')
rightUpperArm:add_child(rightForeArmNode)
rightForeArmNode:scale(1.0/armRatio[1], 1.0/armRatio[2], 1.0/armRatio[3])
rightForeArmNode:translate(0.0, -0.1, 0.0)

rightForeArmJoint = gr.joint('rightForeArmJoint', {-75, 0, 5}, {0, 0, 0});
rightForeArmNode:add_child(rightForeArmJoint)

rightForeArm = gr.mesh('sphere', 'rightForeArm')
rightForeArm:scale(armRatio[1], armRatio[2], armRatio[3])
rightForeArm:translate(0.0, -0.16, 0.0)
rightForeArm:set_material(skinColor)
rightForeArmJoint:add_child(rightForeArm)

rightHandNode = gr.node('rightHandNode')
rightForeArm:add_child(rightHandNode)
rightHandNode:scale(1.0/armRatio[1], 1.0/armRatio[2], 1.0/armRatio[3])
rightHandNode:translate(0.0, -0.1, 0.0)

rightHandJoint = gr.joint('rightHandJoint', {-30, 0, 30}, {0, 0, 0});
rightHandNode:add_child(rightHandJoint)

rightHand = gr.mesh('sphere', 'rightHand')
rightHand:scale(armRatio[1], armRatio[2], armRatio[3])
rightHand:translate(0.0, -0.16, 0.0)
rightHand:set_material(handColor)
rightHandJoint:add_child(rightHand)

return rootNode;