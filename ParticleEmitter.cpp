#include "ParticleEmitter.hpp"




// emmitter
ParticleEmitter::ParticleEmitter(glm::vec3 emitterPos)
{

	// set particle lifespan 
	lowerLife = 60.0; // was 22
	upperLife = 61.0; // was 25
	radius = 2.0f;

	// set emitter location
	this->emitterPos = emitterPos;

	// create 400 particles

	this->particles.resize( 400 ); // was 200
	for ( int i = 0 ; i < this->particles.size() ; ++i )
	{
		// give every particle a random position within sphere
		glm::vec3 partPos = getSpherePos(std::nullopt);
		this->particles[i].startingPos = glm::vec3(emitterPos.x + (partPos.x * radius), emitterPos.y + (partPos.y * radius), emitterPos.z + (partPos.z * radius));
		this->particles[i].position = glm::vec3(emitterPos.x + (partPos.x * radius), emitterPos.y + (partPos.y * radius), emitterPos.z + (partPos.z * radius));
		this->particles[i].lifetime = RandomFloat(lowerLife, upperLife); // between 5 and 10 seconds lifetime 
		this->particles[i].aliveTime = 0.0f;
	}
}



ParticleEmitter::~ParticleEmitter()
{
	//glDeleteBuffers(1, this->vertexBuffer);
}



void ParticleEmitter::update(const float dt, const double curTime, Camera camera, std::optional<CloudType> clusterInput, glm::vec3 movement)
{
	this->cameraDistance = glm::length(camera.Position - this->emitterPos); // sets camera distance for emitter transparency sorting

	for (int i = 0; i < this->particles.size(); ++i)
	{

		// subtract from the particles lifetime and add to aliveTime
		this->particles[i].lifetime -= dt;
		this->particles[i].aliveTime += dt;
		this->particles[i].position += (movement * dt); // move particle based on direction (need to respawn if goes out of bounds) 

		// if the lifetime is below 0 respawn the particle
		if (this->particles[i].lifetime <= 0.0f)
		{
			glm::vec3 partPos = getSpherePos(clusterInput);
			this->particles[i].position = glm::vec3(emitterPos.x + (partPos.x * radius), emitterPos.y + (partPos.y * radius), emitterPos.z + (partPos.z * radius));
			this->particles[i].lifetime = RandomFloat(lowerLife, upperLife); // between 3 and 4 seconds lifetime 
			this->particles[i].aliveTime = 0.0f;

		}

		// stores camera distance 
		particles[i].cameraDistance = glm::length(camera.Position - particles[i].position);
	}
}

void ParticleEmitter::reset()
{
	for (int i = 0; i < this->particles.size(); i++)
	{
		this->particles[i].lifetime = 0.0f;
	}
}

// returns position within sphere
glm::vec3 ParticleEmitter::getSpherePos(std::optional<CloudType> clusterType) {
	glm::vec3 pos;

	if (clusterType) // change shape of emitter based on cloud type
	{
		auto cloudType = std::move(*clusterType);
		if (cloudType == Cumulus)
		{
			pos = glm::vec3(RandomFloat(-1.0f, 1.0f), RandomFloat(-1.0f, 1.0f), RandomFloat(-1.0f, 1.0f));

			while (((pos.x * pos.x) + (pos.y * pos.y) + (pos.z * pos.z)) >= 1) { 
				pos = glm::vec3(RandomFloat(-1.0f, 1.0f), RandomFloat(-1.0f, 1.0f), RandomFloat(-1.0f, 1.0f)); // produces full spheres
			}
		}
		else if (cloudType == Stratus)
		{
			pos = glm::vec3(RandomFloat(-1.0f, 1.0f), RandomFloat(-1.0f, 1.0f), RandomFloat(-1.0f, 1.0f));

			while (((pos.x * pos.x) + (pos.y * pos.y) + (pos.z * pos.z)) >= 1) { 
				pos = glm::vec3(RandomFloat(-1.0f, 1.0f), RandomFloat(-0.25f, 0.25f), RandomFloat(-1.0f, 1.0f)); // produce flat based spheres
			}
		}
		else if (cloudType == Cirrus)
		{
			float x, y, z;
			x = RandomFloat(-1.0f, 1.0f);
			y = (x * x) + RandomFloat(-0.2f, 0.2f); // creates curve and adds offset so it dosen't look so uniform
			z = RandomFloat(-0.5f, 0.5f);
			pos = glm::vec3(x, y, z);

		}
	}
	else {
		pos = glm::vec3(RandomFloat(-1.0f, 1.0f), RandomFloat(-1.0f, 1.0f), RandomFloat(-1.0f, 1.0f));

		while (((pos.x * pos.x) + (pos.y * pos.y) + (pos.z * pos.z)) >= 1) { // discard positions not within sphere until one found that is within
			pos = glm::vec3(RandomFloat(-1.0f, 1.0f), RandomFloat(-1.0f, 1.0f), RandomFloat(-1.0f, 1.0f));
		}
	}

	return pos;
}

float ParticleEmitter::RandomFloat(float a, float b) {
	float random = ((float)rand()) / (float)RAND_MAX;
	float diff = b - a;
	float r = random * diff;
	return a + r;
}




// controller
EmitterController::EmitterController(glm::vec3 emitterPos)
{
	this->emitter = new ParticleEmitter(emitterPos);
}

EmitterController::~EmitterController()
{
	// need to work on this (Causes exception with the cluster class, deletes previous vector item in constructor) 
	//delete this->emitter;
}