#include "Common.h"
#include "Item.h"
#include "Layer.h"
#include "Bin.h"
#include "CPLEX.h"


int main(int argc, char **argv)
{
	//Set parameters based on the toggled practical constraints
	if (binWeightToggle == 1)
		maxBinWeight = 1500000;
	else
		maxBinWeight = 100000000;

	if (verticalSupportToggle == 1)
		itemSupportPercentage = 60.0;
	else
		itemSupportPercentage = 0;

	//Get the input file name list
	vector<string> filesToRead;

	ifstream myfile1;
	myfile1.open("filesToRead.txt");

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

	for (int i = 0; i < v.size(); i++)
	{
		filesToRead.push_back(v.at(i).at(0).c_str());
	}

	string filename;

	int instanceCt = 0;
	double averageBin = 0.0;
	int sumBin = 0;


	//Main solution loop
	for (int q = 0; q < filesToRead.size(); q++)
	{
		//Set up the solution CPU time counter
		clock_t overallStart;
		double duration;
		overallStart = clock();

		string filename = filesToRead[q];

		totalVol = 0;

		//Parse the items from the input file
		parseItems(filename);
		//Set up the list that tracks items that are covered
		for (int i = 0; i < uniqueItemList.size(); i++)
		{
			coveredList.push_back(0);
		}
		int nLayers;

		//Loop that places all the items
		while (remainingItemList.size() > 0 || itemList.size() > 0)
		{
			nbLeftoverItems = 0;
			//Generate super items and group items based on their heights
			generateSuperItems();

			//Set up parameters used during the solution process
			int iteration = 1;
			int nbLayers = 0;
			int improvementCounter = 0;
			double objectiveValue = pow(10.0, 15);
			int nUniqueItems = nItems;
			double improvementObj = pow(10.0, 15);
			int nbLayersLast = 0;
			nItems = itemList.size();

			//Set up CPLEX parameters and variables
			IloEnv env;
			IloNumVar::Type varType = ILOFLOAT;
			IloModel mod(env);
			IloNumVarArray alpha(env);
			IloNumArray constrMin(env, uniqueItemList.size());
			IloNumArray constrMax(env, uniqueItemList.size());
			for (IloInt i = 0; i < uniqueItemList.size(); i++)
			{
				constrMin[i] = 1;
				constrMax[i] = IloInfinity;
			}
			IloRangeArray range(env, constrMin, constrMax);
			IloObjective cost = IloAdd(mod, IloMinimize(env));

			//Start iteration timer
			clock_t start;
			start = clock();

			//Layer generation loop
			while (true)
			{
				bool cont = false;
				
				//Generate layers
				if (iteration == 1)
					generateInitialLayers();			//Generate layers using random dual values
				else
					cont = generateLayers("normal");	//Generate layers using calculated dual values

				//std::cout << "Case\t" << q << "\t||\tIteration " << iteration << endl;

				//If no layers are generated, break
				if (iteration > 1 && cont == false)
					break;

				if (iteration > 1 && layerList.size() == nbLayers)
					break;

				//If there are no significant improvements in the last few iterations, break
				if (improvementCounter == improvementLimit)
					break;

				//If the time limit is violated, break
				if ((clock() - start) / (double)CLOCKS_PER_SEC > 1200)
					break;
				
				//If there are less than 10 items, don't generate layers
				if (itemList.size() < 10)
					break;

				nbLayers = layerList.size();

				if (iteration == 1)
				{
					//Set up one item layers to guarantee feasibility of the mathematical model
					for (int i = 0; i < uniqueItemList.size(); i++)
					{
						Layer newLayer;
						newLayer.itemList.push_back(uniqueItemList[i]);
						newLayer.orientationList.push_back(0);
						newLayer.xVec.push_back(0);
						newLayer.yVec.push_back(0);
						newLayer.layerHeight = uniqueItemList[i].h;
						newLayer.layerOccupancy = (uniqueItemList[i].d * uniqueItemList[i].w * 100.0) / (BinDepth * BinWidth);
						layerList.push_back(newLayer);
					}
				}

				if (iteration == 1)
				{
					//Build the model for the first time
					IloInt nColumns = layerList.size();
					buildModelByColumn(mod, alpha, uniqueItemList.size(), nColumns, layerList, varType, range, cost);
				}
				else
				{
					//Add new columns
					for (int k = nbLayersLast; k < layerList.size(); k++)
					{
						IloNumColumn col;
						col = cost(layerList[k].layerHeight);

						for (int i = 0; i < uniqueItemList.size(); i++)
						{
							col += range[i](0);
						}

						alpha.add(IloNumVar(col, 0, 1, varType));
						col.end();

						for (int i = 0; i < layerList[k].itemList.size(); i++)
						{
							if (layerList[k].itemList[i].SIList.size() == 0)
								range[layerList[k].itemList[i].ID].setLinearCoef(alpha[k], 1);
							else
							{
								for (int j = 0; j < layerList[k].itemList[i].SIList.size(); j++)
								{
									range[layerList[k].itemList[i].SIList[j].ID].setLinearCoef(alpha[k], 1);
								}
							}
						}
					}
				}

				//Create the mathematical model object
				IloCplex cplex(mod);
				//cplex.exportModel("Model.mps");
				cplex.setOut(env.getNullStream());
				cplex.solve();

				nbLayersLast = layerList.size();

				if (iteration == 1)
					improvementObj = cplex.getObjValue();

				//Get the objective functuion value and reset the improvement counter if necessary
				if ((improvementObj - cplex.getObjValue()) / cplex.getObjValue() > 0.001)
				{
					improvementCounter = 0;
					improvementObj = cplex.getObjValue();
				}
				else
					improvementCounter++;

				objectiveValue = cplex.getObjValue();

				//Update the dual values
				for (int i = 0; i < uniqueItemList.size(); i++)
				{
					for (int j = 0; j < itemList.size(); j++)
					{
						if (uniqueItemList[i].ID == itemList[j].ID)
							itemList[j].reducedCost = cplex.getDual(range[uniqueItemList[i].ID]);
					}
					uniqueItemList[i].reducedCost = cplex.getDual(range[i]);
				}

				//Update the dual values of the super items
				for (int i = 0; i < superItems.size(); i++)
				{
					superItems[i].reducedCost = 0;
					for (int j = 0; j < superItems[i].SIList.size(); j++)
					{
						superItems[i].reducedCost += uniqueItemList[superItems[i].SIList[j].ID].reducedCost;
					}
				}

				iteration++;
				cplex.end();
			}//while(true)

			nLayers = layerList.size();
			//Construct the bins
			constructBinsVerticalSupport();

			//Reset lists that will need to be regenerated
			superItems.clear();
			itemGroups.clear();
			alpha.end();
			constrMin.end();
			constrMax.end();
			range.end();
			mod.end();
		}//while(remainingItemList.size() > 0)

		//Write the solution
		writeBins(filename);

		//Write the total solution time
		//cout << "Total time elapsed is: " << (clock() - overallStart) / (double)CLOCKS_PER_SEC << endl;
		cout << "Finished the case:\t" << filename << endl;
		//Write solution stats
		ofstream solStats;
		char fileName[100];
		sprintf(fileName, "Results/");
		sprintf(fileName + strlen(fileName), filename.c_str());
		sprintf(fileName + strlen(fileName), "/SolutionStats.txt");
		solStats.open(fileName);

		solStats << uniqueItemList.size() << "\t" << (clock() - overallStart) / (double)CLOCKS_PER_SEC << "\t" << binList.size() << "\t" << nLayers << "\t" << nbLeftoverItems << 
			"\t" << infAreaSupport << "\t" << inf4CSupport << "\t" << infLoadBearing << "\t" << infBinWeight << endl;
		
		//Clear everything to ensure no memory leaks
		uniqueItemList.clear();
		superItems.clear();
		layerList.clear();
		selectedLayerList.clear();
		binList.clear();
		itemGroups.clear();
		coveredList.clear();
	}
}