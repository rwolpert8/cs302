#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctype.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <unordered_map>
using namespace std;

/* This code for DisjointSetByRankWPC is copied from disjoint-rank.cpp,
    but modified to be its own class to help with implementation */
class DisjointSetByRankWPC {
    public:
        DisjointSetByRankWPC(int nelements) {
            links.resize(nelements, -1);
            ranks.resize(nelements, 1);
        }

        int Union(int s1, int s2) {
            int p, c;

            if (links[s1] != -1 || links[s2] != -1) {
                cerr << "Must call union on a set, and not just an element.\n";
                exit(1);
            }

            if (ranks[s1] > ranks[s2]) {
                p = s1;
                c = s2;
            } else {
                p = s2;
                c = s1;
            }
            
            links[c] = p;
            if (ranks[p] == ranks[c]) ranks[p]++;
            return p;
        }

        int Find(int e) {
            int p, c;   // P is the parent, c is the child.

            /* Find the root of the tree, but along the way, set
                the parents' links to the children. */

            c = -1;
            while (links[e] != -1) {
                p = links[e];
                links[e] = c;
                c = e;
                e = p;
            }

            /* Now, travel back to the original element, setting
                every link to the root of the tree. */

            p = e;
            e = c;
            while (e != -1) {
                c = links[e];
                links[e] = p;
                e =c;
            }
            return p;
        }

        void Print(){
            int i;

            printf("\n");
            printf("Node:  ");
            for (i = 0; i < links.size(); i++) printf("%3d", i);  
            printf("\n");

            printf("Links: ");
            for (i = 0; i < links.size(); i++) printf("%3d", links[i]);  
            printf("\n");

            printf("Ranks: ");
            for (i = 0; i < links.size(); i++) printf("%3d", ranks[i]);  
            printf("\n\n");
        }   
    protected:
        vector <int> links;
        vector <int> ranks;
    
};

// Error handling function from sb-read.cpp, modified for sb-analyze.cpp
void usage(const char *s) 
{
  fprintf(stderr, "usage: sb-analyze rows cols min-score-size colors\n");
  if (s != NULL) fprintf(stderr, "%s\n", s);
  exit(1);
}

// Superball class from sb-read.cpp
class Superball {
public:
    Superball(int argc, char **argv);
    ~Superball();
    void analyze_superball();
    int r, c, mss, empty;
    vector<int> board;
    vector<int> goals;
    vector<int> colors;
    DisjointSetByRankWPC *d;
};

// Superball constructor also from sb-read.cpp
Superball::Superball(int argc, char **argv)
{
  int i, j;
  string s;

  if (argc != 5) usage(NULL);

  if (sscanf(argv[1], "%d", &r) == 0 || r <= 0) usage("Bad rows");
  if (sscanf(argv[2], "%d", &c) == 0 || c <= 0) usage("Bad cols");
  if (sscanf(argv[3], "%d", &mss) == 0 || mss <= 0) usage("Bad min-score-size");

  colors.resize(256, 0);

  for (i = 0; i < strlen(argv[4]); i++) {
    if (!isalpha(argv[4][i])) usage("Colors must be distinct letters");
    if (!islower(argv[4][i])) usage("Colors must be lowercase letters");
    if (colors[argv[4][i]] != 0) usage("Duplicate color");
    colors[argv[4][i]] = 2+i;
    colors[toupper(argv[4][i])] = 2+i;
  }

  board.resize(r*c);
  goals.resize(r*c, 0);

  empty = 0;

  for (i = 0; i < r; i++) {
    if (!(cin >> s)) {
      fprintf(stderr, "Bad board: not enough rows on standard input\n");
      exit(1);
    }
    if (s.size() != c) {
      fprintf(stderr, "Bad board on row %d - wrong number of characters.\n", i);
      exit(1);
    }
    for (j = 0; j < c; j++) {
      if (s[j] != '*' && s[j] != '.' && colors[s[j]] == 0) {
        fprintf(stderr, "Bad board row %d - bad character %c.\n", i, s[j]);
        exit(1);
      }
      board[i*c+j] = s[j];
      if (board[i*c+j] == '.') empty++;
      if (board[i*c+j] == '*') empty++;
      if (isupper(board[i*c+j]) || board[i*c+j] == '*') {
        goals[i*c+j] = 1;
        board[i*c+j] = tolower(board[i*c+j]);
      }
    }
  }
  d = new DisjointSetByRankWPC(r * c);
}

// Deconstructor - adaptation from original Superball class to clean up memory
Superball::~Superball() {
    delete d;
}

// Custom method to analyze board and print all possible scoring sets
void Superball::analyze_superball() {
    // Union adjacent cells of the same color
    for (int i = 0; i < r; ++i) {
        for (int j = 0; j < c; ++j) {
            if (board[i * c + j] != '.') {
                // Look to the right
                if (j < c - 1 && board[i * c + j] == board[i * c + (j + 1)]) {
                    int root1 = d->Find(i * c + j);
                    int root2 = d->Find(i * c + (j + 1));
                    if (root1 != root2) {
                        d->Union(root1, root2);
                    }
                }
                // Look to the bottom
                if (i < r - 1 && board[i * c + j] == board[(i + 1) * c + j]) {
                    int root1 = d->Find(i * c + j);
                    int root2 = d->Find((i + 1) * c + j);
                    if (root1 != root2) {
                        d->Union(root1, root2);
                    }
                }
            }
        }
    }

    // Find all connected components and their sizes
    unordered_map<int, int> component_size;
    unordered_map<int, pair<int, int>> component_goal_cell;
    for (int i = 0; i < r * c; ++i) {
        if (board[i] != '.') {
            int root = d->Find(i);
            component_size[root]++;
            if (goals[i] == 1) {
                component_goal_cell[root] = make_pair(i / c, i % c);
            }
        }
    }

    // Print all possible scoring sets
    cout << "Scoring sets:" << endl;
    for (unordered_map<int, int>::iterator it = component_size.begin(); it != component_size.end(); it++) {
        if (it->second >= mss) {
            int root = it->first;
            int size = it->second;
            char color = board[root];
            pair<int, int> goal_cell = component_goal_cell[root];
            cout << "  Size: " << setw(2) << size << "  Char: " << color << "  Scoring Cell: " << goal_cell.first << "," << goal_cell.second << endl;
        }
    }
}

int main(int argc, char **argv) {

    Superball *s = new Superball(argc, argv);
    s->analyze_superball();

    delete s;

    return 0;
} 


/*int main(int argc, char **argv) {
    Superball *s = nullptr;
    try {
        s = new Superball(argc, argv);
        s->analyze_superball();
    } catch (const exception &e) {
        cerr << "Exception: " << e.what() << endl;
    }

    delete s;
    return 0;
}*/