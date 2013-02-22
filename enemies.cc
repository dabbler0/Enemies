#include <iostream>
using namespace std;

const double TERRAIN_NUMBER = 0.5;

class Bot {
   private:
      int read;
      int write;
      int error;
   public:
      Bot (char* args[]) {
         //Set up pipes:
         int in_pipe[2];
         int out_pipe[2];
         int error_pipe[2];

         pipe(in_pipe);
         pipe(out_pipe);
         pipe(error_pipe);

         //Fork ourselves:
         int pid = fork();
         
         if (pid == -1) {
            //If the fork failed, throw an error.
            cout << "Fork error." << endl;
            throw 0;
         }
         else if (pid == 0) {
            //If we are the child, redirect pipes and execute the program.
            
            //Close unneeded pipe ends:
            close(in_pipe[1]); //STDIN write end
            close(out_pipe[0]); //STDOUT read end
            close (error_pipe[0]); //STDERR read end

            //Redirect pipes:
            if (dup2(in_pipe[0], STDIN_FILENO) < 0) {
               //If dup2 returns less than 0, an error has occurred.
               cout << "Error rerouting STDIN." << endl;
               throw 1;
            }
            if (dup2(out_pipe[1], STDOUT_FILENO) < 0) {
               //If dup2 returns less than 0, an error has occurred.
               cout << "Error rerouting STDOUT." << endl;
               throw 2;
            }
            if (dup2(error_pipe[1], STDERR_FILENO) < 0) {
               //If dup2 returns less than 0, an error has occurred.
               cout << "Error rerouting STDERR." << endl;
               throw 3;
            }
            
            //Execute the program:
            execvp(args[0], args);

            //If we reach this point, an error has occurred with execvp:
            cout << "Error executing program." << endl;
            throw 4;
         }
         else {
            //If we are the parent, store needed pipe ends.

            //Close unneeded pipe ends:
            close(in_pipe[0]); //STDIN read end
            close(out_pipe[1]); //STDOUT write end
            close(error_pipe[1]); //STDERR write end
            
            write = in_pipe[1];
            read = out_pipe[0];
            error = error_pipe[0];
         }
      }
}

struct Node {
   static int last_id = 0;
   int id;
   int color;
   double coords[2];
   double power;
   Node () {
      coords[0] = 0;
      coords[1] = 0;
      id = last_id;
      power = 1;
      color = -1;
      
      last_id += 1;
   }
   Node (double x, double y, double i) {
      coords[0] = x;
      coords[1] = y;
      id = i;
      power = 1;
      color = -1;
   }
}

class Board {
   private:
      Node* nodes;
      double** contributions;
      int l;
   public:
      Board (int n, int w, int h) {
         //Set up an array for temporary storage:
         nodes = new Node[2*n];
         contributions = new double[2*n][2*n];
         
         //Set up the contribution array:
         for (int i = 0; i < 2*n; i += 1) {
            for (int x = 0; x < 2*n; x += 1) {
               if (x == 1) [i][x] = 1;
               else [i][x] = 0;
            }
         }

         //Store the number of nodes:
         l = 2*n;
         
         //"Home" nodes:
         temp[0].color = 1; //Red home
         temp[1].coords[0] = w;
         temp[1].color = 2; //Blue home

         //Randomly generated nodes:
         for (int i = 2; i < n; i += 2) {
            //Randomly generate the coordinates of the new node:
            double x = w * rand() / RAND_MAX;
            double y = h * rand() / RAND_MAX;
            
            //Move node i into position:
            nodes[i].coords[0] = x;
            nodes[i].coords[1] = y;

            //Each node has a reflected pair, to make a fair board:
            nodes[i + 1].coords[0] = w - x;
            nodes[i + 1].coords[1] = y;
         }
      }

      double distance(int a, int b) {
         //Make sure the requested nodes are valid:
         if (a < 0 || a >= l || b < 0 || b >= l) {
            cout << "Array index out of bounds." << endl;
            throw 5;
         }
         else {
            //If they are valid, just find the distance:
            return sqrt(pow(nodes[a].coords[0] - nodes[b].coords[0], 2) + pow(nodes[a].coords[1] - nodes[b].coords[2], 2));
         }
      }
      
      bool connect(int a, int b) {
         if (a < 0 || a >= l || b < 0 || b >= l) return false;
         if (a.color != b.color) {
            //If a and b are enemies, have a attack b:
            double terrain_factor = pow(TERRAIN_NUMBER, distance(a,b));

            if (a.power * terrain_factor > b.power) {
               //If the attack is successfuly, remove all enemy paths involving b:
               for (int i = 0; i < l; i += 1) {
                  if (nodes[i].color == b.color) {
                     for (int x = 0; x < l; x += 1) {
                        if (nodes[x].color == b.color) {
                           contributions[i][x] -= contributions[i][b] * contributions[b][x];
                        }
                     }
                  }
               }
               
               //Change b's power and color:
               b.power = a.power * terrain_factor;
               b.color = a.color;

                          
               //Then go to every node owned by (a) and increase its power:
               for (int i = 0; i < l; i += 1) {
                  if (nodes[i].color == a.color) {
                     nodes[i].power += pow(TERRAIN_NUMBER, contributions[i][a]) * terrain_factor;
                     contributions[i][x] = contributions[i][a] * terrain_factor;
                  }
               }
               return true;
            }
            else {
               //Otherwise, return as such:
               return false;
            }
         }

         else {
            //If a and b are allied, record contributions and increase powers:
            double difficulty = pow(TERRAIN_NUMBER, distance(a,b));
            for (int i = 0; i < l; i += 1) {
               if (nodes[i].color == a.color) {
                  for (int x = 0; x < l; x += 1) {
                     if (nodes[x].color == a.color) {
                        //Recompute contributions:
                        double new_contribution = contributions[i][a] * difficulty * contributions[b][x];
                        contributions[i][x] += new_contribution;
                        nodes[x].power += new_contribution;
                     }
                  }
               }
            }
         }
         //If we get here, we are done!
         return true;
      }

      string fullDescription() {
         //A description of the entire board for delivery at game start:
         stringstream s;
         for (int i = 0; i < l; i += 1) {
            s << nodes[i].coords[0] << ' ' << nodes[i].coords[1] << ' ' << nodes[i].color << ' ' << nodes[i].power << ' '
         }
         return s.str();
      }

      string boardState() {
         //A description of all the nodes' powers for delivery after every move:
         stringstream s;
         for (int i = 0; i < l; i += 1) {
            s << nodes[i].power << ' ';
         }
         return s.str();
      }
}

int main() {
   //Currently, just a test of the board:
   Board test (30);
   cout << test.fullDescription() << endl;
   return 0;
}
