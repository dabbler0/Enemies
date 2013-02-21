#include <iostream>
using namepsace std;

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
         int out_pipe[2]:
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
   int id;
   int color;
   double coords[2];
   double power;
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
      double** paths;
      int l;
   public:
      Board (int n, int w, int h) {
         //Set up an array for temporary storage:
         nodes = new Node[2*n];
         paths = new double[2*n][2*n];

         //Fill the path lengths with negative ones, signifying "no such path."
         for (int i = 0; i < 2*n; i += 1) {
            for (int x = 0; x < 2*n; x += 1) {
               if (x == 1) paths[i][x] = 0;
               else paths[i][x] = -1;
            }
         }

         //Store the number of nodes:
         l = 2*n;
         
         //"Home" nodes:
         Node red_home (0, 0);
         temp[0] = red_home;
         Node blue_home (w, 0);
         temp[1] = blue_home;

         //Randomly generated nodes:
         for (int i = 2; i < n; i += 2) {
            //Randomly generate the coordinates of the new node:
            double x = w * rand() / RAND_MAX;
            double y = h * rand() / RAND_MAX;

            //Each node has a reflected node, to make a fair board:
            Node k (x, y, i);
            Node l (w - x, y, i + 1)
            nodes[i] = k;
            nodes[i + 1] = l;
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
               //If the attack is successful, change b's power and color:
               b.power = a.power * terrain_factor;
               b.color = a.color;
            
               //Then go to every owned node and increase its power:
               for (int i = 0; i < l; i += 1) {
                  if (nodes[i].color == a.color) {
                     nodes[i].power += pow(TERRAIN_NUMBER, paths[i][a]) * terrain_factor;
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
            //If a and b are allied, shorten paths and increase powers:
            double distance = distance(a,b);
            for (int i = 0; i < l; i += 1) {
               if (nodes[i].color == a.color) {
                  for (int x = 0; x < l; x += 1) {
                     if (nodes[x].color == a.color && paths[i][x] > paths[i][a] + distance + paths[b][x]) {
                        
                        //Unlink the old path:
                        nodes[i].power -= pow(TERRAIN_NUMBER, paths[i][x]);
                        nodes[x].power -= pow(TERRAIN_NUMBER, paths[i][x]);
                        
                        //Link up the new path:

                        paths[i][x] = paths[i][a] + distance + paths[b][x]; 
                     }
                  }
               }
            }
         }
      }
}
