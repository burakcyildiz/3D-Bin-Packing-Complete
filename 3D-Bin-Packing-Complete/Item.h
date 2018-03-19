#ifndef ITEM_H
#define ITEM_H

#include "Common.h"

//Class definition for items
struct Item
{
	//Parameters representing the ID, planogram sequence, width, depth, height, weight, load capacity, edge reduce width and depth, the support type, and the top surface area of the item
	int ID, planogramSeq, w, d, h, weight, loadCap, edgeReduceWidth, edgeReduceDepth, supportType, topSurfaceArea;
	
	//Support types:
	// 0	-	Full support
	// 1	-	All edges
	// 2	-	Short edges
	// 3	-	Long edges
	// 4	-	Corners

	double reducedCost;		//Reduced cost of an item

	int superItemOnTop;		//Nb of items on top of this item in a super item
	double totalSupportedWeight;	//Total weight supported by the item (in grams)
	vector<Item> SIList;			//Nb of items in the super item (if this item is a super item)
	vector<int> nbItemsInSILayer;	//Nb of items in every super item layer
	vector<int> SIx, SIy, SIz, SIw, SId, SIh;	//Coordinates of the items in the super item (considering the front bottom left corner of the item is (0, 0, 0))
	vector<int> supportedItems;		//List of items that support this item
	vector<double> supportPercentages;		//Percentage of support for the supporting items (to calculate weight distribution)

	//Default constructor
	struct Item()
	{
	}

	//Constructor
	struct Item(int id, int seq, int W, int D, int H, int wgt, int lCap, int eRWidth, int eRDepth, string sptType)
	{
		this->ID = id;
		this->planogramSeq = seq;
		this->w = W;
		this->d = D;
		this->h = H;
		this->weight = wgt;
		this->loadCap = lCap;
		this->edgeReduceWidth = eRWidth;
		this->edgeReduceDepth = eRDepth;
		this->totalSupportedWeight = 0;

		//Update total load cap based on the top surface area of the item
		if (sptType == "FULL")
		{
			this->supportType = 0;
			if(edgeReduceToggle == 1)
				this->topSurfaceArea = (this->w - 2 * this->edgeReduceWidth) * (this->d - 2*this->edgeReduceDepth);
			else
				this->topSurfaceArea = this->w * this->d;
		}
		else if (sptType == "ALL_EDGES")
		{
			this->supportType = 1;
			double supportArea = this->w * 2 * 27 + (this->d - 2 * 27) * 27;
			this->topSurfaceArea = supportArea;
			this->loadCap = ((this->w * this->d) / supportArea) * this->loadCap;
		}
		else if (sptType == "SHORT_EDGES")
		{
			this->supportType = 2;
			double supportArea = this->d * 2 * 27;
			this->topSurfaceArea = supportArea;
			this->loadCap = ((this->w * this->d) / supportArea) * this->loadCap;
		}
		else if (sptType == "LONG_EDGES")
		{
			this->supportType = 3;
			double supportArea = this->w * 2 * 27;
			this->topSurfaceArea = supportArea;
			this->loadCap = ((this->w * this->d) / supportArea) * this->loadCap;
		}
		else
		{
			this->supportType = 4;
			double supportArea = 4 * 27 * 27;
			this->topSurfaceArea = supportArea;
			this->loadCap = ((this->w * this->d) / supportArea) * this->loadCap;
		}
	}

	//Simple constructor
	struct Item(int W, int D, int H)
	{
		this->w = W;
		this->d = D;
		this->h = H;
		this->superItemOnTop = 1;
	}

	//Destructor
	~Item()
	{
		SIx.clear();
		SIy.clear();
		SIz.clear();
		SIw.clear();
		SId.clear();
		SIh.clear();
		SIList.clear();
		supportedItems.clear();
		supportPercentages.clear();
	}

	//Add position of a new item to the super item
	void superItemPosition(int newX, int newY, int newZ)
	{
		SIx.push_back(newX);
		SIy.push_back(newY);
		SIz.push_back(newZ);
	}

	//Add the dimensions of a new item to the super item
	void superItemDimension(int newWidth, int newDepth, int newHeight)
	{
		SIw.push_back(newWidth);
		SId.push_back(newDepth);
		SIh.push_back(newHeight);
	}

	//Rotate an item in the width-depth dimension
	void rotateItem()
	{
		int temp = this->w;
		this->w = this->d;
		this->d = temp;
	}

	//Main sorting operator
	const bool operator< (const Item &other)
	{
		return h < other.h;
	}

	//Main equalizing operator
	const bool operator== (const Item &other)
	{
		if (ID == other.ID && w == other.w && d == other.d && h == other.h)
			return true;
		else
			return false;
	}
};

//*************************
//Item lists
//*************************
vector<Item> itemList;				//Item list currently in consideration for layer and bin building
vector<Item> uniqueItemList;		//List of all items in the input file
vector<Item> cpyUnique;				//An implementation list, a copy of the unique item list
vector<Item> remainingItemList;		//Items not considered for layer and bin building yet
vector<Item> superItems;			//Super item list
vector<vector<Item>> itemGroups;	//List of similar height item groups
vector<int> coveredList;			//A list that tracks covered items (with the value of 1)

const bool operator== (const Item &i1, const Item &i2)
{
	if (i1.ID == i2.ID && i1.w == i2.w && i1.d == i2.d && i1.h == i2.h)
		return true;
	else
		return false;
}

bool sortDescReducedCost(const Item &item1, const Item &item2)
{
	return (item1.reducedCost / (item1.w * item1.d)) > (item2.reducedCost / (item2.w * item2.d));
}

bool sortDescHeight(const Item &i1, const Item &i2)
{
	if (i1.h > i2.h) return true;
	else if (i1.h < i2.h) return false;
	else if (i1.w > i2.w) return true;
	else if (i1.w < i2.w) return false;
	else if (i1.d > i2.d) return true;
	else return false;
}

bool sortAscHeight(const Item &i1, const Item &i2)
{
	return i1.h < i2.h;
}

bool sortItems(const Item &i1, const Item &i2)
{
	return i1.w * i1.d > i2.w * i2.d;
}

bool sortItemsByPlanogramSeq(const Item &i1, const Item &i2)
{
	return i1.planogramSeq > i2.planogramSeq;
}

bool sortItemsByWidth(const Item &i1, const Item &i2)
{
	return i1.w < i2.w;
}

bool sortItemsHorizontalArea(const Item &i1, const Item &i2)
{
	if (i1.loadCap > i2.loadCap)
		return true;
	else if (i1.loadCap < i2.loadCap)
		return false;
	else if (i1.w * i1.d > i2.w * i2.d)
		return true;
	else if (i1.w * i1.d < i2.w * i2.d)
		return false;
	else if (i1.h > i1.h)
		return true;
	else
		return false;
}

/**
Parses items in the input file

@param file name
*/
void parseItems(string fileName)
{
	//Get file location
	ifstream myfile1;
	myfile1.open("../../Parameters/" + fileName + ".txt");

	string line;

	size_t pos = 0;
	string token;
	int tempTime;

	const string delimiter = "\t";
	vector<vector<string>> v;
	vector<string> fields;

	//Read data
	for (myfile1; getline(myfile1, line);)
	{
		v.push_back(split(fields, line, delimiter));
	}
	myfile1.close();

	int itemCount = 0;

	//Calculate the total number of items in the file
	for (int i = 0; i < v.size(); i++)
	{
		if(academicToggle == 0)
			itemCount += atoi(v.at(i).at(8).c_str());
		else
			itemCount += atoi(v.at(i).at(3).c_str());
	}

	nItems = itemCount;
	maxSeqNo = 0;

	int index = 0;
	int supIndex = 0;

	vector<Item> otherItemList;

	//Create items
	for (int i = 0; i < v.size(); i++)
	{
		int repetition;
		if(academicToggle == 1)
			repetition = atoi(v.at(i).at(3).c_str());
		else
			repetition = atoi(v.at(i).at(8).c_str());
		nbLines++;
		for (int j = 0; j < repetition; j++)
		{
			int id = index;
			int width = atoi(v.at(i).at(0).c_str());
			int depth = atoi(v.at(i).at(1).c_str());
			int height = atoi(v.at(i).at(2).c_str());
			int weight, edgeReduceWidth, edgeReduceDepth, sequence;
			
			if (academicToggle == 1)
			{
				weight = 0;
				edgeReduceWidth = 0;
				edgeReduceDepth = 0;
				sequence = 0;
			}
			else
			{
				weight = atoi(v.at(i).at(3).c_str());
				edgeReduceWidth = atoi(v.at(i).at(5).c_str());
				edgeReduceDepth = atoi(v.at(i).at(6).c_str());
				sequence = atoi(v.at(i).at(9).c_str());
			}

			totalVol += width * depth * height / 1000000;
			
			int loadCap;
			string supportType;

			if (width < shortestWD)
				shortestWD = width;

			if (depth < shortestWD)
				shortestWD = depth;

			if (height < shortestHeight)
				shortestHeight = height;

			if (sequence > maxSeqNo)
				maxSeqNo = sequence;

			if (loadCapToggle == 1)
				loadCap = atoi(v.at(i).at(4).c_str()) * width * depth / 200;
			else
				loadCap = 10000000;

			if (loadCap > maxLoadCap)
				maxLoadCap = loadCap;

			if (itemShapeToggle == 1)
				supportType = v.at(i).at(7).c_str();
			else
				supportType = "FULL";

			Item newItem(id, sequence, width, depth, height, weight, loadCap, edgeReduceWidth, edgeReduceDepth, supportType);
			uniqueItemList.push_back(newItem);
			index++;
		}
	}

	v.clear();
	fields.clear();
	remainingItemList = uniqueItemList;
}

/**
Groups items and superitems into lists based on their heights so that they can be used in BuildLayer
*/
void groupItems()
{
	vector<Item> groupElements;
	vector<int> groupStartIndexList;

	groupStartIndexList.push_back(itemList[0].ID);
	
	//Create group starter item list
	for (int i = 0; i < itemList.size(); i++)
	{
		int itemHeight = uniqueItemList[itemList[i].ID].h;

		if (i > 0)
			if (itemHeight != uniqueItemList[itemList[i - 1].ID].h)
			{
				groupStartIndexList.push_back(itemList[i].ID);
			}
	}

	//Use superitems as group starters

	if (superItems.size() > 0)
		groupStartIndexList.push_back(superItems[0].ID);

	for (int i = 0; i < superItems.size(); i++)
	{
		int itemHeight = superItems[i].h;

		if (i > 0)
			if (itemHeight != superItems[i - 1].h)
				groupStartIndexList.push_back(superItems[i].ID);
	}

	//Build groups based on height tolerances
	for (int i = 0; i < groupStartIndexList.size(); i++)
	{
		int groupStartHeight;
		double totalArea = 0;

		if (groupStartIndexList[i] < uniqueItemList.size())
		{
			groupStartHeight = uniqueItemList[groupStartIndexList[i]].h;
			for (int j = 0; j < itemList.size(); j++)
			{
				if (groupStartHeight >= uniqueItemList[itemList[j].ID].h && abs(groupStartHeight - uniqueItemList[itemList[j].ID].h) <= LayerHeightTolerance)
				{
					groupElements.push_back(uniqueItemList[itemList[j].ID]);
					totalArea += uniqueItemList[itemList[j].ID].topSurfaceArea;
				}
			}

			for (int j = 0; j < superItems.size(); j++)
			{
				if (groupStartHeight >= superItems[j].h && abs(groupStartHeight - superItems[j].h) <= LayerHeightTolerance)
				{
					groupElements.push_back(superItems[j]);
					totalArea += superItems[j].topSurfaceArea;
				}
			}
		}//if(groupStartIndexList[i] < uniqueItemList.size())
		else
		{
			groupStartHeight = superItems[groupStartIndexList[i] - uniqueItemList.size()].h;

			for (int j = 0; j < itemList.size(); j++)
			{
				if (groupStartHeight >= uniqueItemList[itemList[j].ID].h && abs(groupStartHeight - uniqueItemList[itemList[j].ID].h) <= LayerHeightTolerance)
				{
					groupElements.push_back(itemList[j]);
					totalArea += uniqueItemList[itemList[j].ID].topSurfaceArea;
				}
			}

			for (int j = 0; j < superItems.size(); j++)
			{
				if (groupStartHeight >= superItems[j].h && abs(groupStartHeight - superItems[j].h) <= LayerHeightTolerance)
				{
					groupElements.push_back(superItems[j]);
					totalArea += superItems[j].topSurfaceArea;
				}
			}
		}//else
		if (groupElements.size() > 2 && totalArea * 100 / (BinWidth * BinDepth) >= 60.0)
			itemGroups.push_back(groupElements);

		groupElements.clear();
	}//for (int i = 0; i < groupStartIndexList.size(); i++)

	//int igSize = itemGroups.size();
	//for (int i = 0; i < igSize; i++)
	//{
	//	int minLB = 100000000; int maxLB = 0;

	//	for (int j = 0; j < itemGroups[i].size(); j++)
	//	{
	//		if (itemGroups[i][j].loadCap > maxLB)
	//			maxLB = itemGroups[i][j].loadCap;
	//		if (itemGroups[i][j].loadCap < minLB)
	//			minLB = itemGroups[i][j].loadCap;
	//	}

	//	vector<Item> newItemGroup1;
	//	vector<Item> newItemGroup2;
	//	vector<Item> newItemGroup3;
	//	vector<Item> newItemGroup4;
	//	vector<Item> newItemGroup5;

	//	for (int j = 0; j < itemGroups[i].size(); j++)
	//	{
	//		if (itemGroups[i][j].loadCap <= minLB + 0.2*(maxLB - minLB))
	//			newItemGroup1.push_back(itemGroups[i][j]);
	//		else if (itemGroups[i][j].loadCap <= minLB + 0.4*(maxLB - minLB))
	//			newItemGroup2.push_back(itemGroups[i][j]);
	//		else if (itemGroups[i][j].loadCap <= minLB + 0.6*(maxLB - minLB))
	//			newItemGroup3.push_back(itemGroups[i][j]);
	//		else if (itemGroups[i][j].loadCap <= minLB + 0.8*(maxLB - minLB))
	//			newItemGroup4.push_back(itemGroups[i][j]);
	//		else if (itemGroups[i][j].loadCap <= minLB + 1.0*(maxLB - minLB))
	//			newItemGroup5.push_back(itemGroups[i][j]);
	//	}

	//	itemGroups.push_back(newItemGroup1);
	//	itemGroups.push_back(newItemGroup2);
	//	itemGroups.push_back(newItemGroup3);
	//	itemGroups.push_back(newItemGroup4);
	//	itemGroups.push_back(newItemGroup5);
	//}

	//for (int i = 0; i < itemGroups.size(); i++)
	//{
	//	double totalArea = 0;

	//	for (int j = 0; j < itemGroups[i].size(); j++)
	//	{
	//		totalArea += itemGroups[i][j].topSurfaceArea;
	//	}

	//	if (totalArea * 100 / (BinWidth * BinDepth) < 60.0 || itemGroups[i].size() < 3)
	//	{
	//		itemGroups.erase(itemGroups.begin() + i);
	//		i--;
	//	}
	//}

	for (int i = 0; i < itemGroups.size(); i++)
	{
		double avgloadbearing = 0;
		for (int j = 0; j < itemGroups[i].size(); j++)
		{
			avgloadbearing += itemGroups[i][j].loadCap;
		}
		avgloadbearing /= itemGroups[i].size();

		for (int j = 0; j < itemGroups[i].size(); j++)
		{
			if (itemGroups[i][j].loadCap < 0.9 * avgloadbearing/* || itemgroups[i][j].loadcap > 1.3 * avgloadbearing*/)
			{
				itemGroups[i].erase(itemGroups[i].begin() + j);
				j--;
			}
		}
	}
}

/**
Generates super items
*/
void generateSuperItems()
{
	//Get the number of items from the remaining item list based on batch size
	int currBatchSize;
	if (planogramSeqToggle == 1)
	{
		//currBatchSize = min(BatchSize, (int)remainingItemList.size());

		//Sort the remaining items based on their planogram sequence number in descending order
		sort(remainingItemList.begin(), remainingItemList.end(), &sortItemsByPlanogramSeq);
	}
	/*else
		currBatchSize = remainingItemList.size();*/

	//Get the next batch of items from the remaining items
	while(itemList.size() < BatchSize)
	{
		if (remainingItemList.size() == 0)
			break;
		itemList.push_back(remainingItemList.back());
		remainingItemList.pop_back();
	}

	//**********************************************
	//Generate horizontal super-items with 2 items
	//**********************************************

	for (int i = 0; i < itemList.size() - 1; i++)
	{
		Item item1 = itemList[i];
		Item item1turned = itemList[i];
		item1turned.d = item1.w;
		item1turned.w = item1.d;

		if (i > 0 && itemList[i].w == itemList[i - 1].w && itemList[i].d == itemList[i - 1].d)
			continue;

		for (int j = i + 1; j < itemList.size(); j++)
		{
			Item item2 = itemList[j];
			Item item2turned = itemList[j];
			item2turned.d = item2.w;
			item2turned.w = item2.d;

			if (j > i + 1 && itemList[j].w == itemList[j - 1].w && itemList[j].d == itemList[j - 1].d)
				continue;


			//******************************
			//Try matching depth dimensions
			//******************************

			//Without turning any of the items
			if ((double)min(item1.d,item2.d)/max(item1.d,item2.d) >= minEdgeRatio && abs(item1.h - item2.h) <= LayerHeightTolerance)
			{
				Item newItem(item1.w + item2.w, max(item1.d, item2.d), max(item1.h, item2.h));
				newItem.SIList.push_back(item1);
				newItem.superItemPosition(0, 0, 0);
				newItem.superItemDimension(item1.w, item1.d, item1.h);
				newItem.SIList.push_back(item2);
				newItem.superItemPosition(item1.w, 0, 0);
				newItem.superItemDimension(item2.w, item2.d, item2.h);
				newItem.nbItemsInSILayer.push_back(2);
				newItem.weight = item1.weight + item2.weight;
				newItem.loadCap = item1.loadCap + item2.loadCap;
				newItem.topSurfaceArea = item1.topSurfaceArea + item2.topSurfaceArea;
				superItems.push_back(newItem);
			}

			//Turn the first item
			if ((double)min(item1turned.d, item2.d) / max(item1turned.d, item2.d) >= minEdgeRatio && abs(item1turned.h - item2.h) <= LayerHeightTolerance)
			{
				Item newItem(item1turned.w + item2.w, max(item1turned.d, item2.d), max(item1turned.h, item2.h));
				newItem.SIList.push_back(item1);
				newItem.superItemPosition(0, 0, 0);
				newItem.superItemDimension(item1turned.w, item1turned.d, item1turned.h);
				newItem.SIList.push_back(item2);
				newItem.superItemPosition(item1turned.w, 0, 0);
				newItem.superItemDimension(item2.w, item2.d, item2.h);
				newItem.nbItemsInSILayer.push_back(2);
				newItem.weight = item1.weight + item2.weight;
				newItem.loadCap = item1.loadCap + item2.loadCap;
				newItem.topSurfaceArea = item1.topSurfaceArea + item2.topSurfaceArea;
				superItems.push_back(newItem);
			}

			//Turn the second item
			if ((double)min(item1.d, item2turned.d) / max(item1.d, item2turned.d) >= minEdgeRatio && abs(item1.h - item2turned.h) <= LayerHeightTolerance)
			{
				Item newItem(item1.w + item2turned.w, max(item1.d, item2turned.d), max(item1.h, item2turned.h));
				newItem.SIList.push_back(item1);
				newItem.superItemPosition(0, 0, 0);
				newItem.superItemDimension(item1.w, item1.d, item1.h);
				newItem.SIList.push_back(item2);
				newItem.superItemPosition(item1.w, 0, 0);
				newItem.superItemDimension(item2turned.w, item2turned.d, item2turned.h);
				newItem.nbItemsInSILayer.push_back(2);
				newItem.weight = item1.weight + item2.weight;
				newItem.loadCap = item1.loadCap + item2.loadCap;
				newItem.topSurfaceArea = item1.topSurfaceArea + item2.topSurfaceArea;
				superItems.push_back(newItem);
			}
		}
	}


	//********************************************************************************************************
	//Generate horizontal super-items with 4 items by matching the width dimensions of the 2-item super-items
	//********************************************************************************************************
	/*
	int superItemsSize = superItems.size();
	for (int i = 0; i < superItemsSize - 1; i++)
	{
		Item sItem1 = superItems[i];

		if (i > 0 && sItem1.w == superItems[i - 1].w && sItem1.d == superItems[i - 1].d)
			continue;

		for (int j = i + 1; j < superItemsSize; j++)
		{
			Item sItem2 = superItems[j];
			vector<int> items(0);

			if (j > i + 1 && sItem2.w == superItems[j - 1].w && sItem2.d == superItems[j - 1].d)
				continue;

			//Check if they include the same item or not
			bool duplicateItem = false;
			for (int siIt1 = 0; siIt1 < sItem1.SIList.size(); siIt1++)
			{
				for (int siIt2 = 0; siIt2 < sItem2.SIList.size(); siIt2++)
				{
					if (sItem1.SIList[siIt1].ID == sItem2.SIList[siIt2].ID)
					{
						duplicateItem = true;
						break;
					}
					items.push_back(sItem2.SIList[siIt2].ID);
				}
				if (duplicateItem == true)
					break;
				items.push_back(sItem1.SIList[siIt1].ID);
			}

			if (duplicateItem == true)
				continue;
			
			//Check if their width dimensions match
			if ((double)min(sItem1.w, sItem2.w) / max(sItem1.w, sItem2.w) >= minEdgeRatio && abs(sItem1.h - sItem2.h) <= LayerHeightTolerance)
			{
				Item newItem(max(sItem1.w, sItem2.w), sItem1.d + sItem2.d, max(sItem1.h, sItem2.h));
				newItem.SIList.push_back(sItem1.SIList[0]);
				newItem.superItemPosition(0, 0, 0);
				newItem.superItemDimension(sItem1.SIList[0].w, sItem1.SIList[0].d, sItem1.SIList[0].h);
				newItem.SIList.push_back(sItem1.SIList[1]);
				newItem.superItemPosition(sItem1.SIList[0].w, 0, 0);
				newItem.superItemDimension(sItem1.SIList[1].w, sItem1.SIList[1].d, sItem1.SIList[1].h);
				newItem.SIList.push_back(sItem2.SIList[0]);
				newItem.superItemPosition(0, sItem1.d, 0);
				newItem.superItemDimension(sItem2.SIList[0].w, sItem2.SIList[0].d, sItem2.SIList[0].h);
				newItem.SIList.push_back(sItem2.SIList[1]);
				newItem.superItemPosition(sItem2.SIList[0].w, sItem1.d, 0);
				newItem.superItemDimension(sItem2.SIList[1].w, sItem2.SIList[1].d, sItem2.SIList[1].h);
				newItem.nbItemsInSILayer.push_back(4);
				newItem.weight = sItem1.weight + sItem2.weight;
				newItem.loadCap = sItem1.loadCap + sItem2.loadCap;
				newItem.topSurfaceArea + sItem1.topSurfaceArea + sItem2.topSurfaceArea;
				superItems.push_back(newItem);
			}
		}//for (int j = i + 1; j < itemList.size() - 2; j++)
	}//for (int i = 0; i < itemList.size() - 3; i++)
	
	*/
	 //**************************************************************************************************
	 //Generate initial vertical super-items based on previously generated super-items and single items
	 //**************************************************************************************************

	 //Sort items by decreasing area
	std::sort(itemList.begin(), itemList.end(), &sortItems);

	//Create initial super-items by stacking items vertically
	for (int i = 0; i < itemList.size() - 1; i++)
	{
		Item bottomItem = itemList[i];

		if (i > 0 && bottomItem.w == itemList[i - 1].w && bottomItem.d == itemList[i - 1].d)
			continue;

		for (int j = i + 1; j < itemList.size(); j++)
		{
			Item topItem = itemList[j];

			if (j > i + 1 && topItem.w == itemList[j - 1].w && topItem.d == itemList[j - 1].d)
				continue;

			if (topItem.w >= bottomItem.w && topItem.w <= bottomItem.w * (1 + (1 - superItemWidthTolerance)) && bottomItem.ID != topItem.ID && topItem.h + bottomItem.h <= 700)
			{
				if (topItem.d >= bottomItem.d && topItem.d <= bottomItem.d * (1 + (1 - superItemDepthTolerance)))
				{
					if (topItem.weight > bottomItem.loadCap)
						continue;

					Item newItem(topItem.w, topItem.d, bottomItem.h + topItem.h);
					newItem.superItemOnTop++;
					newItem.SIList.push_back(bottomItem);
					newItem.superItemPosition((topItem.w - bottomItem.w) / 2, (topItem.d - bottomItem.d) / 2, 0);
					newItem.superItemDimension(bottomItem.w, bottomItem.d, bottomItem.h);
					newItem.nbItemsInSILayer.push_back(1);

					newItem.SIList.push_back(topItem);
					newItem.superItemPosition(0, 0, bottomItem.h);
					newItem.superItemDimension(topItem.w, topItem.d, topItem.h);
					newItem.nbItemsInSILayer.push_back(1);
					newItem.weight = topItem.weight + bottomItem.weight;
					newItem.loadCap = min(topItem.loadCap, bottomItem.loadCap - topItem.weight);
					newItem.topSurfaceArea = topItem.topSurfaceArea;

					superItems.push_back(newItem);
				}
			}//if
		}//for (int j = i + 1; j < itemList.size(); j++)
	}//for (int i = 0; i < itemList.size() - 1; i++)

	std::sort(itemList.begin(), itemList.end(), &sortDescHeight);

	//Create super-items, based on other super-items
	for (int i = 0; i < superItems.size(); i++)
	{
		Item superItem = superItems[i];

		if (superItem.superItemOnTop == maximumSuperItem)
			continue;

		for (int j = 0; j < itemList.size(); j++)
		{
			int check = 0;
			Item topItem = itemList[j];

			for (int k = 0; k < superItem.SIList.size(); k++)
			{
				if (topItem.ID == superItem.SIList[k].ID)
				{
					check = 1;
					break;
				}
			}

			if (check == 0)
			{
				if (topItem.w >= superItem.w && topItem.d >= superItem.d && topItem.h + superItem.h <= 700)
				{
					if (topItem.w <= (1 + (1 - superItemWidthTolerance)) * superItem.w && topItem.d <= (1 + (1 - superItemDepthTolerance)) * superItem.d)
					{
						if (topItem.weight > superItem.loadCap)
							continue;

						Item newItem(topItem.w, topItem.d, superItem.h + topItem.h);
						newItem.SIList = superItem.SIList;
						newItem.SIx = superItem.SIx;
						newItem.SIy = superItem.SIy;
						newItem.SIz = superItem.SIz;
						newItem.superItemOnTop = superItem.superItemOnTop + 1;
						newItem.SIw = superItem.SIw;
						newItem.SId = superItem.SId;
						newItem.SIh = superItem.SIh;
						newItem.nbItemsInSILayer = superItem.nbItemsInSILayer;

						newItem.SIList.push_back(topItem);
						newItem.superItemPosition(0, 0, superItem.h);
						newItem.superItemDimension(topItem.w, topItem.d, topItem.h);
						newItem.nbItemsInSILayer.push_back(1);
						newItem.weight = superItem.weight + topItem.weight;
						newItem.loadCap = min(topItem.loadCap, superItem.loadCap - topItem.weight);
						newItem.topSurfaceArea = topItem.topSurfaceArea;
						superItems.push_back(newItem);
					}//if (topItem.width <= (1 + (1 - superItemWidthTolerance)) * superItem.width && topItem.depth <= (1 + (1 - superItemDepthTolerance)) * superItem.depth)
				}//if (topItem.width >= superItem.width && topItem.depth >= superItem.depth && topItem.height + superItem.height <= superItemHeightTolerance)
			}//if (check == 0)
		}//for (int j = 0; j < itemList.size(); j++)
	}//for (int i = 0; i < superItems.size(); i++)

	//Remove super-items without anything on top or bottom
	for (int i = 0; i < superItems.size(); i++)
	{
		if (superItems[i].nbItemsInSILayer.size() == 1)
		{
			Item tempItem = superItems.back();
			superItems.back() = superItems[i];
			superItems[i] = tempItem;
			superItems.pop_back();
			i--;
		}
	}

	sort(itemList.begin(), itemList.end(), &sortDescHeight);

	//Give super items IDs, continuing from the ID of the last item
	if (superItems.size() > 0)
	{
		sort(superItems.begin(), superItems.end(), &sortDescHeight);

		for (int i = 0; i < superItems.size(); i++)
		{
			superItems[i].ID = uniqueItemList.size() + i;
		}
	}

	//Group items
	groupItems();
}

#endif