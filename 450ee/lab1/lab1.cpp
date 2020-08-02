#include <iostream>
#include <cstring>
// #include <iomanip>
#include <fstream>
#include <sstream>
using namespace std;

typedef struct edges {
	int src;
	int dest;
	int weight;
} Edge;

typedef struct graphs {
	int num_vert;
	int num_edge;
	Edge *edge;
} Graph;

/* Add edges obtained from the input file into the stucture */

void constructEdges(Graph *graph, int edge_weight, int u, int v)
{
	graph->edge[graph->num_edge].src = u;
	graph->edge[graph->num_edge].dest = v;
	graph->edge[graph->num_edge].weight = edge_weight;
	graph->num_edge++;
}

/* Bellman Ford Algorithm */

void applyBellmanFord(Graph *graph, int source, ofstream& fout)
{
	int vert = graph->num_vert;
	int edg = graph->num_edge;
	int distance[graph->num_vert];
	int preceeding[graph->num_vert];

/* Initiate distances from source to each of the vertex as infinity (here INT_MAX) */

	for (int i = 0; i < vert; ++i)
	{
		distance[i] = INT_MAX;
	}
	distance[source] = 0;
	preceeding[source] = 0;

/* Iterate V - 1 times to obtain the min cost path */

	for (int i = 1; i <= vert - 1; ++i)
	{
		int stop_iter = 1;
		for (int j = 0; j < edg; ++j)
		{

/* Store the min cost value in the distance array */

			if (distance[graph->edge[j].src] != INT_MAX && distance[graph->edge[j].src] + graph->edge[j].weight < distance[graph->edge[j].dest])
			{
				distance[graph->edge[j].dest] = distance[graph->edge[j].src] + graph->edge[j].weight;
				preceeding[graph->edge[j].dest] = graph->edge[j].src;
				stop_iter = 0;
			}
		}

/* Terminate early if no change in min cost values in distance array */

		if (stop_iter == 1)
		{
			break;
		}
	}

/* Detect negetive cost cycles in the graph by running one extra time */

	for (int i = 0; i < edg; ++i)
	{
		if (distance[graph->edge[i].src] != INT_MAX && distance[graph->edge[i].src] + graph->edge[i].weight < distance[graph->edge[i].dest])
		{
			cout << "Graph contains negetive cost cycles." << endl;
		}
	}

/* Write min cost values separated by commas into the output file */

	fout << distance[0];
	for (int i = 1; i < graph->num_vert; ++i)
	{
		fout << "," << distance[i];
	}
	fout << endl;

/* Write min cost path from source to each node in the graph */

	for (int i = 0; i < graph->num_vert; ++i)
	{
		int temp = i;
		string printPath;
		// printPath = to_string(temp);
		printPath = static_cast<ostringstream*>(&(ostringstream() << temp)) -> str();
		while (temp > 0)
		{
			string tempPath = static_cast<ostringstream*>(&(ostringstream() << preceeding[temp])) -> str();
			// printPath = to_string(preceeding[temp]) + "->" + printPath;
			printPath = tempPath + "->" + printPath;
			temp = preceeding[temp];
		}
		fout << printPath << endl;
	}
}

/* Main function */

int main(int argc, char *argv[])
{
	Graph *graph = (Graph*)malloc(sizeof(Graph));
	string line;
	string filename;
	filename = strcat(argv[1], ".txt");

/* Open input file based on the given command line argument */

	ifstream inFile (filename.c_str());
	string outfilename = "output-" + filename;
	if (!inFile.is_open())
	{
		cout << "Could not open input file " << argv[1] << ".txt";
	}
	else
	{
		getline(inFile, line);
		graph->num_vert = atoi(line.c_str());
		graph->num_edge = 0;
		graph->edge = (Edge*)malloc(graph->num_vert * graph->num_vert * sizeof(Edge));
		for (int i = 0; i < graph->num_vert; ++i)
		{
			getline(inFile, line);
			istringstream ss(line);
			string token;

/* Split the input string delimited by commas and put it into the structure */

			for (int j = 0; j < graph->num_vert; ++j)
			{
				getline(ss, token, ',');
				int edge_weight = atoi(token.c_str());
				if (edge_weight != 0)
				{
					constructEdges(graph, edge_weight, i, j);
				}
			}
		}

/* Open output file */

		// ofstream outFile ("outfile.txt");
		ofstream outFile (outfilename.c_str());
		if (!outFile.is_open())
		{
			cout << "Could not open output file " << endl;
		}
		else
		{

/* Apply Bellman Ford algorithm */

			applyBellmanFord(graph, 0, outFile);
		}

/* Close all files */

		inFile.close();
		outFile.close();
	}
	return 0;
}
