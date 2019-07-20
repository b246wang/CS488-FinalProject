// Spring 2019

#pragma once

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"
#include "cs488-framework/MeshConsolidator.hpp"

#include "SceneNode.hpp"
#include "JointNode.hpp"
#include "Player.hpp"
#include "Animation.hpp"
#include "Keyframe.hpp"

#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <set>

struct LightSource {
	glm::vec3 position;
	glm::vec3 rgbIntensity;
};

struct JointHistory {
	JointNode * n;
	double x;
	double y;
	glm::mat4 t;

	JointHistory(JointNode * node) {
		n = node;
		x = node->m_joint_x.curr;
		y = node->m_joint_y.curr;
		t = node->trans;
	}
};

struct Obstacle {
	float x;
	float y;
	bool destroyed;

	Obstacle(float pos_x, float pos_y) {
		x = pos_x;
		y = pos_y;
		destroyed = false;
	}
};

class A3 : public CS488Window {
public:
	A3(const std::string & luaSceneFile);
	virtual ~A3();

protected:
	virtual void init() override;
	virtual void appLogic() override;
	virtual void guiLogic() override;
	virtual void draw() override;
	virtual void cleanup() override;

	//-- Virtual callback methods
	virtual bool cursorEnterWindowEvent(int entered) override;
	virtual bool mouseMoveEvent(double xPos, double yPos) override;
	virtual bool mouseButtonInputEvent(int button, int actions, int mods) override;
	virtual bool mouseScrollEvent(double xOffSet, double yOffSet) override;
	virtual bool windowResizeEvent(int width, int height) override;
	virtual bool keyInputEvent(int key, int action, int mods) override;

	//-- One time initialization methods:
	void processLuaSceneFile(const std::string & filename);
	void createShaderProgram();
	void enableVertexShaderInputSlots();
	void uploadVertexDataToVbos(const MeshConsolidator & meshConsolidator);
	void mapVboDataToVertexShaderInputLocations();
	void initViewMatrix();
	void initLightSources();

	void initPerspectiveMatrix();
	void initVar();
	void initGrid();
	void initFloor();
	void initObstacles();
	void initShadows();
	GLuint createTexture(std::string filename);
	void resetPosition();
	void resetRotation();
	void resetJoints();
	void resetAll();
	void uploadCommonSceneUniforms();
	void renderSceneGraph(const SceneNode &node);
	void renderArcCircle();
	void renderGrid();
	void renderFloor();
	void renderObstacles();
	void renderShadows();
	void drawNodes(const SceneNode *node, glm::mat4 t);
	void pickNode();
	JointNode* bfsJoint(SceneNode * root, unsigned int id);
	JointNode* bfsJoint(SceneNode * root, std::string name);
	void pushJointStack();
	void undo();
	void redo();

	glm::mat4 m_perpsective;
	glm::mat4 m_view;

	LightSource m_light;

	GLuint m_grid_vao;
	GLuint m_grid_vbo;
	GLuint m_floor_vao;
	GLuint m_floor_vbo;
	GLuint m_floor_uv_vbo;
	GLuint m_cube_vao;
	GLuint m_cube_vbo;
	GLuint m_cube_uv_vbo;
	GLuint floor_texture;
	GLuint obstacle_texture;
	std::vector<Obstacle> obstacles;
	ShaderProgram m_tex_shader;

	// shadow mapping
	glm::mat4 lightSpaceMatrix;
	GLuint shadowFrameBuffer;
	GLuint shadowTexture;
	ShaderProgram shadow_shader;
	ShaderProgram depth_shader;

	// player1
	Player player1;
	bool keyLeftActive;
	bool keyRightActive;
	bool keyUpActive;
	bool keyDownActive;
	bool checkCollision(Player &p);
	// void collide(Player &p, char dir);

	//-- GL resources for mesh geometry data:
	GLuint m_vao_meshData;
	GLuint m_vbo_vertexPositions;
	GLuint m_vbo_vertexNormals;
	GLint m_positionAttribLocation;
	GLint m_normalAttribLocation;
	ShaderProgram m_shader;

	//-- GL resources for trackball circle geometry:
	GLuint m_vbo_arcCircle;
	GLuint m_vao_arcCircle;
	GLint m_arc_positionAttribLocation;
	ShaderProgram m_shader_arcCircle;

	// BatchInfoMap is an associative container that maps a unique MeshId to a BatchInfo
	// object. Each BatchInfo object contains an index offset and the number of indices
	// required to render the mesh with identifier MeshId.
	BatchInfoMap m_batchInfoMap;

	std::string m_luaSceneFile;

	std::shared_ptr<SceneNode> m_rootNode;

	bool do_picking;
	bool mouseLeftActive;
	bool mouseMiddleActive;
	bool mouseRightActive;
	double prev_x;
	double prev_y;
	bool showCircle;
	// 0: Position/Orientation, 1: Joints
	int current_mode;
	std::set<JointNode*> selectedJointNodes;

	// Transformation matrices
	glm::mat4 m_trans;
	glm::mat4 m_rot;
	std::string message;
	std::vector<std::vector<JointHistory>> jointStack;
	int stack_idx;

};
