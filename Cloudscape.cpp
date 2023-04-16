#include "Cloudscape.h"

Cloudscape::Cloudscape(int sizeXInput, int sizeYInput, int sizeZInput, int stratusNumInput, int cumulusNumInput, int cirrusNumInput, unsigned int textureArrayInput, int numOfTexturesInput, std::vector<std::vector<std::vector<glm::vec3>>> vectorFieldIn)
{
	// set bounds of cloudscape
	sizeX = sizeXInput;
	sizeY = sizeYInput;
	sizeZ = sizeZInput;

	// set clouds in cloudscape
	stratusNum = stratusNumInput;
	cumulusNum = cumulusNumInput;
	cirrusNum = cirrusNumInput;

	// set texture 
	textureArray = textureArrayInput;
	numOfTextures = numOfTexturesInput;

	// set vector field
	vc.vectorField = vectorFieldIn;
	vc.sizeX = vectorFieldIn.size();
	vc.sizeY = vectorFieldIn[0].size();
	vc.sizeZ = vectorFieldIn[0][0].size();

	for (int i = 0; i < stratusNum; i++) // create stratus clouds
	{
		addCloud(Stratus, false);
	}

	for (int i = 0; i < cumulusNum; i++) // create cumulus clouds
	{
		addCloud(Cumulus, false);
	}

	for (int i = 0; i < cirrusNum; i++) // create cirrus clouds
	{
		addCloud(Cirrus, false);
	}

	particleNum = 0;
	for (std::shared_ptr<CloudCluster> c : clouds)
	{
		for (std::shared_ptr<EmitterController> e : c->emitters)
		{
			particleNum += e->emitter->particles.size();
		}
	}

	for (std::shared_ptr<CloudCluster> c : clouds) // setup buffers
	{
		glBindVertexArray(0);
		c->setupBuffers(particleNum);
		glBindVertexArray(0);
	}

	for (std::shared_ptr<CloudCluster> c : clouds)
	{
		c->resetCluster();
	}

}

void Cloudscape::drawScape(const float dt, const double curTime, const glm::mat4 projection, const glm::mat4& view, const Camera camera)
{
	checkForMerge(); // respawning clouds costs performance. Need to edit function to use current ones

	for (std::shared_ptr<CloudCluster> c : clouds)
	{
		c->cameraDistance = glm::length(camera.Position - c->clusterCenter);
	}
	std::sort(clouds.begin(), clouds.end(), [](const std::shared_ptr<CloudCluster> a, const std::shared_ptr<CloudCluster> b) {return a->cameraDistance > b->cameraDistance; });

	for (std::shared_ptr<CloudCluster> c : clouds)
	{
		// get x,y,z position for vector field
		int posX = trunc((c->clusterCenter.x / sizeX) * vc.sizeX);
		int posY = trunc((c->clusterCenter.y / sizeY) * vc.sizeY);
		int posZ = trunc((c->clusterCenter.z / sizeZ) * vc.sizeZ);

		if (posX >= vc.sizeX - 1 || posY >= vc.sizeY - 1 || posZ >= vc.sizeZ - 1) // if out of bounds respawn (still vector issues -1)
		{
			c->remove = true;
		}
	}

	// work out what clouds need replacing
	std::vector<CloudType> removedClouds;
	for (std::shared_ptr<CloudCluster> c : clouds)
	{
		if (c->remove == true)
		{
			removedClouds.push_back(c->type);
		}
	}

	//std::remove_if(clouds.begin(), clouds.end(), [](const std::shared_ptr<CloudCluster>) {return a->remove}), clouds.end());
	clouds.erase(std::remove_if(clouds.begin(), clouds.end(),[](std::shared_ptr<CloudCluster> c) {  return c->remove == true; }),clouds.end());

	for (CloudType t : removedClouds)
	{
		addCloud(t, true);
	}



	for (std::shared_ptr<CloudCluster> c : clouds)
	{
		if (c->buffersRunning == false)
		{
			c->setupBuffers(particleNum);
			c->resetCluster();
		}
		// get x,y,z position for vector field
		int posX = trunc((c->clusterCenter.x / sizeX) * vc.sizeX);
		int posY = trunc((c->clusterCenter.y / sizeY) * vc.sizeY);
		int posZ = trunc((c->clusterCenter.z / sizeZ) * vc.sizeZ);

		if (c->merged == false)
		{
			c->drawCluster(dt, curTime, projection, view, camera, vc.vectorField[posX][posY][posZ]);
			//std::cout << "size " << clouds.size() << std::endl;
		}
		else {
			c->clusterCenter += vc.vectorField[posX][posY][posZ];
		}
	}
}

void Cloudscape::checkForMerge() // not working clouds dissapearing (cloud keep track of original amount of emitters and remove all the remaining ones once respawned, then set the merged clouds bool to false again so they are respawned).
{
	for (std::shared_ptr<CloudCluster> c : clouds) // check if two clouds are touching and then merge if they are
	{
		if (c->merged == false)
		{
			for (int i = 0; i < clouds.size(); i++)
			{
				// clouds[i] = comparison cloud
				if ((c != clouds[i]) && (clouds[i]->merged == false))
				{
					float centerDistance = glm::distance(c->clusterCenter, clouds[i]->clusterCenter);
					float overlapDistance = centerDistance - c->clusterRadius; // distance from furthest cluster point to comparison points cluster center 

					//std::cout << " center: " << centerDistance << " overlapDistance: " << overlapDistance << std::endl;

					if (overlapDistance < clouds[i]->clusterRadius)
					{
						c->clusterParticles.insert(c->clusterParticles.end(), clouds[i]->clusterParticles.begin(), clouds[i]->clusterParticles.end());
						c->emitters.insert(c->emitters.end(), clouds[i]->emitters.begin(), clouds[i]->emitters.end());

						clouds[i]->merged = true;
					}
				}
			}
		}
	}
}


void Cloudscape::addCloud(CloudType type, bool respawn)
{
	if (type == Stratus)
	{
		//int cloudSize = round(RandomFloat(40, 50)); // was(40,50)
		int cloudSize = round(RandomFloat(60, 100));
		glm::vec3 cloudPos = getCloudPos(Stratus, respawn);

		std::vector<std::shared_ptr<EmitterController>> emitters;

		for (int j = 0; j < cloudSize; j++) // build particle emitters
		{
			emitters.push_back(std::move(std::shared_ptr<EmitterController>(new EmitterController(cloudPos))));
		}

		clouds.push_back(std::move(std::shared_ptr<CloudCluster>(new CloudCluster(emitters, Stratus, textureArray, numOfTextures))));
	} 
	else if (type == Cumulus)
	{
		int cloudSize = round(RandomFloat(20, 60)); // was(10)
		//int cloudSize = 10;
		glm::vec3 cloudPos = getCloudPos(Cumulus, respawn);

		std::vector<std::shared_ptr<EmitterController>> emitters;

		for (int j = 0; j < cloudSize; j++) // build particle emitters
		{
			emitters.push_back(std::move(std::shared_ptr<EmitterController>(new EmitterController(cloudPos))));
		}

		clouds.push_back(std::move(std::shared_ptr<CloudCluster>(new CloudCluster(emitters, Cumulus, textureArray, numOfTextures))));
	}
	else if (type == Cirrus)
	{
		int cloudSize = round(RandomFloat(5, 10));
		glm::vec3 cloudPos = getCloudPos(Cirrus, respawn);

		std::vector<std::shared_ptr<EmitterController>> emitters;

		for (int j = 0; j < cloudSize; j++) // build particle emitters
		{
			emitters.push_back(std::move(std::shared_ptr<EmitterController>(new EmitterController(cloudPos))));
		}

		clouds.push_back(std::move(std::shared_ptr<CloudCluster>(new CloudCluster(emitters, Cirrus, textureArray, numOfTextures))));
	}

}

glm::vec3 Cloudscape::getCloudPos(CloudType type, bool respawn)
{
	// respawn checks to see if this is a cloud that has previously been spawned and it wants a new position
	if (type == Stratus)
	{
		if (respawn == true)
		{
			return glm::vec3(0, RandomFloat(0, (sizeY / 10)), RandomFloat(0, sizeZ));
		}
		return glm::vec3(RandomFloat(0, sizeX), RandomFloat(0, (sizeY / 10)), RandomFloat(0, sizeZ));
	}
	else if (type == Cumulus)
	{
		if (respawn == true)
		{
			return glm::vec3(0, RandomFloat((2 * sizeY / 5), (2.25 * sizeY) / 5), RandomFloat(0, sizeZ));
		}
		return glm::vec3(RandomFloat(0, sizeX), RandomFloat((2 * sizeY / 5), (2.25 * sizeY) / 5), RandomFloat(0, sizeZ));
	}
	else if (type == Cirrus)
	{
		if (respawn == true)
		{
			return glm::vec3(0, RandomFloat((4 * sizeY) / 5, sizeY), RandomFloat(0, sizeZ));
		}
		return glm::vec3(RandomFloat(0, sizeX), RandomFloat((4 * sizeY) / 5, sizeY), RandomFloat(0, sizeZ));
	}
}


float Cloudscape::RandomFloat(float a, float b) {
	std::random_device random;
	std::mt19937 gen(random());
	std::uniform_real_distribution<> dis(a, b);

	return dis(gen);
}