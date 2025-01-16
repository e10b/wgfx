#pragma once

namespace World
{
	const float renderDistance = 400.0f; // block render radius
	const unsigned renderSpeed = 120; // chunks generated per frame

	// Entity gravity force
	const float gravity = 20.0f;

	// Chunk dimensions
	const unsigned chunkHeight = 256;
	const unsigned chunkSize = 16;
	const unsigned chunkArea = chunkSize * chunkSize;
	const unsigned chunkVolume = chunkArea * chunkHeight;

	// Configurable world gen variables
	namespace Generation
	{
		// Lowest block elevation possible
		const unsigned minHeight = 1;

		// Biome noise
		const float landScale = 2048.0f;
		const float landMinMult = 0.1f;
		const float landTransitionSharpness = 2.0f;
		const float landMountainBias = 0.2f; // -1 (flat) to 1 (mountains)

		// Mountain noise
		const float heightScale = 256.0f;
		const float heightWeight = 0.8f;
		const unsigned heightMaxHeight = 250;

		// Hill noise
		const float detailedScale = 32.0f;
		const float detailWeight = 1.0f - heightWeight;
		const unsigned detailMaxHeight = 100;

		// Trees
		const float treeDensity = 0.01f;//0.03f;

		// Interpolation grid size
		const float terrainInterpGrid = 4.0f;
	}
}