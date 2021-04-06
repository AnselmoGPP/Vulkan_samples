#ifndef WORLD_HPP
#define WORLD_HPP

#include <iostream>
#include <cmath>
#include <map>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "geometry.hpp"

/// Bidimensional index (x, y) that satisfies the "Compare" set of requirements for its use in std::map.
class BinaryKey
{
public:
    int x;
    int y;

    /// Constructor
    BinaryKey(int first, int second);

    /// Set (x, y) values simultaneously
    void set(int first, int second);

    /// Comparison (binary predicate). Strict weak ordering (true if a precedes b)
    bool operator <( const BinaryKey &rhs ) const;

    /// Equivalence. True if a == b, false otherwise.
    bool operator ==( const BinaryKey &rhs ) const;
};

/*
 * TODO:
 * Circular area of chunks
 * Update only new chunks. Move or delete the others
 */
class terrainChunks
{
    //glm::vec3 viewerPosition;
    //std::vector<std::vector<terrainChunk>> chunk;
    //std::vector<std::vector<glm::vec2>> chunkName;

public:
    noiseSet noise;             ///< Noise generator
    float    maxViewDist;       ///< Maximum view distance from viewer
    float    chunkSize;         ///< Size of each chunk (meters)
    int      chunksVisible;     ///< Number of chunkSizes for reaching maxViewDist
    int      vertexPerSide;     ///< Number of vertex per chunk's side

    std::map<BinaryKey, terrainGenerator> chunkDict;    ///< Collection of all the chunks (as a dictionary)

    terrainChunks(noiseSet noise, float maxViewDist, float chunkSize, unsigned vertexPerSide);
    ~terrainChunks();

    int getNumVertex();
    int getNumIndices();
    int getMaxViewDist();

    void updateVisibleChunks(glm::vec3 viewerPos);
    void updateTerrainParameters(noiseSet noise, float maxViewDist, float chunkSize, unsigned vertexPerSide);
    void setNoise(noiseSet newNoise);
};

#endif
