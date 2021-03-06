Find Feature Neighbor C-Axis Misalignments {#findfeatureneighborcaxismisalignments}
==========

## Group (Subgroup) ##
Statistics Filters (Crystallographic)

## Description ##
This Filter determines, for each **Feature**, the c-axis misalignments with the **Features** that are in contact with it.  The c-axis misalignments are stored as a list (for each **Feature**) of angles.

## Parameters ##

| Name | Type |
|-------|------|
|Find Average C-Axis Mialignments | Boolean |

## Required DataContainers ##
Volume

## Required Arrays ##

| Type | Default Name | Description | Comment | Filters Known to Create Data |
|------|--------------|-------------|---------|-----|
| Feature | NeighborLists | List of the contiguous neighboring **Features** for a given **Feature** |  | Find Feature Neighbors (Statistics) |
| Feature | AvgQuats | Five (5) values (floats) defining the average orientation of the **Feature** in quaternion representation | Filter will calculate average quaternions for **Features** if not already calculated. | Find Feature Average Orientations (Statistics) |
| Feature | FeaturePhases | Phase Id (int) specifying the phase of the **Feature**| | Find Feature Phases (Generic), Read Feature Info File (IO), Pack Primary Phases (SyntheticBuilding), Insert Precipitate Phases (SyntheticBuilding), Establish Matrix Phase (SyntheticBuilding) |
| Ensemble | CrystalStructures | Enumeration (int) specifying the crystal structure of each Ensemble/phase (Hexagonal=0, Cubic=1, Orthorhombic=2) | Values should be present from experimental data or synthetic generation and cannot be determined by this filter. Not having these values will result in the filter to fail/not execute. | Read H5Ebsd File (IO), Read Ensemble Info File (IO), Initialize Synthetic Volume (SyntheticBuilding) |

## Created Arrays ##

| Type | Default Name | Description | Comment |
|------|--------------|-------------|---------|
| Feature | CAxisMisalignmentList | List of the c-axis misalignment angles with the contiguous neighboring **Features** for a given **Feature** |  |
| Feature | AvgCAxisMisalignments | Number weighted average of neighbor c-axis misalignments. | Only created if Find Average C-Axis Misalignments IS checked. |

## Authors ##

**Copyright:** 2013 Joseph C. Tucker, 2012 Michael A. Groeber (AFRL), 2012 Michael A. Jackson (BlueQuartz Software)

**Contact Info:** dream3d@bluequartz.net

**Version:** 1.0.0

**License:**  See the License.txt file that came with DREAM3D.




See a bug? Does this documentation need updated with a citation? Send comments, corrections and additions to [The DREAM3D development team](mailto:dream3d@bluequartz.net?subject=Documentation%20Correction)

