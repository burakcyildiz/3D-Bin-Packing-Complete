#ifndef COMMON_H
#define COMMON_H

#include <vector>
#include <queue>
#include <fstream>
#include <string>
#include <algorithm>
#include <ctime>
#include "Rect.h"
#include "MaxRectsBinPack.h"
#include "ilcplex\ilocplex.h"

//*********************************
//PARAMETERS
//*********************************

//Main toggle
#define academicToggle 0				//Toggles between the academic and practical testing (parameter file format is different)

//Bin size parameters
#define BinWidth 610	
#define BinDepth 820
int BinHeight = 2200;
int normalLayerHeight = 0.8 * BinHeight;			//Height that the layers will be placed until

//Planogram sequencing parameters
#define planogramSeqToggle 1			//Toggles planogram sequencing capabilities
#define BatchSize 200					//The number of items that will be planned for at a time

//Load cap parameters
#define loadCapToggle 1					//Toggles load cap capabilities

//Bin weight parameters
#define binWeightToggle 1				//Togg bin weight limitation

//Item shape parameters
#define itemShapeToggle 0				//Toggles the inclusion of different item shapes
#define edgeReduceToggle 0
#define edgeSupportThickness 27

//Vertical support parameters
#define verticalSupportToggle 1			//Toggles vertical support capabilities

//Tolerance parameters
#define LayerHeightTolerance 8			// Default 5
#define superItemWidthTolerance 0.6		// Default 0.6
#define superItemDepthTolerance 0.6		// Default 0.6

//Implementation parameters
#define improvementLimit 15				// Nb iterations without improvement after the layer generation loop will be stopped			---		Default: 20
#define MAXRECTSHeuristic 5				// 0: CornerPoint		1: LongSide		2: ShortSide		3: Area		4: BL		5: All		---		Default: 5
#define maximumSuperItem 2				// Maximum number of items that will be placed vertically to generate super items				---		Default: 2
#define industryTolerance 1	
#define binPlacementHeuristic 1			// 1: Bottom to Top
#define minEdgeRatio 0.8				// Minimum edge length ratio for super items
#define epIncrement 50					// Increment of coordinates that is used during extreme point generation
int nItems;
int nbLines;
int shortestHeight = BinHeight;
int shortestWD = BinDepth;
int maxSeqNo;
int maxLoadCap = 0;
int maxBinWeight;						//Max bin weight limit in grams
double itemSupportPercentage;			//Minimum bottom support percentage required per item
int nbLeftoverItems;
double totalVol;

//Infeasibility counters (DEBUG)
int infAreaSupport;
int inf4CSupport;
int infLoadBearing;
int infBinWeight;

using namespace std;

//Type definitions for CPLEX parameters and variables
typedef IloArray<IloNumArray> Float2Matrix;
typedef IloArray<Float2Matrix> Float3Matrix;
typedef IloArray<Float3Matrix> Float4Matrix;

typedef IloArray<IloNumVarArray> Var2Matrix;
typedef IloArray<Var2Matrix> Var3Matrix;
typedef IloArray<Var3Matrix> Var4Matrix;

typedef IloArray<IloRangeArray> Range2Matrix;
typedef IloArray<Range2Matrix> Range3Matrix;
typedef IloArray<Range3Matrix> Range4Matrix;

//Extreme point structure for ease of use
//An extreme point is a candidate location for single item placement
struct extremePoint
{
	//Coordinates of the extreme point
	int x, y, z;

	//Default constructor
	struct extremePoint()
	{
		this->x = 0;
		this->y = 0;
		this->z = 0;
	}

	//Destructor
	~extremePoint()
	{
	}
};

/**
Sorts extreme point in ascending order of z, y, and x, respectively

@param 2 extreme point objects
@return sorting rule
*/
bool sortEP(const extremePoint &e1, extremePoint &e2)
{
	if (e1.z < e2.z)
		return true;
	else if (e1.z > e2.z)
		return false;
	else if (e1.x < e2.x)
		return true;
	else if (e1.x > e2.x)
		return false;
	else if (e1.y < e2.y)
		return true;
	else
		return false;
}

//Container for file parsing
template <typename Container>
Container& split(Container& result,
	const typename Container::value_type& s,
	const typename Container::value_type& delimiters)
{
	result.clear();
	size_t current;
	size_t next = -1;
	do {
		current = next + 1;
		next = s.find_first_of(delimiters, current);
		result.push_back(s.substr(current, next - current));
	} while (next != Container::value_type::npos);
	return result;
}

/**
Calculates width-depth overlap between two items. Item 1 is at the bottom and Item 2 is on top

@param x-y coordinates and width-depth parameters of each item, suppport type and the edge reduce width-depth of the second item
@return total overlap area value between items, based on item shapes
*/
double calculateOverlap(int x1, int y1, int w1, int d1, int x2, int y2, int w2, int d2, int supportType1, int edgeReduceW1, int edgeReduceD1)
{
	//If there is edge reduce, update the starting coordinates and width, depth values of the bottom item
	x1 += edgeReduceW1;
	y1 += edgeReduceD1;
	w1 -= edgeReduceW1 * 2;
	d1 -= edgeReduceD1 * 2;

	//If the items do not overlap at all, return 0
	if (x1 + w1 < x2)
		return 0;
	if (y1 + d1 < y2)
		return 0;
	if (x2 + w2 < x1)
		return 0;
	if (y2 + d2 < y1)
		return 0;

	double overlap;
	int maxLeftX, minRightX, maxFrontY, minBackY;

	//Calculate overlap based on the shape of the bottom item
	switch (supportType1)
	{
	case 0:
	{
		maxLeftX = max(x1, x2);
		minRightX = min(x1 + w1, x2 + w2);
		maxFrontY = max(y1, y2);
		minBackY = min(y1 + d1, y2 + d2);

		overlap = (minRightX - maxLeftX) * (minBackY - maxFrontY);
		break;
	}
	case 1:
	{
		maxLeftX = max(x1, x2);
		minRightX = min(x1 + w1, x2 + w2);
		maxFrontY = max(y1, y2);
		minBackY = min(y1 + d1, y2 + d2);

		int insideX1 = x1 + edgeSupportThickness;
		int insideW1 = w1 - 2 * edgeSupportThickness;
		int insideY1 = y1 + edgeSupportThickness;
		int insideD1 = d1 - 2 * edgeSupportThickness;
		int insideMaxLeftX = max(insideX1, x2);
		int insideMinRightX = min(insideX1 + insideW1, x2 + w2);
		int insideMaxFrontY = max(insideY1, y2);
		int insideMinBackY = min(insideY1 + insideD1, y2 + d2);

		double insideOverlap = (insideMinRightX - insideMaxLeftX) * (insideMinBackY - insideMaxFrontY);
		overlap = (minRightX - maxLeftX) * (minBackY - maxFrontY) - insideOverlap;
		break;
	}
	case 2:
	{
		maxLeftX = max(x1, x2);
		minRightX = min(x1 + w1, x2 + w2);
		maxFrontY = max(y1, y2);
		minBackY = min(y1 + d1, y2 + d2);
		int insideX1, insideW1, insideY1, insideD1;
		if (w1 < d1)
		{
			insideX1 = x1;
			insideW1 = w1;
			insideY1 = y1 + edgeSupportThickness;
			insideD1 = d1 - 2 * edgeSupportThickness;
		}
		else
		{
			insideX1 = x1 + edgeSupportThickness;
			insideW1 = w1 - 2 * edgeSupportThickness;
			insideY1 = y1;
			insideD1 = d1;
		}

		int insideMaxLeftX = max(insideX1, x2);
		int insideMinRightX = min(insideX1 + insideW1, x2 + w2);
		int insideMaxFrontY = max(insideY1, y2);
		int insideMinBackY = min(insideY1 + insideD1, y2 + d2);

		double insideOverlap = (insideMinRightX - insideMaxLeftX) * (insideMinBackY - insideMaxFrontY);
		overlap = (minRightX - maxLeftX) * (minBackY - maxFrontY) - insideOverlap;
		break;
	}
	case 3:
	{
		maxLeftX = max(x1, x2);
		minRightX = min(x1 + w1, x2 + w2);
		maxFrontY = max(y1, y2);
		minBackY = min(y1 + d1, y2 + d2);
		int insideX1, insideW1, insideY1, insideD1;
		if (w1 > d1)
		{
			insideX1 = x1;
			insideW1 = w1;
			insideY1 = y1 + edgeSupportThickness;
			insideD1 = d1 - 2 * edgeSupportThickness;
		}
		else
		{
			insideX1 = x1 + edgeSupportThickness;
			insideW1 = w1 - 2 * edgeSupportThickness;
			insideY1 = y1;
			insideD1 = d1;
		}

		int insideMaxLeftX = max(insideX1, x2);
		int insideMinRightX = min(insideX1 + insideW1, x2 + w2);
		int insideMaxFrontY = max(insideY1, y2);
		int insideMinBackY = min(insideY1 + insideD1, y2 + d2);

		double insideOverlap = (insideMinRightX - insideMaxLeftX) * (insideMinBackY - insideMaxFrontY);
		overlap = (minRightX - maxLeftX) * (minBackY - maxFrontY) - insideOverlap;
		break;
	}
	case 4:
	{
		int insideX1_Corner1, insideY1_Corner1, insideX1_Corner2, insideY1_Corner2, insideX1_Corner3, insideY1_Corner3, insideX1_Corner4, insideY1_Corner4,
			insideMaxLeftX_C1, insideMinRightX_C1, insideMaxFrontY_C1, insideMinBackY_C1,
			insideMaxLeftX_C2, insideMinRightX_C2, insideMaxFrontY_C2, insideMinBackY_C2,
			insideMaxLeftX_C3, insideMinRightX_C3, insideMaxFrontY_C3, insideMinBackY_C3,
			insideMaxLeftX_C4, insideMinRightX_C4, insideMaxFrontY_C4, insideMinBackY_C4;

		double insideOverlap_C1, insideOverlap_C2, insideOverlap_C3, insideOverlap_C4;

		insideX1_Corner1 = x1; insideY1_Corner1 = y1;
		insideX1_Corner2 = x1; insideY1_Corner2 = y1 + (d1 - edgeSupportThickness);
		insideX1_Corner3 = x1 + (w1 - edgeSupportThickness); insideY1_Corner3 = y1;
		insideX1_Corner4 = x1 + (w1 - edgeSupportThickness); insideY1_Corner4 = y1 + (d1 - edgeSupportThickness);

		insideMaxLeftX_C1 = max(insideX1_Corner1, x2); insideMinRightX_C1 = min(insideX1_Corner1 + edgeSupportThickness, x2 + w2); insideMaxFrontY_C1 = max(insideY1_Corner1, y2); insideMinBackY_C1 = min(insideY1_Corner1 + edgeSupportThickness, y2 + d2);
		insideMaxLeftX_C2 = max(insideX1_Corner2, x2); insideMinRightX_C2 = min(insideX1_Corner2 + edgeSupportThickness, x2 + w2); insideMaxFrontY_C2 = max(insideY1_Corner2, y2); insideMinBackY_C2 = min(insideY1_Corner2 + edgeSupportThickness, y2 + d2);
		insideMaxLeftX_C3 = max(insideX1_Corner3, x2); insideMinRightX_C3 = min(insideX1_Corner3 + edgeSupportThickness, x2 + w2); insideMaxFrontY_C3 = max(insideY1_Corner3, y2); insideMinBackY_C3 = min(insideY1_Corner3 + edgeSupportThickness, y2 + d2);
		insideMaxLeftX_C4 = max(insideX1_Corner4, x2); insideMinRightX_C4 = min(insideX1_Corner4 + edgeSupportThickness, x2 + w2); insideMaxFrontY_C4 = max(insideY1_Corner4, y2); insideMinBackY_C4 = min(insideY1_Corner4 + edgeSupportThickness, y2 + d2);

		insideOverlap_C1 = (insideMinRightX_C1 - insideMaxLeftX_C1) * (insideMinBackY_C1 - insideMaxFrontY_C1);
		insideOverlap_C2 = (insideMinRightX_C2 - insideMaxLeftX_C2) * (insideMinBackY_C2 - insideMaxFrontY_C2);
		insideOverlap_C3 = (insideMinRightX_C3 - insideMaxLeftX_C3) * (insideMinBackY_C3 - insideMaxFrontY_C3);
		insideOverlap_C4 = (insideMinRightX_C4 - insideMaxLeftX_C4) * (insideMinBackY_C4 - insideMaxFrontY_C4);

		overlap = insideOverlap_C1 + insideOverlap_C2 + insideOverlap_C3 + insideOverlap_C4;
		break;
	}
	}
	
	return overlap;
}

#endif