#ifndef BIN_H
#define BIN_H

#include "Layer.h"

using namespace std;

//Class definition for a bin
struct Bin
{
	vector<Layer> binLayers;		//List of layers in a bin
	//Lists for the items and their width, depth, height dimensions, and x, y, z coordinates in the bin
	vector<int> itemWidths, itemDepths, itemHeights, itemXCoords, itemYCoords, itemZCoords, itemList;
	vector<extremePoint> epList;	//List of extreme points in a bin
	int height;						//Height of a bin
	int layerHeight;				//Height that layers are placed until in this bin
	int weight;						//Total weight of a bin

	//Default constructor
	struct Bin()
	{
		this->height = 0;
		this->weight = 0;
	}

	//Destructor
	~Bin()
	{
		this->binLayers.clear();
		this->itemWidths.clear();
		this->itemDepths.clear();
		this->itemHeights.clear();
		this->itemXCoords.clear();
		this->itemYCoords.clear();
		this->itemZCoords.clear();
		this->itemList.clear();
	}

	/**
	Insert an item from a layer to the bin
	@param the width, depth, height dimensions and x, y, z coordinates (front bottom left corner) of the item to be placed
	*/
	void insertItemFromLayer(int width, int depth, int height, int x, int y, int z)
	{
		itemWidths.push_back(width);
		itemDepths.push_back(depth);
		itemHeights.push_back(height);
		itemXCoords.push_back(x);
		itemYCoords.push_back(y);
		itemZCoords.push_back(z);

		//Add new extreme points 
		for (int w = 0; w <= width; w += epIncrement)
		{
			if (BinDepth - (y + depth) >= shortestWD)
			{
				extremePoint newEP;
				newEP.x = x + w; newEP.y = y + depth; newEP.z = z;
				this->epList.push_back(newEP);
			}
		}
		for (int d = 0; d <= depth; d += epIncrement)
		{
			if (BinWidth - (x + width) >= shortestWD)
			{
				extremePoint newEP;
				newEP.x = x + width; newEP.y = y + d; newEP.z = z;
				this->epList.push_back(newEP);
			}
		}

		//Remove obsolete extreme points from epList
		for (int i = epList.size() - 1; i >= 0; i--)
		{
			if ((x <= epList[i].x && x + width > epList[i].x)
				&& (y <= epList[i].y && y + depth > epList[i].y)
				&& (z <= epList[i].z && z + height > epList[i].z))
			{
				extremePoint temp = epList.back();
				epList.back() = epList[i];
				epList[i] = temp;
				epList.pop_back();
			}
		}

		for (int w = 0; w <= width; w += epIncrement)
		{
			for (int d = 0; d <= depth; d += epIncrement)
			{
				if (BinHeight - (z + height) >= shortestHeight)
				{
					if (BinDepth - (y + depth) >= shortestWD || BinWidth - (x + width) >= shortestWD)
					{
						extremePoint newEP;
						newEP.x = x + w; newEP.y = y + d; newEP.z = z + height;
						this->epList.push_back(newEP);
					}
				}
			}
		}
	}

	/**
	Insert an item at an extreme point
	@param the width, depth, height dimensions, x, y, z coordinates (front bottom left corner), and the ID of the item to be placed
	*/
	void insertItemAtEP(int width, int depth, int height, int x, int y, int z, int ID)
	{
		itemWidths.push_back(width);
		itemDepths.push_back(depth);
		itemHeights.push_back(height);
		itemXCoords.push_back(x);
		itemYCoords.push_back(y);
		itemZCoords.push_back(z);
		itemList.push_back(ID);

		//Add new extreme points 
		for (int w = 0; w <= width; w += epIncrement)
		{
			if (BinDepth - (y + depth) >= shortestWD)
			{
				extremePoint newEP;
				newEP.x = x + w; newEP.y = y + depth; newEP.z = z;
				this->epList.push_back(newEP);
			}
		}
		for (int d = 0; d <= depth; d += epIncrement)
		{
			if (BinWidth - (x + width) >= shortestWD)
			{
				extremePoint newEP;
				newEP.x = x + width; newEP.y = y + d; newEP.z = z;
				this->epList.push_back(newEP);
			}
		}

		//Remove obsolete extreme points from epList
		for (int i = epList.size() - 1; i >= 0; i--)
		{
			if ((x <= epList[i].x && x + width > epList[i].x)
				&& (y <= epList[i].y && y + depth > epList[i].y)
				&& (z <= epList[i].z && z + height > epList[i].z))
			{
				extremePoint temp = epList.back();
				epList.back() = epList[i];
				epList[i] = temp;
				epList.pop_back();
			}
		}

		for (int w = 0; w <= width; w += epIncrement)
		{
			for (int d = 0; d <= depth; d += epIncrement)
			{
				if (BinHeight - (z + height) >= shortestHeight)
				{
					if (BinDepth - (y + depth) >= shortestWD || BinWidth - (x + width) >= shortestWD)
					{
						extremePoint newEP;
						newEP.x = x + w; newEP.y = y + d; newEP.z = z + height;
						this->epList.push_back(newEP);
					}
				}
			}
		}
		
		if (z + height > this->height) this->height = z + height;
		this->weight += uniqueItemList[ID].weight;

		//cout << epList.size() << endl;
	}

	/**
	Check the feasibility of placing an item at an extreme point, considering toggled practical constrailts and overlaps
	@param an extreme point and an item object, and width, depth, height dimensions of the item
	@return true if the item can be feasibly placed in the bin
	*/
	bool checkFeasibility(extremePoint EP, Item i, int curW, int curD, int curH, int corner)
	{
		//Set up lists and parameters for the implementation
		cpyUnique = uniqueItemList;
		bool feasibilityFlag = false;
		vector<int> supportingItems;
		vector<int> supportedItems;
		//Tracker for the support on the corners of the item. 1 if a corner is supported.
		vector<int> cornerSupport(4, 0);	//indices for corners->  0: front bottom left	1: back bottom left		2: front bottom right	3: back bottom right
		vector<double> supportAreas;
		vector<double> supportPercentages;
		double totalOverlap = 0;
		int curX, curY, curZ;
		curZ = EP.z;
		if (corner == 0)
		{
			curX = EP.x; curY = EP.y;
		}
		else if (corner == 1)
		{
			curX = EP.x; curY = EP.y - curD;
		}
		else if (corner == 2)
		{
			curX = EP.x - curW; curY = EP.y;
		}
		else if (corner == 3)
		{
			curX = EP.x - curW; curY = EP.y - curD;
		}
		

		//Check if the weight limit is exceeded by the placement of the item
		if (this->weight + i.weight > maxBinWeight)
		{
			infBinWeight++;
			return false;
		}
		
		if (curZ > 0)	//If the item is not being placed at the bottom of a bin
		{
			//Loop over items in the bin
			for (int binIt = 0; binIt < itemWidths.size(); binIt++)
			{
				int binX, binY, binZ, binW, binD, binH;
				binX = this->itemXCoords[binIt];
				binY = this->itemYCoords[binIt];
				binZ = this->itemZCoords[binIt];
				binW = this->itemWidths[binIt];
				binD = this->itemDepths[binIt];
				binH = this->itemHeights[binIt];

				bool overlapFeasible = false;

				//Check if the placed item does NOT overlap with the current item in the bin
				if (curX + curW <= binX) overlapFeasible = true;		//Left
				else if (curY + curD <= binY) overlapFeasible = true;	//Front
				else if (curZ + curH <= binZ) overlapFeasible = true;	//Bottom
				else if (curX >= binX + binW) overlapFeasible = true;	//Right
				else if (curY >= binY + binD) overlapFeasible = true;	//Back
				else if (curZ >= binZ + binH) overlapFeasible = true;	//Top

				//If there is overlap, return false
				if (overlapFeasible == false)
					return false;

				//If the placement is in the height tolerance of an item in the bin
				if (curZ >= binZ + binH && curZ - (binZ + binH) <= LayerHeightTolerance)
				{
					//Calculate their width-depth overlap
					double overlap = calculateOverlap(binX, binY, binW, binD, curX, curY, curW, curD, uniqueItemList[this->itemList[binIt]].supportType, uniqueItemList[this->itemList[binIt]].edgeReduceWidth, uniqueItemList[this->itemList[binIt]].edgeReduceDepth);

					if (overlap == 0) continue;

					//Set corner support values
					if (curX >= binX && curX <= binX + binW && curY >= binY && curY <= binY + binD) cornerSupport[0] = 1;
					if (curX >= binX && curX <= binX + binW && curY + curD >= binY && curY + curD <= binY + binD) cornerSupport[1] = 1;
					if (curX + curW >= binX && curX + curW <= binX + binW && curY >= binY && curY <= binY + binD) cornerSupport[2] = 1;
					if (curX + curW >= binX && curX + curW <= binX + binW && curY + curD >= binY && curY + curD <= binY + binD) cornerSupport[3] = 1;

					//Set up support(overlap) values so that weight distribution can be calculated
					supportAreas.push_back(overlap);
					totalOverlap += overlap;
					supportedItems.push_back(this->itemList[binIt]);
				}
			}//for (int binIt = 0; binIt < itemWidths.size(); binIt++)


			//Check vertical support
			if ((cornerSupport[0] == 0 || cornerSupport[1] == 0 || cornerSupport[2] == 0 || cornerSupport[3] == 0))
			{
				if ((double)100 * totalOverlap / (curW * curD) < itemSupportPercentage)
				{
					infAreaSupport++;
					return false;	//If all the corners are not supported, and the support limit is not satisfied, return false
				}
			}
			else if ((double)100 * totalOverlap / (curW * curD) < (double)100 * (4 * edgeSupportThickness * edgeSupportThickness) / (curW * curD))
			{
				inf4CSupport++;
				return false;	//If all the corners are supported, decrease the support limit
			}

			//Set up queues for BFS implementation, for weight distribution for load cap check
			queue<int> iteratorList;
			queue<double> weightList;

			if (loadCapToggle == 1)
			{
				//Calculate weight distribution of the current item to all the items that support it in the bin
				for (int j = 0; j < supportedItems.size(); j++)
				{
					supportPercentages.push_back((double)supportAreas[j] / totalOverlap);
					iteratorList.push(supportedItems[j]);
					weightList.push(i.weight * supportPercentages.back());
				}

				//Update the total weights on items through a BFS
				while (iteratorList.size() > 0)
				{
					int curItem = iteratorList.front();
					iteratorList.pop();
					cpyUnique[curItem].totalSupportedWeight += weightList.front();

					if (cpyUnique[curItem].totalSupportedWeight > cpyUnique[curItem].loadCap)
					{
						feasibilityFlag = true;
						infLoadBearing++;
						break;
					}

					for (int j = 0; j < cpyUnique[curItem].supportedItems.size(); j++)
					{
						iteratorList.push(cpyUnique[curItem].supportedItems[j]);
						weightList.push(cpyUnique[curItem].supportPercentages[j] * weightList.front());
					}

					weightList.pop();
				}
			}
			//If not infeasible, make uniqueItemList = copied list and update the current item's support info in it, if infeasible, return false
			if (feasibilityFlag == true)
			{
				cpyUnique.clear();
				iteratorList.empty();
				weightList.empty();
				supportingItems.clear();
				supportedItems.clear();
				supportAreas.clear();
				supportPercentages.clear();
				return false;
			}

			cpyUnique[i.ID].supportedItems = supportedItems;
			cpyUnique[i.ID].supportPercentages = supportPercentages;

			return true;
		}
		else
		{
			//If the item is placed at the bottom of a bin, vertical support and load cap checks are not needed
			for (int binIt = 0; binIt < itemWidths.size(); binIt++)
			{
				int binX, binY, binZ, binW, binD, binH;
				binX = this->itemXCoords[binIt];
				binY = this->itemYCoords[binIt];
				binZ = this->itemZCoords[binIt];
				binW = this->itemWidths[binIt];
				binD = this->itemDepths[binIt];
				binH = this->itemHeights[binIt];

				bool overlapFeasible = false;

				//Check if the placed item does NOT overlap with the current item in the bin
				if (curX + curW <= binX) overlapFeasible = true;		//Left
				else if (curY + curD <= binY) overlapFeasible = true;	//Front
				else if (curZ + curH <= binZ) overlapFeasible = true;	//Bottom
				else if (curX >= binX + binW) overlapFeasible = true;	//Right
				else if (curY >= binY + binD) overlapFeasible = true;	//Back
				else if (curZ >= binZ + binH) overlapFeasible = true;	//Top

				if (overlapFeasible == false) return false;
			}//for (int binIt = 0; binIt < itemWidths.size(); binIt++)

			return true;
		}
	}

	/**
	Initiate extreme point list of a bin, for the bottom
	*/
	void initiateEPList()
	{
		for (int x = 0; x < BinWidth; x+=epIncrement)
		{
			for (int y = 0; y < BinDepth; y+=epIncrement)
			{
				extremePoint newEP;
				newEP.x = x; newEP.y = y; newEP.z = this->height;
				this->epList.push_back(newEP);
			}
		}
		this->layerHeight = height;
	}

	/**
	Place items in a layer to the bin
	@param a layer object
	*/
	void unpackLayer(Layer &curLayer)
	{
		int x, y, z, w, d, h;

		for (int i = 0; i < curLayer.itemList.size(); i++)
		{
			x = curLayer.xVec[i];
			y = curLayer.yVec[i];
			z = this->height + curLayer.zVec[i];
			w = curLayer.wList[i]; 
			d = curLayer.dList[i];
			h = curLayer.hList[i];
			this->insertItemFromLayer(w, d, h, x, y, z);
			this->itemList.push_back(curLayer.itemList[i].ID);
		}

		if (this->height == 0)
		{
			//If the layer is being placed at the bottom, update the support and load cap information for the top items in the super items
			for (int i = 0; i < this->itemList.size(); i++)
			{
				if (this->itemZCoords[i] > 0)
				{
					for (int j = 0; j < this->itemList.size(); j++)
					{
						if (this->itemList[i] == this->itemList[j])
							continue;

						int x1 = this->itemXCoords[j]; int y1 = this->itemYCoords[j]; int w1 = this->itemWidths[j]; int d1 = this->itemDepths[j];
						int x2 = this->itemXCoords[i]; int y2 = this->itemYCoords[i]; int w2 = this->itemWidths[i]; int d2 = this->itemDepths[i];

						double overlap = calculateOverlap(x1, y1, w1, d1, x2, y2, w2, d2, uniqueItemList[this->itemList[j]].supportType, uniqueItemList[this->itemList[j]].edgeReduceWidth, uniqueItemList[this->itemList[j]].edgeReduceDepth);

						if (overlap == 0)
							continue;

						uniqueItemList[this->itemList[i]].supportedItems.push_back(this->itemList[j]);
						uniqueItemList[this->itemList[i]].supportPercentages.push_back(overlap);
					}
				}
			}

			for (int i = 0; i < this->itemList.size(); i++)
			{
				double totalSupport = 0;
				for (int j = 0; j < uniqueItemList[this->itemList[i]].supportPercentages.size(); j++)
				{
					totalSupport += uniqueItemList[this->itemList[i]].supportPercentages[j];
				}
				for (int j = 0; j < uniqueItemList[this->itemList[i]].supportPercentages.size(); j++)
				{
					uniqueItemList[this->itemList[i]].supportPercentages[j] /= totalSupport;
				}
			}
		}//if (this->height == 0)
	}
};

//List of bins
vector<Bin> binList;

/**
Check if at least one item in a layer is placed before
@param a list that tracks the already placed items, a layer object
@return true if the layer has at least one item that is placed before
*/
bool isItCovered(vector<int> coveredItems, Layer curLayer)
{
	bool coveredFlag = false;

	//Check if the layer has any previously covered items
	for (int j = 0; j < curLayer.itemList.size(); j++)
	{
		//If it is a normal item
		if (curLayer.itemList[j].SIList.size() == 0)
		{
			if (coveredItems[curLayer.itemList[j].ID] == 1)
			{
				coveredFlag = true;
				break;
			}
		}
		//If it is a super-item
		else
		{
			//Loop through items in the super-item
			bool secondCoveredFlag = false;
			for (int k = 0; k < curLayer.itemList[j].SIList.size(); k++)
			{
				if (coveredItems[curLayer.itemList[j].SIList[k].ID] == 1)
				{
					coveredFlag = true;
					secondCoveredFlag = true;
					break;
				}
			}
			if (secondCoveredFlag == true)
				break;
		}
	}
	return coveredFlag;
}

void removeCoveredItems(vector<int> &coveredItems, Layer &curLayer) 
{
	//Check if the layer has any previously covered items
	for (int j = 0; j < curLayer.itemList.size(); j++)
	{
		//If it is a normal item
		if (curLayer.itemList[j].SIList.size() == 0)
		{
			if (coveredItems[curLayer.itemList[j].ID] == 1)
			{
				curLayer.deleteItem(j);
				j--;
			}
		}
		//If it is a super-item
		else
		{
			//Loop through items in the super-item
			bool secondCoveredFlag = false;
			for (int k = 0; k < curLayer.itemList[j].SIList.size(); k++)
			{
				if (coveredItems[curLayer.itemList[j].SIList[k].ID] == 1)
				{
					curLayer.deleteItem(j);
					j--;
					break;
				}
			}
		}
	}
}

/**
Update the placed item tracker after a layer is placed in a bin
@param a list that tracks the already placed items, a layer object
*/
void updateCoveredList(vector<int> &coveredItems, Layer curLayer)
{
	for (int i = 0; i < curLayer.itemList.size(); i++)
	{
		Item curItem = curLayer.itemList[i];

		if (curItem.SIList.size() == 0)
			coveredItems[curItem.ID] = 1;
		else
		{
			for (int j = 0; j < curItem.SIList.size(); j++)
			{
				coveredItems[curItem.SIList[j].ID] = 1;
			}
		}
	}
}

/**
Calculate the merit function value of an item-extreme point pair. NOT USED ANYMORE.
@param x, y, z coordinates of the extreme point and width, depth, height, sequence and load cap parameters of an item
@return the merit function value
*/
double calculateMerit(int x, int y, int z, int height, int width, int depth, int seqNo, int loadCap)
{
	return (double)100000000 * (1 + (double)(BinHeight - (z + height))/(BinHeight - height)) + 100 * (1 + (double)(BinDepth - (y + depth)) / (BinDepth - depth)) + 10 * (1 + (double)(BinWidth - (x + width)) / (BinWidth - width));
	//return 1000000 * (1 + (double)(BinHeight - z) / (BinHeight)) * (1 + ((double)(width * depth) / (BinWidth * BinDepth))) * (1 + loadCap / maxLoadCap) + 100 * (1 + (double)(BinDepth - y) / (BinDepth)) + 10 * (1 + (double)(BinWidth - x) / (BinWidth));
}

/**
Place the remaining items after all possible layers are placed into a bin
@param a bin object, a list of remaining items
*/
void placeLeftovers(Bin &bin, vector<Item> &leftovers)
{
	//Main item loop for placement
	sort(leftovers.begin(), leftovers.end(), &sortItemsHorizontalArea);
	bin.initiateEPList();
	int layerHeight = 0;
	for (int i = 0; i < bin.binLayers.size(); i++)
	{
		layerHeight += bin.binLayers[i].layerHeight;
	}
	
	//Loop as long as there are items to be placed
	while (leftovers.size() > 0)
	{
		//Sort extreme points based on the sortEP sortation rule
		sort(bin.epList.begin(), bin.epList.end(), &sortEP);
		bool itemPlaced = false;	//Flag for checking if any item has been placed
		int bestX, bestY, bestZ;
		Item bestItem;
		int bestIndex = -1;
		vector<Item> bestCopyItemList = uniqueItemList;

		for (int i = 0; i < leftovers.size(); i++)
		{
			Item curItem = leftovers[i];
			
			//Loop through all the extreme points
			for (int j = 0; j < bin.epList.size(); j++)
			{
				extremePoint curEP = bin.epList[j];

				//Try both orientations of the item
				for (int o = 0; o < 2; o++)
				{
					if (o == 1)
						curItem.rotateItem();
					for (int c = 3; c >= 0; c--)
					{
						switch (c)
						{
						case 0:
							//If an item is out of bounds of a bin, go to next extreme point
							if (curEP.x + curItem.w > BinWidth || curEP.y + curItem.d > BinDepth || curEP.z + curItem.h > BinHeight)
								continue;
							break;
						case 1:
							//If an item is out of bounds of a bin, go to next extreme point
							if (curEP.x + curItem.w > BinWidth || curEP.y - curItem.d < 0 || curEP.z + curItem.h > BinHeight)
								continue;
							break;
						case 2:
							//If an item is out of bounds of a bin, go to next extreme point
							if (curEP.x - curItem.w < 0 || curEP.y + curItem.d > BinDepth || curEP.z + curItem.h > BinHeight)
								continue;
							break;
						case 3:
							//If an item is out of bounds of a bin, go to next extreme point
							if (curEP.x - curItem.w < 0 || curEP.y - curItem.d < 0 || curEP.z + curItem.h > BinHeight)
								continue;
							break;


						}
						//Check if the placement is feasible
						if (bin.checkFeasibility(curEP, curItem, curItem.w, curItem.d, curItem.h, c))
						{
							itemPlaced = true;
							//double merit = calculateMerit(curEP.x, curEP.y, curEP.z, curItem.h, curItem.w, curItem.d, curItem.planogramSeq, curItem.loadCap);							
							bestItem = curItem;
							if (c == 0)
							{
								bestX = curEP.x; bestY = curEP.y; bestZ = curEP.z;
							}
							else if (c == 1)
							{
								bestX = curEP.x; bestY = curEP.y - curItem.d; bestZ = curEP.z;
							}
							else if (c == 2)
							{
								bestX = curEP.x - curItem.w; bestY = curEP.y; bestZ = curEP.z;
							}
							else if (c == 3)
							{
								bestX = curEP.x - curItem.w; bestY = curEP.y - curItem.d; bestZ = curEP.z;
							}
							bestIndex = i;
							bestCopyItemList = cpyUnique;
							cpyUnique.clear();

							//Since both the items and the extreme points are sorted properly, we can exit the loop if there is a feasible placement
							//This guarantees that the first feasible placement provides the maximum merit function value regardless
							break;
						}
					}

					if (itemPlaced == true)
						break;

				}//for (int o = 0; o < 2; o++)
				if (itemPlaced == true)
					break;
			}//for (int j = 0; j < bin.epList.size(); j++)
			if (itemPlaced == true)
				break;
		}//for (int i = 0; i < leftovers.size(); i++)

		//If no feasible placement is found in the remaining item and extreme point list, break out of the main loop
		if (itemPlaced == false)
			break;
		else
		{
			bin.insertItemAtEP(bestItem.w, bestItem.d, bestItem.h, bestX, bestY, bestZ, bestItem.ID);
			leftovers.erase(leftovers.begin() + bestIndex);
			std::cout << "Leftover size is: " << leftovers.size() << endl;
			coveredList[bestItem.ID] = 1;
			uniqueItemList = bestCopyItemList;
			bestCopyItemList.clear();

			if (bestZ >= layerHeight)
			{
				for (int ep = bin.epList.size() - 1; ep >= 0; ep--)
				{
					if (bin.epList[ep].z < layerHeight)
					{
						extremePoint tempEP = bin.epList.back();
						bin.epList.back() = bin.epList[ep];
						bin.epList[ep] = tempEP;
						bin.epList.pop_back();
					}
				}
			}
		}
	}//while (leftovers.size() > 0)
}

/**
Check the feasibility of placing a layer on top of the layers in a bin based on toggled practical constraints and overlaps
NOTE: Implementation of this function is similar to the feasibility check in the bin class definition, refer to that one
@param a layer object to be placed, and a bin object
@return true if the placement is feasible
*/
bool layerFeasibility(const Layer &topLayer, const Bin &currentBin)
{
	vector<Item> copyItemList = uniqueItemList;
	for (int i = topLayer.itemList.size() - 1; i >= 0; i--)
	{
		bool feasibilityFlag = false;
		vector<int> supportingItems;
		vector<int> supportedItems;
		vector<int> cornerSupport(4, 0);
		vector<double> supportAreas;
		vector<double> supportPercentages;
		Item currentItem = topLayer.itemList[i];
		double totalOverlap = 0;
		int curX, curY, curZ, curW, curD, curH;

		curX = topLayer.xVec[i];
		curY = topLayer.yVec[i];
		curZ = topLayer.zVec[i];
		curW = topLayer.wList[i];
		curD = topLayer.dList[i];
		curH = topLayer.hList[i];

		if (curZ == 0)
		{
			for (int j = 0; j < currentBin.itemWidths.size(); j++)
			{
				if (currentBin.height - (currentBin.itemZCoords[j] + currentBin.itemHeights[j]) <= LayerHeightTolerance)
				{
					int binX, binY, binW, binD;
					binX = currentBin.itemXCoords[j];
					binY = currentBin.itemYCoords[j];
					binW = currentBin.itemWidths[j];
					binD = currentBin.itemDepths[j];

					double overlap = calculateOverlap(binX, binY, binW, binD, curX, curY, curW, curD, uniqueItemList[currentBin.itemList[j]].supportType, uniqueItemList[currentBin.itemList[j]].edgeReduceWidth, uniqueItemList[currentBin.itemList[j]].edgeReduceDepth);

					if (overlap == 0) continue;

					if (curX >= binX && curX <= binX + binW && curY >= binY && curY <= binY + binD) cornerSupport[0] = 1;
					if (curX >= binX && curX <= binX + binW && curY + curD >= binY && curY + curD <= binY + binD) cornerSupport[1] = 1;
					if (curX + curW >= binX && curX + curW <= binX + binW && curY >= binY && curY <= binY + binD) cornerSupport[2] = 1;
					if (curX + curW >= binX && curX + curW <= binX + binW && curY + curD >= binY && curY + curD <= binY + binD) cornerSupport[3] = 1;

					supportAreas.push_back(overlap);
					totalOverlap += overlap;
					supportedItems.push_back(currentBin.itemList[j]);
				}
			}
		}
		else
		{
			for (int j = 0; j < topLayer.itemList.size() && topLayer.itemList[j].ID != currentItem.ID; j++)
			{
				if (curZ - (topLayer.zVec[j] + topLayer.hList[j]) <= LayerHeightTolerance)
				{
					int layerX, layerY, layerW, layerD;
					layerX = topLayer.xVec[j];
					layerY = topLayer.yVec[j];
					layerW = topLayer.wList[j];
					layerD = topLayer.dList[j];

					double overlap = calculateOverlap(layerX, layerY, layerW, layerD, curX, curY, curW, curD, topLayer.itemList[j].supportType, topLayer.itemList[j].edgeReduceWidth, topLayer.itemList[j].edgeReduceDepth);

					if (overlap == 0) continue;

					supportAreas.push_back(overlap);
					totalOverlap += overlap;
					supportedItems.push_back(topLayer.itemList[j].ID);
				}
			}
		}
		
		//Check vertical support
		if ((cornerSupport[0] == 0 || cornerSupport[1] == 0 || cornerSupport[2] == 0 || cornerSupport[3] == 0) && (double)100 * totalOverlap / (curW * curD) < itemSupportPercentage)
		{
			if((double)100 * totalOverlap / (curW * curD) < itemSupportPercentage)
				return false;
		}
		else if ((double)100 * totalOverlap / (curW * curD) < (double)100 * (4 * edgeSupportThickness * edgeSupportThickness) / (curW * curD))
			return false;

		queue<int> iteratorList;
		queue<double> weightList;

		if (loadCapToggle == 1)
		{
			for (int j = 0; j < supportedItems.size(); j++)
			{
				supportPercentages.push_back((double)supportAreas[j] / totalOverlap);
				iteratorList.push(supportedItems[j]);
				weightList.push(currentItem.weight * supportPercentages.back());
			}

			//Update the total weights on items through a BFS
			while (iteratorList.size() > 0)
			{
				int curItem = iteratorList.front();
				iteratorList.pop();
				copyItemList[curItem].totalSupportedWeight += weightList.front();

				if (copyItemList[curItem].totalSupportedWeight > copyItemList[curItem].loadCap)
				{
					feasibilityFlag = true;
					break;
				}

				for (int j = 0; j < copyItemList[curItem].supportedItems.size(); j++)
				{
					iteratorList.push(copyItemList[curItem].supportedItems[j]);
					weightList.push(copyItemList[curItem].supportPercentages[j] * weightList.front());
				}

				weightList.pop();
			}
		}
		//If not infeasible, make uniqueItemList = copied list and update the current item's support info in it, if infeasible, return false
		
		if (feasibilityFlag == true)
		{
			copyItemList.clear();
			iteratorList.empty();
			weightList.empty();
			supportingItems.clear();
			supportedItems.clear();
			supportAreas.clear();
			supportPercentages.clear();
			return false;
		}
		

		copyItemList[currentItem.ID].supportedItems = supportedItems;
		copyItemList[currentItem.ID].supportPercentages = supportPercentages;
	}
	uniqueItemList = copyItemList;
	copyItemList.clear();

	return true;
}

/**
Bin construction heuristic
*/
void constructBinsVerticalSupport()
{
	infLoadBearing = 0;
	infAreaSupport = 0;
	inf4CSupport = 0;
	infBinWeight = 0;
	//Necessary lists
	vector<Bin> newBinList;
	vector<Bin> openBinList;
	if (planogramSeqToggle == 1)
		openBinList.resize(1);
	else
		openBinList.resize(ceil(totalVol / (BinWidth * BinDepth * BinHeight / 1000000)) + 1);

	//cout << totalVol << endl;

	//Populate the extreme point list with initial EPs
	for (int i = 0; i < openBinList.size(); i++)
	{
		openBinList[i].initiateEPList();
	}

	vector<Item> leftoverItems;

	removeDuplicateLayers();

	//Calculate layer densities
	for (int i = 0; i < layerList.size(); i++)
	{
		//layerList[i].calculateLayerOccupancy();

		double coveredSpace = 0;

		for (int j = 0; j < layerList[i].itemList.size(); j++)
		{
			if (academicToggle == 1)
				coveredSpace += layerList[i].itemList[j].w * layerList[i].itemList[j].d;
			else
				coveredSpace += layerList[i].itemList[j].topSurfaceArea;
			//coveredSpace += itemList[i].w * itemList[i].d;

		}
		if (academicToggle == 1)
			layerList[i].layerOccupancy = (coveredSpace / (BinWidth * BinDepth)) * 100;
		else
			layerList[i].layerOccupancy = (coveredSpace / (BinWidth * BinDepth)) * 100;

		if (layerList[i].layerOccupancy < 40.0)
		{
			Layer tempLayer = layerList.back();
			layerList.back() = layerList[i];
			layerList[i] = tempLayer;
			layerList.pop_back();
			i--;
		}
	}

	//Remove layers with less than 60% density
	/*for (int i = 0; i < layerList.size(); i++)
	{
		if (layerList[i].layerOccupancy < 60.0)
		{
			Layer tempLayer = layerList.back();
			layerList.back() = layerList[i];
			layerList[i] = tempLayer;
			layerList.pop_back();
			i--;
		}
	}*/
	/*vector<Layer> layerList1;
	vector<Layer> layerList2;
	vector<Layer> layerList3;
	vector<Layer> layerList4;
	vector<Layer> layerList5;*/

	//Sort the layers based on decreasing density
	sort(layerList.begin(), layerList.end());

	/*while (layerList.size() > 0)
	{
		if (layerList.back().layerOccupancy >= 80.0)
		{
			layerList1.push_back(layerList.back());
			layerList.pop_back();
		}
		else if(layerList.back().layerOccupancy >= 70.0)
		{
			layerList2.push_back(layerList.back());
			layerList.pop_back();
		}
		else if (layerList.back().layerOccupancy >= 60.0)
		{
			layerList3.push_back(layerList.back());
			layerList.pop_back();
		}
		else if (layerList.back().layerOccupancy >= 50.0)
		{
			layerList4.push_back(layerList.back());
			layerList.pop_back();
		}
		else if (layerList.back().layerOccupancy >= 40.0)
		{
			layerList5.push_back(layerList.back());
			layerList.pop_back();
		}
	}
	
	sort(layerList1.begin(), layerList1.end(), &sortLayersLoadCap);
	sort(layerList2.begin(), layerList2.end(), &sortLayersLoadCap);
	sort(layerList3.begin(), layerList3.end(), &sortLayersLoadCap);
	sort(layerList4.begin(), layerList4.end(), &sortLayersLoadCap);
	sort(layerList5.begin(), layerList5.end(), &sortLayersLoadCap);

	layerList = layerList1;
	for (int i = 0; i < layerList2.size(); i++)
	{
		layerList.push_back(layerList2[i]);
	}
	for (int i = 0; i < layerList3.size(); i++)
	{
		layerList.push_back(layerList3[i]);
	}
	for (int i = 0; i < layerList4.size(); i++)
	{
		layerList.push_back(layerList4[i]);
	}
	for (int i = 0; i < layerList5.size(); i++)
	{
		layerList.push_back(layerList5[i]);
	}
	layerList1.clear();
	layerList2.clear();
	layerList3.clear();
	layerList4.clear();
	layerList5.clear();*/

	//Sort the layers based on decreasing alpha
	//sort(layerList.begin(), layerList.end(), &sortLayers);

	//Sort the layers based on decreasing average load cap
	//sort(layerList.begin(), layerList.end(), &sortLayersLoadCap);

	//****************************************************************************
	//Bottom-Up bin construction with vertical support using all generated layers
	//****************************************************************************
	vector<Layer> copyLayerList;
	copyLayerList = layerList;

	if (binPlacementHeuristic == 1)
	{
		//Try to place as many layers as possible to the open bins
		while (layerList.size() > 0)
		{
			int bestLayerIndex = -1;
			int bestBinIndex = -1;
			double bestDensity = 0;
			Layer bestLayer;

			//Loop through layers
			for (int i = 0; i < layerList.size(); i++)
			{
				Layer curLayer = layerList[i];

				//Add layers to all open bins if they are empty
				bool addedFlag = false;
				bool jumpFlag = false;
				for (int j = 0; j < openBinList.size(); j++)
				{
					if (openBinList[j].binLayers.size() == 0)
					{
						if (curLayer.layerOccupancy < 60.0)
						{
							jumpFlag = true;
							break;
						}
						//Space the layer
						if (BinWidth > 700)
							spaceLayer(curLayer);
						curLayer.unpack();
						openBinList[j].binLayers.push_back(curLayer);
						openBinList[j].unpackLayer(curLayer);
						openBinList[j].height += curLayer.layerHeight;
						openBinList[j].weight += curLayer.weight;
						updateCoveredList(coveredList, curLayer);
						addedFlag = true;
						bestLayerIndex = -2;
						break;
					}
				}

				if (jumpFlag == true)
					continue;

				if (addedFlag == true)
					break;


				for (int j = 0; j < openBinList.size(); j++)
				{
					if (openBinList[j].weight + curLayer.weight > maxBinWeight)
						break;

					Layer tempLayer = curLayer;
					//Space the layer to maximize support
					if(BinWidth > 700)
						maximizeSupport(openBinList[j].binLayers.back(), curLayer);
					curLayer.unpack();
					//If the layer is 100% supported from the bottom, add the layer
					if (layerFeasibility(curLayer, openBinList[j]))
					{
						if (curLayer.layerOccupancy > bestDensity)
						{
							bestDensity = curLayer.layerOccupancy;
							bestLayerIndex = i;
							bestBinIndex = j;
							bestLayer = curLayer;
						}
					}
					else
						curLayer = tempLayer;

					if (bestBinIndex >= 0)
						break;
				}//for (int j = 0; j < openBinList.size(); j++)
				if (bestLayerIndex >= 0)
					break;
			}//for (int i = 0; i < layerList.size(); i++)

			if (bestLayerIndex >= 0)
			{
				openBinList[bestBinIndex].binLayers.push_back(bestLayer);
				openBinList[bestBinIndex].unpackLayer(bestLayer);

				openBinList[bestBinIndex].height += bestLayer.layerHeight;
				openBinList[bestBinIndex].weight += bestLayer.weight;
				updateCoveredList(coveredList, bestLayer);

				//Add bin to the final bin list if it is full
				if (openBinList[bestBinIndex].height > normalLayerHeight)
				{
					newBinList.push_back(openBinList[bestBinIndex]);
					openBinList.erase(openBinList.begin() + bestBinIndex);
				}
			}

			if (bestLayerIndex == -1)
			{
				//layerList.clear();
				break;
			}
		
			
			if (openBinList.size() == 0)
				break;

			for (int i = layerList.size() - 1; i >= 0; i--)
			{
				/*if (isItCovered(coveredList, layerList[i]))
				{
					Layer tempLayer = layerList[i];
					layerList[i] = layerList.back();
					layerList.back() = tempLayer;
					layerList.pop_back();
				}*/

				removeCoveredItems(coveredList, layerList[i]);
				layerList[i].calculateLayerOccupancy();
			}

			//Remove layers with less than 60% density
			for (int i = 0; i < layerList.size(); i++)
			{
				if (layerList[i].layerOccupancy < 40.0)
				{
					Layer tempLayer = layerList.back();
					layerList.back() = layerList[i];
					layerList[i] = tempLayer;
					layerList.pop_back();
					i--;
				}
			}

			//Sort the layers based on decreasing density
			sort(layerList.begin(), layerList.end());

			//Sort the layers based on decreasing average load cap
			//sort(layerList.begin(), layerList.end(), &sortLayersLoadCap);

		}//while (layerList.size() > 0)
	}//if (binPlacementHeuristic == 1)

	//If there are any open bins remaining, add them to the new bin list
	if (openBinList.size() > 0)
		for (int i = 0; i < openBinList.size(); i++)
			newBinList.push_back(openBinList[i]);
		
	//Populate the leftover item list with items that are not yet covered
	for (int i = 0; i < itemList.size(); i++)
	{
		if (coveredList[itemList[i].ID] == 0)
			leftoverItems.push_back(itemList[i]);
	}

	//Sort leftover items based on &sortItemsHorizontalArea rule
	sort(leftoverItems.begin(), leftoverItems.end(), &sortItemsHorizontalArea);

	int leftoverStart = leftoverItems.size();

	//If there are any leftover items, try to place them on top of layers in the new bins
	if (leftoverItems.size() > 0)
	{
		for (int i = 0; i < newBinList.size(); i++)
		{
			placeLeftovers(newBinList[i], leftoverItems);
			newBinList[i].epList.clear();
			binList.push_back(newBinList[i]);
		}
	}

	nbLeftoverItems += (leftoverStart - leftoverItems.size());

	newBinList.clear();

	//If planogram sequencing is not used, open bins and place all of the remaining items
	if (planogramSeqToggle == 0)
	{
		while (leftoverItems.size() > 0)
		{
			nbLeftoverItems += leftoverItems.size();
			Bin newBin;
			newBin.initiateEPList();
			placeLeftovers(newBin, leftoverItems);
			newBin.epList.clear();
			binList.push_back(newBin);
		}
	}

	itemList = leftoverItems;
}

/**
Write results
NOTE: The values at the end of each line are there to ensure compatibility with 
the previous pallet visualization code from WanOpt. They can safely be removed.
@param file name
*/
void writeBins(string filename)
{
	//std::cout << "Writing results to files." << endl;
	vector<Bin> newList;
	//int binListSize = (int)(binList.size() / 2);
	for (int i = 0; i < binList.size(); i++)
	{
		if (i % 2 == 1)
		{
			for (int j = 0; j < binList[i].itemList.size(); j++)
			{
				binList[i - 1].itemXCoords.push_back(binList[i].itemXCoords[j] + BinWidth);
				binList[i - 1].itemYCoords.push_back(binList[i].itemYCoords[j]);
				binList[i - 1].itemZCoords.push_back(binList[i].itemZCoords[j]);
				binList[i - 1].itemWidths.push_back(binList[i].itemWidths[j]);
				binList[i - 1].itemDepths.push_back(binList[i].itemDepths[j]);
				binList[i - 1].itemHeights.push_back(binList[i].itemHeights[j]);
			}
			newList.push_back(binList[i - 1]);
		}
	}

	if (binList.size() % 2 == 1)
		newList.push_back(binList.back());

	binList = newList;

	for (int b = 0; b < binList.size(); b++)
	{
		ofstream resultingBins;
		char fileName[100];
		sprintf(fileName, "Results/");
		sprintf(fileName + strlen(fileName), filename.c_str());
		sprintf(fileName + strlen(fileName), "/currentbin%d.txt", b);
		resultingBins.open(fileName);

		for (int i = 0; i < binList[b].itemWidths.size(); i++)
		{
			resultingBins << binList[b].itemXCoords[i] << " "
				<< binList[b].itemYCoords[i] << " "
				<< binList[b].itemZCoords[i] << " "
				<< binList[b].itemWidths[i] << " "
				<< binList[b].itemDepths[i] << " "
				<< binList[b].itemHeights[i] << " "
				<< "5" << " "
				<< "11585 2 128650" << endl;
		}
		resultingBins.close();
		//calcBins.push_back(newBin);
	}//for (int i = 0; i < binLayers.size(); i++)
}

#endif