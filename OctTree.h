/*
 * OctTree.h
 *
 *  Created on: Sep 12, 2011
 *      Author: dkoes
 *
 *      An octtree representation of a molecular shape.
 *      Requires a predefined bounding box and resolution.
 *      Voxelizes a molecule represented as a collection of spheres.
 *
 *      This is a pointer-based structure designed for easy of coding and use.
 */

#ifndef OCTTREE_H_
#define OCTTREE_H_

#include "MolSphere.h"
#include <cstdlib>
#include <cstring>
#include <vector>
#include <iostream>

using namespace std;

/* A cube. Functions for consistently dividing into octants */
class Cube
{
	float x, y, z; //bottom corner
	float dim;

	inline float squared(float v) const { return v * v; }

public:

	Cube(float d) :
		x(0), y(0), z(0), dim(d)
	{

	}

	float getDimension() const
	{
		return dim;
	}

	//return i'th octant
	Cube getOctant(unsigned i) const
	{
		Cube res = *this;
		res.dim /= 2.0;

		switch (i)
		{
		case 0:
			break;
		case 1:
			res.x += res.dim;
			break;
		case 2:
			res.y += res.dim;
			break;
		case 3:
			res.x += res.dim;
			res.y += res.dim;
			break;
		case 4:
			res.z += res.dim;
			break;
		case 5:
			res.x += res.dim;
			res.z += res.dim;
			break;
		case 6:
			res.y += res.dim;
			res.z += res.dim;
			break;
		case 7:
			res.x += res.dim;
			res.y += res.dim;
			res.z += res.dim;
			break;
		default:
			abort();
		}
		return res;
	}

	//thank you stack overflow for not making me think..
	bool intersectsSphere(const MolSphere& sphere) const
	{
		float dist = sphere.r*sphere.r;
		float x2 = x+dim;
		float y2 = y+dim;
		float z2 = z+dim;

		if(sphere.x < x)
			dist -= squared(sphere.x-x);
		else if(sphere.x > x2)
			dist -= squared(sphere.x - x2);

		if(sphere.y < y)
			dist -= squared(sphere.y-y);
		else if(sphere.y > y2)
			dist -= squared(sphere.y - y2);

		if(sphere.z < z)
			dist -= squared(sphere.z-z);
		else if(sphere.z > z2)
			dist -= squared(sphere.z - z2);

		return dist > 0;
	}

};

class OctTree
{
	float dimension; //sits within a cube of this dimension, power of 2
	float resolution; //increase resolution down to this increment (power of 2)

	enum OctType
	{
		Empty, Full, Children
	};
	struct OctNode
	{
		OctType type;
		float dim; //this is slightly unnecessary, the dimension of this node
		OctNode *children[8]; //pointers belong to this node

		OctNode() :
			type(Empty), dim(0)
		{
			memset(children, 0, sizeof(children));
		}

		OctNode(const OctNode& rhs);

		OctNode& operator=(const OctNode& rhs);


		void deleteChildren()
		{
			if (type == Children)
			{
				for (unsigned i = 0; i < 8; i++)
				{
					if (children[i])
					{
						delete children[i];
						children[i] = NULL;
					}
				}
			}
			type = Empty;
		}

		~OctNode()
		{
			deleteChildren();
		}

		//return true if changed
		bool intersect(const OctNode *rhs);
		bool unionWith(const OctNode *rhs);
		void truncate(double resolution);
		void grow(double resolution);

		float intersectVolume(const OctNode *rhs) const;
		float unionVolume(const OctNode *rhs) const;
		float volume() const; //recursive volume calculation
		unsigned leaves() const; //recursive leafs cnt

		void write(ostream& out) const;
		void read(istream& out);

	};

	OctNode *root;

	//recursively generate an octtree
	void create(OctNode *node, const Cube& cube, const vector<MolSphere>& mol);


public:

	OctTree(): dimension(0), resolution(0), root(NULL)
	{
		root = new OctNode();
	}

	OctTree(float dim, float res): dimension(dim), resolution(res)
	{
		root = new OctNode();
	}

	OctTree(float dim, float res, const vector<MolSphere>& mol): dimension(dim), resolution(res)
	{
		Cube cube(dimension);
		root = new OctNode();
		create(root, cube, mol);
	}

	OctTree(const OctTree& rhs):dimension(rhs.dimension), resolution(rhs.resolution), root(NULL)
	{
		if(rhs.root)
		{
			root = new OctNode(*rhs.root);
		}
	}

	OctTree& operator=(OctTree rhs)
	{
		swap(*this, rhs); //rhs passed by value
		return *this;
	}

	virtual ~OctTree()
	{
		if(root)
			delete root;
	}

    friend void swap(OctTree& first, OctTree& second)
    {
        // enable ADL (not necessary in our case, but good practice)
        using std::swap;

        swap(first.resolution, second.resolution);
        swap(first.dimension, second.dimension);
        swap(first.root, second.root);
    }

	//mogrifying intersection
	bool intersect(const OctTree& rhs) { return root->intersect(rhs.root); }
	//mogrifying union
	bool unionWith(const OctTree& rhs) { return root->unionWith(rhs.root); }

	//truncate (maximum included volume) to specified resolution
	//only nodes that are >= resolution and full are kept
	void truncate(double resolution) { root->truncate(resolution); }

	//expand (minimum surrounding volume) so that any non-empty nodes <= resolution
	//are made full
	void grow(double resolution) { root->grow(resolution); }

	//volume calculations that don't require creating a tmp tree
	float intersectVolume(const OctTree& rhs) const { return root->intersectVolume(rhs.root); }
	float unionVolume(const OctTree& rhs) const { return root->unionVolume(rhs.root); }

	//return total volume contained in octtree
	float volume() const { return root->volume();}

	//return number of leaves
	unsigned leaves() const { return root->leaves(); }

	void clear() { root->deleteChildren(); }
	void fill() { root->deleteChildren(); root->type = Full; }

	void write(ostream& out) const;
	void read(istream& in);
};

#endif /* OCTTREE_H_ */
