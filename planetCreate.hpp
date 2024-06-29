void SolarSystem::createMouseKeyOverlayLayer(std::vector<VertexOverlay>& vDef, std::vector<uint32_t>& vIdx) {
    // d = planet diameter
    // SP is space for the 9 planets: numPlanets*d*1+numPlanets*d*0.2 = numPlanets*d*1.2
    // SLR times 2 is LR space: d*1
    // 2 = SB+2*SLR
    // 2 = d*(numPlanets*1.2+2)
    int numPlanets = 10;
    int numButtons = 4;
    float diameter = 2.0f / (numPlanets*1.2f+2);
    float SLR = 1*diameter;

    int indices = 0;

    // spaces don't need to be added, adding planets
    float Lspace;
    float Dspace = 0.2*diameter;
    for(int j = 0; j < 2; j++) {
        float upV = j*0.2;
        float downV = (j+1)*0.2;
        for(int i = 0; i < 5; i++) {
            Lspace = -1.0+SLR+i*diameter*1.2+j*(diameter*5*1.2);
            vDef.push_back({{Lspace         , 1-diameter-Dspace},{i*0.2    , upV}}); // A
            vDef.push_back({{Lspace+diameter, 1-diameter-Dspace},{(i+1)*0.2, upV}}); // B
            vDef.push_back({{Lspace+diameter, 1-Dspace         },{(i+1)*0.2, downV}}); // C
            vDef.push_back({{Lspace         , 1-Dspace         },{i*0.2    , downV}}); // D
            indices += 4;
            if(indices >= numPlanets*4) break;
        }
	}

    // adding buttons
    Lspace = 1-0.95*diameter;
    float Uspace;
    for(int i = 0; i < numButtons; i++) {
        Uspace = 0.2*diameter+diameter*0.95*i;
        vDef.push_back({{Lspace              , -1+Uspace              },{i*0.2    , 0.4}}); // A
        vDef.push_back({{Lspace+diameter*0.75, -1+Uspace              },{(i+1)*0.2, 0.4}}); // B
        vDef.push_back({{Lspace+diameter*0.75, -1+diameter*0.75+Uspace},{(i+1)*0.2, 0.6}}); // C
        vDef.push_back({{Lspace              , -1+diameter*0.75+Uspace},{i*0.2    , 0.6}}); // D
        indices += 4;
    }

    // adding speed trackbar
    float trackbarLength = 1.7;
    float trackbarHeight = 0.13;
    SLR = 1-trackbarLength/2;
    Uspace = -1+0.05;

    vDef.push_back({{-1+SLR, Uspace               },{0, 0.6}}); // A
    vDef.push_back({{     0, Uspace               },{1, 0.6}}); // B
    vDef.push_back({{     0, Uspace+trackbarHeight},{1, 0.8}}); // C
    vDef.push_back({{-1+SLR, Uspace+trackbarHeight},{0, 0.8}}); // D
    indices += 4;

    vDef.push_back({{     0, Uspace               },{0, 0.8}}); // A
    vDef.push_back({{ 1-SLR, Uspace               },{1, 0.8}}); // B
    vDef.push_back({{ 1-SLR, Uspace+trackbarHeight},{1, 1.0}}); // C
    vDef.push_back({{     0, Uspace+trackbarHeight},{0, 1.0}}); // D
    indices += 4;

    indices += 4;

    for(int i = 0; i < indices; i+=4) {
		vIdx.push_back((i+0) % indices);
		vIdx.push_back((i+1) % indices);
		vIdx.push_back((i+2) % indices);

		vIdx.push_back((i+2) % indices);
		vIdx.push_back((i+3) % indices);
		vIdx.push_back((i+0) % indices);
    }
}

void SolarSystem::createPlanetMesh(float radius, std::vector<VertexMesh>& vDef, std::vector<uint32_t>& vIdx) {
    const int numLatitudes = 30;   // Number of latitude divisions
    const int numLongitudes = 30;  // Number of longitude divisions

    // Generate vertices for the sphere
    for (int lat = 0; lat <= numLatitudes; ++lat) {
        float theta = static_cast<float>(lat) * M_PI / static_cast<float>(numLatitudes); // Latitude angle
        for (int lon = 0; lon <= numLongitudes; ++lon) {
            float phi = static_cast<float>(lon) * 2.0f * M_PI / static_cast<float>(numLongitudes); // Longitude angle

            // Convert spherical coordinates to Cartesian coordinates
            float x = -std::cos(phi) * std::sin(theta);
            float y = std::cos(theta);
            float z = std::sin(phi) * std::sin(theta);

            // Vertex position
            glm::vec3 position(x * radius, y * radius, z * radius);
            // Normal vector is simply the vertex position normalized, as it points from the origin to the surface
            glm::vec3 normal = glm::normalize(position);
            // Texture coordinates based on latitude and longitude
            float u = static_cast<float>(lon) / static_cast<float>(numLongitudes);
            float v = static_cast<float>(lat) / static_cast<float>(numLatitudes);

            // Add the vertex to the vertex definition array
            vDef.push_back({ position, normal, { u, v } });
        }
    }

    // Generate indices for the sphere triangles
    for (int lat = 0; lat < numLatitudes; ++lat) {
        for (int lon = 0; lon < numLongitudes; ++lon) {
            int index0 = lat * (numLongitudes + 1) + lon;
            int index1 = index0 + 1;
            int index2 = (lat + 1) * (numLongitudes + 1) + lon;
            int index3 = index2 + 1;

            // First triangle
            vIdx.push_back(index0);
            vIdx.push_back(index2);
            vIdx.push_back(index1);

            // Second triangle
            vIdx.push_back(index1);
            vIdx.push_back(index2);
            vIdx.push_back(index3);
        }
    }
}

void SolarSystem::createSaturnRing(float radius, std::vector<VertexMesh>& vDef, std::vector<uint32_t>& vIdx) {
    /*
    A_____________B
    |             | BC <-> outer ring
    D-------------C

    UV coor:
	0--->1
	|
	v
	1

    */
   
    const int numSlices = 100;

    for (int i = 0; i < numSlices; i++) {
        float theta = i*2*M_PI/numSlices;

        float x_rel = std::cos(theta)*radius*1.2;
        float z_rel = std::sin(theta)*radius*1.2;

        vDef.push_back({{x_rel    , 0, z_rel    }, {0,1,0}, {0, 0}}); //A
		vDef.push_back({{x_rel*2.0, 0, z_rel*2.0}, {0,1,0}, {1, 0}}); //B
		vDef.push_back({{x_rel*2.0, 0, z_rel*2.0}, {0,1,0}, {1, 1}}); //C
		vDef.push_back({{x_rel    , 0, z_rel    }, {0,1,0}, {0, 1}}); //D
    }

	for(int i = 0; i < numSlices*4; i++) {
		vIdx.push_back((i+0) % (numSlices*4));
		vIdx.push_back((i+1) % (numSlices*4));
		vIdx.push_back((i+2) % (numSlices*4));

		vIdx.push_back((i+2) % (numSlices*4));
		vIdx.push_back((i+3) % (numSlices*4));
		vIdx.push_back((i+0) % (numSlices*4));
	}
}

void SolarSystem::createSkydome(float radius, std::vector<VertexMesh>& vDef, std::vector<uint32_t>& vIdx) {
    const int numLatitudes = 20;   // Number of latitude divisions
    const int numLongitudes = 20;  // Number of longitude divisions

    // Generate vertices for the sphere
    for (int lat = 0; lat <= numLatitudes; ++lat) {
        float theta = static_cast<float>(lat) * M_PI / static_cast<float>(numLatitudes); // Latitude angle
        for (int lon = 0; lon <= numLongitudes; ++lon) {
            float phi = static_cast<float>(lon) * 2.0f * M_PI / static_cast<float>(numLongitudes); // Longitude angle

            // Convert spherical coordinates to Cartesian coordinates
            float x = std::cos(phi) * std::sin(theta);
            float y = std::cos(theta);
            float z = std::sin(phi) * std::sin(theta);

            // Vertex position
            glm::vec3 position(x * radius, y * radius, z * radius);
            // Normal vector is simply the vertex position normalized, as it points from the origin to the surface
            glm::vec3 normal = glm::normalize(position);
            // Texture coordinates based on latitude and longitude
            float u = static_cast<float>(lon) / static_cast<float>(numLongitudes);
            float v = static_cast<float>(lat) / static_cast<float>(numLatitudes);

            // Add the vertex to the vertex definition array
            vDef.push_back({ position, normal, { u, v } });
        }
    }

    // Generate indices for the sphere triangles
    for (int lat = 0; lat < numLatitudes; ++lat) {
        for (int lon = 0; lon < numLongitudes; ++lon) {
            int index0 = lat * (numLongitudes + 1) + lon;
            int index1 = index0 + 1;
            int index2 = (lat + 1) * (numLongitudes + 1) + lon;
            int index3 = index2 + 1;

            // First triangle
            vIdx.push_back(index0);
            vIdx.push_back(index2);
            vIdx.push_back(index1);

            // Second triangle
            vIdx.push_back(index1);
            vIdx.push_back(index2);
            vIdx.push_back(index3);
        }
    }
}

void SolarSystem::createAsteroidsBelt(float radius, std::vector<VertexMesh>& vDef, std::vector<uint32_t>& vIdx) {
    Model<VertexMesh> MAsteroid;
    
    MAsteroid.init(this, &VMesh, "models/Asteroid.mgcg", MGCG);

    const int numAsteroids = 400; 

    for (int j = 0; j < numAsteroids; j++) {
        float theta = j*2*M_PI/numAsteroids;

        float x_rel = std::cos(theta)*radius;
        float y_rel = 0;
        float z_rel = std::sin(theta)*radius;

        // add random tilt to position
        x_rel += ((float)rand() / (float)RAND_MAX) * 8 - 4; // random in range  [-4,+4]
        y_rel += ((float)rand() / (float)RAND_MAX) * 2 - 1; // random in range  [-1,+1]
        z_rel += ((float)rand() / (float)RAND_MAX) * 8 - 4; // random in range  [-4,+4]

        // add random rotation
        float rotAngle =  ((float)rand() / (float)RAND_MAX) * M_PI * 2;
        glm::mat4 R = glm::mat4(1);
        R = glm::rotate(R, rotAngle, glm::vec3(1,0,0));
        R = glm::rotate(R, rotAngle, glm::vec3(0,1,0));
        R = glm::rotate(R, rotAngle, glm::vec3(0,0,1));

        // add random scaling
        float scalingFactor = ((float)rand() / (float)RAND_MAX) * 10 + 5; // random in range [5,15]
        
        //scalingFactor /= 100000/10;

        for(VertexMesh v : MAsteroid.vertices) {
            VertexMesh vToPush;
            vToPush.norm = v.norm;
            vToPush.pos = v.pos;
            vToPush.UV = v.UV;

            // scaling
            vToPush.pos *= scalingFactor;
            
            // rotate with random angle, since to traslation in rotation I can use this method
            // instead of glm::vec3(R * glm::vec4(vToPush.pos,1)); 
            vToPush.pos = glm::mat3(R) * vToPush.pos;

            // traslating
            vToPush.pos[0] += x_rel;
            vToPush.pos[1] += y_rel;
            vToPush.pos[2] += z_rel;

            // transforming the normal, to have correct shading
            vToPush.norm = glm::mat3(R) * vToPush.norm;

            vDef.push_back(vToPush);
        }

        for(uint32_t i : MAsteroid.indices) {
            vIdx.push_back(i+j*MAsteroid.vertices.size());
        }
    }

    MAsteroid.cleanup();
}

void SolarSystem::createOrbits(nlohmann::json solarSystemData, std::vector<VertexOrbit>& vDef, std::vector<uint32_t>& vIdx) {
    const int numLatitudes = 5;   // Number of latitude divisions
    const int numLongitudes = 120;  // Number of longitude divisions

    float radius = 0.015;
    
    std::vector<std::string> cbNames = {
        "Mercury", "Venus", "Earth", "Mars",
        "Jupiter", "Saturn", "Uranus", "Neptune"
        // "Mercury", "Neptune"
    };

    for(std::string coName : cbNames) {
        float Lspace = (float)solarSystemData[coName]["distance_from_sun"];
        glm::mat4 inclinationMat = glm::rotate(
            glm::mat4(1),
            glm::radians((float)solarSystemData[coName]["ecliptic_inclination"]),
            glm::vec3(0, 0, 1)
        );
        for(int i = 0; i < numLatitudes; i++) {
            float theta = i*2*M_PI/numLatitudes;
            for(int j = 0; j < numLongitudes; j++) {
                float phi = j*2*M_PI/numLongitudes;
                float x = (Lspace+std::cos(theta)*radius)*std::cos(phi);
                float y = std::sin(theta)*radius;
                float z = (Lspace+radius*std::cos(theta))*std::sin(phi);

                glm::vec3 P = glm::vec3(inclinationMat*glm::vec4(x, y, z, 1));

                vDef.push_back({ P });
            }
        }
    }

    int numVertices = vDef.size();
    int numVerticesPerOrbit = numLatitudes*numLongitudes;
    
    for(int cbN = 0; cbN < cbNames.size(); cbN++) {
        int startIndex = cbN*numLatitudes*numLongitudes;
        int endIndex = (cbN+1)*numLatitudes*numLongitudes;
        for(int k = 0; k < numLatitudes; k++) {
            for(int j = 0; j < numLongitudes; j++) {
                int A0 = startIndex + (j+0) % numLongitudes + numLongitudes*(k);
                int A1 = startIndex + (j+1) % numLongitudes + numLongitudes*(k);
                int B1 = startIndex + (j+1) % numLongitudes + numLongitudes*(k+1);
                int B0 = startIndex + (j+0) % numLongitudes + numLongitudes*(k+1);
                std::vector<int> indices = {
                    A0 % numVertices,
                    A1 % numVertices,
                    B1 % numVertices,
                    B1 % numVertices,
                    B0 % numVertices,
                    A0 % numVertices
                };
                
                for(int index : indices)
                    if(index >= startIndex && index < endIndex)
                        vIdx.push_back(index);
            }
        }
    }
}

