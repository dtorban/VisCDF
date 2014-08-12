/*
 * Copyright Regents of the University of Minnesota, 2014.  This software is released under the following license: http://opensource.org/licenses/lgpl-3.0.html.
 * Source code originally developed at the University of Minnesota Interactive Visualization Lab (http://ivlab.cs.umn.edu).
 *
 * Funding for this software development was provided in part by the NSF (IIS-1251069) and NIH (1R01EB018205-01).
 * Any opinions, findings, and conclusions or recommendations expressed in this material are those of the author(s) and do not necessarily reflect the views of the National Science Foundation.
 *
 * The content is solely the responsibility of the authors and does not necessarily represent the official views of the National Institutes of Health.
 *
 * Code author(s):
 * 		Dan Orban (dtorban)
 */

#include <stdlib.h>
#include <stdio.h>
#include <netcdf.h>
#include "netcdf/NcFileReader.h"
#include <iostream>
#include "FEASimulation.h"

#define FILE_NAME "HH168.nc"

using namespace viscdfcore;
using namespace vcdf_fea;
using namespace std;

void listDims(IVcGroup::IVcGroupRef group, int level)
{
	vector<Dimension> dims = group->getDimensions();
	for (int f = 0; f < dims.size(); f++)
	{
		for (int i = 0; i < level; i++)
		{
			cout << "\t";
		}
		cout << "-- " << dims[f].getName() << ": " << dims[f].getLength() << endl;
	}
}

void listVars(IVcGroup::IVcGroupRef group, int level)
{
	vector<VariableRef> vars = group->getVariables();
	for (int f = 0; f < vars.size(); f++)
	{
		for (int i = 0; i < level; i++)
		{
			cout << "\t";
		}

		cout << "++ " << vars[f]->getName() << "(";
		vector<Dimension> dimensions = vars[f]->getDimensions();
		for (int i = 0; i < dimensions.size(); i++)
		{
			if (i != 0)
			{
				cout << ", ";
			}
			cout << dimensions[i].getName();
		}

		cout << ")" << endl;
		if (vars[f]->getName() == "S" || vars[f]->getName() == "U")
		{
			cout << vars[f]->getAttributes()->getText("description") << endl;;
		}

		if (vars[f]->getName() == "MAGNITUDE2")
		{
			float *data = new float[vars[f]->getDimensions()[0].getLength()];
			vars[f]->getData(&data[0]);
			for (int x = 0; x < vars[f]->getDimensions()[0].getLength(); x++)
			{
				cout << data[x] << " ";
			}
			cout << endl;

			delete data;
		}
	}
}

void listGroups(IVcGroup::IVcGroupRef group, int level)
{
	for (int f = 0; f < level; f++)
	{
		cout << "\t";
	}
	listDims(group, level);
	listVars(group, level);
	
	vector<IVcGroup::IVcGroupRef> groups = group->getGroups();
	for (int f = 0; f < groups.size(); f++)
	{
		listGroups(groups[f], level + 1);
	}
}

template <typename T> T** allocate2d(int sizeX, int sizeY)
{
	T** arr = new T*[sizeX];
	for(int i = 0; i < sizeX; i++)
	{
		arr[i] = new T[sizeY];
	}

	return arr;
}

template <typename T> void deallocate2d(T** arr, int sizeX)
{
	for(int i = 0; i < sizeX; i++)
	{
		delete [] arr[i];
	}

	delete [] arr;
}

int
main()
{
	NcFileReader reader(FILE_NAME);
	cout << reader.getGroup()->getAttributes()->getText("description") << endl;
	listGroups(reader.getGroup(), 0);
	reader.close();

	FEASimulation sim(string(FILE_NAME));
	vector<string> resultSets = sim.getResultSets();
	for (int f = 0; f < resultSets.size(); f++)
	{
		cout << "Result Set " << f << ": " << resultSets[f] << endl;
	}

	vector<FEAPartRef> parts = sim.getParts();
	for (int f = 0; f < parts.size(); f++)
	{
		int numElements = parts[f]->getNumElements();
		int numNodes = parts[f]->getNumNodes();
		int connectivity = parts[f]->getConnectivity();
		vector<FEAFrameRef> frames = parts[f]->getFrames(0);

		cout << parts[f]->getName() << " ";
		cout << numElements << " ";
		cout << numNodes << " ";
		cout << connectivity << " ";
		cout << frames.size() << " ";
		cout << endl;

		float **nodes = allocate2d<float>(numNodes, 3);
		int **elements = allocate2d<int>(numElements, connectivity);

		parts[f]->loadMesh(&nodes[0][0], &elements[0][0]);

		for (int i = 0; i < 10; i++)
		{
			cout << nodes[i][0] << " " << nodes[i][1] << " " << nodes[i][2] << endl;
			cout << elements[i][0] << " " << elements[i][1] << " " << elements[i][2] << endl;
		}
		cout << "Var: " << endl;

		vector<string> variables = parts[f]->getVariables(0);
		cout << "Va2r: " << endl;

		float *var = new float[numNodes];

		for (int i = 0; i < variables.size(); i++)
		{
			cout << "Var: " << variables[i] << endl;
			frames[30]->getVariable(variables[i], var);
			cout << "  ";
			for (int j = 0; j < 5; j++)
			{
				cout << " " << var[j];
			}
			cout << endl;
		}

		for (int i = 0; i < frames.size(); i++)
		{
			//cout << "Time: " << frames[i]->getStepTime() << endl;
		}

		cout << "test1" << endl;

		float **displacement = allocate2d<float>(numNodes, 3);
		float **timeMesh = allocate2d<float>(numNodes, 3);
		
		cout << "test3" << endl;

		frames[10]->getDisplacement(&displacement[0][0]);
		frames[10]->calcDisplacement(timeMesh, nodes, numNodes);
		
		cout << "test4" << endl;

		for (int i = 0; i < 2; i++)
		{
			cout << "n " << nodes[i][0] << " " << nodes[i][1] << " " << nodes[i][2] << endl;
			cout << "d " << displacement[i][0] << " " << displacement[i][1] << " " << displacement[i][2] << endl;
			cout << "t " << timeMesh[i][0] << " " << timeMesh[i][1] << " " << timeMesh[i][2] << endl;
		}

		deallocate2d(nodes, numNodes);
		deallocate2d(elements, numElements);
		delete var;
		deallocate2d(displacement, numNodes);
		deallocate2d(timeMesh, numNodes);
	}

   cout << "Success!" << endl;
   return 0;
}
