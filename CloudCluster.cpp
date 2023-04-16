#include "CloudCluster.h"

CloudCluster::CloudCluster(std::vector<std::shared_ptr<EmitterController>> emittersInput, CloudType typeInput, unsigned int textureArray, unsigned int texArraySizeInput)
{
	// Setup type
	emitters = emittersInput;
	textureArrayID = textureArray;
	texArraySize = texArraySizeInput;
	type = typeInput;
	cameraDistance = 0.0f;
	merged = false;
	remove = false;
	buffersRunning = false;


	updateRadius();

	updateCenter();

	// setup particle vector and position vector (used for buffer) 

	for (std::shared_ptr<EmitterController> e : emitters) // sets up cluster positions
	{
		glm::vec3 movementPos = (getSpherePos() * clusterRadius) + clusterCenter; // move particle system to random point in cluster (Need to add animation) 

		e->emitter->emitterPos = movementPos;

		//clusterParticles.insert(clusterParticles.end(), e.emitter->particles.begin(), e.emitter->particles.end());
		for (int i = 0; i < e->emitter->particles.size(); i++)
		{
			clusterParticles.push_back(&e->emitter->particles[i]);
		}
	}

	for (int i = 0; i < this->clusterParticles.size(); ++i)
	{
		clusterPositions.push_back(this->clusterParticles[i]->position[0]);
		clusterPositions.push_back(this->clusterParticles[i]->position[1]);
		clusterPositions.push_back(this->clusterParticles[i]->position[2]);
		clusterPositions.push_back(this->clusterParticles[i]->aliveTime);
	}

	startingEmitterNum = emitters.size(); // set starting particle amount
	startingParticleNum = clusterParticles.size();
}

void CloudCluster::setupBuffers(unsigned int skySize)
{

	// create a vertex and position buffer
	glGenVertexArrays(1, &vertexBuffer);
	glGenBuffers(1, &vertexBufferVBO);

	//glGenVertexArrays(1, &positionBuffer);
	glGenBuffers(1, &positionBuffer);

	// fill the vertex buffer

	float vertices[] = {
		// positions         // texture Coords 
		0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
		0.0f, -0.5f,  0.0f,  0.0f,  1.0f,
		1.0f, -0.5f,  0.0f,  1.0f,  1.0f,

		0.0f, 0.5f, 0.0f, 0.0f, 0.0f,
		1.0f, -0.5f, 0.0f, 1.0f, 1.0f,
		1.0f, 0.5f, 0.0f, 1.0f, 0.0f
	};

	glBindVertexArray(vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, this->vertexBufferVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);

	// fill the position buffer

	glBindBuffer(GL_ARRAY_BUFFER, this->positionBuffer);
	glBufferData(GL_ARRAY_BUFFER, skySize * 4 * sizeof(float), nullptr, GL_DYNAMIC_DRAW); // intitalize buffer as far larger than needed so it has space for merging 
	glBufferSubData(GL_ARRAY_BUFFER, 0, this->clusterParticles.size() * 4 * sizeof(float), clusterPositions.data());

	// shader
	this->shader = new Shader("shaders/part.vert", "shaders/part.frag");

	buffersRunning = true;

}

void CloudCluster::updateCenter()
{
	// calculate central cluster point
	glm::vec3 clusterSum = glm::vec3(0.0f, 0.0f, 0.0f);
	for (std::shared_ptr<EmitterController> e : emitters)
	{
		clusterSum += e->emitter->emitterPos;
	}
	clusterCenter = glm::vec3(clusterSum.x / emitters.size(), clusterSum.y / emitters.size(), clusterSum.z / emitters.size());
}

void CloudCluster::updateRadius()
{
	if (type == Cirrus)
	{
		//clusterRadius =  (2 * emittersInput.size()) / 3;
		clusterRadius = emitters.size() / (2 + (emitters.size() / 15));
	}
	else if (type == Cumulus)
	{
		clusterRadius = emitters.size() / (2 + (emitters.size() / 15)); // try 17.5 or 20
	}
	else if (type == Stratus)
	{
		clusterRadius = emitters.size() / (2 + (emitters.size() / 20));
	}
}

void CloudCluster::clusterUpdate(const float dt, const double curTime, Camera camera, glm::vec3 movement)
{
	//clusterParticles.clear();
	clusterPositions.clear();

	for (std::shared_ptr<EmitterController> e : emitters)
	{
		glm::vec3 distanceVector = clusterCenter - e->emitter->emitterPos;
		float clusterDistance = glm::sqrt(glm::dot(distanceVector, distanceVector));

		e->emitter->emitterPos += (movement * dt); // applies movement vector

		// update particle data 
		e->emitter->update(dt, curTime, camera, type, movement);
		//clusterParticles.insert(clusterParticles.end(), e.emitter->particles.begin(), e.emitter->particles.end()); 
	}

	// update buffer data

	for (int i = 0; i < this->clusterParticles.size(); ++i)
	{
		clusterPositions.push_back(this->clusterParticles[i]->position[0]);
		clusterPositions.push_back(this->clusterParticles[i]->position[1]);
		clusterPositions.push_back(this->clusterParticles[i]->position[2]);
		clusterPositions.push_back(this->clusterParticles[i]->aliveTime);
	}

	updateCenter();
}



void CloudCluster::drawCluster(const float dt, const double curTime, const glm::mat4 projection, const glm::mat4& view, const Camera camera, glm::vec3 movement)
{
	clusterUpdate(dt, curTime, camera, movement);

	std::sort(clusterParticles.begin(), clusterParticles.end(), [](const Particle* a, const Particle* b) {return a->cameraDistance > b->cameraDistance; });

	// get min and max y values
	float yMin = 0.0f;
	float yMax = 0.0f;

	// get furthest particle on xz plane 
	float furthestXZDist = 0.0f;

	for (int i = 0; i < clusterParticles.size(); i++)
	{
		if (clusterParticles[i]->position.y < yMin || i == 0)
		{
			yMin = clusterParticles[i]->position.y;
		}
	}

	for (int i = 0; i < clusterParticles.size(); i++)
	{
		if (clusterParticles[i]->position.y > yMax || i == 0)
		{
			yMax = clusterParticles[i]->position.y;
		}
	}

	for (int i = 0; i < clusterParticles.size(); i++)
	{
		float distance = glm::distance(glm::vec2(clusterParticles[i]->position.x, clusterParticles[i]->position.z), glm::vec2(clusterCenter.x, clusterCenter.z));
		if (distance > furthestXZDist)
		{
			furthestXZDist = distance;
		}
	}


	

	this->shader->use();
	this->shader->setMat4("projection", projection);
	this->shader->setMat4("view", view);

	// particle size
	if (type == Cirrus)
	{
		this->shader->setFloat("particleSize", 0.8f); // was (1.0)
	}
	else
	{
		this->shader->setFloat("particleSize", 0.8f); // was (2.0)
	}

	// set texture changing values
	this->shader->setVec3("clusterCenter", clusterCenter);
	this->shader->setInt("texArraySize", texArraySize);
	this->shader->setFloat("clusterRadiusVal", clusterRadius);

	// set shadowing values
	this->shader->setFloat("yMin", yMin); 
	this->shader->setFloat("yMax", yMax);
	this->shader->setFloat("furthestXZDist", furthestXZDist);

	// set cloud type in shader
	if (type == Cirrus)
	{
		this->shader->setInt("cloudType", 0);
		this->shader->setFloat("shadowPercentage", 0.1f);
	}
	else if (type == Cumulus)
	{
		this->shader->setInt("cloudType", 1);
		this->shader->setFloat("shadowPercentage", 0.2f);
	}
	else if (type == Stratus)
	{
		this->shader->setInt("cloudType", 2);
		this->shader->setFloat("shadowPercentage", 0.2f);
	}

	// set movement variables in shader
	this->shader->setVec3("movement", movement);
	this->shader->setFloat("dt", dt);

	// update the position buffer (could perhaps be sped up performance wise here, passing of data causing lag) 
	glBindBuffer(GL_ARRAY_BUFFER, this->positionBuffer);
	glInvalidateBufferData(this->positionBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, this->clusterParticles.size() * 4 * sizeof(float), clusterPositions.data());

	// vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, this->vertexBufferVBO);
	glBindVertexArray(vertexBuffer);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	// position buffer
	glEnableVertexAttribArray(4);
	glBindBuffer(GL_ARRAY_BUFFER, this->positionBuffer);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
	glVertexAttribDivisor(4, 1);

	// draw triangles
	glActiveTexture(GL_TEXTURE0);
	glBindTextureUnit(0, textureArrayID);
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 6, this->clusterParticles.size());

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(4);
}

void CloudCluster::resetCluster()
{
	merged = false;
	// removed any emitters that were merged into the cluster
	emitters.erase(emitters.begin() + startingEmitterNum, emitters.begin() + emitters.size());
	
	clusterParticles.clear();

	for (std::shared_ptr<EmitterController> e : emitters)
	{
		glm::vec3 movementPos = (getSpherePos() * clusterRadius) + clusterCenter; // move particle system to random point in cluster
		e->emitter->emitterPos = movementPos;
		e->emitter->reset();

		for (int i = 0; i < e->emitter->particles.size(); i++)
		{
			clusterParticles.push_back(&e->emitter->particles[i]);
		}
	}
}

// returns position within sphere
glm::vec3 CloudCluster::getSpherePos() 
{
	// generates positions by cloud type
	glm::vec3 pos = glm::vec3(5.0f, 5.0f, 5.0f); // gets into loop

	while (((pos.x * pos.x) + (pos.y * pos.y) + (pos.z * pos.z)) >= 1) { // discard positions not within sphere until one found that is within
		// generates positions by cloud type
		float x, y, z;
		if (this->type == Cumulus)
		{
			x = RandomFloat(-0.5f, 0.5f);
			if (x >= -0.25 && x <= 0.25)
			{
				y = RandomFloat(-0.25f, 0.5f);
			}
			else {
				y = RandomFloat(-0.25f, 0.25f);
			}

			z = RandomFloat(-0.5f, 0.5f); // was (-1.0, 1.0)

			pos = glm::vec3(x, y, z);
		}
		else if (this->type == Stratus)
		{
			x = RandomFloat(-1.0f, 1.0f);
			y = RandomFloat(-0.15f, 0.15f);
			z = RandomFloat(-0.5f, 0.5f);

			pos = glm::vec3(x, y, z);
		}
		else
		{
			pos = glm::vec3(RandomFloat(-1.0f, 1.0f), RandomFloat(-1.0f, 1.0f), RandomFloat(-1.0f, 1.0f));
		}

	}

	return pos;
}

float CloudCluster::RandomFloat(float a, float b) {
	std::random_device random;
	std::mt19937 gen(random());
	std::uniform_real_distribution<> dis(a, b);

	return dis(gen);
}