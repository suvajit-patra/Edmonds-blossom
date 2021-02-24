/*
Edmonds' Blossom Shrinking Algorithm for maximum matching in general graphs
Implemented by Suvajit Patra, MSc. First Year student at RKMVERI, Belur, West Bengal, India
*/

#include <iostream>
#include <fstream>
#include <bits/stdc++.h>

#define MAX_V 100 //Maximum no. of vertices Graph can hold

using namespace std;

class Graph
{
public:
    int n;               //No. of vertices
    int G[MAX_V][MAX_V]; //Graph G
    int M[MAX_V];        //Matched edges
    vector<int> getFreeVerVector();
    set<int> getAdjVers(int);
    void updateM(vector<int>);
    Graph();
    ~Graph();
};

vector<int> Graph::getFreeVerVector()
{
    vector<int> free_vertices;
    for (int i = 0; i < n; i++)
    {
        if (M[i] == -1)
            free_vertices.push_back(i);
    }
    return free_vertices;
}

set<int> Graph::getAdjVers(int vertex)
{
    set<int> adj_vertices;
    for (int i = 0; i < n; i++)
    {
        if (G[vertex][i] == 1 && i != vertex)
            adj_vertices.insert(i);
    }
    return adj_vertices;
}

void Graph::updateM(vector<int> aug_path)
{
    if (aug_path.size() < 2)
        return;

    int count = 0;
    for (auto itr = aug_path.begin(); itr != aug_path.end(); itr++)
    {
        if (count % 2 == 0)
        {
            if (itr + 1 != aug_path.end())
                M[*itr] = *(itr + 1);
        }
        else
            M[*itr] = *(itr - 1);
        count++;
    }
}

Graph::Graph()
{
    //Initialize the graph and Matched edges
    n = 0;
    for (int i = 0; i < MAX_V; i++)
    {
        M[i] = -1;
        for (int j = 0; j < MAX_V; j++)
            G[i][j] = 0;
    }
}

Graph::~Graph()
{
}

class ForestVertex
{
public:
    bool in_forest;                 //vertex is inside forest or not
    int parent, root, dist_to_root; //parent vertex, root of the vertex in the forest, distance of the vertex from it's root
    ForestVertex();
    ~ForestVertex();
};

ForestVertex::ForestVertex()
{
    in_forest = false;
    parent = -1;
    root = -1;
    dist_to_root = -1;
}

ForestVertex::~ForestVertex()
{
}

int getPrvVerFromMap(int map[], int n, int mapped_ver);
Graph contractGraph(Graph &graph, int map[], set<int> B);                                               //contracts a graph and returns contracted graph
vector<int> shortestPath(ForestVertex F[], int descendant_vertex, int ancestor_vertex);                 //shortest path from descendent tree node to ancestor tree node
void addToForest(Graph &graph, ForestVertex F[], int vertex, int adj_ver, vector<int> *nodes_to_check); //add vertices and edges to the forest
vector<int> returnAugPath(ForestVertex F[], int vertex, int adj_ver);                                   //returns simple augmented path
vector<int> blossomRecursion(Graph &graph, ForestVertex F[], int vertex, int adj_ver);                  //handles blossoms or odd cycles
vector<int> findAugPath(Graph &graph);                                                                  //finds augmented path in general graph

int getPrvVerFromMap(int map[], int n, int mapped_ver)
{
    for (int i = 0; i < n; i++)
    {
        if (map[i] == mapped_ver)
        {
            return i;
        }
    }

    return -1;
}

Graph contractGraph(Graph &graph, int map[], set<int> B)
{
    Graph new_graph;
    for (int i = 0; i < graph.n; i++)
    {
        if (graph.M[i] > -1 && find(B.begin(), B.end(), i) == B.end())
        {
            new_graph.M[map[i]] = map[graph.M[i]];
            new_graph.M[map[graph.M[i]]] = map[i];
        }

        for (int j = 0; j < graph.n; j++)
            if (graph.G[i][j] == 1)
                new_graph.G[map[i]][map[j]] = 1;
    }

    new_graph.n = graph.n - B.size() + 1;

    return new_graph;
}

vector<int> shortestPath(ForestVertex F[], int descendant_vertex, int ancestor_vertex)
{
    vector<int> P;
    while (descendant_vertex != ancestor_vertex)
    {
        P.push_back(descendant_vertex);
        descendant_vertex = F[descendant_vertex].parent;
    }
    P.push_back(ancestor_vertex);
    return P;
}

void addToForest(Graph &graph, ForestVertex F[], int vertex, int adj_ver, vector<int> *nodes_to_check)
{
    int end_vertex = graph.M[adj_ver];
    nodes_to_check->push_back(end_vertex);
    F[adj_ver].in_forest = true;
    F[end_vertex].in_forest = true;
    F[adj_ver].root = F[vertex].root;
    F[end_vertex].root = F[vertex].root;
    F[adj_ver].parent = vertex;
    F[end_vertex].parent = adj_ver;
    F[adj_ver].dist_to_root = F[vertex].dist_to_root + 1;
    F[end_vertex].dist_to_root = F[adj_ver].dist_to_root + 1;
}

vector<int> returnAugPath(ForestVertex F[], int vertex, int adj_ver)
{
    vector<int> P1 = shortestPath(F, vertex, F[vertex].root);
    vector<int> P2 = shortestPath(F, adj_ver, F[adj_ver].root);
    reverse(P1.begin(), P1.end());
    P1.insert(P1.end(), P2.begin(), P2.end());
    return P1;
}

vector<int> blossomRecursion(Graph &graph, ForestVertex F[], int vertex, int adj_ver)
{
    vector<int> P1 = shortestPath(F, vertex, F[vertex].root);   //get first vertex and its root's path
    vector<int> P2 = shortestPath(F, adj_ver, F[adj_ver].root); //get second vertex and its root's path
    reverse(P1.begin(), P1.end());
    reverse(P2.begin(), P2.end());
    int counter = 0;
    for (counter = 0; counter < P1.size() && counter < P2.size(); counter++)
        if (P1[counter] != P2[counter])
            break;

    set<int> B; //set of blossom vertices

    for (int i = counter - 1; i < P1.size(); i++)
        B.insert(P1[i]);
    for (int i = counter - 1; i < P2.size(); i++)
        B.insert(P2[i]);

    int map[graph.n] = {-1}; //maps the vertices in graph to contracted graph
    int index = 0;
    int i;
    for (i = 0; i < graph.n; i++)
    {
        if (B.find(i) == B.end())
        {
            map[i] = index;
            index++;
        }
    }
    cout << "Blossom (";
    for (auto itr = B.begin(); itr != B.end(); itr++)
    {
        if (itr != B.begin())
            cout << ",";
        cout << *itr;
        map[*itr] = index;
    }
    cout << ")" << endl;

    int contracted_vertex = index;

    Graph contracted_graph = contractGraph(graph, map, B); //contract the graph with the vertices of B

    P1.erase(P1.begin(), P1.begin() + (counter - 1));
    P2.erase(P2.begin(), P2.begin() + (counter - 1)); //erase both sides same stem
    reverse(P2.begin(), P2.end());
    P1.insert(P1.end(), P2.begin(), P2.end());

    vector<int> circular_path = P1;

    vector<int> P_B = findAugPath(contracted_graph); //call findAugPath to get augmented path in contracted graph

    if (P_B.size() < 2)
        return P_B;

    if (find(P_B.begin(), P_B.end(), contracted_vertex) != P_B.end()) //if contracted vertex is in the augmented path
    {
        int side, left, right, adj_in_B;
        left = 1;
        right = 0;
        adj_in_B = -1; //vertex in blossom, which is adjacent to an unmatched edge of stem

        auto ver_ptr = find(P_B.begin(), P_B.end(), contracted_vertex);
        if (*ver_ptr == P_B.front() || *ver_ptr == P_B.back()) //if contracted vertex is the last or fast element in augmented path
        {
            set<int> adj_vers_of_out_B_ver; //all the adjacent vertices of the vertex that is on the other side of the blossom of the unmatched vertex
            if (*ver_ptr == P_B.back())
            { //the unmatched edge is on the left side of the blossom
                side = left;
                adj_vers_of_out_B_ver = graph.getAdjVers(getPrvVerFromMap(map, graph.n, *(ver_ptr - 1)));
            }
            else
            { //the unmatched edge is on the right side of the blossom
                side = right;
                adj_vers_of_out_B_ver = graph.getAdjVers(getPrvVerFromMap(map, graph.n, *(ver_ptr + 1)));
            }
            for (auto i = adj_vers_of_out_B_ver.begin(); i != adj_vers_of_out_B_ver.end(); i++) //find the adjacent vertex of the unmatched edge that is in blossom
            {
                if (find(B.begin(), B.end(), *i) != B.end())
                {
                    adj_in_B = *i;
                    break;
                }
            }
        }
        else //if contracted vertex is not the edge element in augmented path
        {
            set<int> adj_vers_of_out_B_ver; //all the adjacent vertices of the vertex that is on the other side of the blossom of the unmatched vertex
            if (contracted_graph.M[contracted_vertex] == *(ver_ptr + 1))
            { //the unmatched edge is on the left side of the blossom
                side = left;
                adj_vers_of_out_B_ver = graph.getAdjVers(getPrvVerFromMap(map, graph.n, *(ver_ptr - 1)));
            }
            else
            { //the unmatched edge is on the right side of the blossom
                side = right;
                adj_vers_of_out_B_ver = graph.getAdjVers(getPrvVerFromMap(map, graph.n, *(ver_ptr + 1)));
            }
            for (auto i = adj_vers_of_out_B_ver.begin(); i != adj_vers_of_out_B_ver.end(); i++) //find the adjacent vertex of the unmatched edge that is in blossom
            {
                if (find(B.begin(), B.end(), *i) != B.end())
                {
                    adj_in_B = *i;
                    break;
                }
            }
        }

        int pos = ver_ptr - P_B.begin();
        P_B.erase(ver_ptr); //erase the contracted vertex from the path

        vector<int> temp_path;
        for (auto ver = P_B.begin(); ver != P_B.end(); ver++)
            temp_path.push_back(getPrvVerFromMap(map, graph.n, *ver));
        P_B = temp_path;

        auto B_ver_ptr = find(circular_path.begin(), circular_path.end(), adj_in_B);
        if (*B_ver_ptr == circular_path.front() || *B_ver_ptr == circular_path.back())
        {
            if (*B_ver_ptr == circular_path.front())
                P_B.insert(P_B.begin() + pos, circular_path.begin(), circular_path.begin() + 1);
            else
                P_B.insert(P_B.begin() + pos, circular_path.end() - 1, circular_path.end());
        }
        else
        {
            vector<int> path_in_B;
            if ((B_ver_ptr - circular_path.begin()) % 2 == 0) //if in the circular path from begin to the vertex edge count is even
            {
                path_in_B.insert(path_in_B.begin(), circular_path.begin(), B_ver_ptr + 1);
                if (side == left) //if unmatched edge is on the left side reverse the path in blossom
                    reverse(path_in_B.begin(), path_in_B.end());
                P_B.insert(P_B.begin() + pos, path_in_B.begin(), path_in_B.end());
            }
            else //if in the circular path from the vertex to end edge count is odd
            {
                path_in_B.insert(path_in_B.begin(), B_ver_ptr, circular_path.end());
                if (side == right) //if unmatched edge is on the right side reverse the path in blossom
                    reverse(path_in_B.begin(), path_in_B.end());
                P_B.insert(P_B.begin() + pos, path_in_B.begin(), path_in_B.end());
            }
        }
    }
    else //if contracted vertex is not in the augmented path
    {
        vector<int> temp_path;
        for (auto ver = P_B.begin(); ver != P_B.end(); ver++)
            temp_path.push_back(getPrvVerFromMap(map, graph.n, *ver));
        P_B = temp_path;
    }

    return P_B;
}

vector<int> findAugPath(Graph &graph)
{
    ForestVertex F[graph.n]; //empty forest
    vector<int> P;
    vector<int> nodes_to_check = graph.getFreeVerVector();
    for (auto itr = nodes_to_check.begin(); itr != nodes_to_check.end(); itr++) //insert all the nodes to check to forest and make their roots to itself
    {
        F[*itr].in_forest = true;
        F[*itr].root = *itr;
        F[*itr].parent = *itr;
        F[*itr].dist_to_root = 0;
    }
    int G_marked[MAX_V][MAX_V];
    for (int i = 0; i < graph.n; i++) //mark all matched edges
    {
        for (int j = 0; j < graph.n; j++)
        {
            if (graph.G[i][j] == 1)
            {
                if (j == graph.M[i])
                    G_marked[i][j] = 2;
                else
                    G_marked[i][j] = 1;
            }
            else
                G_marked[i][j] = 0;
        }
    }
    int nodes_to_check_size = nodes_to_check.size();
    for (int i = 0; i < nodes_to_check_size; i++)
    {
        int vertex = nodes_to_check[i];
        set<int> adj_vertices = graph.getAdjVers(vertex);
        for (auto adj_ver = adj_vertices.begin(); adj_ver != adj_vertices.end(); adj_ver++)
        {
            if (G_marked[vertex][*adj_ver] == 1) //check if edge is unmarked of not
            {
                cout << vertex << " -> " << *adj_ver << endl;
                if (!F[*adj_ver].in_forest)
                {
                    addToForest(graph, F, vertex, *adj_ver, &nodes_to_check);
                    nodes_to_check_size++;
                }
                else if (F[*adj_ver].dist_to_root % 2 == 0)
                {
                    if (F[vertex].root != F[*adj_ver].root)
                        P = returnAugPath(F, vertex, *adj_ver);
                    else
                        P = blossomRecursion(graph, F, vertex, *adj_ver);
                    return P;
                }
                G_marked[vertex][*adj_ver] = G_marked[*adj_ver][vertex] = 2; //mark the edge
            }
        }
    }

    return P;
}

void findMaxMatching(Graph &graph)
{
    vector<int> P = findAugPath(graph);
    if (P.size() < 2)
        return;
    else
    {
        cout << "Augmenting Path (";
        for (auto itr = P.begin(); itr != P.end(); itr++)
        {
            cout << *itr;
            if (itr + 1 != P.end())
                cout << "-";
        }
        cout << ")" << endl;
        graph.updateM(P);
        return findMaxMatching(graph);
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        cout << "Please insert text file of graph as command line argument" << endl;
        return 1;
    }
    string line;
    Graph mainGraph;
    ifstream myfile(argv[1]);

    if (myfile.is_open())
    {
        int r, c;
        r = c = 0;
        while (getline(myfile, line))
        {
            for (int i = 0; i < line.length(); i++)
            {
                if (line[i] == '0' || line[i] == '1')
                {
                    mainGraph.G[r][c] = line[i] - '0';
                    cout << mainGraph.G[r][c] << " ";
                    c++;
                }
            }
            cout << endl;
            c = 0;
            r++;
        }

        mainGraph.n = r;

        findMaxMatching(mainGraph);

        cout << "Maximum Matching = ";
        for (int i = 0; i < mainGraph.n; i++)
            cout << i << "[" << mainGraph.M[i] << "] ";
        cout << endl;

        myfile.close();
    }
    else
        cout << "Unable to open file";

    return 0;
}