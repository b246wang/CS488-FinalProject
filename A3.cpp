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
#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <queue>

using namespace glm;

static const size_t DIM = 10;
static const float cube_h = 1.0f;
static bool show_gui = true;
static const float TranslateFactor = 200.0f;
static const float JointRotateFactor = 15.0f;
const size_t CIRCLE_PTS = 48;
static const string BadUndo = "Cannot Undo. Reached the end of the stack.";
static const string BadRedo = "Cannot Redo. Reached the end of the stack.";

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
	  m_grid_vao(0),
	  m_grid_vbo(0),
	  m_floor_vao(0),
	  m_floor_vbo(0),
	  player1(vec3(0.0, 0.0, 0.0))
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
	// player 1
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

	do_picking = false;
	mouseLeftActive = false;
	mouseMiddleActive = false;
	mouseRightActive = false;
	showCircle = true;
	current_mode = 0;
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
	initGrid();
	initFloor();
	initObstacles();
	floor_texture = createTexture(getAssetFilePath("floor.png"));
	obstacle_texture = createTexture(getAssetFilePath("obstacle.png"));

	initPerspectiveMatrix();

	initViewMatrix();

	initLightSources();


	// Exiting the current scope calls delete automatically on meshConsolidator freeing
	// all vertex data resources.  This is fine since we already copied this data to
	// VBOs on the GPU.  We have no use for storing vertex data on the CPU side beyond
	// this point.
}

void A3::initGrid() {
	size_t sz = 3 * 2 * 2 * (DIM+3);

	float *verts = new float[ sz ];
	size_t ct = 0;
	for( int idx = 0; idx < DIM+3; ++idx ) {
		verts[ ct ] = -1;
		verts[ ct+1 ] = 0;
		verts[ ct+2 ] = idx-1;
		verts[ ct+3 ] = DIM+1;
		verts[ ct+4 ] = 0;
		verts[ ct+5 ] = idx-1;
		ct += 6;

		verts[ ct ] = idx-1;
		verts[ ct+1 ] = 0;
		verts[ ct+2 ] = -1;
		verts[ ct+3 ] = idx-1;
		verts[ ct+4 ] = 0;
		verts[ ct+5 ] = DIM+1;
		ct += 6;
	}

	// Create the vertex array to record buffer assignments.
	glGenVertexArrays( 1, &m_grid_vao );
	glBindVertexArray( m_grid_vao );

	// Create the cube vertex buffer
	glGenBuffers( 1, &m_grid_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, m_grid_vbo );
	glBufferData( GL_ARRAY_BUFFER, sz*sizeof(float),
		verts, GL_STATIC_DRAW );

	// Specify the means of extracting the position values properly.
	GLint posAttrib = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray( posAttrib );
	glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

	// Reset state to prevent rogue code from messing with *my* 
	// stuff!
	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	// OpenGL has the buffer now, there's no need for us to keep a copy.
	delete [] verts;

	CHECK_GL_ERRORS;
}

void A3::initFloor()
{
	vec3 floorVertices[] = {
		vec3(0, 0, 0), vec3(DIM, 0, 0), vec3(0, 0, DIM),
		vec3(0, 0, DIM), vec3(DIM, 0, 0), vec3(DIM, 0, DIM)
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

void A3::initObstacles() {
	float x = 2;
	float y = 0;
	vec3 cubeVertices[] = {
		// front
		vec3(x, cube_h, y), vec3(x + 1, cube_h, y), vec3(x, cube_h, y + 1),
		vec3(x, cube_h, y + 1), vec3(x + 1, cube_h, y), vec3(x + 1, cube_h, y + 1),
		// back
		vec3(x, 0, y), vec3(x + 1, 0, y), vec3(x, 0, y + 1),
		vec3(x, 0, y + 1), vec3(x + 1, 0, y), vec3(x + 1, 0, y + 1),
		// left
		vec3(x, cube_h, y), vec3(x, 0, y), vec3(x, 0, y + 1),
		vec3(x, 0, y + 1), vec3(x, cube_h, y), vec3(x, cube_h, y + 1),
		// right
		vec3(x + 1, cube_h, y), vec3(x + 1, 0, y), vec3(x + 1, 0, y + 1),
		vec3(x + 1, 0, y + 1), vec3(x + 1, cube_h, y), vec3(x + 1, cube_h, y + 1),
		// top
		vec3(x, cube_h, y + 1), vec3(x, 0, y + 1), vec3(x + 1, 0, y + 1),
		vec3(x + 1, 0, y + 1), vec3(x, cube_h, y + 1), vec3(x + 1, cube_h, y + 1),
		// bottom
		vec3(x, cube_h, y), vec3(x, 0, y), vec3(x + 1, 0, y),
		vec3(x + 1, 0, y), vec3(x, cube_h, y), vec3(x + 1, cube_h, y),
	};

	vec2 cubeUVVertices[] = {
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

	// Create the vertex array to record buffer assignments.
	glGenVertexArrays( 1, &m_cube_vao );
	glBindVertexArray( m_cube_vao );

	// Create the floor vertex buffer
	glGenBuffers( 1, &m_cube_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, m_cube_vbo );
	glBufferData( GL_ARRAY_BUFFER, sizeof(cubeVertices),
		cubeVertices, GL_STATIC_DRAW );

	// Specify the means of extracting the position values properly.
	GLint posAttrib = m_tex_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray( posAttrib );
	glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

	// Create the floor uv coord buffer
	glGenBuffers( 1, &m_cube_uv_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, m_cube_uv_vbo );
	glBufferData( GL_ARRAY_BUFFER, sizeof(cubeUVVertices),
		cubeUVVertices, GL_STATIC_DRAW );

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

//----------------------------------------------------------------------------------------
void A3::uploadCommonSceneUniforms() {
	m_shader.enable();
	{
		//-- Set Perpsective matrix uniform for the scene:
		GLint location = m_shader.getUniformLocation("Perspective");
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(m_perpsective));
		CHECK_GL_ERRORS;

		location = m_shader.getUniformLocation("picking");
		glUniform1i( location, do_picking ? 1 : 0 );

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
		player1.move();
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
				if (ImGui::MenuItem("Reset Position (I)")) {
					resetPosition();
				}
				if (ImGui::MenuItem("Reset Orientation (O)")) {
					resetRotation();
				}
				if (ImGui::MenuItem("Reset Joints (S)")) {
					resetJoints();
				}
				if (ImGui::MenuItem("Reset All (A)")) {
					resetAll();
				}
				if (ImGui::MenuItem("Quit (Q)")) {
					glfwSetWindowShouldClose(m_window, GL_TRUE);
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Edit")) {
				if (ImGui::MenuItem("Undo (U)")) {
					undo();
				}
				if (ImGui::MenuItem("Redo (R)")) {
					redo();
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Options")) {
				ImGui::Checkbox("Circle (C)", &showCircle);
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		ImGui::RadioButton("Position/Orientation (P)", &current_mode, 0);
		ImGui::RadioButton("Joints (J)", &current_mode, 1);

		if (message != "") {
			ImGui::Text( "%s", message.c_str() );
		}
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

void A3::pushJointStack() {
	if (selectedJointNodes.size()) {
		vector<JointHistory> v;
		for (JointNode * node : selectedJointNodes) {
			JointHistory h(node);
			v.push_back(h);
		}
		jointStack.resize(stack_idx + 1);
		jointStack.push_back(v);
		stack_idx++;
		message = "";
		// cout << "pushed to stack, stack_idx: " << stack_idx << endl;
	}
}

void A3::undo() {
	vector<JointHistory> v;
	if (stack_idx == 1) {
		v = jointStack[stack_idx];
		for (JointHistory h : v) {
			JointNode * node = h.n;
			node->m_joint_x.curr = node->m_joint_x.init;
			node->m_joint_y.curr = node->m_joint_y.init;
			node->trans = mat4(1.0f);
		}
		stack_idx--;
		message = "";
	} else if (stack_idx > 1) {
		v = jointStack[stack_idx-1];
		for (JointHistory h : v) {
			JointNode * node = h.n;
			node->m_joint_x.curr = h.x;
			node->m_joint_y.curr = h.y;
			node->trans = h.t;
		}
		stack_idx--;
		message = "";
	} else {
		message = BadUndo;
	}
	// cout << "undo, stack_idx: " << stack_idx << endl;
}

void A3::redo() {
	if (stack_idx + 1 == jointStack.size()) {
		message = BadRedo;
		return;
	}
	vector<JointHistory> v = jointStack[++stack_idx];
	for (JointHistory h : v) {
		JointNode * node = h.n;
		node->m_joint_x.curr = h.x;
		node->m_joint_y.curr = h.y;
		node->trans = h.t;
	}
	message = "";
	// cout << "redo, stack_idx: " << stack_idx << endl;
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A3::draw() {
	glEnable( GL_DEPTH_TEST );

	renderGrid();
	renderFloor();
	renderObstacles();

	renderSceneGraph(*m_rootNode);

	glDisable( GL_DEPTH_TEST );
}

//----------------------------------------------------------------------------------------
void A3::renderSceneGraph(const SceneNode & root) {

	// Bind the VAO once here, and reuse for all GeometryNode rendering below.
	glBindVertexArray(m_vao_meshData);

	mat4 root_trans = root.trans;
	drawNodes(&root, m_trans * root_trans * m_rot * inverse(root_trans));

	glBindVertexArray(0);
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

//----------------------------------------------------------------------------------------
// Draw the grid
void A3::renderGrid() {
	glBindVertexArray(m_grid_vao);

	m_shader.enable();
		//-- Set ModelView matrix:
		GLint location = m_shader.getUniformLocation("ModelView");
		mat4 modelView = m_view;
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(modelView));

		location = m_shader.getUniformLocation("material.kd");
		vec3 col = vec3(1.0, 1.0, 1.0);
		glUniform3fv(location, 1, value_ptr(col));
		glDrawArrays(GL_LINES, 0, (3+DIM)*4);
	m_shader.disable();

	glBindVertexArray(0);
	CHECK_GL_ERRORS;
}

// Draw the floor
void A3::renderFloor() {
	glBindTexture(GL_TEXTURE_2D, floor_texture);
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

void A3::renderObstacles() {
	glBindTexture(GL_TEXTURE_2D, obstacle_texture);
	glBindVertexArray(m_cube_vao);

	m_tex_shader.enable();
		//-- Set ModelView matrix:
		GLint location = m_tex_shader.getUniformLocation("PVM");
		mat4 PVM = m_perpsective * m_view;
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(PVM));
		glDrawArrays(GL_TRIANGLES, 0, 36);
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
	if (!ImGui::IsMouseHoveringAnyWindow()) {
		double d_x = xPos - prev_x;
		double d_y = yPos - prev_y;

		if (mouseLeftActive) {
			if (current_mode == 0) {
				double tx = d_x / TranslateFactor;
				double ty = d_y / TranslateFactor;
				mat4 trans(1.0f);
				trans[3].x = tx;
				trans[3].y = -ty;
				m_trans = trans * m_trans;
			}
		}
		if (mouseMiddleActive) {
			if (current_mode == 0) {
				double ty = d_y / TranslateFactor;
				mat4 trans(1.0f);
				trans[3].z = ty;
				m_trans = trans * m_trans;
			} else {
				double ty = d_y / JointRotateFactor;
				for (JointNode * node : selectedJointNodes) {
					node->rotate('x', ty);
				}
			}
		}
		if (mouseRightActive) {
			if (current_mode == 1) {
				double ty = d_y / JointRotateFactor;
				for (JointNode * node : selectedJointNodes) {
					node->rotate('y', ty);
				}
			}
		}

		prev_x = xPos;
		prev_y = yPos;
		eventHandled = true;
	}

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
				if (current_mode == 1) {
					pickNode();
				}
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
				if (current_mode == 1) {
					pushJointStack();
				}
				mouseMiddleActive = false;
			}
			if (button == GLFW_MOUSE_BUTTON_RIGHT) {
				mouseRightActive = false;
				if (current_mode == 1) {
					pushJointStack();
				}
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
		if( key == GLFW_KEY_P ) {
			current_mode = 0;
		}
		if( key == GLFW_KEY_J ) {
			current_mode = 1;
		}
		if( key == GLFW_KEY_I ) {
			resetPosition();
		}
		if( key == GLFW_KEY_O ) {
			resetRotation();
		}
		if( key == GLFW_KEY_S ) {
			resetJoints();
		}
		if( key == GLFW_KEY_A ) {
			resetAll();
		}
		if( key == GLFW_KEY_U ) {
			undo();
		}
		if( key == GLFW_KEY_R ) {
			redo();
		}
		if( key == GLFW_KEY_C ) {
			showCircle = !showCircle;
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
		eventHandled = true;
	}

	if( action == GLFW_RELEASE ) {
		if(key == GLFW_KEY_LEFT) {
			keyLeftActive = false;
		}
		if(key == GLFW_KEY_RIGHT) {
			keyRightActive = false;
		}
		if(key == GLFW_KEY_UP) {
			keyUpActive = false;
		}
		if(key == GLFW_KEY_DOWN) {
			keyDownActive = false;
		}
	}

	return eventHandled;
}
