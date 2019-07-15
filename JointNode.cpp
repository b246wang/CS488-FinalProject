// Winter 2019

#include "JointNode.hpp"
#include "cs488-framework/MathUtils.hpp"
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

using namespace glm;

//---------------------------------------------------------------------------------------
JointNode::JointNode(const std::string& name)
	: SceneNode(name)
{
	m_nodeType = NodeType::JointNode;
}

//---------------------------------------------------------------------------------------
JointNode::~JointNode() {

}
 //---------------------------------------------------------------------------------------
void JointNode::set_joint_x(double min, double init, double max) {
	m_joint_x.min = min;
	m_joint_x.init = init;
	m_joint_x.curr = init;
	m_joint_x.max = max;
}

//---------------------------------------------------------------------------------------
void JointNode::set_joint_y(double min, double init, double max) {
	m_joint_y.min = min;
	m_joint_y.init = init;
	m_joint_y.curr = init;
	m_joint_y.max = max;
}

void JointNode::rotate(char axis, float angle) {
	float next_angle;
	mat4 rot_matrix;
	switch (axis) {
		case 'x':
			next_angle = glm::clamp(m_joint_x.curr + angle, m_joint_x.min, m_joint_x.max);
			rot_matrix = glm::rotate(degreesToRadians(float(next_angle - m_joint_x.curr)), vec3(1,0,0));
			m_joint_x.curr = next_angle;
			trans = rot_matrix * trans;
			break;
		case 'y':
			next_angle = glm::clamp(m_joint_y.curr + angle, m_joint_y.min, m_joint_y.max);
			rot_matrix = glm::rotate(degreesToRadians(float(next_angle - m_joint_y.curr)), vec3(0,1,0));
			m_joint_y.curr = next_angle;
			trans = rot_matrix * trans;
	        break;
		default:
			break;
	}
}
