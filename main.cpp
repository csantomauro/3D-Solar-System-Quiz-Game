
// This has been adapted from the Vulkan tutorial
#include "Starter.hpp"
#define M_PI 3.1415926

// The uniform buffer objects data structures
// Remember to use the correct alignas(...) value
//        float : alignas(4)
//        vec2  : alignas(8)
//        vec3  : alignas(16)
//        vec4  : alignas(16)
//        mat3  : alignas(16)
//        mat4  : alignas(16)

struct MeshUniformBlock {
	alignas(4) float amb;
	alignas(4) float gamma;
	alignas(16) glm::vec3 sColor;
	alignas(16) glm::mat4 mvpMat;
	alignas(16) glm::mat4 mMat;
	alignas(16) glm::mat4 nMat;
};

struct OverlayUniformBlock {
	alignas(4) float visible;
};

struct OverlaySpeedIndicatorUniformBlock {
	alignas(4) float x;
};

// struct GlobalUniformBlockDirect {
// 	alignas(16) glm::vec3 DlightDir;
// 	alignas(16) glm::vec3 DlightColor;
// 	alignas(16) glm::vec3 AmbLightColor;
// 	alignas(16) glm::vec3 eyePos;
// };

struct GlobalUniformBlockPoint {
	alignas(16) glm::vec3 lightPos;
	alignas(16) glm::vec4 lightColor;
	alignas(16) glm::vec3 AmbLightColor;
	alignas(16) glm::vec3 eyePos;
};

struct OrbitsUniformBlock {
	alignas(4) float visible;
	alignas(16) glm::mat4 mvpMat;
	alignas(16) glm::mat4 mMat;
	alignas(16) glm::mat4 nMat;
};

struct VertexMesh {
	glm::vec3 pos;
	glm::vec3 norm;
	glm::vec2 UV;
};

struct VertexOverlay {
	glm::vec2 pos;
	glm::vec2 UV;
};

struct VertexOverlaySpeedIndicator {
	glm::vec2 pos;
};

struct VertexOrbit {
	glm::vec3 pos;
};

struct CelestialBodyVulkanData {
	std::string name;
	float* rot;
	float* rev;
	Model<VertexMesh>* model;
	DescriptorSet* DS;
	Texture* tex;
	MeshUniformBlock* ubo;
};

struct CelestialBodyInfoVulkanData {
	std::string name;
	Model<VertexOverlay>* model;
	DescriptorSet* DS;
	Texture* tex;
	OverlayUniformBlock* ubo;
};

enum ClickAction {
	SHOW_INFO_SUN, SHOW_INFO_MERCURY, SHOW_INFO_VENUS, SHOW_INFO_EARTH,
	SHOW_INFO_MARS, SHOW_INFO_JUPITER, SHOW_INFO_SATURN, SHOW_INFO_URANUS, SHOW_INFO_NEPTUNE,
	HIDE_INFO,
	START_MOVING, STOP_MOVING,
	SHOW_ORBITS, HIDE_ORBITS,
	SPEED_TRACKBAR_L, SPEED_TRACKBAR_R
};

class SolarSystem : public BaseProject {
protected:
	// Current aspect ratio (used by the callback that resized the window
	float Ar;

	DescriptorSetLayout DSLPlanet, DSLSun, DSLSkydome, DSLOverlay;
	DescriptorSetLayout DSLOrbits;
	DescriptorSetLayout DSLOverlaySpeedIndicator;

	// Vertex formats
	VertexDescriptor VMesh;
	VertexDescriptor VOverlay;
	VertexDescriptor VOrbits;
	VertexDescriptor VOverlaySpeedIndicator;

	// Pipelines [Shader couples]
	Pipeline PPlanet, PSun, PSkydome, POverlay;
	Pipeline POrbits;
	Pipeline POverlaySpeedIndicator;

	// Models, textures and Descriptors (values assigned to the uniforms)
	// Please note that Model objects depends on the corresponding vertex structure
	Model<VertexMesh> MSun, MSkydome,
		MMercury, MVenus, MEarth, MMars, MJupiter,
		MSaturn, MSaturnRing, MUranus, MNeptune;
	Model<VertexMesh> MAsteroidsBelt;
	Model<VertexOverlay> MKey;
	Model<VertexOverlay> MSunI, MMercuryI, MVenusI, MEarthI, MMarsI, MJupiterI, MSaturnI, MUranusI, MNeptuneI;
	Model<VertexOrbit> MOrbits;
	Model<VertexOverlaySpeedIndicator> MOverlaySpeedIndicator;

	DescriptorSet DSSun;
	DescriptorSet DSSkydome;
	DescriptorSet DSMercury, DSVenus, DSEarth, DSMars, DSJupiter,
		DSSaturn, DSSaturnRing, DSUranus, DSNeptune, DSAsteroidsBelt;
	DescriptorSet DSSunI, DSMercuryI, DSVenusI, DSEarthI, DSMarsI,
		DSJupiterI, DSSaturnI, DSUranusI, DSNeptuneI, DSKey;
	DescriptorSet DSOrbits;
	DescriptorSet DSOverlaySpeedIndicator;

	Texture TSun, TSkydome,
		TMercury, TVenus, TEarth, TMars, TJupiter,
		TSaturn, TSaturnRing, TUranus, TNeptune, TKey;
	Texture TSunI, TMercuryI, TVenusI, TEarthI, TMarsI, TJupiterI, TSaturnI, TUranusI, TNeptuneI;
	Texture TAsteroid;

	// C++ storage foruniform variables
	MeshUniformBlock uboSun, uboSkydome,
		uboMercury, uboVenus, uboEarth, uboMars, uboJupiter,
		uboSaturn, uboSaturnRing, uboUranus, uboNeptune;
	MeshUniformBlock uboAsteroid;


	OverlayUniformBlock uboKey;
	OverlayUniformBlock uboSunI, uboMercuryI, uboVenusI, uboEarthI, uboMarsI, uboJupiterI, uboSaturnI, uboUranusI, uboNeptuneI;
	OverlaySpeedIndicatorUniformBlock uboOverlaySpeedIndicator = {0};
	
	GlobalUniformBlockPoint pgubo;
	OrbitsUniformBlock uboOrbits;
	
	std::vector<CelestialBodyVulkanData> celestialBodies = {};
	std::vector<CelestialBodyInfoVulkanData> celestialBodiesInfo = {};

	// Other application parameters

	nlohmann::json solarSystemData;

	float speedMultiplier = 1;
	int isMoving = 0;
	int orbitsVisibility = 0;

	float SunRev = 0.0;
	float MercuryRev = 0.0;
	float VenusRev = 0.0;
	float EarthRev = 0.0;
	float MarsRev = 0.0;
	float JupiterRev = 0.0;
	float SaturnRev = 0.0;
	float UranusRev = 0.0;
	float NeptuneRev = 0.0;

	float SunRot = 0.0;
	float MercuryRot = 0.0;
	float VenusRot = 0.0;
	float EarthRot = 0.0;
	float MarsRot = 0.0;
	float JupiterRot = 0.0;
	float SaturnRot = 0.0;
	float UranusRot = 0.0;
	float NeptuneRot = 0.0;

	float radiusSkydome = 250.0f;
	int clickAction = HIDE_INFO;
	
	std::vector<glm::vec2> buttonsBounds = {};
	std::vector<float> velocities = {
		0.001, 0.002, 0.005,
		0.01, 0.02, 0.05,
		0.1, 0.2, 0.5,
		1, 2, 5,
		10, 20, 50,
		100, 200, 500,
		1000
	};

	// Camera Parameters
	const float FOVy = glm::radians(60.0f);
	const float nearPlane = 0.1f;
	const float farPlane = 400.0f;
	const float rotSpeed = glm::radians(90.0f);
	const float movSpeed = 15.0f;

	glm::vec3 camPos = glm::vec3(7.8, 3.3, 25);
		
	float camAlpha = 0.0f;
	float camBeta = 0.0f;
	// float camRho = 0.0f;

	int cameraIsColliding = 0;

	// Here you set the main application parameters
	void setWindowParameters() {
		// window size, titile and initial background
		windowWidth = 1000;
		windowHeight = 750;
		windowTitle = "Solar System";
		windowResizable = GLFW_TRUE;
		initialBackgroundColor = { 0.0f, 0.005f, 0.01f, 1.0f };

		// Descriptor pool sizes

		/* Update the requirements forthe size of the pool */
		uniformBlocksInPool = 36;
		texturesInPool = 23; // Sun loaded twice for emission texture
		setsInPool = 24;

		Ar = (float)windowWidth / (float)windowHeight;
	}
	
	// What to do when the window changes size
	void onWindowResize(int w, int h) {
		Ar = (float)w / (float)h;
	}


	// Here you load and setup all your Vulkan Models and Texutures.
	// Here you also create your Descriptor set layouts and load the shaders forthe pipelines
	void localInit() {
		// Various local parameters--------------------------------------------------
		celestialBodies.push_back({ "Sun", &SunRot, &SunRev, &MSun, &DSSun, &TSun, &uboSun });
		celestialBodies.push_back({ "Mercury", &MercuryRot, &MercuryRev, &MMercury, &DSMercury, &TMercury, &uboMercury });
		celestialBodies.push_back({ "Venus", &VenusRot, &VenusRev, &MVenus, &DSVenus, &TVenus, &uboVenus });
		celestialBodies.push_back({ "Earth", &EarthRot, &EarthRev, &MEarth, &DSEarth, &TEarth, &uboEarth });
		celestialBodies.push_back({ "Mars", &MarsRot, &MarsRev, &MMars, &DSMars, &TMars, &uboMars });
		celestialBodies.push_back({ "Jupiter", &JupiterRot, &JupiterRev, &MJupiter, &DSJupiter, &TJupiter, &uboJupiter });
		celestialBodies.push_back({ "Saturn", &SaturnRot, &SaturnRev, &MSaturn, &DSSaturn, &TSaturn, &uboSaturn });
		celestialBodies.push_back({ "Uranus", &UranusRot, &UranusRev, &MUranus, &DSUranus, &TUranus, &uboUranus });
		celestialBodies.push_back({ "Neptune", &NeptuneRot, &NeptuneRev, &MNeptune, &DSNeptune, &TNeptune, &uboNeptune });

		celestialBodiesInfo.push_back({ "Sun", &MSunI, &DSSunI, &TSunI, &uboSunI });
		celestialBodiesInfo.push_back({ "Mercury", &MMercuryI, &DSMercuryI, &TMercuryI, &uboMercuryI });
		celestialBodiesInfo.push_back({ "Venus", &MVenusI, &DSVenusI, &TVenusI, &uboVenusI });
		celestialBodiesInfo.push_back({ "Earth", &MEarthI, &DSEarthI, &TEarthI, &uboEarthI });
		celestialBodiesInfo.push_back({ "Mars", &MMarsI, &DSMarsI, &TMarsI, &uboMarsI });
		celestialBodiesInfo.push_back({ "Jupiter", &MJupiterI, &DSJupiterI, &TJupiterI, &uboJupiterI });
		celestialBodiesInfo.push_back({ "Saturn", &MSaturnI, &DSSaturnI, &TSaturnI, &uboSaturnI });
		celestialBodiesInfo.push_back({ "Uranus", &MUranusI, &DSUranusI, &TUranusI, &uboUranusI });
		celestialBodiesInfo.push_back({ "Neptune", &MNeptuneI, &DSNeptuneI, &TNeptuneI, &uboNeptuneI });

		std::ifstream configFile("data/solarSystemData.json");

		if (!configFile.is_open()) {
			throw std::runtime_error("Failed to open planet data file.");
		}

		std::ostringstream configStringStream;
		configStringStream << configFile.rdbuf();
		configFile.close();

		solarSystemData = nlohmann::json::parse(configStringStream.str());

		// Descriptor Layouts [what will be passed to the shaders]--------------------------------------------------
		DSLPlanet.init(this, {
			// this array contains the bindings:
			// first  element : the binding number
			// second element : the type of element (buffer or texture)
			//                  using the corresponding Vulkan constant
			// third  element : the pipeline stage where it will be used
			//                  using the corresponding Vulkan constant
			{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS},
			{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT},
			{2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT}
		});

		DSLSun.init(this, {
			{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS},
			{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT},
			{2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT},
			{3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}
		});

		DSLOverlay.init(this, {
			{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS},
			{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}
		});

		DSLSkydome.init(this, {
			{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS},
			{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT},
			{2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT}
		});

		DSLOrbits.init(this, {
			{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS}
		});

		DSLOverlaySpeedIndicator.init(this, {
			{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS}
		});

		// Vertex descriptors--------------------------------------------------
		VMesh.init(this, {
			// this array contains the bindings
			// first  element : the binding number
			// second element : the stride of this binging
			// third  element : whether this parameter change per vertex or per instance
			//                  using the corresponding Vulkan constant
			{0, sizeof(VertexMesh), VK_VERTEX_INPUT_RATE_VERTEX}
			}, {
				// this array contains the location
				// first  element : the binding number
				// second element : the location number
				// third  element : the offset of this element in the memory record
				// fourth element : the data type of the element
				//                  using the corresponding Vulkan constant
				// fifth  elmenet : the size in byte of the element
				// sixth  element : a constant defining the element usage
				//                   POSITION - a vec3 with the position
				//                   NORMAL   - a vec3 with the normal vector
				//                   UV       - a vec2 with a UV coordinate
				//                   COLOR    - a vec4 with a RGBA color
				//                   TANGENT  - a vec4 with the tangent vector
				//                   OTHER    - anything else
				//
				// ***************** DOUBLE CHECK ********************
				//    That the Vertex data structure you use in the "offsetoff" and
				//	in the "sizeof" in the previous array, refers to the correct one,
				//	if you have more than one vertex format!
				// ***************************************************
				{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexMesh, pos),
					   sizeof(glm::vec3), POSITION},
				{0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexMesh, norm),
					   sizeof(glm::vec3), NORMAL},
				{0, 2, VK_FORMAT_R32G32_SFLOAT, offsetof(VertexMesh, UV),
					   sizeof(glm::vec2), UV}
			});

		VOverlay.init(this, {
				  {0, sizeof(VertexOverlay), VK_VERTEX_INPUT_RATE_VERTEX}
			}, {
			  {0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(VertexOverlay, pos),
					 sizeof(glm::vec2), OTHER},
			  {0, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(VertexOverlay, UV),
					 sizeof(glm::vec2), UV}
			});

		VOrbits.init(this, {
				  {0, sizeof(VertexOrbit), VK_VERTEX_INPUT_RATE_VERTEX}
			}, {
				{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexOrbit, pos),
						sizeof(glm::vec3), POSITION}
			});

		VOverlaySpeedIndicator.init(this, {
				  {0, sizeof(VertexOverlaySpeedIndicator), VK_VERTEX_INPUT_RATE_VERTEX}
			}, {
				{0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(VertexOverlaySpeedIndicator, pos),
						sizeof(glm::vec2), OTHER}
			});


		// Pipelines [Shader couples]--------------------------------------------------
		// The second parameter is the pointer to the vertex definition
		// Third and fourth parameters are respectively the vertex and fragment shaders
		// The last array, is a vector of pointer to the layouts of the sets that will
		// be used in this pipeline. The first element will be set 0, and so on..

		PPlanet.init(this, &VMesh, "shaders/MeshVert.spv", "shaders/PlanetFrag.spv", { &DSLPlanet });
		// the following line says if the created mesh model will be visible from the inside or not
		PPlanet.setAdvancedFeatures(VK_COMPARE_OP_LESS, VK_POLYGON_MODE_FILL,
			VK_CULL_MODE_NONE, false);

		PSun.init(this, &VMesh, "shaders/MeshVert.spv", "shaders/SunFrag.spv", { &DSLSun });
		// PSun.setAdvancedFeatures(VK_COMPARE_OP_LESS, VK_POLYGON_MODE_FILL,
		// 	VK_CULL_MODE_NONE, false);

		PSkydome.init(this, &VMesh, "shaders/MeshVert.spv", "shaders/SkydomeFrag.spv", { &DSLSkydome});
		PSkydome.setAdvancedFeatures(VK_COMPARE_OP_LESS, VK_POLYGON_MODE_FILL,
			VK_CULL_MODE_NONE, false);

		POverlay.init(this, &VOverlay, "shaders/OverlayVert.spv", "shaders/OverlayFrag.spv", { &DSLOverlay });
		POverlay.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL,
			VK_CULL_MODE_NONE, false);

		POrbits.init(this, &VOrbits, "shaders/OrbitsVert.spv", "shaders/OrbitsFrag.spv", { &DSLOrbits });
		POrbits.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL,
			VK_CULL_MODE_NONE, false);

		POverlaySpeedIndicator.init(this, &VOverlaySpeedIndicator, "shaders/OverlaySpeedIndicatorVert.spv", "shaders/OverlaySpeedIndicatorFrag.spv", { &DSLOverlaySpeedIndicator });
		POverlaySpeedIndicator.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL,
			VK_CULL_MODE_NONE, false);

		// Models, textures and Descriptors (values assigned to the uniforms)--------------------------------------------------
		// The second parameter is the pointer to the vertex definition forthis model
		// The third parameter is the file name
		createSkydome(radiusSkydome, MSkydome.vertices, MSkydome.indices);
		MSkydome.initMesh(this, &VMesh);
		TSkydome.init(this, "textures/Skydome.png");

		for(CelestialBodyVulkanData cb : celestialBodies) {
			float radius = (float)solarSystemData[cb.name]["radius"];
			createPlanetMesh(
				radius,
				(*cb.model).vertices,
				(*cb.model).indices
			);
			(*cb.model).initMesh(this, &VMesh);

			// texture
			std::string texPath = "textures/" + cb.name + ".png";
			(*cb.tex).init(this, texPath.c_str());
		}

		float radius = (float) solarSystemData["Jupiter"]["distance_from_sun"];
		radius += (float) solarSystemData["Mars"]["distance_from_sun"];
		radius /= 2.0f;
		createAsteroidsBelt(radius, MAsteroidsBelt.vertices, MAsteroidsBelt.indices);
		MAsteroidsBelt.initMesh(this, &VMesh);
		TAsteroid.init(this, "textures/Asteroid.png");

		createSaturnRing(2.0f, MSaturnRing.vertices, MSaturnRing.indices);
		MSaturnRing.initMesh(this, &VMesh);
		TSaturnRing.init(this, "textures/SaturnRing.png");

		createMouseKeyOverlayLayer(MKey.vertices, MKey.indices);
		MKey.initMesh(this, &VOverlay);
		TKey.init(this, "textures/MouseKeyOverlayLayer.png");

		// store buttonsbounds for later use
		for(VertexOverlay vertex : MKey.vertices)
			buttonsBounds.push_back(vertex.pos);

		createOrbits(solarSystemData, MOrbits.vertices, MOrbits.indices);
		MOrbits.initMesh(this, &VOrbits);

		float Uspace = -1+0.05+0.13;
		MOverlaySpeedIndicator.vertices = {
			{{-0.015, Uspace+0.015}}, // A
			{{ 0.015, Uspace+0.015}}, // B
			{{ 0   , Uspace}}
		};
		MOverlaySpeedIndicator.indices = { 0, 1, 2};
		MOverlaySpeedIndicator.initMesh(this, &VOverlaySpeedIndicator);

		for(CelestialBodyInfoVulkanData cbI : celestialBodiesInfo) {
			(*cbI.model).vertices = {
				{{-0.7f, -1.0f}, {0.0f, 0.0f}},
				{{-0.7f, 0.8f}, {0.0f,1.0f}},
				{{ 0.7f,-1.0f}, {1.0f,0.0f}},
				{{ 0.7f, 0.8f}, {1.0f,1.0f}}
			};
			(*cbI.model).indices = { 0,1,2, 1,2,3};
			(*cbI.model).initMesh(this, &VOverlay);

			// texture
			std::string texPath = "textures/" + cbI.name + "I.png";
			(*cbI.tex).init(this, texPath.c_str());
		}
	}

	// Here you create your pipelines and Descriptor Sets!
	void pipelinesAndDescriptorSetsInit() {
		// This creates a new pipeline (with the current surface), using its shaders--------------------------------------------------
		PPlanet.create();
		PSun.create();
		PSkydome.create();
		POverlay.create();
		POrbits.create();
		POverlaySpeedIndicator.create();

		// Here you define the data set--------------------------------------------------
		DSSkydome.init(this, &DSLSkydome, {
			{0, UNIFORM, sizeof(MeshUniformBlock), nullptr},
			{1, TEXTURE, 0, &TSkydome}
		});


		for(CelestialBodyVulkanData cb : celestialBodies) {
			if (cb.name.compare("Sun") == 0) {
				(*cb.DS).init(this, &DSLSun, {
					// the second parameter, is a pointer to the Uniform Set Layout of this set
					// the last parameter is an array, with one element per binding of the set.
					// first  elmenet : the binding number
					// second element : UNIFORM or TEXTURE (an enum) depending on the type
					// third  element : only forUNIFORMs, the size of the corresponding C++ object. Fortexture, just put 0
					// fourth element : only forTEXTUREs, the pointer to the corresponding texture object. Foruniforms, use nullptr
					{0, UNIFORM, sizeof(MeshUniformBlock), nullptr},
					{1, TEXTURE, 0, cb.tex},
					{2, UNIFORM, sizeof(GlobalUniformBlockPoint), nullptr},
					{3, TEXTURE, 0, cb.tex}
					});
			}
			else {
				(*cb.DS).init(this, &DSLPlanet, {
					{0, UNIFORM, sizeof(MeshUniformBlock), nullptr},
					{1, TEXTURE, 0, cb.tex},
					{2, UNIFORM, sizeof(GlobalUniformBlockPoint), nullptr}
				});
			}
		}

		DSSaturnRing.init(this, &DSLPlanet, {
			{0, UNIFORM, sizeof(MeshUniformBlock), nullptr},
			{1, TEXTURE, 0, &TSaturnRing},
			{2, UNIFORM, sizeof(GlobalUniformBlockPoint), nullptr}
		});

		DSAsteroidsBelt.init(this, &DSLPlanet, {
			{0, UNIFORM, sizeof(MeshUniformBlock), nullptr},
			{1, TEXTURE, 0, &TAsteroid},
			{2, UNIFORM, sizeof(GlobalUniformBlockPoint), nullptr}
		});

		for(CelestialBodyInfoVulkanData cbI: celestialBodiesInfo) {
			(*cbI.DS).init(this, &DSLOverlay, {
				{0, UNIFORM, sizeof(OverlayUniformBlock), nullptr},
				{1, TEXTURE, 0, cbI.tex}
			});
		}

		DSKey.init(this, &DSLOverlay, {
			{0, UNIFORM, sizeof(OverlayUniformBlock), nullptr},
			{1, TEXTURE, 0, &TKey}
		});

		DSOrbits.init(this, &DSLOrbits, {
			{0, UNIFORM, sizeof(OrbitsUniformBlock), nullptr}
		});

		DSOverlaySpeedIndicator.init(this, &DSLOverlaySpeedIndicator, {
			{0, UNIFORM, sizeof(OverlaySpeedIndicatorUniformBlock), nullptr}
		});
	}

	// Here you destroy your pipelines and Descriptor Sets!
	// All the object classes defined in Starter.hpp have a method .cleanup() forthis purpose
	void pipelinesAndDescriptorSetsCleanup() {
		// Cleanup pipelines--------------------------------------------------
		PPlanet.cleanup();
		PSun.cleanup();
		PSkydome.cleanup();
		POverlay.cleanup();
		POrbits.cleanup();
		POverlaySpeedIndicator.cleanup();

		// Cleanup datasets--------------------------------------------------
		for(CelestialBodyVulkanData cb : celestialBodies)
			(*cb.DS).cleanup();
		for(CelestialBodyInfoVulkanData cbI : celestialBodiesInfo)
			(*cbI.DS).cleanup();

		DSSaturnRing.cleanup();
		DSAsteroidsBelt.cleanup();
		DSKey.cleanup();
		DSOverlaySpeedIndicator.cleanup();
		DSOrbits.cleanup();
		DSSkydome.cleanup();
	}

	// Here you destroy all the Models, Texture and Desc. Set Layouts you created!
	// All the object classes defined in Starter.hpp have a method .cleanup() forthis purpose
	// You also have to destroy the pipelines: since they need to be rebuilt, they have two different
	// methods: .cleanup() recreates them, while .destroy() delete them completely
	void localCleanup() {
		// Cleanup textures--------------------------------------------------
		for(CelestialBodyVulkanData cb : celestialBodies)
			(*cb.tex).cleanup();
		for(CelestialBodyInfoVulkanData cbI : celestialBodiesInfo)
			(*cbI.tex).cleanup();

		TSaturnRing.cleanup();
		TAsteroid.cleanup();
		TSkydome.cleanup();
		TKey.cleanup();

		// Cleanup models--------------------------------------------------
		for(CelestialBodyVulkanData cb : celestialBodies)
			(*cb.model).cleanup();
		for(CelestialBodyInfoVulkanData cbI : celestialBodiesInfo)
			(*cbI.model).cleanup();

		MAsteroidsBelt.cleanup();
		MSaturnRing.cleanup();
		MSkydome.cleanup();
		MKey.cleanup();
		MOrbits.cleanup();
		MOverlaySpeedIndicator.cleanup();

		// Cleanup descriptor set layouts--------------------------------------------------
		DSLPlanet.cleanup();
		DSLSun.cleanup();
		DSLSkydome.cleanup();
		DSLOverlay.cleanup();
		DSLOrbits.cleanup();
		DSLOverlaySpeedIndicator.cleanup();

		// Destroys the pipelines--------------------------------------------------
		PPlanet.destroy();
		PSun.destroy();
		PSkydome.destroy();
		POverlay.destroy();
		POrbits.destroy();
		POverlaySpeedIndicator.destroy();
	}

	// Here it is the creation of the command buffer:
	// You send to the GPU all the objects you want to draw,
	// with their buffers and textures

	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
		// binds the pipeline
		//--------------------------------------------------
		PPlanet.bind(commandBuffer);

		for(CelestialBodyVulkanData cb : celestialBodies) {
			if (cb.name.compare("Sun") != 0) {
				(*cb.model).bind(commandBuffer);
				(*cb.DS).bind(commandBuffer, PPlanet, 0, currentImage);
				vkCmdDrawIndexed(commandBuffer,
					static_cast<uint32_t>((*cb.model).indices.size()), 1, 0, 0, 0);
			}
		}

		MSaturnRing.bind(commandBuffer);
		DSSaturnRing.bind(commandBuffer, PPlanet, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MSaturnRing.indices.size()), 1, 0, 0, 0);

		MAsteroidsBelt.bind(commandBuffer);
		DSAsteroidsBelt.bind(commandBuffer, PPlanet, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MAsteroidsBelt.indices.size()), 1, 0, 0, 0);

		//--------------------------------------------------
		PSun.bind(commandBuffer);
		CelestialBodyVulkanData cb = celestialBodies[0];
		(*cb.model).bind(commandBuffer);
		(*cb.DS).bind(commandBuffer, PSun, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>((*cb.model).indices.size()), 1, 0, 0, 0);

		//--------------------------------------------------
		PSkydome.bind(commandBuffer);
		MSkydome.bind(commandBuffer);
		DSSkydome.bind(commandBuffer, PSkydome, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MSkydome.indices.size()), 1, 0, 0, 0);

		//--------------------------------------------------
		POrbits.bind(commandBuffer);
		MOrbits.bind(commandBuffer);
		DSOrbits.bind(commandBuffer, POrbits, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MOrbits.indices.size()), 1, 0, 0, 0);

		//--------------------------------------------------
		POverlaySpeedIndicator.bind(commandBuffer);
		MOverlaySpeedIndicator.bind(commandBuffer);
		DSOverlaySpeedIndicator.bind(commandBuffer, POverlaySpeedIndicator, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MOverlaySpeedIndicator.indices.size()), 1, 0, 0, 0);

		//--------------------------------------------------
		POverlay.bind(commandBuffer);
		MKey.bind(commandBuffer);
		DSKey.bind(commandBuffer, POverlay, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MKey.indices.size()), 1, 0, 0, 0);

		for(CelestialBodyInfoVulkanData cbI : celestialBodiesInfo) {
			(*cbI.model).bind(commandBuffer);
			(*cbI.DS).bind(commandBuffer, POverlay, 0, currentImage);
			vkCmdDrawIndexed(commandBuffer,
				static_cast<uint32_t>((*cbI.model).indices.size()), 1, 0, 0, 0);
		}
	}

	// Here is where you update the uniforms.
	// Very likely this will be where you will be writing the logic of your application.
	void updateUniformBuffer(uint32_t currentImage) {
		// Integration with the timers and the controllers--------------------------------------------------
		float deltaT;
		glm::vec3 m = glm::vec3(0.0f), r = glm::vec3(0.0f);
		bool fire = false;
		glm::mat4 World;
		getSixAxis(deltaT, m, r, fire, buttonsBounds);
		// getSixAxis() is defined in Starter.hpp in the base class.
		// It fills the float point variable passed in its first parameter with the time
		// since the last call to the procedure.
		// It fills vec3 in the second parameters, with three values in the -1,1 range corresponding
		// to motion (with left stick of the gamepad, or ASWD + RF keys on the keyboard)
		// It fills vec3 in the third parameters, with three values in the -1,1 range corresponding
		// to motion (with right stick of the gamepad, or Arrow keys + QE keys on the keyboard, or mouse)
		// If fills the last boolean variable with true if fire has been pressed:
		//          SPACE on the keyboard, A or B button on the Gamepad, Right mouse button

		// static float debounce = false;
		// static int curDebounce = 0;

		std::vector<glm::mat4> matrices = updateCamPos(deltaT, m, r);
		glm::mat4 View = matrices[0];
		glm::mat4 Prj = matrices[1];

		// Point light--------------------------------------------------
		pgubo.lightPos = glm::vec3(0, 0, 0); // position of the sun
		pgubo.lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		pgubo.AmbLightColor = glm::vec3(0.1f);
		pgubo.eyePos = camPos;

		// the .map() method of a DataSet object, requires the current image of the swap chain as first parameter
		// the second parameter is the pointer to the C++ data structure to transfer to the GPU
		// the third parameter is its size
		// the fourth parameter is the location inside the descriptor set of this uniform block

		// update Skydome uniforms--------------------------------------------------
		World = glm::mat4(1);
		World = glm::translate(World, camPos); // skydome follows camPos
		// World = glm::rotate(World, camAlpha, glm::vec3(0,1,0)); // skydome does not rotate
		// World = glm::rotate(World, camBeta, glm::vec3(1,0,0)); // skydome does not rotate

		uboSkydome.amb = 1.0f;
		uboSkydome.gamma = 180.0f;
		uboSkydome.sColor = glm::vec3(0.3f);
		uboSkydome.mMat = World;
		uboSkydome.nMat = glm::inverse(glm::transpose(World));
		uboSkydome.mvpMat = Prj * View * World;// translate(World, camPos);
		DSSkydome.map(currentImage, &uboSkydome, sizeof(uboSkydome), 0);

		// update Planets uniforms--------------------------------------------------
		if(isMoving == 1) {	
			for(CelestialBodyVulkanData cb : celestialBodies) {
				if(cb.rev == NULL || cb.rot == NULL) continue;

				// solve overflow problem

				// orbital period in years
				float revSpeed = 1.0f / (float) solarSystemData[cb.name]["revolution_period"];
				revSpeed *= speedMultiplier;
				(*cb.rev) += revSpeed * deltaT;

				// rotation period in days
				float rotSpeed = 1.0f / (float) solarSystemData[cb.name]["rotation_period"];
				rotSpeed *= 365;
				rotSpeed *= speedMultiplier;
				(*cb.rot) += rotSpeed * deltaT;
			}
		}

		for(CelestialBodyVulkanData cb : celestialBodies) {
			World = glm::mat4(1);

			// Ecliptic inclination
			float eclipticInclination = (float)solarSystemData[cb.name]["ecliptic_inclination"];
			World = glm::rotate(World, glm::radians(eclipticInclination), glm::vec3(0, 0, 1));

			// Revolution
			if(cb.name.compare("Sun") != 0 && cb.rev != NULL) {
				World = glm::rotate(World, (*cb.rev), glm::vec3(0, 1, 0));
			}

			// Distance from sun
			float distanceFromSun = (float) solarSystemData[cb.name]["distance_from_sun"];
			World = glm::translate(World, glm::vec3(distanceFromSun, 0, 0)); // distance from sun

			// Axial tilt
			float axialTilt = (float)solarSystemData[cb.name]["axial_tilt"];
			World = glm::rotate(World, glm::radians(axialTilt), glm::vec3(0, 0, 1)); // planets axis tilt

			// Rotation
			if(cb.rot != NULL) {
				World = glm::rotate(World, (*cb.rot), glm::vec3(0, 1, 0)); // planets rotation
			}

			(*cb.ubo).amb = 1.0f;
			(*cb.ubo).gamma = 180.0f;
			(*cb.ubo).sColor = glm::vec3(1.0f);
			(*cb.ubo).mvpMat = Prj * View * World;
			(*cb.ubo).mMat = World;
			(*cb.ubo).nMat = glm::inverse(glm::transpose(World));
			(*cb.DS).map(currentImage, cb.ubo, sizeof((*cb.ubo)), 0);
			(*cb.DS).map(currentImage, &pgubo, sizeof(pgubo), 2);

			if (cb.name.compare("Saturn") == 0) { // saturn ring case
				uboSaturnRing.amb = 1.0f;
				uboSaturnRing.gamma = 180.0f;
				uboSaturnRing.sColor = glm::vec3(1.0f);
				uboSaturnRing.mvpMat = Prj * View * World;
				uboSaturnRing.mMat = World;
				uboSaturnRing.nMat = glm::inverse(glm::transpose(World));
				DSSaturnRing.map(currentImage, &uboSaturnRing, sizeof(uboSaturnRing), 0);
				DSSaturnRing.map(currentImage, &pgubo, sizeof(pgubo), 2);
			}
		}

		// Asteroid belt--------------------------------------------------
		MeshUniformBlock ubo{};

		World = glm::mat4(1);
		// asteroid rotation = slightly faster than jupiter
		CelestialBodyVulkanData cbJupiter = celestialBodies[5];
		World = glm::rotate(World, (*cbJupiter.rev)*1.1f, glm::vec3(0, 1, 0));

		ubo.amb = 1.0f;
		ubo.gamma = 180.0f;
		ubo.sColor = glm::vec3(1.0f);
		ubo.mvpMat = Prj * View * World;
		ubo.mMat = World;
		ubo.nMat = glm::inverse(glm::transpose(World));
		DSAsteroidsBelt.map(currentImage, &ubo, sizeof(ubo), 0);
		DSAsteroidsBelt.map(currentImage, &pgubo, sizeof(pgubo), 2);

		// Orbits--------------------------------------------------
		World = glm::mat4(1);
		uboOrbits.mvpMat = Prj *  View * World;
		uboOrbits.mMat = World;
		uboOrbits.nMat = glm::inverse(glm::transpose(World));
		uboOrbits.visible = orbitsVisibility;
		DSOrbits.map(currentImage, &uboOrbits, sizeof(uboOrbits), 0);

		// OverlaySpeedIndicator--------------------------------------------------
		// uboOverlaySpeedIndicator.x = 0;
		DSOverlaySpeedIndicator.map(currentImage,
			&uboOverlaySpeedIndicator,
			sizeof(uboOverlaySpeedIndicator), 0);

		// Key splash
		uboKey.visible = 1;
		DSKey.map(currentImage, &uboKey, sizeof(uboKey), 0);

		// Mouse events--------------------------------------------------
		gameLogic(currentImage, fire, View, Prj, deltaT);
	}

	void gameLogic(uint32_t currentImage, bool fire, glm::mat4 View, glm::mat4 Prj, float deltaT) {
		if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
			double mouseX, mouseY;
			glfwGetCursorPos(window, &mouseX, &mouseY);

			// Convert pixel coordinates to normalized screen coordinates
			int screenWidth, screenHeight;
			glfwGetWindowSize(window, &screenWidth, &screenHeight);
			float nscX = mouseX*2 / (screenWidth-1) - 1;
			float nscY = mouseY*2 / (screenHeight-1) - 1;

			// Each group of 4 vertices in MKey.vertices represent a button 
			// ordered in A, B, C, D top left clockwise
			for(int i = 0; i < MKey.vertices.size(); i += 4) {
				// check if the normalized screen coordinate inside a button
				float xL = MKey.vertices[i+0].pos[0]; // x of A
				float xR = MKey.vertices[i+1].pos[0]; // x of B
				float yU = MKey.vertices[i+0].pos[1]; // y of A
				float yD = MKey.vertices[i+2].pos[1]; // y of C

				if(
					nscX >= xL &&
					nscX <= xR &&
					nscY <= yD && // since y down
					nscY >= yU
				) {
					clickAction = (int) i/4;
					// RebuildPipeline();
					break;
				}
			}
		}

		if(clickAction < HIDE_INFO) {
			for(int i = 0; i < celestialBodiesInfo.size(); i++) {
				CelestialBodyInfoVulkanData cbI = celestialBodiesInfo[i];
				(*cbI.ubo).visible = 0;
				if(i == clickAction)
					(*cbI.ubo).visible = 1;
				(*cbI.DS).map(currentImage, cbI.ubo, sizeof((*cbI.ubo)), 0);
			}
		}
		else if(clickAction == HIDE_INFO) { //close all			
			for(int i = 0; i < celestialBodiesInfo.size(); i++) {
				CelestialBodyInfoVulkanData cbI = celestialBodiesInfo[i];
				(*cbI.ubo).visible = 0;
				(*cbI.DS).map(currentImage, cbI.ubo, sizeof((*cbI.ubo)), 0);
			}
		}
		else if(clickAction == START_MOVING) isMoving = 1; //move
		else if(clickAction == STOP_MOVING) isMoving = 0; //stop
		else if(clickAction == SHOW_ORBITS) orbitsVisibility = 1;
		else if(clickAction == HIDE_ORBITS) orbitsVisibility = 0;
		else if(clickAction == SPEED_TRACKBAR_L || clickAction == SPEED_TRACKBAR_R) {
			// changing the speed
			double mouseX, mouseY;
			glfwGetCursorPos(window, &mouseX, &mouseY);

			// Convert pixel coordinates to normalized screen coordinates
			int screenWidth, screenHeight;
			glfwGetWindowSize(window, &screenWidth, &screenHeight);
			float nscX = mouseX*2 / (screenWidth-1) - 1;
			// float nscY = mouseY*2 / (screenHeight-1) - 1;

			float xL = MKey.vertices[SPEED_TRACKBAR_L*4].pos[0];
			float xR = MKey.vertices[SPEED_TRACKBAR_R*4+1].pos[0];

			int numV = velocities.size();
			float tick = (xR-xL)/numV;
			
			for(int i = 0; i < numV; i++) {
				float Lbound = tick*i+xL;
				float Rbound = tick*(i+1)+xL;
				if(nscX >= Lbound && nscX <= Rbound) {
					speedMultiplier = velocities[i];

					uboOverlaySpeedIndicator.x = (Rbound-Lbound)/2+Lbound;
					DSOverlaySpeedIndicator.map(currentImage,
						&(uboOverlaySpeedIndicator),
						sizeof(uboOverlaySpeedIndicator), 0);

					break;
				}
			}
			clickAction = HIDE_INFO;
		}
	}

	std::vector<glm::mat4> updateCamPos(float deltaT, glm::vec3 m, glm::vec3 r) {
		camAlpha = camAlpha - rotSpeed * deltaT * r.y;
		camBeta = camBeta - rotSpeed * deltaT * r.x;
		camBeta = camBeta < glm::radians(-90.0f) ? glm::radians(-90.0f) :
			(camBeta > glm::radians(90.0f) ? glm::radians(90.0f) : camBeta);
		// camRho = camRho - rotSpeed * deltaT * r.z;
		// camRho = camRho < glm::radians(-180.0f) ? glm::radians(-180.0f) :
		// 	(camRho > glm::radians(180.0f) ? glm::radians(180.0f) : camRho);

		glm::vec3 ux = glm::rotate(glm::mat4(1.0f), camAlpha, glm::vec3(0,1,0)) * glm::vec4(1,0,0,1);
		glm::vec3 uz = glm::rotate(glm::mat4(1.0f), camAlpha, glm::vec3(0,1,0)) * glm::vec4(0,0,-1,1);

		glm::vec3 oldPos = camPos;

		camPos = camPos + movSpeed * m.x * ux * deltaT;
		camPos = camPos + movSpeed * m.y * glm::vec3(0, 1, 0) * deltaT;
		camPos = camPos + movSpeed * m.z * uz * deltaT;

		cameraIsColliding = 0;

		// check if collision with a planet
		for(CelestialBodyVulkanData cb : celestialBodies) {
			float cbRadius = (float) solarSystemData[cb.name]["radius"];

			// get position after rotating and translating due to revolution
			glm::vec3 cbPos = glm::vec3((*cb.ubo).mMat * glm::vec4(0,0,0,1));

			cameraIsColliding = glm::length(camPos-cbPos) <= cbRadius*1.2;
			if(cameraIsColliding) {
				camPos = oldPos;
				break;
			}
		}

		// check if collision with skydome
		float distanceFromOrigin = glm::length(camPos-glm::vec3(0.0f));
		if(distanceFromOrigin >= radiusSkydome*0.9 || cameraIsColliding) {
			camPos = oldPos;
		}
		

		glm::mat4 Prj = glm::perspective(FOVy, Ar, nearPlane, farPlane);
		Prj[1][1] *= -1;

		glm::mat4 View = glm::mat4(1);
		glm::vec3 oppPos = camPos;
		oppPos[0] *= -1;
		oppPos[1] *= -1;
		oppPos[2] *= -1;
		View = glm::rotate(glm::mat4(1.0), -camBeta, glm::vec3(1,0,0)) *
				glm::rotate(glm::mat4(1.0), -camAlpha, glm::vec3(0,1,0)) *
				// glm::rotate(glm::mat4(1.0), -CamRho, glm::vec3(0,0,1)) *
				glm::translate(glm::mat4(1.0), -camPos);
		
		return { View, Prj };
	}

	void createMouseKeyOverlayLayer(std::vector<VertexOverlay>& vDef, std::vector<uint32_t>& vIdx);

	void createPlanetMesh(float radius, std::vector<VertexMesh>& vDef, std::vector<uint32_t>& vIdx);

	void createSaturnRing(float radius, std::vector<VertexMesh>& vDef, std::vector<uint32_t>& vIdx);

	void createSkydome(float radius, std::vector<VertexMesh>& vDef, std::vector<uint32_t>& vIdx);
	
	void createAsteroidsBelt(float radius, std::vector<VertexMesh>& vDef, std::vector<uint32_t>& vIdx);

	void createOrbits(nlohmann::json solarSystemData, std::vector<VertexOrbit>& vDef, std::vector<uint32_t>& vIdx);
};

#include "planetCreate.hpp"

// This is the main: probably you do not need to touch this!
int main() {
	SolarSystem app;

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}