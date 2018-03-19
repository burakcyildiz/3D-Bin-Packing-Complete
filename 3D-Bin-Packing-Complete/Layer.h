#ifndef LAYER_H
#define LAYER_H

#include "Item.h"
#include "Common.h"

//Class definition for layers
struct Layer
{
	double layerOccupancy;	//Layer density (in percentage) for width-depth dimensions
	int layerHeight;		//Height of the layer
	double alpha;			//Decision variable value obtained from the mathematical model for the layer (column, in this case)
	int weight;				//Total weight of the items in the layer

	vector<Item> itemList;	//List of items in the layer
	vector<int> orientationList;	//	0: if unchanged		1: if turned (in width-depth plane)
	vector<int> xVec, yVec, zVec, wList, dList, hList;	//List of x, y, z coordinates (of front bottom left corners) and width, depth, height dimensions of items in the layer

	//Default constructor
	struct Layer()
	{
		layerHeight = 0;
		alpha = 0;
		weight = 0;
	}

	//Destructor
	~Layer()
	{
		itemList.clear();
		orientationList.clear();
		xVec.clear();
		yVec.clear();
		zVec.clear();
		wList.clear();
		dList.clear();
		hList.clear();
	}

	/**
	Insert item into layer
	@param an item object, the orientation by which it will be placed, and the x and y coordinates for the front bottom left corner of the item
	*/
	void insertItem(Item i, int orientation, int xCoord, int yCoord)
	{
		itemList.push_back(i);
		orientationList.push_back(orientation);
		xVec.push_back(xCoord);
		yVec.push_back(yCoord);
	}

	/**
	Insert item into layer
	@param an item object, coordinates of the front bottom left corner and the width, depth, height dimensions of the item
	*/
	void insertItem(Item i, int x, int y, int z, int w, int d, int h)
	{
		itemList.push_back(i);
		xVec.push_back(x);
		yVec.push_back(y);
		zVec.push_back(z);
		wList.push_back(w);
		dList.push_back(d);
		hList.push_back(h);
	}

	/**
	Delete an item from the layer
	@param the index of the item in the layer
	*/
	void deleteItem(int index)
	{
		itemList.erase(itemList.begin() + index);
		orientationList.erase(orientationList.begin() + index);
		xVec.erase(xVec.begin() + index);
		yVec.erase(yVec.begin() + index);
	}

	/**
	Replace an item in the layer with another item
	@param the index of the item in the layer to be replaced, the new item object, and the orientation by which it will be placed
	*/
	void replaceItem(int index, Item i, int orientation)
	{
		int width, depth;
		if (orientationList[index] == 0)
		{
			width = itemList[index].w;
			depth = itemList[index].d;
		}
		else if (orientationList[index] == 1)
		{
			width = itemList[index].d;
			depth = itemList[index].w;
		}
		//Place the new item to center the deleted item
		int x = (width - i.w) / 2;
		int y = (depth - i.d) / 2;
		xVec[index] = x;
		yVec[index] = y;
		itemList[index] = i;
		orientationList[index] = orientation;
	}

	/**
	Calculate the total width depth density of the top surface of a layer
	*/
	void calculateLayerOccupancy()
	{
		double coveredSpace = 0;

		for (int i = 0; i < this->itemList.size(); i++)
		{
			if (academicToggle == 1)
				coveredSpace += this->itemList[i].w * this->itemList[i].d;
			else
				coveredSpace += this->itemList[i].topSurfaceArea;
				//coveredSpace += itemList[i].w * itemList[i].d;
			
		}
		if(academicToggle == 1)
			this->layerOccupancy = (coveredSpace / (BinWidth * BinDepth)) * 100;
		else
			this->layerOccupancy = (coveredSpace / (BinWidth * BinDepth)) * 100;
	}

	/**
	Unpack super items so that every item is singular in the layer, and orientation check is not needed for items
	*/
	void unpack()
	{
		//Create new vectors for the coordinates, item list and dimensions
		vector<Item> newItemList;
		vector<int> newxVec, newyVec, newzVec, newwList, newdList, newhList;

		for (int i = 0; i < this->itemList.size(); i++)
		{
			//If the item is not a super item
			if (this->itemList[i].SIList.size() == 0)
			{
				newItemList.push_back(this->itemList[i]);
				newxVec.push_back(this->xVec[i]);
				newyVec.push_back(this->yVec[i]);
				newzVec.push_back(0);
				newhList.push_back(this->itemList[i].h);
				if (this->orientationList[i] == 0)
				{
					newwList.push_back(this->itemList[i].w);
					newdList.push_back(this->itemList[i].d);
				}
				//If the item is turned, update the dimensions
				else
				{
					newwList.push_back(this->itemList[i].d);
					newdList.push_back(this->itemList[i].w);
				}
			}
			//If the item is a super item
			else
			{
				int itemCt = 0;
				//Loop through the layers of items in the super item
				for (int j = 0; j < this->itemList[i].nbItemsInSILayer.size(); j++)
				{
					//If the layer in the super item has only 1 item
					if (this->itemList[i].nbItemsInSILayer[j] == 1)
					{
						newItemList.push_back(this->itemList[i].SIList[itemCt]);
						newxVec.push_back(this->xVec[i] + this->itemList[i].SIx[itemCt]);
						newyVec.push_back(this->yVec[i] + this->itemList[i].SIy[itemCt]);
						newzVec.push_back(this->itemList[i].SIz[itemCt]);
						newhList.push_back(this->itemList[i].SIh[itemCt]);
						if (this->orientationList[i] == 0)
						{
							newwList.push_back(this->itemList[i].SIw[itemCt]);
							newdList.push_back(this->itemList[i].SId[itemCt]);
						}
						//If the item is turned, update the dimensions
						else
						{
							newwList.push_back(this->itemList[i].SId[itemCt]);
							newdList.push_back(this->itemList[i].SIw[itemCt]);
						}
						itemCt++;
					}
					//If the layer in the super item has more than 1 item
					else
					{
						if (this->orientationList[i] == 0)
						{
							for (int k = 0; k < this->itemList[i].nbItemsInSILayer[j]; k++)
							{
								newwList.push_back(this->itemList[i].SIw[itemCt]);
								newdList.push_back(this->itemList[i].SId[itemCt]);
								newhList.push_back(this->itemList[i].SIh[itemCt]);
								newxVec.push_back(this->xVec[i] + this->itemList[i].SIx[itemCt]);
								newyVec.push_back(this->yVec[i] + this->itemList[i].SIy[itemCt]);
								newzVec.push_back(this->itemList[i].SIz[itemCt]);
								newItemList.push_back(this->itemList[i].SIList[itemCt]);
								itemCt++;
							}
						}
						if (this->orientationList[i] == 1)
						{
							int flbW, flbD;
							for (int k = 0; k < this->itemList[i].nbItemsInSILayer[j]; k++)
							{
								if (this->itemList[i].SIx[itemCt] == 0 && this->itemList[i].SIy[itemCt] == 0)
								{
									newxVec.push_back(this->xVec[i]);
									newyVec.push_back(this->yVec[i]);
									newzVec.push_back(this->itemList[i].SIz[itemCt]);
									newwList.push_back(this->itemList[i].SId[itemCt]);
									newdList.push_back(this->itemList[i].SIw[itemCt]);
									newhList.push_back(this->itemList[i].SIh[itemCt]);
									flbW = this->itemList[i].SId[itemCt]; flbD = this->itemList[i].SIw[itemCt];
								}
								else if (this->itemList[i].SIx[itemCt] > 0 && this->itemList[i].SIy[itemCt] == 0)
								{
									newxVec.push_back(this->xVec[i]);
									newyVec.push_back(this->yVec[i] + flbD);
									newzVec.push_back(this->itemList[i].SIz[itemCt]);
									newwList.push_back(this->itemList[i].SId[itemCt]);
									newdList.push_back(this->itemList[i].SIw[itemCt]);
									newhList.push_back(this->itemList[i].SIh[itemCt]);
								}
								else if (this->itemList[i].SIx[itemCt] == 0 && this->itemList[i].SIy[itemCt] > 0)
								{
									newxVec.push_back(this->xVec[i] + flbW);
									newyVec.push_back(this->yVec[i]);
									newzVec.push_back(this->itemList[i].SIz[itemCt]);
									newwList.push_back(this->itemList[i].SId[itemCt]);
									newdList.push_back(this->itemList[i].SIw[itemCt]);
									newhList.push_back(this->itemList[i].SIh[itemCt]);
								}
								else if (this->itemList[i].SIx[itemCt] > 0 && this->itemList[i].SIy[itemCt] > 0)
								{
									newxVec.push_back(this->xVec[i] + flbW);
									newyVec.push_back(this->yVec[i] + flbD);
									newzVec.push_back(this->itemList[i].SIz[itemCt]);
									newwList.push_back(this->itemList[i].SId[itemCt]);
									newdList.push_back(this->itemList[i].SIw[itemCt]);
									newhList.push_back(this->itemList[i].SIh[itemCt]);
								}
								newItemList.push_back(this->itemList[i].SIList[itemCt]);
								itemCt++;
							}
						}//if (curLayer.orientationList[i] == 1)
					}
				}
			}

		}//for (int i = 0; i < this->itemList.size(); i++)
		this->itemList = newItemList; this->xVec = newxVec; this->yVec = newyVec; this->zVec = newzVec; this->wList = newwList; this->dList = newdList; this->hList = newhList;
		newItemList.clear(); newxVec.clear(); newyVec.clear(); newzVec.clear(); newwList.clear(); newdList.clear(); newhList.clear();

		for (int i = 0; i < itemList.size(); i++)
		{
			weight += itemList[i].weight;
		}
	}

	/**
	Default sorter for layers, based on layer density
	@param another layer object to be compared to the current one
	@return true if positions of layers should be exhanged in the list
	*/
	bool operator< (const Layer &otherL)
	{
		return layerOccupancy > otherL.layerOccupancy;
	}
};

//Layer lists
vector<Layer> layerList;			//Generated layer list
vector<Layer> selectedLayerList;

/**
Checks if two layers are exactly the same within the main layerList
@return true if the layers are the same, false if they are not
*/
bool checkDuplicate(const Layer &l1, const Layer &l2)
{
	if (l1.itemList.size() != l2.itemList.size())
		return false;

	for (int i = 0; i < l1.itemList.size(); i++)
	{
		if (l1.itemList[i].ID != l2.itemList[i].ID)
			return false;
		if (l1.xVec[i] != l2.xVec[i])
			return false;
		if (l1.yVec[i] != l2.yVec[i])
			return false;
	}

	return true;
}

/**
Removes duplicate layers from the main layerList
*/
void removeDuplicateLayers()
{
	for (int i = 0; i < layerList.size() - 1; i++)
	{
		for (int j = i + 1; j < layerList.size(); j++)
		{
			if (checkDuplicate(layerList[i], layerList[j]))
			{
				Layer tempLayer = layerList.back();
				layerList.back() = layerList[j];
				layerList[j] = tempLayer;
				layerList.pop_back();
				j--;
			}
		}
	}
}

/**
Sortation rule based on the alpha values of layers
@param two layer objects
@return true if layer positions should be exhanged in the list
*/
bool sortLayers(const Layer &l1, const Layer &l2)
{
	return l1.alpha > l2.alpha;
}

/**
Sortation rule based on average load cap of items in the layer
@param two layer objects
@return true if layer positions should be exhanged in the list
*/
bool sortLayersLoadCap(const Layer &l1, const Layer &l2)
{
	double lC1 = 0; double lC2 = 0;

	//Calculate average load cap of the items in both layers
	for (int i = 0; i < l1.itemList.size(); i++)
	{
		/*if (l1.itemList[i].loadCap < lC1)
			lC1 = l1.itemList[i].loadCap;*/
		lC1 += l1.itemList[i].loadCap;
	}
	for (int i = 0; i < l2.itemList.size(); i++)
	{
		/*if (l2.itemList[i].loadCap < lC2)
			lC2 = l2.itemList[i].loadCap;*/
		lC2 += l2.itemList[i].loadCap;
	}

	lC1 /= l1.itemList.size();
	lC2 /= l2.itemList.size();
	
	if (lC1 > lC2 && (l1.layerOccupancy > 0.9 * l2.layerOccupancy))
		return true;
	else
		return false;
		
}


/**
Rotate a layer in thw width-depth plane
@param a layer object
@return a new layer object with rotated item positions and dimensions
*/
Layer rotateLayer(const Layer &currLayer)
{
	Layer newLayer;
	newLayer = currLayer;

	for (int i = 0; i < currLayer.itemList.size(); i++)
	{
		if (currLayer.orientationList[i] == 0)
		{
			newLayer.xVec[i] = BinWidth - (newLayer.xVec[i] + newLayer.itemList[i].w);
			newLayer.yVec[i] = BinDepth - (newLayer.yVec[i] + newLayer.itemList[i].d);
		}
		else
		{
			newLayer.xVec[i] = BinWidth - (newLayer.xVec[i] + newLayer.itemList[i].d);
			newLayer.yVec[i] = BinDepth - (newLayer.yVec[i] + newLayer.itemList[i].w);
		}
	}

	return newLayer;
}

/**
Build a layer using the Maxrects heuristic
@param group of similar height items, iteration number, layer type, and the Maxrects heuristic number to be used (check main parameters)
@return true if a layer is succesfully generated
*/
bool BuildLayer(vector<Item> itemsToPack, int iteration, string layerType, int heuristicNb)
{
	using namespace rbp;
	bool result = false;

	//Initialize layer sizes
	MaxRectsBinPack bin;
	bin.Init(BinWidth, BinDepth);
	Layer newLayer;
	double reducedCost = 0;
	//Pack each item into the layer
	for (int i = 0; i < itemsToPack.size(); i++)
	{
		//Get the item
		Item newItem = itemsToPack[i];
		bool match = false;

		if (iteration > 1 && newItem.reducedCost < 0)
			continue;

		//Check if the item already exists in the layer
		for (int j = 0; j < newLayer.itemList.size(); j++)
		{
			//If the item compared in the layer is a normal item
			if (newLayer.itemList[j].SIList.size() == 0)
			{
				//If the new item is a normal item
				if (newItem.SIList.size() == 0)
				{
					if (newItem.ID == newLayer.itemList[j].ID)
					{
						match = true;
						break;
					}
				}
				//If the new item is a super-item
				else
				{
					for (int k = 0; k < newItem.SIList.size(); k++)
					{
						if (newItem.SIList[k].ID == newLayer.itemList[j].ID)
						{
							match = true;
							break;
						}
					}//for (int k = 0; k < superItems[newItem.ID - uniqueItemList.size()].superItemList.size(); k++)

					if (match == true)
						break;
				}
			}
			//If the item compared in the layer is a super-item
			else
			{
				//If the new item is a normal item
				if (newItem.SIList.size() == 0)
				{
					for (int k = 0; k < newLayer.itemList[j].SIList.size(); k++)
					{
						if (newItem.ID == newLayer.itemList[j].SIList[k].ID)
						{
							match = true;
							break;
						}
					}//for (int k = 0; k < superItems[newLayer.itemList[j] - uniqueItemList.size()].superItemList.size(); k++)

					if (match == true)
						break;
				}
				//If the new item is a super-item
				else
				{
					for (int k = 0; k < newItem.SIList.size(); k++)
					{
						for (int l = 0; l < newLayer.itemList[j].SIList.size(); l++)
						{
							if (newItem.SIList[k].ID == newLayer.itemList[j].SIList[l].ID)
							{
								match = true;
								break;
							}
						}//for (int l = 0; l < superItems[newLayer.itemList[j] - uniqueItemList.size()].superItemList.size(); l++)
						if (match == true)
							break;
					}//for (int k = 0; k < superItems[newItem.ID - uniqueItemList.size()].superItemList.size(); k++)
					if (match == true)
						break;
				}
			}
		}//for (int j = 0; j < newLayer.itemList.size(); j++)

		if (match == true)
			continue;

		//Get item sizes
		int itemWidth = newItem.w;
		int itemDepth = newItem.d;
		int itemHeight = newItem.h;

		MaxRectsBinPack::FreeRectChoiceHeuristic heuristic;
		//Perform the packing
		if (heuristicNb == 0)
			heuristic = MaxRectsBinPack::RectContactPointRule;
		else if (heuristicNb == 1)
			heuristic = MaxRectsBinPack::RectBestLongSideFit;
		else if (heuristicNb == 2)
			heuristic = MaxRectsBinPack::RectBestShortSideFit;
		else if (heuristicNb == 3)
			heuristic = MaxRectsBinPack::RectBestAreaFit;
		else if (heuristicNb == 4)
			heuristic = MaxRectsBinPack::RectBottomLeftRule;

		Rect packedRect = bin.Insert(itemWidth, itemDepth, heuristic);

		// Test success or failure.
		if (packedRect.height > 0)
		{
			if (packedRect.width == itemWidth)
				newLayer.insertItem(newItem, 0, packedRect.x, packedRect.y);
			else if (packedRect.width == itemDepth)
				newLayer.insertItem(newItem, 1, packedRect.x, packedRect.y);

			reducedCost += newItem.reducedCost;

			if (newItem.h > newLayer.layerHeight)
				newLayer.layerHeight = newItem.h;

			newLayer.calculateLayerOccupancy();
		}
	}

	reducedCost = newLayer.layerHeight - reducedCost;
	if (iteration == 1 || reducedCost < 0)
	{
		result = true;
		layerList.push_back(newLayer);
	}

	return result;
}

/**
Generate layers for the first iteration with random dual values to warm start column generation
*/
void generateInitialLayers()
{
	for (int i = 0; i < itemGroups.size(); i++)
	{
		//Generate a layer for the group, sorted from tallest to shortest
		if (MAXRECTSHeuristic == 5)
		{
			for (int j = 0; j < 5; j++)
			{
				BuildLayer(itemGroups[i], 1, "normal", j);
			}
		}
		else
			BuildLayer(itemGroups[i], 1, "normal", MAXRECTSHeuristic);

		//Also reverse the group and generate a layer
		vector<Item> groupCopy = itemGroups[i];
		reverse(groupCopy.begin(), groupCopy.end());

		if (MAXRECTSHeuristic == 5)
		{
			for (int j = 0; j < 5; j++)
			{
				BuildLayer(groupCopy, 1, "normal", j);
			}
		}
		else
			BuildLayer(groupCopy, 1, "normal", MAXRECTSHeuristic);

		for (int j = 0; j < 10; j++)
		{
			random_shuffle(groupCopy.begin(), groupCopy.end());

			if (MAXRECTSHeuristic == 5)
			{
				for (int k = 0; k < 5; k++)
				{
					BuildLayer(groupCopy, 1, "normal", k);
				}
			}
			else
				BuildLayer(groupCopy, 1, "normal", MAXRECTSHeuristic);

		}
		groupCopy.clear();
	}
}

/**
Generate layers for column generation
@param layer type (not relevant anymore)
*/
bool generateLayers(string layerType)
{
	bool result = false;
	bool overallResult = false;

	if (layerType == "normal")
	{
		for (int i = 0; i < itemGroups.size(); i++)
		{
			vector<Item> groupCopy = itemGroups[i];
			for (int j = 0; j < groupCopy.size(); j++)
			{
				for (int it = 0; it < itemList.size(); it++)
				{
					if (groupCopy[j].ID == itemList[it].ID)
						groupCopy[j].reducedCost = itemList[it].reducedCost;
				}
				for (int it = 0; it < superItems.size(); it++)
				{
					if (groupCopy[j].ID == superItems[it].ID)
						groupCopy[j].reducedCost = superItems[it].reducedCost;
				}
			}
			sort(groupCopy.begin(), groupCopy.end(), &sortDescReducedCost);

			if (MAXRECTSHeuristic == 5)
			{
				for (int j = 0; j < 5; j++)
				{
					result = BuildLayer(groupCopy, 2, "normal", j);
					if (result == true)
						overallResult = true;
				}
			}
			else
				result = BuildLayer(groupCopy, 2, "normal", MAXRECTSHeuristic);

			groupCopy.clear();

			if (overallResult == false && result == true)
				overallResult = true;
		}
	}
	return overallResult;
}

/**
Construct layers based on previously generated layers
@param Maxrects heuristic number
*/
void constructAdditionalLayers(int heuristicNb)
{
	int layerListSize = layerList.size();
	for (int i = 0; i < layerListSize; i++)
	{
		Layer curLayer = layerList[i];

		curLayer.calculateLayerOccupancy();

		if (curLayer.layerOccupancy < 60.0)
			continue;

		Layer newLayer;
		newLayer.layerHeight = 0;
		rbp::MaxRectsBinPack bin;
		bin.Init(BinWidth, BinDepth);

		rbp::MaxRectsBinPack::FreeRectChoiceHeuristic heuristic;
		//Perform the packing
		if (heuristicNb == 0)
			heuristic = rbp::MaxRectsBinPack::RectContactPointRule;
		else if (heuristicNb == 1)
			heuristic = rbp::MaxRectsBinPack::RectBestLongSideFit;
		else if (heuristicNb == 2)
			heuristic = rbp::MaxRectsBinPack::RectBestShortSideFit;
		else if (heuristicNb == 3)
			heuristic = rbp::MaxRectsBinPack::RectBestAreaFit;
		else if (heuristicNb == 4)
			heuristic = rbp::MaxRectsBinPack::RectBottomLeftRule;

		//Place every item in the layer again
		for (int j = 0; j < curLayer.itemList.size(); j++)
		{
			int itemWidth, itemDepth, itemHeight;
			if (curLayer.orientationList[j] == 0)
			{
				itemWidth = curLayer.itemList[j].w;
				itemDepth = curLayer.itemList[j].d;
			}
			else if (curLayer.orientationList[j] == 1)
			{
				itemWidth = curLayer.itemList[j].d;
				itemDepth = curLayer.itemList[j].w;
			}
			itemHeight = curLayer.itemList[j].h;

			rbp::Rect packedRect = bin.Insert(itemWidth, itemDepth, heuristic);

			if (packedRect.height > 0)
			{
				if (curLayer.itemList[j].w == packedRect.width)
					newLayer.insertItem(curLayer.itemList[j], 0, packedRect.x, packedRect.y);
				else if (curLayer.itemList[j].w == packedRect.height)
					newLayer.insertItem(curLayer.itemList[j], 1, packedRect.x, packedRect.y);

				if (itemHeight > newLayer.layerHeight)
					newLayer.layerHeight = itemHeight;

				newLayer.calculateLayerOccupancy();
			}//if (packedRect.height > 0)
		}//for (int j = 0; j < curLayer.itemList.size(); j++)

		 //Try to place unselected items
		for (int j = 0; j < itemList.size(); j++)
		{
			Item curItem = itemList[j];
			int check = 0;

			for (int k = 0; k < curLayer.itemList.size(); k++)
			{
				if (curItem.ID == curLayer.itemList[k].ID)
				{
					check = 1;
					break;
				}
			}

			if (check == 1)
				continue;

			int itemWidth = curItem.w;
			int itemDepth = curItem.d;
			int itemHeight = curItem.h;

			if (itemHeight > newLayer.layerHeight)
				continue;
			else if (newLayer.layerHeight - itemHeight > LayerHeightTolerance)
				continue;

			rbp::Rect packedRect = bin.Insert(itemWidth, itemDepth, heuristic);

			if (packedRect.height > 0)
			{
				if (curItem.w == packedRect.width)
					newLayer.insertItem(curItem, 0, packedRect.x, packedRect.y);
				else
					newLayer.insertItem(curItem, 1, packedRect.x, packedRect.y);

				if (itemHeight > newLayer.layerHeight)
					newLayer.layerHeight = itemHeight;

				newLayer.calculateLayerOccupancy();
			}//if (packedRect.height > 0)
		}//for (int j = 0; j < itemList.size(); j++)

		for (int j = 0; j < superItems.size(); j++)
		{
			Item currentSI = superItems[j];
			int check = 0;

			for (int k = 0; k < curLayer.itemList.size(); k++)
			{
				for (int l = 0; l < currentSI.SIList.size(); l++)
				{
					if (curLayer.itemList[k].ID == currentSI.SIList[l].ID)
					{
						check = 1;
						break;
					}
				}
				if (check == 1)
					break;
			}

			if (check == 1)
				continue;

			int itemWidth = currentSI.w;
			int itemDepth = currentSI.d;
			int itemHeight = currentSI.h;

			if (itemHeight > newLayer.layerHeight)
				continue;
			else if (newLayer.layerHeight - itemHeight > LayerHeightTolerance)
				continue;

			rbp::Rect packedRect = bin.Insert(itemWidth, itemDepth, heuristic);

			if (packedRect.height > 0)
			{
				if (packedRect.width == itemWidth)
					newLayer.insertItem(currentSI, 0, packedRect.x, packedRect.y);
				else
					newLayer.insertItem(currentSI, 1, packedRect.x, packedRect.y);

				if (itemHeight > newLayer.layerHeight)
					newLayer.layerHeight = itemHeight;

				newLayer.calculateLayerOccupancy();
			}//if (packedRect.height > 0)
		}//for (int j = 0; j < superItems.size(); j++)

		layerList.push_back(curLayer);
	}
}

/**
Generate a width-depth rotated layer for every layer in layerList
*/
void rotateLayers()
{
	int layerListSize = layerList.size();
	for (int i = 0; i < layerListSize; i++)
	{
		layerList.push_back(rotateLayer(layerList[i]));
	}
}

//CPLEX model builder for the main column generation
void buildModelByColumn(IloModel mod,
	IloNumVarArray alpha,
	const IloInt nUniqueItems,
	const IloInt nNormalColumns,
	const vector<Layer> normalLayers,
	IloNumVar::Type type,
	IloRangeArray range,
	IloObjective cost)
{
	IloEnv env = mod.getEnv();
	IloInt i, k;
	mod.add(range);

	for (int k = 0; k < nNormalColumns; k++)
	{
		IloNumColumn col;
		col = cost(normalLayers[k].layerHeight);

		for (int i = 0; i < nUniqueItems; i++)
		{
			col += range[i](0);
		}

		alpha.add(IloNumVar(col, 0, 1, type));
		col.end();

		for (int i = 0; i < normalLayers[k].itemList.size(); i++)
		{
			if (normalLayers[k].itemList[i].SIList.size() == 0)
				range[normalLayers[k].itemList[i].ID].setLinearCoef(alpha[k], 1);
			else
			{
				for (int j = 0; j < normalLayers[k].itemList[i].SIList.size(); j++)
				{
					range[normalLayers[k].itemList[i].SIList[j].ID].setLinearCoef(alpha[k], 1);
				}
			}
		}
	}
}

//CPLEX model builder for the previous layer spacing implementation
void buildModelByRow(IloModel mod,
	Var2Matrix c,
	vector<vector<vector<int>>> zPar,
	const vector<int> w,
	const vector<int> d,
	int nbItems,
	Layer layer)
{
	IloEnv env = mod.getEnv();

	IloExpr sum(env);

	IloNumVarArray a(env, nbItems);
	IloNumVarArray b(env, nbItems);

	vector<vector<int>> xStrips;
	vector<vector<int>> yStrips;

	for (int i = 0; i < nbItems; i++)
	{
		int w, d;
		if (layer.orientationList[i] == 0)
		{
			w = layer.itemList[i].w;
			d = layer.itemList[i].d;
		}
		else
		{
			w = layer.itemList[i].d;
			d = layer.itemList[i].w;
		}

		int xStartCoord = layer.xVec[i];
		int xEndCoord = xStartCoord + w;
		int yStartCoord = layer.yVec[i];
		int yEndCoord = yStartCoord + d;

		vector<int> singleXStrip;
		vector<int> singleYStrip;
		for (int j = 0; j < nbItems; j++)
		{
			if (i != j)
			{
				int w2, d2;
				if (layer.orientationList[j] == 0)
				{
					w2 = layer.itemList[j].w;
					d2 = layer.itemList[j].d;
				}
				else
				{
					w2 = layer.itemList[j].d;
					d2 = layer.itemList[j].w;
				}
				int nXStartCoord = layer.xVec[j];
				int nXEndCoord = nXStartCoord + w2;

				if ((nXStartCoord >= xStartCoord && nXStartCoord <= xEndCoord) || (nXEndCoord >= xStartCoord && nXEndCoord <= xEndCoord)
					|| (nXStartCoord <= xStartCoord && nXEndCoord >= xEndCoord) || (nXStartCoord >= xStartCoord && nXEndCoord <= xEndCoord))
				{
					singleYStrip.push_back(j);
				}

				int nYStartCoord = layer.yVec[j];
				int nYEndCoord = nYStartCoord + d2;

				if ((nYStartCoord >= yStartCoord && nYStartCoord <= yEndCoord) || (nYEndCoord >= yStartCoord && nYEndCoord <= yEndCoord)
					|| (nYStartCoord <= yStartCoord && nYEndCoord >= yEndCoord) || (nYStartCoord >= yStartCoord && nYEndCoord <= yEndCoord))
				{
					singleXStrip.push_back(j);
				}
			}
		}

		singleYStrip.push_back(i);
		singleXStrip.push_back(i);
		sort(singleYStrip.begin(), singleYStrip.end());
		sort(singleXStrip.begin(), singleXStrip.end());
		bool xCheck = false;
		bool yCheck = false;

		for (int j = 0; j < xStrips.size(); j++)
		{
			if (singleXStrip == xStrips[j])
			{
				xCheck = true;
				break;
			}
		}
		for (int j = 0; j < yStrips.size(); j++)
		{
			if (singleYStrip == yStrips[j])
			{
				yCheck = true;
				break;
			}
		}

		if (xCheck == false)
			xStrips.push_back(singleXStrip);

		if (yCheck == false)
			yStrips.push_back(singleYStrip);
	}

	IloNumVar aI(env, -IloInfinity, BinWidth, ILOFLOAT);
	IloNumVar bI(env, -IloInfinity, BinDepth, ILOFLOAT);

	for (int i = 0; i < nbItems; i++)
	{
		a[i] = IloNumVar(env, -IloInfinity, BinWidth, ILOFLOAT);
		b[i] = IloNumVar(env, -IloInfinity, BinDepth, ILOFLOAT);
	}

	for (int i = 0; i < nbItems; i++)
	{
		sum += a[i] + b[i];
	}

	sum += 1000 * (aI + bI);
	mod.add(IloMaximize(env, sum));

	for (int i = 0; i < nbItems; i++)
	{
		for (int j = 0; j < nbItems; j++)
		{
			if (i != j)
			{
				IloExpr expr3(env);
				expr3 = c[i][0] + w[i] - c[j][0] - BinWidth + BinWidth*zPar[i][j][0];
				mod.add(expr3 <= 0);
				expr3.end();

				IloExpr expr4(env);
				expr4 = c[i][1] + d[i] - c[j][1] - BinDepth + BinDepth*zPar[i][j][1];
				mod.add(expr4 <= 0);
				expr4.end();

				if (zPar[i][j][0] == 1)
				{
					IloExpr exprX(env);
					exprX = aI - (c[j][0] - (c[i][0] + w[i]));
					mod.add(exprX <= 0);
					exprX.end();
				}

				if (zPar[i][j][1] == 1)
				{
					IloExpr exprY(env);
					exprY = bI - (c[j][1] - (c[i][1] + d[i]));
					mod.add(exprY <= 0);
					exprY.end();
				}

			}
		}

		IloExpr expr8(env);
		expr8 = c[i][0] - (BinWidth - w[i]);
		mod.add(expr8 <= 0);
		expr8.end();

		IloExpr expr9(env);
		expr9 = c[i][1] - (BinDepth - d[i]);
		mod.add(expr9 <= 0);
		expr9.end();
	}

	for (int j = 0; j < xStrips.size(); j++)
	{
		for (int i = 0; i < xStrips[j].size(); i++)
		{
			for (int k = 0; k < xStrips[j].size(); k++)
			{
				if (xStrips[j][i] != xStrips[j][k] && zPar[xStrips[j][i]][xStrips[j][k]][0] == 1)
				{
					IloExpr exprXs(env);
					exprXs = a[xStrips[j][i]] - (c[xStrips[j][k]][0] - (c[xStrips[j][i]][0] + w[xStrips[j][i]]));
					mod.add(exprXs <= 0);
					exprXs.end();
				}
			}
		}
	}
	for (int j = 0; j < yStrips.size(); j++)
	{
		for (int i = 0; i < yStrips[j].size(); i++)
		{
			for (int k = 0; k < yStrips[j].size(); k++)
			{
				if (yStrips[j][i] != yStrips[j][k] && zPar[yStrips[j][i]][yStrips[j][k]][1] == 1)
				{
					IloExpr exprYs(env);
					exprYs = b[yStrips[j][i]] - (c[yStrips[j][k]][1] - (c[yStrips[j][i]][1] + d[yStrips[j][i]]));
					mod.add(exprYs <= 0);
					exprYs.end();
				}
			}
		}
	}
}

////CPLEX model builder for the previous layer spacing implementation in width dimension
void buildModelByRowX(IloModel mod,
	Var2Matrix c,
	vector<vector<vector<int>>> zPar,
	const vector<int> w,
	const vector<int> d,
	int nbItems,
	Layer layer)
{
	IloEnv env = mod.getEnv();

	IloExpr sum(env);

	IloNumVarArray a(env, nbItems);

	vector<vector<int>> xStrips;
	vector<int> yCoords;

	for (int i = 0; i < nbItems; i++)
	{
		int w1, d1;
		if (layer.orientationList[i] == 0)
		{
			w1 = layer.itemList[i].w;
			d1 = layer.itemList[i].d;
		}
		else
		{
			w1 = layer.itemList[i].d;
			d1 = layer.itemList[i].w;
		}
		int yStartCoord = layer.yVec[i];
		int yEndCoord = yStartCoord + d1;

		vector<int> singleXStrip;
		for (int j = 0; j < nbItems; j++)
		{
			if (i != j)
			{
				int w2, d2;
				if (layer.orientationList[j] == 0)
				{
					w2 = layer.itemList[j].w;
					d2 = layer.itemList[j].d;
				}
				else
				{
					w2 = layer.itemList[j].d;
					d2 = layer.itemList[j].w;
				}
				int nYStartCoord = layer.yVec[j];
				int nYEndCoord = nYStartCoord + d2;

				if ((nYStartCoord >= yStartCoord && nYStartCoord <= yEndCoord) || (nYEndCoord >= yStartCoord && nYEndCoord <= yEndCoord)
					|| (nYStartCoord <= yStartCoord && nYEndCoord >= yEndCoord) || (nYStartCoord >= yStartCoord && nYEndCoord <= yEndCoord))
				{
					singleXStrip.push_back(j);
				}
			}
		}

		singleXStrip.push_back(i);

		std::sort(singleXStrip.begin(), singleXStrip.end());
		bool xCheck = false;

		for (int j = 0; j < xStrips.size(); j++)
		{
			if (singleXStrip == xStrips[j])
			{
				xCheck = true;
				break;
			}
		}

		if (xCheck == false)
			xStrips.push_back(singleXStrip);
	}

	IloNumVar aI(env, -IloInfinity, BinWidth, ILOFLOAT);
	IloNumVar bI(env, -IloInfinity, BinDepth, ILOFLOAT);

	for (int i = 0; i < nbItems; i++)
	{
		a[i] = IloNumVar(env, -IloInfinity, BinWidth, ILOFLOAT);
	}

	for (int i = 0; i < nbItems; i++)
	{
		sum += a[i];
	}

	sum += nbItems * (aI);
	mod.add(IloMaximize(env, sum));

	for (int i = 0; i < nbItems; i++)
	{
		for (int j = 0; j < nbItems; j++)
		{
			if (i != j)
			{
				IloExpr expr3(env);
				expr3 = c[i][0] + w[i] - c[j][0] - BinWidth + BinWidth*zPar[i][j][0];
				mod.add(expr3 <= 0);
				expr3.end();

				IloExpr expr4(env);
				expr4 = c[i][1] + d[i] - c[j][1] - BinDepth + BinDepth*zPar[i][j][1];
				mod.add(expr4 <= 0);
				expr4.end();

				if (zPar[i][j][0] == 1)
				{
					IloExpr exprX(env);
					exprX = aI - (c[j][0] - (c[i][0] + w[i]));
					mod.add(exprX <= 0);
					exprX.end();
				}
			}
		}

		IloExpr expr8(env);
		expr8 = c[i][0] - (BinWidth - w[i]);
		mod.add(expr8 <= 0);
		expr8.end();

		IloExpr expr9(env);
		expr9 = c[i][1] - (BinDepth - d[i]);
		mod.add(expr9 <= 0);
		expr9.end();
	}

	for (int j = 0; j < xStrips.size(); j++)
	{
		for (int i = 0; i < xStrips[j].size(); i++)
		{
			for (int k = 0; k < xStrips[j].size(); k++)
			{
				if (xStrips[j][i] != xStrips[j][k] && zPar[xStrips[j][i]][xStrips[j][k]][0] == 1)
				{
					IloExpr exprXs(env);
					exprXs = a[xStrips[j][i]] - (c[xStrips[j][k]][0] - (c[xStrips[j][i]][0] + w[xStrips[j][i]]));
					mod.add(exprXs <= 0);
					exprXs.end();
				}
			}
		}
	}
}

//CPLEX model builder for the previous layer spacing implementation in depth dimension
void buildModelByRowY(IloModel mod,
	Var2Matrix c,
	vector<vector<vector<int>>> zPar,
	const vector<int> w,
	const vector<int> d,
	int nbItems,
	Layer layer)
{
	IloEnv env = mod.getEnv();

	IloExpr sum(env);

	IloNumVarArray b(env, nbItems);
	IloNumVarArray cI(env, nbItems);

	vector<vector<int>> yStrips;

	for (int i = 0; i < nbItems; i++)
	{
		int w1, d1;
		if (layer.orientationList[i] == 0)
		{
			w1 = layer.itemList[i].w;
			d1 = layer.itemList[i].d;
		}
		else
		{
			w1 = layer.itemList[i].d;
			d1 = layer.itemList[i].w;
		}
		int xStartCoord = layer.xVec[i];
		int xEndCoord = xStartCoord + w1;

		vector<int> singleYStrip;
		for (int j = 0; j < nbItems; j++)
		{
			if (i != j)
			{
				int w2, d2;
				if (layer.orientationList[j] == 0)
				{
					w2 = layer.itemList[j].w;
					d2 = layer.itemList[j].d;
				}
				else
				{
					w2 = layer.itemList[j].d;
					d2 = layer.itemList[j].w;
				}
				int nXStartCoord = layer.xVec[j];
				int nXEndCoord = nXStartCoord + w2;

				if ((nXStartCoord >= xStartCoord && nXStartCoord <= xEndCoord) || (nXEndCoord >= xStartCoord && nXEndCoord <= xEndCoord)
					|| (nXStartCoord <= xStartCoord && nXEndCoord >= xEndCoord) || (nXStartCoord >= xStartCoord && nXEndCoord <= xEndCoord))
				{
					singleYStrip.push_back(j);
				}
			}
		}

		singleYStrip.push_back(i);

		sort(singleYStrip.begin(), singleYStrip.end());
		bool yCheck = false;

		for (int j = 0; j < yStrips.size(); j++)
		{
			if (singleYStrip == yStrips[j])
			{
				yCheck = true;
				break;
			}
		}

		if (yCheck == false)
			yStrips.push_back(singleYStrip);
	}

	IloNumVar bI(env, -IloInfinity, BinDepth, ILOFLOAT);

	for (int i = 0; i < nbItems; i++)
	{
		b[i] = IloNumVar(env, -IloInfinity, BinDepth, ILOFLOAT);
		cI[i] = IloNumVar(env, -IloInfinity, IloInfinity, ILOFLOAT);
	}

	for (int i = 0; i < nbItems; i++)
	{
		sum += b[i];
		sum -= cI[i];
	}

	sum += 1000 * (bI);
	mod.add(IloMaximize(env, sum));

	for (int i = 0; i < nbItems; i++)
	{
		IloExpr exprAbs(env);
		exprAbs = c[i][0] - layer.xVec[i];
		mod.add(cI[i] >= exprAbs);
		exprAbs.end();

		IloExpr exprAbs2(env);
		exprAbs2 = layer.xVec[i] - c[i][0];
		mod.add(cI[i] >= exprAbs2);
		exprAbs2.end();

		for (int j = 0; j < nbItems; j++)
		{
			if (i != j)
			{
				IloExpr expr3(env);
				expr3 = c[i][0] + w[i] - c[j][0] - BinWidth + BinWidth*zPar[i][j][0];
				mod.add(expr3 <= 0);
				expr3.end();

				IloExpr expr4(env);
				expr4 = c[i][1] + d[i] - c[j][1] - BinDepth + BinDepth*zPar[i][j][1];
				mod.add(expr4 <= 0);
				expr4.end();

				if (zPar[i][j][1] == 1)
				{
					IloExpr exprY(env);
					exprY = bI - (c[j][1] - (c[i][1] + d[i]));
					mod.add(exprY <= 0);
					exprY.end();
				}
			}
		}

		IloExpr expr8(env);
		expr8 = c[i][0] - (BinWidth - w[i]);
		mod.add(expr8 <= 0);
		expr8.end();

		IloExpr expr9(env);
		expr9 = c[i][1] - (BinDepth - d[i]);
		mod.add(expr9 <= 0);
		expr9.end();
	}

	for (int j = 0; j < yStrips.size(); j++)
	{
		for (int i = 0; i < yStrips[j].size(); i++)
		{
			for (int k = 0; k < yStrips[j].size(); k++)
			{
				if (yStrips[j][i] != yStrips[j][k] && zPar[yStrips[j][i]][yStrips[j][k]][1] == 1)
				{
					IloExpr exprYs(env);
					exprYs = b[yStrips[j][i]] - (c[yStrips[j][k]][1] - (c[yStrips[j][i]][1] + d[yStrips[j][i]]));
					mod.add(exprYs <= 0);
					exprYs.end();
				}
			}
		}
	}
}

//CPLEX model builder for the current layer spacing implementation that maximizes overlap between two layers
void CPLEXmaximizeSupportForLayers(IloModel mod,
	Var2Matrix c,
	vector<vector<vector<int>>> zPar,
	const vector<int> w,
	const vector<int> d,
	int nbItems,
	Layer bottomLayer,
	Layer topLayer)
{
	IloEnv env = mod.getEnv();

	IloExpr sum(env);

	Var2Matrix xMax(env, nbItems);
	Var2Matrix xMin(env, nbItems);
	Var2Matrix yMax(env, nbItems);
	Var2Matrix yMin(env, nbItems);
	
	Var2Matrix xPos(env, nbItems);
	Var2Matrix yPos(env, nbItems);

	Var2Matrix overlapCount(env, nbItems);

	for (int i = 0; i < nbItems; i++)
	{
		xMax[i] = IloNumVarArray(env, bottomLayer.itemList.size());
		xMin[i] = IloNumVarArray(env, bottomLayer.itemList.size());
		yMax[i] = IloNumVarArray(env, bottomLayer.itemList.size());
		yMin[i] = IloNumVarArray(env, bottomLayer.itemList.size());
		xPos[i] = IloNumVarArray(env, bottomLayer.itemList.size());
		yPos[i] = IloNumVarArray(env, bottomLayer.itemList.size());
		overlapCount[i] = IloNumVarArray(env, bottomLayer.itemList.size());
		for (int j = 0; j < bottomLayer.itemList.size(); j++)
		{
			xMax[i][j] = IloNumVar(env, 0, BinWidth, ILOFLOAT);
			xMin[i][j] = IloNumVar(env, 0, BinWidth, ILOFLOAT);
			yMax[i][j] = IloNumVar(env, 0, BinDepth, ILOFLOAT);
			yMin[i][j] = IloNumVar(env, 0, BinDepth, ILOFLOAT);
			xPos[i][j] = IloNumVar(env, 0, BinWidth, ILOFLOAT);
			yPos[i][j] = IloNumVar(env, 0, BinDepth, ILOFLOAT);
			overlapCount[i][j] = IloNumVar(env, 0, 1, ILOINT);
		}
	}

	for (int i = 0; i < nbItems; i++)
	{
		for (int j = 0; j < bottomLayer.itemList.size(); j++)
		{
			sum += overlapCount[i][j];
			//sum += xPos[i][j] * yPos[i][j];
			//sum += (xMax[i][j] - xMin[i][j]) * (yMax[i][j] - yMin[i][j]);
		}
	}

	mod.add(IloMaximize(env, sum));


	for (int i = 0; i < nbItems; i++)
	{
		for (int j = 0; j < nbItems; j++)
		{
			if (i != j)
			{
				if (zPar[i][j][0] == 1)
				{
					IloExpr expr3(env);
					expr3 = c[i][0] + w[i] - c[j][0];
					mod.add(expr3 <= 0);
					expr3.end();
				}
				if (zPar[i][j][1] == 1)
				{
					IloExpr expr4(env);
					expr4 = c[i][1] + d[i] - c[j][1];
					mod.add(expr4 <= 0);
					expr4.end();
				}
			}
		}

		for (int j = 0; j < bottomLayer.itemList.size(); j++)
		{
			if (bottomLayer.layerHeight - (bottomLayer.zVec[j] + bottomLayer.hList[j]) <= LayerHeightTolerance)
			{
				//if (calculateOverlap(bottomLayer.xVec[j], bottomLayer.yVec[j], bottomLayer.wList[j], bottomLayer.dList[j],
				//	topLayer.xVec[i], topLayer.yVec[i], w[i], d[i], bottomLayer.itemList[j].supportType, bottomLayer.itemList[j].edgeReduceWidth, bottomLayer.itemList[j].edgeReduceDepth) > 0)
				//{
				//	mod.add(xMax[i][j] <= c[i][0] + w[i]);
				//	mod.add(xMax[i][j] <= bottomLayer.xVec[j] + bottomLayer.wList[j]);
				//	mod.add(xMin[i][j] >= c[i][0]);
				//	mod.add(xMin[i][j] >= bottomLayer.xVec[j]);
				//	mod.add(yMax[i][j] <= c[i][1] + d[i]);
				//	mod.add(yMax[i][j] <= bottomLayer.yVec[j] + bottomLayer.dList[j]);
				//	mod.add(yMin[i][j] >= c[i][1]);
				//	mod.add(yMin[i][j] >= bottomLayer.yVec[j]);
				//}
				//else
				//{
				//	mod.add(xMax[i][j] == 0);
				//	mod.add(xMin[i][j] == 0);
				//	mod.add(yMax[i][j] == 0);
				//	mod.add(yMin[i][j] == 0);
				//}
				mod.add(xMax[i][j] <= c[i][0] + w[i]);
				mod.add(xMax[i][j] <= bottomLayer.xVec[j] + bottomLayer.wList[j]);
				mod.add(xMin[i][j] >= c[i][0]);
				mod.add(xMin[i][j] >= bottomLayer.xVec[j]);
				mod.add(yMax[i][j] <= c[i][1] + d[i]);
				mod.add(yMax[i][j] <= bottomLayer.yVec[j] + bottomLayer.dList[j]);
				mod.add(yMin[i][j] >= c[i][1]);
				mod.add(yMin[i][j] >= bottomLayer.yVec[j]);
			}
			mod.add(xPos[i][j] - xMax[i][j] + xMin[i][j] >= 0);
			mod.add(yPos[i][j] - yMax[i][j] + yMin[i][j] >= 0);
			mod.add(overlapCount[i][j] <= xPos[i][j]);
			mod.add(overlapCount[i][j] <= yPos[i][j]);
		}

		IloExpr expr8(env);
		expr8 = c[i][0] - (BinWidth - w[i]);
		mod.add(expr8 <= 0);
		expr8.end();

		IloExpr expr9(env);
		expr9 = c[i][1] - (BinDepth - d[i]);
		mod.add(expr9 <= 0);
		expr9.end();
	}
}

//CPLEX model builder for the current layer spacing implementation that minimizes total overlap, but with overlap constraint
void CPLEXmaximizeSupportForLayersConstrained(IloModel mod,
	Var2Matrix c,
	vector<vector<vector<int>>> zPar,
	const vector<int> w,
	const vector<int> d,
	int nbItems,
	Layer bottomLayer,
	Layer topLayer)
{
	IloEnv env = mod.getEnv();

	IloExpr sum(env);

	Var2Matrix xMax(env, nbItems);
	Var2Matrix xMin(env, nbItems);
	Var2Matrix yMax(env, nbItems);
	Var2Matrix yMin(env, nbItems);
	Var2Matrix xPos(env, nbItems);
	Var2Matrix yPos(env, nbItems);

	for (int i = 0; i < nbItems; i++)
	{
		xMax[i] = IloNumVarArray(env, bottomLayer.itemList.size());
		xMin[i] = IloNumVarArray(env, bottomLayer.itemList.size());
		yMax[i] = IloNumVarArray(env, bottomLayer.itemList.size());
		yMin[i] = IloNumVarArray(env, bottomLayer.itemList.size());
		xPos[i] = IloNumVarArray(env, bottomLayer.itemList.size());
		yPos[i] = IloNumVarArray(env, bottomLayer.itemList.size());

		for (int j = 0; j < bottomLayer.itemList.size(); j++)
		{
			xMax[i][j] = IloNumVar(env, 0, BinWidth, ILOFLOAT);
			xMin[i][j] = IloNumVar(env, 0, BinWidth, ILOFLOAT);
			yMax[i][j] = IloNumVar(env, 0, BinDepth, ILOFLOAT);
			yMin[i][j] = IloNumVar(env, 0, BinDepth, ILOFLOAT);
			xPos[i][j] = IloNumVar(env, 0, BinWidth, ILOFLOAT);
			yPos[i][j] = IloNumVar(env, 0, BinDepth, ILOFLOAT);
		}
	}

	sum = c[0][0];

	mod.add(IloMinimize(env, sum));

	for (int i = 0; i < nbItems; i++)
	{
		for (int j = 0; j < nbItems; j++)
		{
			if (i != j)
			{
				if (zPar[i][j][0] == 1)
				{
					IloExpr expr3(env);
					expr3 = c[i][0] + w[i] - c[j][0];
					mod.add(expr3 <= 0);
					expr3.end();
				}
				if (zPar[i][j][1] == 1)
				{
					IloExpr expr4(env);
					expr4 = c[i][1] + d[i] - c[j][1];
					mod.add(expr4 <= 0);
					expr4.end();
				}
			}
		}

		IloExpr expr10(env);

		for (int j = 0; j < bottomLayer.itemList.size(); j++)
		{
			if (bottomLayer.layerHeight - (bottomLayer.zVec[j] + bottomLayer.hList[j]) <= LayerHeightTolerance)
			{
				//if (calculateOverlap(bottomLayer.xVec[j], bottomLayer.yVec[j], bottomLayer.wList[j], bottomLayer.dList[j],
				//	topLayer.xVec[i], topLayer.yVec[i], w[i], d[i], bottomLayer.itemList[j].supportType, bottomLayer.itemList[j].edgeReduceWidth, bottomLayer.itemList[j].edgeReduceDepth) > 0)
				//{
				//	mod.add(xMax[i][j] <= c[i][0] + w[i]);
				//	mod.add(xMax[i][j] <= bottomLayer.xVec[j] + bottomLayer.wList[j]);
				//	mod.add(xMin[i][j] >= c[i][0]);
				//	mod.add(xMin[i][j] >= bottomLayer.xVec[j]);
				//	mod.add(yMax[i][j] <= c[i][1] + d[i]);
				//	mod.add(yMax[i][j] <= bottomLayer.yVec[j] + bottomLayer.dList[j]);
				//	mod.add(yMin[i][j] >= c[i][1]);
				//	mod.add(yMin[i][j] >= bottomLayer.yVec[j]);
				//}
				//else
				//{
				//	mod.add(xMax[i][j] == 0);
				//	mod.add(xMin[i][j] == 0);
				//	mod.add(yMax[i][j] == 0);
				//	mod.add(yMin[i][j] == 0);
				//}
				
				//expr10 = (xPos[i][j] + yPos[i][j])*(xPos[i][j] + yPos[i][j]) - (xPos[i][j] - yPos[i][j])*(xPos[i][j] - yPos[i][j]);
			}
			mod.add(xMax[i][j] <= c[i][0] + w[i]);
			mod.add(xMax[i][j] <= bottomLayer.xVec[j] + bottomLayer.wList[j]);
			mod.add(xMin[i][j] >= c[i][0]);
			mod.add(xMin[i][j] >= bottomLayer.xVec[j]);
			mod.add(yMax[i][j] <= c[i][1] + d[i]);
			mod.add(yMax[i][j] <= bottomLayer.yVec[j] + bottomLayer.dList[j]);
			mod.add(yMin[i][j] >= c[i][1]);
			mod.add(yMin[i][j] >= bottomLayer.yVec[j]);

			expr10 += (xPos[i][j] + yPos[i][j])*(xPos[i][j] + yPos[i][j]) - (xPos[i][j] - yPos[i][j])*(xPos[i][j] - yPos[i][j]);

			mod.add(xPos[i][j] - xMax[i][j] + xMin[i][j] >= 0);
			mod.add(yPos[i][j] - yMax[i][j] + yMin[i][j] >= 0);
		}
		
		mod.add(expr10 >= 2.8 * w[i] * d[i]);
		expr10.end();

		IloExpr expr8(env);
		expr8 = c[i][0] - (BinWidth - w[i]);
		mod.add(expr8 <= 0);
		expr8.end();

		IloExpr expr9(env);
		expr9 = c[i][1] - (BinDepth - d[i]);
		mod.add(expr9 <= 0);
		expr9.end();
	}
}


/**
Space the bottom most layer in a bin, using the previous layer spacing implementation
@param a layer object
*/
void spaceLayer(Layer &newLayer)
{
	int nbItems = newLayer.itemList.size();

	vector<vector<vector<int>>> zPar;
	vector<int> widths(nbItems, 0);
	vector<int> depths(nbItems, 0);

	zPar.resize(nbItems);

	for (int j = 0; j < nbItems; j++)
	{
		zPar[j].resize(nbItems);

		for (int k = 0; k < nbItems; k++)
		{
			for (int s = 0; s < 2; s++)
			{
				zPar[j][k].push_back(0);
			}
		}
	}

	for (int j = 0; j < newLayer.itemList.size(); j++)
	{
		int w1, d1, w2, d2;
		if (newLayer.orientationList[j] == 0)
		{
			w1 = newLayer.itemList[j].w;
			d1 = newLayer.itemList[j].d;
		}
		else
		{
			w1 = newLayer.itemList[j].d;
			d1 = newLayer.itemList[j].w;
		}

		for (int k = 0; k < newLayer.itemList.size(); k++)
		{
			if (newLayer.orientationList[k] == 0)
			{
				w2 = newLayer.itemList[k].w;
				d2 = newLayer.itemList[k].d;
			}
			else
			{
				w2 = newLayer.itemList[k].d;
				d2 = newLayer.itemList[k].w;
			}

			if (j != k && newLayer.xVec[j] + w1 <= newLayer.xVec[k])
				zPar[j][k][0] = 1;

			else if (j != k && newLayer.xVec[k] + w2 <= newLayer.xVec[j])
				zPar[k][j][0] = 1;

			if (j != k && newLayer.yVec[j] + d1 <= newLayer.yVec[k])
				zPar[j][k][1] = 1;

			else if (j != k && newLayer.yVec[k] + d2 <= newLayer.yVec[j])
				zPar[k][j][1] = 1;
		}
		widths[j] = w1;
		depths[j] = d1;
	}

	IloEnv envRP;
	IloModel modRP(envRP);

	//Define vars
	Var2Matrix c(envRP, nbItems);

	//Initialize vars
	for (int j = 0; j < nbItems; j++)
	{
		c[j] = IloNumVarArray(envRP, 2);

		for (int s = 0; s < 2; s++)
		{
			c[j][s] = IloNumVar(envRP, 0, BinWidth, ILOFLOAT);
		}
	}

	buildModelByRowX(modRP, c, zPar, widths, depths, nbItems, newLayer);

	IloCplex cplexRP(modRP);
	cplexRP.setParam(IloCplex::TiLim, 900.0);
	cplexRP.setOut(envRP.getNullStream());
	cplexRP.solve();

	for (int j = 0; j < newLayer.xVec.size(); j++)
	{
		newLayer.xVec[j] = (int)cplexRP.getValue(c[j][0]);
		newLayer.yVec[j] = (int)cplexRP.getValue(c[j][1]);
	}

	zPar.clear();
	widths.clear();
	depths.clear();

	vector<int> widths2(nbItems, 0);
	vector<int> depths2(nbItems, 0);

	zPar.resize(nbItems);

	for (int j = 0; j < nbItems; j++)
	{
		zPar[j].resize(nbItems);

		for (int k = 0; k < nbItems; k++)
		{
			zPar[j][k].resize(2);
		}
	}

	for (int j = 0; j < newLayer.itemList.size(); j++)
	{
		int w1, w2, d1, d2;
		if (newLayer.orientationList[j] == 0)
		{
			w1 = newLayer.itemList[j].w;
			d1 = newLayer.itemList[j].d;
		}
		else
		{
			w1 = newLayer.itemList[j].d;
			d1 = newLayer.itemList[j].w;
		}

		for (int k = 0; k < newLayer.itemList.size(); k++)
		{
			if (newLayer.orientationList[k] == 0)
			{
				w2 = newLayer.itemList[k].w;
				d2 = newLayer.itemList[k].d;
			}
			else
			{
				w2 = newLayer.itemList[k].d;
				d2 = newLayer.itemList[k].w;
			}

			if (j != k && newLayer.xVec[j] + w1 <= newLayer.xVec[k])
			{
				zPar[j][k][0] = 1;
				zPar[k][j][0] = 0;
			}
			else if (j != k && newLayer.xVec[k] + w2 <= newLayer.xVec[j])
			{
				zPar[j][k][0] = 0;
				zPar[k][j][0] = 1;
			}

			if (j != k && newLayer.yVec[j] + d1 <= newLayer.yVec[k])
			{
				zPar[j][k][1] = 1;
				zPar[k][j][1] = 0;
			}
			else if (j != k && newLayer.yVec[k] + d2 <= newLayer.yVec[j])
			{
				zPar[j][k][1] = 0;
				zPar[k][j][1] = 1;
			}
		}
		widths2[j] = w1;
		depths2[j] = d1;
	}

	IloEnv envRPY;
	IloModel modRPY(envRPY);

	//Define vars
	Var2Matrix c2(envRPY, nbItems);

	//Initialize vars
	for (int j = 0; j < nbItems; j++)
	{
		c2[j] = IloNumVarArray(envRPY, 2);

		for (int s = 0; s < 2; s++)
		{
			c2[j][s] = IloNumVar(envRPY, 0, BinWidth, ILOFLOAT);
		}
	}


	buildModelByRowY(modRPY, c2, zPar, widths2, depths2, nbItems, newLayer);

	IloCplex cplexRPY(modRPY);
	cplexRPY.setOut(envRPY.getNullStream());
	cplexRPY.setParam(IloCplex::TiLim, 900.0);

	cplexRPY.solve();

	for (int j = 0; j < nbItems; j++)
	{
		newLayer.xVec[j] = (int)cplexRPY.getValue(c2[j][0]);
		newLayer.yVec[j] = (int)cplexRPY.getValue(c2[j][1]);
	}

	zPar.clear();
	widths2.clear();
	depths2.clear();
	cplexRPY.end();
	modRPY.end();
	envRPY.end();
	cplexRP.end();
	modRP.end();
	envRP.end();
}

/**
Space a layer so as to maximize its width-depth overlap with a bottom layer
@param two layer objects: a bottom and a top layer
*/
bool maximizeSupport(Layer &bottomLayer, Layer &topLayer)
{
	int nbItems = 0;

	nbItems = topLayer.itemList.size();

	vector<vector<vector<int>>> zPar;
	vector<int> widths(nbItems, 0);
	vector<int> depths(nbItems, 0);

	zPar.resize(nbItems);

	for (int j = 0; j < nbItems; j++)
	{
		zPar[j].resize(nbItems);

		for (int k = 0; k < nbItems; k++)
		{
			for (int s = 0; s < 3; s++)
			{
				zPar[j][k].push_back(0);
			}
		}
	}

	for (int j = 0; j < topLayer.itemList.size(); j++)
	{
		int w1, d1, w2, d2, w3, d3;
		if (topLayer.orientationList[j] == 0)
		{
			w1 = topLayer.itemList[j].w;
			d1 = topLayer.itemList[j].d;
		}
		else
		{
			w1 = topLayer.itemList[j].d;
			d1 = topLayer.itemList[j].w;
		}
		for (int k = 0; k < topLayer.itemList.size(); k++)
		{
			if (topLayer.orientationList[k] == 0)
			{
				w2 = topLayer.itemList[k].w;
				d2 = topLayer.itemList[k].d;
			}
			else
			{
				w2 = topLayer.itemList[k].d;
				d2 = topLayer.itemList[k].w;
			}
			bool match = false;
			for (int l = 0; l < topLayer.itemList.size(); l++)
			{
				if (j == k || j == l || l == k)
					continue;
				if (topLayer.orientationList[l] == 0)
				{
					w3 = topLayer.itemList[l].w;
					d3 = topLayer.itemList[l].d;
				}
				else
				{
					w3 = topLayer.itemList[l].d;
					d3 = topLayer.itemList[l].w;
				}
				if (!(topLayer.yVec[l] > topLayer.yVec[j] + d1 || topLayer.yVec[l] + d3 < topLayer.yVec[j]) &&
					!(topLayer.yVec[k] > topLayer.yVec[j] + d1 || topLayer.yVec[k] + d2 < topLayer.yVec[j]) &&
					(topLayer.xVec[j] + w1 <= topLayer.xVec[l] && topLayer.xVec[l] + w3 < topLayer.xVec[k]))
				{
					match = true;
					break;
				}

				if (!(topLayer.xVec[l] > topLayer.xVec[j] + w1 || topLayer.xVec[l] + w3 < topLayer.xVec[j]) &&
					!(topLayer.xVec[k] > topLayer.xVec[j] + w1 || topLayer.xVec[k] + w2 < topLayer.xVec[j]) &&
					(topLayer.yVec[j] + d1 <= topLayer.yVec[l] && topLayer.yVec[l] + d3 < topLayer.yVec[k]))
				{
					match = true;
					break;
				}
			}
			if (match == true)
				continue;

			if (j != k && topLayer.xVec[j] + w1 <= topLayer.xVec[k])
				zPar[j][k][0] = 1;

			if (j != k && topLayer.yVec[j] + d1 <= topLayer.yVec[k])
				zPar[j][k][1] = 1;
		}
		widths[j] = w1;
		depths[j] = d1;
	}

	IloEnv envRP;
	IloModel modRP(envRP);

	//Define vars
	Var2Matrix c(envRP, nbItems);

	//Initialize vars
	for (int j = 0; j < nbItems; j++)
	{
		c[j] = IloNumVarArray(envRP, 3);

		for (int s = 0; s < 3; s++)
		{
			c[j][s] = IloNumVar(envRP, 0, BinWidth, ILOFLOAT);
		}
	}

	//CPLEXmaximizeSupportForLayers(modRP, c, zPar, widths, depths, nbItems, bottomLayer, topLayer);
	CPLEXmaximizeSupportForLayers(modRP, c, zPar, widths, depths, nbItems, bottomLayer, topLayer);

	IloCplex cplexRP(modRP);
	cplexRP.setParam(IloCplex::TiLim, 900.0);
	cplexRP.setOut(envRP.getNullStream());
	cplexRP.setParam(IloCplex::Param::SolutionTarget,
		IloCplex::SolutionOptimalGlobal);
	//cplexRP.exportModel("model.mps");
	if (!cplexRP.solve())
		return false;

	//std::cout << cplexRP.getStatus() << endl;

	for (int j = 0; j < topLayer.xVec.size(); j++)
	{
		topLayer.xVec[j] = (int)cplexRP.getValue(c[j][0]);
		topLayer.yVec[j] = (int)cplexRP.getValue(c[j][1]);
	}

	zPar.clear();
	widths.clear();
	depths.clear();
	c.end();
	cplexRP.end();
	modRP.end();
	envRP.end();
	return true;
}

#endif