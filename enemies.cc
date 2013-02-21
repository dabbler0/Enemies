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

class Node {
   private:
      double p;
      double x;
      double y;
      int c;
      forward_list<Node*> connections;
   public:
      Node (double coords[2]) {
         p = 1;
         x = coords[0];
         y = coords[1];
         c = -1;
      }
      double* coords() {
         //Return the coordinates of this node.
         double r = {x,y};
         return (double*) r;
      }
      double power() {
         //Return the power of this node.
         return p;
      }
      int color() {
         //Return the color of this node.
         return c;
      }
      int color(int n) {
         //Set the color of this node to n.
         c = n;
         return c;
      }
      double distanceTo(Node other) {
         //Return the metric distance to another node.
         double* other_coords = other.coords();
         return sqrt(pow(other_coords[0] - x, 2) + pow(other_coords[1] - y, 2))
      }
      void disconnect() {
         //Remove all connections.
         connections.clear();
      }
      void push_connection (Node* other) {
         //Add a connection.
         connections.push_front(other);
      }
      bool connect(Node other) {
         if (other.color() != c) {
            //If the node is not ours, check to make sure we can take it over:
            if (other.power() < p * pow(TERRAIN_NUMBER, distanceTo(other))) {
               //If we can, cut all its connections and make it our color.
               other.disconnect();
               other.color(c);
            }
            else {
               //Otherwise, the connection fails.
               return false;
            }
         }

         //Connect us two:
         other.push_connection(this);
         connections.push_front(&other);
      
         
      }
}

class Board {
   private:
      
}
