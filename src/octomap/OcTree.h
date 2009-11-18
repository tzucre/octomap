// $Id$

/**
* Octomap:
* A  probabilistic, flexible, and compact 3D mapping library for robotic systems.
* @author K. M. Wurm, A. Hornung, University of Freiburg, Copyright (C) 2009.
* @see http://octomap.sourceforge.net/
* License: GNU GPL v2, http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
*/

/*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along
* with this program; if not, write to the Free Software Foundation, Inc.,
* 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#ifndef OCTOMAP_BINARYOCTREE_HH
#define OCTOMAP_BINARYOCTREE_HH

#include "AbstractOcTree.h"
#include "OcTreeNode.h"
#include "ScanGraph.h"

namespace octomap {

  /**
   * The actual octomap map data structure, stored in an OcTree.
   *
   */
  class OcTree : public AbstractOcTree <OcTreeNode> {

  public:
    static const int TREETYPE=3;

  public:

    /**
     * Creates a new (empty) OcTree of a given resolution
     * @param _resolution
     */
    OcTree(double _resolution);

    /**
     * Creates a new OcTree by reading in from a binary file
     * @param _filename
     *
     */
    OcTree(std::string _filename);

    virtual ~OcTree();

    /**
     * Updates an OcTreeNode in the OcTree either as observed free or occupied.
     *
     * @param value 3D position of the OcTreeNode that is to be updated
     * @param occupied Whether the node was observed occupied or free
     * @return
     */
    virtual OcTreeNode* updateNode(const point3d& value, bool occupied);

    /// Insert a 3d scan as ScanNode into the tree, creates delta nodes
    void insertScan(const ScanNode& scan);

    /// Insert one ray between origin and end into the tree.
    /// integrateMissOnRay() is called for the ray, the end point is updated as occupied.
    virtual bool insertRay(const point3d& origin, const point3d& end);

    /// Traces a ray from origin to end and updates all voxels on the way as free.
    /// The volume containing "end" is not updated.
    void integrateMissOnRay(const point3d& origin, const point3d& end);

    /// Convert all delta nodes to binary nodes
    void deltaToBinary();

    /**
     * Traverses the tree and collects all OcTreeVolumes regarded as occupied.
     * MIXED nodes are regarded as occupied.
     *
     * @param max_depth how deep to traverse the tree
     * @param occ_thres Occupancy threshold for regarding nodes as occupied
     * @param binary_nodes list of binary OcTreeVolumes which are occupied
     * @param delta_nodes list of delta OcTreeVolumes which are occupied
     */
    void getOccupied(unsigned int max_depth, double occ_thres, std::list<OcTreeVolume>& binary_nodes, std::list<OcTreeVolume>& delta_nodes) const;

    /**
     * Traverses the tree and collects all OcTreeVolumes regarded as free.
     * MIXED nodes are regarded as occupied.
     *
     * @param max_depth how deep to traverse the tree
     * @param occ_thres Occupancy threshold for regarding nodes as free
     * @param binary_nodes list of binary OcTreeVolumes which are free
     * @param delta_nodes list of delta OcTreeVolumes which are free
     */
    void getFreespace(unsigned int max_depth, double occ_thres, std::list<OcTreeVolume>& binary_nodes, std::list<OcTreeVolume>& delta_nodes) const;

    /**
     * Traverses the tree and collects all OcTreeVolumes which are potential obstacles.
     * These are nodes labeled "free" (were binary before), but are now delta
     *
     * @param max_depth how deep to traverse the tree
     * @param occ_thres Occupancy threshold for regarding nodes as free
     * @param nodes list of OcTreeVolumes which are potential obstacles
     */
    void getChangedFreespace(unsigned int max_depth, double occ_thres, std::list<OcTreeVolume>& nodes) const;

    /// \return Memory usage of the OcTree in Bytes.
    unsigned int memoryUsage() const;

    /// \return Memory usage of a full grid of the same size as the OcTree in Bytes (for comparison)
    unsigned int memoryFullGrid();

    /// \return Memory usage of an OcTree using eight pointers in Bytes (for comparison)
    unsigned int memoryUsageEightPointers();

    /// Size of OcTree in meters for x, y and z dimension
    void getMetricSize(double& x, double& y, double& z);
    /// minimum value in x, y, z
    void getMetricMin(double& x, double& y, double& z);
    /// maximum value in x, y, z
    void getMetricMax(double& x, double& y, double& z);

    /// Lossless compression of OcTree: merge children to parent when they
    /// have the same value
    void prune();

    void calcNumberOfNodesPerType(unsigned int& num_binary, unsigned int& num_delta) const;

    /// Reads an OcTree from a binary filestream
    std::istream& readBinary(std::ifstream &s);
    /// Writes OcTree to a binary filestream. The OcTree is converted to binary and pruned first.
    std::ostream& writeBinary(std::ostream &s);

    /// Reads an OcTree from a binary file
    void readBinary(std::string filename);
    /// Writes OcTree to a binary file using writeBinary(). The OcTree is converted to binary and pruned first.
    void writeBinary(std::string filename);

  protected:

    /**
     * Traces a ray from origin to end (excluding), returning all
     * cell centers of cells on beam
     * (Essentially using the DDA algorithm in 3D).
     */
    bool computeRay(const point3d& origin, const point3d& end, std::vector<point3d>& _ray) const;

    void insertScanUniform(const ScanNode& scan);

    // insert only freespace (freespace=true) or occupied space
    void insertScanFreeOrOccupied(const ScanNode& scan, bool freespace);

    /// recursive call of updateNode()
    OcTreeNode* updateNodeRecurs(OcTreeNode* node, bool node_just_created, unsigned short int key[3],
				 unsigned int depth, bool occupied);

    /// recursive call of getOccupied()
    void getOccupiedRecurs(OcTreeNode* node, unsigned int depth, unsigned int max_depth, double occ_thres, const point3d& parent_center,
			   std::list<OcTreeVolume>& binary_nodes, std::list<OcTreeVolume>& delta_nodes) const;
    /// recursive call of getFreeSpace()
    void getFreespaceRecurs(OcTreeNode* node, unsigned int depth, unsigned int max_depth, double occ_thres, const point3d& parent_center,
			    std::list<OcTreeVolume>& binary_nodes, std::list<OcTreeVolume>& delta_nodes) const;

    ///recursive call of deltaToBinary()
    void deltaToBinaryRecurs(OcTreeNode* node, unsigned int depth, unsigned int max_depth);

    /// recursive call of prune()
    void pruneRecurs(OcTreeNode* node, unsigned int depth, unsigned int max_depth, unsigned int& num_pruned);

    void calcNumberOfNodesPerTypeRecurs(OcTreeNode* node,
					unsigned int& num_binary,
					unsigned int& num_delta) const;

    /// Traverses the tree to calculate the total number of nodes
    unsigned int calcNumNodes() const;
    void calcNumNodesRecurs(OcTreeNode* node, unsigned int& num_nodes) const;
    /// recalculates min and max in x, y, z. Only called when needed, after tree size changed.
    void calcMinMax();

  };


} // end namespace

#endif
