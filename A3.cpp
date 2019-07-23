// Spring 2019

#include "A3.hpp"
#include "scene_lua.hpp"
#include "lodepng/lodepng.h"
using namespace std;

#include "cs488-framework/GlErrorCheck.hpp"
#include "cs488-framework/MathUtils.hpp"
#include "GeometryNode.hpp"
#include "JointNode.hpp"

#include <iostream>
#include <string.h>
#include <cmath>
#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <queue>

using namespace glm;
using namespace irrklang;

static const size_t DIM = 10;
static const float cube_h = 1.0f;
static const float floor_height = 0.01f;
static const float collision_square = 0.92f;
static const float water_collision_square = 0.51f;
static const int balloon_lifetime = 150;
static const int water_lifetime = 30;
static const float water_speed = 0.5f;
static const vec2 cubeUVs[] = {
	vec2(0.0f, 0.0f), vec2(1.0f, 0.0f), vec2(0.0f, 1.0f),
	vec2(0.0f, 1.0f), vec2(1.0f, 0.0f), vec2(1.0f, 1.0f),
	vec2(0.0f, 0.0f), vec2(1.0f, 0.0f), vec2(0.0f, 1.0f),
	vec2(0.0f, 1.0f), vec2(1.0f, 0.0f), vec2(1.0f, 1.0f),
	vec2(0.0f, 0.0f), vec2(1.0f, 0.0f), vec2(0.0f, 1.0f),
	vec2(0.0f, 1.0f), vec2(1.0f, 0.0f), vec2(1.0f, 1.0f),
	vec2(0.0f, 0.0f), vec2(1.0f, 0.0f), vec2(0.0f, 1.0f),
	vec2(0.0f, 1.0f), vec2(1.0f, 0.0f), vec2(1.0f, 1.0f),
	vec2(0.0f, 0.0f), vec2(1.0f, 0.0f), vec2(0.0f, 1.0f),
	vec2(0.0f, 1.0f), vec2(1.0f, 0.0f), vec2(1.0f, 1.0f),
	vec2(0.0f, 0.0f), vec2(1.0f, 0.0f), vec2(0.0f, 1.0f),
	vec2(0.0f, 1.0f), vec2(1.0f, 0.0f), vec2(1.0f, 1.0f)
};
static const vec2 meshCubeUVs[] = {
	vec2(1.0f, 1.0f), vec2(0.0f, 1.0f), vec2(0.0f, 0.0f),
	vec2(0.0f, 1.0f), vec2(1.0f, 1.0f), vec2(1.0f, 0.0f),
	vec2(1.0f, 1.0f), vec2(0.0f, 1.0f), vec2(0.0f, 0.0f),
	vec2(1.0f, 1.0f), vec2(0.0f, 1.0f), vec2(1.0f, 0.0f),
	vec2(1.0f, 1.0f), vec2(0.0f, 1.0f), vec2(0.0f, 0.0f),
	vec2(0.0f, 1.0f), vec2(1.0f, 1.0f), vec2(1.0f, 0.0f),
	vec2(1.0f, 1.0f), vec2(0.0f, 1.0f), vec2(0.0f, 0.0f),
	vec2(0.0f, 1.0f), vec2(1.0f, 1.0f), vec2(1.0f, 0.0f),
	vec2(1.0f, 1.0f), vec2(0.0f, 1.0f), vec2(0.0f, 0.0f),
	vec2(0.0f, 1.0f), vec2(1.0f, 0.0f), vec2(1.0f, 1.0f),
	vec2(1.0f, 1.0f), vec2(0.0f, 1.0f), vec2(0.0f, 0.0f),
	vec2(1.0f, 1.0f), vec2(0.0f, 1.0f), vec2(1.0f, 0.0f),
};
static const string balloonFileName = "Assets/balloon.lua";
static const string waterFileName = "Assets/water.lua";
static const string blockFileName = "Assets/block.lua";
static const string p2FileName = "Assets/player2.lua";
static bool show_gui = true;
static const float TranslateFactor = 200.0f;
static const float JointRotateFactor = 15.0f;
const size_t CIRCLE_PTS = 48;

//----------------------------------------------------------------------------------------
// Constructor
A3::A3(const std::string & luaSceneFile)
	: m_luaSceneFile(luaSceneFile),
	  m_positionAttribLocation(0),
	  m_normalAttribLocation(0),
	  m_vao_meshData(0),
	  m_vbo_vertexPositions(0),
	  m_vbo_vertexNormals(0),
	  m_vao_arcCircle(0),
	  m_vbo_arcCircle(0),
	  m_floor_vao(0),
	  m_floor_vbo(0),
	  m_bg_vao(0),
	  m_bg_vbo(0),
	  player1(0.0f, 0.0f),
	  player2(9.0f, 9.0f),
	  engine(NULL),
	  showTexture(true),
	  showAnimation(true),
	  showCollision(true),
	  waterTransparency(0.6)
{

}

//----------------------------------------------------------------------------------------
// Destructor
A3::~A3() {}

void A3::resetPosition() {
	m_trans = mat4(1.0f);
}

void A3::resetRotation() {
	m_rot = mat4(1.0f);
}

void A3::resetJoints() {
	queue<SceneNode *> q;
	jointStack.resize(1);
	message = "";
	stack_idx = 0;
	q.push(m_rootNode.get());

	while(!q.empty()) {
		SceneNode * n = q.front();
		q.pop();

		if (n->m_nodeType == NodeType::JointNode) {
			JointNode * j = static_cast<JointNode *>(n);
			j->set_transform(mat4(1.0f));
			j->m_joint_x.curr = j->m_joint_x.init;
			j->m_joint_y.curr = j->m_joint_y.init;
		}

		for (SceneNode * child : n->children) {
			q.push(child);
		}
	}
}

void A3::resetAll() {
	resetPosition();
	resetRotation();
	resetJoints();
}

void A3::initVar()
{
	// player1
	SceneNode * p1_rootNode = m_rootNode.get();
	player1.setRootNode(p1_rootNode);
	player1.setJoints(
		bfsJoint(p1_rootNode, "neckJoint"), 
		bfsJoint(p1_rootNode, "leftThighJoint"), 
		bfsJoint(p1_rootNode, "rightThighJoint"));
	keyLeftActive = false;
	keyRightActive = false;
	keyUpActive = false;
	keyDownActive = false;

	// player2
	SceneNode * p2_rootNode = m_p2Node.get();
	player2.setRootNode(p2_rootNode);
	player2.setJoints(
		bfsJoint(p2_rootNode, "neckJoint"),
		bfsJoint(p2_rootNode, "leftThighJoint"),
		bfsJoint(p2_rootNode, "rightThighJoint"));
	keyAActive = false;
	keySActive = false;
	keyDActive = false;
	keyWActive = false;

	do_picking = false;
	mouseLeftActive = false;
	mouseMiddleActive = false;
	mouseRightActive = false;
	showCircle = true;
	stack_idx = 0;
	vector<JointHistory> dummy;
	jointStack.push_back(dummy);
	message = "";
	resetPosition();
	resetRotation();
}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A3::init()
{
	if (engine == NULL) engine = createIrrKlangDevice();
	if (!engine) cout << "error starting sound engine...." << endl;

	// Set the background colour.
	glClearColor(0.85, 0.85, 0.85, 1.0);

	createShaderProgram();

	glGenVertexArrays(1, &m_vao_arcCircle);
	glGenVertexArrays(1, &m_vao_meshData);
	enableVertexShaderInputSlots();

	processLuaSceneFile(m_luaSceneFile);

	// Load and decode all .obj files at once here.  You may add additional .obj files to
	// this list in order to support rendering additional mesh types.  All vertex
	// positions, and normals will be extracted and stored within the MeshConsolidator
	// class.
	unique_ptr<MeshConsolidator> meshConsolidator (new MeshConsolidator{
			getAssetFilePath("cube.obj"),
			getAssetFilePath("sphere.obj"),
			getAssetFilePath("suzanne.obj")
	});


	// Acquire the BatchInfoMap from the MeshConsolidator.
	meshConsolidator->getBatchInfoMap(m_batchInfoMap);

	// Take all vertex data within the MeshConsolidator and upload it to VBOs on the GPU.
	uploadVertexDataToVbos(*meshConsolidator);

	mapVboDataToVertexShaderInputLocations();

	initVar();
	initFloor();
	initBackground();
	initObstacles();
	initBlocks();

	bg_texture = createTexture(getAssetFilePath("xmas_board.png"));
	floor_texture = createTexture(getAssetFilePath("wood_floor.png"));
	obstacle_texture = createTexture(getAssetFilePath("tree_cube.png"));
	water_texture = createTexture(getAssetFilePath("water_tex.png"));
	wood_cube_texture = createTexture(getAssetFilePath("wood_cube.png"));
	xmas_cube_texture = createTexture(getAssetFilePath("chrix_cube.png"));
	snow_cube_texture = createTexture(getAssetFilePath("snow_cube.png"));
	gold_cube_texture = createTexture(getAssetFilePath("gold_cube.png"));

	initPerspectiveMatrix();

	initViewMatrix();

	initLightSources();

	CHECK_GL_ERRORS;
	// initShadows();

	// Exiting the current scope calls delete automatically on meshConsolidator freeing
	// all vertex data resources.  This is fine since we already copied this data to
	// VBOs on the GPU.  We have no use for storing vertex data on the CPU side beyond
	// this point.
}

void A3::initFloor()
{
	vec3 floorVertices[] = {
		vec3(0, floor_height, 0), vec3(DIM, floor_height, 0), vec3(0, floor_height, DIM),
		vec3(0, floor_height, DIM), vec3(DIM, floor_height, 0), vec3(DIM, floor_height, DIM)
	};

	vec2 floorUVVertices[] = {
		vec2(0.0f, 0.0f), vec2(1.0f, 0.0f), vec2(0.0f, 1.0f),
		vec2(0.0f, 1.0f), vec2(1.0f, 0.0f), vec2(1.0f, 1.0f)
	};

	// Create the vertex array to record buffer assignments.
	glGenVertexArrays( 1, &m_floor_vao );
	glBindVertexArray( m_floor_vao );

	// Create the floor vertex buffer
	glGenBuffers( 1, &m_floor_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, m_floor_vbo );
	glBufferData( GL_ARRAY_BUFFER, sizeof(floorVertices),
		floorVertices, GL_STATIC_DRAW );

	// Specify the means of extracting the position values properly.
	GLint posAttrib = m_tex_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray( posAttrib );
	glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

	// Create the floor uv coord buffer
	glGenBuffers( 1, &m_floor_uv_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, m_floor_uv_vbo );
	glBufferData( GL_ARRAY_BUFFER, sizeof(floorUVVertices),
		floorUVVertices, GL_STATIC_DRAW );

	// Specify the means of extracting the position values properly.
	GLint UVAttrib = m_tex_shader.getAttribLocation( "vertexUV" );
	glEnableVertexAttribArray( UVAttrib );
	glVertexAttribPointer( UVAttrib, 2, GL_FLOAT, GL_FALSE, 0, nullptr );

	// Reset state to prevent rogue code from messing with *my* 
	// stuff!
	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	CHECK_GL_ERRORS;
}

void A3::initBackground()
{
	float height = -0.5f;
	float height_bottom = -9.0f;
	float left = -8.5f;
	float right = 18.5f;
	float top = -6.5f;
	float bottom = 11.5f;
	vec3 bgVertices[] = {
		vec3(left, height, top), vec3(right, height, top), vec3(left, height_bottom, bottom),
		vec3(left, height_bottom, bottom), vec3(right, height, top), vec3(right, height_bottom, bottom)
	};

	vec2 bgUVVertices[] = {
		vec2(0.0f, 0.0f), vec2(1.0f, 0.0f), vec2(0.0f, 1.0f),
		vec2(0.0f, 1.0f), vec2(1.0f, 0.0f), vec2(1.0f, 1.0f)
	};

	// Create the vertex array to record buffer assignments.
	glGenVertexArrays( 1, &m_bg_vao );
	glBindVertexArray( m_bg_vao );

	// Create the floor vertex buffer
	glGenBuffers( 1, &m_bg_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, m_bg_vbo );
	glBufferData( GL_ARRAY_BUFFER, sizeof(bgVertices),
		bgVertices, GL_STATIC_DRAW );

	// Specify the means of extracting the position values properly.
	GLint posAttrib = m_tex_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray( posAttrib );
	glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

	// Create the bg uv coord buffer
	glGenBuffers( 1, &m_bg_uv_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, m_bg_uv_vbo );
	glBufferData( GL_ARRAY_BUFFER, sizeof(bgUVVertices),
		bgUVVertices, GL_STATIC_DRAW );

	// Specify the means of extracting the position values properly.
	GLint UVAttrib = m_tex_shader.getAttribLocation( "vertexUV" );
	glEnableVertexAttribArray( UVAttrib );
	glVertexAttribPointer( UVAttrib, 2, GL_FLOAT, GL_FALSE, 0, nullptr );

	// Reset state to prevent rogue code from messing with *my* 
	// stuff!
	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	CHECK_GL_ERRORS;
}

void A3::initBlocks() {
	for (int i = 1; i < 9; i++) {
		for (int j = 1; j < 9; j++) {
			if (i == j || i + j == 9) {
				if (i == 1 || i == 8) {
					blocks.push_back(Block(float(i), float(j), 1));
				} else if (i == 2 || i == 7) {
					blocks.push_back(Block(float(i), float(j), 3));
				} else if (i == 3 || i == 6) {
					blocks.push_back(Block(float(i), float(j), 2));
				} else if (i == 4 || i == 5) {
					blocks.push_back(Block(float(i), float(j), 2));
				}
			} else if (i == 4 && j == 1) {
					blocks.push_back(Block(float(i), float(j), 2));
			} else if (i == 5 && j == 1) {
					blocks.push_back(Block(float(i), float(j), 1));
			} else if (i == 4 && j == 8) {
					blocks.push_back(Block(float(i), float(j), 1));
			} else if (i == 5 && j == 8) {
					blocks.push_back(Block(float(i), float(j), 2));
			} else if (i == 1 && j == 4) {
					blocks.push_back(Block(float(i), float(j), 3));
			} else if (i == 1 && j == 5) {
					blocks.push_back(Block(float(i), float(j), 1));
			} else if (i == 8 && j == 4) {
					blocks.push_back(Block(float(i), float(j), 1));
			} else if (i == 8 && j == 5) {
					blocks.push_back(Block(float(i), float(j), 3));
			} else {
				blocks.push_back(Block(float(i), float(j)));
			}
		}
	}
}

void A3::initObstacles() {
	for (int i = -1; i < 11; i++) {
		obstacles.push_back(Obstacle(float(i), -1.0f));
	}
	for (int j = 0; j < 11; j++) {
		obstacles.push_back(Obstacle(-1.0f, float(j)));
		obstacles.push_back(Obstacle(10.0f, float(j)));
	}
	for (int k = 0; k < 10; k++) {
		obstacles.push_back(Obstacle(float(k), 10.0f));
	}

	vector<vec3> cubeVertices;
	vector<vec2> cubeUVVertices;
	for (Obstacle &obstacle : obstacles) {
		float x = obstacle.x;
		float y = obstacle.y;
		vec3 cube[] = {
			// top
			vec3(x, cube_h, y), vec3(x + cube_h, cube_h, y), vec3(x, cube_h, y + cube_h),
			vec3(x, cube_h, y + cube_h), vec3(x + cube_h, cube_h, y), vec3(x + cube_h, cube_h, y + cube_h),
			// back
			vec3(x, 0, y), vec3(x + cube_h, 0, y), vec3(x, 0, y + cube_h),
			vec3(x, 0, y + cube_h), vec3(x + cube_h, 0, y), vec3(x + cube_h, 0, y + cube_h),
			// left
			vec3(x, cube_h, y), vec3(x, 0, y), vec3(x, 0, y + cube_h),
			vec3(x, 0, y + cube_h), vec3(x, cube_h, y), vec3(x, cube_h, y + cube_h),
			// right
			vec3(x + cube_h, cube_h, y), vec3(x + cube_h, 0, y), vec3(x + cube_h, 0, y + cube_h),
			vec3(x + cube_h, 0, y + cube_h), vec3(x + cube_h, cube_h, y), vec3(x + cube_h, cube_h, y + cube_h),
			//front
			vec3(x, cube_h, y + cube_h), vec3(x + cube_h, cube_h, y + cube_h), vec3(x, 0, y + cube_h),
			vec3(x, 0, y + cube_h), vec3(x + cube_h, cube_h, y + cube_h), vec3(x + cube_h, 0, y + cube_h),
			// bottom
			vec3(x, cube_h, y), vec3(x, 0, y), vec3(x + cube_h, 0, y),
			vec3(x + cube_h, 0, y), vec3(x, cube_h, y), vec3(x + cube_h, cube_h, y),
		};
		cubeVertices.insert(cubeVertices.end(), begin(cube), end(cube));
		cubeUVVertices.insert(cubeUVVertices.end(), begin(cubeUVs), end(cubeUVs));
	}

	// Create the vertex array to record buffer assignments.
	glGenVertexArrays( 1, &m_cube_vao );
	glBindVertexArray( m_cube_vao );

	// Create the floor vertex buffer
	glGenBuffers( 1, &m_cube_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, m_cube_vbo );
	glBufferData( GL_ARRAY_BUFFER, sizeof(vec3) * cubeVertices.size(),
		cubeVertices.data(), GL_STATIC_DRAW );

	// Specify the means of extracting the position values properly.
	GLint posAttrib = m_tex_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray( posAttrib );
	glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

	// Create the floor uv coord buffer
	glGenBuffers( 1, &m_cube_uv_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, m_cube_uv_vbo );
	glBufferData( GL_ARRAY_BUFFER, sizeof(vec2) * cubeUVVertices.size(),
		cubeUVVertices.data(), GL_STATIC_DRAW );

	// Specify the means of extracting the position values properly.
	GLint UVAttrib = m_tex_shader.getAttribLocation( "vertexUV" );
	glEnableVertexAttribArray( UVAttrib );
	glVertexAttribPointer( UVAttrib, 2, GL_FLOAT, GL_FALSE, 0, nullptr );

	// Reset state to prevent rogue code from messing with *my* 
	// stuff!
	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	CHECK_GL_ERRORS;
}

GLuint A3::createTexture(string filename) {
	GLuint currTexture;
	std::vector<unsigned char> image;
  	unsigned width, height;
  	unsigned error = lodepng::decode(image, width, height, filename.c_str());
	
	// If there's an error, display it.
	if(error != 0) {
		cout << "error " << error << ": " << lodepng_error_text(error) << endl;
	}

	glGenTextures(1, &currTexture);
	glBindTexture(GL_TEXTURE_2D, currTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return currTexture;
}

//----------------------------------------------------------------------------------------
void A3::processLuaSceneFile(const std::string & filename) {
	// This version of the code treats the Lua file as an Asset,
	// so that you'd launch the program with just the filename
	// of a puppet in the Assets/ directory.
	// std::string assetFilePath = getAssetFilePath(filename.c_str());
	// m_rootNode = std::shared_ptr<SceneNode>(import_lua(assetFilePath));

	// This version of the code treats the main program argument
	// as a straightforward pathname.
	m_rootNode = std::shared_ptr<SceneNode>(import_lua(filename));
	if (!m_rootNode) {
		std::cerr << "Could Not Open " << filename << std::endl;
	}
	m_p2Node = std::shared_ptr<SceneNode>(import_lua(p2FileName));
	if (!m_p2Node) {
		std::cerr << "Could Not Open " << p2FileName << std::endl;
	}

	m_balloonNode = std::shared_ptr<SceneNode>(import_lua(balloonFileName));
	if (!m_balloonNode) {
		std::cerr << "Could Not Open " << balloonFileName << std::endl;
	}

	m_waterNode = std::shared_ptr<SceneNode>(import_lua(waterFileName));
	if (!m_waterNode) {
		std::cerr << "Could Not Open " << waterFileName << std::endl;
	}

	m_blockNode = std::shared_ptr<SceneNode>(import_lua(blockFileName));
	if (!m_blockNode) {
		std::cerr << "Could Not Open " << blockFileName << std::endl;
	}
}

//----------------------------------------------------------------------------------------
void A3::createShaderProgram()
{
	m_tex_shader.generateProgramObject();
	m_tex_shader.attachVertexShader( getAssetFilePath("tex_VertexShader.vs").c_str() );
	m_tex_shader.attachFragmentShader( getAssetFilePath("tex_FragmentShader.fs").c_str() );
	m_tex_shader.link();

	m_shader.generateProgramObject();
	m_shader.attachVertexShader( getAssetFilePath("VertexShader.vs").c_str() );
	m_shader.attachFragmentShader( getAssetFilePath("FragmentShader.fs").c_str() );
	m_shader.link();

	m_shader_arcCircle.generateProgramObject();
	m_shader_arcCircle.attachVertexShader( getAssetFilePath("arc_VertexShader.vs").c_str() );
	m_shader_arcCircle.attachFragmentShader( getAssetFilePath("arc_FragmentShader.fs").c_str() );
	m_shader_arcCircle.link();

	// depth_shader.generateProgramObject();
	// depth_shader.attachVertexShader( getAssetFilePath("depth_VertexShader.vs").c_str() );
	// depth_shader.attachFragmentShader( getAssetFilePath("depth_FragmentShader.fs").c_str() );
	// depth_shader.link();

	// shadow_shader.generateProgramObject();
	// shadow_shader.attachVertexShader( getAssetFilePath("shadow_VertexShader.vs").c_str() );
	// shadow_shader.attachFragmentShader( getAssetFilePath("shadow_FragmentShader.fs").c_str() );
	// shadow_shader.link();
}

//----------------------------------------------------------------------------------------
void A3::enableVertexShaderInputSlots()
{
	//-- Enable input slots for m_vao_meshData:
	{
		glBindVertexArray(m_vao_meshData);

		// Enable the vertex shader attribute location for "position" when rendering.
		m_positionAttribLocation = m_shader.getAttribLocation("position");
		glEnableVertexAttribArray(m_positionAttribLocation);

		// Enable the vertex shader attribute location for "normal" when rendering.
		m_normalAttribLocation = m_shader.getAttribLocation("normal");
		glEnableVertexAttribArray(m_normalAttribLocation);

		CHECK_GL_ERRORS;
	}

	//-- Enable input slots for m_vao_arcCircle:
	{
		glBindVertexArray(m_vao_arcCircle);

		// Enable the vertex shader attribute location for "position" when rendering.
		m_arc_positionAttribLocation = m_shader_arcCircle.getAttribLocation("position");
		glEnableVertexAttribArray(m_arc_positionAttribLocation);

		CHECK_GL_ERRORS;
	}

	// Restore defaults
	glBindVertexArray(0);
}

//----------------------------------------------------------------------------------------
void A3::uploadVertexDataToVbos (
		const MeshConsolidator & meshConsolidator
) {
	// Generate VBO to store all vertex position data
	{
		glGenBuffers(1, &m_vbo_vertexPositions);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexPositions);

		glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumVertexPositionBytes(),
				meshConsolidator.getVertexPositionDataPtr(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}

	// Generate VBO to store all vertex normal data
	{
		glGenBuffers(1, &m_vbo_vertexNormals);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexNormals);

		glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumVertexNormalBytes(),
				meshConsolidator.getVertexNormalDataPtr(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}

	// Generate VBO to store the trackball circle.
	{
		glGenBuffers( 1, &m_vbo_arcCircle );
		glBindBuffer( GL_ARRAY_BUFFER, m_vbo_arcCircle );

		float *pts = new float[ 2 * CIRCLE_PTS ];
		for( size_t idx = 0; idx < CIRCLE_PTS; ++idx ) {
			float ang = 2.0 * M_PI * float(idx) / CIRCLE_PTS;
			pts[2*idx] = cos( ang );
			pts[2*idx+1] = sin( ang );
		}

		glBufferData(GL_ARRAY_BUFFER, 2*CIRCLE_PTS*sizeof(float), pts, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
void A3::mapVboDataToVertexShaderInputLocations()
{
	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao_meshData);

	// Tell GL how to map data from the vertex buffer "m_vbo_vertexPositions" into the
	// "position" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexPositions);
	glVertexAttribPointer(m_positionAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	// Tell GL how to map data from the vertex buffer "m_vbo_vertexNormals" into the
	// "normal" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexNormals);
	glVertexAttribPointer(m_normalAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	// bind water vbo
	glGenBuffers( 1, &m_water_uv_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, m_water_uv_vbo );
	glBufferData( GL_ARRAY_BUFFER, sizeof(vec2) * 36,
		meshCubeUVs, GL_STATIC_DRAW );
	GLint UVAttrib = m_shader.getAttribLocation( "vertexUV" );
	glEnableVertexAttribArray( UVAttrib );
	glVertexAttribPointer( UVAttrib, 2, GL_FLOAT, GL_FALSE, 0, nullptr );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;

	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao_arcCircle);

	// Tell GL how to map data from the vertex buffer "m_vbo_arcCircle" into the
	// "position" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_arcCircle);
	glVertexAttribPointer(m_arc_positionAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A3::initPerspectiveMatrix()
{
	float aspect = ((float)m_windowWidth) / m_windowHeight;
	m_perpsective = glm::perspective(degreesToRadians(30.0f), aspect, 1.0f, 1000.0f);
}


//----------------------------------------------------------------------------------------
void A3::initViewMatrix() {
	m_view = glm::lookAt(
		vec3(0.0f, 2.*float(DIM)*2.0*M_SQRT1_2, float(DIM)*2.0*M_SQRT1_2), 
		vec3(0.0f, 0.0f, 0.0f),
		vec3(0.0f, 1.0f, 0.0f)
	);
	m_view = glm::translate(m_view, vec3( -float(DIM)/2.0f, 0, -float(DIM)/2.0f ));
}

//----------------------------------------------------------------------------------------
void A3::initLightSources() {
	// World-space position
	m_light.position = vec3(-2.0f, 4.0f, 0.0f);
	m_light.rgbIntensity = vec3(1.0f); // light
}

void A3::initShadows() {
	glGenFramebuffers(1, &shadowFrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFrameBuffer);

	glGenTextures(1, &shadowTexture);
	glBindTexture(GL_TEXTURE_2D, shadowTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowTexture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	// Always check that our framebuffer is ok
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		cout << "shadow framebuffer is wrong!!" << endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Specify the means of extracting the position values properly.
	// GLint posAttrib = depth_shader.getAttribLocation( "position" );
	// glEnableVertexAttribArray( posAttrib );
	// glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A3::uploadCommonSceneUniforms() {
	m_shader.enable();
	{
		//-- Set Perpsective matrix uniform for the scene:
		GLint location = m_shader.getUniformLocation("Perspective");
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(m_perpsective));
		CHECK_GL_ERRORS;

		// location = m_shader.getUniformLocation("picking");
		// glUniform1i( location, do_picking ? 1 : 0 );

		if( !do_picking ) {
			//-- Set LightSource uniform for the scene:
			{
				location = m_shader.getUniformLocation("light.position");
				glUniform3fv(location, 1, value_ptr(m_light.position));
				location = m_shader.getUniformLocation("light.rgbIntensity");
				glUniform3fv(location, 1, value_ptr(m_light.rgbIntensity));
				CHECK_GL_ERRORS;
			}

			//-- Set background light ambient intensity
			{
				location = m_shader.getUniformLocation("ambientIntensity");
				vec3 ambientIntensity(0.05f);
				glUniform3fv(location, 1, value_ptr(ambientIntensity));
				CHECK_GL_ERRORS;
			}
		}
	}
	m_shader.disable();
}

bool A3::checkCollision(Player &p) {
	if (!showCollision) return false;
	for (Obstacle &obstacle : obstacles) {
		// Collision x-axis
    bool collisionX = p.x + p.dx + collision_square >= obstacle.x &&
        obstacle.x + collision_square >= p.x + p.dx;
    // Collision y-axis
    bool collisionY = p.y + p.dy + collision_square >= obstacle.y &&
        obstacle.y + collision_square >= p.y + p.dy;
    // Collision only if on both axes
    if (collisionX && collisionY) {
    	return true;
    }
	}
	for (Block &block : blocks) {
		// Collision x-axis
    bool collisionX = p.x + p.dx + collision_square >= block.x &&
        block.x + collision_square >= p.x + p.dx;
    // Collision y-axis
    bool collisionY = p.y + p.dy + collision_square >= block.y &&
        block.y + collision_square >= p.y + p.dy;
    // Collision only if on both axes
    if (collisionX && collisionY) {
    	return true;
    }
	}
	return false;
}

void A3::killPlayer(WaterDamage &w, Player &p) {
	// water damage touches p
	bool waterRightX = w.x + w.curr_right_power > p.x + water_collision_square &&
  	p.x + water_collision_square >= w.x;
  bool waterRightY = w.y + water_collision_square >= p.y && 
  	p.y + water_collision_square >= w.y;
  if (waterRightX && waterRightY) {
  	p.setDead();
  	return;
  }
  bool waterLeftX = w.x + water_collision_square >= p.x &&
  	p.x >= w.x + water_speed - w.curr_left_power;
  bool waterLeftY = w.y + water_collision_square >= p.y && 
  	p.y + water_collision_square >= w.y;
  if (waterLeftX && waterLeftY) {
  	p.setDead();
  	return;
  }
  bool waterDownX = w.x + water_collision_square >= p.x &&
  	p.x + water_collision_square >= w.x;
  bool waterDownY = w.y + w.curr_down_power >= p.y + 0.2f && 
  	p.y + water_collision_square >= w.y;
  if (waterDownX && waterDownY) {
  	p.setDead();
  	return;
  }
  bool waterUpX = w.x + water_collision_square >= p.x &&
  	p.x + water_collision_square >= w.x;
  bool waterUpY = w.y + water_collision_square >= p.y && 
  	p.y >= w.y + water_speed - w.curr_up_power;
  if (waterUpX && waterUpY) {
  	p.setDead();
  	return;
  }
}

void A3::popAnotherBalloon(WaterDamage &w, vector<WaterBalloon> &wbs, int position) {
	WaterBalloon p = wbs.at(position);
	if (w.curr_power < w.power) {
		if (p.y == w.y && p.x > w.x && !w.right_blocked) {
			bool waterRightX = w.x + w.curr_power >= p.x &&
		  	p.x + collision_square >= w.x;
		  if (waterRightX) {
		  	w.right_blocked = true;
		  	waterDamages.push_back(WaterDamage(p.x, p.y, water_lifetime, p.power, p.source));
		  	engine->play2D(getAssetFilePath("splat.wav").c_str());
		  	wbs.erase(wbs.begin() + position);
		  }
		  return;
		}
		
		if (p.y == w.y && p.x < w.x && !w.left_blocked) {
		  bool waterLeftX = w.x + collision_square >= p.x &&
		  	p.x + collision_square >= w.x + water_speed - w.curr_power;
		  if (waterLeftX) {
		  	w.left_blocked = true;
		  	waterDamages.push_back(WaterDamage(p.x, p.y, water_lifetime, p.power, p.source));
		  	engine->play2D(getAssetFilePath("splat.wav").c_str());
		  	wbs.erase(wbs.begin() + position);
		  }
		  return;
		}

		if (p.x == w.x && p.y > w.y && !w.down_blocked) {
		  bool waterDownY = w.y + w.curr_power >= p.y && 
		  	p.y + collision_square >= w.y;
		  if (waterDownY) {
		  	w.down_blocked = true;
		  	waterDamages.push_back(WaterDamage(p.x, p.y, water_lifetime, p.power, p.source));
		  	engine->play2D(getAssetFilePath("splat.wav").c_str());
		  	wbs.erase(wbs.begin() + position);
		  }
		  return;
		}

		if (p.x == w.x && p.y < w.y && !w.up_blocked) {
		  bool waterUpY = w.y + collision_square >= p.y && 
		  	p.y + collision_square >= w.y + water_speed - w.curr_power;
		  if (waterUpY) {
		  	w.up_blocked = true;
		  	waterDamages.push_back(WaterDamage(p.x, p.y, water_lifetime, p.power, p.source));
		  	engine->play2D(getAssetFilePath("splat.wav").c_str());
		  	wbs.erase(wbs.begin() + position);
		  }
		  return;
		}
	}
}

void A3::powerPlayer(Block &b, int source) {
	if (b.special == 0) {
		return;
	}
	Player &p = source == 1 ? player1 : player2;
	int s = b.special;
	// 0: no effect, 1: speed up, 2: balloon up, 3: power up
	if (s == 1) {
		p.speedUp();
	} else if (s == 2) {
		p.balloonUp();
	} else if (s == 3) {
		p.powerUp();
	}
}

void A3::waterCollision(WaterDamage &w) {
	// water stops at obstacle
	for (Obstacle &obstacle: obstacles) {
		if (!w.right_blocked && w.y == obstacle.y && obstacle.x > w.x) {
			bool collisionX = w.x + w.curr_right_power >= obstacle.x;
	    if (collisionX) {
	    	w.right_blocked = true;
	    }
		}
		if (!w.left_blocked && w.y == obstacle.y && obstacle.x < w.x) {
			bool collisionX = obstacle.x >= w.x - w.curr_left_power;
	    if (collisionX) {
	    	w.left_blocked = true;
	    }
		}
		if (!w.down_blocked && w.x == obstacle.x && obstacle.y > w.y) {
	    bool collisionY = w.y + w.curr_down_power >= obstacle.y;
	    if (collisionY) {
	    	w.down_blocked = true;
	    }
		}
		if (!w.up_blocked && w.x == obstacle.x && obstacle.y < w.y) {
	    bool collisionY = obstacle.y >= w.y - w.curr_up_power;
	    if (collisionY) {
	    	w.up_blocked = true;
	    }
		}
	}

	// water pops another balloon
	for (int i = 0; i < waterBalloons.size(); i++) {
		popAnotherBalloon(w, waterBalloons, i);
	}
	for (int j = 0; j < p2_waterBalloons.size(); j++) {
		popAnotherBalloon(w, p2_waterBalloons, j);
	}

	for (int i = 0; i < blocks.size(); i++) {
		Block block = blocks.at(i);
		if (!w.right_blocked && w.y == block.y && block.x > w.x) {
			bool collisionX = w.x + w.curr_right_power >= block.x && w.curr_right_power < w.power;
	    if (collisionX) {
	    	w.right_blocked = true;
	    	powerPlayer(block, w.source);
	    	blocks.erase(blocks.begin() + i);
	    }
		}
		if (!w.left_blocked && w.y == block.y && block.x < w.x) {
			bool collisionX = block.x >= w.x - w.curr_left_power && w.curr_left_power < w.power;
	    if (collisionX) {
	    	w.left_blocked = true;
	    	powerPlayer(block, w.source);
	    	blocks.erase(blocks.begin() + i);
	    }
		}
		if (!w.down_blocked && w.x == block.x && block.y > w.y) {
	    bool collisionY = w.y + w.curr_down_power >= block.y && w.curr_down_power < w.power;
	    if (collisionY) {
	    	w.down_blocked = true;
	    	powerPlayer(block, w.source);
	    	blocks.erase(blocks.begin() + i);
	    }
		}
		if (!w.up_blocked && w.x == block.x && block.y < w.y) {
	    bool collisionY = block.y >= w.y - w.curr_up_power && w.curr_up_power < w.power;
	    if (collisionY) {
	    	w.up_blocked = true;
	    	powerPlayer(block, w.source);
	    	blocks.erase(blocks.begin() + i);
	    }
		}
	}

	// check if water damage touches players
	killPlayer(w, player1);
	killPlayer(w, player2);
}

void A3::pushWaterBalloon(vector<WaterBalloon> &wbs, float x, float y, float power, int source) {
	for (WaterBalloon &balloon : wbs) {
		if (balloon.x == x && balloon.y == y) {
			return;
		}
	}
	for (Obstacle &obstacle : obstacles) {
		if (obstacle.x == x && obstacle.y == y) {
			return;
		}
	}
	for (Block &block : blocks) {
		if (block.x == x && block.y == y) {
			return;
		}
	}
	engine->play2D(getAssetFilePath("bounce.wav").c_str());
	wbs.push_back(WaterBalloon(x, y, balloon_lifetime, power, source));
}

// void A3::collide(Player &p, char dir) {
// 	if (dir == 'x') {
// 		if (checkCollision(p)) p.dx = 0.0f;
// 	}
// 	if (dir == 'y') {
// 		if (checkCollision(p)) p.dy = 0.0f;
// 	}
// }

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A3::appLogic()
{
	// Place per frame, application logic here ...

	uploadCommonSceneUniforms();

	// handle character movement
	if (keyLeftActive || keyRightActive || keyUpActive || keyDownActive) {
		player1.move(checkCollision(player1), showAnimation);
		// collide(player1, 'x');
		// collide(player1, 'y');
	}

	if (keyWActive || keyAActive || keySActive || keyDActive) {
		player2.move(checkCollision(player2), showAnimation);
	}

	// decrease water balloon lifetime
	if (waterBalloons.size() > 0 && waterBalloons.front().lifetime <= 0) {
		WaterBalloon b = waterBalloons.front();
		waterDamages.push_back(WaterDamage(b.x, b.y, water_lifetime, b.power, b.source));
		engine->play2D(getAssetFilePath("splat.wav").c_str());
		waterBalloons.erase(waterBalloons.begin());
	}
	for (WaterBalloon &balloon : waterBalloons) {
		balloon.lifetime -= 1;
	}

	if (p2_waterBalloons.size() > 0 && p2_waterBalloons.front().lifetime <= 0) {
		WaterBalloon b = p2_waterBalloons.front();
		waterDamages.push_back(WaterDamage(b.x, b.y, water_lifetime, b.power, b.source));
		engine->play2D(getAssetFilePath("splat.wav").c_str());
		p2_waterBalloons.erase(p2_waterBalloons.begin());
	}
	for (WaterBalloon &balloon : p2_waterBalloons) {
		balloon.lifetime -= 1;
	}

	// decrease water damage lifetime
	if (waterDamages.size() > 0 && waterDamages.front().lifetime <= 0) {
		waterDamages.erase(waterDamages.begin());
	}
	for (WaterDamage &w : waterDamages) {
		waterCollision(w);
		w.lifetime -= 1;
	}
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A3::guiLogic()
{
	if( !show_gui ) {
		return;
	}

	static bool firstRun(true);
	if (firstRun) {
		ImGui::SetNextWindowPos(ImVec2(50, 50));
		firstRun = false;
	}

	static bool showDebugWindow(true);
	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
	float opacity(0.5f);

	ImGui::Begin("Properties", &showDebugWindow, ImVec2(100,100), opacity,
			windowFlags);


		// Add more gui elements here here ...
		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("Application")) {
				if (ImGui::MenuItem("Reset All (A)")) {
					resetAll();
				}
				if (ImGui::MenuItem("Quit (Q)")) {
					glfwSetWindowShouldClose(m_window, GL_TRUE);
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Options")) {
				ImGui::Checkbox("Texture", &showTexture);
				ImGui::Checkbox("Animation", &showAnimation);
				ImGui::Checkbox("Collision", &showCollision);
				ImGui::SliderFloat("Water Transparency", &waterTransparency, 0.1f, 1.0f, "%.3f");
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		if (message != "") {
			ImGui::Text( "%s", message.c_str() );
		}
		ImGui::Text( "Player1 Speed: %.1f, Balloon: %i, Power: %i", 
			player1.speed, player1.balloonNumber, int(player1.power));
		ImGui::Text( "Player2 Speed: %.1f, Balloon: %i, Power: %i", 
			player2.speed, player2.balloonNumber, int(player2.power));
		ImGui::Text( "Player1 health: %i", player1.health);
		ImGui::Text( "Player2 health: %i", player2.health);
		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

	ImGui::End();
}

//----------------------------------------------------------------------------------------
// Update mesh specific shader uniforms:
static void updateShaderUniforms(
		const ShaderProgram & shader,
		const GeometryNode & node,
		const glm::mat4 & viewMatrix,
		const glm::mat4 & modalTrans,
		bool do_picking
) {

	shader.enable();
	{
		//-- Set ModelView matrix:
		GLint location = shader.getUniformLocation("ModelView");
		mat4 modelView = viewMatrix * modalTrans;
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(modelView));
		CHECK_GL_ERRORS;

		if( do_picking ) {
			unsigned int idx = node.m_nodeId;
			float r = float(idx&0xff) / 255.0f;
			float g = float((idx>>8)&0xff) / 255.0f;
			float b = float((idx>>16)&0xff) / 255.0f;

			location = shader.getUniformLocation("material.kd");
			glUniform3f( location, r, g, b );
			CHECK_GL_ERRORS;
		} else {
			//-- Set NormMatrix:
			location = shader.getUniformLocation("NormalMatrix");
			mat3 normalMatrix = glm::transpose(glm::inverse(mat3(modelView)));
			glUniformMatrix3fv(location, 1, GL_FALSE, value_ptr(normalMatrix));
			CHECK_GL_ERRORS;

			if (node.isSelected) {
				//-- Set Material values:
				location = shader.getUniformLocation("material.kd");
				vec3 col = vec3( 1.0, 1.0, 0.0 );
				glUniform3fv(location, 1, value_ptr(col));
				CHECK_GL_ERRORS;
			} else {
				//-- Set Material values:
				location = shader.getUniformLocation("material.kd");
				vec3 kd = node.material.kd;
				glUniform3fv(location, 1, value_ptr(kd));
				CHECK_GL_ERRORS;
				location = shader.getUniformLocation("material.ks");
				vec3 ks = node.material.ks;
				glUniform3fv(location, 1, value_ptr(ks));
				CHECK_GL_ERRORS;
				location = shader.getUniformLocation("material.shininess");
				glUniform1f(location, node.material.shininess);
				CHECK_GL_ERRORS;
			}
		}

	}
	shader.disable();

}

JointNode* A3::bfsJoint(SceneNode * root, unsigned int id) {
	queue<SceneNode *> q;
	q.push(root);

	while(!q.empty()) {
		SceneNode * n = q.front();
		q.pop();
		
		for (SceneNode * child : n->children) {
			if (child->m_nodeId == id) {
				if (n->m_nodeType == NodeType::JointNode) {
					child->isSelected = !child->isSelected;
					return static_cast<JointNode *>(n);
				} else {
					return NULL;
				}
			} else {
				q.push(child);
			}
		}
	}
	return NULL;
}

JointNode* A3::bfsJoint(SceneNode * root, string name) {
	queue<SceneNode *> q;
	q.push(root);

	while(!q.empty()) {
		SceneNode * n = q.front();
		q.pop();
		
		for (SceneNode * child : n->children) {
			if (child->m_name == name) {
				if (child->m_nodeType == NodeType::JointNode) {
					return static_cast<JointNode *>(child);
				} else {
					return NULL;
				}
			} else {
				q.push(child);
			}
		}
	}
	return NULL;
}

void A3::pickNode() {
	double xpos, ypos;
	glfwGetCursorPos( m_window, &xpos, &ypos );

	do_picking = true;

	uploadCommonSceneUniforms();
	glClearColor(1.0, 1.0, 1.0, 1.0 );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glClearColor(0.85, 0.85, 0.85, 1.0);

	draw();
	CHECK_GL_ERRORS;

	xpos *= double(m_framebufferWidth) / double(m_windowWidth);
	ypos = m_windowHeight - ypos;
	ypos *= double(m_framebufferHeight) / double(m_windowHeight);

	GLubyte buffer[ 4 ] = { 0, 0, 0, 0 };
	glReadBuffer( GL_BACK );
	glReadPixels( int(xpos), int(ypos), 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, buffer );
	CHECK_GL_ERRORS;

	unsigned int what = buffer[0] + (buffer[1] << 8) + (buffer[2] << 16);
	JointNode * j = bfsJoint(m_rootNode.get(), what);

	if( j != NULL ) {
		if (selectedJointNodes.find(j) != selectedJointNodes.end()) {
			selectedJointNodes.erase(j);
		} else {
			selectedJointNodes.insert(j);
		}
	}

	do_picking = false;

	CHECK_GL_ERRORS;
}

void A3::drawNodes(const SceneNode *node, mat4 t) {
	mat4 new_trans = t * node->trans;
	if (node->m_nodeType == NodeType::GeometryNode) {
		const GeometryNode * geometryNode = static_cast<const GeometryNode *>(node);
		updateShaderUniforms(m_shader, *geometryNode, m_view, new_trans, do_picking);

		// Get the BatchInfo corresponding to the GeometryNode's unique MeshId.
		BatchInfo batchInfo = m_batchInfoMap[geometryNode->meshId];

		//-- Now render the mesh:
		m_shader.enable();
		glDrawArrays(GL_TRIANGLES, batchInfo.startIndex, batchInfo.numIndices);
		m_shader.disable();
	}

	for (const SceneNode * n : node->children) {
		drawNodes(n, new_trans);
	}
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A3::draw() {
	// engine->play2D(getAssetFilePath("bell.wav").c_str());
	glEnable( GL_DEPTH_TEST );
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// renderShadows();

	renderFloor();
	renderBackground();
	
	// glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// glBindTexture(GL_TEXTURE_2D, shadowTexture);
	renderObstacles();
	renderSceneGraph(*m_rootNode);
	renderPlayer2(*m_p2Node);

	for (WaterBalloon &b : waterBalloons) {
		renderBalloon(*m_balloonNode, b);
	}

	for (WaterBalloon &b : p2_waterBalloons) {
		renderBalloon(*m_balloonNode, b);
	}

	for (WaterDamage &w : waterDamages) {
		renderWater(*m_waterNode, w);
	}

	for (Block &b : blocks) {
		renderBlock(*m_blockNode, b);
	}

	glDisable( GL_DEPTH_TEST );
}

void A3::renderShadows() {
	mat4 shadowP = ortho<float>(-10.0f, 10.0f, -10.0f, 10.0f, -10.0f, 1000.0f);
	mat4 shadowV = lookAt(m_light.position, glm::vec3(0,0,0), glm::vec3(0,1,0));
	shadowV = translate(shadowV, vec3( -float(DIM)/2.0f, 0, -float(DIM)/2.0f ));
	lightSpaceMatrix = shadowP * shadowV;
	glViewport(0, 0, 1024, 1024);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFrameBuffer);
		glClear(GL_DEPTH_BUFFER_BIT);

		depth_shader.enable();
		  	// render obstacles
		  	glBindVertexArray(m_cube_vao);
				GLint location = depth_shader.getUniformLocation("PVM");
				glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(lightSpaceMatrix));
				glDrawArrays(GL_TRIANGLES, 0, 36 * obstacles.size());
				glBindVertexArray(0);
				// render character... TODO
		depth_shader.disable();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glViewport(0, 0, m_framebufferWidth, m_framebufferHeight);
}

//----------------------------------------------------------------------------------------
void A3::renderSceneGraph(const SceneNode & root) {

	// Bind the VAO once here, and reuse for all GeometryNode rendering below.
	glBindVertexArray(m_vao_meshData);

	mat4 root_trans = root.trans;
	drawNodes(&root, root_trans * m_rot * inverse(root_trans));

	glBindVertexArray(0);
	CHECK_GL_ERRORS;
}

void A3::renderPlayer2(const SceneNode & node) {
	// Bind the VAO once here, and reuse for all GeometryNode rendering below.
	glBindVertexArray(m_vao_meshData);

	mat4 root_trans = node.trans;
	drawNodes(&node, root_trans * p2_rot * inverse(root_trans));

	glBindVertexArray(0);
	CHECK_GL_ERRORS;
}

void A3::renderBalloon(const SceneNode & root, WaterBalloon &b) {
	// Bind the VAO once here, and reuse for all GeometryNode rendering below.
	glBindVertexArray(m_vao_meshData);
	drawNodes(&root, b.trans);
	glBindVertexArray(0);
	CHECK_GL_ERRORS;
}

void A3::renderBlock(const SceneNode &root, Block &b) {
	if (showTexture) {
		if (b.special == 1) {
			glBindTexture(GL_TEXTURE_2D, xmas_cube_texture);
		} else if (b.special == 2) {
			glBindTexture(GL_TEXTURE_2D, snow_cube_texture);
		} else if (b.special == 3) {
			glBindTexture(GL_TEXTURE_2D, gold_cube_texture);
		} else {
			glBindTexture(GL_TEXTURE_2D, wood_cube_texture);
		}
	}
	
	glBindVertexArray(m_vao_meshData);
	m_shader.enable();
		GLint location = m_shader.getUniformLocation("useTexture");
		glUniform1i( location, 1 );
	m_shader.disable();

	drawNodes(&root, b.trans);

	m_shader.enable();
		location = m_shader.getUniformLocation("useTexture");
		glUniform1i( location, 0 );
	m_shader.disable();
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	CHECK_GL_ERRORS;
}

void A3::renderWater(const SceneNode &root, WaterDamage &w) {
	// Bind the VAO once here, and reuse for all GeometryNode rendering below.
	if (showTexture) glBindTexture(GL_TEXTURE_2D, water_texture);
	glBindVertexArray(m_vao_meshData);
	m_shader.enable();
		GLint location = m_shader.getUniformLocation("useTexture");
		glUniform1i( location, 1 );
		location = m_shader.getUniformLocation("alpha");
		glUniform1f( location, waterTransparency );
	m_shader.disable();
	CHECK_GL_ERRORS;

	if (w.curr_power < w.power) {
		if (!w.right_blocked) {
			w.trans1[0].x += water_speed;
			w.curr_right_power += water_speed;
		}
		if (!w.left_blocked) {
			w.trans2[0].x -= water_speed;
			w.curr_left_power += water_speed;
		}
		if (!w.down_blocked) {
			w.trans3[2].z += water_speed;
			w.curr_down_power += water_speed;
		}
		if (!w.up_blocked) {
			w.trans4[2].z -= water_speed;
			w.curr_up_power += water_speed;
		}
		w.curr_power += water_speed;
	}

	drawNodes(&root, w.trans1); // right
	drawNodes(&root, w.trans2); // left
	drawNodes(&root, w.trans3); // down
	drawNodes(&root, w.trans4); // up

	m_shader.enable();
		location = m_shader.getUniformLocation("useTexture");
		glUniform1i( location, 0 );
		location = m_shader.getUniformLocation("alpha");
		glUniform1f( location, 1.0 );
	m_shader.disable();
	CHECK_GL_ERRORS;
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
// Draw the trackball circle.
void A3::renderArcCircle() {
	glBindVertexArray(m_vao_arcCircle);

	m_shader_arcCircle.enable();
		GLint m_location = m_shader_arcCircle.getUniformLocation( "M" );
		float aspect = float(m_framebufferWidth)/float(m_framebufferHeight);
		glm::mat4 M;
		if( aspect > 1.0 ) {
			M = glm::scale( glm::mat4(), glm::vec3( 0.5/aspect, 0.5, 1.0 ) );
		} else {
			M = glm::scale( glm::mat4(), glm::vec3( 0.5, 0.5*aspect, 1.0 ) );
		}
		glUniformMatrix4fv( m_location, 1, GL_FALSE, value_ptr( M ) );
		glDrawArrays( GL_LINE_LOOP, 0, CIRCLE_PTS );
	m_shader_arcCircle.disable();

	glBindVertexArray(0);
	CHECK_GL_ERRORS;
}

// Draw the floor
void A3::renderFloor() {
	if (showTexture) glBindTexture(GL_TEXTURE_2D, floor_texture);
	glBindVertexArray(m_floor_vao);

	m_tex_shader.enable();
		//-- Set ModelView matrix:
		GLint location = m_tex_shader.getUniformLocation("PVM");
		mat4 PVM = m_perpsective * m_view;
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(PVM));
		glDrawArrays(GL_TRIANGLES, 0, 6);
	m_tex_shader.disable();

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	CHECK_GL_ERRORS;
}

void A3::renderBackground() {
	if (showTexture) glBindTexture(GL_TEXTURE_2D, bg_texture);
	glBindVertexArray(m_bg_vao);

	m_tex_shader.enable();
		//-- Set ModelView matrix:
		GLint location = m_tex_shader.getUniformLocation("PVM");
		mat4 PVM = m_perpsective * m_view;
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(PVM));
		glDrawArrays(GL_TRIANGLES, 0, 6);
	m_tex_shader.disable();

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	CHECK_GL_ERRORS;
}

void A3::renderObstacles() {
	// glActiveTexture(GL_TEXTURE0 + 0);
	if (showTexture) glBindTexture(GL_TEXTURE_2D, obstacle_texture);
	// glActiveTexture(GL_TEXTURE0 + 1);
	// glBindTexture(GL_TEXTURE_2D, shadowTexture);
	glBindVertexArray(m_cube_vao);

	m_tex_shader.enable();
		//-- Set ModelView matrix:
		GLint location = m_tex_shader.getUniformLocation("PVM");
		mat4 PVM = m_perpsective * m_view;
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(PVM));

		// location = shadow_shader.getUniformLocation("lightMatrix");
		// glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(lightSpaceMatrix));

		glDrawArrays(GL_TRIANGLES, 0, 36 * obstacles.size());
	m_tex_shader.disable();

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A3::cleanup()
{

}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A3::cursorEnterWindowEvent (
		int entered
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse cursor movement events.
 */
bool A3::mouseMoveEvent (
		double xPos,
		double yPos
) {
	bool eventHandled(false);

	// Fill in with event handling code...
	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A3::mouseButtonInputEvent (
		int button,
		int actions,
		int mods
) {
	bool eventHandled(false);

	// Fill in with event handling code...
	if (!ImGui::IsMouseHoveringAnyWindow()) {
		if (actions == GLFW_PRESS) {
			if (button == GLFW_MOUSE_BUTTON_LEFT) {
				
				mouseLeftActive = true;
			}
			if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
				mouseMiddleActive = true;
			}
			if (button == GLFW_MOUSE_BUTTON_RIGHT) {
				mouseRightActive = true;
			}
			eventHandled = true;
		}

		if (actions == GLFW_RELEASE) {
			if (button == GLFW_MOUSE_BUTTON_LEFT) {
				mouseLeftActive = false;
			}
			if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
				mouseMiddleActive = false;
			}
			if (button == GLFW_MOUSE_BUTTON_RIGHT) {
				mouseRightActive = false;
			}
			eventHandled = true;
		}
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A3::mouseScrollEvent (
		double xOffSet,
		double yOffSet
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles window resize events.
 */
bool A3::windowResizeEvent (
		int width,
		int height
) {
	bool eventHandled(false);
	initPerspectiveMatrix();
	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A3::keyInputEvent (
		int key,
		int action,
		int mods
) {
	bool eventHandled(false);
	SceneNode & rootNode = *m_rootNode;
	if( action == GLFW_PRESS ) {
		if( key == GLFW_KEY_M ) {
			show_gui = !show_gui;
		}
		if (key == GLFW_KEY_Q) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}
		if(key == GLFW_KEY_LEFT) {
			keyLeftActive = true;
			m_rot = player1.setDirection(1);
		}
		if(key == GLFW_KEY_RIGHT) {
			keyRightActive = true;
			m_rot = player1.setDirection(3);
		}
		if(key == GLFW_KEY_UP) {
			keyUpActive = true;
			m_rot = player1.setDirection(2);
		}
		if(key == GLFW_KEY_DOWN) {
			keyDownActive = true;
			m_rot = player1.setDirection(0);
		}
		if(key == GLFW_KEY_SPACE) {
			if (waterBalloons.size() < player1.balloonNumber) {
				pushWaterBalloon(waterBalloons, round(player1.x), round(player1.y), player1.power, 1);
			}
		}
		if( key == GLFW_KEY_A ) {
			keyAActive = true;
			p2_rot = player2.setDirection(1);
		}
		if( key == GLFW_KEY_D ) {
			keyDActive = true;
			p2_rot = player2.setDirection(3);
		}
		if(key == GLFW_KEY_W) {
			keyWActive = true;
			p2_rot = player2.setDirection(2);
		}
		if(key == GLFW_KEY_S) {
			keySActive = true;
			p2_rot = player2.setDirection(0);
		}
		if(key == GLFW_KEY_LEFT_SHIFT) {
			if (p2_waterBalloons.size() < player2.balloonNumber) {
				pushWaterBalloon(p2_waterBalloons, round(player2.x), round(player2.y), player2.power, 2);
			}
		}
		eventHandled = true;
	}

	if( action == GLFW_RELEASE ) {
		if(key == GLFW_KEY_LEFT) {
			keyLeftActive = false;
			player1.removeDirection(1);
		}
		if(key == GLFW_KEY_RIGHT) {
			keyRightActive = false;
			player1.removeDirection(3);
		}
		if(key == GLFW_KEY_UP) {
			keyUpActive = false;
			player1.removeDirection(2);
		}
		if(key == GLFW_KEY_DOWN) {
			keyDownActive = false;
			player1.removeDirection(0);
		}
		if( key == GLFW_KEY_A ) {
			keyAActive = false;
			player2.removeDirection(1);
		}
		if( key == GLFW_KEY_D ) {
			keyDActive = false;
			player2.removeDirection(3);
		}
		if(key == GLFW_KEY_W) {
			keyWActive = false;
			player2.removeDirection(2);
		}
		if(key == GLFW_KEY_S) {
			keySActive = false;
			player2.removeDirection(0);
		}
	}

	return eventHandled;
}
